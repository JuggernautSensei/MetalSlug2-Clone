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

[[nodiscard]] uint32_t MakePartOwner(
    const uint32_t _enemyId,
    const uint32_t _partIndex);

[[nodiscard]] int GetPartIndex(
    const uint32_t _owner);

[[nodiscard]] bool IsEnemyOwner(
    const uint32_t _owner);

struct EnemyHandle
{
    uint32_t id = 0;

    [[nodiscard]] bool IsNull() const;

    [[nodiscard]] constexpr bool operator==(const EnemyHandle&) const = default;
};

struct SpawnParams
{
    int      facing     = -1;   // -1 왼쪽 / +1 오른쪽
    int      hp         = -1;   // -1: def 기본값
    String   drop       = {};
    int      aiVariant  = 0;
    float    delaySec   = 0.f;   // 이 시간 뒤에 등장 (그 동안은 판정/렌더 없음)
    uint32_t userTag    = 0;
    bool     bActivated = false;
    Aabb     arena      = {};
    Vec2     climbEnd   = { 0.f, 0.f };   // 계단 병사: arena.max 뒤 마지막 오름 목표 (y 가 0 이면 없음)
    Vec2     roofSpan   = { 0.f, 0.f };   // 계단 병사: 지붕 발판의 월드 x 범위 [최소, 최대] (최대 <= 최소 이면 없음)
    int      z          = kDefaultZ;      // 인스턴스 depth (kDefaultZ: 종류 기본값)
};

enum class eEnemyEventKind : uint8_t
{
    Died,
    ItemDropped,
    Freed,
    Transformed,
    Removed,
    PartDestroyed   // 다부위 보스의 부위 파괴 (점수만 준다, 적 사망/게이트와 무관)
};

struct EnemyEvent
{
    eEnemyEventKind kind     = eEnemyEventKind::Died;
    EnemyHandle     handle   = {};
    EnemyHandle     other    = {};
    int             defIndex = -1;
    String          defName  = {};
    Vec2            pos      = { 0.f, 0.f };   // 발 위치
    int             score    = 0;
    String          drop     = {};
    uint32_t        userTag  = 0;
};

struct EnemyPlayerInfo
{
    Vec2 feet          = { 0.f, 0.f };
    Vec2 center        = { 0.f, 0.f };   // 피격 상자 중심 (조준 목표)
    Aabb bodyBox       = {};
    bool bAlive        = true;
    bool bInvulnerable = false;
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
    int            z          = 0;   // 인스턴스 depth (기본 = GetEnemyDefaultZ)

    Body   body      = {};
    Vec2   prevFeet  = { 0.f, 0.f };
    int    facing    = -1;
    Health health    = {};
    bool   bActive   = false;
    bool   bInit     = false;
    bool   bOnOneWay = false;
    bool   bGrounded = false;

    int            state        = 0;
    int            stateTicks   = 0;   // 현재 상태에 들어온 뒤 경과 틱 (진입 틱 = 0)
    int            counter      = 0;   // 남은 발수 등
    int            aux          = 0;   // 행동별 임의 정수
    int            aux2         = 0;
    int            aux3         = 0;
    int            aux4         = 0;
    bool           bTransformed = false;
    ARRAY<int, 48> ext          = {};
    Aabb           arena        = {};

    int  clip      = -1;   // EnemyDef::clips 인덱스
    int  clipTicks = 0;    // 클립 시작 후 경과 틱
    bool bVisible  = true;

    int                              hitFlashTicks  = 0;    // 전체 피격 플래시 남은 틱 (0 = 없음, 피격 시 kHitFlashTicks 로 채움, ON 중 재피격은 무시: Game/Combat/HitFlash.h)
    ARRAY<int, kEnemyFlashPartCount> partFlashTicks = {};   // 부위별 피격 플래시 남은 틱 (다부위 보스/버스용, TriggerPartHitFlash 참고)

    uint32_t meleeHitId     = kNoHitId;
    int      meleeTicksLeft = 0;
    int      meleeWeapon    = -1;
    Vec2     meleeOrigin    = { 0.f, 0.f };
    uint32_t boxHitId       = kNoHitId;   // 정의 hitboxes 공격 상자용

    int      deathPhase  = 0;
    int      deathTicks  = 0;   // 최종 클립 시작 후 경과 틱 (깜빡임/제거 타이머)
    eHitKind lastHitKind = eHitKind::Bullet;
    int      shotsFired  = 0;

    bool bShieldUp    = false;
    int  shieldDir    = 0;   // 0 앞 / 1 위 / 2 아래
    int  defenceTicks = 0;

    int      aiVariant = 0;
    uint32_t userTag   = 0;
    String   drop      = {};
    Vec2     climbEnd  = { 0.f, 0.f };
    Vec2     roofSpan  = { 0.f, 0.f };
    bool     bLeap     = false;   // 계단 끝 전방 점프 중: 공중에서 수평 속도를 유지한다 (착지하면 꺼짐)

    [[nodiscard]] Vec2 GetFeet() const;

    [[nodiscard]] bool IsAlive() const;
};

// 전체 스프라이트 플래시 양 (0 또는 1). 렌더 경로에서 SpriteDrawOptions::flashAmount 로 전달한다.
[[nodiscard]] float HitFlashAmount(
    const Enemy& _e);

// 부위 플래시 양 (_part 가 범위 밖이면 0).
[[nodiscard]] float PartFlashAmount(
    const Enemy& _e,
    const int    _part);

// 부위 피격 플래시 시작/갱신 (_part 범위 밖이면 무시). 부위 파괴 시 호출자가 0 으로 되돌린다.
void TriggerPartHitFlash(
    Enemy&    _e,
    const int _part);

struct EnemyFilter
{
    int      defIndex     = -1;   // -1: 전부
    uint32_t userTag      = 0;    // 0: 전부
    bool     bHostileOnly = true;
};
