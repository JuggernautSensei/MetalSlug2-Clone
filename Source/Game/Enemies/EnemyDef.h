#pragma once
#include <cstdint>
#include <optional>

#include "Base/Result.h"
#include "Base/Types.h"
#include "Base/Math.h"

#include "World/Combat.h"
#include "World/Geometry.h"

namespace slug
{

// ===========================================
//  적 정의 (Assets/Defs/Enemies.json)
//   수치(HP/박스/속도/타이머/클립 이름/공격 파라미터)는 전부 JSON 에서 오고 코드는 행동 "종류" 만 안다.
//   스키마는 Docs/EnemyDefsSchema.md, 수치 근거는 Docs/EnemySpecs.md.
//   시간은 JSON 에서 초 단위이며 시뮬레이션에서는 틱(1/60 s)으로 환산해 쓴다.
// ===========================================

// 행동 종류 (switch / 테이블 인덱스). 새 적 종류는 여기에 추가한다 (Docs: 새 적 추가 방법).
enum class eEnemyBehavior : uint8_t
{
    Soldier,     // 일반 병사 3종 (aiType: bomb / sword / missile)
    Bzk,         // 바주카 병사
    Hang,        // 매달린 병사
    FlyingBzk,   // 낙하산 바주카 (착지 시 Bzk 로 변신)
    Shielder,    // 방패병
    Pow,         // 포로 (Older)
    Bus,         // 버스 중간보스 (MBTruck): 탄/병사 투하, 8단계 파손
    BattleShip,  // 최종보스 (Big Shiee): 레일/캐논/포탑 2기, 파괴 연출
    Jeep,        // 이동 로켓 지프 (Docs/EnemySpecs.md 2.1)
    BTank,       // 고정 포탑 전차 (2.3)
    MissleTank,  // 미사일 전차 (2.4)
    Sarubia,     // 보행 전차 / 중간보스 게이트 (2.5)
    Aparch,      // 공격 헬기 (2.6)
    Count
};

[[nodiscard]] const char*                    GetBehaviorName(eEnemyBehavior _behavior);
[[nodiscard]] std::optional<eEnemyBehavior> FindBehavior(slug::StringView _name);

// 애니메이션 시간 단위 (Render/Animation.h 의 kAnimTimeUnitsPerSec / 60). 한 틱 = 100,000 단위.
inline constexpr int64_t kAnimUnitsPerTick = 100'000;

// 초 → 틱 (가장 가까운 정수)
[[nodiscard]] int SecToTicks(float _sec);

// ===========================================
//  박스 / 클립
// ===========================================

// 발 기준 박스 (중심 cx, cy / 크기 w, h). 오른쪽(+1) 기준이며 왼쪽을 볼 때 x 가 미러된다.
struct BoxDef
{
    float w  = 0.f;
    float h  = 0.f;
    float cx = 0.f;
    float cy = 0.f;

    [[nodiscard]] Aabb At(const Vec2 _feet, const int _facing) const
    {
        const float cxWorld = _feet.x + cx * static_cast<float>(_facing < 0 ? -1 : 1);
        return Aabb::FromCenterSize({ cxWorld, _feet.y + cy }, { w, h });
    }

    [[nodiscard]] bool IsEmpty() const { return w <= 0.f || h <= 0.f; }
};

// 클립 타이밍 (anim.json 과 같아야 한다 — 테스트가 검증). 시뮬레이션이 애니메이션 라이브러리 없이도 같은 타이밍으로 돌도록 JSON 에 둔다.
struct ClipDef
{
    slug::String name      = {};      // "SD_Idle_{side}" — {side} 는 Left/Right 로 치환
    int         frames    = 1;
    float       frameSec  = 0.1f;
    bool        bLoop     = false;
    int64_t     frameUnits = 0;      // frameSec 를 애니메이션 시간 단위로 환산한 값 (로더가 채운다)
    bool        bSided    = false;   // name 에 {side} 가 있는가

    [[nodiscard]] bool IsValid() const { return !name.empty(); }

    // 클립 시작 후 _ticks 틱이 지났을 때 보이는 프레임 (루프는 순환, 비루프는 마지막 프레임에서 정지)
    [[nodiscard]] int FrameAt(int _ticks) const;

    // 프레임 _frame 에 처음 진입하는 틱 (상태 진입 후 경과 틱). 이벤트 시각 계산용
    [[nodiscard]] int TickOfFrame(int _frame) const;

