# 적/소품/NPC/투사체 사양 (M3)

레거시 `Client/WindowAPIProject` 코드를 읽고 정리한 구현용 스펙이다. 수치는 레거시 그대로 옮기고(인용: `파일:줄`), 새 게임 규칙(고정 60Hz, 발 기준 원점, HitResolver 단일 판정, 데이터 주도)에 맞춘 **권장값/교정 의도**를 따로 표시한다.
데이터 스키마는 `EnemyDefsSchema.md`. 모든 수치는 `Assets/Tuning/EnemyDefs.json` 로 빼는 것이 원칙(코드 상수 금지).

## 0. 공통 규약

### 0.1 좌표/크기 변환
- 레거시 원점 = 오브젝트 **중심**(`SetPos`). 충돌체 `BoxCollider{scale, offset}` = 중심 `pos+offset`, 전체 폭/높이 `scale` (`BoxCollider.cpp:68-78`).
- 새 원점 = **발 중앙**(x 중앙, y 바닥). 변환: `feet = legacyPos + (0, F)`, `F = collider.offset.y + scale.y/2` (몬스터 지면 보정이 쓰는 값, `Monster::UpdatePixelGround`). 새 spawn offset `= legacyOffset − (0, F)`.
- 표기: `box W×H @(cx,cy)` = 박스 **중심**의 발 기준 좌표(y 아래가 +, 위는 −). `muzzle (x,y)` 도 발 기준.
- 지면: 스테이지 0 가로 도로 지면 y=378 (`Ground/Stage0_0.bin` 에서 확인), 강바닥 658, 개활지 1286. 레거시 스폰은 `FindGroundY(x)` 로 픽셀맵 첫 고체 y 를 읽어 `ground − F` 로 배치(`DevScene.cpp:1032-1087`). 새 스테이지 JSON 은 `{x, ground:true}` 로 지면에 자동 안착시킨다.
- 해상도 640×448. `W=640,H=448`.

### 0.2 시간/프레임
- 레거시는 가변 dt 였으나 모든 타이머가 초 단위 누적이라 그대로 초로 쓴다. 속도 px/s, 가속도 px/s².
- `fN` = 클립의 N번째 프레임(0 기준, 레거시 `GetMomentAtIndex(N)` = 그 프레임에 **진입하는 순간 1회**). 이벤트 시각 = `N × frameDuration`. 새 구현은 클립 이벤트(`events:[{frame:N, do:...}]`)로 선언한다.
- "애니 종료"(`IsAnimationEnd`) = 비루프 클립의 마지막 프레임 재생이 끝난 시점. 루프 클립은 항상 true 이니 **상태 전이 조건에 루프 클립을 쓰지 말 것**(레거시도 안 씀).

### 0.3 Monster 공통 (`Monster.h/.cpp`)
| 항목 | 값 | 의도 |
|---|---|---|
| 활성화 | 화면 좌표 x ≤ W+margin 이 되면 **영구 활성**(래치). margin 기본 150, Aparch 400 (`Monster.cpp:28-40`) | "화면 폭 + 마진 안으로 들어오면 행동 시작". 활성 전에는 AI/발사 정지(중력/착지는 동작) |
| 제거 | `x < cameraLeft − 200` 이면 제거 (`CollisionModule.cpp:200`). BattleShip/터렛은 예외 | 뒤처진 적 정리. 새 구현은 `despawn.behindCameraPx=200` |
| 점수 | GRUNT 100 / VEHICLE 500 / TANK 1000 / MID_BOSS 2000(레거시 미사용) (`Monster.h:13-19`). `AwardKillScore` 1회만 | 점수는 def 의 `score` |
| 피격 | 레거시: 플레이어 HITBOX 겹침 시 `Projectile` 의 `damage` 만큼 HP 감소(탄 1, 샷건 20, 폭탄 10, 칼 3). 병사류는 HP=1 이라 무조건 즉사 | 새: HitResolver 가 `{kind,damage,pierce}` 로 처리. 적 클래스는 레이어/타입을 보지 않는다 |
| 몸통 접촉 | 플레이어에게 무해 | 유지 |

### 0.4 적 투사체/히트박스 공통 (`Projectile.cpp`)
- 이동 `pos += force·speed·dt`(force 정규화). 화면 밖 `x≤−100 | x≥W+100 | y≤−100 | y≥H+100` 이면 제거, 수명 5 s (`Projectile.cpp:20-38`).
- 레이어 `MONSTER_HITBOX`, 플레이어(`CL_PLAYER`) 와 겹치면 **플레이어 즉사**(damage 필드는 의미 없음; `MyPlayer.cpp:245`). 플레이어가 사망/무적(`CanBeHit()==false`)이면 **통과**(`IsUnhittablePlayer`). → 새: 모든 적 투사체 동일 규칙 (BattleShipBullet 은 레거시에서 이 검사를 빠뜨림 = 버그).
- 지형 소멸: 픽셀맵 고체(파랑) 또는 `GROUND/WALL` 충돌체. 단 `GROUND` 와는 **윗면(TOP) 접촉은 무시**(`GetCollisionFlag==TOP_CLS` 면 return).
- 폭발 이펙트: `Explosion_{Small,Mid,Large,Huge}` 모두 28/28/28/24 프레임, 1.68/1.68/1.68/1.20 s. `CreateExplosionEffect(pos,offset,bound,s,m,l)` 은 S/M 을 `+rand[0,bound)` **양(+)방향으로만** 치우쳐 뿌림(버그) → 새: 중심 ±bound/2 균등.

### 0.5 사망 연출 공통 (병사류)
`SD_Die_*` 클립 재사용. 지상 사망: `Die_Stand_1`(f10에서) / `Die_Stand_2`(f8에서) 50:50 → 지면 접촉 중이면 `Die_Fall`(중력 ON) → 착지 시 `Die_Land` (`Soldier.cpp:262-277,353-373`). 공중 사망: `Die_Jump`(중력 ON) → 착지 시 `Die_Land`. 시체는 충돌 없음(레이어 DEFAULT), **`Die_Land` 시작 후 2.0 s 에 깜빡임(1 s, 0.1 s 주기), 3.0 s 에 제거**(`Soldier.cpp:301-315`). 레거시가 안 쓴 `Die_Bomb`(8f,.64 s)/`Die_Slash`(12f,.96 s) 가 존재 → 권장: Blast→Die_Bomb, Melee→Die_Slash.

### 0.6 새 데미지 모델과 HP 환산
- PlayerSpec: 권총 2, HMG 2, 샷건 6, 폭탄 20, 칼 4. 권총 홀드 연사 6틱 간격 = 10발/s (이론 20 dps). 사람 탭 + 명중률 감안한 **기준 지속 dps = 10 (5 hits/s × 2)**.
- `HP = TTK(s) × 10`. 목표 TTK: 지프 8, 전차 12, 버스 25, 보스 90 → 80 / 120 / 250 / 900. 폭탄(20)은 HP/20 발, 샷건(6, 관통 근접)은 HP/6 발.
- 계획서(`zippy-dreaming-beaver.md`)의 예시(Jeep 20/전차 30/버스 60/보스 400)는 같은 환산으로 TTK 2/3/6/40 s 에 해당 → **이번 TTK 목표에 맞춰 위 값으로 대체**. 한 상수(`dpsRef`)로 일괄 재조정 가능하게 def 에 TTK 를 같이 적는다.
- 파괴물(상점/집 등)은 "권총 N발" 수를 레거시와 동일하게 유지: `newHp = legacyHp × 2`.
- 병사류: HP 1 = 어떤 피해든 즉사 ("1타 처치").

### 0.7 알려진 레거시 공통 버그 (재현 금지)
1. 칼 히트박스가 `ObjectType::UI` 로 생성되어 `Projectile` 캐스트 검사(`Shielder`, `BattleShipDoubleTurret`, `Alian`...)에서 누락. → HitResolver `Melee` 로 일원화.
2. 상태 이중버퍼(CURR/RESERVE) 로 1프레임 지연(BTank/MissleTank/Sarubia/Aparch/Truck/Older). → 같은 틱 전이.
3. 이동 중 바라보는 방향 갱신 누락(Soldier MOVE, Shielder MOVE). → 의사결정 시점(상태 진입)마다 플레이어 쪽으로 갱신.
4. `CreateExplosionEffect` 양방향 편향, `_timeSum = 0.1 - _timeSum`(스모크 주기 오타, `-=` 의도).
5. 픽셀 한 점 판정(`GetPixel`)으로 탄이 얇은 지형을 통과. → 선분 스윕.

---

## 1. 보병 계열

### 1.1 Soldier (일반 병사, 3 AI) — `Soldier.h/.cpp`, `Soldier*AI.cpp`
| 항목 | 값 |
|---|---|
| 역할 | 근접/수류탄/미사일 병사. 같은 몸에 AI 만 교체 (BOMB_THROWER / SWORD_MAN / MISSLE_THROWER) |
| 충돌 | 몸통 `box 40×75 @(0,−37.5)` (`Soldier.cpp:56`), 벽 감지 트리거 `40×50` offset `(±25,0)` (진행 방향, `:63`) → 새: 전방 프로브 `box 40×50 @(±25,−37.5)`. F=37.5 (레거시 스폰 `ground−40`) |
| HP | 레거시 1 → 새 **1** (즉사) |
| 점수/드롭 | 100, 드롭 없음 |
| 물리 | 중력 1000 px/s², 점프 vy=−450 (정점 ≈101 px, 체공 0.9 s), 이동 100 px/s, `MoveDown` 100 px/s (`Soldier.h:61-62`) |
| 활성 | margin 150 래치. 스폰 직후 `ON_AIR_V` 로 낙하해 착지 |
| 방향 | 클립 접미 `_Left/_Right` = 바라보는/이동 방향. **IDLE 에서만** 플레이어 쪽으로 전환(`Soldier.cpp:101-117`) → 새: 모든 의사결정 시점에 갱신(공격 중 제외) |

