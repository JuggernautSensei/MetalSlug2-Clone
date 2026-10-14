"""마무리 소재: Source3 뒷부분의 별도 플레이(원본 소리 포함)와 양옆 블러 배경. 원본은 읽기만 한다."""
import subprocess
from pathlib import Path

SRC = Path("C:/Users/Ahnjiwoo/Desktop/My/포폴자료/메탈슬러그/Source3 (editor + play).mp4")
OUT = Path(__file__).parent / "assets"
GAME = "crop=960:672:480:186"
PLAY_IN, PLAY_LEN = 796.0, 62.0          # 시내 → 버스 → 지하 → 강가 → 다운타운


def ff(*args):
    subprocess.run(["ffmpeg", "-v", "error", "-y", *args], check=True)


def main():
    OUT.mkdir(exist_ok=True)
    ff("-ss", str(PLAY_IN), "-t", str(PLAY_LEN), "-i", str(SRC),
       "-vf", f"{GAME},scale=1542:1080:flags=lanczos,fps=30",
       "-c:v", "libx264", "-g", "30", "-crf", "17", "-pix_fmt", "yuv420p",
       "-c:a", "aac", "-b:a", "192k", str(OUT / "play.mp4"))
    ff("-ss", str(PLAY_IN), "-t", str(PLAY_LEN), "-i", str(SRC), "-an",
       "-vf", f"{GAME},scale=960:672,crop=960:540:0:66,gblur=sigma=18,scale=480:270,fps=30",
       "-c:v", "libx264", "-g", "30", "-crf", "28", "-pix_fmt", "yuv420p", str(OUT / "side_blur.mp4"))


if __name__ == "__main__":
    main()
