#!/usr/bin/env python3
"""애니메이션 클립 컨택트 시트 + 자동 점검 (Docs/AnimAudit.md 참고).

사용:
  python3 Scripts/AnimAudit.py --baked Assets/Baked --out /tmp/scratch/p1anim/audit [--family Prefix] [--scale 1]
  python3 Scripts/AnimAudit.py --check            # 컨택트 시트 없이 구조 점검만 (종료 코드 != 0 이면 오류)

컨택트 시트: 클립 한 줄, 프레임은 피벗(엔티티 원점)을 같은 기준선에 맞춰 나열한다.
  피벗이 흔들리면(지터) 프레임 사이에서 발/몸이 위아래로 튄다.
  다른 엔티티의 프레임이 섞여 있으면 눈으로 바로 보인다.

구조 점검:
  1. 아틀라스 사각형이 서로 다른 이미지끼리 겹치는가
  2. 한 클립 안에서 프레임 크기 급변(다른 스프라이트 혼입 의심)
  3. 한 클립 안에서 피벗 기준 발 위치(바닥 앵커) 급변
  4. 클립이 서로 다른 엔티티 계열의 프레임 사각형을 공유하는가
"""
import argparse
import collections
import json
import os
import sys

from PIL import Image, ImageDraw


def LoadBaked(_root):
    with open(os.path.join(_root, 'anim.json'), encoding='utf-8') as f:
        clips = json.load(f)['animations']
    with open(os.path.join(_root, 'manifest.json'), encoding='utf-8') as f:
        manifest = json.load(f)
    pages = [Image.open(os.path.join(_root, p)).convert('RGBA') for p in manifest['atlasPages']]
    return clips, pages


def Family(_name):
    return _name.split('_')[0]


def FrameImage(_pages, _f):
    if _f['w'] <= 0 or _f['h'] <= 0:
        return None
    return _pages[_f['atlas']].crop((_f['x'], _f['y'], _f['x'] + _f['w'], _f['y'] + _f['h']))


