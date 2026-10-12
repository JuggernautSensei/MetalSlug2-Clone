"""Read-only access to the remake's sprite atlas (Assets/anim.json + Assets/Atlas/*.png).

Used to build template images (game px, 1 sprite px = 1 game px) for the scorer, and to dump clip strips for inspection:
  python Scripts/VideoScore/sprites.py Boss_Cannon_Shot3 Boss_Turret_A_Idle1 --out strip.png [--scale 1]
Never writes into Assets/.
"""
import argparse
import json
import os

import cv2
import numpy as np

ROOT = os.path.abspath(os.path.join(os.path.dirname(__file__), "..", ".."))
ASSETS = os.path.join(ROOT, "Assets")

_anim = None
_pages = {}


def anims():
    global _anim
    if _anim is None:
        with open(os.path.join(ASSETS, "anim.json"), encoding="utf-8") as f:
            _anim = json.load(f)["animations"]
    return _anim


def page(i):
    if i not in _pages:
        _pages[i] = cv2.imread(os.path.join(ASSETS, "Atlas", "atlas_%03d.png" % i), cv2.IMREAD_UNCHANGED)
    return _pages[i]


def frame(clip, index=0):
    """BGRA image of one frame, plus (pivotX, pivotY). Returns (None, None) when missing/empty."""
    a = anims().get(clip)
    if not a or not a.get("frames"):
        return None, None
    fr = a["frames"][max(0, min(index, len(a["frames"]) - 1))]
    if fr.get("w", 0) <= 0 or fr.get("h", 0) <= 0:
        return None, None
    img = page(fr["atlas"])[fr["y"]:fr["y"] + fr["h"], fr["x"]:fr["x"] + fr["w"]]
    if img.shape[2] == 3:
        img = cv2.cvtColor(img, cv2.COLOR_BGR2BGRA)
    return img.copy(), (fr.get("pivotX", 0), fr.get("pivotY", 0))


def frame_count(clip):
    a = anims().get(clip)
    return len(a["frames"]) if a and a.get("frames") else 0


def strip(clips, scale=1, bg=(60, 60, 60)):
    rows = []
    for c in clips:
        n = frame_count(c)
        imgs = []
        for i in range(n):
            im, _ = frame(c, i)
            if im is None:
                im = np.zeros((8, 8, 4), np.uint8)
            canvas = np.full((im.shape[0], im.shape[1], 3), bg, np.uint8)
            alpha = im[:, :, 3:4].astype(np.float32) / 255.0
            canvas = (im[:, :, :3] * alpha + canvas * (1 - alpha)).astype(np.uint8)
            cv2.putText(canvas, str(i), (2, 12), cv2.FONT_HERSHEY_SIMPLEX, 0.4, (0, 255, 255), 1)
            imgs.append(canvas)
        if not imgs:
            continue
        h = max(i.shape[0] for i in imgs)
        imgs = [cv2.copyMakeBorder(i, 0, h - i.shape[0], 0, 2, cv2.BORDER_CONSTANT, value=(0, 0, 0)) for i in imgs]
        row = np.hstack(imgs)
        label = np.zeros((16, row.shape[1], 3), np.uint8)
        cv2.putText(label, c, (2, 12), cv2.FONT_HERSHEY_SIMPLEX, 0.45, (255, 255, 255), 1)
        rows.append(np.vstack([label, row]))
    if not rows:
        return None
    w = max(r.shape[1] for r in rows)
    out = np.vstack([cv2.copyMakeBorder(r, 0, 2, 0, w - r.shape[1], cv2.BORDER_CONSTANT, value=(0, 0, 0)) for r in rows])
    if scale != 1:
        out = cv2.resize(out, None, fx=scale, fy=scale, interpolation=cv2.INTER_NEAREST)
    return out


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("clips", nargs="+")
    ap.add_argument("--out", required=True)
    ap.add_argument("--scale", type=float, default=1)
    a = ap.parse_args()
    img = strip(a.clips, a.scale)
    if img is None:
        raise SystemExit("no frames")
    cv2.imwrite(a.out, img)
    print("wrote", a.out, img.shape)


if __name__ == "__main__":
    main()
