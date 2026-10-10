#!/usr/bin/env python3
"""Big Shiee 보스 시트(40204.png) 클립 사각형 생성기.

사용: python3 Scripts/GenBossSheet.py            -> Assets/Source/bossSheet.json 을 쓴다 (sheets.json 의 40204 항목)
      python3 Scripts/GenBossSheet.py --merge    -> Assets/Source/sheets.json 의 40204 항목을 갱신/추가한다

시트 배경색 (153,217,234), (0,255,0) 을 키로 보고, 행마다 지정한 y 창 안의 전경 덩어리(bbox)를 x 순으로 모아
`frames` 사각형을 만든다. 라벨 글자는 y 창에서 빠져 있다. 개수가 기대와 다르면 즉시 실패한다.
좌표는 시트(1x) 픽셀이다 (Baker 가 scale 2 로 키운다). Docs/Spec_BigShiee.md 7절 참고.
"""
import argparse
import json
import os
import sys

import numpy as np
from PIL import Image
import cv2

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
SHEET = os.path.join(ROOT, "Assets", "Source", "40204.png")

# Baker 의 프레임 겹침 정렬(register)을 끄는 접두어: 자라거나 흩어지는 효과, 탄, 전개 연출은 원래 위치(원본 사각형 기준)를 지킨다
NOREG = ["Boss_Dot", "Boss_Missile", "Boss_Shell", "Boss_Casing", "Boss_Smoke", "Boss_Disc", "Boss_Blast", "Boss_Cannon_Deploy", "Boss_Wreck"]

KEY_A = (153, 217, 234)
KEY_B = (0, 255, 0)


def load_fg():
    a = np.array(Image.open(SHEET).convert("RGBA"))
    key = ((a[..., :3] == KEY_A).all(-1)) | ((a[..., :3] == KEY_B).all(-1)) | (a[..., 3] == 0)
    return (~key).astype(np.uint8)


FG = load_fg()


def comps(x0, y0, x1, y1, gap=1, minpix=6):
    sub = FG[y0:y1, x0:x1]
    k = cv2.getStructuringElement(cv2.MORPH_RECT, (2 * gap + 1, 2 * gap + 1))
    d = cv2.dilate(sub, k)
    n, lab, _, _ = cv2.connectedComponentsWithStats(d, connectivity=8)
    out = []
    for i in range(1, n):
        m = (lab == i) & (sub > 0)
        ys, xs = np.where(m)
        if len(xs) < minpix:
            continue
        out.append([x0 + int(xs.min()), y0 + int(ys.min()), int(xs.max() - xs.min() + 1), int(ys.max() - ys.min() + 1)])
    out.sort()
    return out


def row(name, window, count, gap=1, minpix=6, skip=0, take=None, **kw):
    x0, y0, x1, y1 = window
    fr = comps(x0, y0, x1, y1, gap, minpix)
    if skip:
        fr = fr[skip:]
    if take is not None:
        fr = fr[:take]
    if count is not None and len(fr) != count:
        sys.exit("%s: %d frames, expected %d" % (name, len(fr), count))
    clip = {"name": name, "frames": fr}
    clip.update(kw)
    return clip


def cells(name, x, y0, w, h, rows, **kw):
    fr = [[x, y0 + r * 213, w, h] for r in rows]
    clip = {"name": name, "frames": fr}
    clip.update(kw)
    return clip


