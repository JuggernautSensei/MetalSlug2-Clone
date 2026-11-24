#include "Game/Combat/CombatSystem.h"

#include "Base/Types.h"

void CombatSystem::BeginTick()
{
    m_hitboxes.clear();
    m_hurtboxes.clear();
    m_events.clear();
}

const Vector<HitEvent>& CombatSystem::Resolve()
{
    m_events = m_resolver.Resolve(m_hitboxes, m_hurtboxes);
    return m_events;
}

bool CombatSystem::HasTargetIn(
    const eTeam    _attackerTeam,
    const eHitKind _kind,
    const Aabb&    _box,
    const uint32_t _ignoreOwner) const
{
    for (const Hurtbox& hurtbox: m_hurtboxes)
    {
        if (!CanTeamHit(_attackerTeam, hurtbox.team) || hurtbox.bInvulnerable || !hurtbox.accepts.Has(_kind))
        {
            continue;
        }
        if (_ignoreOwner != kNoOwner && hurtbox.owner == _ignoreOwner)
        {
            continue;
        }
        if (_box.Overlaps(hurtbox.box))
        {
            return true;
        }
    }
    return false;
}

void CombatSystem::AddHurtbox(
    const Hurtbox& _hurtbox)
{
    m_hurtboxes.push_back(_hurtbox);
}

void CombatSystem::AddHitbox(
    const Hitbox& _hitbox)
{
    m_hitboxes.push_back(_hitbox);
}

uint32_t CombatSystem::NewHitId()
{
    return ++m_nextHitId;
}

void CombatSystem::ReleaseHitId(
    const uint32_t _hitId)
{
    m_resolver.Release(_hitId);
}

Span<const Hurtbox> CombatSystem::GetHurtboxes() const
{
    return m_hurtboxes;
}

Span<const Hitbox> CombatSystem::GetHitboxes() const
{
    return m_hitboxes;
}

HitResolver& CombatSystem::GetResolver()
{
    return m_resolver;
}
