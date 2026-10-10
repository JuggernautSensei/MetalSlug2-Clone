# 스테이지 스크립트 포맷 (`Assets/Stage/*.json`)

레거시 `DevScene.cpp` 의 하드코딩 이벤트 표를 데이터로 옮긴 포맷. **커밋 대상**(Assets/Baked 와 달리 git 무시 아님).
첫 스테이지: `Assets/Stage/Stage0.json` (MS2 Mission 4). 스프라이트/애니/지면 이름은 `Assets/Baked/{sprites,anim,manifest}.json` 의 키와 일치해야 한다.
엔티티 `def` 이름(적/소품/NPC/아이템)은 `Docs/EnemySpecs.md` 와 맞춘다. 이 문서 §9 는 Stage0 이 쓰는 def 목록과 레거시 대응표다.
엄격 규칙: **알 수 없는 키/타입/enum 값은 로드 오류**(`Result` 실패), `note` 키만 어디서나 허용(무시됨).

## 1. 좌표/단위 규칙
| 항목 | 규칙 |
|---|---|
| 월드 좌표 | px, +x 오른쪽, +y 아래, 원점 = `Stage0_Main` 좌상단 (14094x1328). float 허용 |
| 시간 | 초(고정 60Hz 시뮬 시간). `delay` 는 "그 액션 목록이 실행된 시점으로부터" |
| 트리거 | 플레이어 x **>** `triggerX` (엄격 초과). 섹션은 배열 순서대로, 한 틱에 여러 개 발동 가능, 각 1회 |
| `origin:"feet"` | `(x,y)` = 엔티티 **발 중앙(바닥 접지점)**. `kind` 가 `enemy`/`npc` 일 때 기본값 |
| `origin:"legacyCenter"` | `(x,y)` = 레거시 `SetPos` 값 = Baker 가 굽은 스프라이트/애니 원점(정적 스프라이트는 이미지 **중앙**, 애니는 `pivotX/Y`) = 충돌체 중심. `prop`/`decor`/`item`/`collider` 의 기본값. 변환 불필요(그대로 그린다) |
| `halfHeight` | `legacyCenter` 를 쓰는 **크리처**(enemy/npc)에 필수: 충돌체 반높이. `feet.y = y + halfHeight` |
| 지면 스냅 | `groundSnap:true` → `y` 무시, `feet.y = FindGroundY(x, fromY)`: **현재 활성 지면 맵**에서 `fromY`(기본 0)부터 아래로 처음 만나는 고체 픽셀 행. 없으면 스폰 생략(레거시 동작) |
| `settle:true` | 주어진 위치에서 시작해 중력으로 착지(POW/플레이어). 착지면 = 픽셀 지면 + 소품 플랫폼 |
| 카메라 | 뷰 640x448, `camera.*` 의 중심 좌표 = 뷰 중앙 |

## 2. 최상위
| 키 | 타입 | 설명 |
|---|---|---|
| `version` | int | 1 |
| `name` | string | 스테이지 ID |
| `world` | `{size:[w,h], view:[w,h], background}` | `background` = sprites.json 키(`Stage0_Main`). 좌상단 (0,0) 에 그림 |
| `bgm` | `{name, loop}` | sounds.json 키. 스테이지 시작 시 재생 |
| `ground` | `{initial, maps:[...]}` | manifest `ground` 키. `setGround` 로 교체 |
| `backdrop` | `{initial, modes:{id:{sprite, offset:[x,y], parallaxX, period?}}}` | 원경 1장. `period` 가 있으면 그 간격으로 가로 반복, 없으면 한 장(화면보다 넓으면 끝에서 멈춤). 그리는 순서는 원경 → Stage0_Main → 월드. §4 |
| `player` | spawn 필드 부분집합 | 시작 위치. `x,y,origin,halfHeight,settle,groundSnap,fromY` |
| `rects` | `{name:[l,t,r,b]}` | 이름 붙은 사각형. rect 필드는 배열 또는 이 이름 |
| `camera` | §3 | |
| `sections` | 배열 | §5 |

