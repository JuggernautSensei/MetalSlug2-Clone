#pragma once
#include <cstdint>
#include <optional>

#include "Base/Math.h"
#include "Base/SlugError.h"
#include "Base/Types.h"
#include "Game/Combat/HitFlash.h"
#include "Game/Enemies/BigShieeCfg.h"
#include "World/Combat.h"
#include "World/Geometry.h"

enum class eEnemyBehavior : uint8_t
{
    Soldier,
    Bzk,
    Hang,
    FlyingBzk,
    Shielder,
    Pow,
    Jeep,
    BTank,
    MissleTank,
    Sarubia,
    Bus,
    Heli,
    HeliBomb,
    BigShiee,
    Count
};

[[nodiscard]] const char*                   GetBehaviorName(eEnemyBehavior _behavior);
[[nodiscard]] std::optional<eEnemyBehavior> FindBehavior(StringView _name);

inline constexpr int64_t kAnimUnitsPerTick = 100'000;

[[nodiscard]] int SecToTicks(float _sec);

struct BoxDef
{
    float w  = 0.f;
    float h  = 0.f;
    float cx = 0.f;
    float cy = 0.f;

    [[nodiscard]] Aabb At(
        const Vec2 _feet,
        const int  _facing) const
    {
        const float cxWorld = _feet.x + cx * static_cast<float>(_facing < 0 ? -1 : 1);
        return Aabb::FromCenterSize({ cxWorld, _feet.y + cy }, { w, h });
    }

    [[nodiscard]] bool IsEmpty() const
    {
        return w <= 0.f || h <= 0.f;
    }
};

struct ClipDef
{
    String  name       = {};
    int     frames     = 1;
    float   frameSec   = 0.1f;
    bool    bLoop      = false;
    int64_t frameUnits = 0;
    bool    bSided     = false;

    [[nodiscard]] bool IsValid() const
    {
        return !name.empty();
    }

    [[nodiscard]] int TickOfFrame(int _frame) const;

    [[nodiscard]] int GetDurationTicks() const;

    [[nodiscard]] bool IsFinished(
        const int _ticks) const
    {
        return !bLoop && _ticks >= GetDurationTicks();
    }
};

struct NamedClip
{
    String  name = {};
    ClipDef clip = {};
};

struct FxDef
{
    String clip     = {};
    Vec2   offset   = { 0.f, 0.f };
    float  delaySec = 0.f;
    int    depth    = 60;
    bool   bDebris  = false;
    bool   bMirror  = false;
};

struct ProjectileDef
{
    String  name            = {};
    Vec2    size            = { 20.f, 20.f };
    int     damage          = 1;
    float   speed           = 0.f;
    bool    bRamp           = false;
    float   rampFrom        = 0.f;
    float   rampAccel       = 0.f;
    float   rampMax         = 0.f;
    Vec2    v0              = { 0.f, 0.f };
    float   gravity         = 0.f;
    float   maxFall         = 3000.f;
    float   lifetimeSec     = 5.f;
    float   startDelaySec   = 0.f;
    bool    bExplodeOnSolid = true;
    ClipDef begin           = {};
    ClipDef fly             = {};
    FxDef   explosion       = {};
    String  explosionSfx    = {};   // 착탄 폭발 효과음 이름 (비면 없음)
    FxDef   trail           = {};   // 비행 중 주기적으로 남기는 꼬리 연출 (clip 이 비면 없음)
    int     trailTicks      = 0;    // 꼬리 생성 간격 (틱, 0 = 없음)
    float   rollSpeed       = 0.f;
    float   rollSec         = 0.f;
    bool    bExpireFx       = false;
    float   groundProbeY    = 0.f;
    ClipDef land            = {};
    Vec2    blastSize       = { 0.f, 0.f };   // 착탄/만료로 터질 때 남기는 폭발 판정 상자 (0 = 없음). 중심은 터진 점에서 blastUp 만큼 위
    float   blastUp         = 0.f;
    float   blastSec        = 0.f;
    int     blastDamage     = 1;
    float   scale           = 1.f;
    bool    bRotate         = false;
};

enum class eWeaponAim : uint8_t
{
    Facing,
    Given
};

struct WeaponEffect
{
    String clip   = {};
    Vec2   offset = { 0.f, 0.f };
    int    frame  = 0;
    int    depth  = 60;
};