**AI 공통 정의** (플레이어 상대량: `dx=player.x−self.x`, `dy=player.y−self.y` (+ = 플레이어가 아래), 발 기준 y 사용 권장. `onSolid` = 지면/발판 위 정상 접촉):
- `DROP` = `onSolid && dy>50` → `MoveDown`(원웨이 발판 통과 하강). 레거시는 일반 지면에서도 조건이 참이라 오작동 가능 → 새: **원웨이 발판 위에서만**.
- `JUMP` = `dy<−100` → `H_Jump`.
- 의사결정 쿨타임 `COOLTIME=1 s` (IDLE/MOVE 에서 `_timeSum`≥1 s 일 때만 판단. 상태 진입 시 0). `rand()%k` 균등.

**상태/전이 (공통 모션)**
| 상태 | 클립(`_L/_R`) | 동작/종료 |
|---|---|---|
| Idle | `SD_Idle` 6f×.1 루프 | 방향 전환만 |
| Move | `SD_Move` 12f×.08 루프 | 100 px/s 전방 이동 |
| MoveStop | `SD_MoveStop` 4f×.08 | (정의만, 어느 AI 도 진입 안 함) |
| MoveDown | `SD_MoveDown` 9f×.07=.63 s | 지면 충돌 OFF, y+=100·dt (≈63 px). 종료 시 `OnAir_V`+중력 ON |
| H_Jump | `SD_MoveJump` 6f×.08 | 즉시 vy=−450, 공중 전방 100 px/s |
| V_Jump | `SD_Jump` 5f×.1 | 즉시 vy=−450 (AI 미사용) |
| Land | `SD_Land` 3f×.08=.24 s | 종료→Idle |
| Throw_Bomb | `SD_Throw_Bomb` 14f×.08=1.12 s | **f10(0.80 s)** 폭탄 생성, 종료→Idle |
| Throw_Missle | `SD_Throw_Missle` 18f×.08=1.44 s | **f10(0.80 s)** 미사일 생성, 종료→Idle |
| Slash | `SD_Slash` 23f×.08=1.84 s | **f9(0.72 s)** 칼 히트박스 생성, 종료→Idle |
| Jump_Slash_Jump→Ready | `SD_Jump_Slash_Ready` 2f×.12 | vy=−450 후 Ready 진입, 공중 100 px/s 전진, **0.5 s** 후 Jump_Slash |
| Jump_Slash | `SD_Jump_Slash` 5f×.08 | f2(0.16 s) 히트박스, 착지→Jump_Slash_Land |
| Jump_Slash_Land | `SD_Jump_Slash_Land` 14f×.08=1.12 s | 종료→Idle |

**BOMB_THROWER** (`SoldierBombThrowerAI.cpp`, DETECT 350):
- Idle(≥1 s): `|dx|≥350` → 50% Move / 50% {DROP→MoveDown, JUMP→H_Jump, else Move}. `<350` → 1/3 Throw_Bomb · 1/3 {DROP, JUMP, else Throw_Bomb} · 1/3 Move.
- Move: (≥1 s) 1/3 Idle · 1/3 {DROP, JUMP, else Throw_Bomb} · 1/3 Throw_Bomb. 타이머 전: `|dx|≤200` → 50% Idle / 50% Throw_Bomb; 전방 벽 → 50% H_Jump / 50% Throw_Bomb.
**SWORD_MAN** (`SoldierSwordManAI.cpp`, DETECT 350, JUMP_ATTACK 150):
- Idle(≥1 s): `|dx|≥350` → 50% Move / 50% {DROP, JUMP, else Move}. `150≤|dx|<350` → {DROP, JUMP 우선, else 50% Jump_Slash_Jump / 50% Move}. `<150` → 1/3 Slash · 1/3 {DROP else H_Jump} · 1/3 Move.
- Move: (≥1 s) {DROP, JUMP 우선, else 1/3 Idle · 1/3 Jump_Slash_Jump · 1/3 Move}. 타이머 전: `|dx|≤150` → 50% Idle / 50% (`|dx|≤100`? Slash : Jump_Slash_Jump); 전방 벽 → H_Jump.
**MISSLE_THROWER** (`SoldierMissleThrowerAI.cpp`, DETECT 400):
- Idle(≥1 s): `|dx|≥400` → 50% Move / 50% {DROP, JUMP, else Move}. 그 외 {DROP, JUMP 우선, else 50% Throw_Missle / 50% Move}.
- Move: (≥1 s) {DROP, JUMP 우선, else 50% Throw_Missle / 50% Idle}; 전방 벽 → H_Jump.
> 새 구현: 위 표를 가중치 전이 테이블(스키마 `choose[]`)로 그대로 데이터화. 가중치 `rand()%k` = 균등.

**공격 정의 (발 기준)**
| 이름 | 발사 | 값 |
|---|---|---|
| SoldierBomb | Throw_Bomb f10, 위치 `(0,−37)` (레거시 pos), 방향 전방 | `box 20×20`, vx=±200, vy0=−500, 중력 750 (정점 +167 px, 수평 사거리 ≈267 px, 체공 1.33 s). 클립 `Soldier_Bomb` 16f×.07 루프. 지면/벽/플레이어 접촉 시 `Enemy_Bomb_Exp`(8f×.05) 를 `(0,−8)` 에 생성. 폭발 이펙트 자체는 무해, **탄 박스 접촉만 치명**. `SPEED_IN_MOVE=350` 은 미사용 |
| SoldierMissle | Throw_Missle f10, `(0,−62)` | `box 40×20`, 속도 0→400 선형가속(400 px/s², 1 s 에 최대), 클립 `SD_Missle_{L/R}` 4f×.07, 스폰 시 `|dy|>25` 면 방향에 ±0.1 y 기울임(정규화). 지면/벽/플레이어 시 `Explosion_Mid`(0,−35) |
| SlashHitbox(Slash) | Slash f9 | `box 60×50 @(±35,−37)`, 지속 **0.4 s**, 비추종, 플레이어 즉사 |
| SlashHitbox(Jump) | Jump_Slash f2 | `box 70×70 @(±35,−37)`, 지속 0.25 s, 병사와 같이 100 px/s 전진 |
- 스폰 요건: ① 지면 배치 `{x, ground}` (레거시 `ground−40`) ② 원웨이 지붕/발판 위 `topY−40` ③ **화면 오른쪽 위 낙하**(`camRight−50−rand(100)`, `camTop+10`) 웨이브(`DropSoldier`). 스테이지 사용량: 거의 모든 구간 (`DevScene.cpp:1177-1489`).
- 레거시 버그: `Soldier.h` 의 `DETECH_RANGE=500` 미사용(AI 헤더 값이 실제). 시체 `_timeSum` 이 상태 전환마다 리셋됨(의도는 착지 후 3 s). `DIE_BY_BOMB/SLASH` 상태 미사용.

### 1.2 SoldierBzk (바주카 병사) — `SoldierBzk.h/.cpp`, `SdBzkPorj.*`
| 항목 | 값 |
|---|---|
| 충돌 | `box 40×75 @(0,−37.5)`, 벽 프로브 `50×50` offset `(∓30,0)` (`:91,:100`). F=37.5 |
| HP/점수 | 1(즉사) / 100. 사망 시 디브리 `Bazuka_Debics`(AnimeDebrics) |
| 물리 | 중력 800, 점프 vy=−350 (정점 ≈77 px), 이동 100 px/s, `COOLTIME=1`, `DETECT=400`(**유클리드 거리**), `BULLET=2` |
| 활성 | margin 150. Idle 에서 활성 전 대기 (`:136-138`) |

