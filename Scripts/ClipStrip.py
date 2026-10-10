# Baked anim.json 클립의 프레임을 피벗 기준으로 겹쳐 그려 떨림(정합 어긋남)을 눈으로 확인하는 도구.
#
# 사용:
#   python Scripts/ClipStrip.py --baked Assets/Baked --out Captures/ClipStrip/after Bus_Opening Heli_Flying
#   python Scripts/ClipStrip.py --baked Assets/Baked --out Captures/ClipStrip/after --prefix Bus_
#
# 출력: <out>/<clip>.png  (위: 양파껍질 = 모든 프레임을 피벗 위치에 30% 알파로 겹침, 아래: 프레임 스트립)
#       표준출력: 클립별 "mask shift" = 연속 프레임(루프 닫힘 포함)의 최대 내용 어긋남(px).
#       피벗이 맞으면 몸통 픽셀이 같은 자리에 와서 값이 0 근처가 된다. 측정은 알파 마스크의 겹침 최대(FFT 상호상관)이다.
#
# 원점 표시: 빨간 십자 = 엔티티 원점(피벗). 프레임 외곽선 = 각 프레임의 그려지는 사각형.
import argparse
import json
import os
import sys

import numpy as np
from PIL import Image, ImageDraw

ORIGIN_COLOR = (255, 0, 0, 255)
GRID_BG = (236, 236, 236, 255)
SEARCH_R = 40          # 어긋남 탐색 반경 (px)
MARGIN = 12            # 캔버스 여백 (px)


def load_manifest(baked):
    with open(os.path.join(baked, 'manifest.json'), encoding='utf-8') as f:
        manifest = json.load(f)
    with open(os.path.join(baked, manifest['animations']), encoding='utf-8') as f:
        anims = json.load(f)['animations']
    pages = {}
    return manifest, anims, pages


def page_image(baked, pages, rel, cache):
    if rel not in cache:
        cache[rel] = Image.open(os.path.join(baked, rel)).convert('RGBA')
    return cache[rel]


def frame_pieces(baked, anim, pages, cache, atlas_pages):
    """Returns list of (w,h,pivotX,pivotY, RGBA numpy array or None) per frame. Empty frames have w=h=0."""
    out = []
    for fr in anim['frames']:
        w, h = fr['w'], fr['h']
        if w <= 0 or h <= 0:
            out.append((0, 0, 0, 0, None))
            continue
        page = page_image(baked, pages, atlas_pages[fr['atlas']], cache)
        crop = page.crop((fr['x'], fr['y'], fr['x'] + w, fr['y'] + h))
        out.append((w, h, fr['pivotX'], fr['pivotY'], np.asarray(crop, dtype=np.uint8)))
    return out


def mask_shift(pieces, loop):
    """Max mask-overlap misregistration between consecutive non-empty frames (px). FFT cross-correlation."""
    items = [p for p in pieces if p[4] is not None]
    if len(items) < 2:
        return 0.0
    if loop:
        items = items + [items[0]]
    x0 = min(-p[2] for p in items) - SEARCH_R
    y0 = min(-p[3] for p in items) - SEARCH_R
    x1 = max(-p[2] + p[0] for p in items) + SEARCH_R
    y1 = max(-p[3] + p[1] for p in items) + SEARCH_R
    H, W = y1 - y0, x1 - x0
    worst = 0.0
    for a, b in zip(items, items[1:]):
        ma = np.zeros((H, W), dtype=np.float64)
        mb = np.zeros((H, W), dtype=np.float64)
        ax, ay = -a[2] - x0, -a[3] - y0
        bx, by = -b[2] - x0, -b[3] - y0
        ma[ay:ay + a[1], ax:ax + a[0]] = a[4][:, :, 3] / 255.0
        mb[by:by + b[1], bx:bx + b[0]] = b[4][:, :, 3] / 255.0
        corr = np.fft.irfft2(np.fft.rfft2(ma, s=(H, W)) * np.conj(np.fft.rfft2(mb, s=(H, W))), s=(H, W))
        # corr[k] = sum_y A(y) B(y-k): B placed at shift k relative to A. Wrap-around is excluded by the padding.
        base_dx, base_dy = bx - ax, by - ay
        best, bdx, bdy = -1.0, 0, 0
        for dy in range(-SEARCH_R, SEARCH_R + 1):
            for dx in range(-SEARCH_R, SEARCH_R + 1):
                v = corr[(base_dy + dy) % H, (base_dx + dx) % W]
                if v > best:
                    best, bdx, bdy = v, dx, dy
        worst = max(worst, float(max(abs(bdx), abs(bdy))))
    return worst


def composite(canvas, piece, ox, oy, alpha_scale):
    w, h, px, py, arr = piece
    if arr is None:
        return
    dx = ox - px
    dy = oy - py
    layer = arr.copy()
    layer[:, :, 3] = (layer[:, :, 3].astype(np.float32) * alpha_scale).astype(np.uint8)
    region = canvas[dy:dy + h, dx:dx + w]
    src = layer.astype(np.float32)
    a = src[:, :, 3:4] / 255.0
    dst = region.astype(np.float32)
    dst[:, :, :3] = dst[:, :, :3] * (1 - a) + src[:, :, :3] * a
    dst[:, :, 3:4] = np.maximum(dst[:, :, 3:4], src[:, :, 3:4])
    canvas[dy:dy + h, dx:dx + w] = dst.astype(np.uint8)


