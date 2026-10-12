# C6 하드코딩 수치 감사

생성: `Scripts/AuditCoords.ps1` (약 2초, 종료코드 1 = 미허용 있음). 허용 목록: `Scripts/AuditCoords.allow.txt`. 상세: `Docs/Wayfinder/C6_audit.csv` (미허용 287행).

## 1. 합계

- 스캔: Source/Game 의 .cpp/.h 97개 (Ui 제외). 매치 346건 = 허용 59건 + 미허용 287건 (48개 파일).
- 미허용 종류별: other 116, position 66, time 26, count 26, size 24, speed 18, offset 11.

## 2. 디렉터리별 (미허용)

| 영역 | 건수 | 주 종류 |
|---|---|---|
| Enemies | 91 | other 36, position 22, count 13 |
| Source/Game 루트 (SandboxState, StateFactory 등) | 62 | position 30, other 23 |
| Player | 48 | other 25, size 7, time 5 |
| Props | 31 | other 12, time 9, speed 4 |
| Stage | 24 | other 15, size 4 |
| Sandbox | 16 | speed 6, other 5 |
| Combat | 15 | time 5, size 5 |

## 3. 상위 파일 15

SandboxState.cpp 43, EnemyDef.h 26, BigShieeCfg.h 17, SlugVehicle.cpp 16, SlugTypes.h 13, BigShiee.cpp 12, Bus.cpp 10, StateFactory.cpp 10, ItemWorld.h 9, StageState.cpp 9, PlayerController.cpp 8, ItemWorld.cpp 8, EnemyWorld.cpp 7, Effects.h 7, PropWorld.h 6

## 4. 기존 Defs 위치 계열 키 (Assets/Defs/**, Assets/Tuning/Tuning.json)

키 이름에 offset|muzzle|anchor|origin|pos|hotspot|spawn|turret 를 포함하는 키 41종. 출현 수는 JSON 안에서 그 키가 나온 횟수 (def 객체 수 근사).

| 키 | 출현 | 파일 |
|---|---|---|
| offset | 125 | BigShiee, Enemies, Helicopter, Props, Vehicles |
| muzzle | 17 | BigShiee, Enemies, Helicopter, Tuning, Vehicles |
| muzzleFx | 9 | Enemies, Vehicles |
| muzzleBase | 2 | Enemies |
| muzzleDist | 2 | Enemies |
| muzzleJitter | 2 | Tuning |
| centerAnchor | 2 | Helicopter |
| noDespawn | 2 | BigShiee, Bus (spawn 부분일치 오탐) |
| 기타 단발 (각 1회) | 33 | turretAnchor/Side/Twin/Hp/Box/Muzzle/Score, muzzleGap, cannonAnchor, cannonBoxOffset, missileMuzzle, cannonMuzzleX, wreckOffset, cannonWreckOffset, soldierSpawn, despawnBehindCameraPx, despawnBelowCameraPx, postureDeg, paraDestroyOffset, itemOffsetX, bombMuzzle, puffOffset, podOffset, rotorOffset, spawns, guard_post, muzzleAdjust, releaseOffset, respawnInvincible, respawnOffsetX, respawnDropY, chuteOffset, dummyRespawnTicks |

요약: 위치 필드 스키마는 이미 `[x, y]` 배열 형태로 존재한다 (offset/muzzle 계열). 현재 CSV 의 position/offset 항목 77건 중 상당수는 이 키들로 옮길 수 있다. 단, 키 명명이 BigShiee 에서 cannon*/turret* 로 나뉘어 있어 일관성 정리가 필요하다.

## 5. 설계 결정 필요 (공유되거나 계산되는 값)

1. 탄/폭발 상자 `size`, `blastSize`, `hitSize` 가 EnemyDef.h, Projectiles.h, SlugTypes.h 에 같은 의미로 반복된다. 공용 projectile 스키마로 묶을지.
2. muzzle/puffOffset/podOffset 등은 x 가 바라보는 방향에 곱해진다 (미러 규칙). 스키마에 명시할지, 음수 x 를 허용할지.
3. BigShieeCfg.h 의 cannonAimDeg/cannonMuzzleX 표와 SlugClips.h 의 kVulcanAngleStep (= 360/32) 은 행 수 상수에서 파생되는 값이다. 배열로 뺄지 식으로 둘지.
4. 속도 단위 혼재: `350.f / 60.f`, `1000.f / 60.f` 처럼 px/s 를 틱으로 환산한 값이 코드에 흩어져 있다. 데이터는 px/s 로 통일할지.
5. SlugTypes.h 의 standSize/hurtStand 등 플레이어 상자 값이 스프라이트 프레임 크기와 연결되는지 확인이 필요하다 (자동 파생 vs 수동 값).
6. SandboxState.cpp 43건은 디버그 샌드박스 전용 좌표다. 튜닝 대상에서 제외할지.
7. kEnemyLayer, kSlugBodyLayer 등 렌더 레이어 번호와 aux 슬롯 인덱스는 데이터가 아니다. 허용 목록 처리할지.
8. Bus.cpp 의 kPxPerSheetPx, kAnchorU/V 는 시트 좌표 변환이다. 아트 변환 상수이므로 데이터 이동 대상인지.
9. kind 와 suggestedJsonPath 는 심볼명 기반 추정이다. World_, Submit 등 함수 인자 리터럴은 심볼이 함수명이라 사람이 이름을 다시 붙여야 한다.
