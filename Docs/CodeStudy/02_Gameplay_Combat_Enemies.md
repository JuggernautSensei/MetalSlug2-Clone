# 게임플레이 코드 학습: 플레이어, 전투, 적, 소품, 실행 조율

이 문서는 2026-10-14 작업 디렉터리의 실제 소스를 읽은 결과다. 확인한 HEAD는 `0ef7104`다. 게임 실행, 빌드, 테스트, 캡처는 하지 않았다. 함수 본문에서 확인할 수 있는 동작을 설명하며, 작성자나 생성 시점, 개인의 시행착오를 추정하지 않는다.

본문의 **코드 사실**은 현재 구현에 직접 근거한다. **설계 해석**은 그 구현에서 읽을 수 있는 책임 분배를 설명하는 추론이며, 작성자의 의도가 확인됐다는 뜻이 아니다. **역사적 사실**은 이 문서에서 주장하지 않는다. Docs의 이전 완료 기록도 현재 소스의 대체 근거로 사용하지 않았다.

읽는 범위는 Player 14파일, Combat 8파일, Enemies 44파일, Props 8파일, Sandbox 4파일, SandboxState 2파일, StageState/StageWorldHost/StageHost 6파일이다. 합계 86파일, 22,712 물리적 줄의 본문·선언을 확인했다. World/Combat, Constants, StageRunner::Tick, StateFactory와 일부 호출자도 경계를 확인하기 위해 추가로 읽었다.

## 1. 전체 구조를 먼저 설명하기

게임플레이의 시작점은 `StageState::Tick` 또는 `SandboxState::Tick`이다. 이 두 상태가 플레이어, 적, 소품, 투사체, 전투 판정을 어떤 순서로 호출할지 결정한다. 플레이어나 적이 전투 결과를 즉시 서로에게 전달하는 구조가 아니다. 각 시스템이 이번 틱의 상자를 등록하고, CombatSystem이 HitEvent 목록을 만든 뒤 상태가 그 목록을 각 수신자에게 전달한다.

[StageState::Tick](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Stage/StageState.cpp:310>), [SandboxState::Tick](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/SandboxState.cpp:440>), [CombatSystem](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Combat/CombatSystem.h:5>)이 전체 흐름의 출발점이다.

| 담당 | 직접 소유하는 상태 | 직접 하는 일 |
|---|---|---|
| PlayerController | 몸체, 입력 버퍼, 무기·탄 수, 이동 상태, 공격 진행, 목숨·점수 | 입력 해석 후 이동·공격, 플레이어 판정 등록, 죽음·부활 |
| PlayerRig | 상·하체 AnimPlayer, 클립 바인딩 표 | 논리 상태를 표시 클립으로 바꾸고 그림 제출 |
| CombatSystem | 이번 틱 Hitbox/Hurtbox/HitEvent, 증가하는 공격 ID, HitResolver | 등록된 상자 간 피해 이벤트 생성 |
| ProjectileSystem | Bullet/Bomb/Shot/Blast와 연출 요청 배열 | 이동·지면 접촉·수명, 공격 상자 등록, 명중 후 제거 |
| EnemyWorld | Scoped로 소유한 Enemy 객체 배열, ID, 지연 스폰, 이벤트·연출·난수 | 종류별 행동 호출, 생성·제거, 결과 이벤트 관리 |
| Enemy 파생 클래스 | 종류별 진행 값 | 그 적의 상태 전이와 공격 패턴 |
| PropWorld | 소품 인스턴스·충돌 배열·파편·효과·ItemWorld·DecorWorld | 소품 파괴·드롭·충돌·시각 요소 |
| StageWorldHost | 스테이지 ID→실제 월드 핸들 경로, 지면 목록, 콜라이더, 플레이스홀더 | StageRunner 요청을 게임 세계와 UI·오디오에 연결 |
| StageRunner | 별도 문서의 스테이지 진행 상태 | 섹션·지연 작업·곡선·게이트·카메라 |
| StageState / SandboxState | 위 시스템과 카메라·UI | 틱과 렌더의 호출 순서 |

소유권 근거: [PlayerController 필드](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Player/PlayerController.h:115>), [PlayerRig 필드](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Player/PlayerRig.h:101>), [EnemyWorld 필드](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/EnemyWorld.h:105>), [PropWorld 필드](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Props/PropWorld.h:182>), [StageState 필드](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Stage/StageState.h:106>).

**설계 해석:** EnemyWorld와 파생 Enemy의 경계는 “적 전체의 수명과 배열 관리”와 “종류별 행동 상태”를 나눈다. 이것이 모든 데이터 분기를 클래스로 만들어야 한다는 일반 규칙은 아니다. 소품, 투사체, 표시 슬롯은 값 구조체와 분기로 표현돼 있다.

## 2. 좌표, 시간, ID, 난수의 단위

| 값 | 실제 의미와 경계 |
|---|---|
| `Body::position` | 플레이어·보행 적의 발 중심. 상자는 bottom-center에서 위로 올라간다. 소품 `pos`는 중심/표시 원점이며 같은 의미로 일반화하지 않는다. |
| x / y | x가 오른쪽, y가 아래쪽. 점프는 음수 y 속도, 중력은 양수 y 가속도다. |
| `kStepSec` | 1/60초. 논리 틱 하나의 기본 시간 간격. |
| speed / velocity | 코드 계산상 px/s. 이동량은 velocity × dt. |
| gravity / accel | 코드 계산상 px/s². 속도 증가량은 acceleration × dt. |
| `stateTicks / clipTicks / ageTicks` | 정수 논리 틱. float 초 타이머와 섞어 쓰지 않는다. |
| `SecToTicks` | float 초를 double로 승격한 뒤 ×60을 lround한다. 단순 int 절삭이 아니다. |
| `ClipDef::frameUnits` | 애니메이션 정수 시간 단위. 틱당 100,000단위. 지정 프레임의 틱은 올림 나눗셈이다. |
| facing | 대부분 -1=왼쪽, +1=오른쪽. 입력 이동 x는 -1/0/+1. |
| z / depth / sort | 그림 제출의 정렬 값. 물리 z 좌표가 아니다. |
| owner | 피해의 주체/대상 식별 번호. 0은 미지정. |
| hit ID | 한 공격의 중복 피해 기록을 구분하는 번호. 0은 추적하지 않는 공격. |
| EnemyHandle / SpawnId | 적 ID와 스테이지 ID는 서로 다른 번호. StageWorldHost의 SpawnRoute가 연결한다. |

근거: [고정 스텝](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/World/Constants.h:3>), [PlayerHitboxes 좌표](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Player/PlayerHitboxes.cpp:20>), [BoxDef::At](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/EnemyDef.h:45>), [시간 변환](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/EnemyDef.cpp:50>), [애니메이션 틱 단위](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/EnemyDef.h:34>), [SpawnRoute](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Stage/StageWorldHost.h:50>).

난수도 하나가 아니다.

- 플레이어 `m_shotRng`와 `m_flameRng`는 각각 보관된다. 버스트 길이를 뽑기 전, 화염 분출 시작 시 별도로 reseed한다.
- `MakeShotSeed_`는 shotsFired와 발 좌표 float의 원시 비트로 seed를 만든다. 벽시계나 외부 “전역 랜덤”을 쓰는 코드가 아니다.
- EnemyWorld는 seed 1로 시작/재설정하는 `m_rng`를 가진다. `Roll(n)`은 n≤1이면 난수를 소비하지 않고 0, 그 외 Generate()%n이다.
- PropWorld와 내부 ItemWorld는 같은 초기 seed를 전달받아도 **서로 별개의 난수 객체**다.
- 소품 float 난수는 `Random<double>`를 float로 변환한 다음 범위를 계산한다. 이를 `Random<float>`와 수치적으로 동일하다고 설명하지 않는다.
- ItemWorld의 bounce 스폰은 수직 속도만 설정하지만 Generate 한 번을 일부러 소비한다. 이 소비도 실제 순서의 일부다.
- StageState의 StageRunner용 난수는 `0x57A6E0`로 초기화한다. 적/소품의 seed 1과 다른 스트림이다.

[공격 seed](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Player/PlayerAttack.cpp:295>), [플레이어 난수 필드](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Player/PlayerController.h:142>), [적 Roll](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/EnemyWorld.cpp:1086>), [소품 RollFloat](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Props/PropWorld.cpp:70>), [아이템 난수 소비](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Props/ItemWorld.cpp:71>), [스테이지 재시작 seed](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Stage/StageState.cpp:274>).

## 3. 실제 틱 순서: StageState

[실제 본문](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Stage/StageState.cpp:310>)의 순서를 먼저 읽고 아래 순서를 대조한다.

1. tickCount 증가. 로드되지 않았으면 반환한다. 엔드 화면 진행 중이면 UI/ScreenFlow만 갱신하고 반환한다. Runner가 이미 끝났어도 반환한다.
2. 디버그 warp/무기 요청을 처리한다.
3. CombatSystem::BeginTick으로 이번 틱의 상자/이벤트 배열을 비운다.
4. PropWorld::Tick으로 소품, 아이템, 장식, 소품 효과를 진행한다.
5. UpdateWalls_로 플레이어/적의 벽·발판 배열을 다시 만든다.
6. 플레이스홀더 → 적 → 소품의 Hurtbox를 등록한다.
7. PlayerController::Tick을 실행한다. 이동 후 플레이어 Hurtbox도 여기에서 등록된다.
8. 플레이어 몸 상자와 아이템을 겹쳐 Collect하고 보급·무기·점수를 적용한다.
9. 현재 플레이어 정보와 **현재 카메라**를 EnemyContext에 넣고 EnemyWorld::Tick을 호출한다.
10. 적이 요청한 cameraScroll을 Runner에 전달한다.
11. bosslab의 bAutoDamage가 켜졌으면 보스 부위/화면 안 tower에 합성 피해를 직접 전달한다. 일반 stage0 기본 설정에서는 실행하지 않는 분기다.
12. ProjectileSystem::Step을 호출한다. 활성 지면과 **적용 발판 배열 m_enemyPlatforms**를 넘긴다.
13. CombatSystem::Resolve가 HitEvent 배열을 만든다.
14. 소품 → 적 → 플레이스홀더 → 플레이어 → 투사체 순으로 같은 이벤트 배열을 전달한다. 플레이스홀더 처치 점수도 적용한다.
15. 플레이스홀더 죽음 타이머 진행.
16. 적 이벤트를 소비한다: 점수/드롭/구출 수/효과음. 소품 이벤트를 소비한다: 폭발음/지형 패치.
17. 투사체 FX를 가져와 소리, UI, EffectList, 폭탄/낙하산 애니메이션, PlayerRig를 갱신한다.
18. 이전 카메라 위치를 저장한 뒤 **마지막에 StageRunner::Tick**을 호출한다.
19. 섹션이 바뀌면 표시 타이머를 reset하고 timer.Tick한다.
20. EndStage 요청 또는 GameOver가 있으면 화면 종료 흐름을 시작한다.

**코드에서 바로 나오는 결과:** 적 Hurtbox는 적 Tick **전** 위치로 등록되고, 플레이어 Hurtbox는 플레이어 이동 **후** 위치로 등록된다. 적이 이번 틱에 움직였다고 이미 등록된 Hurtbox가 자동으로 함께 바뀌지 않는다. 소품 Tick은 플레이어 Tick보다 앞이므로 아이템은 움직인 위치에서 수집된다. Runner가 틱 말미에 생성한 새 적은 다음 틱의 등록·적 행동 단계에 참가한다. 이 차이를 모두 “같은 틱에 최신 위치로 충돌한다”라고 단순화하면 틀린 설명이 된다.

[Hurtbox 등록 순서](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Stage/StageState.cpp:344>), [플레이어 Tick과 Collect](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Stage/StageState.cpp:368>), [적 Tick](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Stage/StageState.cpp:381>), [Resolve 이후 순서](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Stage/StageState.cpp:459>), [Runner 호출](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Stage/StageState.cpp:508>).

StageRunner::Tick 자체의 확인된 순서는 CancelAbortables → 플레이어 logical feet의 x로 섹션 발동 → pending → curves → gates → 카메라 target/update → 내부 tick 증가다. 섹션 트리거는 `feet.x > triggerX`이며 ≥가 아니다. [StageRunner::Tick](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Stage/StageRunner.cpp:210>).

## 4. 플레이어: 입력에서 몸과 공격까지

### 4.1 필드와 함수의 책임

PlayerController는 “이동 상태 하나”만 가진 객체가 아니다. `m_state`는 Idle/Run/Jump/Crouch/CrouchMove/Dead/Respawn/Victory이고, `m_pose`는 Stand/Crouch/Air, `m_aim`은 5방향, `m_overlay`는 None/Shot/Slash/Throw다. 총·칼·투척 진행은 별도 타이머와 ID로 저장한다. [종류 선언](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Player/PlayerTypes.h:4>), [필드](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Player/PlayerController.h:128>).

| 파일/함수 | 주요 책임 |
|---|---|
| PlayerInputBuffer::Update / Read / Consume | 눌림 후 경과 틱, held 상태, 행동별 버퍼 소비 |
| PlayerController::Reset | 몸·입력·무기·점수·목숨·공격·낙하산을 시작 상태로 설정 |
| PlayerController::Tick | pending ID 해제 → 입력 갱신 → 이전 발 저장 → 무기 cooldown 진행 → 생존/죽음 → Hurtbox → 무적 감소 |
| TickAlive_ | Victory 입력 비우기, 낙하산 특수 처리, 이동→조준→상태도출→공격 |
| TickMovement_ | 앉기/머리 여유/가로 속도/점프/낙하/Physics Step/grounded |
| UpdateAim_ / DeriveState_ | 무기별 조준 변화, 물리 결과를 이동 상태로 변환 |
| TickActions_ | 칼→폭탄→총; 표시 오버레이 우선순위 결정 |
| TickKnife_ / TickBomb_ / TickShot_ | 각 공격의 조건, 입력 소비, 발사 시점, 진행 종료 |
| Kill / TickDead_ / Respawn_ | 죽음의 guard, 목숨 감소, 죽은 몸의 물리, 부활 위치·무기·무적 |
| MakeHurtbox / MakeRigState | 물리/전투/그림 간 필요한 값 만들기 |
| HashState | 검사에 필요한 일부 값 혼합. 전체 상태 직렬화는 아님 |
| PlayerHitboxes 자유 함수 | 칼 트리거/실제 스윙, 총구/조준 벡터/폭탄 위치/피격 상자 |
| WeaponState | 무기·탄·폭탄·사격 간격·탄 소진 |
| Weapon / HmgWeapon / FlameWeapon | BulletSpawn 제작 차이 |
| PlayerRig | 클립 탐색·fallback·상/하체 시간 진행·그림 제출 |
| PlayerTypes의 이름 함수 | 상태/조준/오버레이/디버그 무적의 문자열 표시 |

근거: [PlayerController::Tick](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Player/PlayerController.cpp:161>), [TickAlive_](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Player/PlayerController.cpp:258>), [공격 조율](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Player/PlayerAttack.cpp:10>), [WeaponState](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Player/PlayerWeapons.cpp:29>).

### 4.2 입력 버퍼는 “계속 눌렀다”와 다르다

매 틱 InputHistory::WasPressed이면 해당 행동 경과를 0으로 만든다. 새로 누르지 않았으면 경과를 증가시킨다. `IsBuffered(action, window)`는 경과 < window일 때만 true다. Consume하면 kNever=1,000,000으로 바뀌므로 같은 눌림을 재사용하지 않는다.

Read는 좌우 held를 빼서 moveX를 만든다. 양쪽을 같이 누르면 0이다. Up/Down/Shot held와 Jump/Shot/Bomb buffered를 동시에 제공한다. 사용자가 키를 계속 누른 상태와 최근 눌림을 기억한 상태가 명령 안에 함께 존재한다. [PlayerInput.cpp](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Player/PlayerInput.cpp:8>).

