"""에디터 구간 index.html 생성. 장면과 자막은 아래 표에서 고치고 다시 실행한다."""
import html
from pathlib import Path

ROOT = Path(__file__).parent
END = 224.3
BOX = (80, 130, 1760, 750)            # 미디어 카드가 들어갈 영역 (x, y, w, h)
SIZE = {"win": (946, 660), "ai": (1456, 668), "game": (960, 672)}

# 섹션: (id, 제목, 시작, 끝). 섹션이 바뀔 때 휙 전환한다.
SECTIONS = [
    ("s_entry", "디버그 / 에디터", 3.0, 12.0),
    ("s_over", "디버그 / 에디터", 12.0, 41.0),
    ("s_anim", "애니메이션 에디터", 41.0, 94.0),
    ("s_map", "맵 에디터", 94.0, 139.0),
    ("s_col", "픽셀 충돌체 에디터", 139.0, 169.7),
    ("s_play", "게임 플레이 에디터", 169.7, END),
]
ATLAS_END = 47.5      # 스프라이트 시트 이미지가 사라지는 시각
LAYERS_END = 153.3    # 충돌 레이어 비교 이미지가 사라지는 시각
MASK_AT = 144.1       # 충돌 마스크가 옆에 나타나는 시각
DRAG = (130.0, 139.0)  # 섹션 시작선 드래그 장면: triggerX 칸을 민트 테두리로 표시

# 영상 카드: (섹션, 파일, 크기 종류, 시작, 길이). 같은 자리에서 앞 카드가 사라지고 다음 카드가 페이드로 들어온다.
VIDEOS = [
    ("s_entry", "entry", "win", 7.4, 4.6),
    ("s_anim", "anim_char", "win", 47.5, 36.0),
    ("s_anim", "anim_sar1", "win", 83.5, 10.5),
    ("s_map", "map_sold", "win", 94.0, 16.0),
    ("s_map", "map_shield", "win", 110.0, 15.5),
    ("s_map", "map_down1", "win", 125.5, 4.5),
    ("s_map", "map_drag", "win", 130.0, 9.0),
    ("s_col", "ground1", "win", 153.3, 9.4),
    ("s_col", "result", "game", 162.7, 7.0),
    ("s_play", "ai_tank", "ai", 169.7, 11.0),
    ("s_play", "ai_soldier", "ai", 180.7, 9.6),
    ("s_play", "ai_boss", "ai", 190.3, 11.0),
    ("s_play", "ai_sarubia", "ai", 201.3, 11.0),
    ("s_play", "ai_mass", "ai", 212.3, 12.0),
]

OVERVIEW = [  # 2x2 카드: (파일, 크기 종류, 제목, 강조 시작)
    ("ov_anim", "win", "애니메이션 에디터", 16.3),
    ("ov_map", "win", "맵 에디터", 20.0),
    ("ov_ground", "win", "픽셀 충돌체 에디터", 23.7),
    ("ov_ai", "ai", "게임 플레이 에디터", 27.4),
]

