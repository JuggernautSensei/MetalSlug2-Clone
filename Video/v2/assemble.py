"""구간별 최신 렌더를 이어 전체본을 만든다: 구간마다 음량을 맞추고 0.5초 크로스페이드로 잇는다."""
import subprocess
import time
from pathlib import Path

ROOT = Path(__file__).parent
SECTIONS = ["intro", "gameplay", "editor", "outro"]
XFADE = 0.5
LOUDNESS = "loudnorm=I=-16:TP=-1.5:LRA=11"


def latest(section):
    return max((ROOT / section / "renders").glob("*.mp4"), key=lambda p: p.stat().st_mtime)


def duration(path):
    out = subprocess.run(["ffprobe", "-v", "error", "-show_entries", "format=duration", "-of", "csv=p=0", str(path)],
                         capture_output=True, text=True, check=True).stdout
    return float(out)


def main():
    files = [latest(s) for s in SECTIONS]
    durs = [duration(f) for f in files]
    args = ["ffmpeg", "-v", "error", "-y"]
    for f in files:
        args += ["-i", str(f)]

    chains = [f"[{i}:a]{LOUDNESS},aresample=48000[a{i}]" for i in range(len(files))]
    v_prev, a_prev, offset = "[0:v]", "[a0]", durs[0]
    for i in range(1, len(files)):
        offset -= XFADE
        v_out, a_out = f"[v{i}]", f"[ax{i}]"
        chains.append(f"{v_prev}[{i}:v]xfade=transition=fade:duration={XFADE}:offset={offset:.3f}{v_out}")
        chains.append(f"{a_prev}[a{i}]acrossfade=d={XFADE}{a_out}")
        v_prev, a_prev = v_out, a_out
        offset += durs[i]

    out = ROOT / "renders" / f"full_{time.strftime('%Y-%m-%d_%H-%M-%S')}.mp4"
    out.parent.mkdir(exist_ok=True)
    args += ["-filter_complex", ";".join(chains), "-map", v_prev, "-map", a_prev,
             "-c:v", "libx264", "-crf", "18", "-preset", "medium", "-pix_fmt", "yuv420p",
             "-c:a", "aac", "-b:a", "192k", "-movflags", "+faststart", str(out)]
    for f, d in zip(files, durs):
        print(f"{f.relative_to(ROOT)}  {d:.1f}s")
    subprocess.run(args, check=True)
    print(out)


if __name__ == "__main__":
    main()