    // 비루프 클립이 끝나는(마지막 프레임이 다 재생되는) 틱. 루프 클립에는 의미가 없다.
    [[nodiscard]] int GetDurationTicks() const;

    [[nodiscard]] bool IsFinished(int _ticks) const { return !bLoop && _ticks >= GetDurationTicks(); }
};

struct NamedClip
{
    slug::String name = {};   // 논리 이름 ("idle", "throw", ...)
    ClipDef     clip = {};
};

// 일회성 이펙트 요청 정의 (애니메이션 이름 + 오프셋(발 기준, 좌향이면 x 미러) + 지연)
struct FxDef
{
    slug::String  clip     = {};
    Vec2 offset   = { 0.f, 0.f };
    float        delaySec = 0.f;
    int          layer    = 60;
};

// ===========================================
//  무기 / 투사체
// ===========================================

struct ProjectileDef
{
    slug::String  name            = {};
    Vec2 size            = { 20.f, 20.f };
    int          damage          = 1;
    float        speed           = 0.f;   // 일정 속도 (방향 벡터에 곱한다). 0 이면 v0 / speedRamp 를 쓴다
    bool         bRamp           = false;   // 선형 가속: 0 → rampMax (진행 방향)
    float        rampFrom        = 0.f;
    float        rampAccel       = 0.f;
    float        rampMax         = 0.f;
    Vec2 v0              = { 0.f, 0.f };   // 포물선 초기 속도 (x 는 바라보는 방향 부호가 곱해진다)
    float        gravity         = 0.f;
    float        maxFall         = 3000.f;
    float        lifetimeSec     = 5.f;
    float        startDelaySec   = 0.f;
    bool         bExplodeOnSolid = true;
    ClipDef      begin           = {};   // 점화 대기 동안의 클립 (선택)
    ClipDef      fly             = {};
    FxDef        explosion       = {};   // 적중/지형 접촉 시 연출 (clip 이 비면 없음)
    // 구르는 탄 (사루비아 수류탄): 착지 후 지면을 rollSpeed 로 구르고 rollSec 뒤 소멸. 착지 연출은 land 클립
    float        rollSpeed       = 0.f;
    float        rollSec         = 0.f;
    bool         bExpireFx       = false;   // 수명 종료 시에도 explosion 연출 (시각 전용)
    ClipDef      land            = {};      // 착지 직후 클립 (rollSpeed > 0 일 때 fly 앞에 재생)
};

enum class eWeaponAim : uint8_t
{
    Facing,   // 바라보는 방향 수평
    Given     // 호출자가 방향 벡터를 준다 (바주카/방패병)
};

struct WeaponDef
{
    slug::String  name            = {};
    int          projectile      = -1;   // EnemyDefLibrary::GetProjectile 인덱스 (-1 = 투사체 없음)
    Vec2 muzzle          = { 0.f, 0.f };   // 발 기준, 좌향이면 x 미러
    eWeaponAim   aim             = eWeaponAim::Facing;
    float        tiltThreshold   = 0.f;   // > 0 이면 |dy| 가 이 값을 넘을 때 방향에 tiltDy 만큼 y 기울임
    float        tiltDy          = 0.f;
    bool         bArc            = false;   // 포물선: 플레이어 근처에 떨어지도록 수평 속도를 계산한다
    float        arcMinVx        = 0.f;
    float        arcMaxVx        = 0.f;
    FxDef        muzzleFx        = {};   // 발사 연출 (오프셋은 muzzle 기준)
    bool         bHitbox         = false;   // 근접 판정 무기 (칼): Hitbox 를 직접 등록한다
    BoxDef       hitbox          = {};
    float        hitboxLifeSec   = 0.f;
    int          hitboxDamage    = 1;
    bool         bHitboxFollows  = false;   // true 면 판정이 몸을 따라 이동 (점프 베기)
};

// ===========================================
//  사망 / 드롭
// ===========================================

// 병사류 사망 연출 (논리 클립 인덱스는 EnemyDef::clips 기준, -1 = 없음)
struct DeathDef
{
    int   clipBomb       = -1;   // 폭발 계열 피격 → 날아감
    int   clipSlash      = -1;   // 근접(칼) 피격 → 베임
    int   clipStand1     = -1;
    int   clipStand2     = -1;
    int   clipFall       = -1;
    int   clipLand       = -1;
    int   clipJump       = -1;   // 공중 사망
    int   fallAt1        = 0;    // Stand1 이 이 프레임에 닿으면 쓰러짐(Land/Fall) 으로 전환
    int   fallAt2        = 0;
    float blinkAtSec     = 2.f;   // 최종 클립 시작 후
    float blinkForSec    = 1.f;
    float blinkPeriodSec = 0.1f;
    float removeAtSec    = 3.f;
    slug::Vector<FxDef> fx = {};   // 사망 순간 이펙트 (디브리 등)
};

struct DropDef
{
    slug::String item  = {};
    int         count = 1;
};

struct ActivationDef
{
    float marginPx = 150.f;   // 화면 오른쪽 가장자리 + marginPx 안으로 들어오면 활성
    bool  bLatch   = true;
    bool  bAlways  = false;   // true: 스폰 즉시 활성 (화면 검사 없음)
};

struct PhysicsDef
{
    float gravity       = 1000.f;
    float maxFall       = 3000.f;
    float speed         = 100.f;    // 기본 이동 속도 px/s
    float jumpVy        = -450.f;
    float dropSpeed     = 100.f;    // 원웨이 하강 속도
    float footHalfWidth = 4.f;
    bool  bGravity      = true;     // false: 살아 있는 동안 중력/지면 물리 없음 (매달린 병사, 낙하산)
};

// ===========================================
//  행동별 설정 (JSON "params" + "clips" 에서 로더가 채운다)
//   -1 은 "없음".
// ===========================================

enum class eSoldierAi : uint8_t
{
    Bomb,
    Sword,
    Missile
};

struct SoldierCfg
{
    eSoldierAi ai = eSoldierAi::Bomb;