CAPTIONS = [
    (3.3, 4.0, "실행 인자로 --editor를 넣으면 디버그·에디터 모드로 실행됩니다."),
    (7.6, 4.2, "에디터는 Dear ImGui로 개발했습니다."),
    (12.4, 3.8, "에디터는 네 가지로 나뉩니다."),
    (16.3, 3.6, "애니메이션 에디터: 스프라이트 위치를 조절해 애니메이션을 다듬습니다."),
    (20.0, 3.6, "맵 에디터: 오브젝트를 직접 배치해 기획대로 스테이지를 구성합니다."),
    (23.7, 3.6, "픽셀 충돌체 에디터: 충돌 영역을 직접 그려 원하는 충돌을 만듭니다."),
    (27.4, 3.6, "게임 플레이 에디터: 조작과 적 AI를 보며 데이터를 바로 조절합니다."),
    (31.3, 4.8, "게임의 상당 부분을 데이터 중심으로 만들어, 수치와 배치를 JSON으로 저장하고 다시 읽습니다."),
    (36.2, 4.6, "애니메이션과 튜닝 값은 바로 반영되고, 맵과 충돌 데이터는 빌드 없이 다음 실행에 반영됩니다."),
    (41.4, 4.0, "메탈슬러그는 스프라이트가 방대해 리소스 작업량이 어마어마합니다."),
    (48.0, 4.2, "미리 정리한 스프라이트 시트를 불러와 편집합니다."),
    (54.0, 4.6, "어니언 스킨으로 직전 프레임을 반투명하게 겹쳐 보며 맞춥니다."),
    (62.5, 5.0, "픽셀 게임은 1픽셀 차이도 손맛을 바꿔, 프레임마다 위치를 세밀하게 조절합니다."),
    (73.0, 4.2, "애니메이션 속도도 프레임 단위로 조절합니다."),
    (84.0, 5.0, "캐릭터뿐만 아니라 적 유닛도 같은 방식으로 프레임마다 맞춥니다."),
    (94.4, 5.0, "한 장면에도 오브젝트가 엄청나게 많아, 직접 배치하고 섹션을 나누는 도구가 필요했습니다."),
    (100.8, 4.2, "적을 골라 원하는 위치에 바로 배치합니다."),
    (110.8, 5.0, "우르르 나오는 적, 특정 위치의 탱크와 헬기, 펑펑 터지는 나무 구조물까지"),
    (116.4, 3.6, "모두 이 에디터에서 편집합니다."),
    (125.7, 4.2, "스테이지 전체를 둘러보며 섹션 구간을 확인하고,"),
    (130.4, 4.6, "섹션을 선택하고 노란 시작선을 드래그해, 적이 나오는 위치를 옮깁니다."),
    (135.2, 3.6, "플레이어가 이 선을 넘으면 그 섹션의 적이 나타납니다."),
    (139.4, 4.6, "화면에 보이지 않는 충돌 이미지가 사선과 곡선 충돌을 만듭니다."),
    (144.1, 4.4, "발밑 픽셀을 읽어, 흰색은 빈 공간, 파란색은 바닥으로 처리합니다."),
    (148.6, 4.6, "OBB 충돌체도 있지만, 복잡도를 생각하면 픽셀 방식이 더 단순합니다."),
    (153.6, 4.8, "보이는 맵 위에 직접 충돌 영역을 그려, 둘을 정확히 맞춥니다."),
    (158.5, 4.0, "사각형, 선, 채우기 도구로 칠하고 저장합니다."),
    (163.0, 5.0, "칠한 영역이 그대로 캐릭터가 밟는 바닥이 됩니다."),
    (170.1, 5.0, "히트박스, 투사체 발사 위치, 몬스터 패턴 같은 값을 모두 데이터로 분리했습니다."),
    (175.7, 4.4, "에디터에서 조절한 값은 다시 빌드하지 않고 바로 확인합니다."),
    (181.1, 5.0, "병사부터 탱크, 보스까지 유닛마다 같은 화면에서 편집하고 확인합니다."),
    (190.7, 5.4, "메탈슬러그는 오브젝트가 부위별로 잘게 나뉘어 있어, 세세한 편집이 꼭 필요했습니다."),
    (201.7, 5.0, "투사체 수, 폭발 크기와 위치, 간격 같은 값도 바로 조절합니다."),
    (212.7, 5.0, "몬스터의 충돌체를 추가하고, 적을 배치하며 AI 동작을 확인합니다."),
]


def fit(kind):
    w, h = SIZE[kind]
    bx, by, bw, bh = BOX
    s = min(bw / w, bh / h)
    W, H = round(w * s), round(h * s)
    return bx + (bw - W) // 2, by + (bh - H) // 2, W, H


