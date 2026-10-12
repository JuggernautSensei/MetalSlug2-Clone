#!/usr/bin/env python3
"""Rough similarity between an original frame and a game capture.

Usage:
  python3 Scripts/Video/diffscore.py A.png B.png [--size 160x112]
Both images are converted to grayscale, resized to --size (default 160x112) and compared.
Prints: MAD (mean abs diff 0-255, lower better), SSIM (global-window, 0-1 higher better, needs numpy),
and 'score' = SSIM (or 1-MAD/255 without numpy). Import: from diffscore import score.
Note: original frames from video 2 should be cropped with extract.py --crop-game; HUD/sky differences will lower scores.
"""
import argparse
from PIL import Image

try:
    import numpy as np
except ImportError:
    np = None


def load(path, size):
    return Image.open(path).convert("L").resize(size, Image.BILINEAR)


def score(path_a, path_b, size=(160, 112)):
    a, b = load(path_a, size), load(path_b, size)
    if np is None:
        pa, pb = list(a.getdata()), list(b.getdata())
        mad = sum(abs(x - y) for x, y in zip(pa, pb)) / len(pa)
        return {"mad": mad, "ssim": None, "score": 1 - mad / 255}
    x, y = np.asarray(a, float), np.asarray(b, float)
    mad = float(np.abs(x - y).mean())
    # block SSIM, 8x8 non-overlapping windows
    c1, c2 = (0.01 * 255) ** 2, (0.03 * 255) ** 2
    h, w = x.shape
    vals = []
    for j in range(0, h - 7, 8):
        for i in range(0, w - 7, 8):
            p, q = x[j:j + 8, i:i + 8], y[j:j + 8, i:i + 8]
            mp, mq = p.mean(), q.mean()
            vp, vq = p.var(), q.var()
            cv = ((p - mp) * (q - mq)).mean()
            vals.append(((2 * mp * mq + c1) * (2 * cv + c2)) / ((mp * mp + mq * mq + c1) * (vp + vq + c2)))
    s = float(np.mean(vals))
    return {"mad": mad, "ssim": s, "score": s}


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("a")
    ap.add_argument("b")
    ap.add_argument("--size", default="160x112")
    o = ap.parse_args()
    r = score(o.a, o.b, tuple(int(v) for v in o.size.split("x")))
    print("MAD=%.2f SSIM=%s score=%.4f" % (r["mad"], "n/a" if r["ssim"] is None else "%.4f" % r["ssim"], r["score"]))


if __name__ == "__main__":
    main()