    // 수치
    int   cooldownTicks   = 60;
    float detect          = 350.f;
    float jumpAttack      = 150.f;    // Sword
    float closeStop       = 200.f;    // Bomb: 이 거리 안이면 멈춰서 던짐
    float swordNear       = 100.f;    // Sword: 제자리 베기 거리
    float dropDy          = 50.f;     // 플레이어가 이만큼 아래면 하강(원웨이 위에서만)
    float jumpDy          = 100.f;    // 플레이어가 이만큼 위면 점프
    int   fireFrame       = 10;       // Throw 클립 안에서 발사 프레임
    int   slashFrame      = 9;
    int   jumpSlashFrame  = 2;
    int   jumpReadyTicks  = 30;       // JumpSlashReady → JumpSlash (0.5 s)
    float retreatBelow    = 0.f;      // > 0: 이 거리보다 가까우면 Move 가 플레이어 반대로 후퇴
    float wallProbeW      = 40.f;
    float wallProbeH      = 50.f;
    float wallProbeX      = 25.f;
    float wallProbeY      = -37.5f;

    // 클립 인덱스 (EnemyDef::clips)
    int clipIdle = -1, clipMove = -1, clipMoveDown = -1, clipFall = -1, clipHJump = -1, clipLand = -1;
    int clipThrow = -1, clipSlash = -1, clipJumpSlashReady = -1, clipJumpSlash = -1, clipJumpSlashLand = -1;

    // 무기 인덱스 (EnemyDef::weapons)
    int weaponAttack = -1, weaponSlash = -1, weaponJumpSlash = -1;
};

struct BzkCfg
{
    int   cooldownTicks = 60;
    float detect        = 400.f;    // 유클리드 거리
    int   bullets       = 2;
    int   readyTicks    = 60;       // Shot_Ready 대기 (1.0 s)
    int   jumpFrame     = 3;        // HJump 클립 안에서 도약 프레임
    int   fireFrame     = 1;
    float angleStep[4]  = { 11.25f, 33.75f, 56.25f, 78.75f };   // 각도 구간 경계 (도, 수평 기준)
    Vec2 muzzleBase = { 0.f, -63.f };
    float muzzleDist    = 50.f;
    float wallProbeW    = 50.f;
    float wallProbeH    = 50.f;
    float wallProbeX    = 30.f;
    float wallProbeY    = -37.5f;

