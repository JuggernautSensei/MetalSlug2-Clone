# 버스 계단 병사 2차 (U-T1..U-T4, U-T6)

## U-T1 추적 결과 (가설 판정)
추적: `BUS_TRACE=<csv> SlugTest --gtest_filter=Bus.StairSoldierTrace*` (리그, 앵커 2400, 솔리드 벽 포함) 와 `BUS_TRACE_STAGE=<csv> ... StageIntegration.BusStairSoldierTrace*` (실제 Stage0, bus 섹션 워프).
- 오르기 자체는 멈추지 않는다: 스폰 (230,0) -> (186,0) -> (100,-106) 을 약 108 틱 (1.8 s) 에 걷는다 (H4 화면 밖 구간만 사실). H3 (경로-그림 불일치) 는 아님: 계단 그림 (Bus_BackD1 경사 윗끝 = 앵커 + 88, 높이 약 108) 이 stairsBase/stairsTop 과 맞는다.
- 진짜 원인 (H1 + H2): 승강대 (y-106) 에 도착한 뒤 **거기서 끝난다**. 폭탄병 (variant 3) 은 승강대에 영구 정지, 대검병/바주카는 solid 윗면 (y-150, 앵커 + 23..28, 위층 창 높이 = 몸통 그림 안) 에 올라 선다. 그래서 "지붕 위"에 가지 못하고 "버스 안에서 멈춘 듯" 보인다. (이전 리그는 평지 폭 3000 밖 (앵커 2985+100) 에 병사가 떨어져 승강대를 보지 못했다: 리그 벽 추가 + 앵커 2400 로 수정.)
- 그리기 (U2): 단계 >= 1 에서 덮개 (Bus_OpenTop) 가 없어 병사 (sort = 발 y) 가 몸통 (sort 2) 앞에 그려졌다.

## 수정
- 새 선택 파라미터 (Bus.json `roofTop` [88,-256], `roofSpan` [-27,92]; 없으면 예전 동작): 계단 끝 (stairsTop) 뒤에 지붕 위 점까지 한 구간을 더 오른다 (`ClimbStep`, EnemyCommon.cpp, Soldier/Bzk 공용; `SpawnParams.climbEnd`), 지붕 발판은 얇은 솔리드 (EnemyWorld::GetSolidBoxes). 사용자 숫자 (stairsBase/stairsTop/solid/soldierMax ...) 는 그대로.
- 지붕 높이 256 은 Bus_BodyD1..3/OpenBody 윗선 실측 (x 8..90 에서 250..264).
- 공격은 Climb 이 끝난 뒤 (지붕 도착, 착지 클립 + cooldown) 에만 시작: Climb 상태에는 공격 결정이 없다 (테스트로 고정).
- 그리기: 단계 >= 1 에서 BodyD 를 덮개 파트 (depth 16, sort 3) 로 한 번 더 그려 지붕/창 앞면이 병사 앞에 오게 한다. 창 파편은 덮개보다 위 (depth 16, sort 4).
- 리그 (VehicleRig) 는 이제 적 solid 를 벽으로 쓴다 (실제 게임과 동일).

## 테스트
- `Bus.StairSoldiersClimbToTheRoofWithoutGettingStuck` (지붕 y 도달 <= 400 틱, 30 틱 이상 정지 없음)
- `Bus.StairSoldiersDoNotAttackBeforeReachingTheRoof` (시드 6 개, 도착 전 사격/베기 없음)
- `Bus.StairSoldiersOnTheRoofAreDrawnInFrontOfTheBodyAndBehindTheCoverInEveryStage` (단계 0, 1, 4: body < soldier < cover)
- 진단: `Bus.StairSoldierTraceWritesCsvWhenRequested`, `StageIntegration.BusStairSoldierTraceWritesCsvWhenRequested`

## 결과 / 한계
- SlugTest: 알려진 3 개만 실패 (AnimIntegrity.LeftRightTwins..., StageIntegration.Boss* 2). Debug 경고 0.
- Regress: enemylab_fight 동일. **stage0_walk 해시가 바뀐다** (acfc69d63f33c641 -> 4489aa958d2ebdda). Bus.json 에서 roofTop/roofSpan 을 빼면 acfc69... 로 PASS: 병사가 지붕까지 올라 행동이 달라진 의도된 변화이므로 확인 뒤 `Regress.ps1 -Update`.
- 하네스 bus_climb: 전 0.255 -> 후 0.256 (timing 0.007, position 0.19, frame 0.622). 막는 것: (1) 버스 화면 x 가 원작보다 약 54..66 px 오른쪽 (buslab 앵커 = 락 오른쪽 - 640 + 489, 사용자 위치 숫자), (2) buslab_bot 이 버스를 약 148 틱 일찍 부숨 (봇 화력). 둘 다 게임 쪽 값이라 장면 설정만으로는 0.75 에 닿지 않는다. 캡처 시트에서 지붕 위 병사는 보이고 몸통을 뚫지 않는다 (`Captures/VideoScore/after1/runs/bus`).