## 3. camera
| 키 | 설명 (레거시 값) |
|---|---|
| `policy` | `"forward"`: 카메라 중심의 x,y 모두 **단조 비감소**(`max(prev, target)`). (`CAMERA_MOVE_POLICY::FORWARD`) |
| `initialBounds` | `[0,0,3520,448]` — 카메라 **뷰 영역**이 들어가야 하는 월드 사각형. 중심은 `[l+320, r-320] x [t+224, b-224]` 로 clamp |
| `initialOffset` | 추적 목표 = 플레이어 위치 + offset. `setCameraOffset` 으로 변경 |
| `playerMargin` | 플레이어를 뷰 좌우 14px 안쪽으로 가둔다(`ClampPlayerToScreen`) |
| `boundsCurves[]` | 매 틱 경계를 플레이어 위치로 계산하는 규칙(시가지 경사). 아래 |

`boundsCurves[]` 항목: `{id, source:"playerX", from, until, left, right, height, top:[[x,y],...], round}`
- 활성 구간 = 섹션 `from` 발동 시점 ~ 섹션 `until` 발동 직전. 활성 중 매 틱 `setBounds([left, T, right, T+height])`, `T = polyline(top, player.x)` (구간 밖은 양 끝 y 로 clamp, `round:"trunc"` 는 정수 절사).
- Stage0: `top=[[6500,311.75],[8610.7843,850]]` ⇔ 레거시 `min(850, 350+(x-6650)*0.255)`.
- 우선순위: 영구 잠금(§6 `permanent`) > 비영구 잠금 중(복원 경계만 갱신) > 일반 적용.

## 4. backdrop
화면 좌표의 원경 스프라이트 중심 = `(320 + offset.x - parallaxX*camCenterX, 224 + offset.y)`. (레거시 `FarBackground`: 오브젝트를 카메라 위치에 두고 offset 에 `-camX*0.15/0.1` 포함.) `setBackdrop {mode}` 로 전환.
Stage0: `first`(Stage0_Background_0, 249,-64, 0.15) → `second`(같은 스프라이트, 700,-130, 0.15) → `third`(Stage0_Background_2, 750,0, 0.1) → `fourth`(Stage0_Background_1, 1100,-30, 0.1).

## 5. sections
`{ id, triggerX, actions[], note? }` — `id` 유일. 액션은 **배열 순서대로** 실행(소품 → 적 → 웨이브 순으로 정렬되어 있음, 레거시 `EnterEvent` → `SpawnSectionEnemies` 순서와 동일).

### 모든 액션 공통
`type`(필수), `delay`(초, 기본 0), `when`: `"normal"`/`"silent"`/`"always"`. 디버그 워프(레거시 `_silent`: 건너뛴 구간은 소품·경계·지면만 적용, 적/게이트/연출 없음)에서 `silent` 실행 시 어떤 액션을 돌릴지 정한다. 기본값: spawn(kind=prop/decor/item/collider/npc), setBounds, setBackdrop, setCameraOffset, unlockCamera = `always`; 나머지 = `normal`. 일반 플레이에서는 `normal`/`always` 만 실행.

### 액션 표
| type | 필드 | 의미 |
|---|---|---|
| `spawn` | §7 | 엔티티 1개(또는 `repeat`) 생성 |
| `spawnWave` | `entries[]` (= spawn 필드 + 필수 `delay`, 선택 `repeat`(공간), 선택 `every`(초)+`times`(횟수, 기본 1, `times>1` 이면 `every>0` 필수)) | 지연 스폰 목록. 레거시 `AddWave`. 시각 = 액션 실행 시각 + `delay`, 이후 `every` 간격으로 `times` 번 발동(시간 반복). 카메라 기준 좌표는 **발동 시점**의 카메라로 계산. `clear waves` 는 반복분까지 전부 취소 |
| `setBounds` | `rect` | 카메라 경계 교체. 비영구 잠금 중이면 **복원 경계만 갱신** |
| `lockCamera` | `rect`(배열/이름/`"current"`), `glide?`(초), `permanent?` | 현재 경계를 스택에 저장하고 경계 = rect. `"current"` = 실행 시점의 뷰 영역. `glide`: 카메라 중심을 rect 중심으로 선형 이동(뷰 크기 rect 일 때). `permanent:true`: 해제 불가 + 이후 `setBounds`/`boundsCurves` 무시, 대기 웨이브·열린 게이트는 `clear` 로 정리 |
| `unlockCamera` | - | 가장 최근 비영구 잠금을 해제하고 경계 복원(없으면 무시) |
| `cameraMove` | `to:[cx,cy]`, `seconds` | 카메라 중심을 선형 이동. 끝나면 추적 재개(레거시 `CMR_MOVE_OP_UNLOCK`) |
| `setCameraOffset` | `offset:[x,y]` | 추적 offset |
| `setBackdrop` | `mode` | §4 |
| `setGround` | `name` | 지면 맵 교체(`ground.maps` 중) |
| `gate` | `id`, `waitFor:[spawnId]`, `until?`, `actions[]`, `abortOnGameOver?` | `waitFor` 의 모든 스폰이 소멸/격파되면 `actions` 실행(각 `delay` 는 이 시점 기준). `until:"defeated"`(기본, HP≤0 또는 제거) / `"destroyFinished"`(사망 연출 종료). `abortOnGameOver`: 게임오버 시 `actions` 취소(레거시 피날레) |
| `command` | `target:spawnId`, `command` | 엔티티에 명령. 현재 사용하는 적 없음 (재설계 예정) |
| `clear` | `waves?:bool`, `layers?:["monster"]` | 대기 웨이브 취소 / 해당 레이어 오브젝트 전부 제거(레거시: ObjectType::Monster = 적+트럭+보트 포함) |
| `banner` | `text`(`\n` 줄바꿈), `seconds` | HUD 배너 |
| `fade` | `to:"black"\|"clear"`, `seconds` | 화면 페이드 |
| `playBgm` | `name`, `loop` | BGM 교체 (Stage0 미사용) |
| `playerInvincible` | `seconds` | 플레이어 무적 |
| `endStage` | `next:"title"` | 스테이지 종료 후 다음 상태 |