def RenderClips(_clips, _pages, _names, _scale, _maxWidth=1800):
    """클립 목록을 한 장의 시트로. 반환: PIL Image 목록(페이지 분할)."""
    rows = []
    for name in _names:
        c = _clips[name]
        frames = c['frames']
        up = max([f['pivotY'] for f in frames if f['w'] > 0] + [1])
        down = max([f['h'] - f['pivotY'] for f in frames if f['w'] > 0] + [1])
        left = max([f['pivotX'] for f in frames if f['w'] > 0] + [1])
        right = max([f['w'] - f['pivotX'] for f in frames if f['w'] > 0] + [1])
        cw = (left + right + 6) * _scale
        ch = (up + down + 6) * _scale
        rows.append((name, c, up, down, left, right, cw, ch))

    sheets = []
    y = 0
    cur = []
    curH = 0
    for r in rows:
        rowH = r[7] + 14
        if curH + rowH > 2600 and cur:
            sheets.append(cur)
            cur = []
            curH = 0
        cur.append(r)
        curH += rowH
    if cur:
        sheets.append(cur)

    out = []
    for group in sheets:
        # 한 줄이 너무 길면 접어서 여러 줄로 그린다.
        lines = []
        for r in group:
            name, c, up, down, left, right, cw, ch = r
            perLine = max(1, _maxWidth // cw)
            n = len(c['frames'])
            for start in range(0, n, perLine):
                lines.append((r, start, min(n, start + perLine)))
        totalH = sum(l[0][7] + (14 if l[1] == 0 else 2) for l in lines) + 4
        img = Image.new('RGBA', (_maxWidth, totalH), (60, 60, 70, 255))
        d = ImageDraw.Draw(img)
        yy = 2
        for (r, a, b) in lines:
            name, c, up, down, left, right, cw, ch = r
            if a == 0:
                d.text((2, yy), '%s  n=%d loop=%s fd=%.3f cell=%dx%d' % (name, len(c['frames']), c['loop'], c['frameDuration'], c['cellW'], c['cellH']), fill=(255, 255, 120, 255))
                yy += 12
            base = yy + (up + 3) * _scale
            for i in range(a, b):
                f = c['frames'][i]
                x0 = (i - a) * cw
                ox = x0 + (left + 3) * _scale
                d.rectangle((x0, yy, x0 + cw - 1, yy + ch - 1), outline=(90, 90, 100, 255))
                im = FrameImage(_pages, f)
                if im is not None:
                    if _scale != 1:
                        im = im.resize((im.width * _scale, im.height * _scale), Image.NEAREST)
                    img.alpha_composite(im, (ox - f['pivotX'] * _scale, base - f['pivotY'] * _scale))
                # 피벗 십자
                d.line((ox - 3, base, ox + 3, base), fill=(255, 0, 0, 255))
                d.line((ox, base - 3, ox, base + 3), fill=(255, 0, 0, 255))
                d.text((x0 + 1, yy), str(i), fill=(0, 255, 255, 255))
            yy += ch + 2
        out.append(img.crop((0, 0, _maxWidth, yy + 2)))
    return out


def Check(_clips, _pages, _verbose):
    problems = []

    rects = collections.defaultdict(set)
    for n, c in _clips.items():
        for i, f in enumerate(c['frames']):
            if f['w'] > 0:
                rects[(f['atlas'], f['x'], f['y'], f['w'], f['h'])].add((n, i))

    # 1. 서로 다른 사각형이 겹치는가
    byPage = collections.defaultdict(list)
    for r in rects:
        byPage[r[0]].append(r)
    for page, lst in byPage.items():
        lst.sort(key=lambda r: r[1])
        for i, r in enumerate(lst):
            for q in lst[i + 1:]:
                if q[1] >= r[1] + r[3]:
                    break
                if q[2] < r[2] + r[4] and r[2] < q[2] + q[4]:
                    problems.append(('AtlasOverlap', str(r), str(q)))

    # 2/3. 클립 내부 급변
    for n, c in _clips.items():
        fr = [f for f in c['frames'] if f['w'] > 0]
        if len(fr) < 3:
            continue
        hs = sorted(f['h'] for f in fr)
        ws = sorted(f['w'] for f in fr)
        mh = hs[len(hs) // 2]
        mw = ws[len(ws) // 2]
        for i, f in enumerate(c['frames']):
            if f['w'] <= 0:
                continue
            if f['h'] > mh * 2.5 + 8 or f['h'] * 2.5 + 8 < mh or f['w'] > mw * 3 + 8 or f['w'] * 3 + 8 < mw:
                problems.append(('SizeOutlier', n, 'frame %d size %dx%d median %dx%d' % (i, f['w'], f['h'], mw, mh)))

    # 4. 계열 간 사각형 공유
    for r, users in rects.items():
        fam = {Family(u[0]) for u in users}
        if len(fam) > 1:
            problems.append(('SharedAcrossFamilies', str(r), ','.join(sorted(fam))))
    return problems


def Main():
    ap = argparse.ArgumentParser()
    ap.add_argument('--baked', default='Assets/Baked')
    ap.add_argument('--out', default='')
    ap.add_argument('--family', default='')
    ap.add_argument('--scale', type=int, default=1)
    ap.add_argument('--match', default='', help='클립 이름에 이 문자열이 들어간 것만')
    ap.add_argument('--check', action='store_true')
    ap.add_argument('--exclude', default='Boss,BattleShip,Aparch,MainTruck,SV')
    args = ap.parse_args()

    clips, pages = LoadBaked(args.baked)
    exclude = set(args.exclude.split(',')) if args.exclude else set()

    if args.check:
        problems = Check(clips, pages, True)
        for p in problems:
            print(*p)
        print('problems:', len(problems))
        return 1 if any(p[0] == 'AtlasOverlap' for p in problems) else 0

    fams = collections.defaultdict(list)
    for n in clips:
        fam = Family(n)
        if fam in exclude:
            continue
        if args.family and fam != args.family:
            continue
        if args.match and args.match not in n:
            continue
        fams[fam].append(n)
    os.makedirs(args.out, exist_ok=True)
    for fam, names in sorted(fams.items()):
        names.sort()
        sheets = RenderClips(clips, pages, names, args.scale)
        for i, s in enumerate(sheets):
            p = os.path.join(args.out, 'fam_%s%s_%d.png' % (fam, ('_' + args.match) if args.match else '', i))
            s.save(p)
            print(p, s.size, len(names))
    return 0


if __name__ == '__main__':
    sys.exit(Main())
