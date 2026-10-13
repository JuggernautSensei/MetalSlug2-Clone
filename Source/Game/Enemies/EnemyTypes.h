#pragma once
#include <cstdint>

#include "Base/Math.h"
#include "Base/Types.h"
#include "Core/Tuning.h"
#include "Game/Combat/CombatSystem.h"
#include "Game/Combat/Projectiles.h"
#include "Game/Enemies/EnemyDef.h"
#include "Render/Animation.h"
#include "World/Combat.h"
#include "World/Physics.h"

inline constexpr uint32_t kEnemyOwnerBase = 2000;
inline constexpr uint32_t kShieldOwnerBit = 0x40000000u;

inline constexpr uint32_t kEnemyIdMask = 0x00FFFFFFu;

[[nodiscard]] uint32_t MakePartOwner(uint32_t _enemyId, uint32_t _partIndex);

[[nodiscard]] int GetPartIndex(uint32_t _owner);

[[nodiscard]] bool IsEnemyOwner(uint32_t _owner);

struct EnemyHandle
{
    uint32_t id = 0;

    [[nodiscard]] bool IsNull() const;

    [[nodiscard]] constexpr bool operator==(const EnemyHandle&) const = default;
};

struct SpawnParams
{
    int      facing     = -1;
    int      hp         = -1;
    String   drop       = {};
    int      aiVariant  = 0;
    float    delaySec   = 0.f;   // 이 시간 뒤에 등장
    uint32_t userTag    = 0;
    bool     bActivated = false;
    Aabb     arena      = {};
    VECTOR2  climbEnd   = { 0.f, 0.f };
    VECTOR2  roofSpan   = { 0.f, 0.f };
    int      z          = kDefaultZ;
};

enum class eEnemyEventKind : uint8_t
{
    Died,
    ItemDropped,
    Freed,
    Transformed,
    Removed,
    PartDestroyed   // 다부위 보스의 부위 파괴
};

struct EnemyEvent
{
    eEnemyEventKind kind     = eEnemyEventKind::Died;
    EnemyHandle     handle   = {};
    EnemyHandle     other    = {};
    int             defIndex = -1;
    VECTOR2         pos      = { 0.f, 0.f };
    int             score    = 0;
    String          drop     = {};
    uint32_t        userTag  = 0;
};

struct EnemyPlayerInfo
{
    VECTOR2 feet          = { 0.f, 0.f };
    VECTOR2 center        = { 0.f, 0.f };
    Aabb    bodyBox       = {};
    bool    bAlive        = true;
    bool    bInvulnerable = false;
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
    uint32_t       id         = 0;
    int            defIndex   = -1;
    eEnemyBehavior behavior   = eEnemyBehavior::Soldier;
    eEnemyLife     life       = eEnemyLife::Alive;
    int            delayTicks = 0;
    int            z          = 0;

    Body    body      = {};
    VECTOR2 prevFeet  = { 0.f, 0.f };
    int     facing    = -1;
    Health  health    = {};
    bool    bActive   = false;
    bool    bInit     = false;
    bool    bOnOneWay = false;
    bool    bGrounded = false;

    int            state        = 0;
    int            stateTicks   = 0;
    int            counter      = 0;
    int            aux          = 0;
    int            aux2         = 0;
    int            aux3         = 0;
    int            aux4         = 0;
    bool           bTransformed = false;
    ARRAY<int, 48> ext          = {};
    Aabb           arena        = {};

    int  clip      = -1;
    int  clipTicks = 0;
    bool bVisible  = true;

    int                              hitFlashTicks  = 0;   // 전체 피격 플래시 남은 틱
    ARRAY<int, kEnemyFlashPartCount> partFlashTicks = {};

    uint32_t meleeHitId     = kNoHitId;
    int      meleeTicksLeft = 0;
    int      meleeWeapon    = -1;
    VECTOR2  meleeOrigin    = { 0.f, 0.f };
    uint32_t boxHitId       = kNoHitId;

    int      deathPhase  = 0;
    int      deathTicks  = 0;
    eHitKind lastHitKind = eHitKind::Bullet;
    int      shotsFired  = 0;

    bool bShieldUp    = false;
    int  shieldDir    = 0;
    int  defenceTicks = 0;

    int      aiVariant = 0;
    uint32_t userTag   = 0;
    String   drop      = {};
    VECTOR2  climbEnd  = { 0.f, 0.f };
    VECTOR2  roofSpan  = { 0.f, 0.f };
    bool     bLeap     = false;

    [[nodiscard]] VECTOR2 GetFeet() const;

    [[nodiscard]] bool IsAlive() const;
};

// 전체 스프라이트 플래시 양
[[nodiscard]] float HitFlashAmount(const Enemy& _e);

[[nodiscard]] float PartFlashAmount(const Enemy& _e, int _part);

void TriggerPartHitFlash(Enemy& _e, int _part);

struct EnemyFilter
{
    int      defIndex     = -1;
    uint32_t userTag      = 0;
    bool     bHostileOnly = true;
};