## 6. 카메라 잠금 의미 (레거시 대응)
| 레거시 | 데이터 |
|---|---|
| `SetCameraMove(2790,224, 1s, LOCK)` + 버스 사망 시 `SetCameraLock(false)` | `lockCamera {rect:[2470,0,3110,448], glide:1}` + `gate(bus) → unlockCamera` |
| `OpenGate` (헬기/사루비아: 현재 뷰로 경계 고정, 사망 시 복원) | `lockCamera {rect:"current"}` + `gate → unlockCamera` |

## 7. spawn 필드
| 필드 | 필수 | 설명 |
|---|---|---|
| `kind` | O | `enemy` `npc` `prop` `decor` `item` `collider` |
| `def` | O | 엔티티 def 이름(§9). `decor` 는 `anim_decor`(+`anim`) / `sprite_decor`(+`sprite`) |
| `id` | | 스폰 ID(유일). `gate.waitFor`/`command.target` 이 참조 |
| `x`,`y` | * | 월드 좌표(§1). `groundSnap` 이면 `y` 생략. `camera` 사용 시 둘 다 생략 |
| `origin`,`halfHeight`,`groundSnap`,`fromY`,`settle` | | §1 |
| `camera` | | `{ax:"left"\|"right"\|"center", dx:int\|[min,max], ay:"top"\|"bottom"\|"center", dy:int}` — 발동 시점 카메라 뷰 기준. `dx` 범위는 [min,max] 정수 균등 난수(스테이지 시드 RNG). 이때 좌표는 **발 기준**(origin 기본값 규칙 동일) |
| `repeat` | | `{count, stepX, stepY}` — i번째 = `(x+i*stepX, y+i*stepY)`. 카운트는 spawn `count` 로 센다 |
| `facing` | | `"left"\|"right"` |
| `hp` | | 정수, def 기본 HP 덮어쓰기 (Stage0 미사용) |
| `drop` | | 아이템 ID(§9). POW 구출/적 사망 시 드롭 |
| `ambush` | | `{xMin,xMax}` 예약 필드: 플레이어 x 가 창 안일 때만 활성화 (Stage0 미사용) |
| `layer` | | 렌더/갱신 레이어 힌트: `farBackground` `background` `interact` `monster` `npc` `item` `frontBackground` `collider`. 생략 시 def 기본값 |
| `cullMargin` | | 객체 x 가 `camLeft - margin` 보다 왼쪽이면 제거. 기본 200 (레거시 `RemoveObjectOutOfCamera`) |
| `size`,`collisionLayer` | | `kind:"collider"` 전용(필수): 중심 `(x,y)`, 크기 `[w,h]`, `collisionLayer`: `wall` \| `groundExceptPlayer` |
| `anim`/`sprite` | | decor 전용 (anims/sprites 키) |
| `params` | | def 별 인스턴스 파라미터(§9). 알 수 없는 키는 def 가 거부 |

