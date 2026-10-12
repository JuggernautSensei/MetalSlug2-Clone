# Vehicles plan: V1 강 지프 진입 / V2 BTank 방향 전환

분석 전용 문서 (2026-10-12). 코드/에셋은 고치지 않았다. 증거 이미지: `Docs/Wayfinder/ref/Vehicles_*.jpg`, 리메이크 이벤트 로그 `ref/Vehicles_remake_events.jsonl`.
라벨: **SONNET** = 판단이 필요한 코드 변경, **HAIKU** = 기계적 JSON/주석/테스트 수치 변경.

좌표 환산 (Scoring.md): 아케이드 304 px = 리메이크 608 px (2x), 리메이크 뷰 640 은 좌우 16 px 더 보인다.
V1 (496 폭) 1 vpx = 1.226 게임 px, V2 (480 폭, 위아래 4 px 크롭) 1 vpx = 1.267 게임 px. 리메이크 뷰 x = 16 + 아케이드2x x.

---

## V1. 강 지프 (river_jeep): "왼쪽에서 나타나 왼쪽에서 공격"

### 1.1 원작 영상 증거 (아케이드 기준)

| 사건 | V2 (`videoplayback (2).mp4`) | V1 (`videoplayback (1).mp4`) | 비고 |
|---|---|---|---|
| 지프 앞코가 화면 왼쪽 끝에 나타남 (우향, 앞코 먼저) | 2:36.40 | 3:04.15 | 정지 상태로 보이는 프레임 없음: 화면 밖에서 이미 달려 들어온다 |
| 정지 (우향, 앞코 x) | 2:36.75, 앞코 약 55 vpx | 3:04.80, 앞코 약 82 vpx | 진입 0.35~0.65 s, 속도 약 145 vpx/s (V2) = **약 185 게임 px/s**, V1 약 155 (끝에 감속) |
| Turn (앞코가 왼쪽으로 돌아감, 발사대가 오른쪽=플레이어 쪽) | 2:36.9 ~ 2:37.95 | 3:04.9 ~ 3:05.8 | Jeep_Turn 0.96 s 와 일치 |
| 선회 후 정지 위치 (좌향 차체) | 차체 0..62 vpx, 중심 약 31 vpx = 리메이크 뷰 **x 약 55** | 거의 같음 | 현재 edgeInset 75 |
| Idle (발사대 위로) | 2:37.95 ~ 2:38.9 (약 1.0 s) | 3:05.8 ~ 약 3:06.8 | idleSec 1.0 일치 |
| Aimming (발사대 수평) | 2:38.9 ~ 2:39.6 | 약 3:06.8 ~ 3:07.6 | |
| **조준 유지 (발사 전 대기)** | **2:39.6 ~ 2:41.45 (약 1.85 s)** | **약 1.1 s** | 리메이크는 4발째(counter=4) 에서 대기 0 |
| Shot_Ready (발사대 뒤쪽=왼쪽으로 화염/연기) | 2:41.45 ~ 2:42.5 | 3:08.8 ~ 3:09.6 | 0.96 s 일치. 이미지 `ref/Vehicles_V2_jeep_firstrocket.jpg` |
| 첫 로켓이 오른쪽으로 날아감 | 2:42.5 생성, 2:42.9 이동 시작 | 약 3:09.8 | 정지 -> 첫 로켓: **V2 5.75 s, V1 5.0 s** |
| 이후 | 플레이어 수류탄 폭발에 가려 2발째 확인 불가, 2:44.0~2:44.5 Turn(우향), 2:45.0~2:47 오른쪽으로 주행 (약 150 게임 px/s, 플레이어를 지나감), 2:47.2~2:47.8 우측에서 파괴 | 3:10.4 왼쪽 자리에서 파괴 | 떠나는 쪽 = 플레이어 반대편 가장자리 (현 IdleAndMove 규칙과 일치). 4연발 여부는 미확인 -> bullets 4 유지 |

