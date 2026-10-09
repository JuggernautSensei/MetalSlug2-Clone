#pragma once
#include <cstdint>

#include "Base/Types.h"
#include "Base/Math.h"

#include "Core/Tuning.h"
#include "Game/Combat/CombatSystem.h"
#include "Game/Combat/Projectiles.h"
#include "Game/Enemies/EnemyDef.h"
#include "Render/Animation.h"
#include "World/Combat.h"
#include "World/Physics.h"

namespace slug
{
inline constexpr uint32_t kEnemyOwnerBase = 2000;
inline constexpr uint32_t kShieldOwnerBit = 0x40000000u;

inline constexpr uint32_t kEnemyIdMask = 0x00FFFFFFu;
[[nodiscard]] constexpr uint32_t MakePartOwner(
    const uint32_t _enemyId,
    const uint32_t _partIndex)
{
    return (_enemyId & kEnemyIdMask) | ((_partIndex + 1u) << 24);
}
[[nodiscard]] constexpr int GetPartIndex(
    const uint32_t _owner)
{
    const uint32_t part = (_owner >> 24) & 0x3Fu;
    return part == 0u ? -1 : static_cast<int>(part) - 1;
}

[[nodiscard]] constexpr bool IsEnemyOwner(
    const uint32_t _owner)
{
    const uint32_t id = _owner & ~kShieldOwnerBit;
    return id >= kEnemyOwnerBase && id < kShieldOwnerBit;
}

struct EnemyHandle
{
    uint32_t id = 0;

    [[nodiscard]] constexpr bool IsNull() const { return id == 0; }
    [[nodiscard]] constexpr bool operator==(const EnemyHandle&) const = default;
};

struct SpawnParams
{
    int         facing     = -1;      // -1 왼쪽 / +1 오른쪽
    int         hp         = -1;      // -1: def 기본값
    slug::String drop       = {};
    int         aiVariant  = 0;
    float       delaySec   = 0.f;     // 이 시간 뒤에 등장 (그 동안은 판정/렌더 없음)
    uint32_t    userTag    = 0;
    bool        bActivated = false;
    Aabb        arena      = {};
};

enum class eEnemyEventKind : uint8_t
{
    Died,
    ItemDropped,
    Freed,
    Transformed,
    Removed
};

struct EnemyEvent
{
    eEnemyEventKind kind     = eEnemyEventKind::Died;
    EnemyHandle     handle   = {};
    EnemyHandle     other    = {};
    int             defIndex = -1;
    slug::String     defName  = {};
    Vec2    pos      = { 0.f, 0.f };   // 발 위치
    int             score    = 0;
    slug::String     drop     = {};
    uint32_t        userTag  = 0;
};

struct EnemyPlayerInfo
{
    Vec2 feet           = { 0.f, 0.f };
    Vec2 center         = { 0.f, 0.f };   // 피격 상자 중심 (조준 목표)
    Aabb         bodyBox        = {};
    bool         bAlive         = true;
    bool         bInvulnerable  = false;
};

struct EnemyContext
{
    const Tuning*           pTuning      = nullptr;
    PhysicsWorld            physics      = {};
    CombatSystem*           pCombat      = nullptr;
    ProjectileSystem*       pProjectiles = nullptr;
    EnemyPlayerInfo         player       = {};
    Aabb                    cameraView   = {};
    const AnimationLibrary* pAnims       = nullptr;
};

enum class eEnemyLife : uint8_t
{
    Pending,
    Alive,
    Dying,
    Gone
};

struct Enemy
{
    uint32_t       id        = 0;
    int            defIndex  = -1;
    eEnemyBehavior behavior  = eEnemyBehavior::Soldier;
    eEnemyLife     life      = eEnemyLife::Alive;
    int            delayTicks = 0;

    Body         body      = {};
    Vec2 prevFeet  = { 0.f, 0.f };
    int          facing    = -1;
    Health       health    = {};
    bool         bActive   = false;
    bool         bInit     = false;
    bool         bOnOneWay = false;
    bool         bGrounded = false;

    int state      = 0;
    int stateTicks = 0;   // 현재 상태에 들어온 뒤 경과 틱 (진입 틱 = 0)
    int counter    = 0;   // 남은 발수 등
    int aux        = 0;   // 행동별 임의 정수
    int aux2       = 0;
    int aux3       = 0;
    int aux4       = 0;
    bool bTransformed = false;
    int  ext[24]      = {};
    Aabb arena        = {};

    int  clip      = -1;   // EnemyDef::clips 인덱스
    int  clipTicks = 0;    // 클립 시작 후 경과 틱
    bool bVisible  = true;

    uint32_t meleeHitId    = kNoHitId;
    int      meleeTicksLeft = 0;
    int      meleeWeapon   = -1;
    Vec2 meleeOrigin = { 0.f, 0.f };

    int      deathPhase = 0;
    int      deathTicks = 0;   // 최종 클립 시작 후 경과 틱 (깜빡임/제거 타이머)
    eHitKind lastHitKind = eHitKind::Bullet;
    int      shotsFired = 0;

    bool bShieldUp  = false;
    int  shieldDir  = 0;   // 0 앞 / 1 위 / 2 아래
    int  defenceTicks = 0;

    int         aiVariant = 0;
    uint32_t    userTag   = 0;
    slug::String drop      = {};

    [[nodiscard]] Vec2 GetFeet() const { return body.position; }
    [[nodiscard]] bool         IsAlive() const { return life == eEnemyLife::Alive; }
};

struct EnemyFilter
{
    int      defIndex = -1;   // -1: 전부
    uint32_t userTag  = 0;    // 0: 전부
    bool     bHostileOnly = true;
};
}
