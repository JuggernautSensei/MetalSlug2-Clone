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
enum class eEnemyBehavior : uint8_t
{
    Soldier,
    Bzk,
    Hang,
    FlyingBzk,
    Shielder,
    Pow,
    Bus,
    BigShiee,
    Jeep,
    BTank,
    MissleTank,
    Sarubia,
    Aparch,
    Count
};

[[nodiscard]] const char*                    GetBehaviorName(eEnemyBehavior _behavior);
[[nodiscard]] std::optional<eEnemyBehavior> FindBehavior(slug::StringView _name);

inline constexpr int64_t kAnimUnitsPerTick = 100'000;

[[nodiscard]] int SecToTicks(float _sec);

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

struct ClipDef
{
    slug::String name      = {};
    int         frames    = 1;
    float       frameSec  = 0.1f;
    bool        bLoop     = false;
    int64_t     frameUnits = 0;
    bool        bSided    = false;

    [[nodiscard]] bool IsValid() const { return !name.empty(); }

    [[nodiscard]] int TickOfFrame(int _frame) const;

    [[nodiscard]] int GetDurationTicks() const;

    [[nodiscard]] bool IsFinished(int _ticks) const { return !bLoop && _ticks >= GetDurationTicks(); }
};

struct NamedClip
{
    slug::String name = {};
    ClipDef     clip = {};
};

struct FxDef
{
    slug::String  clip     = {};
    Vec2 offset   = { 0.f, 0.f };
    float        delaySec = 0.f;
    int          layer    = 60;
};

struct ProjectileDef
{
    slug::String  name            = {};
    Vec2 size            = { 20.f, 20.f };
    int          damage          = 1;
    float        speed           = 0.f;
    bool         bRamp           = false;
    float        rampFrom        = 0.f;
    float        rampAccel       = 0.f;
    float        rampMax         = 0.f;
    Vec2 v0              = { 0.f, 0.f };   // 포물선 초기 속도 (x 는 바라보는 방향 부호가 곱해진다)
    float        gravity         = 0.f;
    float        maxFall         = 3000.f;
    float        lifetimeSec     = 5.f;
    float        startDelaySec   = 0.f;
    bool         bExplodeOnSolid = true;
    ClipDef      begin           = {};
    ClipDef      fly             = {};
    FxDef        explosion       = {};
    float        rollSpeed       = 0.f;
    float        rollSec         = 0.f;
    bool         bExpireFx       = false;
    ClipDef      land            = {};
};

enum class eWeaponAim : uint8_t
{
    Facing,
    Given
};

struct WeaponDef
{
    slug::String  name            = {};
    int          projectile      = -1;
    Vec2 muzzle          = { 0.f, 0.f };   // 발 기준, 좌향이면 x 미러
    eWeaponAim   aim             = eWeaponAim::Facing;
    float        tiltThreshold   = 0.f;
    float        tiltDy          = 0.f;
    bool         bArc            = false;
    float        arcMinVx        = 0.f;
    float        arcMaxVx        = 0.f;
    FxDef        muzzleFx        = {};
    bool         bHitbox         = false;
    BoxDef       hitbox          = {};
    float        hitboxLifeSec   = 0.f;
    int          hitboxDamage    = 1;
    bool         bHitboxFollows  = false;
};

struct DeathDef
{
    int   clipBomb       = -1;   // 폭발 계열 피격 → 날아감
    int   clipSlash      = -1;   // 근접(칼) 피격 → 베임
    int   clipStand1     = -1;
    int   clipStand2     = -1;
    int   clipFall       = -1;
    int   clipLand       = -1;
    int   clipJump       = -1;   // 공중 사망
    int   fallAt1        = 0;
    int   fallAt2        = 0;
    float blinkAtSec     = 2.f;   // 최종 클립 시작 후
    float blinkForSec    = 1.f;
    float blinkPeriodSec = 0.1f;
    float removeAtSec    = 3.f;
    slug::Vector<FxDef> fx = {};
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
    bool  bAlways  = false;
};