    int clipIdle = -1, clipMove = -1, clipHJump = -1, clipStandUp = -1;
    int clipAimUp = -1, clipAimDown = -1, clipRecoverUp = -1, clipRecoverDown = -1;
    int clipReadyF = -1, clipShotF = -1;
    int clipReadyUp[4]   = { -1, -1, -1, -1 };
    int clipReadyDown[4] = { -1, -1, -1, -1 };
    int clipShotUp[4]    = { -1, -1, -1, -1 };
    int clipShotDown[4]  = { -1, -1, -1, -1 };

    int weaponRocket = -1;
};

struct HangCfg
{
    float detectX     = 500.f;
    int   throwTicks  = 120;     // 2.0 s
    int   fireFrame   = 9;
    int   clipIdle = -1, clipThrow = -1, clipDieJump = -1, clipDieLand = -1;
    int   weaponBomb  = -1;
};

struct FlyingBzkCfg
{
    float descendSpeed  = 40.f;
    float driftSpeed    = 20.f;
    int   driftFlipTicks = 48;      // 0.8 s
    int   flipPercent   = 50;
    int   fireIntervalTicks = 96;   // 1.6 s
    int   maxShots      = 3;
    int   fireFrame     = 1;
    float postureDeg[5]  = { 0.f, 25.f, 50.f, 75.f, 90.f };   // 자세 중심 각도 (수평에서 아래로)
    float windowDeg      = 10.f;
    float fireDeg[5]     = { 0.f, 22.5f, 45.f, 67.5f, 90.f };
    Vec2 muzzleBase = { 0.f, -63.f };
    float muzzleDist     = 50.f;
    float landProbe      = 1.f;     // 발 아래 이 거리 안에 지면이 있으면 착지로 본다
    slug::String landedDef = {};     // 착지 시 변신할 def 이름

    int clipReady[5] = { -1, -1, -1, -1, -1 };
    int clipShot[5]  = { -1, -1, -1, -1, -1 };
    int clipDieJump = -1, clipDieLand = -1;
    int weaponRocket = -1;

    // 낙하산 (몸 위에 겹쳐 그리는 오버레이) / 파괴 이펙트
    int          clipPara = -1;
    Vec2 paraOffset = { -5.f, -87.5f };
    FxDef        paraDestroy = {};
};

struct ShielderCfg
{
    int   cooldownTicks = 60;
    float detect        = 250.f;
    float guard         = 50.f;     // 이 가로 거리 안이면 위/아래를 본다
    float levelDy       = 50.f;     // |dy| 가 이 이하면 수평
    float upDy          = 100.f;    // dy < -upDy 면 위쪽
    float moveDy        = 100.f;    // Move 중 사격 전환 |dy| 한계
    int   bullets       = 3;
    int   startTicks    = 30;       // Shot_Start 대기 (0.5 s)
    int   readyTicks    = 20;       // Shot_Ready 대기 (0.333 s)
    int   moveMaxTicks  = 120;      // Move 최대 지속 (2.0 s)
    int   fireFrame     = 1;
    float edgeProbeDist = 24.f;     // 발 앞쪽 낭떠러지 검사 거리
    float edgeProbeDrop = 12.f;     // 이만큼 아래까지 바닥이 없으면 낭떠러지
    float wallProbeW    = 40.f;
    float wallProbeH    = 50.f;
    float wallProbeX    = 25.f;
    float wallProbeY    = -37.5f;
    int   defenceTicks  = 24;       // 방어 연출 (0.4 s)

    // 방패 판정 상자 (발 기준, 오른쪽 기준 x 미러). 앞/위/아래
    BoxDef shieldFront = {};
    BoxDef shieldUp    = {};
    BoxDef shieldDown  = {};

    int clipIdle = -1, clipMove = -1, clipMoveStop = -1;
    int clipShotStart = -1, clipShotReady = -1, clipShot = -1, clipShotEnd = -1;
    int clipShotStartUp = -1, clipShotReadyUp = -1, clipShotUp = -1, clipShotEndUp = -1;
    int clipShotStartDown = -1, clipShotReadyDown = -1, clipShotDown = -1, clipShotEndDown = -1;
    int clipLookUpStart = -1, clipLookUpIdle = -1, clipLookUpEnd = -1;
    int clipLookDownStart = -1, clipLookDownIdle = -1, clipLookDownEnd = -1;
    int clipDefence = -1, clipDefenceUp = -1, clipDefenceDown = -1;
    int clipFall = -1;

