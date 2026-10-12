# 버스 중간보스 계단 병사 개선 계획 U1~U2

원작: V1 1:07.7~1:19 (`videoplayback (1).mp4`), 증거 `Docs/Wayfinder/ref/U_orig_bus_stairs_V1_69.6-75.4.jpg`.
리메이크: `--state buslab_bot` 오프스크린 캡처 (`ref/U2_remake_soldiers_over_body_t420-620.jpg`, `ref/U2_remake_bus_right_t300-590.jpg`, `ref/baseline/bus_climb_sheet.jpg`).
기준 점수: **bus_climb 0.255** (timing 0.012, position 0.190, frame 0.615) - 목표 >= 0.75.

## 원작 (V1)
- 67.5~68.5 버스가 오른쪽에서 미끄러져 들어와 선다 (리메이크는 사전 배치, Spec_Bus R3-06 결정).
- 69.8~70.2 버스 오른쪽 (뒤) 에 계단이 위에서 날아와 붙는다. 계단은 버스 **바깥 옆면** (화면 안, 버스 오른쪽 끝 바로 옆).
- 72.4~75.4 초록 병사들이 그 계단 위 여러 높이에 서 있다/올라간다 (계단 난간 뒤). 병사가 버스 몸통 앞 (창/광고판 위) 에 겹쳐 그려지는 프레임은 없다. 창 안 병사 (inner soldiers) 는 창틀 안에만 보인다.
- 78.0 대폭발, 78.5~79.5 잔해 (`Bus_Wreck` 템플릿 conf 0.55 @ 79.5). 개방 (계단 날아옴 69.6) -> 파괴 약 9 s.
- 버스 화면 위치 (Bus_OpenBody 템플릿, 608 폭 게임 px): 피벗 x 464~476, y 386.

## 리메이크 현재
- 계단 병사 스폰 Bus.cpp:130-163 (`SpawnStairSoldier_`): 스폰 앵커+(230,0) -> arena.min = stairsBase (186,0) -> arena.max = stairsTop (100,-106) (Bus.json:29-31). 동시 최대 `soldierMax` 4, 간격 `soldierGapSec` 1 (Bus.json:33-36; EnemyDef.h:621-623 기본 2 / 2.8 s, Spec_Bus 는 2 / 2.83 s -> **데이터가 스펙과 어긋남**).
- 오르기 Soldier.cpp:146-177 / Bzk.cpp:103-131 (`TickClimb_`): 두 점을 직선으로 미끄러진다 (물리/중력 없음, 클립은 걷기). 끝나면 Fall -> 승강대 벽 (EnemyWorld.cpp:912-917, 폭 34, 윗면 = 앵커 y-106) 에 선다. Climb 중에는 공격 판단이 없다 (정상).
- 착지 뒤: 폭탄병 (variant 3) 은 승강대에서 제자리 투척 (Soldier.cpp:72-75, 193). 대검병/바주카 (variant 2) 는 일반 AI: 왼쪽 (플레이어) 으로 걸으면 승강대 왼쪽 끝 (앵커+83) 에서 버스 solid (170x150, 윗면 앵커 y-150, Bus.json:17) 의 44 px 턱을 만나 WallAhead -> HJump (jumpVy -350, 정점 76 px) 로 solid 윗면 (y-150) 에 올라선다.
- 그리기: 병사 = 레이어 15, sort = 발 y (EnemyWorld.cpp:715). 버스 back/body = 레이어 15 sort 1/2, cover = 16 (Bus.json:45-54) 인데 **cover (`Bus_OpenTop`) 는 단계 0 에서만 그린다** (Bus.cpp:322-323). 단계 1 이후에는 body (sort 2) 위로 병사 (sort ~272) 가 항상 앞에 그려진다.
- 카메라: 스테이지 버스 락 [2482, 3122] (Stage0.json bus 섹션, 사용자 수정), 버스랩 [1800, 3136]. 계단 아래 (stairsBase x 3171), 스폰 (3215) 은 뷰 오른쪽 밖 -> **오르는 과정의 대부분이 화면 밖**이고, 병사는 승강대 (x 3085, 높이 106) 에 "튀어나오듯" 보인다.