**상태머신**
| 상태 | 클립 | 동작 |
|---|---|---|
| Idle | `Bazuka_Idle` 6f×.1 루프 | 항상 플레이어 방향으로 전환. ≥1 s: dist>400 → Move; else 2/3 Aim / 1/3 Move |
| Move | `Bazuka_Move` 11f×.08 루프 | 100 px/s. dist≤400 → Aim; 벽 프로브 → H_Jump; ≥1 s → Idle |
| H_Jump | `Bazuka_Jump_Forward` 8f×.1 | **f3(0.3 s)** 에 vy=−350 → OnAir_H(100 px/s 전진) → 착지 `Bazuka_Stand_Up`(5f×.08)→Idle (제자리 점프는 `Bazuka_Jump`, `Lending`) |
| Aim | `Bazuka_Aiming_Up/Down` 3f×.1=0.3 s | 플레이어가 아래(dy>0)면 Down, 아니면 Up. 종료 시 **각도 5구간 선택**(`:255-300`), `bullet=2` |
| Shot_Ready_x | `Bazuka_Shot_Ready[_Up0..3 / _Down0..3]` 1f | **1.0 s** 대기 |
| Shot_x | `Bazuka_Shot[_Up0..3 / _Down0..3]` 2f×.08 | **f1(0.08 s)** 발사, `bullet--`. 종료: bullet>0 → Shot_Ready_x(1.0 s 후 재발사, 각도 재계산 안 함) / 0 → Recover |
| Recover | `Bazuka_Up_Recover`/`Down_Recover` 3f×.1 | →Idle |
- 각도 구간(수평 기준, 위쪽 Up 계열 / 아래쪽 Down 계열): `<11.25°`→F(0°), `<33.75`→22.5°, `<56.25`→45°, `<78.75`→67.5°, 이상→90°. 클립 매핑: Up0..3 = 22.5/45/67.5/90°(위), Down0..3 = 22.5/45/67.5/90°(아래).
- 한 사이클 ≈ Idle 1 + Aim .3 + (1+.16)×2 + Recover .3 ≈ 4 s.
- **투사체 SdBzkPorj**: 머즐 `(0,−63)+dir·50` (레거시 `(0,−26)+dir·50`, dir 은 8방향 단위벡터, 좌향이면 x 반전). `box 25×25`, **속도 200 일정, 중력 없음**, 클립 `Bazuka_Projectile` 4f×.08 루프. 소멸: 픽셀맵 고체/플레이어 → `Explosion_Mid`(0,0). 발사 이펙트 `Bazuka_Shot_Effect`(16f×.08) 머즐 위치.
- 사망: §0.5 규칙(지상 `Die_Stand_1/2`, 공중 `Die_Jump`) + 디브리 `Bazuka_Debics` 를 `pos` 에 생성(`:535`).
- 스폰: 지면 `{x, ground}`(`ground−40`); FlyingBzk 착지 변신. 레거시 버그: 벽 프로브 offset 방향 갱신이 Idle 에서만(이동 중 방향이 바뀔 일은 없으나 점프 후 복귀 시 불일치 가능), 두 번째 발사는 각도 재계산 없음(플레이어가 이동해도 첫 각도 유지 → 의도로 보고 유지).

### 1.3 SoldierHang (매달린 병사) — `SoldierHang.*`
- 감시탑(GuardPost) 아래에 정적으로 매달림. 충돌 `box 40×75 @(0,−37.5)`(바닥 = `SD_Hang_Idle` 50×80 피벗 (25,40) 과 일치). 중력은 **사망 시에만 ON**. HP 1, 점수 100.
- Idle(`SD_Hang_Idle` 1f 루프): 플레이어 쪽으로 방향 전환. `|dx|≤500` 이면 타이머 누적, **2.0 s** → Throw. 탄 투하 후 쿨은 상태 리셋으로 사실상 `2 s + 0.88 s` 주기.
- Throw(`SD_Hang_Throw_Bomb` 11f×.08=.88 s): **f9(0.72 s)** 에 SoldierBomb 생성 `(0,−20)`→발 기준 `(0,−57)`, 전방 vx 200, 위 §1.1 폭탄과 동일 (중력 750, vy0 −500). 종료→Idle.
- 사망: 중력 ON `SD_Die_Jump` → 착지 `SD_Die_Land` → 2 s 깜빡/3 s 제거. 착지 못하면 5 s 후 제거. (바닥 고체에 닿지 않는 위치라 보통 추락/제거.)
- 스폰: 수동 배치 `(5735,592)`, `(5865,592)` (`DevScene.cpp:1279-1287`), `SetFaceLeft(true)`. 활성 margin 은 쓰지 않음(탐지 거리만).

### 1.4 FlyingBzk (낙하산 바주카) — `FlyingBzk.*`
| 항목 | 값 |
|---|---|
| 충돌 | `box 40×75 @(0,−37.5)`; 낙하산은 별도 Animator `Bazuka_Para_Idle`(9f×.08 루프) offset `(−5,−50)`. HP 1, 점수 100 |
| 이동 | 중력 OFF, **하강 40 px/s + 수평 ±20 px/s**. 수평 방향은 0.8 s 마다 50% 확률로 반전(초기 LEFT) |
| 활성 | margin 150, 활성 전 정지 |
| 발사 | 1.6 s 주기. **화면 안(0≤x≤W, 0≤y≤H)에 있을 때만** 발사 |
- 에임: 플레이어 방향각 `dgr` 으로 5구간 자세(중앙 ±10° 창): F(0°)/D1(−25)/D2(−50)/D3(−75)/D4(−90), 창 사이 간격에서는 직전 자세 유지. 바라보는 쪽은 `_dgr` 부호(−90 미만→LEFT). 플레이어가 위에 있으면 수평으로 쏨. 회전 속도 1500°/s(사실상 즉시). 발사각 = 0/22.5/45/67.5/90° **아래** (`ShotHandler<>`).
- 클립: 준비 `Bazuka_Fly_Shot_Ready_0..4`(1f), 발사 `Bazuka_Fly_Shot_0..4`(2f×.08), **f1** 발사 → 종료 후 같은 자세 Ready. 투사체/머즐 = SoldierBzk 와 동일.
- 착지 변신: 발 아래 `(0,+37)` 픽셀이 고체가 되면 `SoldierBzk` 를 같은 위치에 생성, `Bazuka_Para_Destroy`(17f×.08) 이펙트 `(−5,−50)` (배경 레이어), 자신 제거 (`FlyingBzk.cpp:364-390`). 단 한 번만.
- 사망(착지 전): 낙하산 파괴 이펙트+디브리 `Bazuka_Debics`, `SD_Die_Jump`(중력 800 ON) → 착지 `SD_Die_Land`, **사망 시점부터 2 s 깜빡 / 3 s 제거**.
- 스폰: `ParaBzk` = `(camRight−80−rand(120), camTop+30)` 웨이브 (`DevScene.cpp:1069`). 낙하 ≈ 400 px/40 = 10 s 동안 최대 6발. 권장: 상한 3발(과도) — 튜닝 항목.

### 1.5 Shielder (방패병) — `Shielder.*`, `ShielderProj.*`
| 항목 | 값 |
|---|---|
| 충돌 | `box 40×75 @(0,−37.5)` (`_shd` 방패 박스는 선언만 하고 생성 안 함). HP 1, 점수 100, 디브리 `Shield_Trash`(8f×.08 루프) |
| 물리 | 중력 1000, 이동 100 px/s (점프 상태는 정의만 있고 AI 미사용), `DETECT=250`, `GUARD=50`, `COOLTIME=1`, `BULLET=3` |
- **방어**: `_block && 정면` 일 때 플레이어 **총알**(Projectile 타입, HITBOX)을 막고 `Shield_Defence`(4f×.1=.4 s, 위/아래 변형 `_Up/_Down`)→원 상태. 정면 판정: Forward=탄의 x 가 바라보는 쪽, Up=탄 y<자신 y, Down=탄 y>자신 y. `_block` 은 Idle/Look*_Start 에서 true, **Shot_Ready 이후 false(사격 중 취약)**, Idle 복귀 시 true (`Shielder.cpp:200-203,296`). 막지 못하면 즉사.
  - 새: 방패는 `Bullet` 종류(권총/HMG/샷건 펠릿)만 막고 **`Melee`/`Blast` 는 방패 무시**(칼·폭탄은 방패병 처치 수단). 레거시는 칼이 아예 안 닿는 버그라 의도 추정.
- 상태머신(시간 s):
| 상태 | 클립 | 전이 |
|---|---|---|
| Idle | `Shield_Idle` 6f×.1 루프 | 활성 후 타이머. ≥1 s: `|dy|≤50` → {`|dx|≥250` Move / 50% Idle(타이머 리셋)·50% Shot_Start}; `dy<−100` → {`|dx|≤50` LookUp_Start / Move}; 그 외(아래) → {`|dx|≤50` LookDown_Start / Move} (※ −100<dy<−50 구간도 "아래" 취급 = 레거시 구멍) |
| Move | `Shield_Move` 12f×.1 루프 | 100 px/s. ≥2 s: 50% MoveStop / 50% Move 재시작. `|dx|≤250 && |dy|≤100` → Shot_Start. `|dx|≤50` → LookUp(dy<−100)/LookDown |
| MoveStop | `Shield_Move_Stop` 4f×.08 | →Idle |
| Shot_Start | `Shield_Shot_Ready` 5f×.08 | ≥**0.5 s** → Shot_Ready, `bullet=3` |
| Shot_Ready | `Shield_Shot_Idle` 1f | ≥**0.333 s**: bullet==0 → Shot_End, else Shot (`bullet--`) |
| Shot | `Shield_Shot` 6f×.08=.48 s | **f1** 탄 생성; 종료→Shot_Ready |
| Shot_End | `Shield_Shot_End` 5f×.08 | →Idle |
| LookUp/Down_Start·Idle·End·Block | `Shield_LookUp/Down_Start`(3f×.08), `_Idle`(6f×.1 루프), `_End`(3f×.08), `Shield_Defence_Up/Down` | Idle(look) ≥1 s: `|dx|≤50` → `Shot_Ready_Up/Down`→3연발 **수직 사격**, 아니면 End→Idle |
- 3연발 한 세트 ≈ 0.5 + 3×(0.333+0.48) + 0.333 + 0.4 ≈ 3.7 s.
- **ShielderProj**: `box 25×25`, 속도 200 일정, 클립 `Shield_Proj`(2f×.08 루프), 머즐(레거시→발 기준): 전방 `(±50,−65)`, 아래 `(0,−8)`, 위 `(0,−97)`. 이펙트 `Shield_Proj_Effect_{Left,Right,Up,Down}`(6f×.08). 소멸 시 `Explosion_Small`(0,0).
- 스폰: 다리 도로 위 `topY=405` (`ShielderAt`, `DevScene.cpp:1229-1253`). 방패병이 도로에서 떨어지지 않게 **발판 가장자리 감지**가 필요(레거시는 Move 가 벽 감지 없이 전진 → 낙하 가능).

---

## 2. 차량/전차/헬기