    int weaponForward = -1, weaponUp = -1, weaponDown = -1;
};

struct PowCfg
{
    float walkSpeed      = 75.f;
    int   firstFlipTicks = 120;     // 첫 방향 반전 간격 (2 s)
    int   flipMinTicks   = 60;      // 이후 1 + rand(0..flipRange) s
    int   flipRangeTicks = 120;
    float glideMaxFall   = 150.f;
    float dropDy         = 100.f;   // 플레이어가 이만큼 아래면 하강
    int   dropMinPatrolTicks = 60;
    int   itemFrame      = 8;
    float closeNudge     = 16.f;
    float itemOffsetX    = 18.f;
    float runSpeed       = 200.f;
    int   blinkAtTicks   = 168;     // 2.8 s
    int   blinkForTicks  = 120;
    int   blinkPeriodTicks = 12;
    int   removeAtTicks  = 240;     // 4 s
    float offscreenMargin = 50.f;
    float edgeProbeDist  = 20.f;
    float edgeProbeDrop  = 12.f;

    int clipStuck = -1, clipEscape = -1, clipMove = -1, clipGlide = -1, clipMoveDown = -1, clipItem = -1, clipRun = -1;
};

// ===========================================
//  차량 / 전차 / 헬기 (Docs/EnemySpecs.md §2). 수치는 Vehicles.json "params", 클립/무기는 "clips"/"weapons" 에서 로더가 채운다.
// ===========================================

// 이동 로켓 지프 (2.1). 원점 = 발 (지면 추종). 클립 접미 = 주행 방향(_Left: 좌주행 / 우발사).
struct JeepCfg
{
    float speed        = 200.f;   // 주행 px/s
    float targetTol    = 10.f;    // 목표 x 와 이 거리 안이면 도착
    float edgeInset    = 75.f;    // 화면 가장자리에서 떨어진 목표 x
    int   idleTicks    = 60;      // Idle / IdleAndMove 대기 (1 s)
    int   waitTicks    = 120;     // 사격 쌍 사이 휴지 (2 s)
    int   bullets      = 4;       // 한 번 조준당 로켓 수
    int   fireFrame    = 1;       // Shot 클립 안 발사 프레임

    int clipIdle = -1, clipMove = -1, clipTurn = -1, clipAimming = -1, clipReady = -1, clipShotReady = -1, clipShot = -1, clipReAim = -1, clipDestroy = -1;
    int weaponRocket = -1;
};

// 고정 포탑 전차 (2.3). 원점 = 발. 항상 좌향. 활성 = 플레이어 x >= 전차 x - activateDx.
struct BTankCfg
{
    float activateDx    = 500.f;
    int   idleTicks     = 180;     // 열린 뒤 발사 대기 (3 s)
    int   shotsPerCycle = 2;       // 한 번에 연속 발사 수
    int   fireFrame     = 2;       // Fire 클립 안 발사 프레임

    int clipIdle = -1, clipOpen = -1, clipFire = -1, clipDestroy = -1;
    int weaponShell = -1;
};

// 미사일 전차 (2.4). 원점 = 발. 조준 모드 Up(곡사)/Down(직사). 활성 = |dx| <= rangeDx 동안 타이머 누적.
struct MissleTankCfg
{
    float rangeDx      = 600.f;   // 이 거리 안에서만 행동 타이머가 간다
    float farDx        = 300.f;   // |dx| > farDx 이면 곡사(Up), 아니면 직사(Down)
    int   detectTicks  = 60;      // Idle 누적 (1 s)
    int   readyTicks   = 120;     // 발사 준비 (2 s)
    int   bullets      = 5;
    int   fireFrame    = 2;

    int clipIdleUp = -1, clipIdleDown = -1, clipAimUp = -1, clipAimDown = -1;
    int clipReadyUp = -1, clipReadyDown = -1, clipShotUp = -1, clipShotDown = -1;
    int clipDestroyUp = -1, clipDestroyDown = -1;
    int weaponUp = -1, weaponDown = -1;
};

// 보행 전차 / 중간보스 게이트 (2.5). 원점 = 발. 롤링 수류탄(사루비아 탄)을 쏜다.
struct SarubiaCfg
{
    int   cooldownTicks = 120;    // Idle 2 s
    float chaseDx       = 500.f;  // |dx| 가 이보다 크면 전진 (Move)
    float stopDx        = 400.f;  // 전진 중 |dx| 가 이 값 이하가 되면 멈춘다
    int   moveMaxTicks  = 120;    // 전진 최대 2 s
    int   shotReadyTicks = 90;    // 사격 준비 1.5 s
    int   fireFrame     = 1;

