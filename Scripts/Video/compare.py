#!/usr/bin/env python3
"""Side-by-side (or stacked) comparison sheet: original frame vs game capture.

Usage:
  python3 Scripts/Video/compare.py --pair "1:100.5=cap_00120.bmp" --pair "2:250=cap_00300.bmp" --out cmp.png [--stack] [--tick 120,300] [--scores]
  python3 Scripts/Video/compare.py --json pairs.json --out cmp.png
Pair syntax: VIDEO:TIME=GAMEIMAGE   (VIDEO 1|2, TIME seconds or m:ss; GAMEIMAGE is a path; tick is parsed from a trailing _NNNNN in the name)
  An ORIGINAL side can also be an existing image: "file.png=cap.bmp".
JSON: [{"video":1,"time":100.5,"game":"cap_00120.bmp","tick":120}, {"original":"x.png","game":"y.bmp","label":"..."}]
Original frames are pulled with ffmpeg, cropped (extract.py rectangles) and nearest-upscaled to 640x448;
the game capture is resized to 640x448 if needed. Each row: "ORIGINAL t=..." | "GAME tick=...". --stack puts them vertically instead.
--scores appends SSIM score (diffscore) to the game label. Output width capped at --max-width (default 1800).
"""
import argparse, json, os, re, subprocess, sys, tempfile
from PIL import Image, ImageDraw, ImageFont

sys.path.insert(0, os.path.dirname(__file__))
from extract import CROP, GAME_SIZE, parse_time, video_path


def grab(video, t):
    w, h, x, y = CROP[video]
    with tempfile.TemporaryDirectory() as d:
        p = os.path.join(d, "f.png")
        subprocess.check_call(["ffmpeg", "-v", "error", "-y", "-ss", "%.3f" % t, "-i", video_path(video),
                               "-frames:v", "1", "-vf", "crop=%d:%d:%d:%d,scale=%d:%d:flags=neighbor" % (w, h, x, y, *GAME_SIZE), p])
        return Image.open(p).convert("RGB")


def parse_pairs(a):
    items = []
    if a.json:
        for e in json.load(open(a.json)):
            items.append(e)
    for s in a.pair or []:
        left, game = s.split("=", 1)
        if ":" in left and left.split(":", 1)[0] in ("1", "2"):
            v, t = left.split(":", 1)
            items.append({"video": int(v), "time": parse_time(t), "game": game})
        else:
            items.append({"original": left, "game": game})
    ticks = [int(x) for x in a.tick.split(",")] if a.tick else []
    for i, e in enumerate(items):
        if "tick" not in e:
            m = re.search(r"_(\d{5})\.\w+$", e["game"])
            if i < len(ticks):
                e["tick"] = ticks[i]
            elif m:
                e["tick"] = int(m.group(1))
    return items


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--pair", action="append")
    ap.add_argument("--json")
    ap.add_argument("--out", required=True)
    ap.add_argument("--stack", action="store_true")
    ap.add_argument("--tick")
    ap.add_argument("--scores", action="store_true")
    ap.add_argument("--max-width", type=int, default=1800)
    a = ap.parse_args()
    items = parse_pairs(a)
    if not items:
        raise SystemExit("no pairs")
    font = ImageFont.load_default()
    W, H = GAME_SIZE
    lab, pad = 16, 4
    rows = []
    for e in items:
        if "original" in e:
            o = Image.open(e["original"]).convert("RGB").resize(GAME_SIZE, Image.NEAREST)
            ol = e.get("label", os.path.basename(e["original"]))
        else:
            o = grab(e["video"], parse_time(str(e["time"])))
            t = parse_time(str(e["time"]))
            ol = "ORIGINAL v%d t=%.2fs (%d:%05.2f)" % (e["video"], t, t // 60, t % 60)
        g = Image.open(e["game"]).convert("RGB")
        if g.size != GAME_SIZE:
            g = g.resize(GAME_SIZE, Image.NEAREST)
        gl = "GAME tick=%s" % e["tick"] if "tick" in e else "GAME " + os.path.basename(e["game"])
        if a.scores:
            from diffscore import score
            with tempfile.TemporaryDirectory() as d:
                o.save(d + "/o.png"); g.save(d + "/g.png")
                gl += "  ssim=%.3f" % score(d + "/o.png", d + "/g.png")["score"]
        rows.append((o, ol, g, gl))
    if a.stack:
        sheet = Image.new("RGB", (pad * 2 + W, pad + len(rows) * 2 * (H + lab + pad)), (24, 24, 24))
        y = pad
        cells = [(im, l) for o, ol, g, gl in rows for im, l in ((o, ol), (g, gl))]
        for im, l in cells:
            sheet.paste(im, (pad, y + lab)); ImageDraw.Draw(sheet).text((pad + 2, y + 2), l, fill=(255, 255, 0), font=font)
            y += H + lab + pad
    else:
        sheet = Image.new("RGB", (pad * 3 + 2 * W, pad + len(rows) * (H + lab + pad)), (24, 24, 24))
        d = ImageDraw.Draw(sheet)
        for i, (o, ol, g, gl) in enumerate(rows):
            y = pad + i * (H + lab + pad)
            sheet.paste(o, (pad, y + lab)); sheet.paste(g, (pad * 2 + W, y + lab))
            d.text((pad + 2, y + 2), ol, fill=(255, 255, 0), font=font)
            d.text((pad * 2 + W + 2, y + 2), gl, fill=(0, 255, 255), font=font)
    if sheet.width > a.max_width:
        sheet = sheet.resize((a.max_width, int(sheet.height * a.max_width / sheet.width)), Image.LANCZOS)
    sheet.save(a.out)
    print("wrote %s %s" % (a.out, sheet.size))


if __name__ == "__main__":
    main()
