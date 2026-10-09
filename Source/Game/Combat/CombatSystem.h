#pragma once
#include "Base/Types.h"

#include "World/Combat.h"

namespace slug
{

// ===========================================
//  CombatSystem
//   한 틱 동안의 Hitbox / Hurtbox 모음 + HitResolver.
//   모든 피해는 이곳을 거친다: 적/소품은 매 틱 자신의 Hurtbox 를 등록하고, 플레이어/탄/적 공격은 Hitbox 를 등록한다.
//   Resolve() 가 반환한 HitEvent 를 각 소유자가 자기 Health 에 적용한다 (대상별 특수 처리 없음).
//
//   틱 순서 (SandboxState 기준):
//     BeginTick → (Hurtbox 등록) → 플레이어 Tick (칼 자동 발동 질의 포함) → (Hitbox 등록) → Resolve → 이벤트 적용
// ===========================================

class CombatSystem
{
public:
    void BeginTick();

    void AddHurtbox(const Hurtbox& _hurtbox) { m_hurtboxes.push_back(_hurtbox); }
    void AddHitbox(const Hitbox& _hitbox) { m_hitboxes.push_back(_hitbox); }

    // 등록된 모든 판정을 해소한다. 반환 참조는 다음 BeginTick/Resolve 전까지 유효.
    const slug::Vector<HitEvent>& Resolve();

    // 새 공격 인스턴스 ID (0 은 "추적 안 함" 이므로 1 부터)
    [[nodiscard]] uint32_t NewHitId() { return ++m_nextHitId; }
    void                   ReleaseHitId(const uint32_t _hitId) { m_resolver.Release(_hitId); }

    // _attackerTeam 이 _kind 로 _box 를 때릴 수 있는 Hurtbox 가 하나라도 있는가 (칼 자동 발동, 폭탄 접촉 판단용).
    // 규칙은 HitResolver 와 동일 (팀 표 / 무적 / 받는 종류 / 겹침 / 자기 자신 제외).
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

}   // namespace slug