## U2. 군인이 버스 껍질을 뚫는 것 - 원인 판정
증거 프레임 (`U2_remake_soldiers_over_body_t420-620.jpg`): tick 540 에 대검병 + 바주카병이 버스 앞/위층 창 위에 겹쳐 서 있다. tick 420 에 병사 1 명이 버스 지붕 위.
원인은 둘 다다:
1. **그리기 순서 (주원인)**: 단계 >= 1 에서 앞 덮개가 없고, 병사 sort (발 y) 가 body sort 2 보다 커서 몸통 앞에 그려진다. cover 16 은 단계 0 (개방 애니) 에만 존재.
2. **서는 높이/경로**: 승강대 윗면 y-106 은 버스 (높이 264) 의 아래층 창 높이, solid 윗면 y-150 은 위층 한가운데다. 병사가 이 높이에 서거나 걸으면 그림상 버스 몸통 "안" 이다. 원작은 계단이 버스 옆 바깥이라 이런 일이 없다.
(충돌/물리로 solid 를 통과하는 것은 아니다: Climb 경로 (186,0)->(100,-106) 는 solid (x <= 85) 와 겹치지 않는다.)

## U1. 계단에서 멈춤 - 진단
틱별 추적 (위치/상태) 은 게임/테스트 코드를 고치지 않고는 뽑을 수 없었다 (CLI 덤프 없음, 계단 구간이 화면 밖이라 캡처로도 안 보임). 코드 근거로 본 후보 (가능성 순):
- H1 승강대-턱 루프: 대검병/바주카가 승강대 (34 px) 위에서 왼쪽으로 가다 44 px 턱 -> HJump -> solid 윗면 -> 다시 Move/Idle 결정. `cooldownTicks`/WallAhead 반복으로 같은 자리에서 뛰었다 섰다 하면 "멈춘" 것처럼 보인다.
- H2 폭탄병은 설계상 승강대에 계속 서 있다 (variant 3 = 제자리) -> 사용자에게는 "올라가다 멈춤" 으로 보임. 게다가 `soldierMax 4` 라 승강대에 여러 명이 겹쳐 선다.
- H3 경로와 그림 불일치: 계단 그림 `Bus_StairsRail` 은 피벗 -82 (앵커 오른쪽 82~210 px 에 그려짐, 높이 258) 인데 경로는 두 점 직선 + 걷기 클립 -> 계단 칸을 밟지 않고 떠서 미끄러진다. 끝점 (100,-106) 은 계단 꼭대기 (위층 높이) 가 아니라 중간.
- H4 화면 밖 구간: 앞 절반이 뷰 밖이라 플레이어에게는 승강대에서 갑자기 나타나 멈춘 것처럼 보인다.
-> 첫 작업으로 추적 테스트를 만들어 H1~H4 를 확정한다 (U-T1).