struct WeaponDef
{
    String               name           = {};
    int                  projectile     = -1;
    Vec2                 muzzle         = { 0.f, 0.f };
    eWeaponAim           aim            = eWeaponAim::Facing;
    float                tiltThreshold  = 0.f;
    float                tiltDy         = 0.f;
    bool                 bArc           = false;
    float                arcMinVx       = 0.f;
    float                arcMaxVx       = 0.f;
    FxDef                muzzleFx       = {};
    String               motionClip     = {};
    Vector<WeaponEffect> effects        = {};
    bool                 bHitbox        = false;
    BoxDef               hitbox         = {};
    float                hitboxLifeSec  = 0.f;
    int                  hitboxDamage   = 1;
    bool                 bHitboxFollows = false;
};

struct DeathDef
{
    int           clipBomb       = -1;
    int           clipSlash      = -1;
    int           clipStand1     = -1;
    int           clipStand2     = -1;
    int           clipFall       = -1;
    int           clipLand       = -1;
    int           clipJump       = -1;
    int           fallAt1        = 0;
    int           fallAt2        = 0;
    float         blinkAtSec     = 2.f;
    float         blinkForSec    = 1.f;
    float         blinkPeriodSec = 0.1f;
    float         removeAtSec    = 3.f;
    float         bloodOffsetY   = -27.5f;
    Vec2          bombVelocity   = { 110.f, -430.f };   // 폭발 피격 날아감 초기 속도 px/s (x 는 플레이어 반대 방향 부호가 곱해진다)
    Vector<FxDef> fx             = {};
};

struct DropDef
{
    String item  = {};
    int    count = 1;
};

struct ActivationDef
{
    float marginPx = 150.f;
    bool  bLatch   = true;
    bool  bAlways  = false;
};

struct PhysicsDef
{
    float gravity       = 1000.f;
    float maxFall       = 3000.f;
    float speed         = 100.f;   // 기본 이동 속도 px/s
    float jumpVy        = -450.f;
    float dropSpeed     = 100.f;
    float footHalfWidth = 4.f;
    bool  bGravity      = true;
};

enum class eSoldierAi : uint8_t
{
    Bomb,
    Sword,
    Missile
};

struct SoldierCfg
{
    eSoldierAi ai = eSoldierAi::Bomb;

    int   cooldownTicks  = 60;
    float detect         = 350.f;
    float jumpAttack     = 150.f;
    float closeStop      = 200.f;
    float swordNear      = 100.f;
    float dropDy         = 50.f;
    float jumpDy         = 100.f;
    int   fireFrame      = 10;
    int   slashFrame     = 9;
    int   jumpSlashFrame = 2;
    int   jumpReadyTicks = 30;    // JumpSlashReady → JumpSlash (0.5 s)
    float retreatBelow   = 0.f;   // > 0: 이 거리보다 가까우면 Move 가 플레이어 반대로 후퇴
    float wallProbeW     = 40.f;
    float wallProbeH     = 50.f;
    float wallProbeX     = 25.f;
    float wallProbeY     = -37.5f;

    int clipIdle           = -1;
    int clipMove           = -1;
    int clipMoveDown       = -1;
    int clipFall           = -1;
    int clipHJump          = -1;
    int clipLand           = -1;
    int clipThrow          = -1;
    int clipSlash          = -1;
    int clipJumpSlashReady = -1;
    int clipJumpSlash      = -1;
    int clipJumpSlashLand  = -1;

    int weaponAttack    = -1;
    int weaponSlash     = -1;
    int weaponJumpSlash = -1;
};

struct BzkCfg
{
    int             cooldownTicks = 60;
    float           detect        = 400.f;
    int             bullets       = 2;
    int             readyTicks    = 60;   // Shot_Ready 대기 (1.0 s)
    int             jumpFrame     = 3;
    int             fireFrame     = 1;
    ARRAY<float, 4> angleStep     = { 11.25f, 33.75f, 56.25f, 78.75f };   // 각도 구간 경계 (도, 수평 기준)
    Vec2            muzzleBase    = { 0.f, -63.f };
    float           muzzleDist    = 50.f;
    float           wallProbeW    = 50.f;
    float           wallProbeH    = 50.f;
    float           wallProbeX    = 30.f;
    float           wallProbeY    = -37.5f;