요약: **화면 왼쪽 밖에서 우향으로 달려 들어와 (대기 없음) 왼쪽 가장자리 근처에서 멈추고, 한 번 선회해 좌향 (클립 `_Left`, 발사대 후방이 오른쪽) 으로 오른쪽 플레이어에게 로켓을 쏜다.** 처음부터 끝까지 지프는 플레이어 왼쪽에 있다.
이미지: `ref/Vehicles_V2_jeep_entry.jpg` (2:35.5~2:38.3, 6 fps), `ref/Vehicles_V1_jeep_entry.jpg` (3:03.4~3:05.1, 10 fps, 20 vpx 격자), `ref/Vehicles_V2_jeep_aimhold.jpg`, `ref/Vehicles_V2_jeep_driveaway.jpg`.
클립 방향 확인: `Jeep_*_Left` = 앞코 왼쪽, 로켓 화염은 왼쪽으로 / 로켓은 오른쪽으로 (EnemySpecs 2.1 규약 맞음, sprites.py 로 확인).

### 1.2 현재 동작 (리메이크 캡처)

실행: Release 빌드 (03:49 빌드 = 최신 소스), `--replay walk135.rep (오른쪽 135 틱 후 정지) --warp-x 4990 --life 99 --event-log --capture-ticks 0..1795/5`, 오프스크린 소프트웨어 렌더. 플레이어 정지 x 약 5552.
이미지: `ref/Vehicles_remake_jeep_now.jpg` (틱 80..410, 10 틱 간격, 시트 라벨 시각은 무의미), `ref/Vehicles_remake_jeep_rightedge.jpg` (틱 340..820, 15 틱 간격).

| 틱 | 상태 / 위치 |
|---|---|
| 약 126 | river_mid (triggerX 5515) -> 지프 스폰 x 5335 우향. 뷰 min 약 5357 -> 22 px 밖이라 **차체 절반이 보이는 채로** BTank (x 5400) 위에 겹쳐 서 있다 |
| 126 ~ 186 | `Wait -> IdleAndMove` 1 s 정지 (VehicleJeep.cpp:93-106). 원작에는 없는 정지 |
| 186 | 플레이어가 오른쪽 -> `aux=+1` -> 목표 = **뷰 오른쪽 - 75 (약 5922)** (VehicleJeep.cpp:103, 57-63) |
| 186 ~ 362 | 200 px/s 로 화면을 가로질러 플레이어를 지나 오른쪽 끝으로 |
| 362 ~ 600 | Turn (좌향) -> Idle -> 플레이어가 발사 쪽 (+x) 이 아님 -> Turn (우향) -> Idle (VehicleJeep.cpp:127-148) |
| 약 719 | 첫 로켓 예정 (플레이어 사망 중이라 FireWeapon 이 쏘지 않음, EnemyCommon.cpp:308) |
| 818 / 1035 / 1133 | `enemy.shot jeep x 5847..5867 vx -298.5`: **오른쪽 끝에서 왼쪽으로** 발사 |

차이 (gap):
1. 진입 방향/목표: 첫 Move 목표를 플레이어 쪽 가장자리로 고른다 -> 왼쪽에서 들어와도 오른쪽 끝까지 달려가 **오른쪽에서 공격**한다 (사용자 지적의 원인).
2. 스폰 직후 1 s 정지 (IdleAndMove) + 스폰 x 가 고정 월드 좌표 5335: 섹션을 옮기면서 화면 밖 22 px 가 되어 반쯤 보이며 서 있다. 카메라 기준이 아니라 섹션 이동 때마다 깨진다.
3. 첫 발 전 조준 유지가 없다: 원작 정지 -> 첫 로켓 5.0~5.75 s, 리메이크 경로로 계산 0.96+1.0+0.96+0.96+0.08 = 3.96 s.
4. (작음) 정지 위치 edgeInset 75 vs 원작 약 55. 속도 200 vs 원작 약 150~185 (오차 범위 큼, 유지 권장).
5. (위험) 왼쪽 정지점 (뷰 min + 55 = 약 5412) 이 BTank (5400) 와 겹친다.

### 1.3 변경 목록

