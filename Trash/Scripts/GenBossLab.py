#!/usr/bin/env python3
"""보스 연습장 스테이지 (Assets/Stage/BossLab.json) 생성기.

Stage0.json 의 world/ground/backdrop/bgm/camera 설정과 openland (11350 이후) 의 소품/장식/충돌체 스폰을 그대로 가져오고,
적/아이템/NPC/웨이브는 뺀다. boss_arena 섹션은 이 파일의 ARENA (Stage0.json 에 넣을 최종 JSON 과 같다) 를 쓴다.
사용: python3 Scripts/GenBossLab.py
"""
import copy
import json
import os

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
STAGE = os.path.join(ROOT, "Assets", "Stage")

ARENA = {
    "id": "boss_arena",
    "triggerX": 13700,
    "note": "최종보스 전함 아레나 (Docs/Spec_BigShiee.md). 트리거 13700 은 사루비아 락 (플레이어 x <= 13066) 밖이라 사루비아를 잡고 락이 풀린 뒤에만 발동한다. 락 사각형은 보스가 자동 스크롤하며 오른쪽으로 옮긴다 (StageRunner::AddCameraScroll). 보스는 락 왼쪽 밖에서 스스로 입장한다 (x 는 무시되고 y 는 지면). 파괴 연출이 끝나면 (destroyFinished) 락 해제 -> 클리어 흐름.",
    "actions": [
        {"type": "lockCamera", "rect": [13700, 864, 14340, 1312], "glide": 1.0},
        {"type": "spawn", "kind": "enemy", "def": "bigshiee", "id": "bigshiee", "x": 14100, "groundSnap": True, "fromY": 1100, "facing": "right"},
        {
            "type": "gate",
            "id": "bigshiee_dead",
            "waitFor": ["bigshiee"],
            "until": "destroyFinished",
            "actions": [
                {"type": "unlockCamera"},
                {"type": "clear", "waves": True},
                {"type": "playerInvincible", "seconds": 10.0},
                {"type": "endStage", "next": "title"},
            ],
        },
    ],
}


def main():
    with open(os.path.join(STAGE, "Stage0.json"), encoding="utf-8") as f:
        src = json.load(f)

    out = {
        "version": 1,
        "name": "BossLab",
        "note": "Scripts/GenBossLab.py 가 Stage0.json 에서 만든 보스 연습장. 직접 고치지 말 것.",
        "world": src["world"],
        "bgm": src["bgm"],
        "ground": {"initial": "Stage0_1", "maps": src["ground"]["maps"]},
        "backdrop": {"initial": "fourth", "modes": src["backdrop"]["modes"]},
        "player": {"x": 13300, "groundSnap": True, "fromY": 1000},
        "camera": {
            "policy": "forward",
            "initialBounds": [3180, 864, 14094, 1312],
            "initialOffset": src["camera"]["initialOffset"],
            "playerMargin": src["camera"]["playerMargin"],
        },
    }

    props = []
    for section in src["sections"]:
        if section["triggerX"] < 11350 or section["id"] == "boss_arena":
            continue
        for action in section["actions"]:
            if action.get("type") == "spawn" and action.get("kind") in ("prop", "decor", "collider"):
                props.append(copy.deepcopy(action))

    # 끝 소품을 주기 (1024) 반복으로 늘린다: 보스 구간은 무한맵이다
    extended = []
    for action in props:
        if action.get("def") in ("big_house", "bridge_block") and action["x"] >= 16242:
            action["repeat"] = {"count": 14, "stepX": 1024, "stepY": 0}
        extended.append(action)

    out["sections"] = [
        {"id": "openland_props", "triggerX": 11350, "actions": extended},
        copy.deepcopy(ARENA),
    ]

    with open(os.path.join(STAGE, "BossLab.json"), "w", encoding="utf-8", newline="\n") as f:
        json.dump(out, f, ensure_ascii=False, indent=1)
        f.write("\n")
    with open(os.path.join(STAGE, "BossArena.json"), "w", encoding="utf-8", newline="\n") as f:
        json.dump(ARENA, f, ensure_ascii=False, indent=1)
        f.write("\n")
    print("wrote BossLab.json", len(extended), "props")


if __name__ == "__main__":
    main()