    int           clipIdle        = -1;
    int           clipMove        = -1;
    int           clipHJump       = -1;
    int           clipStandUp     = -1;
    int           clipAimUp       = -1;
    int           clipAimDown     = -1;
    int           clipRecoverUp   = -1;
    int           clipRecoverDown = -1;
    int           clipReadyF      = -1;
    int           clipShotF       = -1;
    ARRAY<int, 4> clipReadyUp     = { -1, -1, -1, -1 };
    ARRAY<int, 4> clipReadyDown   = { -1, -1, -1, -1 };
    ARRAY<int, 4> clipShotUp      = { -1, -1, -1, -1 };
    ARRAY<int, 4> clipShotDown    = { -1, -1, -1, -1 };

    int weaponRocket = -1;
};

struct HangCfg
{
    float detectX    = 500.f;
    int   throwTicks = 120;   // 2.0 s
    int   fireFrame  = 9;
    int   clipIdle   = -1;
    int   clipThrow  = -1;
    int   weaponBomb = -1;
};

struct FlyingBzkCfg
{
    float           descendSpeed      = 40.f;
    float           driftSpeed        = 20.f;
    int             driftFlipTicks    = 48;   // 0.8 s
    int             flipPercent       = 50;
    int             fireIntervalTicks = 96;   // 1.6 s
    int             maxShots          = 3;
    int             fireFrame         = 1;
    ARRAY<float, 5> postureDeg        = { 0.f, 25.f, 50.f, 75.f, 90.f };
    float           windowDeg         = 10.f;
    ARRAY<float, 5> fireDeg           = { 0.f, 22.5f, 45.f, 67.5f, 90.f };
    Vec2            muzzleBase        = { 0.f, -63.f };
    float           muzzleDist        = 50.f;
    float           landProbe         = 1.f;
    String          landedDef         = {};
    float           fallGravity       = 0.f;     // px/s^2. 0 이면 descendSpeed 등속 낙하, 양수면 descendSpeed 에서 시작해 가속
    float           fallMaxSpeed      = 0.f;     // 가속 낙하의 최대 속도 px/s (fallGravity > 0 일 때)
    bool            bNoHurtbox        = false;

    ARRAY<int, 5> clipReady    = { -1, -1, -1, -1, -1 };
    ARRAY<int, 5> clipShot     = { -1, -1, -1, -1, -1 };
    int           weaponRocket = -1;

    Vec2  paraOffset  = { -5.f, -87.5f };
    FxDef paraDestroy = {};
};

struct ShielderCfg
{
    int   cooldownTicks = 60;
    float detect        = 250.f;
    float guard         = 50.f;
    float levelDy       = 50.f;
    float upDy          = 100.f;
    float moveDy        = 100.f;
    int   bullets       = 3;
    int   startTicks    = 30;    // Shot_Start 대기 (0.5 s)
    int   readyTicks    = 20;    // Shot_Ready 대기 (0.333 s)
    int   moveMaxTicks  = 120;   // Move 최대 지속 (2.0 s)
    int   fireFrame     = 1;
    float edgeProbeDist = 24.f;
    float edgeProbeDrop = 12.f;
    float wallProbeW    = 40.f;
    float wallProbeH    = 50.f;
    float wallProbeX    = 25.f;
    float wallProbeY    = -37.5f;
    int   defenceTicks  = 24;   // 방어 연출 (0.4 s)

    BoxDef shieldFront = {};
    BoxDef shieldUp    = {};
    BoxDef shieldDown  = {};

    int clipIdle          = -1;
    int clipMove          = -1;
    int clipMoveStop      = -1;
    int clipShotStart     = -1;
    int clipShotReady     = -1;
    int clipShot          = -1;
    int clipShotEnd       = -1;
    int clipShotStartUp   = -1;
    int clipShotReadyUp   = -1;
    int clipShotUp        = -1;
    int clipShotEndUp     = -1;
    int clipShotStartDown = -1;
    int clipShotReadyDown = -1;
    int clipShotDown      = -1;
    int clipShotEndDown   = -1;
    int clipLookUpStart   = -1;
    int clipLookUpIdle    = -1;
    int clipLookUpEnd     = -1;
    int clipLookDownStart = -1;
    int clipLookDownIdle  = -1;
    int clipLookDownEnd   = -1;
    int clipDefence       = -1;
    int clipDefenceUp     = -1;
    int clipDefenceDown   = -1;
    int clipFall          = -1;

    int weaponForward = -1;
    int weaponUp      = -1;
    int weaponDown    = -1;
};