| # | 라벨 | 파일 | 내용 |
|---|---|---|---|
| J1 | **SONNET** | `Source/Game/Enemies/VehicleJeep.cpp` (Wait 상태 93-98) | **화면 밖 진입 규칙**: 활성화 순간 지프 x 가 뷰 밖 (또는 `x < view.min + edgeInset` 이고 facing +1, 대칭으로 오른쪽) 이면 `aux = -facing` (들어온 쪽 가장자리) 로 정하고 **IdleAndMove 없이 바로 Move**. 그 외 (뷰 안 스폰) 는 기존 경로 유지 -> 기존 테스트 `WaitsOneSecondBeforeItMovesAfterActivation`, `DrivesAtTwoHundred...` 영향 없음. Move 도착 -> Turn -> Idle -> (플레이어가 오른쪽이면 발사 쪽) Aimming 은 기존 코드 그대로 맞다. 새 설정값 없음 |
| J2 | **SONNET** | `VehicleJeep.cpp` ShotIdle (157-169), `ParseJeepCfg`, `EnemyDef.h` JeepCfg | 첫 발 전 대기 `firstWaitSec` 추가: `counter == bullets` 일 때도 `firstWaitTicks` 대기 후 ShotReady. 기본 0 (기존 동작) |
| J3 | **HAIKU** | `Assets/Defs/Vehicles.json` jeep.params | `"firstWaitSec": 1.5` (V1 1.1 / V2 1.85 의 중간), `"edgeInset": 55`. speed 200 유지 (원작 측정 150~185, 신뢰도 낮음) |
| J4 | **HAIKU** (사용자 확인 후, Stage 편집 금지 규칙 대상) | `Assets/Stage/Stage0.json` river_mid 의 river_jeep | `"x": 5335, "groundSnap": true` 를 카메라 앵커로: `"camera": {"ax": "left", "dx": -70, "ay": "top", "dy": 360}` (+ `"facing": "right"` 유지). dy 360 = 락 top 288 기준 강바닥 발 y 658 보다 10 px 위 (지프 중력으로 착지; 발 y 는 로켓 muzzle y 570 + 88 에서 역산). dx -70 = 차체 반폭 약 60 + 10 이라 완전히 화면 밖. 카메라 앵커는 groundSnap 과 같이 못 쓴다 (StageData.cpp:1638) 그래서 dy 로 둔다. 섹션을 옮겨도 "현재 뷰 왼쪽 밖" 이 유지된다 (StageRunner.cpp:699-711, 스폰 순간 카메라 위치 기준) |
| J5 | 사용자 결정 | Stage0.json river_start 의 btank x 5400 | 지프 정지점 (약 5412) 과 겹침. 원작에서 이 자리 전차 (적색 X 차량) 는 헬기 구간 전에 파괴된다. 옵션: 그대로 (보통 먼저 파괴됨) / btank 를 지프 정지점과 안 겹치게 이동. 정하지 않으면 두 차량이 겹쳐 그려질 수 있음을 알고 진행 |
| J6 | **HAIKU** | `Test/StageIntegrationTest.cpp:773, 811` 주석 | 주석의 triggerX 5480 / 5600 이 현재 값 (5357 / 5515) 과 다름. 주석만 갱신 |

선택 (신뢰도 낮음, 하지 않아도 됨): 진입 끝 감속 (V1 은 마지막 0.3 s 감속) / speed 170.

### 1.4 수락 테스트

단위 (`Test/VehicleBehaviorTest.cpp`, 기존 VehicleTestUtil 사용):
- `VehicleJeep.EntersFromOffscreenLeftWithoutIdling`: 뷰 min - 70 에 우향 스폰, 플레이어 뷰 가운데. 활성 후 2 틱 안에 vx = +200; `view.min + edgeInset ± targetTol` 에서 멈춤; Turn 후 facing = -1; 첫 로켓 vx > 0; 그동안 지프 x < 플레이어 x.
- `VehicleJeep.FirstRocketWaitsTheAimHold`: 정지 -> 첫 로켓 = 0.96 + 1.0 + 0.96 + firstWait 1.5 + 0.96 + 0.08 = 5.46 s (± 2 틱). 원작 5.0 / 5.75 s 사이.
- `VehicleJeep.InViewSpawnKeepsTheLegacyIdle`: 뷰 안 스폰은 기존처럼 1 s 대기 후 플레이어 쪽 가장자리로 (기존 두 테스트가 그대로 통과하면 충분).
- `StageIntegration.HeliSpawns...` 보강: river_jeep 스폰 틱에 `jeep.x + 60 < cam.x` (보이지 않음), 첫 `enemy.shot jeep` 의 vx > 0 이고 x < 플레이어 x.
- `StageDataTest` 스폰 수 197 유지 (카메라 앵커로 바꿔도 개수 동일).