struct PhysicsDef
{
    float gravity       = 1000.f;
    float maxFall       = 3000.f;
    float speed         = 100.f;    // 기본 이동 속도 px/s
    float jumpVy        = -450.f;
    float dropSpeed     = 100.f;    // 원웨이 하강 속도
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

    int clipIdle = -1, clipMove = -1, clipMoveDown = -1, clipFall = -1, clipHJump = -1, clipLand = -1;
    int clipThrow = -1, clipSlash = -1, clipJumpSlashReady = -1, clipJumpSlash = -1, clipJumpSlashLand = -1;

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
    int   clipIdle = -1, clipThrow = -1;
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
    slug::String landedDef = {};

    int clipReady[5] = { -1, -1, -1, -1, -1 };
    int clipShot[5]  = { -1, -1, -1, -1, -1 };
    int weaponRocket = -1;

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

struct BTankCfg
{
    float activateDx    = 500.f;
    int   idleTicks     = 180;     // 열린 뒤 발사 대기 (3 s)
    int   shotsPerCycle = 2;       // 한 번에 연속 발사 수
    int   fireFrame     = 2;       // Fire 클립 안 발사 프레임

    int clipIdle = -1, clipOpen = -1, clipFire = -1, clipDestroy = -1;
    int weaponShell = -1;
};

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

struct AparchCfg
{
    float speed        = 300.f;   // 접근 속도
    float decelSq      = 10000.f;
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

struct BusCfg
{
    float    originDepth        = 123.f;
    BoxDef   trigger            = {};
    int      openTicks          = 210;       // MainTruck_Begin 길이 (3.5 s)
    int      shellFirstTicks    = 120;       // 첫 포탄 (전투 시작 후 2 s)
    int      shellIntervalTicks = 180;       // 이후 3 s 간격
    int      soldierFirstTicks  = 180;       // 첫 병사 투하 (3 s)
    int      soldierIntervalTicks = 300;     // 이후 5 s 간격
    int      soldierAliveMax    = 2;         // 동시 생존 투하 병사
    int      soldierTotalMax    = 8;         // 누적 투하 병사
    Vec2 soldierOffset  = { -150.f, -40.f };   // 원점 기준 투하 위치
    slug::String  soldierDef     = {};
    int      weaponShell        = -1;        // 포탄 무기 (Given 이 아닌 Facing + arc)
};

inline constexpr int kBigShieeTurretCount = 7;   // 포탑 A..G

struct BigShieeCfg
{
    float    originDepth       = 208.f;    // 발(바닥) → 프레임 중심 (아트 높이 416 의 절반)
    float    homeX             = 290.f;    // 아레나 왼쪽 기준 중심 x (화면 좌표)
    float    driveMinX         = 205.f;    // 선체 주행 범위 (화면 좌표)
    float    driveMaxX         = 290.f;
    float    driveSpeed        = 50.f;     // px/s
    int      driveRestTicks    = 180;      // 목표 도착 후 정지 (3 s)
    float    entranceFromX     = 420.f;    // 등장 시작 = home + 이 값 (오른쪽 화면 밖에서 굴러 들어온다)
    int      entranceTicks     = 150;      // 2.5 s
    int      rumbleTicks       = 15;       // 등장 중 기저 폭발 주기
    float    hullBlockDx       = -60.f;    // 플레이어 x 상한 = 보스 x + 이 값 (플레이어는 보스 왼쪽 띠에 머문다)
    int      assaultTicks      = 720;      // 1단계: 포탑 연사 12 s
    int      burstTicks        = 72;       // 포탑 연사 간격 1.2 s
    int      burstCount        = 2;        // 한 번에 발사하는 포탑 수
    int      deployTicks       = 240;      // 2단계: 캐논 전개 4 s (클립 4 단계)
    int      volleyMinTicks    = 180;      // 캐논 단계 볼리 간격 3 s ~ 4 s
    int      volleyMaxTicks    = 240;
    int      volleyStreaks     = 3;        // 볼리당 청록 트레이서 수
    int      shellEvery        = 2;        // N 번째 볼리마다 주황 포탄 1발
    int      telegraphTicks    = 24;       // 사격 예고 (포탑 Shooting 클립) 0.4 s
    int      streakStaggerTicks = 6;       // 트레이서 간격 0.1 s
    float    shellJitter       = 20.f;     // 포탄 착탄 목표 = 플레이어 x ± 이 값
    float    enragedRate       = 1.3f;     // 격앙 시 발사 빈도 배율 (캐논 파괴 또는 선체 50% 이하)
    float    streakSpeed       = 400.f;    // 청록 트레이서 속도 px/s
    int      turretHp          = 60;
    int      cannonHp          = 300;
    Vec2     turretOffset[kBigShieeTurretCount] = {};   // 보스 원점 기준 포탑 중심
    Vec2     turretMuzzle      = { -40.f, -10.f };      // 포탑 중심 기준 포구 (왼쪽 발사)
    BoxDef   turretBox         = {};                    // 포탑 중심 기준 피격 상자
    Vec2     cannonOffset      = { -133.f, -75.f };     // 보스 원점 기준 캐논 헤드 중심 (Gun_* 클립 피벗)
    Vec2     cannonMuzzle      = { -173.f, -64.f };     // 보스 원점 기준 포구
    BoxDef   cannonBox         = {};                    // 보스 원점 기준 피격 상자
    BoxDef   hullBox           = {};                    // 보스 원점 기준 선체 피격 상자
    Vec2     emblemOffset      = { 245.f, 25.f };       // 보스 원점 기준 엠블럼 스핀 오버레이
    Vec2     wreckCannonOffset = { -150.f, 60.f };      // 잔해 원점 기준 떨어진 캐논 조각
    slug::String tracerProjectile = {};
    slug::String shellProjectile  = {};
    slug::String blastProjectile  = {};
    float    destroyChainSec   = 8.f;      // 선체 폭발 연쇄 (사망 후)
    float    destroyStepSec    = 0.2f;
    float    destroyFinishSec  = 10.f;     // 침몰 포함 IsDestroyFinished
    float    smokeStepSec      = 0.5f;     // 잔해 연기 주기
};

struct EnemyDef
{
    slug::String    name     = {};
    eEnemyBehavior behavior = eEnemyBehavior::Soldier;
    eTeam          team     = eTeam::Enemy;
    int            hp       = 1;
    int            score    = 0;

