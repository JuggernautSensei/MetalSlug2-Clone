#pragma once
#include <JugX/Typedef.h>
#include <JugX/Vector2.h>

#include "Core/Tuning.h"
#include "Game/Combat/CombatSystem.h"
#include "Game/Combat/Projectiles.h"
#include "Render/Animation.h"
#include "World/Combat.h"

namespace slug
{

// ===========================================
//  훈련용 표적 (M3 적 구현 전 판정 검증용 임시 객체)
//   적과 똑같이 Hurtbox 를 등록하고 HitEvent 로만 피해를 받는다. 종류마다 규칙이 다르지 않다 (크기/체력만 다름).
// ===========================================

enum class eDummyKind : uint8_t
{
    Grunt,   // 보병 크기, 1 타
    Jeep,    // 차량 (체력 20)
    Bus      // 대형 (체력 60)
};

inline constexpr uint32_t kDummyOwnerBase = 1000;
inline constexpr uint8_t  kEnemyBulletTag = 100;

struct TrainingDummy
{
    uint32_t     id         = kNoOwner;
    eDummyKind   kind       = eDummyKind::Grunt;
    bool         bShooter   = false;   // 주기적으로 플레이어를 향해 총알 Hitbox 를 쏜다
    jug::VECTOR2 feet       = { 0.f, 0.f };
    jug::VECTOR2 size       = { 20.f, 44.f };
    int          facing     = -1;
    Health       health     = {};
    int          flashTicks = 0;    // 피격 번쩍임 남은 틱
    int          deadTicks  = -1;   // -1: 생존, 0..: 사망 후 경과 틱
    int          shootTimer = 0;
    int          hitsTaken  = 0;
    int          lastDamage = 0;
    eHitKind     lastKind   = eHitKind::Bullet;
    AnimPlayer   anim       = {};

    [[nodiscard]] bool IsAlive() const { return deadTicks < 0 && health.IsAlive(); }
};

class TrainingTargets
{
public:
    void Clear();

    // 표적 추가: 크기/체력은 종류와 Tuning 으로 정해진다. 반환: 소유자 ID
    uint32_t Add(eDummyKind _kind, jug::VECTOR2 _feet, bool _bShooter, const SandboxTuning& _tuning);

    // 살아 있는 표적의 Hurtbox 를 등록한다 (플레이어 Tick 전에 호출)
    void RegisterHurtboxes(CombatSystem& _combat) const;

    // 사수: 플레이어가 범위 안이면 주기적으로 적 총알을 쏜다 (Projectiles.Step 전에 호출)
    void TickShooters(const SandboxTuning& _tuning, jug::VECTOR2 _playerFeet, bool _bPlayerAlive, ProjectileSystem& _projectiles, CombatSystem& _combat);

    // Resolve 결과 반영. 반환: 이번에 처치로 얻은 점수 합
    int ApplyEvents(jug::Span<const HitEvent> _events, const SandboxTuning& _tuning);

    // 타이머 진행 (번쩍임, 사망 연출, 부활)
    void Tick(const SandboxTuning& _tuning);

    [[nodiscard]] jug::Span<const TrainingDummy> GetDummies() const { return m_dummies; }
    [[nodiscard]] jug::Span<TrainingDummy>       GetDummies() { return m_dummies; }
    [[nodiscard]] const TrainingDummy*           Find(uint32_t _id) const;
    [[nodiscard]] int                            GetKillCount() const { return m_killCount; }
    [[nodiscard]] uint64_t                       HashState() const;

private:
    void Revive_(TrainingDummy& _dummy, const SandboxTuning& _tuning) const;

    jug::Vector<TrainingDummy> m_dummies   = {};
    uint32_t                   m_nextId    = kDummyOwnerBase;
    int                        m_killCount = 0;
};

[[nodiscard]] jug::VECTOR2 GetDummySize(eDummyKind _kind);
[[nodiscard]] int          GetDummyHp(eDummyKind _kind, const SandboxTuning& _tuning);
[[nodiscard]] const char*  GetDummyName(eDummyKind _kind);

}   // namespace slug