### 2.1 Jeep (이동 로켓 지프) — `Jeep.*`, `JeepProj.*`
| 항목 | 값 |
|---|---|
| 충돌 | `box 75×60 @(0,−30)`, F=30. 픽셀 지면 추종(하향 8 px 스냅) + 일반 중력 |
| HP/점수 | 레거시 60 → 새 **80**(TTK 8 s). 점수 500. 사망 시 `Explosion_Huge`×2, 금속 디브리 5 |
| 이동 | 200 px/s |
| 활성 | margin 150(`_entered` 래치). 활성되면 화면 좌측 `screenLeft+75` 로 주행 시작 |
- **방향 규약(중요)**: 클립 접미 = 레거시 `_dir`. `_Left`: **좌(−x)로 주행**, **우(+x)로 발사**(후방 발사). `_Right`: 우 주행 / 좌 발사. 발사 가능 조건 = 플레이어가 **발사 쪽**(`dir==Right && player.x<x` 또는 `dir==Left && player.x>x`)에 있을 때. (FixedJeep 주석 "기본 `_Right`=좌측 발사"와 일치.) *스프라이트 시각 확인 필요(§9 불확실성).*
- 상태머신:
| 상태 | 클립 | 동작/전이 |
|---|---|---|
| Idle | `Jeep_Idle` 2f×.08 루프 | 1 s 후: 플레이어가 발사 쪽 → Aimming / 아니면 Turn |
| Move | `Jeep_Move` 8f×.08 루프 | 200 px/s, 목표 x 와 `|Δx|≤10` → Turn |
| Turn | `Jeep_Turn` 12f×.08=.96 s | 종료 시 방향 반전 → Idle |
| TurnToMove | `Jeep_Turn` | 목표가 반대편일 때 선회 후 Move |
| Aimming | `Jeep_Aimming` 12f×.08=.96 s | →Shot_Idle, `bullet=4` |
| Shot_Idle | `Jeep_Ready` 2f 루프 | bullet∈{4,3,1} → 즉시 Shot_Ready; bullet∈{2,0} → **2 s 대기** 후 (2: Shot_Ready, 0: ReAimming) |
| Shot_Ready | `Jeep_Shot_Ready` 12f×.08=.96 s | →Shot |
| Shot | `Jeep_Shot` 8f×.08=.64 s | **f1(0.08 s)** 로켓 생성, 종료 시 `bullet--` →Shot_Idle |
| ReAimming | `Jeep_ReAimming` 12f×.08 | →TurnAfterShot |
| TurnAfterShot | `Jeep_Turn` | 반전 후 IdleAndMove |
| IdleAndMove | `Jeep_Idle` | 1 s 후 Move, 목표: 플레이어가 왼쪽이면 `screenLeft+75` 아니면 `screenRight−75` |
| Destroy | `Jeep_Destroy` 17f×.08=1.36 s | 즉시 `Explosion_Huge`(0,−20)(배경층)+충돌 OFF, **1.7 s** 후 `Explosion_Huge`+금속 디브리 5 → 제거 |
- 사격 패턴: 4발 = (Ready→Shot)×2 연속, **2 s 휴지**, (Ready→Shot)×2, **2 s 휴지**, ReAim. 1회 조준당 ≈ `0.96+4×1.6+4+0.96+0.96 ≈ 13.3 s`, 발사 쌍 간격 1.6 s.
- **JeepProj**: `box 50×20`, 속도 **300 일정**, 중력 없음. 생성 후 `Jeep_Proj_Begin`(5f×.08=.4 s) 동안 **정지(점화)**, 이후 `Jeep_Proj`(3f×.08 루프)로 비행. `|dy|>50` 이면 방향에 ±0.1 y 기울임(스폰 시 1회, 정규화) → 약 ±5.7°. 0.1 s 마다 `Rocket_Smoke`(16f×.06) `(+15,−15)`. 소멸: 픽셀/WALL/플레이어 → `Explosion_Mid`(0,−35).
- 머즐(발 기준): 레거시 `(+26+20k, −58)`(우발사) / `(−75−20k, −58)`(좌발사), `k=bullet&1` → y = **−88**. 비대칭은 애니 오프셋(25,0) 때문 → 권장: 아트 중심 기준 `(±(51+20k), −88)`, 박스를 아트에 맞춤.
- 스폰: `{x, ground}` 지면, `RIVER_MID`(6000,g−30), 헬기와 함께, 화면 오른쪽 밖에서 활성 시 입장 (`DevScene.cpp:1302-1310`).

### 2.2 FixedJeep (고정 지프) — `FixedJeep.*`  ※ Stage0 미배치(레거시 데드 코드)
- Jeep 과 동일 몸체/탄/클립/HP(60→80)/점수 500. 이동 없음.
- 상태: Idle(매 프레임) `|dx|≤450` → 발사 쪽이면 Aimming 아니면 Turn → Detech(2 s) → 판단 반복. Aimming→`bullet=4`→Jeep 과 동일 사격 패턴→ReAimming→**Detech(2 s)**(TurnAfterShot 없음). `SetFireDirection(true)` 가 좌발사.

### 2.3 BTank (고정 포탑 전차) — `BTank.*`, `BTankProjectile.*`
| 항목 | 값 |
|---|---|
| 충돌 | `box 100×100 @(0,−50)` (레거시 offset (0,25)), **F=75**. 항상 좌향, 이동 없음 |
| HP/점수 | 레거시 80 → 새 **120**(TTK 12 s). 점수 1000 |
| 활성 | **플레이어 x ≥ 전차 x − 500** (카메라 마진 아님) |
- 상태: NotOpen(`BTank_Idle` 1f) → Opening(`BTank_Animing_Up` 8f×.12=.96 s) → Idle(마지막 프레임 유지) **3 s** → Shot(`BTank_Fire` 11f×.08=.88 s, **f2(0.16 s)** 포탄) → Idle. **`doubleShot` 토글: 첫 발 후 즉시 2발째(Idle 대기 없이), 그 후 3 s 대기** → 3 s+0.88×2 ≈ **4.8 s 주기, 2연발**. 발사 시 이펙트 `BTank_Shot_Effect`(16f×.08) 를 전차 원점에 생성(클립 피벗이 포구 쪽으로 오프셋됨).
- **BTankProjectile**: 발 기준 머즐 `(−80,−95)`, **vx=−400**, vy0=−200, 중력 500(최대낙하 500) → 정점 +40 px 후 낙하, 지면 높이에서 사거리 ≈ **450 px**(체공 1.13 s). `box 20×20`, 클립 `BTank_Proj`(3f×.05 루프)+`BTank_Proj_Effect`(9f×.05) 를 클립 f0 마다 꼬리로 생성. 소멸: 픽셀/GROUND(윗면 제외)/WALL/플레이어 → `Explosion_Mid`(0,−35).
- 사망: 즉시 `Explosion_Huge`(0,+35 레거시)+금속 디브리 5, 충돌 OFF, `BTank_Destory`(8f×.12=.96 s, 마지막 프레임 잔해 유지), **4 s 후 제거**.
- 스폰: `OPENLAND_3` `(11900, g−75)` (`DevScene.cpp:1443`).

### 2.4 MissleTank (미사일 전차) — `MissleTank.*`, `MissleTankProjectile.*`
| 항목 | 값 |
|---|---|
| 충돌 | `box 100×100 @(0,−50)`, F=50. 항상 좌향, 이동 없음(이동 클립은 있으나 미사용) |
| HP/점수 | 레거시 50 → 새 **100**(TTK 10 s). 점수 1000 |
| 활성 | `|dx|≤600`(FAR) 동안만 타이머 누적, 벗어나면 리셋 (래치 아님). 카메라 activation 은 쓰지 않음 |
- 조준 모드: `Aim ∈ {DOWN(직사), UP(곡사)}`, 초기 DOWN. Idle 에서 1 s 누적 후: `(|dx|>300 && DOWN)||(|dx|≤300 && UP)` → Aimming(`MissleTank_AimmingUp/Down` 9f×.08=.72 s, 종료 시 모드 토글) → Idle. **원거리(>300)=곡사 UP, 근거리=직사 DOWN**.
- 사격: Idle 조건 충족 시 Ready_Shot(`*_Ready_Shot` 3f×.08 루프) **2 s** → `bullet=5` → Shot(`*_Shot` 7f×.08=.56 s, **f2(0.16 s)** 미사일) 반복 5회(Ready_Shot 은 즉시 Shot 으로 넘어감) → Idle. 주기 ≈ 1+2+5×0.56 ≈ 5.8 s.
- **MissleTankProjectile**: `box 25×25`, vx=−300, 중력 400. DOWN 모드: 머즐 `(−75, −90+rand[0,24])`, vy0=0, 클립 `MissleTank_Proj_1`(7f×.1, 비루프) → 사거리 ≈170–200 px. UP 모드: `(−50, −180+rand[0,24])`, vy0=−200, 클립 `MissleTank_Proj_0`(12f×.1) → 사거리 ≈455–470 px. 소멸: 픽셀/GROUND(윗면 제외)/WALL/플레이어 → `Explosion_Mid`(0,0). (클립은 방향 접미 없음.)
- 사망: `MissleTank_{Up,Down}_Destroy_{L}`(20/21f×.08), **즉시 폭발 없음**, **2 s 후** 금속 디브리 6 + `Explosion_Huge`(0,+20) → 제거. 충돌 OFF 는 즉시.
- 스폰: `OPENLAND_1` `(10900, g−50)`.