**예시:** 지면에 닿기 직전 Jump를 눌렀지만 그 틱에는 아직 공중이면, jumpBufferTicks 동안 입력이 살아 있다. TickMovement_는 내려가는 몸을 복사해 Physics::Step으로 이번 틱 착지할지를 미리 검사한다. 착지할 수 있으면 실제 몸에 점프 속도를 설정하고 버퍼를 소비한다. 이것은 기록된 입력을 재생하는 시스템과 다른, 현재 조작의 짧은 기억이다. [착지 probe](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Player/PlayerMovement.cpp:145>).

### 4.3 이동 상태는 물리 결과에서 도출된다

TickMovement_는 처음의 grounded를 기준으로 앉기와 속도를 정한다. 아래를 누르고 지상이며 Respawn 상태가 아니면 앉기를 시도한다. 이미 앉았는데 일어날 공간의 머리 영역이 막혀 있으면 앉은 상태를 유지한다. 지면 마스크, 양방향 발판, 벽을 검사한다. [머리 영역 검사](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Player/PlayerMovement.cpp:9>), [앉기](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Player/PlayerMovement.cpp:127>).

가로 방향이 0이 아니면 facing을 바꾼다. 앉은 상태에서 Shot held, 총 오버레이 진행, 투척 진행, Bomb buffered 중 하나가 있으면 가로 속도가 0이다. 서 있는 지상은 runSpeed, 공중은 airSpeed, 비사격 앉기는 crouchSpeed다. [가로 속도](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Player/PlayerMovement.cpp:136>).

점프 허용은 “지상” 또는 “아직 소비하지 않은 coyote 허용 + airTicks 1..coyoteTicks” 또는 “이번 물리 스텝에 착지할 예측”이다. OneWay 발판 위에서 아래+점프면 RequestDropThrough하고 Jump를 소비한다. 그 밖에는 머리 영역에 여유가 있을 때만 y속도를 -jumpSpeed로 바꾸고 Jump를 소비한다. 머리가 막혔으면 아직 소비하지 않는다. [점프 분기](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Player/PlayerMovement.cpp:154>).

실제 Physics::Step 뒤 grounded이면 airTicks=0, coyoteAvailable=true, Jumping=false, Stand/Crouch를 정한다. 아니면 airTicks를 늘리고 Air가 된다. 벽에 막히지 않은 moveX가 있을 때 bMoving이다. DeriveState_는 공중 Jump, 앉은 지상 Crouch/CrouchMove, 서 있는 지상 Idle/Run을 고른다. Dead/Victory는 유지하며 공중 Respawn도 유지한다. [결과 갱신](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Player/PlayerMovement.cpp:180>), [DeriveState_](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Player/PlayerMovement.cpp:248>).

### 4.4 낙하산은 일반 점프와 다른 시작 단계다

BeginChuteDescent는 원래 발 위치를 landing feet로 저장하고 몸을 chuteStartHeightPx만큼 위로 옮긴다. Descend 중 logical feet는 landing feet이며, 실제 표시 발은 위에 있다. StageRunner가 PlayerFeet로 읽는 것은 이 logical feet다. Descend는 Physics::Step을 쓰지 않고 chuteFallSpeedPxSec × 1/60씩 직접 내려온다.

사격 버퍼가 없으면 낙하산 진행만 하고 TickAlive_를 반환한다. 사격 버퍼가 있으면 입력을 소비하고 Release로 전환하며 그 틱 command를 비운다. 착지하면 자동 Release가 되고 지정 주기/기간 동안 낙하산 그림이 깜빡인다. Kill은 Descend 중 죽음을 거부한다. [낙하산 구현](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Player/PlayerMovement.cpp:47>), [실제/논리 발](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Player/PlayerMovement.cpp:89>), [사격으로 해제](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Player/PlayerController.cpp:268>).

### 4.5 조준

HMG 이외 무기는 targetDeg를 즉시 적용한다. 서기/공중의 Up은 +90도, 공중 Down은 -90도, 앉기는 전방이다. HMG는 사격 중 350/60도씩 목표를 향해 돌고, 비사격 복귀는 1000/60도씩 움직인다. 절댓값 22.5도 미만 전방, 67.5도 미만 대각, 그 이상 상/하로 표시 방향을 고른다.

HMG의 실제 탄은 이 연속 각도의 cos/sin으로 날아간다. 따라서 5방향 표시 eAim과 실제 HMG 탄 각도는 같은 정보가 아니다. 나머지 무기의 GetAimDirection은 대각 1/√2를 쓰는 정규화된 5방향이다. [UpdateAim_](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Player/PlayerMovement.cpp:204>), [HMG MakeShot](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Player/PlayerWeapons.cpp:176>), [GetAimDirection](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Player/PlayerHitboxes.cpp:63>).

### 4.6 가까운 인간을 쏘면 칼이 먼저 선택되는 흐름

TickKnife_는 앞쪽 트리거 상자 안에 Player가 Melee로 때릴 수 있는 Hurtbox가 있는지 묻는다. 인간이 아닌 기본 적/소품은 보통 Bullet/Blast만 accept하므로 칼 대상이 되지 않는다. 다만 파생 RegisterHurtboxes가 별도 규칙을 만들 수 있으므로 모든 적을 “human 필드만으로 칼 가능”이라고 일반화하지 않는다.

범위 내 대상 + Shot buffered 또는 held + 새 칼 가능/chain window이면 기존 칼을 끝내고 새 hit ID를 만든다. 1·2번 칼 표시를 번갈아 바꾸고 Shot 입력을 소비하며 투척을 취소한다. 지정 activeFirstTick..activeLastTick(양 끝 포함)에만 실제 스윙 상자를 등록한다. 칼 Hitbox는 pierce=true이므로 여러 대상에 닿을 수 있지만 같은 ID/같은 owner에는 한 번만 피해가 발생한다.

칼 시작 이후 TickBomb_와 TickShot_가 뒤에 실행된다. 총은 진행 중 칼/투척 또는 그 틱 표시 플래그가 있으면 burst를 취소하고 반환한다. 오버레이는 Slash > Throw > Shot > None다. [TickKnife_](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Player/PlayerAttack.cpp:43>), [실제 판정 활성 범위](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Player/PlayerHitboxes.cpp:98>), [오버레이](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Player/PlayerAttack.cpp:25>).

### 4.7 폭탄은 입력 틱과 실제 생성 틱이 다르다

Bomb buffered, 폭탄 수 >0, 칼 진행 없음, 현재 플레이어 폭탄 수 <maxInFlight이고 투척이 시작되지 않았으면 m_throwTick=0이다. 그때 Bomb 입력을 소비하지만 아직 탄 수는 줄지 않는다. releaseTick에 BombSpawn을 생성하고 폭탄을 하나 줄인다.

생성 위치는 pose/facing에 따른 offset이며 앉으면 y를 14px 더 내린다. x속도는 facing×speedX + 현재 이동 x속도×inheritMoveX, y속도와 gravity·fuse·blast는 tuning에서 가져온다. 플레이어 폭탄은 bounce=true다. throwTicks에 도달하면 투척 진행이 끝난다. [TickBomb_](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Player/PlayerAttack.cpp:91>), [release 위치](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Player/PlayerHitboxes.cpp:79>).

### 4.8 일반 사격과 버스트

WeaponState::ShouldFire는 동일 owner/무기 tag의 살아 있는 Bullet 수 제한을 먼저 검사하고 fireIntervalTicks를 검사한다. buffered 입력이면 발사하며, held는 holdRepeatTicks>0이고 충분히 기다렸을 때만 재발사한다. Pistol은 유한 탄이 없고 나머지 무기는 OnFired에서 ammo를 줄인다. 소진 즉시 Pistol로 전환된다. [ShouldFire/OnFired](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Player/PlayerWeapons.cpp:61>).

버스트는 tuning의 burstShotsMax>1이며 Pistol이 아닐 때 적용한다. 눌림 한 번에 MakeShotSeed로 reseed → 최소/최대 사이 정수 발 수 추첨 → 첫 탄 생성 → 남은 발 수/간격/공중 여부 저장이다. 진행 중에는 새로운 시작 조건을 다시 검사하지 않고 timer가 끝나면 다음 탄을 쏜다. 지상/공중이 바뀌거나 현재 무기가 Pistol이면 취소한다. 칼·투척에도 취소한다.

FireBullet_은 WeaponBehavior가 만든 BulletSpawn에 depth를 붙여 SpawnBullet하고, shotsFired 증가 후 OnFired한다. 마지막 탄이 무기를 Pistol로 바꿨다면 burstLeft=0이 된다. 탄 소진 전후의 이 쓰기 순서를 바꿔 생각하면 안 된다. [TickShot_](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Player/PlayerAttack.cpp:215>), [FireBullet_](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Player/PlayerAttack.cpp:316>).

Weapon은 정적 const 인스턴스를 반환한다. Pistol/Shotgun/Rocket은 기본 Weapon의 MakeShot을 공유하고, HmgWeapon/FlameWeapon만 override한다. 발사할 때마다 새 Weapon을 할당하는 구조가 아니다. Shotgun도 여기서는 여러 펠릿을 생성하는 코드가 아니라 하나의 BulletSpawn을 만든다. [GetWeaponBehavior](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Player/PlayerWeapons.cpp:218>), [기본 MakeShot](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Player/PlayerWeapons.cpp:147>).

### 4.9 화염: 분출 기간과 입자 수명, 피해 기록을 구분하기

StartFlame_은 공격 횟수·탄을 한 번 소비하고 별도 flame RNG를 reseed하며 공용 flameHitId를 만든다. 분출 동안 particleIntervalTicks마다 입자를 생성한다. spawnEnd는 particleCount×interval이다. 입자마다 확산 각도, y jitter(설정 시), 속도, 수명 난수를 순서대로 뽑는다. range는 speed×life/60, hitSize는 이동 거리에 따라 sizeEnd까지 커진다.

칼/투척 또는 지상↔공중 전환이면 새 입자 생성만 멈춘다. 이미 생긴 입자는 ProjectileSystem에서 자기 수명 동안 남는다. Shot held를 놓았다는 이유만으로 분출 진행을 즉시 끝내는 분기는 없다. EndFlame은 spawnEnd+최대 입자 수명을 **초과한** 때다.

모든 입자가 같은 hitId를 공유해 같은 대상에 대한 중복 피해를 막는다. hitReleaseTicks 간격에는 HitResolver 기록을 Release해 같은 ID가 다시 피해를 줄 수 있게 한다. EndFlame의 ID 해제는 pendingRelease에 넣어 다음 Player Tick 시작에 처리한다. [화염 진행](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Player/PlayerAttack.cpp:145>), [입자 MakeShot](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Player/PlayerWeapons.cpp:190>), [공유 ID 보관](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Combat/Projectiles.cpp:105>).

### 4.10 피격, 죽음, 부활, 승리

OnHitEvents는 player owner=1을 대상으로 하고 attackerTeam이 Player가 아닌 이벤트를 찾으면 Kill을 한 번 호출한다. 플레이어는 이벤트 damage만큼 HP를 줄이는 모델이 아니다. Kill이 허용되면 곧바로 Dead, deaths++, lives--다.

Kill은 이미 Dead/Victory, invincibleTicks>0, 낙하산 Descend, godMode≠Off일 때 거부한다. NoDeath도 죽음을 막지만 Hurtbox를 무적으로 표시하는 조건은 godMode==Invincible이다. “두 god mode가 같은 Hurtbox”라고 설명하면 틀리다. [MakeHurtbox](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Player/PlayerController.cpp:150>), [Kill](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Player/PlayerController.cpp:113>).

죽음은 가로 속도를 0으로 만들고 공격·낙하산 진행을 정리한다. 칼/화염 ID는 pending release로 넘긴다. 목숨이 0이면 GameOver를 세운다. 죽은 틱에도 Physics::Step은 계속한다. GameOver가 아니고 deathTicks에 도달하면 respawnFeet로 이동하고 무기를 reset, Respawn/Air, respawnInvincible, coyote 불허로 시작한다. Score는 respawn에서 reset하지 않는다. [TickDead_/Respawn_](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Player/PlayerController.cpp:210>).

SetVictory는 Dead이면 하지 않으며 Victory에서 공격을 정리한다. IsInvincible은 Victory도 true로 보지만 SetVictory를 Stage 종료 흐름이 자동 호출한다고 이 문서에서는 주장하지 않는다. 실제 호출은 별도 검색으로 확인해야 한다. [SetVictory](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Player/PlayerController.cpp:76>).

### 4.11 PlayerRig는 물리·전투의 주인이 아니다

Rig는 weapon×좌우×slot 표를 바인딩한다. 클립 이름의 {W}/{S}를 실제 이름으로 펼치며 무기별 후보 토큰 순서를 쓴다. HMG/Rocket은 MG→SG→DG, Flame은 SG→MG→DG 등이다. 누락 슬롯은 최대 4단계 fallback을 찾고 필수 누락 수를 보고한다.

Dead/Victory/Crouch는 단일 상체 클립, 서기/공중은 상·하체로 나누어 표시한다. jump timeline은 airTicks로 정규화 시간을 계산하고, 걸어 떨어진 경우 클립 길이의 40% 지점에서 시작한다. 새 overlaySerial/stateSerial이면 같은 이름 클립도 restart한다. 공격 표시 속도는 클립 길이/overlayDurationTicks로 맞춘다. 지상 Move 상체는 LowerMove와 위상을 동기화한다.

Submit은 하체를 먼저, 상체를 뒤에 제출한다. 발 좌표에서 baseline lift를 빼 표시 원점을 만든다. hurtbox/총구/실제 칼 상자는 Rig 프레임이 아니라 앞서 설명한 논리 함수가 만든다. [ResolveClipName/Bind](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Player/PlayerRig.cpp:140>), [Update](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Player/PlayerRig.cpp:263>), [Submit](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Player/PlayerRig.cpp:410>).

## 5. CombatSystem: 피해 이벤트가 만들어지는 정확한 규칙

### 5.1 BeginTick과 공격 ID의 수명

BeginTick은 hitboxes, hurtboxes, events만 clear한다. nextHitId와 HitResolver의 이전 피해 기록은 유지한다. NewHitId는 ++nextHitId다. ReleaseHitId는 resolver의 해당 기록을 지운다. “매 틱 combat reset”은 등록 배열만 reset하는 것이며, 지속 공격의 중복 피해 기록까지 reset하는 것은 아니다. 재시작 경로에서는 GetResolver().Clear를 별도로 호출한다. [CombatSystem.cpp](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Combat/CombatSystem.cpp:5>).

### 5.2 팀, 공격 종류, 자기 자신

| 공격 팀 | 가능한 대상 |
|---|---|
| Player | Enemy, Neutral |
| Enemy | Player |
| Neutral | Player, Enemy |

Hurtbox의 accepts mask가 해당 HitKind를 포함해야 하고 bInvulnerable이면 제외한다. owner가 0이 아닌 공격과 동일 owner의 Hurtbox도 제외한다. AcceptsFor(false)는 Bullet/Blast만, true는 All이다. 이 때문에 기본 비인간 적과 소품은 보통 칼 대상에서 제외된다. [팀 규칙](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/World/Combat.cpp:161>), [AcceptsFor](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/World/Combat.cpp:123>).

### 5.3 등록 순서와 관통

Resolve는 Hitbox 배열을 바깥 반복, Hurtbox 배열을 안쪽 반복으로 돈다. 빈 공격 상자는 건너뛴다. tracked ID가 이미 consumed라면 전체 공격을 건너뛴다. 팀·무적·종류·자기 owner·AABB overlap을 통과한 대상을 이벤트로 기록한다.

owner가 있으면 owner를 targetKey로 삼고, 없으면 hurtbox 인덱스에 별도 상위 비트를 붙인다. 같은 ID와 targetKey의 중복은 제외한다. 비관통 공격은 처음 맞는 Hurtbox에서 consumed=true를 쓰고 inner loop를 break한다. 관통은 대상별 한 번씩 이어간다.

