"""에디터 구간 소재: Source3 편집 클립(게임 창만 크롭), 스프라이트 시트, 충돌 레이어, 배경, BGM.

원본 녹화와 Assets는 읽기만 한다. 데스크톱 전체가 보이지 않도록 창 영역만 자른다.
"""
import subprocess
from pathlib import Path

from PIL import Image, ImageFilter

PROJECT = Path(__file__).resolve().parents[3]
SRC = Path("C:/Users/Ahnjiwoo/Desktop/My/포폴자료/메탈슬러그/Source3 (editor + play).mp4")
OUT = Path(__file__).parent / "assets"

WIN = "crop=946:660:392:178"         # 애니메이션·맵·충돌 탭: 게임 창
AI_A = "crop=1456:668:384:178"       # AI 탭 270~400초 배치: 게임 + 오른쪽 패널
AI_B = "crop=1448:668:352:178"       # AI 탭 480초 이후 배치
GAME = "crop=960:672:480:186"        # 일반 플레이 창
TOOLBAR = "crop=760:28:300:20,scale=1520:56:flags=neighbor"

# 이름: (원본 시작초, 길이, 필터)
CLIPS = {
    "toolbar": (0.6, 4.4, TOOLBAR),
    "entry": (25.0, 4.6, WIN),
    "ov_anim": (30.0, 29.6, WIN),
    "ov_map": (100.0, 29.6, WIN),
    "ov_ground": (230.0, 29.6, WIN),
    "ov_ai": (300.0, 29.6, AI_A),
    "anim_char": (30.0, 36.0, WIN),       # 캐릭터 어니언 스킨 편집
    "anim_sar1": (72.2, 11.0, WIN),       # 사루비아 (66~72초 클립 탐색은 뺀다)
    "map_sold": (100.0, 16.5, WIN),       # 군인 배치
    "map_shield": (160.5, 16.0, WIN),     # 방패병 배치
    "map_down1": (190.0, 5.0, WIN),       # 다운타운 둘러보기
    "map_drag": (194.5, 9.5, WIN),        # openland_1 섹션 선택 후 시작선(triggerX) 드래그
    "ground1": (238.0, 9.9, WIN),         # 247.4초 이후 공중에 선 긋기는 뺀다
    "result": (849.0, 7.6, GAME),
    "ai_tank": (352.0, 11.5, AI_A),
    "ai_soldier": (435.0, 10.0, AI_B),    # 445초 이후 메인 화면 전환은 뺀다
    "ai_boss": (482.0, 11.5, AI_B),
    "ai_sarubia": (709.0, 11.5, AI_B),
    "ai_mass": (749.0, 12.5, AI_B),
}
LAYER_X, LAYER_Y, LAYER_W, LAYER_H = 8150, 760, 1100, 560   # 경사가 보이는 구간
ATLASES = ["atlas_001.png", "atlas_005.png", "atlas_009.png", "atlas_013.png"]


def ff(*args):
    subprocess.run(["ffmpeg", "-v", "error", "-y", *args], check=True)


def make_clips(only=None):
    for name, (t, d, vf) in CLIPS.items():
        if only and name not in only:
            continue
        ff("-ss", str(t), "-t", str(d), "-i", str(SRC), "-an", "-vf", f"{vf},fps=30",
           "-c:v", "libx264", "-g", "30", "-crf", "18", "-pix_fmt", "yuv420p", str(OUT / f"{name}.mp4"))


def make_images():
    box = (LAYER_X, LAYER_Y, LAYER_X + LAYER_W, LAYER_Y + LAYER_H)
    Image.open(PROJECT / "Assets/Textures/Stage0_Main.png").convert("RGB").crop(box).save(OUT / "layer_bg.png")
    Image.open(PROJECT / "Assets/Ground/Stage0_0.png").convert("RGB").crop(box).save(OUT / "layer_mask.png")
    for name in ATLASES:
        im = Image.open(PROJECT / "Assets/Atlas" / name).convert("RGBA")
        bg = Image.new("RGBA", im.size, (28, 32, 42, 255))
        bg.alpha_composite(im)
        bg.convert("RGB").resize((1024, 1024), Image.LANCZOS).save(OUT / name.replace(".png", ".jpg"), quality=90)
    # 배경: 맵 에디터 화면을 크게 흐린 것
    ff("-ss", "110", "-i", str(SRC), "-frames:v", "1", "-vf", f"{WIN},scale=1920:1340", str(OUT / "bg_src.png"))
    Image.open(OUT / "bg_src.png").crop((0, 130, 1920, 1210)).filter(ImageFilter.GaussianBlur(40)).save(OUT / "bg_blur.jpg", quality=88)
    (OUT / "bg_src.png").unlink()


def make_audio():
    # 곡(3분 5초)이 구간보다 짧아 두 번 이어 붙인다
    bgm = str(PROJECT / "Assets/Audio/Metal Slug 2 Judgment.mp3")
    ff("-i", bgm, "-i", bgm, "-filter_complex", "[0:a][1:a]acrossfade=d=3", "-c:a", "pcm_s16le", str(OUT / "bgm_editor.wav"))


def main(only=None):
    OUT.mkdir(exist_ok=True)
    make_clips(only)
    if only:
        return
    make_images()
    make_audio()


if __name__ == "__main__":
    import sys
    main(sys.argv[1:] or None)
