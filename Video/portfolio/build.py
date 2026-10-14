"""script.json -> assets/clips/*.mp4 + index.html"""
import html
import json
import os
import subprocess
import sys

ROOT = os.path.dirname(os.path.abspath(__file__))
CLIPS = os.path.join(ROOT, "assets", "clips")
W, H = 1920, 1080
PLAY_W = 1552
CARD = (140, 150, 1640, 780)  # x, y, w, h area for editor cards
CHAPTER_LEN = 2.6
XF = 0.3  # crossfade between scenes


def ffmpeg(args):
    subprocess.run(["ffmpeg", "-loglevel", "error", "-y", *args], check=True)


def cut(src, start, length, vf, out, audio):
    if os.path.exists(out):
        return
    a = ["-c:a", "aac", "-b:a", "160k"] if audio else ["-an"]
    ffmpeg(["-ss", str(start), "-t", str(length), "-i", src, "-vf", vf, "-r", "30",
            "-c:v", "libx264", "-preset", "medium", "-crf", "20", "-pix_fmt", "yuv420p", *a, out])


def game_vf(crop):
    x, y, w, h = crop
    return f"crop={w}:{h}:{x}:{y},scale={PLAY_W}:{H}:flags=neighbor"


def fit(cw, ch):
    ax, ay, aw, ah = CARD
    s = min(aw / cw, ah / ch)
    w, h = round(cw * s / 2) * 2, round(ch * s / 2) * 2
    return ax + (aw - w) // 2, ay + (ah - h) // 2, w, h


def esc(s):
    return html.escape(s, quote=True)


def hl(text, word):
    t = esc(text)
    return t.replace(esc(word), f'<span class="hl">{esc(word)}</span>', 1) if word else t


class Doc:
    def __init__(self):
        self.t = 0.0
        self.els = []
        self.tw = []
        self.n = 0

    def uid(self, p):
        self.n += 1
        return f"{p}{self.n}"

    def scene(self, dur, inner, fade_in=True):
        sid = self.uid("sc")
        self.els.append(f'<section id="{sid}" class="clip" data-start="{self.t:.2f}" data-duration="{dur + XF:.2f}" data-track-index="0"><div class="wrap" id="{sid}w">{inner}</div></section>')
        if fade_in:
            self.tw.append(f'tl.fromTo("#{sid}w",{{opacity:0}},{{opacity:1,duration:{XF}}},{self.t:.2f});')
        self.tw.append(f'tl.to("#{sid}w",{{opacity:0,duration:{XF}}},{self.t + dur - XF:.2f});')
        return sid

    def video(self, src, start, dur, box, audio, fade=True):
        vid = self.uid("v")
        x, y, w, h = box
        snd = 'data-has-audio="true"' if audio else "muted"
        radius = "18px" if w < W else "0"
        self.els.append(f'<video id="{vid}" class="clip media" src="{src}" {snd} playsinline data-start="{start:.2f}" data-duration="{dur:.2f}" data-track-index="1" style="left:{x}px;top:{y}px;width:{w}px;height:{h}px;border-radius:{radius}"></video>')
        if fade:
            self.tw.append(f'tl.fromTo("#{vid}",{{opacity:0}},{{opacity:1,duration:{XF}}},{start:.2f});')
            self.tw.append(f'tl.to("#{vid}",{{opacity:0,duration:{XF}}},{start + dur - XF:.2f});')

    def caption(self, start, dur, text, band=False):
        cid = self.uid("c")
        cls = "clip cap band" if band else "clip cap"
        self.els.append(f'<div id="{cid}" class="{cls}" data-start="{start:.2f}" data-duration="{dur:.2f}" data-track-index="3"><p id="{cid}p">{esc(text)}</p></div>')
        self.tw.append(f'tl.fromTo("#{cid}p",{{opacity:0}},{{opacity:1,duration:0.2}},{start:.2f});')
        self.tw.append(f'tl.to("#{cid}p",{{opacity:0,duration:0.2}},{start + dur - 0.2:.2f});')


def header(title, sub, badge):
    return (f'<div class="scene-title"><h2>{title}</h2><p>{esc(sub)}</p></div>'
            f'<div class="badge"><b>{esc(badge[0])}</b>{esc(badge[1])}</div>')