## 8. 실행 순서 요약
1. 로드 → 지면 = `ground.initial`, 카메라 = `initialBounds`/offset, `backdrop.initial`, BGM, `player` 배치.
2. 매 틱: 플레이어 x 로 미발동 섹션을 순서대로 검사 → 발동 시 액션 실행 → 대기 웨이브 처리 → `boundsCurves` → 게이트 판정 → 플레이어 화면 clamp.
3. 한 섹션 내 `delay` 가 있는 액션은 그 섹션 시작 시각 기준 타이머로 예약.

## 9. Stage0 에서 쓰는 def 와 레거시 대응
**적(kind=enemy)** — 발 기준. 레거시 충돌체 반높이 `halfHeight`:
| def | 레거시 | halfHeight | 비고 |
|---|---|---|---|
| `soldier_bomb` `soldier_sword` `soldier_missile` | `Soldier` + `SOLDIER_AI_TYPE` BOMB_THROWER/SWORD_MAN/MISSLE_THROWER | 37.5 | 40x75 |
| `soldier_bzk` | `SoldierBzk` | 37.5 | |
| `soldier_hang` | `SoldierHang` | 37.5 | 감시탑 밑에 매달림, `facing:left` |
| `flying_bzk` | `FlyingBzk` (낙하산) | 37.5 | |
| `shielder` | `Shielder` | 37.5 | |
| `jeep` | `Jeep` | 30 | 75x60, 지면 스냅 |
| `missile_tank` | `MissleTank` | 50 | 100x100 |
| `btank` | `BTank` | 75 | 100x100, offset y+25 → 발 = pos+75 |
| `sarubia` | `Sarubia` | 50 | 중간보스 |
| `mb_truck` | `MBTruck` (버스 중간보스) | 130 | 180x260. `legacyCenter`. 사망 시 지면 `Stage0_1` 교체는 스테이지 데이터(gate)에서, 파괴 이펙트/`MainTruck_Die_Front`(pos+(12,88))는 def 쪽 |
| (미사용) `fixed_jeep` `alian` | `FixedJeep` `Alian` | 30 / 50 | Stage0 DevScene 은 스폰하지 않음 |

**NPC/아이템**: `pow`(= 레거시 `Older`, 충돌체 50x80 → halfHeight 40, 항상 `settle`) / 아이템 ID: `gunitem_mg` `gunitem_sg` `bomb_supple` `bullet_supple` `item_letter` `random_score`(과일 7+애니 6 중 균등) `random_fruit`(apple carrot banana banana_bundle watermelon cabbage pair) `random_anime`(mando letter chicken fish bread fruits).

**소품(kind=prop)** — `legacyCenter`. 모두 HP 를 가진 파괴물, 파괴 시 스프라이트 교체:
| def | 레거시 | 스프라이트(idle → destroyed) | 충돌체 / 비고 |
|---|---|---|---|
| `shop_0..5` | `S0Building` | `params.sprites` | HP 15. `params.hitbox{size,offset}`(INTERACTION, 피격) + `params.standBox{size,offset}`(GROUND, 올라서기). 파괴 시 standBox 제거; `shop_5` 만 파괴 시 간판(`Stage0_Sign_0→1`, pos+(50,-200)) + `bomb_supple` 드롭, standBox → `[220,20]` offset `[47,130]` |
| `truck` | `Truck` | 애니 `Truck_`/`LoadedTruck_`/`RefTruck_` `Idle/Hit/Down` (변종 랜덤) | HP 15, 머리 44x75 + 꼬리 70x25(또는 70x90). 변종 LOADED 는 y-10, REFRI 는 y-17 보정 |
| `fence` | `Pence` | `Pence0..2` 랜덤 | 60x30 offset (0,-20). 접촉 즉시 파괴 |
| `mob_boat` | `MobBoat` | `Mob_Boat_Idle → Mob_Boat_Destroy` | HP 30, 150x150 offset (0,25) wall. `params.explodeOnDeath` (true: 2초 후 폭발·삭제). 사망 시 점수 아이템 3+3 드롭 |
| `bridge_deck` | `Stage0Bridge::BRIDGE` | (없음, 배경에 포함) → `Stage0_Bridge_0` | HP 10, wall 175x100 offset (25,0) |
| `bridge_house` | `…::HOUSE` | → `Stage0_Bridge_2` | 150x100 offset (0,0) |
| `bridge_house2house` | `…::HOUSE2HOUSE` | → `Stage0_Bridge_1` | 150x100 offset (0,-25) |
| `bridge_block` | `…::BLOCK` | `Stage0_Block_Idle → Stage0_Block_Destroy` | 135x95 offset 0; 파괴 후 72x91 offset (30,0) |
| `house_upper` `house_open_window` `house_cloth` `house_no_cloth` `house_yellow` | `Stage0House` HOUSE_* | `Stage0_House_{0_Idle, 1_Idle, 2_Idle_0, 2_Idle_1, 3_Idle} → …_Destroy` | HP 10. 지붕 플랫폼(적 전용, GROUND_EXCEPT_PLAYER) 150x30 offset y = 3 / -14 / -27 / -27 / -27 → 윗면 y = pos.y + offset.y - 15 |
| `ground_mini_house` | `Stage0GroundMiniHouse` | → `Stage0_Bridge_3` | HP 10, 원 r70 offset (40,40) + 벽 20x45 offset (85,-1) |
| `ground_house` | `Stage0GroundHouse` | `Stage0_House_4_Idle → Destroy_0 (HP≤10) → Destroy_1 (HP≤0)` | HP 20, 원 r110 offset (10,50). 파괴 시 지면 발자국 패치 `Stage0_House_4_PixelMap` @ `pos - size/2 + (12,5)` (발자국만 지운다, PropSpecs §2.3) |
| `big_house` | `Stage0BigHouse` | `Stage0_House_5_Idle → Destroy_0 (HP≤20) → Destroy_1 (HP≤0)` | HP 40, 박스 310x120 offset (10,0) + 원 r120 offset (-110,60),(120,60). 지면 발자국 패치 `Stage0_House_5_PixelMap` @ `pos - size/2 + (12,9)` (PropSpecs §2.3). `cullMargin` 150 |
| `tower` | `Stage0Tower` | `Stage0_Tower_Idle → Destroy` | HP 10, 피격 90x210, 플랫폼 80x10 offset (5,45),(5,-25) |
| `guard_post` | `GuardPost` (애니 `GuardPost`) | - | 플랫폼 130x10 offset (0,-65), 165x10 offset (0,10) |

