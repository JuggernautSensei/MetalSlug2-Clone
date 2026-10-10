# 소품 / 아이템 정의 (`Assets/Defs/Props/Props.json`)

구현: `Source/Game/Props/` (`PropDef.*` 정의 로더, `PropWorld.*` 소품, `ItemWorld.*` 아이템/점수 팝업).
수치 근거는 `Docs/EnemySpecs.md` §4~7 (파괴 구조물, 아이템, 디브리). 스테이지 사용처는 `Docs/StageFormat.md` §7/§9.
**파일 위치는 `Assets/Defs/Props/` 하위 폴더다.** `EnemyDefLibrary::LoadDirectory` 는 `Assets/Defs/*.json` 을 전부 적 스키마로 읽으므로, 최상위에 두면 `VehicleDefs`/`EnemySandbox` 테스트가 깨진다 (실제로 깨졌던 이력이 있어 옮겼다). 하위 폴더는 그 로더가 건너뛴다.

## 0. 공통 규약
- 좌표: **스프라이트 중심(legacyCenter)** 기준. 모든 `offset` 은 중심에서의 오프셋(+y 아래). `size` = 상자 폭/높이.
- HP: 권총 damage(3) 기준 원작 체감치. 상점 12(4발), 트럭 9(3발), 보트 12(4발), 집류 18(6발), 지상집/큰집/탑 24(8발). 폭탄(20) 은 1~2발. 임계값(`damagedAtHp`) 은 HP 의 절반 (지상집/큰집 12).
- JSON 은 엄격: 알 수 없는 키/값, 잘못된 형식, 없는 def/item/group 참조는 로드 실패 (`eError::PropDefInvalid`, 메시지 `"<def>: <필드>: 사유"`). `note` 키는 어디서나 허용된다.
- 정의 이름/클립/스프라이트 이름은 `Assets/Baked/{anim,sprites}.json` 키와 일치해야 한다 (`PropDefLibrary::FindMissingAsset` 으로 검사).

## 1. 최상위
| 키 | 설명 |
|---|---|
| `version` | `1` 만 지원 |
| `defs` | `{ name: PropDef }` 소품 정의 |
| `items` | `{ name: ItemDef }` 습득 아이템 |
| `groups` | `{ name: [itemName...] }` 균등 추첨 그룹 (`random_fruit`, `random_anime`, `random_score`) |

## 2. PropDef
| 키 | 타입 | 기본 | 설명 |
|---|---|---|---|
| `hp` | int ≥ 0 | (필수) | 0 = 피격 불가(불사). 피격 상자가 없으면 무관 |
| `cullMargin` | float | 200 | 중심 x 가 `camLeft - margin` 보다 왼쪽이면 제거 |
| `layer` | `"background"`/`"front"` | background | 렌더 레이어 |
| `idle` / `damaged` / `destroyed` / `hit` | Visual | 없음 | 상태별 표시. `hit` 은 피격 때마다 한 번 재생 |
| `damagedAtHp` | int | -1 | `0 < hp ≤ 값` 이면 `damaged` 표시 + `onDamaged` 1회 |
| `parts` | Part[] | [] | 충돌/피격 상자 (§2.1) |
| `variants` | Variant[] | 없음 | 스폰 시 균등 랜덤(또는 `params.variant`) 선택: `{name, dy, idle, hit, parts}`. `dy` = 레거시 FinalInit y 보정 |
| `remove` | bool | false | 파괴 연출 `destroyDuration` 뒤 제거 (잔해 없음) |
| `destroyDuration` | float s | 0.5 | `IsDestroyFinished` 까지 시간 |
| `onDamaged` | Burst | 없음 | 손상 단계 진입 시 1회 |
| `onDestroy` | Burst | 없음 | 파괴 순간 |
| `secondary` | `{afterSec, explosions, debris}` | 없음 | `params.explodeOnDeath=true` 일 때만: `afterSec` 뒤 2차 폭발 후 제거 |
| `drops` | Drop[] | [] | 파괴 시 아이템 (`{item|group, count, spreadX, bounce}`). `bounce`: vy -400, 수평 ±75 |
| `spawns` | `[{def, offset}]` | [] | 파괴 시 다른 정의 생성 (예: `sign_fall`) |
| `fall` | `{gravity, landed, afterLandSec, halfHeight}` | 없음 | 낙하체 (hp 없음). 지면(GroundMap)에 닿으면 `landed`, `afterLandSec` 뒤 `onDestroy` 후 제거 |
| `groundPatch` | `{sprite, size, inset}` | 없음 | 파괴 시 `GroundPatch` 이벤트. 좌상단 = `pos - (size/2 - inset)` (실수면 내림). 마스크 = **발자국** (아래 §2.3) |