struct PowCfg
{
    float walkSpeed          = 75.f;
    int   firstFlipTicks     = 120;   // 첫 방향 반전 간격 (2 s)
    int   flipMinTicks       = 60;
    int   flipRangeTicks     = 120;
    float glideMaxFall       = 150.f;
    float dropDy             = 100.f;
    int   dropMinPatrolTicks = 60;
    int   itemFrame          = 8;
    float closeNudge         = 16.f;
    float itemOffsetX        = 18.f;
    float runSpeed           = 200.f;
    int   blinkAtTicks       = 168;   // 2.8 s
    int   blinkForTicks      = 120;
    int   blinkPeriodTicks   = 12;
    int   removeAtTicks      = 240;   // 4 s
    float offscreenMargin    = 50.f;
    float edgeProbeDist      = 20.f;
    float edgeProbeDrop      = 12.f;

    int clipStuck    = -1;
    int clipEscape   = -1;
    int clipMove     = -1;
    int clipGlide    = -1;
    int clipMoveDown = -1;
    int clipItem     = -1;
    int clipRun      = -1;

    // 외형 변형 (SpawnParams.aiVariant 1..3): 슬롯 순서 Stuck, Escape, Move, Glide, MoveDown, Item, Run. 없는 슬롯은 기본 클립을 쓴다
    static constexpr int                         kVariantCount = 3;
    static constexpr int                         kSlotCount    = 7;
    ARRAY<ARRAY<int, kSlotCount>, kVariantCount> clipVariants  = {};
};

struct JeepCfg
{
    float speed          = 200.f;   // 주행 px/s
    float targetTol      = 10.f;
    float edgeInset      = 75.f;
    int   idleTicks      = 60;      // Idle / IdleAndMove 대기 (1 s)
    int   waitTicks      = 120;     // 사격 쌍 사이 휴지 (2 s)
    int   bullets        = 4;
    int   fireFrame      = 1;
    float rocketShiftX   = 20.f;
    int   firstWaitTicks = 0;       // 조준 후 첫 발 전 대기 (0 = 없음)

    int clipIdle      = -1;
    int clipMove      = -1;
    int clipTurn      = -1;
    int clipAimming   = -1;
    int clipReady     = -1;
    int clipShotReady = -1;
    int clipShot      = -1;
    int clipReAim     = -1;
    int clipDestroy   = -1;
    int weaponRocket  = -1;
};

struct BTankCfg
{
    float activateDx    = 500.f;
    int   idleTicks     = 180;   // 열린 뒤 발사 대기 (3 s)
    int   shotsPerCycle = 2;
    int   fireFrame     = 2;

    int clipIdle    = -1;
    int clipOpen    = -1;
    int clipFire    = -1;
    int clipDestroy = -1;
    int weaponShell = -1;
};

struct MissleTankCfg
{
    float rangeDx     = 600.f;
    float farDx       = 300.f;
    int   detectTicks = 60;      // Idle 누적 (1 s)
    int   readyTicks  = 120;     // 발사 준비 (2 s)
    int   bullets     = 5;
    int   fireFrame   = 2;

    int clipIdleUp      = -1;
    int clipIdleDown    = -1;
    int clipAimUp       = -1;
    int clipAimDown     = -1;
    int clipReadyUp     = -1;
    int clipReadyDown   = -1;
    int clipShotUp      = -1;
    int clipShotDown    = -1;
    int clipDestroyUp   = -1;
    int clipDestroyDown = -1;
    int clipAppear      = -1;   // 선택: 스폰 직후 땅에서 올라오는 연출 (kIdle 첫 1 s 동안만 재생, 타이밍 불변)
    int weaponUp        = -1;
    int weaponDown      = -1;
};

struct SarubiaCfg
{
    int   cooldownTicks  = 120;     // Idle 2 s
    float chaseDx        = 500.f;
    float stopDx         = 400.f;
    int   moveMaxTicks   = 120;     // 전진 최대 2 s
    int   shotReadyTicks = 90;      // 사격 준비 1.5 s
    int   fireFrame      = 1;

    int clipIdle      = -1;
    int clipTurn      = -1;
    int clipMoveStart = -1;
    int clipMove      = -1;
    int clipMoveStop  = -1;
    int clipShotStart = -1;
    int clipShot      = -1;
    int clipDestroy   = -1;
    int weaponGrenade = -1;
};

// 버스 폭발 한 개: 단계 진입 (또는 사망) 때 버스 로컬 좌표 (아케이드 px) 에 터진다
struct BusBurstDef
{
    int    stage    = 0;
    String clip     = {};
    Vec2   at       = { 0.f, 0.f };
    float  delaySec = 0.f;
};

