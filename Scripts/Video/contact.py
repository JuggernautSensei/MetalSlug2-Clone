#!/usr/bin/env python3
"""Labeled contact sheet from a folder of frames.

Usage:
  python3 Scripts/Video/contact.py FRAMES_DIR OUT.png [--cols 5] [--max-width 1800] [--limit N] [--start-index I]
Label = file stem (for extract.py output "t0100.500" = video seconds; for game captures "..._00120" = tick),
with m:ss converted when the name looks like tNNNN.NNN.
Frames are shrunk (not enlarged) so the sheet is <= --max-width px wide.
"""
import argparse, os, glob, re
from PIL import Image, ImageDraw, ImageFont


def label_for(path):
    stem = os.path.splitext(os.path.basename(path))[0]
    m = re.fullmatch(r"t(\d+\.\d+)", stem)
    if m:
        t = float(m.group(1))
        return "%s (%d:%05.2f)" % (stem[1:], int(t // 60), t % 60)
    return stem


def build_sheet(files, cols=5, max_width=1800, pad=4, labels=None):
    imgs = [Image.open(f).convert("RGB") for f in files]
    cell_w = (max_width - pad * (cols + 1)) // cols
    scale = min(1.0, cell_w / imgs[0].width)
    cw, ch = int(imgs[0].width * scale), int(imgs[0].height * scale)
    rows = (len(imgs) + cols - 1) // cols
    lab_h = 14
    sheet = Image.new("RGB", (pad + cols * (cw + pad), pad + rows * (ch + lab_h + pad)), (24, 24, 24))
    d = ImageDraw.Draw(sheet)
    font = ImageFont.load_default()
    for i, im in enumerate(imgs):
        x = pad + (i % cols) * (cw + pad)
        y = pad + (i // cols) * (ch + lab_h + pad)
        sheet.paste(im.resize((cw, ch), Image.NEAREST if scale == 1 else Image.LANCZOS), (x, y + lab_h))
        text = labels[i] if labels else label_for(files[i])
        d.rectangle([x, y, x + cw, y + lab_h - 1], fill=(0, 0, 0))
        d.text((x + 2, y + 1), text, fill=(255, 255, 0), font=font)
    return sheet


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("frames_dir")
    ap.add_argument("out")
    ap.add_argument("--cols", type=int, default=5)
    ap.add_argument("--max-width", type=int, default=1800)
    ap.add_argument("--limit", type=int, default=0)
    ap.add_argument("--start-index", type=int, default=0)
    a = ap.parse_args()
    files = sorted(glob.glob(os.path.join(a.frames_dir, "*.png")) + glob.glob(os.path.join(a.frames_dir, "*.bmp")))
    files = [f for f in files if not os.path.basename(f).startswith("_tmp")][a.start_index:]
    if a.limit:
        files = files[:a.limit]
    if not files:
        raise SystemExit("no frames")
    build_sheet(files, a.cols, a.max_width).save(a.out)
    print("wrote %s (%d frames)" % (a.out, len(files)))


if __name__ == "__main__":
    main()
