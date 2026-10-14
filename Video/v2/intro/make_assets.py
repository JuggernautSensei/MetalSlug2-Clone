"""인트로 소재 생성: 하이라이트 컷, 타이틀 뒤 블러 클립, 휙 효과음.

원본 녹화는 읽기만 한다. 컷을 바꾸면 SHOTS를 고치고 다시 실행한다.
"""
import subprocess
from pathlib import Path

FOOTAGE = Path("C:/Users/Ahnjiwoo/Desktop/My/포폴자료/메탈슬러그")
SRCS = {"s1": FOOTAGE / "Source1 (play with bug).mp4", "s2": FOOTAGE / "Source2 (play).mp4"}
OUT = Path(__file__).parent / "assets"
GAME = "crop=960:672:480:186"            # 게임 화면 (320x224의 3배)
FULL = ("split[a][b];[a]scale=1920:1344,crop=1920:1080:0:132,gblur=sigma=24,eq=brightness=-0.12[bg];"
        "[b]scale=1542:1080:flags=lanczos[fg];[bg][fg]overlay=189:0")   # 게임 전체 + 양옆 흐린 배경
WHIP = 0.32                              # 전환 길이. 컷마다 이만큼 꼬리를 더 자른다.

# (원본, 시작초, 화면에 머무는 길이)
SHOTS = [
    ("s2", 154.8, 1.8),   # 시내 폭발
    ("s2", 159.4, 1.8),   # 잔해 거리, 헤비머신건
    ("s2", 176.0, 1.8),   # 버스 폭파
    ("s2", 189.8, 1.8),   # 지하 대형 폭발
    ("s2", 231.2, 1.8),   # 강가 헬기
    ("s2", 243.8, 1.8),   # 다운타운 폭탄 연쇄
    ("s1", 175.4, 1.8),   # 파이어샷 대 탱크
    ("s2", 272.4, 1.8),   # 사막 낙하산 병사
    ("s2", 327.2, 1.8),   # 보스 포탑
    ("s2", 383.3, 2.4),   # 보스 최후
]
BLUR_IN, BLUR_LEN, BLUR_SPEED = 385.7, 4.5, 0.75
BGM = Path(__file__).resolve().parents[3] / "Assets/Audio/Stage1_BGM.wav"


def ff(*args):
    subprocess.run(["ffmpeg", "-v", "error", "-y", *args], check=True)


def main():
    OUT.mkdir(exist_ok=True)
    for i, (src, t, d) in enumerate(SHOTS):
        tail = 0.7 if i == len(SHOTS) - 1 else WHIP
        ff("-ss", str(t), "-t", f"{d + tail:.2f}", "-i", str(SRCS[src]), "-an",
           "-filter_complex", f"[0:v]{GAME},{FULL},fps=30", "-c:v", "libx264", "-g", "30", "-crf", "16",
           "-pix_fmt", "yuv420p", str(OUT / f"shot{i}.mp4"))
    ff("-ss", str(BLUR_IN), "-t", str(BLUR_LEN), "-i", str(SRCS["s2"]), "-an",
       "-vf", f"{GAME},scale=1920:1344,crop=1920:1080:0:150,gblur=sigma=26,"
              f"setpts=PTS/{BLUR_SPEED},fps=30",
       "-c:v", "libx264", "-g", "30", "-crf", "20", "-pix_fmt", "yuv420p", str(OUT / "title_bg.mp4"))
    ff("-ss", "0.5", "-t", "32", "-i", str(BGM), "-c:a", "pcm_s16le", str(OUT / "bgm_intro.wav"))
    # 휙: 핑크 노이즈를 대역 통과시키고 짧게 부풀렸다 줄인다.
    ff("-f", "lavfi", "-i", "anoisesrc=color=pink:duration=0.42:sample_rate=48000",
       "-af", "highpass=f=500,lowpass=f=5000,afade=t=in:d=0.2:curve=exp,"
              "afade=t=out:st=0.2:d=0.22:curve=exp,volume=0.9",
       "-ac", "2", str(OUT / "whoosh.wav"))


if __name__ == "__main__":
    main()
