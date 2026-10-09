#pragma once
#include "Base/Types.h"

#include "World/Combat.h"

namespace slug
{
class CombatSystem
{
public:
    void BeginTick();

    void AddHurtbox(const Hurtbox& _hurtbox) { m_hurtboxes.push_back(_hurtbox); }
    void AddHitbox(const Hitbox& _hitbox) { m_hitboxes.push_back(_hitbox); }

    const slug::Vector<HitEvent>& Resolve();

    [[nodiscard]] uint32_t NewHitId() { return ++m_nextHitId; }
    void                   ReleaseHitId(const uint32_t _hitId) { m_resolver.Release(_hitId); }

    [[nodiscard]] bool HasTargetIn(eTeam _attackerTeam, eHitKind _kind, const Aabb& _box, uint32_t _ignoreOwner = kNoOwner) const;

    [[nodiscard]] slug::Span<const Hurtbox>     GetHurtboxes() const { return m_hurtboxes; }
    [[nodiscard]] slug::Span<const Hitbox>      GetHitboxes() const { return m_hitboxes; }
    [[nodiscard]] const slug::Vector<HitEvent>& GetEvents() const { return m_events; }
    [[nodiscard]] HitResolver&                 GetResolver() { return m_resolver; }

private:
    HitResolver           m_resolver  = {};
    slug::Vector<Hitbox>   m_hitboxes  = {};
    slug::Vector<Hurtbox>  m_hurtboxes = {};
    slug::Vector<HitEvent> m_events    = {};
    uint32_t              m_nextHitId = 0;
};
}
