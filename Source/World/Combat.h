#pragma once
#include <cstdint>

#include "Base/EnumFlags.h"
#include "Base/Types.h"

#include "World/Geometry.h"

namespace slug
{
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

using HitKindMask = slug::IndexedFlags<eHitKind>;

inline constexpr uint32_t kNoOwner = 0;
inline constexpr uint32_t kNoHitId = 0;

[[nodiscard]] constexpr bool CanTeamHit(
    const eTeam _attacker,
    const eTeam _target)
{
    switch (_attacker)
    {
        case eTeam::Player: return _target == eTeam::Enemy || _target == eTeam::Neutral;
        case eTeam::Enemy: return _target == eTeam::Player;
        case eTeam::Neutral: return _target == eTeam::Player || _target == eTeam::Enemy;
    }
    return false;
}

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
    HitKindMask accepts       = slug::kAllFlag;
    bool        bInvulnerable = false;
};

[[nodiscard]] inline HitKindMask AcceptsFor(const bool _bMeleeable)
{
    if (_bMeleeable)
    {
        return slug::kAllFlag;
    }
    return HitKindMask { eHitKind::Bullet, eHitKind::Blast };
}

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
    [[nodiscard]] slug::Vector<HitEvent> Resolve(slug::Span<const Hitbox> _hitboxes, slug::Span<const Hurtbox> _hurtboxes);

    void Release(uint32_t _hitId);
    void Clear();

    [[nodiscard]] size_t GetTrackedHitCount() const { return m_hitRecords.size(); }

private:
    struct HitRecord
    {
        bool                  bConsumed = false;
        slug::Vector<uint64_t> targets   = {};
    };

    slug::HashMap<uint32_t, HitRecord> m_hitRecords = {};
};

struct Health
{
    int hp    = 1;
    int maxHp = 1;

    [[nodiscard]] static constexpr Health Make(
        const int _maxHp)
    {
        return Health { _maxHp, _maxHp };
    }

    [[nodiscard]] constexpr bool IsDead() const
    {
        return hp <= 0;
    }

    [[nodiscard]] constexpr bool IsAlive() const
    {
        return hp > 0;
    }

    [[nodiscard]] constexpr float GetRatio() const
    {
        return maxHp > 0 ? static_cast<float>(hp) / static_cast<float>(maxHp) : 0.f;
    }

    constexpr int ApplyDamage(
        const int _damage)
    {
        if (_damage <= 0 || hp <= 0)
        {
            return 0;
        }
        const int applied = _damage < hp ? _damage : hp;
        hp -= applied;
        return applied;
    }

    constexpr int Heal(
        const int _amount)
    {
        if (_amount <= 0 || hp <= 0)
        {
            return 0;
        }
        const int room    = maxHp - hp;
        const int applied = _amount < room ? _amount : room;
        hp += applied;
        return applied;
    }
};
}