    int clipIdle = -1, clipTurn = -1, clipMoveStart = -1, clipMove = -1, clipMoveStop = -1;
    int clipShotStart = -1, clipShot = -1, clipDestroy = -1;
    int weaponGrenade = -1;
};

// 공격 헬기 (2.6). 원점 = 기체 중심 (비행: 지면 없음). 항상 좌향.
struct AparchCfg
{
    float speed        = 300.f;   // 접근 속도
    float decelSq      = 10000.f; // 목표까지 거리² 이하면 감속 (v = max(minSpeed, v - decel*dt))
    float arriveSq     = 400.f;   // 이 거리² 이하면 정지
    float decel        = 600.f;
    float minSpeed     = 50.f;
    float approachX    = 445.f;   // 화면 좌표 (진입 지점)
    float approachY    = 125.f;
    float missileX     = 540.f;   // 화면 좌표 (미사일 자세)
    float missileY     = 248.f;
    float homeX        = 540.f;   // 화면 좌표 (복귀 지점)
    float homeY        = 100.f;
    int   idleTicks    = 120;     // 패턴 사이 2 s
    int   gunTicks     = 60;      // 포 전개 1 s
    int   moveTicks    = 60;      // 자세 이동 1 s
    int   missileIntervalTicks = 60;   // 미사일 간격 1 s (3발)
    int   missileShots = 3;
    int   endTicks     = 60;      // 포 수납 + 복귀 1 s
    float followOffsetY = 275.f;  // 폭탄 패턴: 플레이어 위 이 높이를 따라다닌다
    float followGain   = 1.5f;
    int   bombStepTicks = 30;     // 0.5 s 틱 (폭탄 패턴 리듬)
    int   bombSteps    = 12;      // 6 s
    float driftX       = 50.f;    // 사망 표류 (px/s)
    float driftY       = -70.f;
    int   chainCount   = 6;       // 사망 연쇄 폭발 횟수
    int   chainIntervalTicks = 18;   // 0.3 s
    int   removeTicks  = 90;      // 사망 후 1.5 s 에 제거