영상 (스코어링 하네스, `Scripts/VideoScore/scenes.json` 에 장면 추가 = 구현 담당이 할 일):
- 장면 `river_jeep`: ref V2 anchor 156.40 (앞코 등장), window [-0.5, 7]. events: 정지 +0.35 s, Turn 끝 +1.55 s, Shot_Ready 시작 +5.05 s, 로켓 생성 +6.1 s. 리메이크 런 = stage0 `--warp-x 4990` + 위 walk135 리플레이 (오른쪽 135 틱 후 정지), `--event-log` (로켓 = `enemy.shot` def jeep 로 log 앵커). remake anchor = `appear` 템플릿 `Jeep_Move_Right:0`. track = 정지 후 지프 화면 x: 목표 55 ± 15 px. 통과 기준 기존 0.75.
- 수동 확인: 캡처 시트에서 지프가 뷰 왼쪽 밖에서 달려 들어와 왼쪽 1/6 안에서 멈추고 오른쪽으로 쏘는지.

### 1.5 위험
- J1 의 "뷰 밖" 판정은 스폰 순간의 카메라 (락 글라이드 1.0 s 중일 수 있음: heli_river 5357 -> river_mid 5515 는 걷기 0.63 s) 를 쓴다. 목표 x 는 매 틱 현재 뷰로 다시 계산 (TargetX_) 하므로 글라이드가 끝나면 정지점도 따라간다.
- 지프는 activation `marginPx 150` 이라 dx -70 에서도 즉시 활성. 다른 곳에 화면 안 지프를 두면 기존 동작 (1 s 대기) 이 그대로라 혼동 없음.
- J5 겹침.

---

## V2. BTank (Raw/108017.png): "항상 좌향 -> 플레이어 위치에 따라 방향 전환"

### 2.1 원작 증거
- 시트 `Raw/108017.png` 의 첫 줄 제목이 **"TURNING (UNUSED UNTIL MS4)"**: MS2 에서 이 전차 (Girida-O 계열, 차체 "07") 는 **돌지 않는다**. MS2 원작은 항상 좌향이다.
- TURNING 6 프레임은 차체가 아니라 **포탑만** 도는 그림: 프레임 0 = 포신이 오른쪽 위, 2~3 = 정면 (위), 5 = 왼쪽 위 (고각). 차체 (궤도) 는 6 프레임 모두 왼쪽을 향한 같은 그림. 이미지 `ref/Vehicles_108017_turning.jpg`.
- 미션 4 영상의 강 구간 전차 (V1 2:48~2:58, V2 2:20~2:29) 는 이 위장 전차가 아니라 **키 큰 적색 X 장갑차** (우측에서 물을 가르며 접근, 병사 탑승) 다. `ref/Vehicles_V2_redX_vehicle.jpg`. Stage0 의 btank (5400) 는 이 차량의 대역 (Stage0.json:151 주석). 사막 구간 1 fps 스캔에서도 위장 "07" 전차는 식별되지 않았다 (해상도 한계로 확정은 아님).
- 결론: **방향 전환은 원작 패리티가 아니라 사용자 설계 요청**이다. 영상 점수로는 검증할 수 없고, 단위 테스트 + 리메이크 캡처로 수락한다.

