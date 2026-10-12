#!/usr/bin/env python3
"""Extract frames from a reference gameplay video.

Usage:
  python3 Scripts/Video/extract.py --video 1 --start 100 --end 110 --fps 2 --out /tmp/f1 [--crop-game] [--scale 640x448 | --upscale] [--format png]
  python3 Scripts/Video/extract.py --video 2 --start 250 --end 252 --every-frame --out /tmp/f2 --crop-game --upscale

Output files are named t<seconds:08.3f>.png (timestamp in the video, e.g. t0100.500.png).
--crop-game  : cut the letterbox so only the game screen remains.
                 video 1 (496x360): no border -> full frame (crop 496:360:0:0)
                 video 2 (480x360): 4px black bars top/bottom -> crop 480:352:0:4
--upscale    : nearest-neighbour resize to the game's output size 640x448 (kLogicalWidth/Height in Source/Render/Renderer.h).
--scale WxH  : nearest-neighbour resize to an arbitrary size.
Times accept seconds (12.5) or m:ss (1:12.5).
"""
import argparse, os, subprocess, sys, glob

VIDEOS = {1: "Assets/Source/videoplayback (1).mp4", 2: "Assets/Source/videoplayback (2).mp4"}
FPS = {1: 30.0, 2: 29.5}
CROP = {1: (496, 360, 0, 0), 2: (480, 352, 0, 4)}  # w,h,x,y
GAME_SIZE = (640, 448)


def parse_time(s):
    if ":" in s:
        m, sec = s.split(":", 1)
        return int(m) * 60 + float(sec)
    return float(s)


def repo_root():
    return os.path.abspath(os.path.join(os.path.dirname(__file__), "..", ".."))


def video_path(v):
    return os.path.join(repo_root(), VIDEOS[v])


def build_filters(v, fps, crop, size):
    f = []
    if fps:
        f.append("fps=%s" % fps)
    if crop:
        w, h, x, y = CROP[v]
        f.append("crop=%d:%d:%d:%d" % (w, h, x, y))
    if size:
        f.append("scale=%d:%d:flags=neighbor" % size)
    return ",".join(f) if f else "null"


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--video", type=int, choices=(1, 2), required=True)
    ap.add_argument("--start", default="0")
    ap.add_argument("--end", required=True)
    g = ap.add_mutually_exclusive_group()
    g.add_argument("--fps", type=float)
    g.add_argument("--every-frame", action="store_true")
    ap.add_argument("--out", required=True)
    ap.add_argument("--crop-game", action="store_true")
    ap.add_argument("--upscale", action="store_true")
    ap.add_argument("--scale", help="WxH")
    ap.add_argument("--format", default="png")
    a = ap.parse_args()
    start, end = parse_time(a.start), parse_time(a.end)
    out_fps = None if a.every_frame else (a.fps or 2.0)
    step = 1.0 / (FPS[a.video] if a.every_frame else out_fps)
    size = None
    if a.upscale:
        size = GAME_SIZE
    if a.scale:
        size = tuple(int(x) for x in a.scale.lower().split("x"))
    os.makedirs(a.out, exist_ok=True)
    tmp = os.path.join(a.out, "_tmp_%05d." + a.format)
    cmd = ["ffmpeg", "-v", "error", "-y", "-ss", "%.3f" % start, "-t", "%.3f" % (end - start),
           "-i", video_path(a.video), "-vf", build_filters(a.video, out_fps, a.crop_game, size),
           "-vsync", "0" if a.every_frame else "vfr", tmp]
    subprocess.check_call(cmd)
    files = sorted(glob.glob(os.path.join(a.out, "_tmp_*." + a.format)))
    for i, f in enumerate(files):
        t = start + i * step
        os.rename(f, os.path.join(a.out, "t%08.3f.%s" % (t, a.format)))
    print("wrote %d frames to %s" % (len(files), a.out))


if __name__ == "__main__":
    main()
