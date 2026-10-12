# 버스 중간보스 스펙 (Bus.cpp / Bus.json / Bzk.cpp / Soldier.cpp)

원작 영상 V1 1:07.7~1:19, V2 1:09~1:20 (VideoTimeline.md B절). 좌표는 앵커(바퀴 바닥 중앙) 기준, +x 월드 오른쪽, 게임 px.

## 배치와 개방
- 슬라이드 인 없음. 스폰 위치가 곧 정지 앵커 (스테이지 기준 x 2985, y 378).
- Parked 상태: 체력을 매 틱 채워 무적, 비공격, `Bus_Idle` 그림.
- 오프닝 트리거 (Stage0.json `bus` 섹션): 플레이어 발 x 가 `triggerX` 2694 를 넘으면 발동.
  - 근거: 영화 홍보 간판 건물 `shop_5` (x 2513, 스프라이트 폭 약 322 -> 우측 끝 약 2674) 를 발 기준으로 완전히 지난 지점 = 우측 끝 + 플레이어 반폭 (~20).
  - 발동 시 `lockCamera` rect [2496, 0, 3136, 448] 를 2.5 s 동안 글라이드 (락 뷰 x 2496 = 버스 왼쪽 끝 - 640 + 여유). 글라이드 중 카메라 수평 속도 변화는 2 px/tick 이하 (StageIntegration `BusLockGlideMovesTheCameraSmoothly`).
  - 버스 개방은 카메라 뷰 오른쪽 끝 >= 앵커 + `openViewDx` (-100 = 버스 왼쪽 끝 2885) 에서 시작. 이후 aux = 경과 틱.
- 개방 시간표: openDelay 10틱 -> 개방 9프레임(36틱) / 계단 +46틱 -> 비행 32 + 착지 6 -> 난간 클립.

## 패턴
- 버스 본체는 공격하지 않는다 (창 사격, 폭탄 투척 없음). 병사만 공격한다.
- 계단 병사: 계단이 펼쳐진 뒤 `soldierFirstSec` 0.67 s 부터 `soldierGapSec` 2.83 s 간격, 동시 최대 2명 (`soldierMax`), 단계 5 (`soldierLastStage`) 까지.
  - 스폰: 앵커 + (230, 0) (화면 오른쪽 밖), userTag 0x42555300, SpawnDeferred.
  - 종류 비율 (결정적 난수): 대검병 40% (`swordPercent`, aiVariant 2), 바주카병 30% (`bazookaPercent`, aiVariant 2), 폭탄병 나머지 30% (aiVariant 3).
- 공통 계단 이동 (Soldier.cpp / Bzk.cpp `Climb` 상태, aiVariant 2/3): 스폰 -> `arena.min` (계단 아래 앵커 + (186, 0)) -> `arena.max` (승강대 앵커 + (100, -106)) 를 병사 속도로 걷는다 (물리 없음). 끝나면 낙하로 넘어가 승강대(벽, `landingW` 34) 에 선다.
  - 폭탄병 (aiVariant 3): 승강대에 서서 탐지 거리와 무관하게 폭탄 투척.
  - 바주카병 (aiVariant 2): 승강대(버스 위)에 서서 플레이어를 향해 로켓 (`Bzk.cpp`, 자세별 22.5 도 단위 조준, 탄수 `bullets`).
  - 대검병 (aiVariant 2): 일반 AI. 승강대에서 지붕(버스 solid 윗면)으로 점프 베기 후 왼쪽으로 내려온다.

## 피해 4단계 / 파괴
- stageHp [112,100,85,62,48,34,24] (HP 160): 온전 -> 창 파손 -> 상층 소실 + 화재 -> 폭파. 단계 진입마다 폭발 묶음.
- 사망: 대폭발 + 파편, 0.1 s 뒤 차체 -> `Bus_Wreck`, 1.0 s 에 파괴 완료 (게이트 bus_clear). 잔해 지형은 스테이지 ground_swap (Stage0_1).
- 히트박스: hurtbox 208 x 264 (스프라이트 204 x 276 에 맞춤), solid 170 x 150 (벽) + 승강대 벽.

## 랩 / 검증
- `MyMetalSlug2.exe --state buslab_bot --no-audio --capture <prefix> --capture-ticks ...`: 플레이어 시작 x 2300, 봇이 x 2780 까지 걷고 tick 420 부터 HMG 사격, 무적. `buslab` 은 사람이 플레이.
- 테스트: Test/BusTest.cpp (개방 트리거, 무적, 병사 스폰/계단, 바주카병 승강대 사격), Test/StageIntegrationTest.cpp (`BusIsPreplaced...`, `BusLockGlideMovesTheCameraSmoothly`).
