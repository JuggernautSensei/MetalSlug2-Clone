#pragma once
#include <cstdint>

#include <JugX/EnumFlags.h>
#include <JugX/Typedef.h>

#include "World/Geometry.h"

namespace slug
{

// ===========================================
//  판정 기본 타입
//   모든 피격 규칙의 단일 진입점은 HitResolver 이다.
//   적/무기 클래스는 레이어나 타입을 직접 비교하지 않는다. (칼이 특정 적을 못 때리는 류의 버그 방지)
// ===========================================

enum class eTeam : uint8_t
{
    Player,
    Enemy,
    Neutral   // 파괴 가능한 소품, 포로, 함정 등
};

enum class eHitKind : uint8_t
{
    Bullet,
    Melee,
    Blast,
    Hazard,
    Count
};

using HitKindMask = jug::IndexedFlags<eHitKind>;

inline constexpr uint32_t kNoOwner = 0;
inline constexpr uint32_t kNoHitId = 0;

// 공격자 팀이 대상 팀을 때릴 수 있는가.
//   Player  → Enemy, Neutral
//   Enemy   → Player
//   Neutral → Player, Enemy   (함정/위험물. Neutral 끼리는 무시)
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

// ===========================================
//  Hitbox / Hurtbox
// ===========================================

struct Hitbox
{
    Aabb     box     = {};
    eTeam    team    = eTeam::Player;
    eHitKind kind    = eHitKind::Bullet;
    int      damage  = 1;
    bool     bPierce = false;      // true: 여러 대상을 관통 (대상마다 1회). false: 첫 피격으로 소모
    uint32_t owner   = kNoOwner;   // 공격자 엔티티. 자기 자신의 Hurtbox 는 무시한다
    uint32_t id      = kNoHitId;   // 공격 인스턴스 ID. 같은 id 는 같은 대상을 두 번 때리지 않는다 (0 = 추적 안 함)
};

struct Hurtbox
{
    Aabb        box           = {};
    eTeam       team          = eTeam::Enemy;
    uint32_t    owner         = kNoOwner;        // 피격 엔티티
    HitKindMask accepts       = jug::kAllFlag;   // 받아들이는 공격 종류 (기본: 전부)
    bool        bInvulnerable = false;
};

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

// ===========================================
//  HitResolver
//   Hitbox × Hurtbox 규칙:
//    1) CanTeamHit(공격 팀, 대상 팀)
//    2) 대상이 무적이 아니고 해당 공격 종류(kind)를 받아들임
//    3) 상자가 겹침 (면적 > 0)
//    4) 자기 자신(owner 동일)이 아님
//    5) 같은 hit id 가 같은 대상(owner)을 이미 때리지 않았음, 비관통 hit id 가 소모되지 않았음
//   결과는 (hitbox 인덱스, hurtbox 인덱스) 오름차순으로 결정적이다.
//   비관통 Hitbox 는 첫 피격(가장 낮은 hurtbox 인덱스)으로 소모되며 이후 호출에서도 id 가 소모된 상태로 남는다.
//   같은 owner 가 Hurtbox 를 여러 개 가진 경우, 관통 Hitbox 라도 owner 당 1회만 맞는다.
//   이미 끝난 공격의 기록은 Release(id) 로 지운다.
// ===========================================

class HitResolver
{
public:
    [[nodiscard]] jug::Vector<HitEvent> Resolve(jug::Span<const Hitbox> _hitboxes, jug::Span<const Hurtbox> _hurtboxes);

    // 공격 인스턴스가 끝났을 때 호출하여 기록을 지운다.
    void Release(uint32_t _hitId);
    void Clear();

    [[nodiscard]] size_t GetTrackedHitCount() const { return m_hitRecords.size(); }

private:
    struct HitRecord
    {
        bool                  bConsumed = false;   // 비관통 공격이 이미 적중함
        jug::Vector<uint64_t> targets   = {};      // 이미 맞은 대상 키 (owner 또는 hurtbox 인덱스)
    };

    jug::HashMap<uint32_t, HitRecord> m_hitRecords = {};
};

// ===========================================
//  Health
// ===========================================

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

    // 실제로 깎인 양을 반환한다 (남은 체력보다 큰 피해는 남은 체력만큼). 음수 피해는 무시.
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

    // 실제로 회복된 양
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

}   // namespace slug