    int clipIdle = -1, clipMove = -1, clipMoveStop = -1, clipDestroy = -1;
    int weaponMissile = -1, weaponDrop = -1;
};

// 버스 중간보스 (Docs/EnemySpecs.md 3.1). 좌표는 "원점" 기준: 원점 = 발 - (0, originDepth). 방향은 항상 +1 (레거시 좌표 그대로).
struct BusCfg
{
    float    originDepth        = 123.f;
    BoxDef   trigger            = {};        // 플레이어 몸이 닿으면 전투 시작 (문 개방)
    int      openTicks          = 210;       // MainTruck_Begin 길이 (3.5 s)
    int      shellFirstTicks    = 120;       // 첫 포탄 (전투 시작 후 2 s)
    int      shellIntervalTicks = 180;       // 이후 3 s 간격
    int      soldierFirstTicks  = 180;       // 첫 병사 투하 (3 s)
    int      soldierIntervalTicks = 300;     // 이후 5 s 간격
    int      soldierAliveMax    = 2;         // 동시 생존 투하 병사
    int      soldierTotalMax    = 8;         // 누적 투하 병사
    Vec2 soldierOffset  = { -150.f, -40.f };   // 원점 기준 투하 위치
    slug::String  soldierDef     = {};        // 투하 병사 정의 (soldier_bomb)
    int      weaponShell        = -1;        // 포탄 무기 (Given 이 아닌 Facing + arc)
};

// 최종보스 (Docs/EnemySpecs.md 3.2). 좌표는 "본체 원점" 기준. 원점 = 발 - (0, originDepth).
struct BattleShipCfg
{
    float    originDepth      = 132.f;
    float    entranceOffsetX  = -300.f;     // 등장: arena.left + 이 값 에서 레일로 굴러 들어온다
    float    railSlot[3]      = { 30.f, 100.f, 190.f };   // arena.left 기준 (back, home, forward)
    int      homeSlot         = 1;
    float    railMaxSpeed     = 100.f;
    float    railAccel        = 50.f;
    float    railDecel        = 100.f;
    int      idleTicks        = 96;          // 1.6 s (격앙 0.8 s)
    int      idleEnragedTicks = 48;
    int      headOpenTicks    = 86;          // Head_Swiching_Start 1.44 s
    int      cannonStartTicks = 96;          // Cannon_Start 1.6 s
    int      cannonIdleTicks  = 60;          // 1.0 s (격앙 0.6 s)
    int      cannonIdleEnragedTicks = 36;
    float    cannonShotSec    = 0.56f;       // Cannon_Shot_{0..4} 한 번 (7 f x 0.08)
    int      cannonShots      = 3;           // 격앙 5
    int      cannonShotsEnraged = 5;
    int      cannonEndTicks   = 67;          // Cannon_End 1.12 s
    int      headCloseTicks   = 101;         // Head_Swiching_End 1.68 s
    float    cannonTrackDeg   = 60.f;        // 초당 (격앙 90)
    float    cannonTrackDegEnraged = 90.f;
    float    cannonAngleStart = -60.f;
    float    cannonAngleMin   = -135.f;
    float    cannonAngleMax   = -45.f;
    Vec2 cannonOffset = { 123.f, -149.f };   // 원점 기준 포 중심
    float    bulletSpeed      = 300.f;
    float    bulletSpread     = 37.f;        // 발사 위치 (idx - 2) * spread
    slug::String bulletProjectile = {};       // 투사체 정의 이름 (battleship_bullet). 발사 때 라이브러리에서 찾는다
    BoxDef   bow              = {};          // 피격 상자 (원점 기준)
    BoxDef   bridge           = {};
    BoxDef   aft              = {};
    float    hullBlockDx      = 270.f;       // 파괴 전: 플레이어 x 는 선체 x + 이 값 이상
    // 포탑 (2기)
    Vec2 turretOffset[2] = { { -200.f, -70.f }, { -100.f, -120.f } };
    BoxDef   turretBox        = {};          // 포탑 원점 기준 피격 상자
    Vec2 turretMuzzle[3] = { { -30.f, -30.f }, { 0.f, -35.f }, { 30.f, -30.f } };   // LEFT / MID / RIGHT
    int      turretHp         = 40;
    float    turretCoolSec    = 2.4f;        // 격앙 1.4 s
    float    turretCoolEnragedSec = 1.4f;
    float    turretFirstStepSec = 1.2f;      // i * 이 값 = 첫 발 지연
    float    turretTurnSec    = 0.24f;
    float    turretShotSec    = 1.12f;       // DCannon_Shot_* (14 f x 0.08)
    float    turretMidDx      = 60.f;
    float    turretAimJitter  = 30.f;        // 플레이어 x ± 이 값 목표
    float    turretRandomSec  = 0.5f;       // 발사 후 추가 랜덤 [0, 이 값)
    float    turretArcMinVx   = 40.f;
    float    turretArcMaxVx   = 450.f;
    slug::String turretProjectile = {};       // 포탑 포탄 정의 이름 (btank_shell)
    // 격앙 / 파괴
    int      enragedSmallTicks = 60;         // 격앙 중 선체 소폭발 주기 (1 s)
    float    destroyChainSec  = 2.8f;
    float    destroyChainStepSec = 0.12f;
    float    destroyFinishSec = 3.6f;
};

// ===========================================
//  EnemyDef
// ===========================================

struct EnemyDef
{
    slug::String    name     = {};
    eEnemyBehavior behavior = eEnemyBehavior::Soldier;
    eTeam          team     = eTeam::Enemy;
    int            hp       = 1;
    int            score    = 0;

    BoxDef        body      = {};   // 이동/지형 충돌 박스
    BoxDef        hurtbox   = {};   // 피격 박스 (기본: body)
    ActivationDef activation = {};
    PhysicsDef    physics   = {};

    slug::Vector<NamedClip> clips   = {};
    slug::Vector<WeaponDef> weapons = {};
    DeathDef               death   = {};
    slug::Vector<DropDef>   drops   = {};

    // 몸 위에 겹쳐 그리는 오버레이 클립 (선택): 낙하산 등. clips 인덱스, -1 = 없음
    int          overlayClip   = -1;
    Vec2 overlayOffset = { 0.f, 0.f };