**예시:** 같은 적 owner를 가진 몸/추가 Hurtbox가 겹쳐도 같은 tracked 공격은 두 번 피해를 주지 않는다. 그러나 방패 owner나 보스 부위 owner는 본체와 구분된 ID다. 이때는 서로 다른 targetKey다. 비관통 총알이 어디에 먼저 맞는지는 registration order에도 달려 있다. 가장 가까운 충돌 지점/시간을 정렬하는 코드가 아니다. [HitResolver::Resolve](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/World/Combat.cpp:17>).

### 5.4 HitFlash는 피해 자체와 다르다

StartHitFlash는 7틱을 설정한다. 7은 시작 표시 예약이며 실제 On은 ticksLeft가 3보다 크고 7보다 작을 때(6/5/4)다. 이미 On 또는 시작값 7이면 플래시를 다시 시작하지 않는다. TickHitFlash는 양수에서 1씩 감소한다. 이 함수가 재피격을 무시해도 Health::ApplyDamage가 무시된다는 의미는 아니다. [HitFlash](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Combat/HitFlash.cpp:3>).

## 6. ProjectileSystem: 네 종류의 값 데이터

### 6.1 Bullet, Shot, Bomb, Blast의 차이

| 종류 | 생성과 진행 |
|---|---|
| Bullet | 방향×speed, range 누적, 필요 시 자체 scalar acceleration, 공유/독립 hit ID, 관통·성장·rocket trail |
| Shot | 적 ProjectileDef에서 생성. 벡터 accel, gravity, start delay, lifetime, 착지 후 굴림, visual index |
| Bomb | hit ID 없이 낙하/반사. 지면 접촉·fuse·대상 접촉 때 Blast를 생성 |
| Blast | 고정 AABB, 남은 ticks, 새 hit ID, Blast/pierce 판정 |

[구조체 선언](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Combat/Projectiles.h:8>), [생성 함수](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Combat/Projectiles.cpp:88>).

Step의 순서는 Bullet → Shot → Bomb → Blast다. 각 종류의 죽은 항목을 지우며, 뒤에서 생성된 Blast는 그 Step의 마지막 Blast 루프에서 바로 등록된다. 반대로 Resolve **후** OnHitEvents에서 만든 Blast는 그 틱에 Resolve를 다시 하지 않으므로 다음 Step에서 판정에 참가한다. [Step](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Combat/Projectiles.cpp:206>), [OnHitEvents](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Combat/Projectiles.cpp:495>).

### 6.2 이동한 탄의 상자는 선분 전체를 감싼다

SweptBox_는 hitFrom과 현재 위치의 min/max에 반 크기를 더한 **축 정렬 상자**다. 선분 자체와 대상의 정밀 교차 시험이 아니다. SpawnBullet의 첫 hitFrom은 sweepFrom(보통 가슴 위치)여서 첫 틱에는 가슴→총구→이동 끝 범위를 감싼다. 다음 틱부터는 직전 탄 위치가 hitFrom이다. [SweptBox_](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Combat/Projectiles.cpp:67>), [SpawnBullet](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Combat/Projectiles.cpp:88>).

지면 검사는 이동 거리를 2px 이하 간격으로 나누어 중심점을 검사한다. Bullet은 renderPrev→position을 검사하며 크기 전체를 지면에 검사하지 않는다. range를 **초과하면** 먼저 소멸/폭발한다. 거리와 같을 때는 살아 있다. Bullet은 이 코드에서 넘겨받은 발판을 검사하지 않는다. [SegmentHitsGround_](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Combat/Projectiles.cpp:12>), [Bullet 이동](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Combat/Projectiles.cpp:212>).

### 6.3 적 Shot의 지연, 가속, 굴림

Shot은 ageTicks를 늘린 후 ageTicks>startDelayTicks일 때만 가속/중력/위치 이동한다. maxSpeed가 있으면 accel 적용 뒤 속력 벡터를 제한하고, 그 뒤 gravity를 적용한다. 지연 중에도 Hitbox 등록 루프 자체는 실행된다.

rollSpeed가 있고 착지했다면 이후 x속도는 rollDirX×rollSpeed다. 착지 후 rollTicks를 **초과**했는지 또는 기본 lifetimeTicks를 초과했는지로 만료를 정한다. 지면/발판 검사에는 groundProbeY offset을 더한다. 발판은 아래로 이동하면서 윗면을 통과하는 가장 이른 t를 골라 검사한다. 지면 접촉이 먼저 있으면 발판 접촉을 따로 고르지 않는다. 굴림 탄은 지면에서 최대 64px 위로 밀어 free 위치를 찾거나 발판 윗면에 맞춘다. [Shot 루프](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Combat/Projectiles.cpp:290>), [발판 선분](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Combat/Projectiles.cpp:39>).

### 6.4 플레이어 폭탄의 반사와 폭발

bounce 폭탄은 x이동과 y이동을 따로 검사한다. 벽 반사는 vx=-vx×0.4, 아래쪽 지면 반사는 vy=-vy×0.4와 vx×0.82다. 반사 vy가 -40보다 크면 0으로 놓는다. fuse에 도달하거나 Blast로 때릴 수 있는 대상과 현재 contact box가 겹치면 폭발한다. bounce 경로는 플랫폼 상자를 검사하지 않는다.

비bounce 폭탄은 한 번에 next 위치를 만들고 지면에 닿으면 즉시 폭발, 그렇지 않으면 현재 target contact를 검사한다. [Bomb 루프](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Combat/Projectiles.cpp:398>).

Blast는 생성할 때 새 ID를 가진다. 수명이 0 이하인 기존 blast의 기록을 해제·삭제한 뒤, 살아 있는 blast를 pierce=true로 등록하고 ticksLeft를 줄인다. 같은 blast가 여러 틱 존재해도 기록을 유지해 대상별 한 번 피해를 준다. [Blast 루프](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Combat/Projectiles.cpp:471>).

### 6.5 표시/소리는 별도 해석이다

BulletVisual은 HMG 각도를 6도 단위 60방향 sprite 이름에 매핑하고, Shotgun 방향별 clip, Rocket 회전, Flame head/loop/fade 및 크기 변화, Pistol 기본 H/V sprite를 고른다. Flame 그림은 수명에 따른 scale과 가로 24px 전방 shift를 쓰며 논리 Hitbox를 이 그림에서 다시 계산하지 않는다. [SubmitPlayerBullet](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Combat/BulletVisual.cpp:75>).

PlayShotSfx/PlayWeaponPickupSfx는 무기 종류를 소리 이름·볼륨/간격에 매핑한다. PlayerStats 이전값과 현재값의 차이를 Stage/Sandbox가 읽어 호출한다. 이 통계 카운터가 곧 발사된 소리의 고유 이벤트 객체인 것은 아니다. [소리 매핑](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Combat/BulletVisual.cpp:166>), [Stage RequestSounds_](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Stage/StageState.cpp:762>).


## 7. EnemyWorld: 적 객체를 누가 만들고 끝내는가

### 7.1 생성 시 한 번 선택하는 다형성

`MakeEnemy(eEnemyBehavior)`의 switch는 14가지 파생 객체를 `MakeScoped<T>`로 생성한다. EnemyWorld는 `Vector<Scoped<Enemy>>`를 소유한다. Enemy는 virtual destructor와 Init/Tick 및 선택적 OnHit/OnDeath/TickDying/RegisterHurtboxes/SubmitParts/DeathDone/BodyFrame을 제공한다. 매 틱 종류별 함수 포인터 테이블을 찾는 구현은 현재 이 범위에 없다. [Enemy 인터페이스](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/Enemy.h:47>), [14종 팩토리](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/Enemy.cpp:56>).

EnemyState는 ID, 정의 인덱스, life, Body/prevFeet, 방향·HP·활성·초기화, 공통 state/stateTicks/clipTicks, 근접/추가 상자 ID, 등반·사망·방패·drop·arena 등 공통 데이터를 담는다. 각 파생 클래스가 별도 진행 필드를 소유한다. 예를 들어 Bzk의 남은 탄과 자세, Heli의 비행 좌표, Bus의 단계별 시작 틱, BigShiee의 포탑/주포 배열은 각 파생 객체 안에 있다. [EnemyState](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/EnemyTypes.h:104>).

**설계 해석:** 이는 행동과 행동에 필요한 진행 값을 같은 클래스에서 읽도록 하는 배치다. 하지만 EnemyState의 int state가 종류별로 같은 의미를 가진다는 뜻은 아니다. Soldier state=3과 Bus state=3을 하나의 공용 FSM 코드로 해석하면 안 된다.

### 7.2 ID, 핸들, 지연 생성

SpawnByIndex는 유효 정의를 확인하고 m_nextId++를 배정한다. 최초 ID는 2000이다. Create_는 종류 객체를 만들고, delaySec→delayTicks, Body 크기/중력/feet/facing/HP/active/tag/drop/arena 등을 설정해 배열 끝에 넣는다. HP override는 **양수일 때만** 쓰며 그 외는 def.hp다. snap world가 있으면 16px SnapToGround한다.

Init는 Create_ 시점에 호출되지 않는다. EnemyWorld::Tick에서 첫 방문 시 bInit=true를 쓰고 Init를 호출한다. delayTicks>0이면 Pending이며 카운트가 끝날 때 Alive로 바뀐 뒤 초기화한다. [Spawn/Create_](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/EnemyWorld.cpp:117>).

SpawnDeferred는 즉시 ID를 예약하고 생성 요청을 deferred에 넣는다. 현재 적 반복이 끝난 뒤 요청 순서대로 Create_하며 snap world를 넘기지 않는다. 따라서 부모 적이 Tick 중 새 적을 생성해도 현재 적 반복의 크기를 바꾸지 않는다. 새 객체는 같은 틱 말미에 배열에 존재하지만 행동 Init/Tick은 다음 EnemyWorld::Tick부터다. [지연 생성](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/EnemyWorld.cpp:133>), [flush](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/EnemyWorld.cpp:473>).

변신은 NoteSuccessor(old,new)로 연결한다. ResolveHandle은 최대 64회 연결을 따라간다. IsDefeated/IsDestroyFinished는 후속 핸들을 해석해 확인하지만 Get/FindEnemy는 요청한 ID를 직접 찾는다. 오래된 핸들로 Get하면 변신 이후 객체를 자동 반환하는 API가 아니다. [ResolveHandle](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/EnemyWorld.cpp:823>), [Get](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/EnemyWorld.cpp:803>).

### 7.3 EnemyWorld::Tick 내부 순서

1. EnemyContext를 lastCtx에 저장한다.
2. 전체/부위 HitFlash의 남은 틱을 감소한다.
3. ProjectileSystem의 이전 ShotFx를 소비해 정의의 폭발 애니메이션/소리 요청을 만든다.
4. 살아 있는 Shot의 trail 간격에 맞춰 꼬리 연출을 만든다.
5. 기존 pendingFx를 flush하고 EffectList를 update한다.
6. 배열 순서대로 적을 방문한다.
7. Gone이면 건너뛴다. Pending의 delay를 감소시키고 끝나면 Alive.
8. 아직 Init하지 않았으면 bInit=true 후 Init.
9. prevFeet를 현재 발로 저장.
10. Alive면 activation을 latch하고 화면 뒤/아래 despawn 조건을 확인한다.
11. stateTicks/clipTicks 증가 → 파생 Tick → 에디터 MotionPlay 덮어쓰기/발사 → WeaponEffects → ExtraBoxes.
12. Tick 이후 Alive+HP≤0이면 BeginDying.
13. Dying이면 stateTicks/clipTicks 증가 → 추가 공격 ID 해제 → 파생 TickDying.
14. deferred를 생성하고 Gone을 순서 유지 압축으로 제거한다.

[EnemyWorld::Tick](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/EnemyWorld.cpp:346>).

activation은 bActive가 false이고 always이거나 x≤camera.max.x+margin이면 true로 만든다. 오른쪽/왼쪽/수직 전체 view.contains 판정이 아니다. 뒤쪽/아래 despawn은 noDespawn=false일 때 적용한다. `activation.bLatch`는 파서에서 읽지만 이 Tick은 이를 검사하지 않는다. “latch=false면 다시 비활성화”는 현재 코드에 없는 설명이다. [activation/despawn](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/EnemyWorld.cpp:426>), [bLatch 읽기](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/EnemyDefLoader.cpp:1091>).

### 7.4 일반 피해와 파생 OnHit

OnHitEvents는 IsEnemyOwner인 대상의 하위 24bit를 적 ID로 찾고 Alive만 처리한다. 파생 OnHit가 true를 반환하면 기본 Health 피해를 하지 않는다. 필요 시 해당 부위 flash만 시작하고 다음 이벤트로 넘어간다. false면 ApplyDamage → lastHitKind → 죽으면 BeginDying / 안 죽으면 전체 HitFlash다.

BeginDying은 life=Dying, vx=0, Died 이벤트를 먼저 push하고 파생 OnDeath를 호출한다. BigShiee/POW/Shielder처럼 OnHit를 직접 처리하는 종류는 이 기본 경로와 다르다. [OnHitEvents](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/EnemyWorld.cpp:219>), [BeginDying_](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/EnemyWorld.cpp:270>).

MakePartOwner는 적 ID 하위 24bit와 (partIndex+1)<<24를 합친다. part 없는 본체는 GetPartIndex=-1이다. 방패는 bit 0x40000000을 사용한다. part owner를 enemy ID와 같은 번호로 보면 중복 피해·부위 HP 설명이 틀어진다. [부위/방패 ID](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/EnemyTypes.cpp:3>).

### 7.5 Gone, Defeated, DestroyFinished는 서로 다르다

Defeated는 Pending이면 false, Alive가 아니면 true다. DestroyFinished는 Gone/객체 없음이면 true, Dying이면 파생 DeathDone을 묻는다. 기본 DeathDone은 false다. 기본 TickDying은 곧 Gone으로 만든다. 보스/버스는 Dying 객체를 계속 두면서 전용 연출 완료 시간을 반환한다.

RemoveGone_은 앞에서부터 살아남는 Scoped를 앞으로 move해 원래 생존 순서를 유지한다. bTransformed가 아니면 Removed 이벤트를 보낸다. 변신한 객체는 Transformed 이벤트가 별도로 있어 Removed를 생략한다. [수명 질의](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/EnemyWorld.cpp:878>), [제거](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/EnemyWorld.cpp:516>), [기본 죽음](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/Enemy.cpp:27>).

### 7.6 공통 행동 함수는 같은 의미를 명시한다

| 함수 | 책임 |
|---|---|
| EnterState / SetClip | stateTicks를 0으로, clip 변경/restart 때 clipTicks를 0으로 |
| SideToPlayer / FacePlayer | x차가 4px deadzone 이내면 기존 facing 유지 |
| ClimbStep / ClimbLeap | arena.min→arena.max→선택적 climbEnd 이동, 완료 후 jumpVy/speed로 도약 |
| OnRoof / AtRoofEdge / CanLeaveRoof / HoldOnRoof | roofSpan과 climbEnd 높이로 지붕 여부·끝·이탈 영역·clamp |
| PrepareBody / StepPhysics | 정의의 몸 크기·발 폭을 적용하고 grounded/oneway 결과 기록 |
| WallAhead / FloorAhead | 지면·벽·양방향 발판 probe / 별도 Body의 아래 지면 snap probe |
| FireWeapon | EnemyDef의 무기·ProjectileDef를 ShotSpawn으로 변환 |
| StartMelee / TickMelee / CancelMelee | 근접 공격 ID/수명, 시작 원점 또는 따라가는 상자 |
| RequestFx / SubmitNamedPart | 방향별 이름, delay/debris, 파츠 그림과 flash/depth |
| BeginSoldierDeath / TickSoldierDeath | 인간 계열의 폭발·칼·서서넘어짐·낙하 사망 |
| BeginVehicleDeath / TickVehicleDeath | 차량의 파괴 클립·FX와 removeAtSec 타이머 |