// 사망 때 튀는 파편. at = 버스 로컬 좌표, velocity = 게임 px/s
struct BusDebrisDef
{
    String clip     = {};
    Vec2   at       = { 0.f, 0.f };
    Vec2   velocity = { 0.f, 0.f };
    float  fps      = 12.f;
};

// 창 안 병사 (버스 로컬 좌표의 발 위치). stage 이상이면 보인다
struct BusInnerSoldierDef
{
    int  stage = 0;
    Vec2 at    = { 0.f, 0.f };
    int  phase = 0;
};

// 창 파편 클립: stage 이상이면 한 번 재생한다
struct BusWindowDef
{
    String clip  = {};
    int    stage = 0;
};

// 더블데커 버스 중간보스 (Docs/Spec_Bus.md). 시각 연출은 baked 클립 (Bus_*) 을 코드가 직접 고른다. 여기에는 시간/체력 단계/좌표 값을 둔다.
// 좌표 오프셋은 전부 앵커(바퀴 바닥 중앙) 기준, +x 가 월드 오른쪽 (버스는 왼쪽을 향한다), y 는 발 기준 (위가 음수).
struct BusCfg
{
    static constexpr int kStageCount = 7;

    float pxPerSheetPx   = 2.f;
    Vec2  anchor         = { 52.f, 130.f };
    float debrisGroundDy = -8.f;
    int   fxDepth        = 60;

    Vector<BusBurstDef> bursts = {
        { 2, "Explosion_Large",  { 51.f, -5.f },        0.f },
        { 2, "Explosion_Large",  { 42.f, 86.f },        0.f },
        { 2, "Explosion_Large",  { 87.f, 70.f },        0.f },
        { 3, "Explosion_Large",  { 21.f, 72.f },        0.f },
        { 3,   "Explosion_Mid", { 91.f, 113.f },        0.f },
        { 4, "Explosion_Large",  { 50.f, 95.f },        0.f },
        { 4,   "Explosion_Mid", { 80.f, 100.f }, 6.f / 60.f },
        { 5, "Explosion_Large", { 42.f, 101.f },        0.f },
        { 6,   "Explosion_Mid", { 45.f, 100.f },        0.f },
        { 7,   "Explosion_Mid",  { 9.f, 116.f },        0.f },
        { 7,   "Explosion_Mid",  { 78.f, 95.f }, 3.f / 60.f },
        { 6, "Explosion_Small",  { 80.f, 40.f }, 4.f / 60.f }
    };

    Vector<BusBurstDef> deathBursts = {
        { 0, "Explosion_Large", { 20.f, -10.f },         0.f },
        { 0, "Explosion_Large",   { 75.f, 0.f },         0.f },
        { 0, "Explosion_Large",  { 48.f, 35.f },  2.f / 60.f },
        { 0, "Explosion_Large",  { 10.f, 60.f },  3.f / 60.f },
        { 0, "Explosion_Large",  { 90.f, 55.f },  3.f / 60.f },
        { 0, "Explosion_Large",  { 30.f, 95.f },  5.f / 60.f },
        { 0, "Explosion_Large", { 70.f, 100.f },  6.f / 60.f },
        { 0, "Explosion_Large", { 48.f, 125.f },  8.f / 60.f },
        { 0,   "Explosion_Mid",  { 40.f, 20.f }, 14.f / 60.f },
        { 0,   "Explosion_Mid",  { 60.f, 80.f }, 18.f / 60.f }
    };

    Vector<BusDebrisDef> debris = {
        { "Bus_DebGrille", { 52.f, 112.f },  { -60.f, -520.f }, 14.f },
        { "Bus_DebBumper", { 52.f, 124.f },   { 90.f, -430.f }, 10.f },
        { "Bus_DebFrameA",  { 30.f, 30.f }, { -170.f, -640.f }, 12.f },
        { "Bus_DebFrameB",  { 75.f, 28.f },  { 160.f, -600.f }, 12.f },
        {  "Bus_DebChunk",  { 20.f, 70.f }, { -220.f, -480.f }, 16.f },
        {  "Bus_DebChunk",  { 80.f, 60.f },  { 240.f, -520.f }, 16.f },
        {  "Bus_DebChunk",  { 50.f, 20.f },   { 30.f, -700.f }, 16.f },
        {    "Bus_DebBar", { 15.f, 105.f }, { -140.f, -380.f }, 10.f },
        {    "Bus_DebBar", { 90.f, 100.f },  { 130.f, -420.f }, 10.f }
    };

