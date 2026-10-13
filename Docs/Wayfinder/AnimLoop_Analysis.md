# anim.json 좌/우 loop 불일치 4쌍 조사 (읽기 전용)

조사일 2026-10-13. Source/Assets 수정 없음, 빌드 없음.

## 결론 (맨 위)

**안전하게 통일 가능하다. 값은 `loop: false` (4쌍 모두, `_Left` 4개를 false 로).**

- 게임의 판정 기준은 `Assets/Defs/Vehicles.json` 의 클립 정의(`loop` 키 없음 = `ClipDef.bLoop=false`, `EnemyDefLoader.cpp:446` / `EnemyDef.h:65`) 이고, 상태 전이(`IsClipDone`/`ClipFinished` -> `ClipDef::IsFinished`, `EnemyDef.h:78-82`)는 anim.json 의 loop 를 전혀 읽지 않는다.
- 4개 클립 모두 Defs 쪽은 비루프, `_Right` 도 이미 false. `_Left` 만 true 로 어긋나 있다 (손 편집 실수로 판단되는 단일 방향 오류).
- 시뮬레이션 해시(리플레이 15개)는 영향 없음. 화면(렌더)만 바뀌며, 실제로 시각이 달라지는 것은 `MissleTank_Up_Destroy_Left` 하나뿐이다 (2.0 s 사망 구간 중 1.6 s 이후 첫 프레임들이 되감겨 재생되는 현상이 사라짐).
- anim.json 은 사용자 손 편집본이므로(git status `M Assets/anim.json`), 복원이 아니라 해당 4개 항목의 `"loop": true` -> `false` 만 국소 수정할 것.

## 1. 재생 경로 추적

| 용도 | 위치 | loop 출처 |
|---|---|---|
| 적/차량 로직(상태 전이, 프레임 도달, 추가 히트박스 프레임) | `EnemyCommon.cpp:111` `ClipFinished`, `EnemyDef.h:78` `IsFinished = !bLoop && ticks >= duration`, `EnemyWorld.cpp:332` | **Defs `ClipDef.bLoop`** (anim.json 아님) |
| 적/차량 렌더 프레임 선택 | `EnemyWorld.cpp:17-34` `FrameAt_` (`_clip.bLoop`), 호출 `EnemyWorld.cpp:680,704,751` | **anim.json `AnimClip.bLoop`** (`Render/Animation.cpp:65-70` 에서 로드) |
| 클립 이름 생성 | `EnemyCommon.cpp:278-290` `ResolveSide`: facing<0 -> `Left`, 그 외 `Right`. 바인딩 `EnemyWorld.cpp:618` | - |
| 해시 | `EnemyWorld.cpp:1090-1117` `HashState`: `state/stateTicks/clipTicks/clip/facing/위치` 등. 렌더 프레임 인덱스·anim loop 미포함. `Headless.cpp:145-155` 는 `DebugHash()` 만 섞음 | - |

즉 anim.json 의 loop 는 이 4개 클립에서 "표시 프레임 인덱스를 wrap 할지 마지막 프레임에 고정할지" 만 결정한다.

## 2. 클립별 판정표

모든 클립: frames/frameDuration/loopStart/cellW/cellH 는 좌우 동일 (Jeep 3종 12프레임 0.08 s, MissleTank_Up_Destroy 20프레임 0.08 s, loopStart 0).

| 클립 | _Left loop | _Right loop | 게임이 기대하는 값 | 근거 | 통일 방향 | 통일 시 재생 변화 / 해시 |
|---|---|---|---|---|---|---|
| `Jeep_Turn` | true | false | **false** (한 번 재생 후 종료 대기) | Defs `Vehicles.json:34` turn (loop 키 없음). `VehicleJeep.cpp:152-158` `Turn`: `IsClipDone` 이면 facing 반전 후 Idle. `:127-133` `TurnToMove`, `:226-232` `TurnAfterShot` 도 동일 대기 | `_Left` -> false | 변화 없음. 상태가 clipTicks==duration 틱에 `Go_`->`SetClip(...,true)` 로 clipTicks=0 이 되므로 렌더가 index>=12 를 볼 일이 없음. 해시 불변 |
| `Jeep_Shot_Ready` | true | false | **false** | `Vehicles.json:37`. `VehicleJeep.cpp:190-196` `ShotReady`: `IsClipDone` 후 `Shot` 으로 | `_Left` -> false | 변화 없음 (위와 동일 논리). 해시 불변 |
| `Jeep_ReAimming` | true | false | **false** | `Vehicles.json:39`. `VehicleJeep.cpp:218-224` `ReAim`: `IsClipDone` 후 `TurnAfterShot` | `_Left` -> false | 변화 없음. 해시 불변 |
| `MissleTank_Up_Destroy` | true | false | **false** (사망 연출 1회, 마지막 프레임 고정) | `Vehicles.json:163` destroyUp (loop 키 없음, 반면 같은 파일의 Idle/Ready 는 `"loop": true` 를 명시 = 의도적 구분). `VehicleMissleTank.cpp:147` `BeginVehicleDeath(...clipDestroyUp)`, `VehicleCommon.cpp:6-36`. `removeAtSec 2.0` (`Vehicles.json` death) > 클립 길이 20x0.08=1.6 s | `_Left` -> false | **렌더 변화 있음**: 현재 Left 는 1.6~2.0 s 구간에서 프레임 0.. 로 되감겨 재생됨(버그성). false 면 Right/`MissleTank_Down_Destroy_*`(21프레임, 둘 다 false)처럼 마지막 프레임 고정. 로직·해시 불변 |

