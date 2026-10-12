#pragma once
#include "Base/Types.h"
#include "World/Combat.h"

class CombatSystem
{
public:
    void BeginTick();

    void AddHurtbox(const Hurtbox& _hurtbox);
    void AddHitbox(const Hitbox& _hitbox);

    const Vector<HitEvent>& Resolve();

    [[nodiscard]] uint32_t NewHitId();
    void                   ReleaseHitId(uint32_t _hitId);

    [[nodiscard]] bool HasTargetIn(
        eTeam       _attackerTeam,
        eHitKind    _kind,
        const Aabb& _box,
        uint32_t    _ignoreOwner = kNoOwner) const;

    [[nodiscard]] Span<const Hurtbox>     GetHurtboxes() const;
    [[nodiscard]] Span<const Hitbox>      GetHitboxes() const;
    [[nodiscard]] const Vector<HitEvent>& GetEvents() const;
    [[nodiscard]] HitResolver&            GetResolver();

private:
    HitResolver      m_resolver  = {};
    Vector<Hitbox>   m_hitboxes  = {};
    Vector<Hurtbox>  m_hurtboxes = {};
    Vector<HitEvent> m_events    = {};
    uint32_t         m_nextHitId = 0;
};