[공통 헤더](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/EnemyCommon.h:6>), [실제 무기 발사](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/EnemyCommon.cpp:351>), [차량 공통](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/VehicleCommon.cpp:5>).

FireWeapon은 player.bAlive와 projectiles/combat가 있어야 발사한다. aim=Given이면 전달 방향, Facing이면 facing 수평을 선택하고 tilt를 더한 뒤 정규화한다. projectile speed, ramp, v0 우선순위로 속도를 만든다. arc는 시작 vy와 중력으로 목표 높이까지의 시간을 구하고 x속도를 제한한다. 유도탄처럼 매 틱 플레이어를 재추적하는 공통 코드가 아니다. maxFall/lifetime/delay/roll/blast/trail/visual은 정의에서 옮긴다.

공통 인간 사망은 Blast면 sprite를 30px 위로 올리고 충돌을 해제해 발사된 시체가 화면 아래로 나가게 한다. Melee면 slash clip 완료 시 Gone, 지상 총알이면 두 standing death 중 난수 선택 후 지정 fall frame에서 land/fall로 전환, 공중이면 jump/fall이다. Land 등 최종 단계에서는 blink/removeAtSec를 적용한다. [BeginSoldierDeath](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/EnemyCommon.cpp:524>), [TickSoldierDeath](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/EnemyCommon.cpp:588>).

## 8. 적 14종을 각각 설명하기

각 항목의 시간·수치 기본값은 C++ Config 초기값 또는 함수의 분기다. 실제 스테이지에서는 JSON이 override할 수 있다. 기본값을 곧 화면에서 검증된 튜닝값이라고 단정하지 않는다. 모든 종류의 Parse...Config 함수도 읽었으며, 필수 clip/weapon은 DefReader에서 논리 이름을 인덱스로 해석한다.

### 8.1 SoldierEnemy — 폭탄/대검/미사일 보병

[상태 선언](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/SoldierEnemy.h:4>), [Tick](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/Soldier.cpp:22>), [Go_](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/Soldier.cpp:315>), [AI 선택](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/Soldier.cpp:415>).

상태는 Wait, Fall, Land, Idle, Move, MoveDown, HJump, Throw, Slash, JumpSlashReady, JumpSlash, JumpSlashLand, Climb이다. Soldier에는 별도 private 숫자 진행 필드가 없으며 공통 state/stateTicks/climbSegment 등을 쓴다. config.ai는 Bomb/Sword/Missile이다.

Init는 variant≥2면 Climb, 그 외 Fall로 시작한다. variant 1(Stay)·3(ClimbStay)는 Move 요청을 Idle로 바꾼다. Climb은 실제 물리를 진행하지 않고 ClimbStep으로 움직이며 끝나면 Fall+ClimbLeap이다. 다른 상태에서는 물리 후 HoldOnRoof한다.

- Fall: 일반 낙하면 vx=0, 등반 도약 bLeap이면 vx 유지. 착지 후 active면 Land, 아니면 Wait.
- Idle: 공중이면 Fall. 지상이면 해당 AI 판단.
- Move: 플레이어를 보며 AI 판단. 계속 Move이면 speed로 전진하고 retreatBelow보다 가까우면 후진.
- MoveDown: 중력을 잠시 끄고 dropSpeed로 일방향 발판을 통과. 완료/착지 후 일반 낙하·Land.
- HJump: 물리 jumpVy와 가로 speed로 지형을 넘음.
- Throw: fireFrame에 weaponAttack 발사하고 클립 완료 후 Idle.
- Slash / JumpSlash: 지정 프레임에 StartMelee. JumpSlashReady에서 jumpReadyTicks가 지난 후 공중 공격 클립으로 전환하고 착지 후 JumpSlashLand.

Bomb AI는 cooldown 후 detect/closeStop/벽 probe/플레이어 높이와 2·3갈래 Roll로 이동·투척·점프·드롭을 고른다. Sword AI는 detect/jumpAttack/swordNear로 근접·점프 칼 범위를 나누고 난수로 이동/Idle/공격을 섞는다. Missile AI도 거리에 따른 이동/Throw와 지형 reposition을 쓰지만 Sword의 근접 상태를 쓰지 않는다.

Reposition_은 grounded+oneway이고 플레이어가 dropDy 아래면 MoveDown, 플레이어가 jumpDy 위면 HJump, 그 외 요청 fallback이다. 지붕 중간에서 JumpSlashReady를 요청하면 CanLeaveRoof가 false일 때 Move로 바꾼다. 사망은 공통 인간 사망이다. HashActionState는 climbSegment와 공통 사망 시작 값을 섞는다. [Reposition_](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/Soldier.cpp:293>), [사망](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/Soldier.cpp:200>).

### 8.2 BzkEnemy — 지상 바주카병

[헤더와 필드](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/BzkEnemy.h:19>), [Tick](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/Bzk.cpp:20>), [자세 선택](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/Bzk.cpp:343>).

상태: Wait/Fall/Idle/Move/HJump/Landing/Aim/ShotReady/Shot/Recover/Climb. private 필드: shotsRemaining, aimPosture, aimDown, jumpStarted, bHasAimPosture.

Idle에서 cooldown이 끝나면 detect 밖은 Move, 안은 Roll(3)으로 Move 또는 Aim. Move는 detect 안/roof edge면 Aim, 벽이면 HJump, cooldown이면 Idle, 그 외 접근한다. HJump는 **클립 jumpFrame**에 vy를 설정하기 전까지 vx=0이다. Soldier처럼 상태에 들어가는 즉시 jumpVy를 쓰는 것과 다르다.

Aim clip이 끝날 때 플레이어 center와 muzzleBase로 각도를 재계산한다. angleStep 네 경계를 비교해 자세 0..4, down 여부를 저장하고 bullets만큼 남은 탄을 설정한다. ShotReady는 readyTicks 대기. Shot의 fireFrame에서 posture×22.5도 방향으로 rocket 발사 후 remaining 감소, 클립 완료 시 탄이 남으면 ShotReady, 아니면 Recover→Idle이다. 각 탄마다 새 목표 각도를 계산하지 않고 Aim에서 저장한 자세를 이어 쓴다.

등반 variant≥2, roof hold, 공통 인간 사망을 쓴다. HashActionState는 남은 탄/자세 또는 climb segment/down/jumpStarted를 담지만 모든 private 필드를 완전 저장하지는 않는다. [Go_](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/Bzk.cpp:250>), [Config parser](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/Bzk.cpp:207>).

### 8.3 HangEnemy — 매달려 폭탄을 던지는 적

[헤더](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/HangEnemy.h:4>), [Tick](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/Hang.cpp:18>).

Idle/Throw 두 상태, private throwTicks다. Init에서 중력을 끄고 Idle. Idle은 FacePlayer하고 active이며 |dx|≤detectX일 때만 throwTicks를 증가한다. 범위 밖에 나가도 이 분기에서는 누적 throwTicks를 즉시 0으로 만들지 않는다. Idle로 다시 들어갈 때 0이 된다.

throwTicks에 도달하면 Throw, fireFrame에 bomb 무기 발사, clip 완료 후 Idle다. 살아 있을 때는 body.velocity=0이고 매달린 위치를 물리로 떨어뜨리지 않는다. 죽으면 grounded=false 후 공통 인간 사망으로 바뀐다. 난수 선택은 자체 Tick에 없다. 공통 사망/FX가 사용하는 EnemyWorld RNG는 별도다. [Go_](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/Hang.cpp:81>), [사망](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/Hang.cpp:51>).

### 8.4 FlyingBzkEnemy — 낙하·표류·사격·착지 변신

[필드](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/FlyingBzkEnemy.h:29>), [Tick](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/FlyingBzk.cpp:20>).

Descend/Shot 두 상태. private fallTicks, shotsFired, fireTicks, aimIndex, driftDirection, driftTicks. Init에서 중력을 끄고 driftDirection=-1. active 전에는 아무 이동도 하지 않는다.

driftFlipTicks마다 Roll(100)<flipPercent면 가로 표류 방향을 뒤집는다. x는 driftSpeed×step으로 직접 이동. y는 descendSpeed, 또는 fallGravity>0이면 fallTicks로 가속해 fallMaxSpeed까지 제한한다. 물리 Step 대신 직접 낙하 후 SnapToGround probe를 쓴다. 가속 낙하는 이전 y부터 이동 거리+landProbe를 훑어 착지를 확인한다.

착지하면 landedDef를 deferred 생성한다. facing/drop/userTag/activated를 전달하고 successor를 연결하며 Transformed 이벤트·낙하산 파괴 FX를 요청한다. 기존 객체는 bTransformed=true/Gone. **기존 HP와 z를 새 params로 복사하는 코드는 없다.** 이 경계를 “완전히 같은 개체가 땅으로 내려온다”라고 설명하면 정확하지 않다.

공중에서는 player center 각도가 postureDeg 창 안에 들어오면 aimIndex를 바꾼다. Descend에서 interval·maxShots·화면 내 center·player alive 조건을 통과하면 Shot. fireFrame에 fireDeg[aimIndex]로 rocket 발사 성공 때만 shot count 증가한다. 클립이 끝나면 Descend. bNoHurtbox 설정이면 Hurtbox 등록을 생략한다. [변신](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/FlyingBzk.cpp:51>), [판정](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/FlyingBzk.cpp:133>), [자세](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/FlyingBzk.cpp:198>).

### 8.5 ShielderEnemy — 방패 판정과 본체 판정

[상태/필드](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/ShielderEnemy.h:4>), [Tick](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/Shielder.cpp:30>), [Hurtbox](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/Shielder.cpp:274>).

Wait/Fall/Idle/Move/MoveStop, ShotStart/ShotReady/Shot/ShotEnd, LookStart/LookIdle/LookEnd. private shotsRemaining이고 방향/방패/defenceTicks는 EnemyState에 있다.

Idle은 cooldown 뒤 player dy와 absDx를 보고 앞/위/아래 경계·접근·사격을 고른다. Move는 바닥 probe 실패나 벽이면 MoveStop, moveMaxTicks이면 Roll(2), 사거리·높이에 따라 ShotStart/LookStart. Wait/Fall/Idle/Move 계열은 shieldUp=true. ShotReady/Shot/ShotEnd는 false다. LookIdle에서 guard 범위면 ShotReady, 멀면 LookEnd다.

ShotStart는 startTicks, ShotReady는 readyTicks를 기다리고 남은 탄을 감소한 뒤 Shot. fireFrame에서 shieldDir 0/1/2에 따라 forward/up/down 무기를 쏜다. 클립 완료 후 ShotReady, 다 쏘면 ShotEnd→Idle.

shieldUp일 때 **방패 Hurtbox를 본체보다 먼저** 등록한다. 방패는 owner|ShieldBit이며 Bullet만 받는다. 본체는 별도 owner로 기본 All mask다. 방패 OnHit는 defenceTicks를 설정하고 true를 반환해 기본 HP 피해를 막는다. 본체는 false를 반환해 기본 피해를 받는다. Blast/Melee는 방패 mask를 통과하지 않으므로 본체 판정으로 진행할 수 있다.

관통 총알의 ID는 방패와 본체를 서로 다른 targetKey로 본다. “모든 종류의 총알이 방패에서 완전히 멈춘다”는 일반화는 피한다. 표시 clip은 defenceTicks가 남고 방패가 올라가면 방어 clip을 사용한다. 사망은 방패 내린 뒤 공통 인간 사망. [OnHit](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/Shielder.cpp:244>), [Go_](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/Shielder.cpp:389>).

### 8.6 PowEnemy — 구출되는 NPC

[상태/필드](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/PowEnemy.h:4>), [Tick](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/Pow.cpp:18>).

Stuck/Escape/Walk/MoveDown/Glide/Close/Run. private turnTicks/turnDelayTicks/patrolTicks. Stuck+grounded일 때만 Hurtbox를 등록한다. OnHit는 항상 true로 기본 HP 피해를 막는다. Stuck이면 Escape로 전환하고 Freed 이벤트를 보낸다. 구출 수 증가는 이 이벤트를 StageState가 소비할 때다. [OnHit/Register](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/Pow.cpp:162>).

Escape clip 완료 → Walk. Walk는 walkSpeed로 순찰, 방향 전환 시간이 끝나면 뒤집고 다음 시간은 flipMinTicks+Roll(3)×(flipRangeTicks/2). 벽 충돌도 뒤집는다. 바닥이 없거나 공중이면 Glide, oneWay 위이며 최소 순찰 시간이 지난 뒤 player가 아래면 MoveDown. Glide는 terminal velocity를 glideMaxFall로 낮춘다.

살아 있는 플레이어 body와 접촉하면 Close. 특히 공중 Stuck도 접촉으로 Close할 수 있다. Close 시작은 플레이어 쪽을 향해 약간 밀어 위치를 조정한다. itemFrame에 drop이 비어 있지 않으면 ItemDropped 이벤트, clip 완료 → Run. Run은 항상 왼쪽 -runSpeed로 달리고 설정 시간에 깜빡이다 removeAtTicks 또는 화면 밖이면 Gone이다. aiVariant 1..3은 variant clip이 존재할 때 해당 표시를 사용한다. [Go_](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/Pow.cpp:254>).

### 8.7 JeepEnemy — 화면 가장자리 이동 후 후방 로켓

[상태/필드](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/JeepEnemy.h:4>), [Tick](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/VehicleJeep.cpp:17>).

Wait/IdleAndMove/Move/Turn/TurnToMove/Idle/Aimming/ShotIdle/ShotReady/Shot/ReAim/TurnAfterShot. private shotsRemaining, parkSide, hasTurned.

active 때 화면 가장자리 park 위치를 확인한다. 가장자리면 그 반대 방향으로 Move, 아니면 IdleAndMove 후 player x를 보고 이동 방향을 선택한다. Move target은 view.min.x+edgeInset 또는 view.max.x-edgeInset이다. targetTol 안으로 들어가면 Turn, clip 끝에 facing을 뒤집고 Idle.

이 지프의 발사 쪽은 **facing의 반대쪽**이다. PlayerOnFireSide는 dx×facing<0이고 FireWeapon에 back=-facing을 넘긴다. Idle에서 플레이어가 발사 쪽이면 bullets를 채워 Aimming→ShotIdle→ShotReady→Shot. ShotIdle은 첫 발, remaining=2 또는 0일 때 별도 pause를 넣고 나머지에서는 바로 준비한다. Shot의 fireFrame에 remaining의 홀짝에 따라 rocketShiftX를 다르게 해 후방 발사한다. 다 쏘면 ReAim→TurnAfterShot→IdleAndMove.

이동은 물리를 사용하며 죽음은 차량 공통이다. 자체 Tick에는 난수 분기 없다. 카메라 가장자리가 움직이면 target도 그 기준으로 계산되므로 “고정 월드 x로 주차한다”는 설명은 틀리다. [target/발사 쪽](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/VehicleJeep.cpp:234>).

### 8.8 BTankEnemy — 고정 포대의 개방·연속 발사

[헤더](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/BTankEnemy.h:4>), [Tick](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/VehicleBTank.cpp:21>).

Closed/Open/Idle/Fire, private shotsRemaining. Init에서 facing=-1, Closed. active가 아니면 player x≥enemy x-activateDx 조건으로 활성화할 수 있고, 그 전에도 물리는 진행한다.

Closed→Open, 개방 clip 시간이 끝나면 Idle. Idle는 open clip을 마지막 위치에 유지하고 idleTicks 후 shotsPerCycle을 채운다. Fire는 fireFrame에서 shell을 발사하고 clip 완료 때 remaining 감소. 남으면 Fire 재시작, 없으면 Idle. 방향을 플레이어에 따라 매 틱 뒤집지 않는다.