### 2.1 Part
`{ role, when?, size, offset }`. `role` = 문자열 또는 배열 (배열이면 같은 상자가 여러 역할을 가진다).
- `hurt`: 피격 상자 (Neutral Hurtbox, Bullet/Melee/Blast 수용).
- `wall`: 고체 (플레이어/적 모두 막힘) → `GetWalls()`.
- `platform`: 원웨이 발판 (플레이어/적) → `GetPlatforms()`.
- `enemyPlatform`: 원웨이 발판, 적만 → `GetEnemyPlatforms()` (집 지붕).
- `when`: `intact`(기본, 파괴 전만) / `destroyed`(파괴 후만) / `always`.

### 2.3 groundPatch 의미 (발자국 지우기)
- 마스크 `Baked/Textures/<sprite>.png` 의 **파란 픽셀(R<80, G<80, B>=200, A>=128) = 파괴 시 공기가 되는 발자국**, 나머지는 무시된다. `ApplyGroundPatch(map, topLeft, footprint)` 는 발자국의 고체 픽셀만 공기로 만들고 **발자국 밖은 절대 건드리지 않는다**.
- 가드 (걸리면 아무것도 바꾸지 않고 `false`, `StageState` 가 `SLUG_LOG_ERROR`): (1) 발자국이 어떤 열의 바닥 (지면 맵에서 그 열의 가장 아래 고체 픽셀) 을 지우면 거부, (2) 발자국 사각형 안 고체 픽셀의 50 % 를 넘게 지우면 거부. 고체가 0 인 마스크는 로드하지 않는다 (무시).
- 베이커 (`Source/Tools/Baker` `BakeGroundPatchMasks_`) 가 만든다. 원본은 레거시 `Stage/map/Stage0_Pixel_0.bmp` 기준 지면:
  - `Stage0_House_5_PixelMap` (큰 집): 레거시 마스크는 '집 몸체를 뺀 지면' 스냅샷이므로 발자국 = 기준 지면 - 스냅샷 (집 몸체 윗부분).
  - `Stage0_House_4_PixelMap` (지상 집): 레거시 마스크는 전부 흰색이라 쓸 수 없다. 발자국 = 도로 윗면(y 1286) 위로 솟은 중심 열의 봉우리 한 덩어리.
- 레거시 `BitBlt(SRCCOPY)` 는 사각형 전체를 덮어써 도로를 뚫었다 (이 버그가 softlock 원인). 새 규칙은 그 대신 발자국만 지운다.

### 2.2 Visual / Burst / Drop
- Visual: `{ "anim": "<클립>" }` 또는 `{ "sprite": "<스프라이트>" }` (정확히 하나).
- Burst: `{ explosions: [{clip, count=1, spread=0, offset=[0,0]}], debris: {set: wood|metal|rock, count} }`. 폭발은 `EffectList` 로, 디브리는 중력 1000/수명 5 s/충돌 없음 조각으로 나온다. 난수 오프셋은 `±spread`.
- 디브리 클립: `Wood1..6`, `Metal1..12`, `Rock1..3`.

## 3. 스폰 params (stage JSON `spawn.params`)
| 키 | 설명 |
|---|---|
| `hp` | 새 HP 로 덮어쓰기 (레거시 값을 넣지 말 것) |
| `sprites` | `[idle, destroyed]` 스프라이트 이름 덮어쓰기 (상점) |
| `hitbox` | `{size, offset}` 첫 `hurt` 파츠 덮어쓰기 |
| `standBox` | `{size, offset}` 첫 `platform` 파츠 덮어쓰기 |
| `explodeOnDeath` | bool. true 면 `secondary` 예약 (MobBoat 첫 배) |
| `variant` | 변종 번호 강제 (없으면 시드 RNG) |

`PropDefLibrary::ParseSpawnParams(defName, paramsJson, out, err)` 가 이 키들만 허용한다.

## 4. ItemDef (`items`)
| 키 | 설명 |
|---|---|
| `kind` | `gun` (무기 교체) / `supply` (보급) / `score` (점수) |
| `weapon` | gun: `hmg` / `shotgun` |
| `supply`, `amount` | supply: `bomb`(+amount) / `bullet`(지금 든 유한 무기의 탄 보충) |
| `score`, `roll` | score: 점수, `roll`: 과일처럼 굴러가다 감속 (수평 -50..49 px/s 시작) |
| `visual` (필수), `begin`, `pickup` | 바닥 상태, 첫 재생(Mando_Begin → Idle), 습득 연출 (0.06 s) |