### 2.5 Sarubia (보행 전차, 중간보스 게이트) — `Sarubia.*`, `SaruProjectile.*`
| 항목 | 값 |
|---|---|
| 충돌 | `box 100×100 @(0,−50)`, F=50 |
| HP/점수 | 레거시 80 → 새 **150**(TTK 15 s, 게이트 보스). 점수 1000 (권장 2000) |
| 이동/탐지 | 100 px/s, 탐지 500, 쿨 2 s, 활성 margin 150 |
- 상태(시간 s): Idle(활성 후 `_timeSum`≥2): 플레이어가 반대편 → Turn(`Sarubia_Turn` 11f×.08=.88 s, 종료 후 방향 반전) ; `|dx|>500` → MoveStart(`Sarubia_Move_Start` 6f×.08) → Move(`Sarubia_Move` 6f×.08 루프, 100 px/s, **2 s 경과 또는 `|dx|≤400`** → MoveEnd(`Sarubia_Move_Stop` 6f×.08)→Idle) ; 그 외 → Shot_Ready(`Sarubia_Shot_Start` 7f×.08, 상태는 **1.5 s** 유지) → Shot(`Sarubia_Shot` 12f×.08=.96 s, **f1** 사격) → Idle.
- **SaruProjectile (롤링 수류탄)**: 머즐 `(±60,−150)`, 이펙트 `Sarubia_Proj_Effect_{L/R}`(전차 위치). `box 25×25`. Begin(`Sarubia_Proj_Start` 8f×.08): **수직 낙하**(중력 300, 최대 300, 수평 이동 없음) → 착지 → Collision(`Sarubia_Proj_Collision` 6f×.08=.48 s) → **지면을 따라 100 px/s 로 굴러감**(`Sarubia_Proj_{L/R}` 21f×.08 루프, 중력 유지, 벽에 닿아도 소멸 안 함) **5 s** 후 `PlayerBombExplosion`(27f×.02)(0,+10) 이펙트(무해)로 소멸. 플레이어 접촉 시 소멸+플레이어 사망. 플레이어 총알에 파괴되지 않음.
- 사망: 즉시 `CreateExplosionEffect(pos,(0,−50),50, S1 M1 L1)`, 충돌 OFF, `Sarubia_Destroy`(4f×.12 유지), **2 s** 후 금속 디브리 6+`Explosion_Huge`(0,+20)→제거.
- 스폰: `OPENLAND_4` `(12790, g−50)` + **카메라 게이트**(`OpenGate`): 처치 시까지 카메라 정지(화면 영역 고정). 새: `gate:{kill:[id]}`.

### 2.6 Aparch (헬기, 중간보스 게이트/일반) — `Aparch.*`, `AparchProj.*`, `AparchDropProj.*`
| 항목 | 값 |
|---|---|
| 충돌 | `box 100×100` 중심 원점 (**비행: 원점=중심 유지**, 지면 개념 없음), 충돌체 `CL_MONSTER` |
| HP/점수 | 레거시 50 → 새 **120**(TTK 12 s). 점수 1000 (게이트면 권장 2000) |
| 렌더 | 본체 `Aparch_Idle/Move/MoveStop/Destroy_Left`(`_Right` 쌍 존재, **항상 Left 사용**), 로터 `Aparch_Flight_Left4`(5f×.04 루프) offset (−18,−54), 포 `Aparch_Gun_Left`/`Aparch_Gun_End_Left`(9f×.08) offset (−6,45) |
| 활성 | margin **400**, 활성 시 화면 좌표 `(W/2+125, 125)=(445,125)` 로 진입 |
- 이동: 목표점까지 300 px/s, 남은 거리² ≤10000 이면 `v = max(50, v − 600·dt)`, 거리² ≤400 이면 정지→`Aparch_MoveStop`(6f×.06). 진입 직후 `_timeSum=1`.
- 패턴 (Idle **2 s** 후, `_patternCnt` 홀/짝 번갈아, 시작 패리티 랜덤):
  - **미사일 패턴**: 포 전개 `Aparch_Gun_Left` **1 s** → 화면 `(W−100, H−200)=(540,248)` 로 이동 **1 s** → 3발: 각각 직전으로부터 **1 s** 간격, 스폰 `pos+(−30,+50)`, 이펙트 `Aparch_Missle_Effect_Left`(10f×.08) `(−30,+45)`, 3발 후 1 s → Idle, 포 수납 `Aparch_Gun_End_Left`, `(W−100,100)` 로 복귀. ≈ 6 s.
  - **폭탄 투하 패턴**: 플레이어 위 추종(`v = (player.x−x, player.y−y−275)·1.5/s`), 0.5 s 틱 `t` 증가, `(t%7)&1` 일 때 `AparchDropProj` 투하(투하 시 `t+=1`). 결과: 투하 시각 **0.5/1.0/1.5, 2.5/3.0/3.5, 4.5/5.0/5.5 s** (3발×3묶음, 묶음 사이 1 s), **6.0 s** 종료→Idle. 스폰 `(10,60)`, 이펙트 `Aparch_DropMissle_Effect`(8f×.06).
- **AparchProj**: `box 50×20`, 속도 300 일정(좌향), `Aparch_Missle_Left`(2f×.08 루프) 아트 오프셋 (25,0), ±0.1 y 기울임(|dy|>50), 스모크 0.1 s. 소멸: 픽셀/플레이어(`WALL` 포함) → `Explosion_Mid`(0,−35).
- **AparchDropProj**: `box 30×40`, 정지에서 시작, **중력 300 px/s²**(`vy=300t`), 클립 `Aparch_DropMissle`(12f×.08 루프). 소멸: 픽셀/GROUND(윗면 무시)/WALL/플레이어 → `Explosion_Mid`(0,−35).
- 사망: `Aparch_Destroy_Left` + 이동 `(50,−70)` px/s 로 표류, 충돌 OFF, **1.5 s 연쇄 폭발**(0.3 s 마다 `S+M` at `(−30,−20)+rand[0,100)`, 총 6회) → `Explosion_Large`+금속 디브리 5 → 제거.
- 스폰: `RIVER_MID` `(camRight+250, camTop+125)` + **게이트**(처치까지 카메라 고정), `OPENLAND_3` 일반(게이트 없음). 

### 2.7 Alian (비행 외계인) — `Alian.*`, `AlianProjectile.*`  ※ Stage0 미배치
- 원점=중심, 충돌 `box 100×100`(`CL_MONSTER`, 지면 없음). 레거시 HP 10(탄 1발=1) → 새 40(TTK 4 s). 점수 100. 활성 margin 150.
- 상태: Move(진입 시 카메라 기준 목표 `x∈[50,W−50], y∈[50,H/2]` 랜덤, **속도 = 5×거리**(지수 접근, τ≈0.2 s), 거리≤1 px→Idle) → Idle 1 s → 50% Attack / 50% Move. Attack: 이펙트 `Alian_Shot_Ready`(13f×.08=1.04 s)를 몸에서 재생, **f7(0.56 s)** 에 `AlianProjectile` 생성 `(0,+35)`(원점 중심 기준), 이펙트 종료 → Idle.
- 투사체: `box 25×25`, 속도 300, 플레이어를 향한 단위벡터, 방향별 클립 `Alian_Bullet_1..9`(3f×.08 루프) = `idx = floor((−atan2(dy,dx)+180)/20°)`(하반구만, 위쪽이면 수평 클립 클램프). 소멸: 픽셀/GROUND/WALL/플레이어 → `Explosion_Mid`.
- 사망: HP≤0 → 충돌 OFF, `Alian_Die`(24f×.08=1.92 s) 후 제거.

---

## 3. 중간보스/보스

### 3.1 Bus (버스 중간보스) — `Source/Game/Enemies/Bus.cpp`, def `mb_truck` (`Assets/Defs/Boss.json`), 스프라이트 `Assets/Source/33825.png` (컷 맵: `Docs/BusSheet.md`)
사양 근거: `Docs/Spec_Bus.md` (원작 영상 실측). 이 절은 구현 기준이다.

