# 진행 중 작업 스냅샷 (미완성)

토큰 한도로 중단된 3단계 작업의 변경분을 패치로 보관한다. 빌드에 포함되지 않으며, 완성/검증되지 않았다.
기준 커밋: `2f5bb86`. 적용: `git apply --3way WIP/<name>.patch` (이후 커밋들과 충돌하면 수동 정리).

| 패치 | 내용 | 상태 |
|---|---|---|
| `bus.patch` | 버스 중간보스 (시트 33825, `Assets/Defs/Bus.json`, `Bus.cpp`, `buslab`) | 구현 중, Baker 재빌드 전 |
| `heli.patch` | 헬기 (시트 20642, `Assets/Defs/Helicopter.json`, `Heli.cpp`, `HeliBomb.cpp`, `helilab`) | 구현 중, 빌드 전 |
| `battleship.patch` | 최종보스 배틀쉽 (시트 40204, `BigShiee.h`, `BigShieeCfg.h`, `Scripts/GenBossSheet.py`) | 설계/초기 구현 |
| `sv001.patch` | 메탈슬러그 SV-001 (시트 32522, `Source/Game/Player/Slug*`, HUD, `sluglab`) | 구현 중, `SlugDef` 작성 단계 |

네 패치 모두 `sheets.json`, `EnemyBehaviors`, `EnemyDef*`, `SandboxState`, `StateFactory`, Baker `SheetSource` 등을 공통으로 건드리므로
함께 적용할 때 충돌을 정리해야 한다.