아이템 물리: 상자 32x32 (발 = 원점), 중력 1400, 수명 15 s (10.5 s 부터 0.2 s 깜빡임), 화면 왼쪽 이탈 제거.
점수 팝업: 0.07 s 마다 빨강/노랑 교대, 1.05 s 뒤 가속 상승(-500 px/s²), 2.8 s 제거.

## 5. 예시
```json
"ground_house": {
  "hp": 40,
  "idle": { "sprite": "Stage0_House_4_Idle" },
  "damaged": { "sprite": "Stage0_House_4_Destroy_0" }, "damagedAtHp": 20,
  "destroyed": { "sprite": "Stage0_House_4_Destroy_1" },
  "parts": [ { "role": "hurt", "size": [220, 220], "offset": [10, 50] } ],
  "onDestroy": { "explosions": [ { "clip": "Explosion_Large", "count": 1, "spread": 100 } ], "debris": { "set": "wood", "count": 5 } },
  "groundPatch": { "sprite": "Stage0_House_4_PixelMap", "size": [1306, 751], "inset": [12, 5] }
}
```

## 6. 알려진 차이 / 해석
- **다리(bridge_*) 는 `wall`** (EnemySpecs §5: WALL+HITBOX). 파괴 후에도 충돌 유지. 원웨이 발판 아님.
- **탑(tower) 발판은 3개** (EnemySpecs: `45-70i`, 꼭대기 y = pos-100). StageFormat §9 의 2개 표기는 옛 요약.
- 원/상자 충돌은 **외접 AABB 로 근사** (`Hurtbox` 는 AABB 전용). 지면집/큰집 원 상자는 판정이 살짝 넓다.
- 지면집/큰집의 발자국 패치는 PropWorld 가 적용하지 않는다. `GroundPatch` 이벤트를 `StageState` 가 받아 `ApplyGroundPatch` 로 활성 지면에 적용한다 (§2.3). 개활지 큰 집 (13542) 은 스테이지에 스폰하지 않지만 그 몸체 봉우리 (x 13397..13701, 윗면 y 1204) 는 지면 bin 에 남아 있다. 보스 아레나 엄폐물로 쓰이며 `BossBot` 밸런스 테스트가 이를 전제하므로 제거하지 않는다 (제거하면 보스 봇 승률이 0 이 된다).
- 무기 보급(`bullet`)은 `PlayerController::PickUpWeapon` 의 무기별 탄 (HMG 200 / SG 30) 을 쓴다. 레거시 '추가 탄' (MG 100 / SG 10) 과 다르다. 정확히 맞추려면 `PlayerController` 에 탄약 추가 메서드가 필요하다.
- MobBoat 드롭의 bounce 는 vy -400 (레거시 -500, 수평 ±75 포함).
- 아이템 깜빡임 구간은 10.5 s 부터 수명 끝까지로 해석.
- **적은 소품 발판(집 지붕 포함)을 아직 밟지 못한다.** 샌드박스는 적 물리에 설정 발판만 넘긴다. 소품 발판을 적 물리에도 넘겨 보았더니 `AudioDeterminism`/`PlayerReplay` 가 접근 위반으로 죽었다 (적 쪽 문맥 사본이 틱마다 다시 채워지는 소품 벡터의 스팬을 들고 있는 것으로 보이나 정확한 경로는 미확인). 스테이지 호스트는 적 물리용 발판을 틱 사이에 안정적인 저장소에 두고, 적 쪽이 스팬을 틱 밖에서 보관하지 않는지 확인해야 한다.
- 스팬(`GetPlatforms`/`GetWalls`)은 마지막 `Tick`/`OnHitEvents` 이후 다음 호출 전까지만 유효하다. 호출자가 스팬을 보관하면 안 된다.
- 소품 이벤트(`TakeEvents`)는 샌드박스에서 버린다 (소비처는 스테이지 호스트).

## 7. 샌드박스 검증 (상태 이름 `proplab`)
`StateFactory` 의 `proplab`: 플레이어 (1500,378), Stage0 상점 거리에 `shop_0..2`, `truck` ×2, 아이템 `gunitem_mg`(뜬 상태) / `random_score` / `bomb_supple`. 소품 발판은 플레이어 물리에만 들어간다 (`SandboxState::UpdatePropCollision_`). 소품 벽은 `UpdateWalls_` 가 더한다. 아이템은 `ApplyItemPickup` 으로 플레이어에 반영된다. 샌드박스 해시는 소품이 있을 때만 소품 상태를 섞는다 (기존 리플레이 해시 유지).