| 항목 | 값 |
|---|---|
| 원점/충돌 | 스폰 `(2995,255)` legacyCenter (발 y 385). 원점 = 발 중앙. 벽 `solid 204x260` (발 기준 cx 0, cy -130) 은 **살아 있는 동안만** (`GetSolidBoxes` 는 Alive 만). 사망 (Dying) 후 벽이 사라져 잔해 위를 걸을 수 있다. 피격 박스 = `hurtbox` (204x272) |
| HP / 점수 | **380** (1P HMG 2 dmg x 20 발/s = 40 dps, 전투 9.5 s). 점수 2000. `hp/8` 마다 한 단계 |
| 등장 | `bus_create` (x>2244): 버스 스폰 (주차, 벽, 무적), `setGround Stage0_1` silent. `bus_start` (x>2560): `lockCamera [2513,0,3153,448] glide 1.9` (팬 1.9 s 후 카메라 x 2513 고정), `command startBattle`, `gate bus_dead` |
| 주차 / 팬 | 주차 중 (상태 kParked, kReveal) 피격은 흡수 (`OnHit_`, 피해 없음). startBattle 은 `BehaviorOps.onCommand` 로 전달 (`EnemyWorld::SendCommand`) |
| 개방 | 팬 종료 뒤 `Bus_Opening` 10 프레임 @7 fps (1.43 s). 개방 직후부터 피격 판정 (`RegisterHurtboxes` 는 기본 경로, 피해는 `OnHit_` 가 결정) |
| 전투 | 포탄/병사/GO! 없음 (원작에서 확인되지 않음). 플레이어 공격만 |
| 파손 단계 (stage = `hp/8` 기준) | 0 `Bus_Open` / 1 `Bus_Damaged_1` / 2 `Bus_Damaged_2` + `Bus_WinLeftUp` / 3 `Bus_Ladder_2` + `Bus_WinRightUp` + `Bus_Deco` / 4 `Bus_Ladder_3` + `Bus_WinLeftDown` / 5 `Bus_Damaged_3` + `Bus_BigWin_Begin`→`End` + `Bus_WinRightDown` / 6 `Bus_Smoke` (루프) + `Bus_Bump` + 폭발 Small / 7 `Bus_Smoke` + `Bus_Sideparts` + 폭발 Mid, Large |
| 사망 | `Died` 이벤트 1회. 연쇄 폭발 (Small 2, Mid 4, Large 4, 0~0.5 s, 중심 발 + (-25,-150)). 0.5 s 후 `Bus_Wreck` (398x222) + `Bus_WreckFront` (발 + 12). 잔해는 남고 (`deathDone` 즉시 true), 게이트 `bus_dead` 가 즉시 열려 `unlockCamera` + `setGround Stage0_1` |
| 구현 파라미터 | `BusCfg { revealTicks 114, openTicks 86, chainTicks 30 }` = `Boss.json` params `revealSec 1.9 / openSec 1.43 / chainSec 0.5` |
| 원작 대비 | 원작 전투 9.5 s, HP 250 (2P 기준). 이 프로젝트 HMG 는 원작보다 연사가 빨라 (20 vs 7.7 발/s) 같은 9.5 s 를 맞추려고 HP 380 을 썼다. 개방은 원작 3.5 s 에서 1.43 s (시트에 있는 10 프레임만 사용). |

### 3.2 Boss Shiee (최종보스, MS2 Mission 4) — `Source/Game/Enemies/BossShiee.*`, def `boss_shiee` (`Assets/Defs/Boss.json`)

구 Big Shiee (`BigShiee.*`, 레일 주행/포탑 연사 방식) 는 **삭제**했다. 원작 영상 `Assets/Source/videoplayback (2).mp4` (t 250–378 s) 의 실측 (`Docs/Spec_Boss.md`) 으로 새로 짰다. 스프라이트: `Boss_*` 105 클립 (`Docs/BossSheet.md`). 위치 실측과 파츠 배치: `Docs/BossLayout.md`. 수치는 모두 def `params` (틱/초 단위는 `*Sec` 키).

**선체 / 카메라 (슬라이딩 아레나)**
| 항목 | 값 |
|---|---|
| 원점 | 발(바닥) − (0, 208). 발 = 지면 1286 (아레나 바닥선). 선체 아트 542x416 중심 = 원점 |
| 등장 | 카메라 왼쪽 밖 (`entranceFromX` −260, 화면 좌표) 에서 4 s 굴러 들어옴 (`Boss_Body_DriveForward`, 기저 폭발 0.5 s 주기). 정지 `stationX` 330 (카메라 왼쪽 기준) |
| 피격 | 선체는 **파괴되지 않는다**. 탄/폭탄은 흡수 (체력 변화 없음, 부위 플래시도 없음) |
| 자동 전진 | 전투 중 (`assault`, `cannon` 단계) `scrollSpeed` 18 px/s 버스트 `scrollOnSec` 3 s / `scrollOffSec` 2 s. 요청량만큼 `EnemyWorld::RequestCameraScroll` → `StageState` → `StageRunner::AddCameraScroll` → `Camera::AddScrollFloor` (카메라 중심 하한, 매 틱 강제). 선체도 같은 양만큼 전진해 **화면상 정지**. 총량 `scrollBudget` 700 px |
| 아레나 | `rects.bossArena` = [12700, 864, 14094, 1312] (잠금 permanent). 보스 트리거 x 12900 (openland_4 게이트 뒤) |
| 지형 | `Assets/Source/GroundEdits.json` (SlugBaker `--ground-edits`): `Stage0_Pixel_0` 의 (12744,1150)–(14094,1286) 고체 언덕 제거 → 바닥선 y 1286 평탄 (잔존 언덕 305x82 포함) |

**파츠 (각자 체력, 파트 번호)**
| 파트 | 체력 | 상자 | 비고 |
|---|---|---|---|
| 포탑 A..G (0..6) | 120 each (합계 840) | 64 폭 x (포탑 중심 − 25 .. 발) 의 기둥 | 아래에서 쏜 탄도 선체보다 먼저 맞는다. 파괴 시 `Boss_Turret_X_Destroyed` 잔해 + Explosion Small/Mid + `PartDestroyed` 점수 500 |
| 스컬 캐논 헤드 (7) | 1000 (= `Enemy::health`, def `hp`) | 100 폭 x (헤드 − 75 .. 발) | 포탑 4기 파괴 (`deployAfterDown`) 또는 전투 30 s (`assaultMaxSec`) 뒤 전개 후에만 맞는다. 피격 시 부위 플래시 (황금 깜빡임), 파괴 시 `PartDestroyed` 점수 2000 |

포탑 사격 (단계 기계, 한 번에 한 기 예고): `Turning` 0.75 s (`Boss_Turret_X_Turning`) → `Shooting` (`Boss_Turret_X_Shooting`, 발사 0.5 s 째 = 예고 합계 1.25 s) → 청록 트레이서 1발 (발사 시점 플레이어 중심을 조준, 리드 없음, 부채꼴 없음), 속도 `streakSpeed` 200 px/s (≤ 260), 판정 14x14, 수명 1.6 s, 피해 즉사. 사격 주기 `turretGapMin/MaxSec` 3.6–4.6 s, 첫 발 `turretFirstSec` 2 s. 발사 플래시 `Boss_Fire_ABCD_1` (A–D) / `Boss_Fire_EFG_1` (E–G).

**캐논 (스컬)**
| 단계 | 시간 | 내용 |
|---|---|---|
| 전개 `kDeploy` | 2.67 s (160 틱) | `Boss_Cannon_Deploy1 → 3 → 6 → 9` (각 40 틱). 전개 후 `Boss_Cannon_Idle` (황금 헤드) |
| 볼리 `kCannon` | 전개 +1 s 첫 볼리, 이후 2.5–3.5 s (`volleyMin/MaxSec`) | 예고 `Boss_Gun_Aim` 0.75 s → 트레이서 3발 (0.25 s 간격, 부채꼴 6° 칸, `Boss_Gun_Shoot_k`) |
| 주황 포탄 | 볼리 2 번째마다 1발 | 비행 1.0 s 고정 (g 1626), 착탄점 = 발사 시점 플레이어 발 x, 착탄 시 `boss_blast` 60x60 0.3 s |
| 격앙 | 캐논 HP ≤ 50 % | 볼리 간격 ÷1.25, 트레이서 +1 |

**사망 연출 (Dying, 10 s)**: 0–2.5 s 캐논 폭발 (Explosion Mid 12 틱 주기, Huge 1회) → 2.5–9.5 s 선체 연쇄 폭발 (Small/Mid/Large 0.2 s 주기, 연기 `Boss_Smoke` 0.5 s) → 7 s 분리 `Boss_Destroyed_Body` → 9.5 s 잔해 `Boss_Wreck` 고정. 10 s 에 `IsDestroyFinished` → 스테이지 게이트 `boss_dead` → 무적 10 s → `endStage` (RECAPTURED PRISONER / 결과 / MISSION COMPLETE). 캐논 사망 시 `Died` 점수 10000 (def `score`).

**HP 바**: `BehaviorOps::bossHpRatio` 훅 (EnemyWorld::GetBossHpRatio 일반화). 포탑 + 캐논 체력 합 / 최대 합 (전부 살아 있으면 1840/1840, 캐논만 남으면 1000/1840). 등장 전과 사망 후는 숨김.

**보조 기능 (구현 메모)**
- 파츠 상자는 파츠 중심에서 발까지 이어지는 기둥 (`RegisterHurtboxes_`): 플레이어가 파츠 아래에서 위로 쏘면 선체 몸통보다 먼저 파츠에 닿는다. 선체 상자는 마지막에 등록 (HitResolver 는 먼저 맞은 상자 하나에만 피해).
- 체력 0 이 된 살아 있는 적은 `EnemyWorld::Tick` 에서 사망 처리 (generic). 캐논 체력은 `Enemy::health` 로 관리.
- 피격 플래시는 `TriggerPartHitFlash` (파트 0..7), 포탑/캐논 개별.

**검증 (`Test/BossShieeTest.cpp` 12, `BossDefTest.cpp` 9, `BossBotTest.cpp` 4 보고 + 2 기준, `CameraTest.cpp` 3 추가, `StageDataTest` 1 추가)**
- 등장 (카메라 왼쪽에서 들어와 정지), 포탑 예고 → 발사, 트레이서 속도/조준, 포탑 파괴 점수 1회와 선체 불변, 포탑 4기 → 전개 → 볼리, 주황 포탄, 캐논 피격/사망/연출 10 s, 플레이어 사망 시 무공격, 스크롤 버스트/예산, 선체-카메라 동기, HP 바 공식.
- 봇 (`BossBotTest.cpp`, 시드 1–5, 결정적): 권총+폭탄 5/5 승 (사망 ≤ 2), 보급 POW 구출 4/5, 권총 단독 0/5 (보고만, 고착 0), 좌우 회피만 3/5. 고착 0 (전 변형).