    Vector<BusInnerSoldierDef> innerSoldiers = {
        { 3, { 98.f, 56.f },  0 },
        { 4, { 55.f, 54.f },  7 },
        { 4, { 83.f, 54.f }, 13 },
        { 5, { 22.f, 56.f },  3 }
    };

    Vector<BusWindowDef> windows = {
        { "Bus_WinUL", 1 },
        { "Bus_WinUR", 1 },
        { "Bus_WinLL", 4 },
        { "Bus_WinLR", 6 }
    };

    // 부품별 깊이. 병사(적 레이어 15, sort = 발 y) 는 back/body 앞, cover 뒤에 그려진다 (Bus.json 이 cover 레이어를 올린다)
    float          coverMaxX        = 82.f;
    EnemyPartDepth partInnerSoldier = { 15, 2 };
    EnemyPartDepth partWreck        = { 15, 0 };
    EnemyPartDepth partBack         = { 15, 1 };
    EnemyPartDepth partStairs       = { 15, 1 };
    EnemyPartDepth partBody         = { 15, 3 };
    EnemyPartDepth partCover        = { 15, 4 };
    EnemyPartDepth partWindow       = { 15, 5 };
    EnemyPartDepth partDebris       = { 15, 20 };

    float openViewDx       = -100.f;
    int   openDelayTicks   = 10;       // 개방 트리거 후 개방 시작까지 (원작 0.17 s)
    int   stairsDelayTicks = 46;
    int   wreckAfterTicks  = 6;
    int   destroyDoneTicks = 60;       // 사망 후 파괴 연출 종료 (카메라 락 해제 시점, 원작 약 1.0 s)

    ARRAY<int, kStageCount> stageHp = { 112, 100, 85, 62, 48, 34, 24 };

    // 계단 병사: 계단이 펼쳐진 뒤 버스 오른쪽 화면 밖에서 나와 계단을 오르고 승강대에서 폭탄을 던지거나 지붕을 건너 내려와 베기를 한다.
    // 바주카병은 계단을 오르지 않고 화면 밖에서 걸어 들어와 로켓을 쏜다.
    String soldierBombDef    = "soldier_bomb";
    String soldierSwordDef   = "soldier_sword";
    String soldierBzkDef     = "soldier_bzk";
    Vec2   soldierSpawn      = { 230.f, 0.f };
    Vec2   stairsBase        = { 186.f, 0.f };
    Vec2   stairsTop         = { 100.f, -106.f };
    float  landingW          = 34.f;                // 승강대 벽 폭 (0 이면 없음). 윗면 = stairsTop.y, 중심 x = stairsTop.x
    int    soldierFirstTicks = 40;
    int    soldierGapTicks   = 170;                 // 병사 스폰 간격 (약 2.8 s)
    int    soldierMax        = 2;                   // 동시에 살아 있는 계단 병사 수 (병사 종류 무관)
    int    swordPercent      = 40;                  // 스폰 병사가 대검병인 확률 (%)
    int    bazookaPercent    = 30;                  // 스폰 병사가 바주카병인 확률 (%), 나머지는 폭탄병
    int    soldierLastStage  = 5;
    Vec2   roofTop           = { 0.f, 0.f };        // 계단 병사가 마지막에 오르는 지붕 위 점 (앵커 기준). y 가 0 이면 지붕 없음 (승강대까지만)
    Vec2   roofSpan          = { 0.f, 0.f };
};

// 헬기 (Docs/Spec_Heli.md). 위치는 전부 기체 중심, 카메라(화면) 기준 px, 시간은 틱.
// 한 사이클: 고공 폭탄 (3발 연속, 휴지) x highBursts -> 저공 가로지르기 폭탄 (3발, 휴지) x lowBursts -> 포드 전개 + 미사일 -> 고공 복귀
struct HeliCfg
{
    // 고공 폭격 (진입 포함): 위 오른쪽에서 내려와 플레이어 위에 떠서 폭탄을 떨어뜨린다
    float enterSpeedY       = 40.f;   // 호버 높이까지 내려오는 속도 px/s
    float hoverDy           = 78.f;
    float followDx          = 15.f;
    float followSpeed       = 46.f;   // 따라가는 가로 속도 px/s
    float followDeadzone    = 6.f;
    int   firstBombTicks    = 54;
    int   reBombTicks       = 24;
    int   bombIntervalTicks = 24;
    int   burstCount        = 3;
    int   burstRestTicks    = 84;
    int   highBursts        = 2;
    int   lowBursts         = 2;
    Vec2  bombMuzzle        = { -8.f, 45.f };
    float bombMinY          = 8.f;