Go_는 Idle의 같은 open clip을 restart하지 않고 clipTicks를 최소 duration까지 올린다. 단순히 Idle clip 이름만 선택한다고 설명하면 열린 상태 유지 이유가 빠진다. 사망은 destroy clip+차량 공통 timer. [Go_](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/VehicleBTank.cpp:138>).

### 8.9 MissileTankEnemy — 거리로 발사대 위/아래 선택

[헤더](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/MissileTankEnemy.h:4>), [Tick](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/VehicleMissileTank.cpp:27>).

Idle/Aim/Ready/Shot. private shotsRemaining, launcherPosition(0=down,1=up). Init는 down+Idle이고 appear clip이 있으면 먼저 보인다.

Idle에서 dx>rangeDx면 stateTicks=0으로 탐지 대기를 계속 늦춘다. detectTicks가 끝나면 dx>farDx인지로 원하는 위/아래를 정한다. 현재와 다르면 Aim clip을 진행해 모드를 뒤집고 Idle, 같으면 Ready다. Ready 대기 후 bullets→Shot. fireFrame마다 EnemyWorld Roll(24)로 y jitter 0..23px를 만들고 up/down 무기를 고른다. clip 끝에서 남은 탄을 줄이고 반복하거나 Idle.

자체 Tick은 bActive를 별도로 검사하지 않는다. EnemyWorld의 화면 despawn/초기화는 적용되지만 “모든 적은 bActive가 false이면 Tick 내용을 중단한다”는 일반화는 틀리다. 사망 clip은 현재 launcher 모드에 맞춰 고른다. [난수 발사](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/VehicleMissileTank.cpp:78>).

### 8.10 SarubiaEnemy — 전진·정지·회전·유탄

[헤더](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/SarubiaEnemy.h:4>), [Tick](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/VehicleSarubia.cpp:19>).

Idle/Turn/MoveStart/Move/MoveStop/ShotStart/Shot. private 진행 필드는 없고 공통 stateTicks를 사용한다. inactive 전에도 물리는 진행하지만 AI는 반환한다.

Idle cooldown 뒤 플레이어가 뒤이면 Turn, 앞이고 chaseDx 밖이면 MoveStart, 가까우면 ShotStart. Turn clip 완료 시 facing 반전. MoveStart 완료→Move, Move는 speed로 전진하고 moveMaxTicks 또는 stopDx 안이면 MoveStop. Stop 완료→Idle. ShotStart readyTicks→Shot, fireFrame에 grenade, clip 완료→Idle다.

자체 난수 분기 없다. 유탄이 착지·굴림하는 방식은 Sarubia Tick이 아니라 해당 ProjectileDef→ShotSpawn→ProjectileSystem에서 결정한다. 차량 공통 사망이다. [Go_](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/VehicleSarubia.cpp:167>).

### 8.11 BusEnemy — 개방, 7단계 파손, 계단 병사, 잔해 유지

[헤더](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/BusEnemy.h:4>), [Tick](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/Bus.cpp:134>), [Config](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/EnemyDef.h:531>).

Parked/Active, private elapsedTicks, damageStage, stageStartTicks[8], nextSoldierTick. Init는 Parked, facing=-1, 일반 body clip=-1로 파츠 그림만 제출한다.

Parked는 매 Tick def.hp로 Health를 다시 만든다. cameraView.max.x≥bus x+openViewDx이면 Active로 전환하고 elapsed=0. 그 뒤 elapsed를 늘려 HP가 threshold 이하인 동안 while로 damageStage를 **여러 단계 한 틱에** 증가시킬 수 있다. 각 단계 시작 틱과 설정 bursts를 기록한다. C++ 기본 threshold는 112/100/85/62/48/34/24이며 JSON override 가능하다. [UpdateStage_](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/Bus.cpp:493>).

계단 병사는 openDelay+stairsDelay+32fly+6land+soldierFirst가 지나야 생성된다. 마지막 허용 파손 단계, 다음 spawn tick, 같은 tag의 CountAlive<soldierMax를 검사한다. Roll(100) 하나로 sword/bazooka/bomb를 선택한다. variant=Climb 또는 ClimbStay, arena 경유점·climbEnd·roofSpan·tag를 전달해 deferred spawn한다. 생성 직후 nextSoldierTick=elapsed+gap다. [병사 생성](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/Bus.cpp:433>), [생성 조건](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/Bus.cpp:473>).

SubmitLive_는 Parked/개방 지연이면 Bus_Idle, 개방 후 계단 Fly→Land→Rail와 OpenBody/OpenTop을 표시한다. 파손 단계는 차체 0/1/2/3 등급으로 묶고 각 Window는 해당 단계 시작 틱으로 짧게 파괴 clip을 재생한다. 창 안 병사는 visual 파츠이며 EnemyWorld의 실제 병사 객체와 별개다.

사망은 deathBursts를 요청하고 deathTicks를 계속 늘린다. wreckAfterTicks 전에는 최종 파손 차체, 그 뒤 Bus_Wreck. SubmitDebris_는 deathTicks/60을 시간으로 포물선을 계산하고 지면 y에서 멈춘다. 이 잔해는 여기서 계산한 렌더 파츠이며 물리 충돌체가 아니다. DeathDone은 destroyDoneTicks 이상이다. TickDying은 Gone을 쓰지 않으므로 잔해 버스 객체가 남을 수 있다. [사망/렌더](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/Bus.cpp:159>), [잔해 계산](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/Bus.cpp:506>).

EnemyWorld::GetSolidBoxes는 살아 있는 bus의 solid, landing wall, roof thickness 12px 상자를 추가한다. 버스가 Dying이 되면 이 함수의 Alive 필터에서 제외된다. Sandbox buslab은 IsDestroyFinished를 확인한 뒤 지면을 Stage0_1로 바꾸고 카메라 bounds를 넓힌다. Stage0 일반 진행은 별도 Runner 게이트가 담당한다. [버스 solid](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/EnemyWorld.cpp:847>), [buslab 해제](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/SandboxState.cpp:220>).

### 8.12 HeliEnemy — 고공 폭격, 낮은 통과, 미사일 제자리 발사

[상태·비행 필드](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/HeliEnemy.h:4>), [Tick](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/Heli.cpp:109>).

Bomb/Low/PodOut/Move/Volley/MoveBack/Turn. HeliFlight는 이동 start/target x/y, bombTime, turnFlip, podTicks, cycle, passDirection, idleTicks를 소유한다. m_turnWaitTicks도 별도다. start/target은 좌표×16을 lround한 int로 저장하고 다시 /16해 움직인다. 전체 몸 위치가 항상 fixed-point라는 뜻은 아니다. [Pack/Curve](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/Heli.cpp:12>).

Init는 중력 false, podTicks=-1, startLow면 Low 아니면 Bomb. inactive면 Tick 반환. Bomb는 camera.top+hoverDy로 내려오고 player x+followDx를 deadzone/속도 제한으로 따라간다. 첫 cycle에는 firstBombTicks, 이후 reBombTicks를 사용해 burstCount발의 bombInterval과 burstRest를 반복한다. 일정 highBursts 후 Low.

플레이어가 facing 반대쪽 turnDx보다 멀리 turnWaitTicks 동안 머무르면 TurnIn→facing 반전→TurnOut→Bomb. bombTime을 저장했다가 stateTicks를 복원해 고공 폭격 일정의 진행 위치를 이어 간다. Turn 동안에도 StepFollow가 실행된다.

Low는 화면 반대 가장자리 lowMargin을 향해 가로로 이동하고 lowDy까지 내려오며 lowBursts를 떨어뜨린다. 이후 pod 표시→PodOut 대기→화면 한쪽 target으로 Move. Move/MoveBack는 x/y 각각 13개 표의 선형 보간으로 이동한다. Volley는 puffToMissile 지연, missileInterval, missiles 수로 연기와 발사를 배치하고 끝나면 높은 위치로 MoveBack, cycle++→Bomb.

폭탄은 Shot/Bomb가 아니라 **HeliBombEnemy 정의를 deferred spawn**한다. 발사 방향별 pod/rotor 파츠, pitch 프레임과 rotor row는 렌더에서 고른다. Turn 중 파츠 Submit을 생략한다. 사망은 차량 공통 연출/타이머이고 bVisible=false로 본체를 숨긴다. 자체 AI 난수는 없다. [DropBomb_](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/Heli.cpp:410>), [SubmitParts/BodyFrame](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/Heli.cpp:237>), [StepMove_](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/Heli.cpp:381>).

### 8.13 HeliBombEnemy — 적 객체인 낙하 폭탄

[헤더](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/HeliBombEnemy.h:4>), [Tick](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/HeliBomb.cpp:43>).

별도 enum FSM이 없으며 Alive/Dying과 private blastTicks를 사용한다. Init는 중력 false, startVy, meleeHitId 하나, fall clip. active이면 vy+=gravity×step 후 maxFall 제한, y 이동.

probe x가 플레이어의 hit/blast 폭 범위 안이고 player alive이면 player feet 높이까지 내려오는 것을 목표로 본다. 그 위에서는 벽/지면을 지나가고, 발 높이에 도달하면 자체 HP를 0으로 만든다. 그렇지 않은 방향이면 probe가 지면 또는 Wall 안에 있는지로 폭발을 결정한다. **플랫폼 상자를 검사하는 코드가 없다.** 도착 전에는 Bullet HitKind의 낙하 접촉 상자를 만든다.

EnemyWorld가 HP 0을 보고 Dying으로 바꾸면 본체를 숨기고 blastTicks=0. 다음 dying 틱부터 config.blastTicks 동안 Blast 상자를 같은 meleeHitId로 등록하고, 완료 뒤 ID 해제/Gone이다. 낙하 접촉과 폭발이 같은 공격 ID를 쓰는 점이 ProjectileSystem의 새 Blast ID와 다르다. [TickDying/AddBombHitbox_](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/HeliBomb.cpp:87>).

### 8.14 BigShieeEnemy — 7개 포탑과 주포, 선체·카메라 진행

[상태 종류](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/BigShiee.h:5>), [전용 필드](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/BigShiee.h:31>), [Tick](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/BigShiee.cpp:271>), [Config](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/BigShieeConfig.h:28>).

보스 phase는 Entrance/Battery/Deploy/Cannon. 포탑 모드는 Idle/Prep/Turn/Shot/Dead, 주포 모드는 Hidden/Deploy/Aim/Ready/Shot이다. 이것은 phase 하나로 모든 부품을 동기화한 단일 상태만 있는 구조가 아니다. 7개의 turret HP/mode/ticks/position, 주포 yaw/row/ticks/발사 예정, age/deploy/lid/scroll/drive/casing/낙하산/작은 폭발 일정이 함께 존재한다.

Init는 facing=1, age=0, 각 포탑 HP와 side×8의 조준 위치, first volley/chute를 설정한다. 주포는 Hidden, deploy/lid/lastKill=-1. 카메라가 유효하면 선체 x는 camera.left+startScreenX로 옮긴다. 최초 Tick 전에는 보스 Hurtbox와 파츠 그림을 등록하지 않는다. [Init](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/BigShiee.cpp:235>), [초기 판정 guard](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/BigShiee.cpp:540>).

**선체와 카메라:** ageSec가 scrollStart를 지났고 pause가 아니며 누적 budget에 닿지 않았으면 cameraScroll=scrollSpeed×step을 월드에 요청한다. Entrance는 (초,screenX) key를 선형 보간해 camera.left+now에 선체를 놓는다. 마지막 key 시간이 끝나면 Battery다. 이후 Battery/Cannon은 driveTriggerX/driveTargetX, 가속·제동 sqrt(2a×remaining)·최저 14px/s로 돌진하며, 설정된 reverse/shuttle drift 분기도 있다. speedX는 lround한 int, moving은 |speed|>12다. 카메라 pause는 명시 표, deploy pause, 반복 late pause 조건을 합친다. [스크롤/돌진](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/BigShiee.cpp:281>), [pause/budget](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/BigShiee.cpp:843>).

**포탑 공격:** TickVolley는 살아 있는 Idle 포탑 목록에서 Roll(count)로 하나를 Prep으로 선택하고 다음 volley 시간을 범위 난수로 정한다. 모두 바쁘면 30틱 뒤 재시도다. 포탑 Idle은 turretTrackTicks가 켜져 있으면 player x 밴드에 따라 0/4/8 위치로 한 칸씩 추적한다. Prep 뒤 목표와 다르면 Turn, 같으면 Shot. Turn은 turnFrameTicks마다 한 칸 이동, Shot은 fireTick에 첫 shell과 puff, twin이면 pairGapTicks 뒤 두 번째 shell, shotTicks 후 Idle다. shell의 무기/탄도는 공통 FireWeapon을 사용한다. 조준 pose가 자체 유도 shell을 뜻하지 않는다. [TickTurret_](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/BigShiee.cpp:924>), [TickVolley_](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/BigShiee.cpp:992>), [FireShell_](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/BigShiee.cpp:907>).

**부위 피해:** 각 살아 있는 turret는 Bullet/Blast만 받는 고유 part owner Hurtbox다. OnHit는 turretHp를 직접 줄인다. 0이 되면 Dead, lastKill=age, PartDestroyed(score=turretScore), explosion/smoke 요청. 주포는 CannonVulnerable일 때만 본체 Health를 줄인다. 다른 owner/부위도 OnHit는 true로 기본 본체 피해를 차단한다. CannonVulnerable은 Hidden=false, Deploy 중이면 cannonTicks≥30, 그 외 true다. [OnHit](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/BigShiee.cpp:426>), [CannonVulnerable_](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/BigShiee.cpp:1060>).

파생 OnHit=true 경로는 EnemyWorld::OnHitEvents의 즉시 BeginDying을 거치지 않는다. 주포 HP가 0이 되면 이후 EnemyWorld::Tick의 Alive+dead 검사에서 Died/OnDeath가 발생한다. HP를 줄이는 코드와 생명주기 전환 코드의 위치를 구분해야 한다.

**전개와 주포:** 모든 turret가 파괴되고 lastKill 이후 deployDelay가 지나면 Deploy, lidStart 기록. cannonDeployDelay 뒤 cannonMode=Deploy. 지정 deployTicks 또는 clip 길이가 끝나면 Aim; player center 방향의 yaw 0..8 중 가장 가까운 값을 향해 cannonTurnTicks마다 한 칸 회전한다. 처음 맞춰지면 Ready와 firstMissile 예정, 보스 phase=Cannon. Ready는 예정 시간이 지나면 5개 발사 row에 해당하는 **짝수 yaw**로 맞춰 Shot에 들어간다. Shot의 missileFireTick에 미사일, puff/smoke, casing 위치 기록. 마지막 프레임을 제외한 ShotTicks 길이가 끝나면 Ready다. [TickCannon_](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/BigShiee.cpp:1187>), [yaw/row 선택](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/BigShiee.cpp:1070>), [FireMissile_](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/BigShiee.cpp:1160>).

**지원 POW와 낮은 HP 연출:** 일정마다 pow_chute를 deferred spawn한다. 낙하 거리/속도로 앞으로 카메라가 이동할 거리를 예상해 x를 정하고, Roll로 가로 spread와 MG/RL/SG/FS drop을 고른다. 체력이 lowHpFxBelow 비율 아래면 turret HP와 본체 HP 합계 비율로 작은 폭발 주기를 줄이며 x/y 난수를 뽑는다. 이 시각 연출 난수도 EnemyWorld의 같은 RNG를 소비한다. [TickChute_](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/BigShiee.cpp:1020>), [low HP FX](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/BigShiee.cpp:370>).