def build(spec):
    src = spec["sources"]
    d = Doc()
    chapter = ("", "")
    for i, s in enumerate(spec["scenes"]):
        k = s["type"]
        if k == "trailer":
            out = os.path.join(CLIPS, "trailer.mp4")
            if not os.path.exists(out):
                parts = []
                for j, c in enumerate(s["cuts"]):
                    p = os.path.join(CLIPS, f"_t{j}.mp4")
                    cut(src[s["src"]], c, s["cutLen"], game_vf(spec["gameCrop"]), p, True)
                    parts.append(p)
                lst = os.path.join(CLIPS, "_t.txt")
                with open(lst, "w", encoding="utf-8") as f:
                    f.writelines(f"file '{p.replace(os.sep, '/')}'\n" for p in parts)
                ffmpeg(["-f", "concat", "-safe", "0", "-i", lst, "-c", "copy", out])
                for p in parts + [lst]:
                    os.remove(p)
            n = len(s["cuts"]) * s["cutLen"]
            sid = d.scene(n, "", fade_in=False)
            d.video("assets/clips/trailer.mp4", d.t, n, ((W - PLAY_W) // 2, 0, PLAY_W, H), True, fade=False)
            for c in s["cards"]:
                cid = d.uid("tc")
                d.els.append(f'<div id="{cid}" class="clip tcard" data-start="{d.t + c["at"]:.2f}" data-duration="{c["dur"]:.2f}" data-track-index="2"><div id="{cid}i" class="tcard-in"><p>{esc(c["label"])}</p><h3>{esc(c["title"])}</h3></div></div>')
                d.tw.append(f'tl.fromTo("#{cid}i",{{opacity:0,y:14}},{{opacity:1,y:0,duration:0.6,ease:"power2.out"}},{d.t + c["at"]:.2f});')
                d.tw.append(f'tl.to("#{cid}i",{{opacity:0,duration:0.5}},{d.t + c["at"] + c["dur"] - 0.5:.2f});')
            d.t += n
            tc = s["titleCard"]
            d.scene(tc["dur"], f'<div class="title-card"><h1 id="tt{i}">{esc(tc["title"])}</h1><p>{esc(tc["sub"])}</p></div>')
            d.tw.append(f'tl.fromTo("#tt{i}",{{opacity:0,scale:0.96}},{{opacity:1,scale:1,duration:1.2,ease:"power2.out"}},{d.t:.2f});')
            d.t += tc["dur"]
        elif k == "chapter":
            chapter = (s["num"], s["name"])
            sid = d.scene(CHAPTER_LEN, f'<div class="ch-num" id="cn{i}">{esc(s["num"])}</div><div class="ch-name" id="cm{i}">{esc(s["name"])}</div><div class="ch-sub" id="cs{i}">{esc(s["sub"])}</div><div class="wipe" id="cw{i}" data-layout-allow-overflow></div>', fade_in=False)
            t = d.t
            d.tw += [f'tl.fromTo("#cw{i}",{{x:-1700,skewX:-18}},{{x:2100,skewX:-18,duration:1.0,ease:"power2.inOut"}},{t:.2f});',
                     f'tl.fromTo("#cn{i}",{{opacity:0,x:-40}},{{opacity:1,x:0,duration:0.5,ease:"power3.out"}},{t + 0.45:.2f});',
                     f'tl.fromTo("#cm{i}",{{opacity:0,y:20}},{{opacity:1,y:0,duration:0.45,ease:"power3.out"}},{t + 0.65:.2f});',
                     f'tl.fromTo("#cs{i}",{{opacity:0}},{{opacity:1,duration:0.4}},{t + 0.85:.2f});']
            d.t += CHAPTER_LEN
        elif k == "play":
            n = s["out"] - s["in"]
            name = f"play_{s['src']}_{s['in']}.mp4"
            cut(src[s["src"]], s["in"], n, game_vf(spec["gameCrop"]), os.path.join(CLIPS, name), True)
            d.scene(n, f'<div class="pbadge">{esc(s["badge"])}</div>')
            d.video(f"assets/clips/{name}", d.t, n, ((W - PLAY_W) // 2, 0, PLAY_W, H), True)
            for at, dur, text in s["captions"]:
                d.caption(d.t + at, dur, text, band=True)
            d.t += n
        elif k == "editor":
            n = s["out"] - s["in"]
            cx, cy, cw, ch = s["crop"]
            box = fit(cw, ch)
            name = f"ed_{s['in']}.mp4"
            cut(src[s["src"]], s["in"], n, f"crop={cw}:{ch}:{cx}:{cy},scale={box[2]}:{box[3]}", os.path.join(CLIPS, name), False)
            frame = f'<div class="media-card" style="left:{box[0]}px;top:{box[1]}px;width:{box[2]}px;height:{box[3]}px"></div>'
            d.scene(n, header(esc(s["title"]), s["sub"], chapter) + frame)
            d.video(f"assets/clips/{name}", d.t, n, box, False)
            for at, dur, text in s["captions"]:
                d.caption(d.t + at, dur, text)
            d.t += n
        elif k == "concept-layers":
            n = s["dur"]
            inner = (header('보이는 그림과 밟는 바닥은 <span class="hl">다른 데이터</span>', "같은 좌표, 다른 PNG 두 장", chapter)
                     + f'<div class="layer-card"><img src="assets/layer_bg.png" alt="" /><img id="lm{i}" src="assets/layer_mask.png" alt="" /></div>'
                     + f'<div class="layer-tags"><div class="tag" id="tb{i}"><i style="background:#c99a5b"></i><code>Stage0_Main.png</code> 보이는 그림</div>'
                     + f'<div class="tag" id="tm{i}"><i style="background:#eb4050"></i><code>Stage0_0.png</code> 밟는 바닥</div></div>')
            d.scene(n, inner)
            d.tw += [f'tl.fromTo("#lm{i}",{{opacity:0,y:-30}},{{opacity:0.78,y:0,duration:0.6,ease:"power2.out"}},{d.t + 2.1:.2f});',
                     f'tl.fromTo("#tm{i}",{{opacity:0.25}},{{opacity:1,duration:0.3}},{d.t + 2.1:.2f});',
                     f'tl.fromTo("#tb{i}",{{opacity:1}},{{opacity:0.45,duration:0.3}},{d.t + 2.1:.2f});']
            d.caption(d.t + 0.7, n - 1.0, "배경 그림과 충돌 판정은 서로 다른 PNG입니다")
            d.t += n
        elif k == "concept-parts":
            n = s["dur"]
            turrets = "".join(f'<div class="part turret" id="pt{i}_{j}">포탑 {j + 1}</div>' for j in range(7))
            inner = (header('보스는 부위의 <span class="hl">조립</span>', "BigShiee · SubmitParts", chapter)
                     + f'<div class="parts"><div class="part hull" id="ph{i}">몸체 hull</div><div class="part ant" id="pa{i}">안테나</div>'
                     + f'<div class="part cannon" id="pc{i}">주포 cannon</div><div class="turrets">{turrets}</div></div>')
            d.scene(n, inner)
            for j, pid in enumerate([f"ph{i}", f"pa{i}", f"pc{i}"] + [f"pt{i}_{x}" for x in range(7)]):
                d.tw.append(f'tl.fromTo("#{pid}",{{opacity:0,y:18}},{{opacity:1,y:0,duration:0.35,ease:"power2.out"}},{d.t + 0.4 + j * 0.15:.2f});')
            d.caption(d.t + 0.7, n - 1.0, "몸체 · 안테나 · 주포 · 포탑 7기 — 체력은 포탑에만 따로 있습니다")
            d.t += n
        elif k == "statement":
            n = s["dur"]
            lines = "".join(f"<span>{hl(l, s['hl'] if s['hl'] in l else '')}</span>" for l in s["lines"])
            d.scene(n, f'<div class="st-label" id="sl{i}">{esc(s["label"])}</div><div class="st-title" id="st{i}">{lines}</div>')
            d.tw += [f'tl.fromTo("#sl{i}",{{opacity:0,y:12}},{{opacity:1,y:0,duration:0.4}},{d.t + 0.2:.2f});',
                     f'tl.fromTo("#st{i}",{{opacity:0,y:24}},{{opacity:1,y:0,duration:0.55,ease:"power3.out"}},{d.t + 0.35:.2f});']
            d.t += n
        elif k == "end":
            n = s["dur"]
            d.scene(n, f'<div class="title-card"><h1>{esc(s["title"])}</h1><p>{esc(s["sub"])}</p></div>')
            d.t += n
    return d


def main():
    os.makedirs(CLIPS, exist_ok=True)
    spec = json.load(open(os.path.join(ROOT, "script.json"), encoding="utf-8"))
    d = build(spec)
    total = round(d.t + XF, 2)
    tpl = open(os.path.join(ROOT, "build", "template.html"), encoding="utf-8").read()
    out = (tpl.replace("{{DURATION}}", str(total))
              .replace("{{ELEMENTS}}", "\n      ".join(d.els))
              .replace("{{TWEENS}}", "\n      ".join(d.tw)))
    open(os.path.join(ROOT, "index.html"), "w", encoding="utf-8").write(out)
    m, s = divmod(total, 60)
    print(f"index.html: {len(d.els)} elements, {int(m)}:{s:05.2f}")


if __name__ == "__main__":
    sys.exit(main())