    float lowDy         = 200.f;
    float lowSpeed      = 90.f;    // 가로지르는 속도 px/s
    float lowMargin     = 90.f;
    float descendSpeedY = 70.f;    // 저공으로 내려오는 속도 px/s
    int   startLow      = 0;       // 1: 고공을 건너뛰고 저공 구간부터 시작

    int              podOutTicks     = 12;
    int              moveTicks       = 70;
    float            sideMargin      = 109.f;
    float            targetDyPlayer  = -22.f;
    float            targetMinY      = 150.f;
    float            targetMaxY      = 300.f;
    ARRAY<float, 13> xCurve          = { 0.f, 0.02f, 0.10f, 0.22f, 0.35f, 0.48f, 0.61f, 0.72f, 0.81f, 0.89f, 0.95f, 0.99f, 1.f };
    ARRAY<float, 13> yCurve          = { 0.f, 0.01f, 0.05f, 0.12f, 0.22f, 0.32f, 0.43f, 0.54f, 0.65f, 0.76f, 0.87f, 0.95f, 1.f };
    int              missiles        = 2;
    int              puffToMissile   = 10;
    int              missileInterval = 126;
    int              volleyEndTicks  = 90;                // 마지막 발사 연기 후 복귀까지 (원작 미관측: 가정값)
    Vec2             puffOffset      = { 55.f, 40.f };
    int              turnWaitTicks   = 24;                // 플레이어가 반대쪽에 이만큼 머물면 선회 (원작 미관측: 가정값)
    float            turnDx          = 60.f;
    Vec2             podOffset       = { 9.f, 32.f };
    Vec2             rotorOffset     = { 18.f, -50.f };
    int              idlePitchFrom   = 0;                 // 비행 (fly) 클립일 때 Pitch 프레임 from..to 를 왕복한다 (from == to 면 끔)
    int              idlePitchTo     = 0;
    int              idleStepTicks   = 12;
    String           rotorRowPrefix  = {};   // 비면 rotor 클립 고정. 아니면 prefix + 표시 중인 Pitch 프레임 번호 클립 (Heli_Rotor_0..6)

    EnemyPartDepth partRotor = { 16, 0 };
    EnemyPartDepth partPod   = { 16, 0 };
    EnemyPartDepth partPuff  = { 58, 0 };

    int    clipFly       = -1;
    int    clipPitch     = -1;
    int    clipTurnIn    = -1;
    int    clipTurnOut   = -1;
    int    clipPod       = -1;
    int    clipRotor     = -1;
    int    clipPuff      = -1;
    int    weaponMissile = -1;
    String bombDef       = {};
};

// 헬기 폭탄: 수직 낙하, 플레이어 총에 맞으면 공중에서 터진다 (원작: 플레이어 총알에 파괴됨)
struct HeliBombCfg
{
    float  startVy      = 100.f;   // 투하 직후 낙하 속도 px/s
    float  gravity      = 80.f;
    float  maxFall      = 400.f;
    Vec2   hitSize      = { 24.f, 50.f };
    Vec2   blastSize    = { 90.f, 80.f };
    int    blastTicks   = 8;
    int    blastDamage  = 1;
    float  groundProbeY = 25.f;
    String groundFx     = {};
    float  groundFxDy   = -6.f;
    int    clipFall     = -1;
};

// 피격 플래시 타이밍은 Game/Combat/HitFlash.h (피격당 ON 펄스 1회, Docs/Spec_HitFlash.md).
inline constexpr int kHitFlashMinHp       = 20;
inline constexpr int kEnemyFlashPartCount = 8;

enum class eExtraBoxKind : int
{
    Hurt,
    Attack
};

// 정의 JSON "hitboxes": 추가 피격 상자 / 공격 상자 (기본 비어 있음)
struct ExtraBoxDef
{
    String        id        = {};
    eExtraBoxKind kind      = eExtraBoxKind::Hurt;
    BoxDef        box       = {};
    int           frameFrom = 0;    // 공격 상자 전용: 클립 프레임 범위, frameTo < 0 이면 항상
    int           frameTo   = -1;
    int           damage    = 1;
};

enum class eArtFacing : uint8_t
{
    None,
    Left,
    Right
};