**원작 대비 (설계값)**: 포탑/캐논의 정확한 위치 (`Docs/BossLayout.md` 는 육안 합성), 트레이서 200 px/s (원작 속도 미측정), 주행/스크롤 총량 700 px (원작 1021 game px 보다 작다, 설계 보정), 권총 3 dmg 기준 HP (권총 단독 실측 전투 약 45–55 s, 목표 60–90 s 에 근접). 사망 연출은 원작 340–350 s 에 맞췄지만 화면 어둡기 (luma) 는 없다.
### 3.3 기타 비행/탈것
- 설계 외 `Plane`, `Parker`, `Stage0Lake`, `Sign` 은 장식/연출(§6).

---

## 4. 중립/소품 NPC

### 4.1 Truck (트럭 3종) — `Truck.*`
- 파괴 가능한 장애물+발판. 생성자에서 `rand()%3` 로 DEFAULT/LOADED/REFRI 결정. 클립 `{Truck|LoadedTruck|RefTruck}_{Idle,Hit,Down}` (Idle 1f, Hit 8~9f×.06, Down 4f×.08).
- HP 15 → 새 30. 점수 없음.
- 충돌(WALL, 플레이어/총알 flag): 머리 `44×75` offset DEFAULT(22,5)/LOADED(22,15)/REFRI(22,20), 꼬리 DEFAULT `70×25 @(−35,15)`, LOADED·REFRI `70×90 @(−35,10)`. `FinalInit` 에서 LOADED −10, REFRI −17 만큼 y 보정. 스폰 y=342 (지면 378 → F: DEFAULT 36, LOADED 46, REFRI 53).
- 피격 → `Hit`, 플레이어가 위에서 착지(겹침 w>h && 상단 접촉) → `Down` 눌림 모션. HP 0 → `Explosion_Huge`(−10,0)+금속 디브리 6, 제거. 화면 왼쪽 이탈 제거.
- 스폰: x=580,850,1430,1810,2200 (`DevScene.cpp:354-467`).

### 4.2 MobBoat (과일 배) — `MobBoat.*`
- HP 30→60. 충돌 WALL+HITBOX `150×150 @(0,25)`. 스프라이트 `Mob_Boat_Idle/Destroy`(293×248, 중심 피벗).
- 사망: 충돌 OFF, `Mob_Boat_Destroy`, `CreateExplosionEffect(pos,(0,−50),100,S2 M2 L2)`, 목재 디브리 3, **드롭: 랜덤 AnimeScoreItem ×3 + FruitScoreItem ×3**(x±25 랜덤, 바운스 vy−500). `explosion=true`(첫 배) 면 **2 s 후** 2차 폭발(S2 M2 L3)+목재 5 → 제거; false(둘째 배)는 잔해 유지.
- 스폰: `(4140,545)` true, `(4634,545)` false.

### 4.3 Older (POW 노인 NPC) — `Older.*`
| 항목 | 값 |
|---|---|
| 충돌 | `box 50×80 @(0,−40)`, F=40. 레이어 NPC (총알 HITBOX·플레이어 겹침) |
| 드롭 | 필수 지정 `SetItemToDrop`(무기 HM·SG·폭탄/탄약 보급) 또는 `SetRandomScoreItem`(과일·연출 아이템 13종 균등) |
- Stuck(`Older_Idle` 8f×.12 루프, 플레이어 쪽을 바라봄, 중력 O 로 발판에 안착): **플레이어 공격 HITBOX 겹침(총알/칼/폭발 불문)** → Escape(`Older_Escape` 4f×.1=.4 s, 충돌 레이어 DEFAULT) → Idle 순찰.
- Idle 순찰: `Older_Move`(12f×.15 루프) 75 px/s, 방향 반전 간격 처음 2 s 이후 `1+rand(0..2) s`, 낭떠러지에서 `Older_Jump`(7f×.1 루프, 중력 최대 150 = 활강), 지면 위·순찰 ≥1 s·플레이어가 100 px 이상 아래·발밑 공기면 `Older_MoveDown`(13f×.05=.65 s, 100 px/s 하강, 접미 없음) 후 추락. **플레이어와 겹치면** Close.
- Close: `Older_Item`(29f×.1=2.9 s), 진입 시 플레이어 반대로 16 px 보정, **f8(0.8 s)** 에 아이템 생성 `(±18,0)`(바라보는 쪽) → 종료 `Order_Run`(**오타 이름 그대로 baked**, 8f×.1 루프) 으로 **좌측으로 200 px/s** 도주, 2.8 s 부터 깜빡(2 s, 0.2 s 주기), **4 s** 또는 화면 밖 ±50 이면 제거.
- 구출 점수 없음(레거시). 스폰: 시작 `y=100`(공중 낙하 안착) — `(1325,100)MG, (1542,100)MG, (2000,100)MG, (2350,100)폭탄, (3900,342)랜덤, (4890,0)SG, (7200,300)랜덤, (7430,300)폭탄, (7865,300)랜덤, (8190,300)탄약, (8270,300)랜덤, (8900,300)MG`.

---

## 5. 파괴 구조물 (HP 는 "권총 N발 유지" 로 레거시×2)

공통: 총알/폭발/칼 모두 피격(HITBOX). 파괴 시 스프라이트 교체, 부스러기(`WoodDebrics`/`RockDebric`), 폭발, 충돌 제거. 화면 왼쪽 이탈 제거. 점수/드롭 없음(상점 5 제외).
| 클래스 | HP(레거시→새) | 충돌 (스프라이트 중심 원점) | 파괴 처리 |
|---|---|---|---|
| **S0Building** 상점 0~5 | 15→30 | 피격 `INTERACTION` 박스 + 지붕 발판 `GROUND`(플레이어/몬스터/HITBOX 대상). 표 ↓ | `Stage0_Shop_n_1`, 목재 6, 폭발 S/M/L 각 1(±75 랜덤), 피격박스 OFF, 지붕 발판 제거(상점5 예외 아래) |
| **Stage0House** ×5종 | 10→20 | 지붕 `150×30` (`GROUND_EXCEPT_PLAYER`: 적만 위에 섬, 플레이어 통과) + 같은 크기 `INTERACTION` 피격. offset: UPPER(0,3) / OPEN_WINDOW(0,−14) / CLOTH·NO_CLOTH·YELLOW(0,−27). 지붕 윗면 y = `pos.y+offset−15` | 스프라이트 `Stage0_House_n_Destroy`, 목재 6, `S3 M3 L1`(bound 150), 지붕/피격 충돌 제거 → **위의 병사 낙하** |
| **Stage0GroundHouse** | 20→40 | 원 `r=110 @(10,50)` INTERACTION | HP≤10: `Destroy_0`+목재 5; HP 0: `Destroy_1`+목재 5+**픽셀맵 패치**(`Stage0_House_4_PixelMap`, pos−(w/2−12, h/2−5))+`S2 M2 L1` |
| **Stage0BigHouse** (긴 집) | 40→80 | 박스 `310×120 @(10,0)` + 원 r=120 @(−110,60),(120,60) | HP≤20 `Destroy_0`+목재 6; HP 0 `Destroy_1`+목재 6+패치 `Stage0_House_5_PixelMap`(pos−(w/2−12, h/2−9))+`S3 M3 L2`(bound 200). Remove 오프셋 −50 |
| **Stage0GroundMiniHouse** | 10→20 | 원 r=70 @(40,40) + 벽 `20×45 @(85,−1)`(WALL) | 스프라이트 `Stage0_Bridge_3`, 목재 6, `S2 M2 L1`, 원 OFF(벽 유지) |
| **Stage0Tower** | 10→20 | 피격 `90×210`(INTERACTION) + 3 발판 `80×10`(GROUND) offset `(5, 45−70i)` | `Stage0_Tower_Destroy`, 목재 5, 모든 충돌 OFF(위 병사 낙하). 꼭대기 y = pos−100 |
| **Stage0Bridge** 4종 | 10→20 | WALL+HITBOX: BRIDGE `175×100 @(25,0)`, HOUSE `150×100 @(0,0)`, HOUSE2HOUSE `150×100 @(0,−25)`, BLOCK `135×95 @(0,0)` | 대응 파괴 스프라이트(`Stage0_Bridge_0/2/1`, `Stage0_Block_Destroy`). BLOCK 은 바위 부스러기 5, 충돌 `72×91 @(30,0)` 로 축소. 나머지는 목재 5, 충돌 유지 |
| **GuardPost** | 불사 | `GuardPost`(6f×.08 루프) + 발판 `130×10 @(0,−65)`, `165×10 @(0,10)`(GROUND) | 없음 |
| **Pence**(울타리) | 1 | `60×30 @(0,−20)` INTERACTION. 스프라이트 `Pence0..2` 랜덤 | 어떤 플레이어 HITBOX 든 1회 접촉 파괴, 목재 5. 적/플레이어 이동 방해 없음 |
| **Sign** | — | 상점5 파괴 시 `pos+(50,−200)` 에서 낙하(중력 700) → 지면 시 `Stage0_Sign_1`, 0.5 s 후 `L+M` 폭발+목재 6 제거 | |
- **상점 표** (`DevScene.cpp:381-446`; 피격박스 `W×H @offset`(중심 원점 기준), 지붕 `W×H @offset`): 0 `(1274,228)` 150×75@(0,−100) / 220×20@(30,10); 1 `(1489,230)` 150×75@(0,−100) / 220×20@(20,10); 2 `(1661,252)` 100×50@(25,−75) / 160×20@(20,15); 3 `(1906,230)` 150×75@(50,−100) / 300×20@(30,10); 4 `(2224,185)` 200×75@(50,−100) / 250×20@(80,20); 5 `(2513,181)` 150×75@(0,−200) / 230×20@(35,55). 파괴 시 상점5: 간판 `Sign` + `SuppleItem(BOMB)` 바운스 + 지붕 발판을 `220×20 @(47,130)` 로 교체(간판 잔해 발판).
- 픽셀맵 패치: 새 구현 = `GroundMap` 영역 교체(Baker 가 `*_PixelMap` 스프라이트를 `GRND` 패치로 변환 필요 — 현재 baked 는 RGBA 스프라이트: `Stage0_House_4_PixelMap`(독립 PNG 1306×751), `Stage0_House_5_PixelMap`(아틀라스 555×124)).
- 스폰(구조물): 집/다리 좌표 `DevScene.cpp:714-875`, OPENLAND 블록·긴집·타워 `x∈{10098,11122,12146,13170}` 마다 `Block(x,1274)`, `BigHouse(x+372,1257)`, `Tower(x+710,1175)`(마지막 구간은 타워 없음).