def build():
    clips = []

    # --- 선체 (셀 사각형, pivot center: 셀 중심이 원점) ---
    clips.append(cells("Boss_Hull_Idle", 5, 27, 271, 208, [0, 1], pivot="center", fps=6, loop=True))
    clips.append(cells("Boss_Hull_Drive", 286, 27, 271, 208, range(8), pivot="center", fps=15, loop=True))
    clips.append(cells("Boss_Hull_D1", 848, 27, 271, 208, [0, 1], pivot="center", fps=6, loop=True))
    clips.append(cells("Boss_Hull_D3", 1413, 27, 278, 208, [0, 1], pivot="center", fps=6, loop=True))
    clips.append(cells("Boss_Hull_D6", 2289, 27, 290, 208, [0, 1], pivot="center", fps=6, loop=True))
    clips.append(cells("Boss_Hull_Open", 2893, 27, 298, 208, [0, 1], pivot="center", fps=6, loop=True))
    clips.append(cells("Boss_Hull_D9", 3817, 27, 300, 208, [0, 1], pivot="center", fps=6, loop=True))

    # --- 포탑 A..G (foot pivot = bbox 아래 중앙) ---
    turret_rows = {
        "A": dict(x=(0, 1030), idle=[(1753, 1790), (1794, 1830), (1834, 1871)], n_idle=22, turn=(1888, 1923), shot=[(1940, 1982), (1986, 2025), (2029, 2071)], n_shot=14, dead=(2088, 2130)),
        "B": dict(x=(0, 1030), idle=[(2156, 2190), (2194, 2230), (2234, 2268)], n_idle=22, turn=(2285, 2320), shot=[(2337, 2372), (2376, 2415), (2419, 2461)], n_shot=14, dead=(2478, 2521)),
        "C": dict(x=(0, 1030), idle=[(2547, 2584), (2588, 2624), (2628, 2665)], n_idle=22, turn=(2682, 2717), shot=[(2734, 2776), (2780, 2819), (2823, 2865)], n_shot=14, dead=(2882, 2912)),
        "D": dict(x=(0, 1030), idle=[(2938, 2973), (2977, 3013), (3017, 3054)], n_idle=22, turn=(3074, 3109), shot=[(3126, 3163), (3167, 3206), (3210, 3252)], n_shot=14, dead=(3269, 3308)),
        "E": dict(x=(1030, 1580), idle=[(1753, 1779), (1783, 1814), (1818, 1844)], n_idle=12, turn=(1861, 1888), shot=[(1905, 1937), (1941, 1971), (1975, 2007)], n_shot=11, dead=(2024, 2062)),
        "F": dict(x=(1030, 1580), idle=[(2088, 2118), (2122, 2155), (2160, 2190)], n_idle=12, turn=(2207, 2238), shot=[(2255, 2291), (2295, 2327), (2331, 2367)], n_shot=11, dead=(2384, 2410)),
        "G": dict(x=(1030, 1580), idle=[(2436, 2464), (2468, 2501), (2505, 2533)], n_idle=12, turn=(2550, 2579), shot=[(2596, 2630), (2634, 2666), (2670, 2704)], n_shot=11, dead=(2721, 2749)),
    }
    for letter, t in turret_rows.items():
        xa, xb = t["x"]
        for i, (ya, yb) in enumerate(t["idle"]):
            clips.append(row("Boss_Turret_%s_Idle%d" % (letter, i + 1), (xa, ya, xb, yb), t["n_idle"], gap=1, minpix=25, pivot="foot", fps=10, loop=True))
        clips.append(row("Boss_Turret_%s_Turn" % letter, (xa, t["turn"][0], xb, t["turn"][1]), 9, gap=1, minpix=25, pivot="foot", fps=12, loop=False))
        for i, (ya, yb) in enumerate(t["shot"]):
            clips.append(row("Boss_Turret_%s_Shot%d" % (letter, i + 1), (xa, ya, xb, yb), t["n_shot"], gap=1, minpix=25, pivot="foot", fps=14, loop=False))
        clips.append(row("Boss_Turret_%s_Dead" % letter, (xa, t["dead"][0], xb, t["dead"][1]), 1, gap=14, pivot="foot", fps=1, loop=False))

    # --- 캐논: 전개 14프레임 (밑변 y=1892 정렬, 왼쪽 받침 기준 -> 코드에서 좌하단 앵커) ---
    lefts = [2428, 2448, 2485, 2542, 2609, 2701, 2789, 2879, 2989, 3100, 3211, 3320, 3428, 3538, 3660]
    fr = []
    for i in range(14):
        sub = FG[1750:1893, lefts[i]:lefts[i + 1]]
        ys, xs = np.where(sub > 0)
        fr.append([lefts[i] + int(xs.min()), 1750 + int(ys.min()), int(xs.max() - xs.min() + 1), int(ys.max() - ys.min() + 1)])
    clips.append({"name": "Boss_Cannon_Deploy", "frames": fr, "pivot": "foot", "fps": 12, "loop": False})
    clips.append(row("Boss_Cannon_Aim", (2420, 1905, 3640, 2052), 9, gap=1, pivot="foot", fps=12, loop=False))
    shot_windows = [(2064, 2224), (2225, 2385), (2386, 2546), (2547, 2707), (2708, 2868)]
    for i, (ya, yb) in enumerate(shot_windows):
        clips.append(row("Boss_Cannon_Shot%d" % (i + 1), (2420, ya, 3640, yb), 8, gap=1, pivot="foot", fps=14, loop=False))

    # --- 탄/미사일/탄피/연기 ---
    for i, nm in enumerate(["Orange", "Red", "Cyan"]):
        y0 = 2659 + i * 12
        clips.append(row("Boss_Dot_%s" % nm, (1588, y0 - 1, 1640, y0 + 11), 5, gap=0, minpix=2, pivot="center", fps=15, loop=False))
    for i, (ya, yb) in enumerate([(1797, 1869), (1875, 1950), (1958, 2034)]):
        clips.append(row("Boss_Missile%d" % (i + 1), (3640, ya, 4624, yb), 12, gap=2, minpix=30, pivot="foot", fps=18, loop=True))
    for i, (ya, yb) in enumerate([(2955, 2981), (2985, 3014), (3018, 3048)]):
        clips.append(row("Boss_Shell%d" % (i + 1), (2400, ya, 2530, yb), 3, gap=1, pivot="center", fps=12, loop=True))
    clips.append(row("Boss_Casing", (2400, 3065, 2700, 3090), 6, gap=1, pivot="center", fps=18, loop=True))
    clips.append(row("Boss_Smoke_Big", (2400, 3107, 3700, 3156), 23, gap=2, skip=2, take=23, pivot="center", fps=20, loop=False))
    clips.append(row("Boss_Smoke_Small", (2400, 3199, 3700, 3252), 25, gap=2, skip=2, take=25, pivot="center", fps=24, loop=False))
    clips.append(row("Boss_Disc_Black", (2400, 3107, 2480, 3156), 1, gap=2, take=1, pivot="center", fps=1, loop=False))
    clips.append(row("Boss_Disc_White", (2480, 3107, 2540, 3156), 1, gap=2, take=1, pivot="center", fps=1, loop=False))

    # --- 포구/폭발 효과 줄 (ABCD x3, EFG x3) ---
    for i, (ya, yb) in enumerate([(2254, 2312), (2330, 2376), (2384, 2442)]):
        clips.append(row("Boss_Blast_ABCD%d" % (i + 1), (1588, ya, 2400, yb), 12, gap=1, minpix=60, pivot="foot", fps=20, loop=False))
    for i, (ya, yb) in enumerate([(2463, 2517), (2525, 2581), (2589, 2643)]):
        clips.append(row("Boss_Blast_EFG%d" % (i + 1), (1588, ya, 2200, yb), 9, gap=1, minpix=60, pivot="foot", fps=20, loop=False))

    # --- 잔해/파괴 ---
    clips.append({"name": "Boss_Wreck", "frames": [[3658, 2051, 299, 191]], "pivot": "foot", "fps": 1, "loop": False})
    return clips


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--merge", action="store_true")
    args = ap.parse_args()
    entry = {
        "file": "40204.png",
        "scale": 2,
        "noRegisterPrefixes": NOREG,
        "key": {"mode": "color", "colors": [list(KEY_A), list(KEY_B)], "tol": 0},
        "clips": build(),
    }
    if args.merge:
        path = os.path.join(ROOT, "Assets", "Source", "sheets.json")
        with open(path, encoding="utf-8") as f:
            text = f.read()
        if '"file": "40204.png"' in text:
            sys.exit("sheets.json 에 이미 40204.png 항목이 있다 (직접 지우고 다시 실행)")
        lines = []
        for c in entry["clips"]:
            frames = json.dumps(c["frames"], separators=(",", ":")).replace("],[", "], [")
            parts = ['"name": "%s"' % c["name"], '"frames": %s' % frames, '"pivot": "%s"' % c["pivot"], '"fps": %d' % c["fps"], '"loop": %s' % ("true" if c["loop"] else "false")]
            lines.append("                { " + ", ".join(parts) + " }")
        block = (
            "        {\n"
            '            "file": "40204.png",\n'
            '            "scale": 2,\n'
            '            "noRegisterPrefixes": ' + json.dumps(NOREG) + ',\n'
            '            "key": { "mode": "color", "colors": [[153, 217, 234], [0, 255, 0]], "tol": 0 },\n'
            '            "clips": [\n' + ",\n".join(lines) + "\n"
            "            ]\n"
            "        }"
        )
        tail = "\n    ]\n}"
        idx = text.rstrip().rfind(tail)
        if idx < 0:
            sys.exit("sheets.json 끝 형식을 찾지 못했다")
        text = text[:idx] + ",\n" + block + text[idx:]
        with open(path, "w", encoding="utf-8", newline="\n") as f:
            f.write(text)
        print("merged", len(entry["clips"]), "clips into", path)
    else:
        path = os.path.join(ROOT, "Assets", "Source", "bossSheet.json")
        with open(path, "w", encoding="utf-8", newline="\n") as f:
            json.dump(entry, f, indent=1)
        print("wrote", path, len(entry["clips"]), "clips")


if __name__ == "__main__":
    main()