**decor / collider**: `anim_decor`(`Parker`, `Stage0_Water`(플레이어 진입 시 물 이펙트 트리거 1050x40, `cullMargin` 100), `Plane`(속도 (70,-50) px/s)), `sprite_decor`(`Sewer`, `frontBackground`), `box_collider`.

## 10. 섹션 표 (레거시 `gStage0Events`)
`intro`(0) `street`(750) `ground_swap`(2244) `boardwalk`(3050) `bridge`(3300) `bridge_down`(3700) `bridge_mid`(4037) `bridge_end`(4600) `river_start`(5000) `river_mid`(5600) `river_end`(6220) `downtown_start`(6500) `downtown_mid`(7400) `downtown_end`(8700) `openland_start`(8980) `openland_1`(9450) `openland_2`(10350) `openland_3`(11350) `openland_4`(12350) `stage_end`(12900).

버스(`bus_*`)/헬기/최종보스 섹션은 삭제했다 (재설계 예정, RemakePlan 3단계). `stage_end` 는 옛 보스 아레나 입구에서 `endStage` 로 스테이지를 끝낸다.

## 11. 알려진 차이 / 주의
- 레거시 `rand()` 사용처(드롭 x 지터, 과일/애니 랜덤, 울타리 스프라이트, 트럭 변종, `random_score`)는 스테이지 시드 RNG 로 대체. 재현 가능.
- 레거시 병사는 지면보다 3px 위(`g-40`, 실제 반높이 37.5)에서 스폰해 중력으로 안착. 새 포맷은 발 y 를 직접 지정(`groundSnap`/플랫폼 윗면).
- 플랫폼 스폰(`y` = 윗면)은 같은 섹션에서 앞서 생성된 소품(집 지붕·다리 도로 `box_collider` y=405) 위에 선다. 순서 의존.
- 검증: `Scripts/ValidateStage.ps1 [-Stage Assets/Stage/Stage0.json]` — 파싱, ID 유일/참조, 이름(스프라이트/애니/지면/BGM) 존재, 섹션별 개수 출력.