`_Left` / `_Right` 사용 여부: 선택은 facing 뿐이다. 스테이지 스폰 기본 facing 은 -1(Left) (`StageWorldHost.cpp:446`), `river_jeep` 만 `facing: right` 로 시작해 Turn 후 Left 가 됨. MissleTank 은 facing 을 바꾸지 않고(`VehicleMissleTank.cpp` 에 facing 변경 없음) 스폰 facing 그대로이므로 Stage0 의 `missle_tank` 4기(`Stage0.json:224,238,266,284`, facing 미지정 = Left) 와 vehiclelab (`SandboxState.cpp:227`, `left.facing=-1`)는 **Left 변형이 실제로 쓰이는 쪽**이다. Right 는 `facing: right` 지정 시에만.

## 3. 다른 필드 어긋남 (AnimIntegrityTest 규칙 재현)

규칙(`Test/AnimIntegrityTest.cpp` `LeftRightTwinsKeepTheSameFootAnchor`): `_Left`/`_Right` 쌍에서 (비어있지 않은 프레임의) `max(h - pivotY)` 차이 <= 64 이고 `bLoop` 동일. 쌍 209개 모두 검사.

- 앵커 차이 최대 14 (`Heli_Pod_Left`), 다음 9 (`Player_Stand_Lookingup_4_SG_Left`), 6 (`MissleTank_Up_Ready_Shot_Left`). 64 초과 없음 -> 앵커 규칙은 전부 통과.
- loop 불일치: 위 4쌍만. (209쌍 중 205쌍 일치)
- 테스트가 검사하지 않는 다른 필드(참고용, 테스트 실패 원인 아님):
  - `Player_Crouch_Throw_SG`: frameDuration Left 0.035 / Right 0.03
  - `Player_Stand_Lookingup_3_SG`, `Player_Stand_Lookingup_4_SG`: cellH 가 좌우 다름 (둘 다 1프레임, loop true, frameDuration 999)
  - 프레임 수는 209쌍 모두 동일.

## 4. 같은 종류의 추가 불일치 (테스트에는 안 걸림)

Defs 가 비루프인데 anim.json 이 true 인 클립이 4개 더 있다 (Defs vs anim.json 324개 참조 중 9건 불일치):
`Jeep_Aimming_{Left,Right}`, `Jeep_Shot_{Left,Right}` (좌우 모두 true 라 테스트 통과), `BTank_Idle` (1프레임이라 무의미). Jeep_Aimming/Jeep_Shot 도 `IsClipDone` 에서 상태가 바뀌므로 화면 변화는 없고 데이터 일관성 문제일 뿐이다. 이번 요청 범위(4쌍) 밖이므로 변경하지 않았고, 원하면 같이 false 로 정리 가능.

## 5. 리플레이/캡처 해시 영향

- 리플레이 해시(`Baseline.txt` 15개, `Test/Replays` 16개 파일)는 `EnemyWorld::HashState` 등 시뮬레이션 상태만 섞는다. anim.json loop 는 (a) 적 로직의 종료 판정에 쓰이지 않고(Defs 사용), (b) 해시 항목에 렌더 프레임이 없다. 따라서 **`loop` 변경으로 해시는 바뀌지 않는다**. (다른 Animator 계열 소비처 - Effects/ItemWorld/Decor 등 - 는 이 4개 클립 이름을 쓰지 않음: 이름은 `Vehicles.json` 에서만 참조됨.)
- 이 클립이 등장하는 상태: jeep 은 `sluglab`, `vehiclelab`(SandboxState.cpp:217,225), stage0(`river_jeep`); missle_tank 은 `vehiclelab`, stage0. `enemylab/buslab/bosslab/helilab` 에는 등장 근거 없음(grep 상 spawn 없음). 등장하는 경우에도 위 이유로 해시 불변.
- 화면 캡처(픽셀) 해시가 있다면 변할 수 있는 경우: **Left 방향 MissleTank 이 Up 모드로 파괴된 뒤 1.6 s 이후의 프레임**을 담은 캡처뿐. Jeep 3종은 픽셀도 불변. 이번 작업트리에는 `Captures/` 디렉터리가 없어 캡처 해시 파일은 확인하지 못했다.
- 참고: `VehicleDefTest.NonLoopClipTimingsMatchTheAnimations` (`Test/VehicleDefTest.cpp:262`)는 프레임 수/프레임 길이만 비교하므로 loop 변경과 무관. 변경 후 `AnimIntegrity.LeftRightTwinsKeepTheSameFootAnchor` 가 통과해야 한다 (loop 불일치가 유일한 실패 원인).
