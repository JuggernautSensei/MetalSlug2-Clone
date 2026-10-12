#include "World/Combat.h"

#include <algorithm>

#include "Base/Types.h"

namespace
{
[[nodiscard]] uint64_t MakeTargetKey_(
    const Hurtbox& _hurtbox,
    const size_t   _index)
{
    return _hurtbox.owner != kNoOwner ? static_cast<uint64_t>(_hurtbox.owner) : ((1ull << 32) | static_cast<uint64_t>(_index));
}
}   // namespace

Vector<HitEvent> HitResolver::Resolve(
    const Span<const Hitbox>  _hitboxes,
    const Span<const Hurtbox> _hurtboxes)
{
    Vector<HitEvent> events;

    for (size_t i = 0; i < _hitboxes.size(); ++i)
    {
        const Hitbox& hitbox = _hitboxes[i];
        if (hitbox.box.IsEmpty())
        {
            continue;
        }

        const bool bTracked = hitbox.id != kNoHitId;
        if (bTracked)
        {
            const HitRecordMap::iterator it = m_hitRecords.find(hitbox.id);
            if (it != m_hitRecords.end() && it->second.bConsumed)
            {
                continue;
            }
        }

        for (size_t j = 0; j < _hurtboxes.size(); ++j)
        {
            const Hurtbox& hurtbox = _hurtboxes[j];

            if (!CanTeamHit(hitbox.team, hurtbox.team))
            {
                continue;
            }
            if (hurtbox.bInvulnerable || !hurtbox.accepts.Has(hitbox.kind))
            {
                continue;
            }
            if (hitbox.owner != kNoOwner && hitbox.owner == hurtbox.owner)
            {
                continue;
            }
            if (!hitbox.box.Overlaps(hurtbox.box))
            {
                continue;
            }

            const uint64_t targetKey = MakeTargetKey_(hurtbox, j);
            if (bTracked)
            {
                const HitRecordMap::iterator it = m_hitRecords.find(hitbox.id);
                if (it != m_hitRecords.end())
                {
                    const Vector<uint64_t>& targets = it->second.targets;
                    if (std::find(targets.begin(), targets.end(), targetKey) != targets.end())
                    {
                        continue;
                    }
                }
            }

            HitEvent event;
            event.hitboxIndex  = static_cast<uint32_t>(i);
            event.hurtboxIndex = static_cast<uint32_t>(j);
            event.damage       = hitbox.damage;
            event.kind         = hitbox.kind;
            event.attackerTeam = hitbox.team;
            event.attacker     = hitbox.owner;
            event.target       = hurtbox.owner;
            event.hitId        = hitbox.id;
            events.push_back(event);

            if (bTracked)
            {
                HitRecord& record = m_hitRecords[hitbox.id];
                record.targets.push_back(targetKey);
                if (!hitbox.bPierce)
                {
                    record.bConsumed = true;
                }
            }

            if (!hitbox.bPierce)
            {
                break;
            }
        }
    }

    return events;
}

void HitResolver::Release(
    const uint32_t _hitId)
{
    m_hitRecords.erase(_hitId);
}

void HitResolver::Clear()
{
    m_hitRecords.clear();
}

size_t HitResolver::GetTrackedHitCount() const
{
    return m_hitRecords.size();
}

HitKindMask AcceptsFor(
    const bool _bMeleeable)
{
    if (_bMeleeable)
    {
        return HitKindMask::All();
    }
    return HitKindMask { eHitKind::Bullet, eHitKind::Blast };
}

HitKindMask::HitKindMask(
    std::initializer_list<eHitKind> _kinds)
{
    for (const eHitKind kind: _kinds)
    {
        m_bits |= Bit_(kind);
    }
}

HitKindMask HitKindMask::All()
{
    HitKindMask mask;
    mask.m_bits = ~0u;
    return mask;
}

bool HitKindMask::Has(
    const eHitKind _kind) const
{
    return (m_bits & Bit_(_kind)) != 0u;
}

uint32_t HitKindMask::Bit_(
    const eHitKind _kind)
{
    return 1u << static_cast<uint32_t>(_kind);
}

bool CanTeamHit(
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

Health Health::Make(
    const int _maxHp)
{
    return Health { _maxHp, _maxHp };
}

bool Health::IsDead() const
{
    return hp <= 0;
}

bool Health::IsAlive() const
{
    return hp > 0;
}

int Health::ApplyDamage(
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
