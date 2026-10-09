#pragma once
#include <cstdint>
#include <optional>

#include <JugX/Result.h>
#include <JugX/Typedef.h>
#include <JugX/Vector2.h>

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
    Count
};

[[nodiscard]] const char*                    GetBehaviorName(eEnemyBehavior _behavior);
[[nodiscard]] std::optional<eEnemyBehavior> FindBehavior(jug::StringView _name);

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

    [[nodiscard]] Aabb At(const jug::VECTOR2 _feet, const int _facing) const
    {
        const float cxWorld = _feet.x + cx * static_cast<float>(_facing < 0 ? -1 : 1);
        return Aabb::FromCenterSize({ cxWorld, _feet.y + cy }, { w, h });
    }

    [[nodiscard]] bool IsEmpty() const { return w <= 0.f || h <= 0.f; }
};

// 클립 타이밍 (anim.json 과 같아야 한다 — 테스트가 검증). 시뮬레이션이 애니메이션 라이브러리 없이도 같은 타이밍으로 돌도록 JSON 에 둔다.
struct ClipDef
{
    jug::String name      = {};      // "SD_Idle_{side}" — {side} 는 Left/Right 로 치환
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
    jug::String name = {};   // 논리 이름 ("idle", "throw", ...)
    ClipDef     clip = {};
};

// 일회성 이펙트 요청 정의 (애니메이션 이름 + 오프셋(발 기준, 좌향이면 x 미러) + 지연)
struct FxDef
{
    jug::String  clip     = {};
    jug::VECTOR2 offset   = { 0.f, 0.f };
    float        delaySec = 0.f;
    int          layer    = 60;
};

// ===========================================
//  무기 / 투사체
// ===========================================

struct ProjectileDef
{
    jug::String  name            = {};
    jug::VECTOR2 size            = { 20.f, 20.f };
    int          damage          = 1;
    float        speed           = 0.f;   // 일정 속도 (방향 벡터에 곱한다). 0 이면 v0 / speedRamp 를 쓴다
    bool         bRamp           = false;   // 선형 가속: 0 → rampMax (진행 방향)
    float        rampFrom        = 0.f;
    float        rampAccel       = 0.f;
    float        rampMax         = 0.f;
    jug::VECTOR2 v0              = { 0.f, 0.f };   // 포물선 초기 속도 (x 는 바라보는 방향 부호가 곱해진다)
    float        gravity         = 0.f;
    float        maxFall         = 3000.f;
    float        lifetimeSec     = 5.f;
    float        startDelaySec   = 0.f;
    bool         bExplodeOnSolid = true;
    ClipDef      begin           = {};   // 점화 대기 동안의 클립 (선택)
    ClipDef      fly             = {};
    FxDef        explosion       = {};   // 적중/지형 접촉 시 연출 (clip 이 비면 없음)
};

enum class eWeaponAim : uint8_t
{
    Facing,   // 바라보는 방향 수평
    Given     // 호출자가 방향 벡터를 준다 (바주카/방패병)
};

struct WeaponDef
{
    jug::String  name            = {};
    int          projectile      = -1;   // EnemyDefLibrary::GetProjectile 인덱스 (-1 = 투사체 없음)
    jug::VECTOR2 muzzle          = { 0.f, 0.f };   // 발 기준, 좌향이면 x 미러
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
    jug::Vector<FxDef> fx = {};   // 사망 순간 이펙트 (디브리 등)
};

struct DropDef
{
    jug::String item  = {};
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
    jug::VECTOR2 muzzleBase = { 0.f, -63.f };
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
    jug::VECTOR2 muzzleBase = { 0.f, -63.f };
    float muzzleDist     = 50.f;
    float landProbe      = 1.f;     // 발 아래 이 거리 안에 지면이 있으면 착지로 본다
    jug::String landedDef = {};     // 착지 시 변신할 def 이름