### 2.2 현재 동작
- `VehicleBTank.cpp:51` `_e.facing = -1` 고정. 상태 Closed -> Open (BTank_Animing_Up 8f x .12) -> Idle 3 s -> Fire x2 (fireFrame 2).
- 활성 조건 `player.feet.x >= x - activateDx` (VehicleBTank.cpp:65): 플레이어가 오른쪽이면 항상 참.
- 렌더: `EnemyWorld.cpp:714-716` 이 `SpriteDrawOptions` 를 만들지만 **bFlipX 를 쓰지 않는다**. 방향은 클립 이름 `{side}` 로만 바뀌는데 BTank 클립엔 `{side}` 가 없다 -> 지금은 facing 을 바꿔도 그림은 그대로 좌향.
- 이미 facing 으로 미러되는 것: 머즐 x (`GetMuzzleWorld`, EnemyCommon.cpp:270), 포탄 vx (`v0.x * facing`, EnemyCommon.cpp:378), 사망/머즐 FX 위치 오프셋 (`RequestFx`, EnemyCommon.cpp:249). 히트박스/바디는 cx 0 이라 대칭 (미러 불필요).
- 미러 안 되는 것: **FX 스프라이트 자체** (`EnemyFxRequest` / `Effect` 에 flip 필드 없음, EnemyBehaviors.h:16, Effects.h:18). `BTank_Shot_Effect` 는 pivotX 148 / w 70 이라 원점 왼쪽 148 px 에 그려진다 -> 우향이면 포연이 전차 **뒤 (왼쪽)** 에 뜬다.
- 리메이크 캡처 증거 (`ref/Vehicles_remake_events.jsonl`): btank 발사 틱 252, 305, 538, 591, 824, 877, ... 모두 `x 5325.5, vx -400`. 플레이어는 오른쪽 (x 약 5552), 락 뷰 min 5357 -> **머즐이 화면 왼쪽 밖 31 px 에서 왼쪽으로** 쏜다. 이 전차는 플레이어를 절대 위협하지 않는다.
- 기존 BTank 클립은 이미 108017 그림을 쓴다 (Idle 1, Animing_Up 8, Fire 11, Destory 8, Shot_Effect 16 확인). 빠진 것은 TURNING 6 프레임뿐.

### 2.3 최소 구현 (1 단계: 미러만, 새 그림 없음)

| # | 라벨 | 파일 | 내용 |
|---|---|---|---|
| B1 | **SONNET** | `VehicleBTank.cpp` | 상태 `Turn` 추가. Idle 상태 (Open 끝난 뒤, Fire 중엔 안 함: 2연발은 끝까지) 에서 `sign(dx) != facing` 이고 `abs(dx) > turnHysteresisPx` 가 `turnDelayTicks` 동안 연속이면 Turn. Turn = `BTank_Idle` (닫힌 포신) 을 `turnTicks` 동안 보여 준 뒤 `facing = -facing` -> Open (Animing_Up 재생 = 재조준) -> Idle 에 `stateTicks = max(0, idleTicks - refireTicks)` 로 들어가 재조준 후 `refireSec` 뒤 발사. 첫 활성 때 (Closed -> Open 전) 플레이어 쪽으로 즉시 facing 을 정한다 (Turn 연출 없이). 지연 카운터는 `aux`, 상태는 기존 `EnterState` 사용 |
| B2 | **SONNET** | `EnemyDef.h` (+ `EnemyDefLoader.cpp` 파싱), `EnemyWorld.cpp:714-727` | def 단위 `"artFacing": "left"` (기본 없음 = 지금처럼 `{side}` 클립). 렌더에서 `options.bFlipX = def.artFacing == Left && e.facing > 0` (본체와 overlay 둘 다). 미러 축 = 발 원점 (히트박스/머즐/FX 오프셋이 이미 발 기준으로 미러되므로 일관). Destory 클립도 본체 경로라 자동 미러 |
| B3 | **SONNET** | `EnemyBehaviors.h` EnemyFxRequest, `EnemyCommon.cpp:237-262` RequestFx, `EnemyWorld.cpp:594-611`, `Source/Game/Sandbox/Effects.h/.cpp` (Effect, Spawn, Submit:91) | FX 미러: `FxDef` 에 `"mirror": true` 를 받아 `request.bFlipX = fx.bMirror && facing > 0` -> `Effect.bFlipX` -> Submit 에서 `SpriteDrawOptions.bFlipX`. 기본 false 라 다른 FX 영향 없음. 계산: 우향 머즐 x+74.5, FX 원점 x+4, flip 시 rect.x = 원점 + pivotX - w = x+82 -> 포구 오른쪽에 뜬다 |
| B4 | **HAIKU** | `Assets/Defs/Vehicles.json` btank | `"artFacing": "left"`, muzzleFx 에 `"mirror": true`, params 에 `"turnDelaySec": 0.5, "turnHysteresisPx": 40, "turnSec": 0.4, "refireSec": 1.0`. note 문구 "고정 포탑" -> "플레이어 쪽으로 방향 전환 (원작 MS2 는 고정, 사용자 요청)". 포탄 `v0 [400,-200]` / muzzle `[74.5,-95]` 는 이미 facing 미러라 변경 없음 |
| B5 | **HAIKU** | `Docs/EnemyDefsSchema.md` | `artFacing`, FxDef `mirror`, BTank 새 params 4 개 문서화 |

