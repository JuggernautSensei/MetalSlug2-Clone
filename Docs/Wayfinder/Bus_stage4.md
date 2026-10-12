# 버스 계단 병사 4차 (지붕까지 오르기 / 몸통 뚫림, 실제 Stage0 경로)

## 재현 (실제 StageState + Stage0.json bus 섹션)
- 캡처: `MetalSlug2_Clone.exe --state stage0 --warp 4 --hold J --capture-ticks ...` (오프스크린). `--warp-x 2600` 은 플레이어가 안 걸어서 bus 섹션 (triggerX 2680) 이 발동하지 않는다 (`--hold Right` 도 이동하지 않음). `--warp 4` 는 bus 섹션 직전까지 조용히 적용한다.
- 테스트: `StageIntegration.BusStairSoldiersReachTheRoofAndNeverEnterTheBusBodyWithThePlayerOnTheLeft` (실제 StageState, 플레이어가 x 2700 에 서서 사격).
- 기존 추적 (`StageIntegration.BusStairSoldierTrace*`) 은 봇이 **오른쪽 락 끝까지 걸어가** 병사가 항상 지붕에 남았다. 사용자는 버스 왼쪽에 서서 쏜다. 이 차이가 "랩은 괜찮고 게임은 뚫린다" 의 원인이다.

## 원인
1. 계단 오르기 자체는 실제 스테이지에서도 정상이다: 스폰 (230,0) -> (186,0) -> (100,-106) -> 지붕 (88,-256) 을 약 190 틱에 오른다. `climbEnd` / `roofTop` / `roofSpan` 은 스테이지 경로에서도 같은 정의 (Bus.json) 로 적용된다. 깊이/z (병사 15, 몸통 15 sort 2, 덮개 16) 도 랩과 같다.
2. 진짜 원인: 지붕 위 병사가 **걸어서 지붕 발판 왼쪽 끝 (roofSpan.x = -27) 밖으로 나가 버스 위층 바닥 (solid 윗면 y-150) 에 떨어진다**. 플레이어가 버스 왼쪽에 있으면 Move/Reposition 이 병사를 왼쪽으로 걷게 하고, 병사 AI 에는 가장자리 판정이 없다. 그 위치 (앵커 -85..-27, 높이 -150) 는 그림상 버스 위층 창 높이라 "몸통 안에 서 있다 / 몸통을 뚫는다 / 중간 높이까지만 올라간다" 로 보인다.
   - 점프 베기도 지붕 한가운데 (x 33..60) 에서 시작하면 앵커 -55..-28 (몸통 위) 에 내려 같은 위층 바닥에 선다.
   - 실측 (수정 전, 플레이어 왼쪽 정지, 단계 0): 대검병/바주카병 모두 y -150 에 서 있음 (지붕 -256 이 아님).
3. 숫자 문제는 아니다 (roofTop/roofSpan/solid/stairsTop 은 그대로). 지붕 발판이 몸통보다 좁은 것은 의도 (지붕 윗선 실측 범위).

## 수정
- `SpawnParams`/`Enemy::roofSpan` (월드 x 범위) 추가, Bus.cpp 가 계단 병사에게 넘긴다 (roofSpan + 앵커).
- EnemyCommon: `OnRoof`, `AtRoofEdge`, `CanLeaveRoof`, `HoldOnRoof`.
  - `HoldOnRoof`: 지붕 위에 서 있는 동안 x 를 발판 안 (양끝 4 px 안쪽) 에 가둔다. Soldier/Bzk 가 물리 한 틱 뒤에 부른다.
  - Soldier: 지붕 위에서 `JumpSlashReady` 는 왼쪽 끝 30 px 안에서만 허용한다. 그 밖이면 Move 로 끝까지 걸어간 뒤 뛴다 -> 점프 호가 버스 왼쪽 밖 (앵커 -100 이하) 에 내린다.
  - Bzk: 지붕 끝에 닿으면 (`AtRoofEdge`) 거리와 상관없이 조준/사격한다 (제자리 걷기 방지).
- 공격 게이트는 그대로: Climb 상태에는 공격 결정이 없다.

## 전/후
| | 전 | 후 |
|---|---|---|
| 지붕 위 병사 (플레이어 왼쪽) | 대검병/바주카병이 y -150 (위층 바닥, 몸통 안) 에 섬 | 지붕 y -256 에 서고, 대검병은 왼쪽 끝에서 점프 베기로 버스 밖 지면에 내림 |
| 몸통 위 (|dx| < 80) 에서 지붕보다 아래 | 있음 | 0 틱 (테스트 고정) |
| 그리기 | 몸통 (15/2) < 병사 (15/발 y) < 덮개 (16/3), 모든 단계 | 같음 (이미 정상, 변경 없음) |

캡처: `Docs/Wayfinder/Bus_stage4.png` (tick 300..1300, 실제 Stage0, 병사는 지붕 위 / 버스 밖 지면, 몸통 안에 서지 않음).

## 테스트
- `StageIntegration.BusStairSoldiersReachTheRoofAndNeverEnterTheBusBodyWithThePlayerOnTheLeft` (실제 스테이지 경로)
- `Bus.StairSoldiersOnTheRoofNeverWalkOrJumpIntoTheBusBody` (리그, 시드 6 개, 1500 틱)
- 수정을 끄면 두 테스트 모두 실패함을 확인했다 (몸통 안 dx -33, 높이 지붕 아래).

## 검증
- Debug 빌드 exit=0, 경고 0. SlugTest: 알려진 3 개만 실패 (AnimIntegrity.LeftRightTwins..., StageIntegration.Boss* 2).
- Regress: enemylab_fight 7b12d9c08892fd47 PASS. stage0_walk = c9be5effe93e52e2 (기준 4489aa958d2ebdda 와 다름). **이 수정과 무관하다**: 수정을 끈 빌드에서도 같은 c9be5effe93e52e2 이다 (리플레이가 버스 구간 병사 행동에 닿지 않거나, 같은 작업트리의 다른 변경 -- 지프/BTank/대포 등 -- 때문). `-Update` 는 하지 않았다.