def main():
    parts, js = [], []
    for sid, title, s, e in SECTIONS:
        inner = [f'<div class="scene-title" id="{sid}_t">{html.escape(title)}</div>']
        for vsec, name, kind, t, d in VIDEOS:
            if vsec != sid:
                continue
            x, y, w, h = fit(kind)
            if abs(t + d - e) < 0.05 and e < END:
                d += 0.5   # 휙 전환으로 밀려 나가는 동안에도 재생되게
            inner.append(f'<div class="card" id="{name}_c" style="left:{x}px;top:{y}px;width:{w}px;height:{h}px">'
                         f'<video id="{name}" class="clip" src="assets/{name}.mp4" muted playsinline data-start="{t}" data-duration="{d}" data-track-index="2"></video></div>')
            js.append(f'cardIn("#{name}_c", {t}, {min(t + d, e):.2f});')
        if sid == "s_entry":
            inner.append('<div class="toolbar" id="tb_c"><video id="toolbar" class="clip" src="assets/toolbar.mp4" muted playsinline data-start="3.0" data-duration="4.4" data-track-index="2"></video></div>')
            js.append('cardIn("#tb_c", 3.0, 7.4);')
        if sid == "s_over":
            cells = []
            for i, (name, kind, label, hl) in enumerate(OVERVIEW):
                cells.append(f'<div class="cell" id="cell{i}"><video id="{name}" class="clip" src="assets/{name}.mp4" muted playsinline data-start="12.0" data-duration="29.5" data-track-index="2"></video>'
                             f'<div class="label"><b>0{i + 1}</b>{html.escape(label)}</div></div>')
                js.append(f'tl.fromTo("#cell{i}", {{opacity: 0, y: 30}}, {{opacity: 1, y: 0, duration: 0.5, ease: "power3.out"}}, {12.2 + i * 0.15:.2f});')
                others = ", ".join(f'"#cell{j}"' for j in range(4) if j != i)
                js.append(f'tl.to([{others}], {{opacity: 0.35, duration: 0.3}}, {hl}); tl.to("#cell{i}", {{opacity: 1, duration: 0.3}}, {hl});')
                js.append(f'tl.to("#cell{i}", {{boxShadow: "0 0 0 4px #3ddbb4, 0 24px 60px rgba(0,0,0,0.5)", duration: 0.3}}, {hl}); tl.to("#cell{i}", {{boxShadow: "0 0 0 0px #3ddbb4, 0 24px 60px rgba(0,0,0,0.5)", duration: 0.3}}, {hl + 3.4:.1f});')
            js.append('tl.to(["#cell0", "#cell1", "#cell2", "#cell3"], {opacity: 0.5, duration: 0.4}, 31.0);')
            inner.append(f'<div class="grid">{"".join(cells)}</div>')
        if sid == "s_anim":
            atl = "".join(f'<img class="atlas" id="at{i}" src="assets/{n}.jpg" alt="" />' for i, n in enumerate(["atlas_001", "atlas_005", "atlas_009", "atlas_013"]))
            inner.append(f'<div class="atlas-row" id="atlas_c">{atl}</div>')
            js.append(f'cardIn("#atlas_c", 41.0, {ATLAS_END});')
            js.extend(f'tl.fromTo("#at{i}", {{opacity: 0, y: 40}}, {{opacity: 1, y: 0, duration: 0.5, ease: "power3.out"}}, {41.3 + i * 0.25:.2f});' for i in range(4))
        if sid == "s_map":
            x, y, w, h = fit("win")
            k = w / SIZE["win"][0]
            inner.append(f'<div class="hint" id="drag_hint" style="left:{x - 4}px;top:{y + round(444 * k)}px;width:{round(226 * k)}px;height:{round(24 * k) + 8}px"></div>')
            js.append(f'tl.fromTo("#drag_hint", {{opacity: 0}}, {{opacity: 1, duration: 0.3, repeat: 3, yoyo: true}}, {DRAG[0] + 0.6});')
            js.append(f'tl.set("#drag_hint", {{opacity: 1}}, {DRAG[0] + 3.0}); tl.to("#drag_hint", {{opacity: 0, duration: 0.3}}, {DRAG[1] - 0.4});')
        if sid == "s_col":
            inner.append('<div class="layers" id="layers_c"><figure id="lay_bg"><img src="assets/layer_bg.png" alt="" /><figcaption>보이는 맵</figcaption></figure>'
                         '<figure id="lay_mask"><img src="assets/layer_mask.png" alt="" /><figcaption>충돌 마스크 (파란색 = 바닥)</figcaption></figure></div>')
            js.append(f'cardIn("#layers_c", {SECTIONS[4][2]}, {LAYERS_END});')
            js.append('tl.fromTo("#lay_mask", {opacity: 0, x: 40}, {opacity: 1, x: 0, duration: 0.5, ease: "power3.out"}, ' + str(MASK_AT) + ');')
        parts.append(f'      <div class="sec" id="{sid}">' + "".join(inner) + "</div>")

    secs = ", ".join(f'["#{sid}", {s}, {e}]' for sid, _, s, e in SECTIONS)
    caps = []
    for i, (t, d, s) in enumerate(CAPTIONS):
        caps.append(f'      <div id="c{i}" class="clip cap" data-start="{t}" data-duration="{d}" data-track-index="{6 + i % 2}"><p id="c{i}p">{html.escape(s)}</p></div>')
    cap_times = ", ".join(f"[{t}, {d}]" for t, d, _ in CAPTIONS)
    whooshes = "\n".join(f'      <audio id="wh{i}" src="assets/whoosh.wav" data-start="{s - 0.12:.2f}" data-duration="0.42" data-volume="0.3"></audio>'
                         for i, (_, _, s, _) in enumerate(SECTIONS[2:]))

    page = (ROOT / "build" / "template.html").read_text(encoding="utf-8")
    page = (page.replace("{{END}}", str(END)).replace("{{SECTIONS}}", "\n".join(parts))
                .replace("{{CAPTIONS}}", "\n".join(caps)).replace("{{WHOOSH}}", whooshes)
                .replace("{{SEC_TIMES}}", secs).replace("{{CAP_TIMES}}", cap_times)
                .replace("{{EXTRA_JS}}", "\n      ".join(js)))
    (ROOT / "index.html").write_text(page, encoding="utf-8")
    print(f"index.html: {len(CAPTIONS)} captions, {END}s")


if __name__ == "__main__":
    main()