    BoxDef        body      = {};
    BoxDef        hurtbox   = {};
    ActivationDef activation = {};
    PhysicsDef    physics   = {};

    slug::Vector<NamedClip> clips   = {};
    slug::Vector<WeaponDef> weapons = {};
    DeathDef               death   = {};
    slug::Vector<DropDef>   drops   = {};

    int          overlayClip   = -1;
    Vec2 overlayOffset = { 0.f, 0.f };

    SoldierCfg   soldier   = {};
    BzkCfg       bzk       = {};
    HangCfg      hang      = {};
    FlyingBzkCfg flyingBzk = {};
    ShielderCfg  shielder  = {};
    PowCfg       pow       = {};
    BusCfg         bus         = {};
    BigShieeCfg    bigShiee    = {};
    JeepCfg        jeep        = {};
    BTankCfg       btank       = {};
    MissleTankCfg  missleTank  = {};
    SarubiaCfg     sarubia     = {};
    AparchCfg      aparch      = {};

    bool bNoDespawn = false;

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

class EnemyDefLibrary
{
public:
    [[nodiscard]] static slug::Result<EnemyDefLibrary> Parse(slug::StringView _json, slug::String* _pMessage = nullptr);
    [[nodiscard]] static slug::Result<EnemyDefLibrary> Load(const slug::FilePath& _path, slug::String* _pMessage = nullptr);

    [[nodiscard]] static slug::Result<EnemyDefLibrary> LoadDirectory(const slug::FilePath& _dir, slug::String* _pMessage = nullptr);

    [[nodiscard]] static slug::FilePath GetDefaultPath(const slug::FilePath& _assetRoot);

    [[nodiscard]] static slug::FilePath GetDefaultDirectory(const slug::FilePath& _assetRoot);

    [[nodiscard]] const EnemyDef* Find(slug::StringView _name) const;
    [[nodiscard]] int             FindIndex(slug::StringView _name) const;
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
}