## 제안
1. (HAIKU, S) U-T1 추적 테스트 `BusTest.StairSoldierTrace`: 버스 + 뷰 2496 (스테이지 락과 같은 값) 에서 900 틱, 계단 병사마다 틱별 (pos, state, clip, bGrounded, aiVariant) 를 `Captures/bus_trace.csv` 로 (환경변수 `BUS_TRACE`, C15_DUMP 방식) 남기고, 단언: (a) Climb 상태에서 30 틱 넘게 위치 변화 0 이 없다, (b) 계단 끝 도달 전 공격 상태 (Throw/Slash/JumpSlash/Bzk Shot) 진입 없음, (c) 끝 도달 시각 <= 스폰 + 3 s. 결과로 H1~H4 판정 문서화.
2. (SONNET, M) U-T2 계단 경로 = 계단 그림을 따라가는 웨이포인트: `stairsPath` [[230,0],[196,0],[176,-26],[156,-56],[136,-90],[116,-130],[100,-170],[86,-262]] (계단 칸 위치는 `Bus_StairsRail` 마지막 프레임에서 실측해 HAIKU 가 채운다; 위 값은 형태 예시) - 마지막 점 = **버스 지붕 뒤쪽** (위층 지붕 윗면). Climb 은 웨이포인트를 차례로, 끝에서 지붕 플랫폼에 착지. 지붕 플랫폼 = 기존 landing 벽 대신 `roofTop` 단방향 발판 (앵커 기준 [-90, 90] x, y = 지붕 높이, 단계 6 이후 (상층 소실) 제거).
3. (SONNET, S) U-T3 공격 게이트: Climb -> Land 끝난 뒤에만 AI 시작 (현재도 Climb 중 공격 없음; Land 후 `climbSettleTicks` 20 을 추가해 착지 직후 즉시 사격 방지). 지붕 위에서 대검병은 지붕 앞끝에서 점프 베기로 내려오고, 폭탄병/바주카는 지붕에서 투척/사격.
4. (SONNET, S) U-T4 그리기: 계단 위 병사는 계단 난간 뒤, 버스 몸통보다는 앞이어야 한다 (원작: 계단은 버스 옆 바깥). 병사가 Climb/지붕 위에 있는 동안 sort 를 `partStairs` 와 같은 레이어/아래로 고정하는 `SpawnParams.depthOverride` (또는 userTag 0x42555300 이면 EnemyWorld 가 depth 를 버스 parts.stairsSoldier 값으로) - 지붕 위에서는 body 앞이 맞다 (지붕 윗면이므로 겹침 없음). 단계 >= 1 에서 몸통 위를 걷지 않도록 solid 윗면 (-150) 을 걷는 경로 자체를 없앤다 (2 번의 지붕 발판 + solid 높이를 지붕까지).
5. (HAIKU, S) U-T5 데이터 정합: `soldierMax` 4 -> 2, `soldierGapSec` 1 -> 2.83 (Spec_Bus 와 EnemyDef.h 기본값; 사용자 의도면 Spec 을 고침 - 사용자 확인).
6. (HAIKU, S) U-T6 채점 개선: buslab 카메라 bound 를 +120 (3256) 으로 넓힌 `buslab_wide` 상태 또는 `--warp-x` 기반 런을 scenes.json 에 추가해 계단이 화면에 들어오게 하고, T0 로그의 `bus.soldier_*` 이벤트로 "계단 끝 도달 -> 첫 공격" 시각을 원작 (V1 계단 병사 72.4 등장, 공격 시작 시각은 T1 추적과 함께 프레임 측정) 과 비교.

| # | 모델 | 크기 | 의존 | 수락 |
|---|---|---|---|---|
| U-T1 | HAIKU | S | - | 추적 CSV + 단언 3 개, H1~H4 판정 기록 |
| U-T5 | HAIKU | S | 사용자 확인 | BusDataTest 기대값 갱신 |
| U-T2 | SONNET | M | U-T1 | `BusTest.StairSoldierFollowsStairsToRoof` (웨이포인트 오차 <= 3 px, 지붕 착지), bus_climb >= 0.75 (U-T6 후) |
| U-T3 | SONNET | S | U-T2 | `BusTest.StairSoldierAttacksOnlyAfterReachingTheRoof` |
| U-T4 | SONNET | S | U-T2 | `BusTest.StairSoldiersNeverDrawOverTheBusBody` (계단/지붕 위 병사의 제출 depth/sort < body 또는 위치가 몸통 사각형 밖), 시트 육안 |
| U-T6 | HAIKU | S | T0 | bus_climb 장면이 계단 구간을 포함, 점수 산출 |

## 새 아트
- 필요 없음. 원작 계단 병사 자세는 저해상도에서 걷기/서기와 구분되지 않는다 (전용 계단 오르기 클립 없음). 계단 칸 위치만 `Bus_StairsRail` 에서 실측.

## 위험
- 사용자가 Stage0.json 버스 섹션 (트리거 2680, 락 [2482,3122]) 을 바꿔 뷰 오른쪽이 3122 로 줄었다: 계단 (앵커+82~+210 = 3067~3195) 의 오른쪽 절반은 원래도 화면 밖. 원작처럼 계단 전체가 보이려면 락을 +80 정도 오른쪽으로 (사용자 영역) 하거나 버스 앵커를 왼쪽으로.
- 버스 위치: 원작 대비 리메이크 버스가 화면에서 66 px 오른쪽 (bus_body 추적 중앙값, 버스랩 기준). 랩 카메라와 스테이지 락이 달라 스테이지 기준 재측정 필요.
- buslab_bot 화력이 원작 플레이보다 세서 버스가 4.5 s 일찍 부서진다 (timing 0.012): 파괴 시각은 장면 점수에서 가중치 0.5 로 낮췄지만 봇 의존이다.