struct EnemyDef
{
    String         name     = {};
    eEnemyBehavior behavior = eEnemyBehavior::Soldier;
    eTeam          team     = eTeam::Enemy;
    int            hp       = 1;
    int            score    = 0;

    BoxDef              body       = {};
    BoxDef              hurtbox    = {};
    Vector<ExtraBoxDef> hitboxes   = {};
    BoxDef              solid      = {};   // 살아 있는 동안 플레이어/적의 벽 (발 기준, 비면 없음). 사망(Dying 이후) 시 제거
    ActivationDef       activation = {};
    PhysicsDef          physics    = {};

    Vector<NamedClip> clips   = {};
    Vector<WeaponDef> weapons = {};
    DeathDef          death   = {};
    Vector<DropDef>   drops   = {};

    int  overlayClip   = -1;
    Vec2 overlayOffset = { 0.f, 0.f };

    EnemyPartDepth partBody    = { 15, 0 };
    EnemyPartDepth partOverlay = { 14, 0 };

    SoldierCfg    soldier    = {};
    BzkCfg        bzk        = {};
    HangCfg       hang       = {};
    FlyingBzkCfg  flyingBzk  = {};
    ShielderCfg   shielder   = {};
    PowCfg        pow        = {};
    JeepCfg       jeep       = {};
    BTankCfg      btank      = {};
    MissleTankCfg missleTank = {};
    SarubiaCfg    sarubia    = {};
    BusCfg        bus        = {};
    HeliCfg       heli       = {};
    HeliBombCfg   heliBomb   = {};
    BigShieeCfg   bigShiee   = {};

    bool bNoDespawn = false;

    eArtFacing artFacing = eArtFacing::None;

    bool bHuman        = false;
    bool bHitFlash     = false;
    bool bCenterAnchor = false;

    [[nodiscard]] const ClipDef& GetClip(
        const int _index) const
    {
        return clips[static_cast<size_t>(_index)].clip;
    }

    [[nodiscard]] int FindClip(StringView _logicalName) const;
    [[nodiscard]] int FindWeapon(StringView _name) const;

    [[nodiscard]] bool ShouldFlipArt(int _facing) const;
};

struct EnemyGlobals
{
    float dpsRef                = 10.f;
    float activationMarginPx    = 150.f;
    float despawnBehindCameraPx = 200.f;
    float despawnBelowCameraPx  = 300.f;
};

class EnemyDefLibrary
{
public:
    [[nodiscard]] static Result<EnemyDefLibrary> Parse(
        StringView _json,
        String*    _pMessage = nullptr);

    [[nodiscard]] static Result<EnemyDefLibrary> Load(
        const FilePath& _path,
        String*         _pMessage = nullptr);

    [[nodiscard]] static Result<EnemyDefLibrary> LoadDirectory(
        const FilePath& _dir,
        String*         _pMessage = nullptr);


    [[nodiscard]] static FilePath GetDefaultDirectory(const FilePath& _assetRoot);

    [[nodiscard]] const EnemyDef* Find(StringView _name) const;
    [[nodiscard]] EnemyDef*       FindMutable(StringView _name);
    [[nodiscard]] int             FindIndex(StringView _name) const;
    [[nodiscard]] const EnemyDef& Get(int _index) const;

    [[nodiscard]] const ProjectileDef* FindProjectile(StringView _name) const;
    [[nodiscard]] int                  FindProjectileIndex(StringView _name) const;
    [[nodiscard]] const ProjectileDef& GetProjectile(int _index) const;

    [[nodiscard]] size_t                    GetDefCount() const;
    [[nodiscard]] size_t                    GetProjectileCount() const;
    [[nodiscard]] Span<const EnemyDef>      GetDefs() const;
    [[nodiscard]] Span<const ProjectileDef> GetProjectiles() const;
    [[nodiscard]] const EnemyGlobals&       GetGlobals() const;

private:
    friend class EnemyDefParser;

    Vector<EnemyDef>      m_defs        = {};
    Vector<ProjectileDef> m_projectiles = {};
    HashMap<String, int>  m_defIndex    = {};
    HashMap<String, int>  m_projIndex   = {};
    EnemyGlobals          m_globals     = {};
};

// 정의 JSON params.parts.<name>.{depth,sort} 로 덮어쓸 수 있는 부품 깊이 (기본값 = 코드 상수)
struct EnemyPartSlot
{
    const char*     pName  = "";
    EnemyPartDepth* pDepth = nullptr;
};

void GetEnemyPartSlots(EnemyDef& _def, Vector<EnemyPartSlot>& _out);
