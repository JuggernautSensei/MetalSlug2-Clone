"""게임 플레이 소재: 원테이크 플레이 영상, 양옆 블러 배경, 타이틀 정지 화면.

원본 녹화는 읽기만 한다.
"""
import subprocess
from pathlib import Path

SRC = Path("C:/Users/Ahnjiwoo/Desktop/My/포폴자료/메탈슬러그/Source2 (play).mp4")
OUT = Path(__file__).parent / "assets"
GAME = "crop=960:672:480:186"              # 게임 화면 (320x224의 3배)
PLAY_IN, PLAY_OUT = 131.0, 399.5           # 타이틀부터 MISSION COMPLETE 페이드까지
TITLE_AT = 131.0                           # 정지 화면으로 늘릴 타이틀 프레임


def ff(*args):
    subprocess.run(["ffmpeg", "-v", "error", "-y", *args], check=True)


def main():
    OUT.mkdir(exist_ok=True)
    span = f"{PLAY_OUT - PLAY_IN:.2f}"
    ff("-ss", str(PLAY_IN), "-t", span, "-i", str(SRC),
       "-vf", f"{GAME},scale=1542:1080:flags=lanczos,fps=30",
       "-c:v", "libx264", "-g", "30", "-crf", "17", "-pix_fmt", "yuv420p",
       "-c:a", "aac", "-b:a", "192k", str(OUT / "play.mp4"))
    # 양옆 여백용: 같은 화면을 크게 흐려 깔아 두는 배경 (작게 만들어 CSS로 키운다)
    ff("-ss", str(PLAY_IN), "-t", span, "-i", str(SRC), "-an",
       "-vf", f"{GAME},scale=960:672,crop=960:540:0:66,gblur=sigma=18,scale=480:270,fps=30",
       "-c:v", "libx264", "-g", "30", "-crf", "28", "-pix_fmt", "yuv420p", str(OUT / "side_blur.mp4"))
    ff("-ss", str(TITLE_AT), "-i", str(SRC), "-frames:v", "1",
       "-vf", f"{GAME},scale=1542:1080:flags=lanczos", str(OUT / "title.png"))
    ff("-ss", str(TITLE_AT), "-i", str(SRC), "-frames:v", "1",
       "-vf", f"{GAME},scale=960:672,crop=960:540:0:66,gblur=sigma=18,scale=480:270",
       str(OUT / "title_blur.png"))


if __name__ == "__main__":
    main()