**사망과 렌더:** 사망 직후 대폭발, Dying에서는 age가 아니라 deathTicks 시간으로 랜덤 폭발/큰 불꽃을 배치한다. dyingWreckSec 뒤 Wreck/CannonWreck과 smoke, dyingDoneSec면 DeathDone=true다. TickDying은 Gone을 쓰지 않는다. SubmitParts는 hull→antenna→turrets→cannon→casing 순으로 제출하지만 depth/sort가 최종 그림 순서를 결정한다. casing은 포물선·착지 후 fade를 렌더에서 계산하며 standalone strip 6칸을 사용한다. [죽음](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/BigShiee.cpp:474>), [파츠 제출](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/BigShiee.cpp:578>), [casing 계산](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/BigShiee.cpp:1125>).

**검사 경계:** BigShiee의 HashActionState는 ageTicks만 5값 형식의 일부로 넘긴다. 모든 turret HP/mode/yaw/casing/scroll RNG가 해시에 들어간다고 설명할 수 없다. HeliFlight도 Heli 해시에서 전체 저장하지 않는다. [보스 hash](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/BigShiee.cpp:1377>), [헬기 hash](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/Heli.cpp:527>).

## 9. 적 정의 데이터가 실제 코드가 되는 과정

EnemyDef는 몸/피격/solid, activation/physics, clip·weapon·death·drops, overlay와 14종 Config를 소유하는 값 데이터다. 파생 객체는 defIndex를 통해 현재 라이브러리 정의를 읽는다. JSON의 논리 clip/weapon 이름은 로드 때 인덱스로 바꾼다. 문자열 찾기를 매 틱 모든 곳에서 하는 것은 아니지만, BigShiee named parts처럼 렌더에서 이름을 쓰는 경로도 있다. [EnemyDef](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/EnemyDef.h:732>).

EnemyDefParser의 중요한 실제 규칙은 다음과 같다.

1. LoadDirectory는 .json 파일 경로를 정렬해 읽는다. BOM/주석 허용 옵션을 사용한다.
2. root version은 1만, 정의 이름 중복은 오류다.
3. extends는 parent부터 chain을 쌓고 깊이>8에서 오류. abstract인 leaf 정의는 실제 항목을 만들지 않는다.
4. 상속 scalar/object를 찾을 때 자식부터 역순 Find, clips/weapons/params object는 parent→child로 이름별 Merge한다. 같은 이름의 항목을 자식 전체 값으로 바꾼다. 모든 중첩 객체를 재귀 deep merge한다는 구현은 아니다.
5. projectile을 먼저 파싱하고 그 뒤 enemy를 파싱해 무기의 projectile 인덱스를 해석한다.
6. Enemy hp≥1, body w/h>0, clip frames≥1/frameSec>0 등의 필수 조건과 알려진 키를 검사한다.
7. DefReader는 각 행동의 params를 읽으면서 bUsed를 표시한다. Finish에서 남은 미사용 param은 “이 행동이 쓰지 않는 값” 오류다.
8. FlyingBzk landedDef와 Bus 일부 병사 정의 참조를 마지막에 검사한다.
9. `missle_tank`라는 데이터 이름 철자는 현재 mapping의 실제 문자열이다. C++ MissileTank 이름과 혼동해 문서에서 자동 교정하지 않는다.

[정렬 로드](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/EnemyDefLoader.cpp:1467>), [extends/merge](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/EnemyDefLoader.cpp:419>), [두 pass](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/EnemyDefLoader.cpp:1345>), [행동 parser dispatch](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/EnemyDefLoader.cpp:1230>), [DefReader::Finish](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/EnemyDef.cpp:397>), [종류 문자열](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/EnemyDef.cpp:12>).

정수/시간 변환도 구분한다. EnemyDefReader Int는 Num→float→double→lround이고, Props 파서 Int는 소수 부분이 있으면 오류다. 이름이 둘 다 Int라고 동일하게 반올림하는 것은 아니다.

**읽혔지만 현재 이 월드 경로가 쓰지 않는 예:** activation.bLatch는 읽고 보관하지만 실제 EnemyWorld activation 조건에서 읽지 않는다. EnemyDef.drops 목록은 파서가 보관하지만 현재 StageState::ConsumeEnemyEvents_는 EnemyState/SpawnParams의 문자열 drop으로 아이템을 생성한다. 따라서 “정의의 drops 배열이 Died 때 모두 자동으로 나온다”는 결론은 이 소스에 없다. [drops 파싱](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/EnemyDefLoader.cpp:1156>), [실제 Died 소비](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Stage/StageState.cpp:654>).

에디터 경로는 실제 기능이다. FireWeaponNow/PlayMotion은 MotionEditor 호출자가 있고, RefreshBodies는 HitboxEditor에서 쓰며, ReloadEnemyDefs/Keep·KillAll은 EnemyLab에서 쓴다. “테스트만 쓰는 API”라고 한 묶음으로 삭제 대상으로 설명하지 않는다. Keep reload는 정의/투사체 개수·이름 순서·behavior·clip 개수가 같을 때 기존 객체를 유지한다. [MotionEditor 호출](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Editor/MotionEditor.cpp:447>), [HitboxEditor 호출](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Editor/HitboxEditor.cpp:154>), [EnemyLab reload](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Editor/EnemyLab.cpp:136>), [Keep reload guard](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/SandboxState.cpp:299>).


## 10. PropWorld: 데이터로 나누는 소품의 상태와 충돌

### 10.1 정의와 인스턴스

PropDef는 원본 설정, PropInstance는 한 번 배치한 물체의 현재 진행 상태다. 정의에는 HP, 시각 참조, parts, variants, 파괴 연출/드롭/자식 생성, fall, secondary, groundPatch가 있고, 인스턴스에는 id/defIndex/variant/pos/Health, 선택한 visual, 애니메이션, 피격·파괴·낙하·착지·제거 타이머가 있다. [정의](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Props/PropDef.h:126>), [인스턴스](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Props/PropWorld.h:59>).

SpawnByIndex는 ID를 5000부터 증가시킨다. 명시 variant가 없으면 변종 개수로 RollInt한다. variant의 dy·idle/hit/parts를 적용하고 spawn params의 sprite/hitbox/standBox/HP override를 덮는다. z override가 있으면 인스턴스 depth로 사용한다. 이 경로의 hitbox/standBox override는 해당 역할의 **첫 part만** 바꾼다. [스폰](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Props/PropWorld.cpp:150>).

소품 그림은 Anim/Sprite/None 참조로 표현한다. ComputeVisual_ 우선순위는 falling의 Idle/Landed → Destroyed 그림이 있는 파괴 상태 → Hit 그림의 피격 재생 → HP threshold의 Damaged → Idle다. bDestroyed인데 destroyed visual이 없으면 그 분기를 통과하지 않고 다른 시각 상태를 선택할 수 있다. “파괴 boolean은 항상 같은 그림 하나”가 아니다. [ComputeVisual_](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Props/PropWorld.cpp:436>).

### 10.2 part의 role과 when

| role | 실제 등록 결과 |
|---|---|
| Hurt | Neutral Hurtbox. AcceptsFor(false), 즉 Bullet/Blast |
| Wall | 플레이어/적에 쓰일 Wall 배열 |
| Platform | OneWay=true 발판. Stage는 플레이어와 적 배열에 모두 복사 |
| EnemyPlatform | OneWay=true 발판. Stage는 적 배열에만 복사 |

when은 Intact/Destroyed/Always다. RebuildCollision_는 bRemove가 아닌 소품의 활성 part만 읽어 배열을 새로 만든다. Hurtbox 등록은 별도 함수다. parts 한 항목의 role이 문자열 배열이면 파서는 동일 size/offset/when의 여러 PropPart를 만들어 준다. [판정 등록](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Props/PropWorld.cpp:254>), [충돌 배열](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Props/PropWorld.cpp:645>), [role 배열 파싱](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Props/PropDef.cpp:327>).

HP=0이 피격 불가라는 정의 주석이 있지만, RegisterHurtboxes 자체는 HP>0을 필터하지 않고 parts를 본다. 결과 피해는 Health::ApplyDamage가 이미 HP≤0이면 0을 반환한다. “HP 0은 Hurtbox가 등록되지 않는다”로 설명하지 않는다.

### 10.3 한 발이 소품을 파괴하는 실제 순서

1. Combat 이벤트의 target으로 인스턴스를 찾는다. 이미 destroyed/remove면 무시한다.
2. Health::ApplyDamage하고 양수 피해면 Damaged 이벤트를 push한다.
3. Hit visual이 있으면 재생 flag/timer=0.5초와 bind flag를 재설정한다.
4. threshold를 새로 넘었고 아직 alive이면 onDamaged burst를 한 번 실행한다.
5. HP가 죽었으면 Destroy_를 실행한다.
6. Destroy_는 destroyed/timer/표시·제거 상태를 쓰고 Destroyed 이벤트를 push한다.
7. onDestroy burst → drops → children pending → 조건부 secondary timer → groundPatch 이벤트 순서다.
8. 전체 hit 이벤트를 읽은 뒤 pending children을 배열 끝에 스폰하고 충돌을 rebuild한다.

[OnHitEvents](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Props/PropWorld.cpp:280>), [Destroy_](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Props/PropWorld.cpp:336>).

Detonate_의 explosion은 clip/count/spread/offset으로 pending 시각 FX를 생성하고, debris는 clip 종류 난수와 초기 vx/vy를 뽑는다. **이 PropBurst 자체는 Combat의 폭발 Hitbox를 만들지 않는다.** 소품 파괴 연출과 실제 피해 Blast를 같은 의미로 설명하지 않는다. drops는 지정 count만큼 x spread 난수 후 ItemWorld::Spawn을 호출하고 DropSpawned 이벤트를 기록한다. [Detonate_/QueueDrops_](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Props/PropWorld.cpp:383>).

secondary는 def에 존재하는 것과 spawn params의 explodeOnDeath가 모두 true일 때 예약된다. 시간이 0보다 작아진 시점에 burst와 SecondaryBurst 이벤트를 만든다. falling 소품은 자체 gravity로 내려오고 GroundMap::FindGroundY를 검사해 halfHeight로 착지, afterLandSec 뒤 onDestroy 시각 burst와 제거를 한다. 이 경로는 Destroy_를 호출하는 일반 피격 파괴와 다르다. [Tick](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Props/PropWorld.cpp:494>).

### 10.4 소품 Tick의 순서와 컬링

Decor Tick → pending FX 생성/EffectList Update → debris 물리/애니메이션·수명 제거 → 각 소품의 낙하/파괴/secondary/visual 진행 → 카메라 뒤 컬링 → bRemove 소품 제거와 Removed 이벤트 → ItemWorld Tick → 아이템 FX 생성 → 충돌 rebuild다.

컬링은 center x<cameraLeft-cullMargin일 때만 후보가 된다. 현재 visual bounds의 max.x가 여전히 화면 왼쪽보다 오른쪽이면 남긴다. 큰 건물을 중심만으로 일찍 지우지 않는 실제 guard다. bounds는 sprite pivot와 w/h 또는 현재 animation frame으로 만든다. [컬링](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Props/PropWorld.cpp:595>), [visual bounds](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Props/PropWorld.cpp:719>).

SortKey는 보통 int(pos.y+sortOffsetY)다. sortByX면 -int(pos.x)×4096+int(pos.y)다. 건물끼리 가로 위치의 정렬 영향이 큰 선택이며, 모든 소품의 정렬이 발 높이 순서라고 일반화할 수 없다. Submit은 소품 → debris → effects → items → decor를 제출한다. [정렬/제출](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Props/PropWorld.cpp:674>).

### 10.5 GroundPatch는 그림과 지면의 명시된 연결이다

Destroy_는 groundPatch sprite 이름과 top-left를 이벤트로 보낸다. StageState는 texture png를 GroundMap mask로 캐시하고 활성 지면에 ApplyGroundPatch를 호출한다. 빈 mask와 로드 실패는 적용하지 않는다.

ApplyGroundPatch는 mask의 solid 픽셀에 해당하는 기존 지면을 air로 바꾼다. 그러나 어떤 열의 가장 아래 solid를 지우는 mask이거나 사각형 안 기존 solid의 50%를 넘게 지우려 하면 먼저 거부한다. total==0도 false다. 검사 완료 뒤 쓰기를 하므로 거부되는 경우 일부만 지우는 경로가 아니다. [patch 소비](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Stage/StageState.cpp:690>), [mask 캐시](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Stage/StageState.cpp:715>), [guard와 쓰기](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Props/PropWorld.cpp:773>).

SandboxState는 props.TakeEvents를 버리므로 이 지형 patch 이벤트를 적용하지 않는다. Stage와 Sandbox를 “같은 소품을 쓰니 지형 변화도 동일”하다고 설명하지 않는다. [Sandbox 이벤트 폐기](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/SandboxState.cpp:540>).

### 10.6 PropDef 파서의 경계

PropDefParser는 root version 1, 알려진 키, role/when/visual 구문, item/group/child/drop 참조와 명시 변종 범위를 검사한다. Visual은 anim 또는 sprite 중 정확히 하나다. Int는 float로 읽은 값이 int 변환값과 같아야 하며 소수는 오류다. 변종이 없는데 variant≥0이면 오류다.

FindMissingAsset은 시각 참조·burst debris 전체 clip·groundPatch 크기·items asset을 검사하는 함수지만 이번에 확인한 호출자는 Test/PropDefTest다. Stage Load_가 이 함수를 자동 실행한다고 주장하지 않는다. [Int](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Props/PropDef.cpp:214>), [SpawnParams](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Props/PropDef.cpp:832>), [FindMissingAsset](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Props/PropDef.cpp:1019>), [테스트 호출](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Test/PropDefTest.cpp:150>).

## 11. ItemWorld: 바닥에 있는 보급품과 점수

### 11.1 생성과 난수

ID는 7000부터 증가한다. 이름이 item group이면 별도 Item RNG로 하나를 뽑은 뒤 실제 정의를 찾는다. Body는 feet 기준 32×32, gravity scale을 1400/default gravity로 설정한다. score+roll이면 [-50,49] 범위의 float 가로 속도를 예약하고, bounce이면 난수 한 번을 소비한 뒤 vy=-400/vx=0을 준다. [Spawn](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Props/ItemWorld.cpp:71>).

begin Anim이 있으면 beginAnim부터 재생하고 끝나면 기본 visual Anim으로 바꾼다. bPickedUp인 동안에는 pickupSec를 줄이고 물리를 더 하지 않는다. 살아 있는 score roll은 지상에서 예약 vx를 한 번 사용하고 roll friction으로 감속하며 stop speed 아래면 0. Physics Step 후 spinScale을 지상 속력에 맞춘다. [Tick](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Props/ItemWorld.cpp:132>).

아이템은 age≥15초 또는 cameraLeft-50보다 뒤이면 제거한다. 10.5초 이후에는 0.2초 단위로 표시를 깜빡인다. 수명은 정수 tick 카운터가 아니라 float ageSec다. [상수](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Props/ItemWorld.h:16>), [컬링](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Props/ItemWorld.cpp:210>).

### 11.2 Collect와 적용을 분리한다

Collect는 playerBox와 32×32 item bottom-center 상자가 겹치면 ItemPickup 값을 만든다. Score item이면 ScorePopup을 만들고 즉시 제거 대상 age로 변경한다. Gun/Supply는 bPickedUp=true, pickupSec=0.06초, Get_Item_Effect 요청으로 잠깐 남긴다. 같은 picked item을 다시 Collect하지 않는다. [Collect](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Props/ItemWorld.cpp:236>).

ApplyItemPickup의 실제 동작:

- Score: 지정 score를 AddScore.
- Gun: 지정 weapon을 PickUpWeapon. 탄량은 item.amount가 아니라 Tuning의 해당 무기 pickup ammo를 사용한다.
- Bomb supply: item.amount만큼 AddBombs.
- Bullet supply: 현재 Pistol이 아니면 현재 무기를 다시 PickUpWeapon해 기본 pickup ammo를 더한다. 여기에서도 amount를 직접 더하지 않는다.

[적용 함수](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Props/ItemWorld.cpp:462>), [무기 pickup ammo](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Player/PlayerController.cpp:69>).