## 12. 런타임 (`StageState`, 상태 이름 `stage0`)
- 구성: `Source/Game/Stage/StageState.*` 가 `Assets/Stage/Stage0.json` 을 `StageRunner` 로 돌린다. 플레이어/전투/투사체/카메라/HUD 는 SandboxState 와 같은 경로. 기본 시작 상태는 `title` (Assets/Baked 가 없으면 `sandbox`), 타이틀 Start → `stage0`.
- 호스트: `StageWorldHost` (`IStageHost` 구현). 스폰 한 개를 아래 순서로 보낸다. 정의가 없으면 다음 단계로 떨어진다.
  - `enemy` / `npc` → `EnemyWorld::Spawn` (def 는 Enemies/Vehicles.json). `params.arena` 는 rect 이름을 풀어 `SpawnParams::arena` 로 준다.
  - `prop` → `PropWorld::Spawn` (Props.json). `params` 의 `sprites` / `hitbox` / `standBox` / `explodeOnDeath` 를 `PropSpawnParams` 로 옮긴다. `hp` 는 def 값을 쓴다 (스테이지 데이터에 hp 덮어쓰기 없음).
  - `item` → `PropWorld::SpawnItem` (아이템 id 또는 `random_*` 그룹). 중심 기준이므로 발 = 중심 + (0, 16).
  - `collider` → 스테이지 충돌체 목록. `collisionLayer: wall` 은 플레이어/적 벽, `groundExceptPlayer` 는 적만 서는 원웨이 바닥. 매 틱 물리 입력에 합친다.
  - 그 밖 (`decor` 의 `anim_decor` / `sprite_decor`, 정의가 아직 없는 이름) → `StageWorldHost` 의 플레이스홀더 (상자). 판정 가능 kind 만 평소에도 그린다. 장식 플레이스홀더는 F1 에서만 보인다.
- 틱 순서: 판정 시작 → 소품 Tick → 벽/발판 안정 저장소 갱신 (`StageState` 멤버 벡터에 복사, 이 틱 동안만 쓴다) → Hurtbox 등록 (플레이스홀더/적/소품, 플레이어 Tick 전) → 플레이어 → 아이템 습득 → 적 → 투사체 Step → Resolve → 피해 적용 (소품/적/플레이스홀더/플레이어/투사체) → 이벤트 소비 → 연출 → `StageRunner::Tick`.
- 이벤트: 적 `Died` → 점수 + 드롭 아이템 (`drop`), `ItemDropped` (포로) → 튀는 아이템, 소품 `GroundPatch` → Baked/Textures/<sprite>.png 의 파란 픽셀 발자국을 `ApplyGroundPatch` 로 활성 지면에서 공기로 만든다 (발자국 밖은 불변, 바닥을 지우는 마스크는 가드로 거부; PropSpecs §2.3).
- 게이트: `IsDefeated` / `IsDestroyFinished` 는 라우팅 표로 해당 월드에 묻는다 (적은 EnemyWorld, 소품은 PropWorld, 아이템은 사라짐, 충돌체는 제거됨).
- 끝 장면: `endStage` → `MISSION COMPLETE` (ScreenFlow, 세계 정지) → `title`. 플레이어 게임 오버 → `GAME OVER` (ScreenFlow) → `title`. Stage0 의 `stage_end` 섹션은 `playerInvincible` 뒤 `endStage` 만 한다 (배너/암전은 ScreenFlow 가 맡는다).
- 디버그: F1 켜진 상태에서 F5 = 다음 섹션으로 워프, F6 = 마지막 구간(`stage_end`)으로 워프 (`--warp-boss` 도 같다) (GameContext::debugWarp). `--warp N` 명령행 옵션은 시작 시 F5 를 N 번 누른 것과 같다 (F1 도 켠다). F1 오버레이에 섹션 id, 열린 게이트, 적/소품/아이템/충돌체/플레이스홀더 개수, 지면 이름이 나온다.
- 한계 / 아직 없는 것:
  - `box_collider` (x 1119, y 350, 137x90, wall) 는 윗면 y 305 이 점프 정점 (약 y 303~312) 과 거의 같아 일반 점프로 넘기 어렵다. 데이터(y 가 중심인지 윗면인지) 또는 물리를 확인할 것.
  - `sendCommand` 는 적에게만 전달한다 (대상 행동의 `onCommand`). 그 밖의 명령은 경고 한 번.
  - `clear` 는 적(Monster 비트, Npc 비트면 포로 포함)과 충돌체를 지운다. 소품/아이템은 지우지 않는다.
  - 원경 장식 (`anim_decor` / `sprite_decor`) 은 플레이스홀더다 (정의/스프라이트 연결 없음). 보스 HP 바 (`bShowBoss`) 는 아직 HUD 에 연결하지 않았다.
  - 무적(`playerInvincible`) 은 플레이어 `ExtendInvincibility` 로 실제 피해를 막는다 (PlayerController 추가 훅 1개).
