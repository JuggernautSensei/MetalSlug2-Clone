#include "World/Combat.h"

#include <algorithm>

namespace slug
{

namespace
{
    // 대상 식별 키: owner 가 있으면 owner, 없으면 hurtbox 인덱스 (겹치지 않도록 상위 비트로 구분)
    [[nodiscard]] uint64_t MakeTargetKey_(
        const Hurtbox& _hurtbox,
        const size_t   _index)
    {
        return _hurtbox.owner != kNoOwner ? static_cast<uint64_t>(_hurtbox.owner) : ((1ull << 32) | static_cast<uint64_t>(_index));
    }
}   // namespace

jug::Vector<HitEvent> HitResolver::Resolve(
    const jug::Span<const Hitbox>  _hitboxes,
    const jug::Span<const Hurtbox> _hurtboxes)
{
    jug::Vector<HitEvent> events;

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
            const auto it = m_hitRecords.find(hitbox.id);
            if (it != m_hitRecords.end() && it->second.bConsumed)
            {
                continue;   // 이미 소모된 비관통 공격
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
                const auto it = m_hitRecords.find(hitbox.id);
                if (it != m_hitRecords.end())
                {
                    const jug::Vector<uint64_t>& targets = it->second.targets;
                    if (std::find(targets.begin(), targets.end(), targetKey) != targets.end())
                    {
                        continue;   // 같은 공격이 같은 대상을 이미 때림
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
                break;   // 비관통: 첫 피격으로 소모
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

}   // namespace slug