    int clipReady[5] = { -1, -1, -1, -1, -1 };
    int clipShot[5]  = { -1, -1, -1, -1, -1 };
    int clipDieJump = -1, clipDieLand = -1;
    int weaponRocket = -1;

    // 낙하산 (몸 위에 겹쳐 그리는 오버레이) / 파괴 이펙트
    int          clipPara = -1;
    jug::VECTOR2 paraOffset = { -5.f, -87.5f };
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
//  EnemyDef
// ===========================================

struct EnemyDef
{
    jug::String    name     = {};
    eEnemyBehavior behavior = eEnemyBehavior::Soldier;
    eTeam          team     = eTeam::Enemy;
    int            hp       = 1;
    int            score    = 0;

    BoxDef        body      = {};   // 이동/지형 충돌 박스
    BoxDef        hurtbox   = {};   // 피격 박스 (기본: body)
    ActivationDef activation = {};
    PhysicsDef    physics   = {};

    jug::Vector<NamedClip> clips   = {};
    jug::Vector<WeaponDef> weapons = {};
    DeathDef               death   = {};
    jug::Vector<DropDef>   drops   = {};

    // 몸 위에 겹쳐 그리는 오버레이 클립 (선택): 낙하산 등. clips 인덱스, -1 = 없음
    int          overlayClip   = -1;
    jug::VECTOR2 overlayOffset = { 0.f, 0.f };

    // 행동별 설정 (behavior 에 해당하는 것만 의미가 있다)
    SoldierCfg   soldier   = {};
    BzkCfg       bzk       = {};
    HangCfg      hang      = {};
    FlyingBzkCfg flyingBzk = {};
    ShielderCfg  shielder  = {};
    PowCfg       pow       = {};

    [[nodiscard]] const ClipDef& GetClip(const int _index) const { return clips[static_cast<size_t>(_index)].clip; }
    [[nodiscard]] int            FindClip(jug::StringView _logicalName) const;
    [[nodiscard]] int            FindWeapon(jug::StringView _name) const;
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
    [[nodiscard]] static jug::Result<EnemyDefLibrary> Parse(jug::StringView _json, jug::String* _pMessage = nullptr);
    [[nodiscard]] static jug::Result<EnemyDefLibrary> Load(const jug::FilePath& _path, jug::String* _pMessage = nullptr);

    // <assetRoot>/Defs/Enemies.json
    [[nodiscard]] static jug::FilePath GetDefaultPath(const jug::FilePath& _assetRoot);

    [[nodiscard]] const EnemyDef* Find(jug::StringView _name) const;
    [[nodiscard]] int             FindIndex(jug::StringView _name) const;   // 없으면 -1
    [[nodiscard]] const EnemyDef& Get(const int _index) const { return m_defs[static_cast<size_t>(_index)]; }

    [[nodiscard]] const ProjectileDef* FindProjectile(jug::StringView _name) const;
    [[nodiscard]] int                  FindProjectileIndex(jug::StringView _name) const;
    [[nodiscard]] const ProjectileDef& GetProjectile(const int _index) const { return m_projectiles[static_cast<size_t>(_index)]; }

    [[nodiscard]] size_t                         GetDefCount() const { return m_defs.size(); }
    [[nodiscard]] size_t                         GetProjectileCount() const { return m_projectiles.size(); }
    [[nodiscard]] jug::Span<const EnemyDef>      GetDefs() const { return m_defs; }
    [[nodiscard]] jug::Span<const ProjectileDef> GetProjectiles() const { return m_projectiles; }
    [[nodiscard]] const EnemyGlobals&            GetGlobals() const { return m_globals; }

private:
    friend class EnemyDefParser;

    jug::Vector<EnemyDef>               m_defs        = {};
    jug::Vector<ProjectileDef>          m_projectiles = {};
    jug::HashMap<jug::String, int>      m_defIndex    = {};
    jug::HashMap<jug::String, int>      m_projIndex   = {};
    EnemyGlobals                        m_globals     = {};
};

}   // namespace slug