def draw_origin(img, ox, oy, size):
    d = ImageDraw.Draw(img)
    d.line([(ox - size, oy), (ox + size, oy)], fill=ORIGIN_COLOR, width=1)
    d.line([(ox, oy - size), (ox, oy + size)], fill=ORIGIN_COLOR, width=1)


def render(name, pieces, loop, out_path, shift):
    live = [p for p in pieces if p[4] is not None]
    if not live:
        return
    minx = min(-p[2] for p in live)
    miny = min(-p[3] for p in live)
    maxx = max(-p[2] + p[0] for p in live)
    maxy = max(-p[3] + p[1] for p in live)
    cw = maxx - minx + 2 * MARGIN
    ch = maxy - miny + 2 * MARGIN
    ox = -minx + MARGIN
    oy = -miny + MARGIN

    # 1) 양파껍질
    onion = np.full((ch, cw, 4), 255, dtype=np.uint8)
    onion[:, :, 3] = 255
    for p in live:
        composite(onion, p, ox, oy, 0.3)
    onion_img = Image.fromarray(onion, 'RGBA')
    d = ImageDraw.Draw(onion_img)
    palette = [(0, 90, 200), (0, 160, 60), (200, 120, 0), (150, 0, 180), (0, 150, 150), (120, 60, 0)]
    for i, p in enumerate(pieces):
        if p[4] is None:
            continue
        x, y = ox - p[2], oy - p[3]
        d.rectangle([x, y, x + p[0] - 1, y + p[1] - 1], outline=palette[i % len(palette)] + (255,))
    draw_origin(onion_img, ox, oy, 10)

    # 2) 스트립 (프레임별 타일, 피벗 맞춤)
    tile_w, tile_h = cw, ch
    cols = max(1, min(len(pieces), 2000 // max(1, tile_w)))
    rows = (len(pieces) + cols - 1) // cols
    strip = Image.new('RGBA', (cols * tile_w, rows * (tile_h + 16)), (255, 255, 255, 255))
    sd = ImageDraw.Draw(strip)
    for i, p in enumerate(pieces):
        tx = (i % cols) * tile_w
        ty = (i // cols) * (tile_h + 16)
        tile = np.full((tile_h, tile_w, 4), 0, dtype=np.uint8)
        tile[:, :, :3] = GRID_BG[:3]
        tile[:, :, 3] = 255
        if p[4] is not None:
            composite(tile, p, ox, oy, 1.0)
        tile_img = Image.fromarray(tile, 'RGBA')
        dt = ImageDraw.Draw(tile_img)
        draw_origin(tile_img, ox, oy, 10)
        strip.paste(tile_img, (tx, ty + 16))
        sd.text((tx + 4, ty + 2), f'{i}', fill=(0, 0, 0, 255))

    # 3) 합치기 (세로)
    W = max(onion_img.width, strip.width)
    H = onion_img.height + 24 + strip.height
    board = Image.new('RGBA', (W, H), (255, 255, 255, 255))
    board.paste(onion_img, (0, 24))
    board.paste(strip, (0, onion_img.height + 24))
    bd = ImageDraw.Draw(board)
    bd.text((4, 4), f'{name}  frames={len(pieces)} loop={loop}  mask shift={shift:.1f}px', fill=(0, 0, 0, 255))
    # 작은 클립은 정수 배율로 키워 본다 (최근접, 픽셀 손실 없음). 너무 크면 2400px 이내로 줄인다.
    zoom = 3 if max(cw, ch) <= 160 else (2 if max(cw, ch) <= 320 else 1)
    if zoom > 1:
        board = board.resize((board.width * zoom, board.height * zoom), Image.NEAREST)
    if max(board.size) > 2400:
        s = 2400 / max(board.size)
        board = board.resize((int(board.width * s), int(board.height * s)), Image.NEAREST)
    board.convert('RGB').save(out_path)


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument('--baked', default=os.path.join(os.path.dirname(__file__), '..', 'Assets', 'Baked'))
    ap.add_argument('--out', required=True)
    ap.add_argument('--prefix', action='append', default=[])
    ap.add_argument('clips', nargs='*')
    args = ap.parse_args()

    baked = os.path.abspath(args.baked)
    manifest, anims, pages = load_manifest(baked)
    atlas_pages = manifest.get('atlasPages', [])
    names = list(args.clips)
    for pref in args.prefix:
        names += [n for n in anims if n.startswith(pref) and n not in names]
    os.makedirs(args.out, exist_ok=True)
    cache = {}
    report = {}
    for name in names:
        if name not in anims:
            print(f'missing clip: {name}', file=sys.stderr)
            continue
        anim = anims[name]
        pieces = frame_pieces(baked, anim, pages, cache, atlas_pages)
        loop = bool(anim.get('loop', False))
        shift = mask_shift(pieces, loop)
        report[name] = shift
        render(name, pieces, loop, os.path.join(args.out, name + '.png'), shift)
        print(f'{name}\tframes={len(pieces)}\tmask_shift={shift:.1f}')
    with open(os.path.join(args.out, 'shifts.json'), 'w', encoding='utf-8') as f:
        json.dump(report, f, indent=1)


if __name__ == '__main__':
    main()
