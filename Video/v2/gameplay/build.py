"""captions.json을 읽어 index.html을 만든다. 자막을 고친 뒤 다시 실행한다."""
import html
import json
from pathlib import Path

ROOT = Path(__file__).parent
CHAPTER = 2.4            # 챕터 카드 길이
FREEZE = 9.0             # 타이틀 정지 화면 길이
PLAY_IN = 131.0          # play.mp4가 시작하는 원본 시각
PLAY_LEN = 268.5
PLAY_START = CHAPTER + FREEZE
END = PLAY_START + PLAY_LEN
GAME_LEFT = (1920 - 1542) // 2


def main():
    caps = json.loads((ROOT / "captions.json").read_text(encoding="utf-8"))
    items = [(CHAPTER + t, d, s) for t, d, s in caps["title"]]
    items += [(t - PLAY_IN + PLAY_START, d, s) for t, d, s in caps["play"]]

    cap_html, cap_js = [], []
    for i, (t, d, s) in enumerate(items):
        cap_html.append(
            f'      <div id="c{i}" class="clip cap" data-start="{t:.2f}" data-duration="{d:.2f}" data-track-index="{6 + i % 2}">'
            f'<p id="c{i}p">{html.escape(s)}</p></div>')
        cap_js.append(f"[{t:.2f}, {d:.2f}]")

    page = (ROOT / "build" / "template.html").read_text(encoding="utf-8")
    page = (page.replace("{{CAPTIONS}}", "\n".join(cap_html))
                .replace("{{CAP_TIMES}}", ", ".join(cap_js))
                .replace("{{END}}", f"{END:.2f}")
                .replace("{{CHAPTER}}", f"{CHAPTER:.2f}")
                .replace("{{PLAY_START}}", f"{PLAY_START:.2f}")
                .replace("{{PLAY_LEN}}", f"{PLAY_LEN:.2f}")
                .replace("{{FREEZE_LEN}}", f"{FREEZE + 0.4:.2f}")
                .replace("{{FREEZE_START}}", f"{CHAPTER - 0.4:.2f}")
                .replace("{{GAME_LEFT}}", str(GAME_LEFT)))
    (ROOT / "index.html").write_text(page, encoding="utf-8")
    print(f"index.html: {len(items)} captions, {END:.1f}s")


if __name__ == "__main__":
    main()