    // 행동별 설정 (behavior 에 해당하는 것만 의미가 있다)
    SoldierCfg   soldier   = {};
    BzkCfg       bzk       = {};
    HangCfg      hang      = {};
    FlyingBzkCfg flyingBzk = {};
    ShielderCfg  shielder  = {};
    PowCfg       pow       = {};
    BusCfg         bus         = {};
    BattleShipCfg  battleShip  = {};
    JeepCfg        jeep        = {};
    BTankCfg       btank       = {};
    MissleTankCfg  missleTank  = {};
    SarubiaCfg     sarubia     = {};
    AparchCfg      aparch      = {};

    // true: 카메라 뒤로 밀려나도 정리하지 않는다 (보스 아레나 고정)
    bool bNoDespawn = false;

    // true: 사람 (칼 대상). 차량/보스는 false 라서 칼을 받지 않는다
    bool bHuman = false;

    [[nodiscard]] const ClipDef& GetClip(const int _index) const { return clips[static_cast<size_t>(_index)].clip; }
    [[nodiscard]] int            FindClip(slug::StringView _logicalName) const;
    [[nodiscard]] int            FindWeapon(slug::StringView _name) const;
};

struct EnemyGlobals
{
    float dpsRef                = 10.f;
    float activationMarginPx    = 150.f;
    float despawnBehindCameraPx = 200.f;
    float despawnBelowCameraPx  = 300.f;
};

// ===========================================
//  EnemyDefLibrary
//   Assets/Defs/Enemies.json 를 읽어 보관한다. 로드 후에는 읽기 전용이다.
//   정의는 로드 순서대로 인덱스가 매겨지며(안정), 이름으로 찾을 수 있다.
// ===========================================

class EnemyDefLibrary
{
public:
    // 오류 시 사유(정의 이름/필드 포함)를 _pMessage 에 채우고 로그로도 남긴다.
    [[nodiscard]] static slug::Result<EnemyDefLibrary> Parse(slug::StringView _json, slug::String* _pMessage = nullptr);
    [[nodiscard]] static slug::Result<EnemyDefLibrary> Load(const slug::FilePath& _path, slug::String* _pMessage = nullptr);

    // 폴더 안의 모든 *.json 을 이름순으로 합쳐서 로드한다 (Enemies.json, Vehicles.json, Props.json, Boss.json ...).
    // 정의 이름은 파일 간에도 유일해야 하고, base 상속은 파일 경계를 넘을 수 있다.
    [[nodiscard]] static slug::Result<EnemyDefLibrary> LoadDirectory(const slug::FilePath& _dir, slug::String* _pMessage = nullptr);

    // <assetRoot>/Defs/Enemies.json
    [[nodiscard]] static slug::FilePath GetDefaultPath(const slug::FilePath& _assetRoot);

    // <assetRoot>/Defs
    [[nodiscard]] static slug::FilePath GetDefaultDirectory(const slug::FilePath& _assetRoot);

    [[nodiscard]] const EnemyDef* Find(slug::StringView _name) const;
    [[nodiscard]] int             FindIndex(slug::StringView _name) const;   // 없으면 -1
    [[nodiscard]] const EnemyDef& Get(const int _index) const { return m_defs[static_cast<size_t>(_index)]; }

    [[nodiscard]] const ProjectileDef* FindProjectile(slug::StringView _name) const;
    [[nodiscard]] int                  FindProjectileIndex(slug::StringView _name) const;
    [[nodiscard]] const ProjectileDef& GetProjectile(const int _index) const { return m_projectiles[static_cast<size_t>(_index)]; }

    [[nodiscard]] size_t                         GetDefCount() const { return m_defs.size(); }
    [[nodiscard]] size_t                         GetProjectileCount() const { return m_projectiles.size(); }
    [[nodiscard]] slug::Span<const EnemyDef>      GetDefs() const { return m_defs; }
    [[nodiscard]] slug::Span<const ProjectileDef> GetProjectiles() const { return m_projectiles; }
    [[nodiscard]] const EnemyGlobals&            GetGlobals() const { return m_globals; }

private:
    friend class EnemyDefParser;

    slug::Vector<EnemyDef>               m_defs        = {};
    slug::Vector<ProjectileDef>          m_projectiles = {};
    slug::HashMap<slug::String, int>      m_defIndex    = {};
    slug::HashMap<slug::String, int>      m_projIndex   = {};
    EnemyGlobals                        m_globals     = {};
};

}   // namespace slug