---

## 6. 아이템 / 이펙트 / 디브리

### 6.1 아이템 공통 (`ItemObject`)
- `box 32×32 @(0,−16)`(F=16; GunItem/Supple 스프라이트 하단 = 박스 하단), 중력 1400, 지면/벽 착지, 수명 **15 s**(70%=10.5 s 부터 깜빡 2 s/0.2 s), 화면 왼쪽 이탈 제거. 플레이어 겹침 시 습득.
| 클래스 | 클립/스프라이트 | 효과 |
|---|---|---|
| GunItem | 스프라이트 `GunItem_{HM,SG,FS,RL,EC}` + `_Disappear`(44×40) | 무기 교체(HM=헤비머신건, SG=샷건; FS/RL/EC 는 본 게임 범위 밖). 습득 시 `Get_Item_Effect`(6f×.04) + Disappear 후 0.06 s 에 제거 |
| SuppleItem | `Bomb_Supplement`(10f×.1 루프) / `Bullet_Supplement`(10f×.1), `_Disappear` | BOMB +10, BULLET + 현재 총의 extra(MG 100/SG 10/DG 10). `SetBounce`: vy−400, 공중 동안 수평 ±75 px/s(랜덤 방향) |
| FruitScoreItem | `Item_{Apple 500, Banana 300, Banana_Bundle 1000, Watermelon 800, Cabbage 700, Pair 400, Carrot 600}` | 점수. 생성 시 수평 `rand[−50,49]` px/s 로 굴러가다 지면에서 `v−=v·dt` 감속(애니 프레임 지연으로 감속 표현, <1 에서 정지) |
| AnimeScoreItem | `Item_{Mando 3000(Begin→Idle), Letter 1100, Chicken 900, Fish 1800, Bread 1600, Fruits 2500}` | 점수 |
- 점수 팝업 `ItemScoreFont`: 숫자 `Font_Red/Yellow` 를 **0.07 s** 마다 교대, 15틱(1.05 s) 후 위로 상승(가속 −500 px/s²), **40틱(2.8 s)** 제거.
- 드롭 규칙: 랜덤 점수 아이템 = 위 13종 **균등**. MobBoat: Anime×3+Fruit×3. 버스 후(`BUS_END`): 과일 `(3375,200)`+연출 `(3450,200)` 고정 위치 무바운스. 감시탑 `Item_Letter (5800,200)`.

### 6.2 디브리 (`AnimeDebrics`, `MetalDebrics`, `RockDebric`, `WoodDebrics`)
- 오브젝트: 중력 1000, `vy0 ∈ [−800,−301]`(Metal/Rock/Wood) 또는 `[−600,−301]`(AnimeDebrics), `vx ∈ [−250,249]`, 수명 5 s, 화면(±50 px, 하단 +50) 이탈 제거. 충돌 없음, 클립 루프.
- 클립: Metal `Metal1..12`(2~10f×.08), Rock `Rock1..3`, Wood `Wood1..6`, 병사 `Bazuka_Debics`(24f×.03), `Shield_Trash`, `BattleShip_Cannon_Destroy_Type1..7`(1f), `Sarubia_Proj_Trash`(미사용).
- 개수 규칙: 지프 Metal 5, 전차(BTank) 5, 트럭 6, MissleTank/Sarubia 6, Aparch 5, MobBoat Wood 3+5, 구조물 Wood 5~6.

### 6.3 장식 오브젝트 (참고)
`Parker`(`AnimeObject`, (386,341) 배경), `Plane`(원경), `Stage0Lake`(물 애니), `Sewer`(전경 스프라이트 (3356,571)) — 판정 없음.

---

## 7. 권장 HP / 점수 / 게이트 요약
| 개체 | 레거시 HP | 새 HP | TTK(권총 10dps) | 폭탄 발수(20) | 점수(레거시→권장) | 게이트 |
|---|---|---|---|---|---|---|
| Soldier*/Bzk/Hang/FlyingBzk | 1 | 1 | 즉사 | 1 | 100 | - |
| Shielder | 1 | 1(방패 무시 = 칼/폭탄 즉사) | 즉사 | 1 | 100 | - |
| Truck | 15 | 30 | 3 s | 2 | 0 | - |
| MobBoat | 30 | 60 | 6 s | 3 | 0 (드롭 6개) | - |
| Jeep / FixedJeep | 60 | 80 | 8 s | 4 | 500 | - |
| MissleTank | 50 | 100 | 10 s | 5 | 1000 | - |
| BTank | 80 | 120 | 12 s | 6 | 1000 | - |
| Aparch | 50 | 120 | 12 s | 6 | 1000→2000 | 처치까지 카메라 고정 (RIVER_MID) |
| Sarubia | 80 | 150 | 15 s | 8 | 1000→2000 | 처치까지 카메라 고정 (OPENLAND_4) |
| MBTruck (버스) | 160 | 250 | 25 s | 13 | 0→2000 | 처치 시 카메라 해제 |
| Big Shiee 선체 (구 BattleShip) | 250 | 900 | 90 s | 45 | 0→10000 | 아레나 락, 처치 시 게이트 `boss_dead` |
| Big Shiee 포탑 A..G (각) | - | 60 | 6 s | 3 | 0 | 선택 파괴 (선체 HP 와 무관) |
| Alian | 10 | 40 | 4 s | 2 | 100 | - |
| 파괴 구조물 | 10~40 | ×2 | - | - | 0 | - |
> 샷건(6, 관통) 으로는 보스 150발 이상 → 보스전은 권총/HMG 중심, 폭탄 10발(=200 dmg=22%). 밸런스는 `dpsRef` 로 조정.

## 8. 사용 클립 검증 결과
`Assets/Baked/anim.json`(애니)·`sprites.json`(정적)에 대해 위 문서의 모든 클립/스프라이트 이름(약 365종)을 grep/스크립트로 대조: **누락 0건**. 레거시 접미 규칙(`GetAnimeString` → `_Left`/`_Right`) 적용 대상은 양쪽 모두 존재 확인. 예외/주의:
- 센티널 문자열 `Continue`(SoldierBzk OnAir_H: 애니 교체 안 함), `None`(Older) 은 클립이 아님.
- 접미 비대칭: `Aparch_Destroy` 는 `_Left` 만 존재(헬기는 항상 Left 사용, 문제 없음), `Older_MoveDown` 은 접미 없음(코드가 접미를 안 붙임), `Aparch_Turn_{L,R}`/`MissleTank_Proj` 는 스프라이트(코드 미사용/`_Proj_0/1` 애니 사용).
- 이름 오타를 그대로 baked: `Order_Run_{Left,Right}`, `BTank_Destory`, `BattleShip_Head_Swiching_*`, `MissleTank_Proj_*`. **새 def 도 이 이름을 그대로 참조**(리네임 금지, 필요 시 별칭 테이블).
- baked 에 있으나 코드 미사용(후속 재료 가능): `SD_Die_Bomb/Slash`(상태만 존재), `BattleShip_SCannon_*`, `BattleShip_Support_Type1..3`, `BattleShip_Mark_*`, `BattleShip_Switching_Effect`, `Sarubia_Proj_Trash`, `MissleTank_*_Move*`, `Aparch_Drop_Bomb_*`, `Aparch_Flight_{Left,Right}1..7`(Left4 만 사용), `Enemy_Chaser_Bullet.png`.

## 9. 불확실성 / 확인 필요
1. **지프 방향 규약**(좌주행=`_Left`=우발사)은 코드에서만 도출. 스프라이트 시각 확인(컨택트 시트 해상도 부족) 필요 → AnimViewer 로 `Jeep_Move_Left` 가 어느 쪽을 보는지 확인 후 규약 확정.
2. **MBTruck 원점 F≈123, 충돌/스프라이트 정렬**: 스프라이트 하단(원점+146) 과 지면(+123) 이 어긋남. 실제 지면 접촉 기준으로 재조정 필요.
3. Shielder: 폭탄/샷건이 방패에 막히는지는 레거시 불명(폭탄 `Projectile` 타입이면 막힘 가능). 본 문서는 "Blast/Melee 우회"로 정함.
4. (구 BattleShip 항목, 폐기) Big Shiee 파트 오프셋은 `Docs/BigShieeLayout.md` §9 의 캡처로 검증했다.
5. HMG 연사 속도는 PlayerSpec 에 없음 → HMG TTK 미산정.
6. Truck F(LOADED/REFRI)·MobBoat 수면 높이 등 일부는 `DevScene` 좌표에서의 역산값.
7. Aparch 패턴 시작 패리티 `rand()` — 새 구현은 결정적(시드 고정 RNG) 으로.
