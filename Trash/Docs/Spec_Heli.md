# 헬기 (R-쇼부 공격헬기) 설계

코드: `Source/Game/Enemies/Heli.cpp`, `HeliBomb.cpp`, 설정 구조체 `HeliCfg`/`HeliBombCfg` (`EnemyDef.h`), 정의 `Assets/Defs/Helicopter.json`, 테스트 `Test/HeliTest.cpp`.
원작 근거: V1 3:03-3:14 (`Docs/VideoTimeline.md` D절), V2 2:35-2:41. 좌표는 기체 중심, 카메라 기준 px.

## 패턴 (사용자 지정 + 원작)
한 사이클 = 고공 폭탄 -> 저공 폭탄 -> 미사일 -> 고공 복귀, 반복.

| 단계 | 상태 | 동작 | 파라미터 |
|---|---|---|---|
| A 고공 | `Bomb` | 우상단에서 진입해 카메라 윗변 + `hoverDy` 높이로 내려와 플레이어 x (+`followDx`) 를 천천히 따라간다. 폭탄 `burstCount`(3)발을 `bombIntervalTicks`(24틱=0.4 s) 간격으로 수직 투하, `burstRestTicks`(84틱=1.4 s) 휴지, 이를 `highBursts`(2)회 | 첫 사이클 첫 폭탄 `firstBombSec`(1.6), 이후 `reBombSec`(0.4) |
| B 저공 | `Low` | 화면 중앙 반대쪽 가장자리(`lowMargin` 안쪽)로 `lowSpeed`(90 px/s) 로 가로지르며 `lowDy`(200) 높이까지 `descendSpeedY` 로 내려온다. 같은 3발 + 휴지를 `lowBursts`(2)회. 진행 방향이 기수 방향이면 `Heli_Pitch` (앞으로 숙임) | |
| C 미사일 | `PodOut`/`Move`/`Volley` | 포드 전개 -> 기수 반대쪽 화면 가장자리 아래로 1.17 s 이동 -> 발사 연기 -> 미사일 2발 (2.1 s 간격, 350 px/s 직진, 연기 꼬리) | `missiles`, `missileIntervalSec` 등 |
| D 복귀 | `MoveBack` | 포드를 접고 고공으로 복귀, 사이클 +1, A 로 | |
| (보조) 선회 | `Turn` | A 중 플레이어가 반대쪽에 `turnWaitSec` 이상 머물면 선회 (원작 미관측: 가정) | |

폭탄은 비행 중 플레이어 총에 맞으면 공중에서 터지고, 지면/발판에 닿으면 터진다. 착지 시 `Heli_Ground` 먼지가 레이어 57 에 깔리고 `Explosion_Mid` 가 레이어 58 에 나온다.

## 격추
체력 150. 격추 시 즉시 `Explosion_Large` (흰 원 포함) + 0.12/0.22/0.34 s 에 `Explosion_Mid` 3개 + 0.3 s `Boss_Smoke_Big`, 레이어 57/58. 원작도 추락 없이 우상단에서 대폭발 (V1 3:14.25) 이라 기체는 즉시 사라진다.

## 히트박스
`hurtbox` 176x96 (중심 오프셋 y +4). 이전엔 body 150x80 을 썼다. 스프라이트(Heli_Fly)는 약 170x90.

## 스테이지 스폰 규약
- 위치: 카메라 우상단 밖 `cam.x + 507, cam.y - 30` (화면 오른쪽 끝에서 133 px 안쪽, 윗변 위 30 px). Stage JSON 의 `camera` 앵커로는 `{"ax":"right","dx":133,"ay":"top","dy":-30}`.
- 활성: `activation.mode = always` (스폰 즉시 동작), 방향 `facing = -1` (왼쪽).
- 저공부터 시작: def 의 `params.startLow = 1` 로 별도 def (`"extends":"heli"`) 를 만들면 된다.
- 락: 강 구간은 카메라를 `[5480,288,6280,736]` (폭 800) 으로 고정한 뒤 스폰, 격파 후 해제 (게이트 `heli_dead`).

## 랩/검증
- `--state helilab` / `helilab_god` (무적), `helilab_bot` (무적 + 봇: 120틱마다 좌우 이동, 위로 사격), `helilab_kill` (봇 + 헬기 hp 40 -> 격추 연출).
- 캡처: `MyMetalSlug2.exe --state helilab_god --no-audio --capture <prefix> --capture-ticks 120,300,540,900`.
- 사운드: `sounds.json` 에 폭발/폭탄/미사일 사운드가 없고 `SoundMap.md` 에서도 원본 WAV 매칭이 0건이라 연결하지 못했다.