Stage/Sandbox의 Collect 호출은 이번 본문에서 player.IsAlive로 guard하지 않는다. 문서에서 “살아 있을 때만 아이템을 먹는다”는 추가 조건을 넣으면 안 된다. 실제 사용되는 playerBox와 입력/죽음 흐름에서 어떤 결과가 생기는지는 실행 검증 없이 단정하지 않는다.

ScorePopup은 2.8초 수명, 0.07초 간격 색 교대, 1.05초 뒤 rise=0.5×500×t²다. 렌더용 값이며 플레이어의 점수 증가와 별도다. item/popup Z는 기본 깊이 34/46 또는 shift한 override를 사용한다. [Submit](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Props/ItemWorld.cpp:313>).

## 12. DecorWorld와 EffectList: 시각 전용 진행

Decor는 충돌/피해/HP가 없다. MakeDecorSpawn은 anim_decor params의 velocity/flip/loop/scale/sequence를 읽는다. sequence 항목은 clip, seconds(최소 0.01), velocity, flip이다. 없으면 단일 clip에 1e9초 구간을 만든다. ID는 자체 1부터 증가한다. [params](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Props/DecorWorld.cpp:12>), [Spawn](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Props/DecorWorld.cpp:70>).

Tick는 필요하면 EnterStep, 남은 시간을 빼고 최대 64개 구간을 넘긴 뒤 **최종 선택된 구간의 velocity로 이번 dt 전체를 이동**한다. 구간 경계를 넘은 dt를 각 구간 속도에 비례 분할하는 구현은 아니다. 비반복 sequence가 끝나면 finished=true, velocity=0이고 마지막 애니메이션 자체는 update한다. [Tick](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Props/DecorWorld.cpp:136>).

Submit은 FarBackground depth=4, 다른 기본 decor=11과 지정 z를 선택하고 y sort로 그림을 제출한다. ClearByLayerMask는 layer bit로 삭제하고 Clear는 ID도 1로 초기화한다. [Submit/Clear](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Props/DecorWorld.cpp:163>).

EffectList는 애니메이션과 위치/depth/flip, 선택적 debris velocity/age만 소유한다. 일반 effect는 AnimPlayer가 끝나면 삭제, debris는 5초 또는 cameraBottom+100 아래이면 삭제한다. debris는 중력 1000과 초기 vx[-250,249], vy[-800,-301] 공용 상수를 사용한다. 이 list는 Hurtbox/Hitbox를 등록하지 않는다. [EffectList](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Sandbox/Effects.cpp:25>), [수명](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Sandbox/Effects.cpp:64>).

## 13. StageWorldHost: 데이터 요청과 실제 세계 사이

### 13.1 외부 ID와 내부 핸들

IStageHost는 SpawnEntity, defeat/destroy 질의, clear/command, ground, player, backdrop/UI/audio/end 등을 StageRunner에 제공하는 인터페이스다. StageWorldHost는 실제 EnemyWorld/PropWorld/Player/UI 포인터를 **소유하지 않고 바인딩**한다. 지면 map과 collider/placeholder/route 값은 직접 소유한다. [인터페이스](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Stage/StageHost.h:61>), [필드](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Stage/StageWorldHost.h:121>).

SpawnId는 1부터 증가하며 route에 eRouteKind와 내부 handle을 저장한다. placeholder combat owner는 별도의 9000 이후 번호다. SpawnId 1과 player owner 1은 용도가 다르며 Combat의 target에 SpawnId를 직접 써서는 안 된다. [AllocId/Route](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Stage/StageWorldHost.cpp:330>), [placeholder](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Stage/StageWorldHost.cpp:395>).

### 13.2 SpawnEntity 분기

1. Collider면 MakeColliderBox 후 별도 콜라이더 배열/route.
2. Enemy/Npc이고 EnemyWorld가 바인딩됐으면 facing/HP/z/drop/variant/userTag=stage ID로 실제 적 생성. 성공하면 Enemy route.
3. Prop이면 params를 MakePropSpawnParams로 만들어 중심 위치에 PropWorld 스폰. 성공하면 Prop route.
4. Item이면 Feet 또는 legacy center 변환을 해 ItemWorld 스폰. 성공하면 Item route.
5. Decor+anim_decor+anim 이름이면 실제 DecorWorld에도 스폰한다.
6. 위에서 반환하지 않은 요청은 placeholder를 만들고 Placeholder route.

[SpawnEntity](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Stage/StageWorldHost.cpp:417>).

anim_decor는 실제 decor 생성 후에도 6번 placeholder를 만든다. 이 route는 DecorWorld의 ID를 직접 보관하지 않는다. `SetBackdrop` 구현은 비어 있지만 StageState::DrawBackdrop_은 Runner의 backdrop mode를 읽는다. 빈 host 메서드를 근거로 전체 backdrop 표시가 없는 것처럼 설명하지 않는다. [SetBackdrop](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Stage/StageWorldHost.cpp:636>), [실제 backdrop 읽기](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Stage/StageState.cpp:885>).

요청 구조체에는 settle/fromCamera/ambush/cullMargin/rect 등의 많은 정보가 있지만 모든 필드가 이 Host에서 사용되는 것은 아니다. 해당 정보가 Runner에서 먼저 해석되는지와 Host에서 무시되는지를 나누어 읽어야 한다. BindRects는 여기서는 복사 저장만 하고 이후 body에 읽는 사용처가 없다.

### 13.3 질의와 clear/command

IsDefeated/IsDestroyFinished는 route 종류별로 실제 월드에 위임한다. enemy는 successor를 해석하는 EnemyWorld 질의, prop은 destroyed/visual duration, item은 존재 여부, collider는 배열 존재 여부다. placeholder는 HP/deadTicks로 판단한다.

ClearEntities는 placeholder/decor/collider를 layer mask로 제거한다. Monster bit가 있어야 EnemyWorld::ClearMonsters를 호출하며 Npc bit도 있으면 NPC 포함 여부를 전달한다. 실제 PropWorld 소품 전체를 layer별로 제거하는 호출은 이 함수에 없다. Item route도 별도로 지우지 않는다. [clear](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Stage/StageWorldHost.cpp:540>).

SendCommand는 실제 Enemy route이면 즉시 반환해 효과가 없다. 그 외는 최초 한 번 경고한다. schema/interface가 존재한다고 “적 command 행동이 구현됐다”라고 설명하지 않는다. [SendCommand](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Stage/StageWorldHost.cpp:557>).

### 13.4 플레이스홀더도 실제 전투 경로에 들어간다

enemy/npc/prop kind는 hittable, decor/item 등은 비피격이다. 등록 Hurtbox 팀은 Enemy다. ApplyPlaceholderEvents는 피해/flash/죽음 카운터를 적용하고 kills 수를 반환한다. TickPlaceholders에서 30틱 뒤 제거한다. Stage는 kills×100 점수, debug draw에서만 placeholder 상자/글자를 보인다. “보이지 않으니 시뮬레이션에서도 없다”는 결론은 틀리다. [등록/이벤트](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Stage/StageWorldHost.cpp:203>), [표시](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Stage/StageState.cpp:962>).

### 13.5 Ground, Warp, UI, Audio, End

SetGround는 저장된 named map 중 활성 인덱스를 바꾼다. GroundFindY는 x/fromY를 Floor한 뒤 활성 map에서 찾는다. PlayerFeet는 낙하산을 포함한 logical feet다.

WarpPlayer는 현재 score만 보관하고 Player::Reset 후 score를 복구한다. 목숨·무기·타이머를 그대로 유지하는 순수 좌표 이동 API가 아니다. Invincible은 초→60Hz 틱으로 바꿔 ExtendInvincibility, Banner/Fade는 UI에 요청, BGM/SFX는 AudioSystem에 요청, EndStage는 nextState 이름과 end requested flag만 쓴다. 실제 상태 교체는 StageState::TakeNextState에서 일어난다. [플레이어/warp](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Stage/StageWorldHost.cpp:605>), [UI/audio/end](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Stage/StageWorldHost.cpp:641>), [TakeNextState](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Stage/StageState.cpp:847>).

## 14. StageState의 시작과 종료, 충돌과 렌더

Load_는 StageData → sprite/backdrop textures → named GroundMaps(실패 시 flat fallback) → Tuning → EnemyDefs/PropDefs → host bind → camera/runner → rig/폭탄/낙하산 클립 순서로 준비한다. 정의 로드 실패는 placeholder 경로로 남을 수 있다. loaded flag는 마지막에 true다. [Load_](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Stage/StageState.cpp:126>).

Restart_는 host/enemies/props, resolver·등록·투사체·효과·patch cache·stage RNG·flow/timer/구출 수를 reset한 뒤 Runner::Begin을 호출한다. start feet를 resolve하고 충돌을 만든 다음 player reset/snap/chute, camera target/snap, UI/이전 stats를 설정한다. ground map의 변경 픽셀까지 원본 이미지로 다시 로드하는 호출은 Restart_에 없다. tickCount나 labSetUp을 이 함수에서 초기화하지도 않는다. [Restart_](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Stage/StageState.cpp:274>).

UpdateWalls_에서 화면벽은 플레이어 배열에만, props wall과 enemy solid는 양쪽, props 일반 platform은 양쪽, enemyPlatform과 stage collider platform은 적에만 넣는다. Stage collider wall은 양쪽이다. [충돌 조립](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Stage/StageState.cpp:579>).

종료 흐름 시작 시 Mission이면 구출 수의 bonus score를 더하고 ScreenFlow를 시작한다. 이후 IsFlowing이면 game world tick을 진행하지 않고 UI/flow만 update한다. IsFinished는 fail/flow done/runner finished를 구분하고 TakeNextState는 한 번만 반환한다. Mission+host end request면 지정 next state, 그 외 title다. [StartFlow_](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Stage/StageState.cpp:530>), [finished](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Stage/StageState.cpp:830>).

Render는 이전/현재 카메라를 alpha로 보간 후 floor한다. backdrop → world → 선택 debug → HUD다. DrawWorld_는 EnemyWorld를 먼저 submit하고 terrainDepth 미만 sprite를 먼저 flush한 뒤 terrain texture를 직접 그린다. 이후 placeholder debug/player+chute/bullets/bombs/props/effects를 제출하고 나머지를 flush한다. 제출 순서와 depth 경계, 직접 그리는 texture가 함께 최종 순서를 만든다. [Render](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Stage/StageState.cpp:862>), [terrain 경계](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Stage/StageState.cpp:942>).

## 15. SandboxState와 TrainingTargets

### 15.1 실제 실행 상태와 설정

StateFactory는 sandbox/proplab/enemylab/vehiclelab/buslab/helilab과 각 bot/god 변형을 SandboxState로 만든다. bosslab은 BossLab.json을 쓰는 StageState다. 이 이름들이 Test/Tools에서만 생성되는 추상 fixture라고 단정하지 않는다. CreateState의 실제 실행 경로다. [StateFactory](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/StateFactory.cpp:10>).

BuildWorld_는 tuning watcher(60틱 poll), ground 로드/flat fallback, forward camera, EnemyDefs를 준비하고 RestartGame, Props/prop lab 배치, rig/폭탄/스프라이트/배경을 준비한다. RestartGame은 resolver/projectiles/effects/targets/enemies/player/camera를 reset하고 설정에 맞춰 기본 targets·적·차량·버스·헬기를 생성한다. **RestartGame 본문은 props를 reset하지 않는다.** BuildWorld_에는 별도의 props.Reset이 있다. [BuildWorld_](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/SandboxState.cpp:47>), [RestartGame](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/SandboxState.cpp:155>).

### 15.2 Sandbox Tick의 정확한 순서

bot 입력 합성·watcher poll → BeginTick → props tick → UpdateWalls → debug spawn/weapon → targets/enemies/props Hurtbox → player Tick → item Collect → targets shooters → enemy solid wall 구성 → EnemyWorld Tick → Projectile Step → Resolve → props hits와 prop events 폐기 → enemies hits와 score events → buslab unlock → targets damage/score → player hits → projectiles hits → targets respawn tick → FX/sound/UI/effects/bomb/dummy animation/rig → camera update → gameover timeout의 RestartGame이다. [전체 Tick](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/SandboxState.cpp:440>).

Stage와 중요한 차이:

| 경계 | Stage | Sandbox |
|---|---|---|
| 진행 | Runner/section/gate/curve/end | Runner 없음, 실험 설정/타깃 |
| Projectile Step | active ground + enemy platforms | ground만 |
| enemy physics | props wall/플랫폼, 적 전용 발판, collider, enemy solid | config.platforms + enemy solid walls |
| item collect | 적용 + kind별 소리 | 적용, pickup 직접 소리 분기 없음 |
| enemy events | Died drop, Part score, Freed count, ItemDropped 실제 아이템 | 모든 event.score 합계, drop/freed는 별도 적용 없음 |
| prop events | 폭발음/ground patch 소비 | TakeEvents 결과 폐기 |
| camera | Runner::Tick | player feet/lookAhead로 직접 update |
| gameover | flow 완료 후 next/title | 지정 시간 후 같은 sandbox restart |

[Sandbox enemy physics](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/SandboxState.cpp:388>), [이벤트 처리](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/SandboxState.cpp:540>), [카메라/재시작](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/SandboxState.cpp:606>).

### 15.3 TrainingTargets는 Enemy 파생 클래스가 아니다

TrainingDummy는 Grunt/Jeep/Bus 세 kind, id/feet/size/Health/flash/deadTicks/shootTimer/hits/lastKind/anim을 가진 값 구조체다. ID는 1000 이후 ++로 배정한다. Grunt만 Melee까지 받으며 Jeep/Bus는 Bullet/Blast만 받는다. 실제 JeepEnemy/BusEnemy의 복잡한 FSM을 여기서 실행하지 않는다. [TrainingDummy](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Sandbox/TrainingTargets.h:20>), [등록](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Sandbox/TrainingTargets.cpp:79>).

Shooter는 player alive/range 안이면 timer를 줄여 수평 Enemy Bullet을 생성한다. range 밖/죽은 플레이어면 timer를 매번 초기화한다. ApplyEvents는 hitsTaken/lastKind/flash/HP를 쓰고 첫 사망 때 kind별 100/500/1000점과 kill count. Tick는 flash를 감소하고 dummyRespawnTicks에 체력을 복구한다. Grunt 그림은 Sandbox가 SD_Idle/죽음 종류 clip으로 맞춘다. [Shooter](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Sandbox/TrainingTargets.cpp:97>), [피해/부활](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Sandbox/TrainingTargets.cpp:142>).

## 16. 검사 전용·진단 경로와 실제 게임 경로

이 표는 현재 검색한 Source/Test/Tools 호출 관계다. 함수 이름의 Lab/Debug/Hash만으로 “미사용”을 판단하지 않는다.

| 경로 | 확인한 사용과 한계 |
|---|---|
| Player/Enemy/Prop/Item/Target/Host/Stage HashState | 상태의 DebugHash와 Test/SimCheck가 소비. HP/좌표 등 일부 값만 검사한다. |
| Enemy::HashActionState_ | 실제 AI 진행을 바꾸지 않는 해시 형식 경계. 5개의 int64 값을 기록하며 사망 시작 값으로 progress를 덮는 규칙 포함. |
| ReseedRng | 검색된 호출은 BusTest. 일반 Stage/Sandbox는 Reset(1) 사용. |
| FireWeaponNow / PlayMotion | MotionEditor가 실제 사용. 기존 Tick의 motion override 경로가 실행된다. |
| RefreshBodies | HitboxEditor가 실제 사용. 정의 몸 크기 편집의 현재 객체 반영. |
| KillAll | EnemyLab/테스트에서 사용. Alive/Pending 객체를 배열에서 제거하고 선택적으로 Died 점수 이벤트 생성. 일반 OnDeath 연출과 동일하지 않다. |
| ClearMonsters | Stage clear/EnemyWorld 테스트에서 사용. 살아있는/죽은 상태와 무관하게 team 기준 제거. |
| AddTarget | 테스트에서 직접 추가. 기본 Sandbox는 AddDefaultTargets_로 생성. |
| bBusBot / bHeliBot / bAutoDamage | StateFactory의 bot 실행 상태로 선택 가능. 기본 stage0에서는 false. |
| FindMissingAsset | Test/PropDefTest에서 호출 확인. 게임 Load_에 자동 검사 호출 없음. |
| SendCommand | Stage schema/interface 존재. 현재 실제 적 command 행동 없음. |
| activation.bLatch / EnemyDef.drops | 파싱·저장 있음. 위에서 설명한 실제 소비 경계는 제한적. |