미러 부작용: 차체 궤도 중심이 발 원점보다 +10 px (2x) 오른쪽 (BTank_Idle 188x90, pivotX 95, 궤도 x 28..182 -> 중심 105) 이라 방향 전환 순간 차체가 20 px 옮겨 보인다. Turn 연출 (닫힌 포신 0.4 s) 에 묻히므로 1 단계는 허용. 거슬리면 `"mirrorAxisDx": 10` (그림만 원점 + 2x10 에 그림, 머즐/FX 도 같은 축) 을 나중에.

### 2.4 선택 (2 단계: TURNING 6 프레임 사용) - 사용자 승인 필요
`sprites.json` 수정과 새 텍스처가 필요하다 (이 문서 작성 규칙상 손대지 않음). anim.json 은 건드리지 않는 추가 전용 경로:
1. **HAIKU (스크립트)**: `Raw/108017.png` 에서 TURNING 6 칸 추출. 1x 사각형 (x, y, w, h), 배경 RGB (16,216,128) -> 알파 0:
   `(10,33,80,63) (97,33,80,63) (184,33,80,63) (271,33,80,63) (358,33,80,63) (445,33,80,63)` (각 프레임 실제 bbox 높이 59~63, 바닥 y=96 공통, 궤도 x 0..77 공통).
   2x nearest 확대 -> 새 파일 `Assets/Textures/BTank_TurnStrip.png` (960x126, 칸 160x126). 프레임 순서: 0 = 포탑 오른쪽 위, 5 = 포탑 왼쪽 위.
2. **HAIKU**: `sprites.json` 끝에 항목 하나 추가 (기존 바이트 불변, Boss_CasingStrip 과 같은 형식):
   `"BTank_TurnStrip": { "file": "Textures/BTank_TurnStrip.png", "w": 960, "h": 126, "columns": 6, "rows": 1, "cellW": 160, "cellH": 126, "col": 0, "line": 0 }`
3. **SONNET**: BTank 에 `submitParts` 추가해 Turn 상태 동안만 `SpriteLibrary` 스탠드얼론 칸을 `SubmitStandalone` 으로 그린다 (BigShiee 탄피 경로와 같음). 정렬: 칸 왼쪽 = 발 x - 67 (BTank_Idle 궤도 왼쪽과 맞춤), 칸 바닥 = 발 y (Idle 의 GetFootBaseline 77 과 같은 바닥) -> 칸 중심 = (발 x - 67 + 80, 발 y - 63). 좌->우: 미러 없이 5 -> 0 재생 후 facing +1 (미러) 로 Open. 우->좌: 미러 상태로 5 -> 0 재생 후 facing -1. 칸당 0.08 s (turnSec = 0.48). 본체 클립은 그 동안 그리지 않는다 (bodyFrame 또는 clip -1).
4. 끝 순간 차체 미러 팝 (20 px + 차체 좌우 반전) 은 남는다. 그래서 1 단계만으로도 충분하다고 본다.

