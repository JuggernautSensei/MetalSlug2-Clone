#pragma once
#include <cstdint>
#include <initializer_list>

#include "Base/Types.h"
#include "World/Geometry.h"

enum class eTeam : uint8_t
{
    Player,
    Enemy,
    Neutral
};

enum class eHitKind : uint8_t
{
    Bullet,
    Melee,
    Blast,
    Hazard,
    Count
};

struct HitKindMask
{
    constexpr HitKindMask() = default;
    HitKindMask(std::initializer_list<eHitKind> _kinds);

    [[nodiscard]] static HitKindMask All();

    [[nodiscard]] bool Has(eHitKind _kind) const;

private:
    static uint32_t Bit_(eHitKind _kind);

    uint32_t m_bits = 0u;
};

inline constexpr uint32_t kNoOwner = 0;
inline constexpr uint32_t kNoHitId = 0;

[[nodiscard]] bool CanTeamHit(eTeam _attacker, eTeam _target);

struct Hitbox
{
    Aabb     box     = {};
    eTeam    team    = eTeam::Player;
    eHitKind kind    = eHitKind::Bullet;
    int      damage  = 1;
    bool     bPierce = false;
    uint32_t owner   = kNoOwner;
    uint32_t id      = kNoHitId;
};

struct Hurtbox
{
    Aabb        box           = {};
    eTeam       team          = eTeam::Enemy;
    uint32_t    owner         = kNoOwner;
    HitKindMask accepts       = HitKindMask::All();
    bool        bInvulnerable = false;
};

[[nodiscard]] HitKindMask AcceptsFor(bool _bMeleeable);

struct HitEvent
{
    uint32_t hitboxIndex  = 0;
    uint32_t hurtboxIndex = 0;
    int      damage       = 0;
    eHitKind kind         = eHitKind::Bullet;
    eTeam    attackerTeam = eTeam::Player;
    uint32_t attacker     = kNoOwner;
    uint32_t target       = kNoOwner;
    uint32_t hitId        = kNoHitId;
};

class HitResolver
{
public:
    [[nodiscard]] Vector<HitEvent> Resolve(Span<const Hitbox> _hitboxes, Span<const Hurtbox> _hurtboxes);

    void Release(uint32_t _hitId);
    void Clear();

    [[nodiscard]] size_t GetTrackedHitCount() const;

private:
    struct HitRecord
    {
        bool             bConsumed = false;
        Vector<uint64_t> targets   = {};
    };

    using HitRecordMap = HashMap<uint32_t, HitRecord>;

    HitRecordMap m_hitRecords = {};
};

struct Health
{
    int hp    = 1;
    int maxHp = 1;

    [[nodiscard]] static Health Make(int _maxHp);

    [[nodiscard]] bool IsDead() const;

    [[nodiscard]] bool IsAlive() const;

    int ApplyDamage(int _damage);
};