[해시 소비](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Tools/SimCheck/Main.cpp:47>), [5값 형식](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/Enemy.cpp:85>), [KillAll](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/EnemyWorld.cpp:1033>), [Reseed 테스트](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Test/BusTest.cpp:156>).

HashState의 중요한 누락:

- Player hash는 input buffer, aim/overlay, RNG 내부, knife/flame/throw의 전체 진행을 담지 않는다.
- Enemy hash는 position×16의 int64 양자화이고 Body velocity와 모든 파생 필드/RNG를 담지 않는다.
- HeliFlight 및 BigShieeData 대부분이 HashActionState에 들어가지 않는다.
- Prop hash는 일부 timer/visual/debris/items를 포함하나 decor와 RNG/모든 pending 요청을 담지 않는다.
- Stage/Sandbox hash는 Bullet/Bomb **개수**를 섞는다. 모든 탄 위치/속도/Shot/Blast 상태를 직렬화하지 않는다.

[Player hash](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Player/PlayerController.cpp:344>), [Enemy hash](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/EnemyWorld.cpp:1061>), [Prop hash](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Props/PropWorld.cpp:843>), [Stage hash](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Stage/StageState.cpp:811>).

그러므로 해시 같음은 특정 값이 같다는 검사 근거이며 모든 틱의 입력 소비·RNG 소비·그림 순서가 완전히 같다는 증명은 아니다. 이 문서는 테스트를 실행하지 않았으므로 기존 결과 숫자를 현재 통과 결과처럼 제공하지 않는다.

## 17. 읽은 파일별 역할과 주요 함수 찾아보기

같은 행에 헤더와 구현을 묶었지만 두 파일의 선언과 본문을 모두 확인했다. getter/name/hash 반복 함수는 행동의 직접 책임과 구분해 읽었다.

| 파일 | 주 역할과 본문 진입점 |
|---|---|
| [PlayerController.h](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Player/PlayerController.h:13>), [PlayerController.cpp](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Player/PlayerController.cpp:10>) | Context/상태 소유. Reset/Tick/TickAlive/TickDead/Kill/Respawn/MakeRigState/Hash/getter |
| [PlayerMovement.cpp](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Player/PlayerMovement.cpp:118>) | 이동·점프·앉기·낙하산·조준·상태도출 |
| [PlayerAttack.cpp](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Player/PlayerAttack.cpp:10>) | 행동 조율, 칼/폭탄/총/버스트/화염/seed/총구 |
| [PlayerInput.h](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Player/PlayerInput.h:6>), [PlayerInput.cpp](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Player/PlayerInput.cpp:8>) | Command/버퍼 Reset·Update·Read·Consume |
| [PlayerHitboxes.h](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Player/PlayerHitboxes.h:7>), [PlayerHitboxes.cpp](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Player/PlayerHitboxes.cpp:20>) | 판정/총구/방향/폭탄 위치의 순수 계산 |
| [PlayerTypes.h](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Player/PlayerTypes.h:4>), [PlayerTypes.cpp](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Player/PlayerTypes.cpp:3>) | 논리 enum/owner/이름 표시 |
| [PlayerWeapons.h](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Player/PlayerWeapons.h:6>), [PlayerWeapons.cpp](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Player/PlayerWeapons.cpp:29>) | 탄·간격, 기본/HMG/Flame shot 제작, 정적 선택 |
| [PlayerRig.h](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Player/PlayerRig.h:48>), [PlayerRig.cpp](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Player/PlayerRig.cpp:140>) | clip table/Bind/fallback/Update/Submit |
| [CombatSystem.h](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Combat/CombatSystem.h:5>), [CombatSystem.cpp](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Combat/CombatSystem.cpp:5>) | 등록 배열·ID·resolver 연결·target probe |
| [Projectiles.h](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Combat/Projectiles.h:8>), [Projectiles.cpp](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Combat/Projectiles.cpp:206>) | spawn 값·이동·접촉·blast·hit 소비·FX take |
| [HitFlash.h](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Combat/HitFlash.h:5>), [HitFlash.cpp](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Combat/HitFlash.cpp:3>) | flash 시간/재시작 guard |
| [BulletVisual.h](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Combat/BulletVisual.h:10>), [BulletVisual.cpp](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Combat/BulletVisual.cpp:75>) | 탄 표시와 무기 소리 매핑 |
| [Enemy.h](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/Enemy.h:47>), [Enemy.cpp](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/Enemy.cpp:18>) | virtual 계약/기본 행동/14종 factory/hash 경계 |
| [EnemyTypes.h](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/EnemyTypes.h:104>), [EnemyTypes.cpp](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/EnemyTypes.cpp:3>) | handle/context/life/common state/part owner/flash |
| [EnemyWorld.h](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/EnemyWorld.h:19>), [EnemyWorld.cpp](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/EnemyWorld.cpp:346>) | 생성/소유/Tick/hit/제거/후속 핸들/solid/render/에디터 API |
| [EnemyDef.h](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/EnemyDef.h:12>), [EnemyDef.cpp](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/EnemyDef.cpp:50>) | 정의·Config, 시간/clip/part/name/DefReader |
| [EnemyDefParse.h](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/EnemyDefParse.h:15>), [EnemyDefLoader.cpp](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/EnemyDefLoader.cpp:13>) | 로드 경계/extends/merge/키 검사/refs/파일정렬 |
| [EnemyCommon.h](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/EnemyCommon.h:6>), [EnemyCommon.cpp](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/EnemyCommon.cpp:71>) | 상태·물리·probe·등반·발사·근접·인간사망·named parts |
| [VehicleCommon.h](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/VehicleCommon.h:5>), [VehicleCommon.cpp](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/VehicleCommon.cpp:5>) | 차량사망·FxAt·clip 완료 |
| [SoldierEnemy.h](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/SoldierEnemy.h:4>), [Soldier.cpp](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/Soldier.cpp:22>) | 보병 3 AI/Go/Reposition/Climb/사망/Parse |
| [BzkEnemy.h](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/BzkEnemy.h:4>), [Bzk.cpp](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/Bzk.cpp:20>) | 자세 선택/준비·탄 수/등반/Parse |
| [HangEnemy.h](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/HangEnemy.h:4>), [Hang.cpp](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/Hang.cpp:18>) | 매달림/투척 누적/Parse |
| [FlyingBzkEnemy.h](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/FlyingBzkEnemy.h:4>), [FlyingBzk.cpp](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/FlyingBzk.cpp:20>) | 표류·낙하·화면사격·착지변신/Parse |
| [ShielderEnemy.h](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/ShielderEnemy.h:4>), [Shielder.cpp](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/Shielder.cpp:30>) | 방패 우선 판정/고도방향/상태별 clip/Parse |
| [PowEnemy.h](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/PowEnemy.h:4>), [Pow.cpp](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/Pow.cpp:18>) | 구출/순찰·활공/드롭/변종 clip/Parse |
| [JeepEnemy.h](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/JeepEnemy.h:4>), [VehicleJeep.cpp](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/VehicleJeep.cpp:17>) | 화면 주차/후방 로켓/Parse |
| [BTankEnemy.h](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/BTankEnemy.h:4>), [VehicleBTank.cpp](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/VehicleBTank.cpp:21>) | 개방·고정 포격/Parse |
| [MissileTankEnemy.h](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/MissileTankEnemy.h:4>), [VehicleMissileTank.cpp](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/VehicleMissileTank.cpp:27>) | 거리·발사대 위치/로켓 jitter/Parse |
| [SarubiaEnemy.h](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/SarubiaEnemy.h:4>), [VehicleSarubia.cpp](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/VehicleSarubia.cpp:19>) | 회전·이동·유탄/Parse |
| [BusEnemy.h](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/BusEnemy.h:4>), [Bus.cpp](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/Bus.cpp:134>) | 7단계 파손·계단병사·분리파츠·잔해/Parse |
| [HeliEnemy.h](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/HeliEnemy.h:4>), [Heli.cpp](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/Heli.cpp:109>) | flight 상태/곡선·폭격·missile·rotor/Parse |
| [HeliBombEnemy.h](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/HeliBombEnemy.h:4>), [HeliBomb.cpp](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/HeliBomb.cpp:43>) | 낙하·발높이 도착/같은 ID의 blast/Parse |
| [BigShieeEnemy.h](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/BigShieeEnemy.h:5>), [BigShiee.cpp](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/BigShiee.cpp:271>) | 보스 행동/부위피해/카메라/파츠·casing/Parse |
| [BigShiee.h](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/BigShiee.h:31>), [BigShieeConfig.h](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/BigShieeConfig.h:28>) | phase·turret/cannon 모드와 진행 값/설정 |
| [PropDef.h](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Props/PropDef.h:126>), [PropDef.cpp](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Props/PropDef.cpp:102>) | 소품/item/group/part 구문·refs·spawn params·asset 검사 |
| [PropWorld.h](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Props/PropWorld.h:124>), [PropWorld.cpp](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Props/PropWorld.cpp:150>) | 소품 spawn/hit/destroy/collision/tick/render/patch |
| [ItemWorld.h](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Props/ItemWorld.h:74>), [ItemWorld.cpp](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Props/ItemWorld.cpp:71>) | 아이템 group/random/physics/Collect/apply/popup |
| [DecorWorld.h](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Props/DecorWorld.h:37>), [DecorWorld.cpp](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Props/DecorWorld.cpp:12>) | 시각 sequence/이동/층별 삭제 |
| [Effects.h](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Sandbox/Effects.h:17>), [Effects.cpp](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Sandbox/Effects.cpp:25>) | effect/debris 수명과 표시 |
| [TrainingTargets.h](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Sandbox/TrainingTargets.h:20>), [TrainingTargets.cpp](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Sandbox/TrainingTargets.cpp:48>) | dummy 생성·피격·수평발사·부활 |
| [SandboxState.h](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/SandboxState.h:21>), [SandboxState.cpp](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/SandboxState.cpp:440>) | sandbox 구성·lab·세계조율·reload·render·hash |
| [StageHost.h](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Stage/StageHost.h:61>), [StageHost.cpp](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Stage/StageHost.cpp:3>) | 스테이지 호출 인터페이스, 기본 section hook |
| [StageWorldHost.h](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Stage/StageWorldHost.h:65>), [StageWorldHost.cpp](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Stage/StageWorldHost.cpp:417>) | 실제 spawn route/ground/placeholder/collider/UI/audio |
| [StageState.h](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Stage/StageState.h:38>), [StageState.cpp](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Stage/StageState.cpp:310>) | Load/Restart/Tick/events/flow/render |

## 18. 직접 설명할 수 있는지 확인하는 질문

코드를 고치거나 테스트를 돌리는 과제가 아니라, 본문과 링크를 보며 호출 순서를 말로 설명하는 질문이다.

1. Player Tick 전에 적 Hurtbox를 등록하고 Player Tick 안에서 플레이어 Hurtbox를 등록하는 이유가 무엇이라고 **추론**할 수 있는가? 실제 동작과 추론을 어떻게 나눌 것인가?
2. Jump pressed와 Jump buffered와 grounded와 coyoteAvailable은 각각 언제 쓰이고 소비되는가?
3. 바로 일어날 공간이 없을 때 앉은 몸의 크기와 가로 속도는 어떻게 남는가?
4. HMG 표시가 DiagUp이어도 실제 탄 각도가 계속 변할 수 있는 이유는 무엇인가?
5. Shot 입력 한 번이 칼로 소비되면 뒤쪽 TickShot_는 어떤 조건 때문에 발사하지 않는가?
6. 폭탄 releaseTick 이전의 bombsThrown/bombs 값과 이후 값은 어떻게 다른가?
7. 마지막 HMG 탄의 OnFired가 Pistol로 바뀌는 순간 남은 burst는 어떻게 되는가?
8. 화염을 “공격 1회”, “입자 1개”, “HitResolver 피해 기록”으로 나누면 각 수명은 무엇인가?
9. 같은 공격 ID로 10틱 칼을 등록해도 같은 owner에 한 번만 피해가 생기는 이유는?
10. 비관통 Hitbox가 등록 순서상 방패에 먼저 닿으면 어떤 값이 기록되는가?
11. Resolve 중 새 Blast를 만들지 않고 OnHitEvents 뒤에 만들면 그 Blast의 최초 피해 틱은 언제인가?
12. EnemyWorld의 Pending과 inactive는 왜 서로 다른 상태인가?
13. deferred spawn은 ID가 배정되는 시점, 객체가 생기는 시점, Init/Tick 시점이 어떻게 다른가?
14. FlyingBzk→landedDef에서 전달하는 값과 전달하지 않는 값은 무엇인가?
15. POW OnHit가 true를 반환하면 기본 Health 경로와 Freed 이벤트는 어떤 관계인가?
16. Jeep의 facing은 왜 “총구 방향”과 반대가 될 수 있는가?
17. Bus damage threshold를 여러 개 넘는 피해를 받으면 그 한 Tick에서 stageStartTicks는 어떻게 기록되는가?
18. Heli 폭탄을 projectile이라고만 설명하면 어떤 Enemy lifecycle을 놓치는가?
19. BigShiee의 전체 phase와 각 turret/cannon mode가 동시에 존재하는 이유를 코드 사실과 설계 해석으로 나눠 설명할 수 있는가?
20. PropBurst의 시각 Explosion과 Projectile Blast의 실제 피해 차이는 무엇인가?
21. Gun/Bullet supply의 amount와 tuning pickup ammo가 실제로 어디에서 사용되는가?
22. Decor sequence 경계에서 dt를 나누지 않는 실제 결과를 어떻게 설명할 것인가?
23. Stage에서 작동하는 groundPatch가 Sandbox에서는 왜 적용되지 않는가?
24. “같은 해시”가 모든 RNG/보스 부위 상태/그림 순서의 동치를 보장하지 않는 이유는?
25. Init 전에 보스가 hidden인 것과 Pending 적이 hidden인 것은 같은 guard인가?
26. 어떤 소스는 실제 게임, 어떤 소스는 에디터, 어떤 함수는 검사 전용인지 caller로 증명할 수 있는가?

## 19. 확인 범위와 남은 한계

86파일의 선언과 실제 구현 본문을 읽었다. 14종 적의 Init/Tick/상태 전이/공격/죽음/부위 표시와 각 Config parser를 확인했다. 파일별 핵심 함수와 호출 경계는 위 링크로 연결했다.

현재 JSON/PNG의 모든 수치·픽셀과 이 구현의 화면 결과를 대조한 문서는 아니다. fallback 경로의 실제 출현 빈도, 에셋 누락 여부, 타이밍/시각 일치, 원작 parity는 이번 작업에서 실행 확인하지 않았다. 테스트 파일은 일부 호출 경계를 정적으로 찾았고 테스트를 실행하거나 테스트 통과를 새로 주장하지 않았다. World의 전체 Physics/Rendering/UI는 01 문서에서 설명한다. StageData/Runner의 전체 schema/진행은 별도 스테이지 문서 범위다.

작성 주체·생성 이력·개인 시행착오를 파일 크기나 구조만으로 추정하지 않았다. 설계 해석은 책임과 실제 데이터 소유 관계로만 제시했고, 그 해석을 사용자 스타일의 확정 기준으로 채택하지 않았다.