### 2.5 수락 테스트
단위 (`Test/VehicleBehaviorTest.cpp`):
- `VehicleBTank.FacesAPlayerOnItsRightWhenItActivates`: 플레이어 x+300 -> 첫 Open 전에 facing +1, 첫 포탄 `vx = +400`, 머즐 x = x + 74.5.
- `VehicleBTank.TurnsAfterTheDelayWhenThePlayerCrosses`: 플레이어가 좌 -> 우로 넘어가면 `turnDelay - 1` 틱엔 facing -1, `turnDelay + turnSec + 2` 틱 안에 +1, 그 뒤 `0.96 + refireSec` 안에 vx > 0 포탄.
- `VehicleBTank.HysteresisPreventsJitter`: 플레이어가 x ± 30 을 매 20 틱 오가면 Turn 0 회.
- `VehicleBTank.DoesNotTurnMidVolley`: Fire 중 플레이어가 넘어가도 2발은 같은 방향, 그 후 Turn.
- `VehicleBTank.MuzzleFxMirrorsWithFacing`: 우향 발사 시 FX 요청 위치 x+4, bFlipX = true; 좌향은 false (기존 위치 x-4).
- 렌더 플래그: `ShouldFlipArt(def, e)` 같은 순수 함수로 빼서 테스트 (artFacing 없는 def 는 항상 false).
- 기존 `ShellFliesLeftwardAsALob`, `OpensThenFiresTwoShells...` 는 플레이어를 왼쪽에 두면 그대로 통과해야 한다 (테스트 셋업의 플레이어 위치 확인 필요).
리메이크 캡처 (영상 비교 대신):
- 위와 같은 walk135 캡처에서 `enemy.shot btank` 가 플레이어가 오른쪽일 때 `vx > 0` 이고 머즐 x >= 뷰 min. 캡처 시트에서 우향 전차 + 포연이 포구 오른쪽.

### 2.6 위험
- 원작 MS2 와 다름 (의도된 차이). Spec/EnemySpecs 2.3 "고정 포탑" 문구와 충돌 -> 노트 갱신 (B4/B5).
- EffectList 는 공용: flip 필드 기본 false 유지 필수.
- 활성 조건이 한쪽뿐 (`player.x >= x - 500`). 오른쪽 멀리 (예: 1000 px) 있어도 활성. 필요하면 `abs(dx) <= activateDx` 로 (기존 테스트 `StaysClosedUntil...` 확인).
- 2 단계는 sprites.json + 새 텍스처 = 사용자 승인 필요.

---

## 3. 작업 순서
1. J1 + J2 (SONNET) -> J3 (HAIKU) -> 지프 단위 테스트.
2. J4 Stage 스폰을 카메라 앵커로 (사용자 승인 후 HAIKU), J5 결정, J6 주석.
3. B2 + B3 (SONNET, 렌더/FX 미러 공용 기반) -> B1 (SONNET) -> B4/B5 (HAIKU) -> BTank 단위 테스트.
4. 캡처 재확인 (walk135 런) + `river_jeep` 점수 장면 추가.
5. (선택) TURNING 스트립 2 단계.

## 4. 재현 메모
- 리플레이: `SREP` v1, seed u64, 상태 이름, 프레임 수, 프레임마다 u16 held (bit1 = Right). 스크래치 `walk135.rep` = 135 틱 Right 후 정지, 1800 틱.
- 실행: `SDL_VIDEO_DRIVER=offscreen SDL_RENDER_DRIVER=software Output/Release/MetalSlug2_Clone.exe --replay walk135.rep --warp-x 4990 --life 99 --no-audio --no-async-load --event-log ev.jsonl --capture f --capture-ticks 0,5,...`
- `--warp-x` 는 목표 x 앞 구간을 조용히 적용 (스폰 생략) 하므로 river_start (5000) 앞인 4990 으로 워프해야 btank/헬기 락이 실제로 발동한다. `--hold Right` 는 오프스크린에서 이동하지 않았다.
- `enemy.shot` 은 플레이어 사망 중엔 기록되지 않는다 (FireWeapon 이 쏘지 않음).
