#pragma once
#include "Base/Math.h"
#include "Base/Types.h"
#include "Core/Tuning.h"
#include "Game/Combat/CombatSystem.h"
#include "Game/Combat/HitFlash.h"
#include "Game/Combat/Projectiles.h"
#include "Render/Animation.h"
#include "World/Combat.h"

enum class eDummyKind : uint8_t
{
    Grunt,
    Jeep,
    Bus
};

inline constexpr uint32_t kDummyOwnerBase = 1000;
inline constexpr uint8_t  kEnemyBulletTag = 100;

struct TrainingDummy
{
    uint32_t   id         = kNoOwner;
    eDummyKind kind       = eDummyKind::Grunt;
    bool       bShooter   = false;
    Vec2       feet       = { 0.f, 0.f };
    Vec2       size       = { 20.f, 44.f };
    int        facing     = -1;
    Health     health     = {};
    int        flashTicks = 0;
    int        deadTicks  = -1;   // -1: 생존, 0..: 사망 후 경과 틱
    int        shootTimer = 0;
    int        hitsTaken  = 0;
    eHitKind   lastKind   = eHitKind::Bullet;
    AnimPlayer anim       = {};

    [[nodiscard]] bool IsAlive() const;
};

class TrainingTargets
{
public:
    void Clear();

    uint32_t Add(eDummyKind _kind, Vec2 _feet, bool _bShooter, const SandboxTuning& _tuning);

    void RegisterHurtboxes(CombatSystem& _combat) const;

    void TickShooters(const SandboxTuning& _tuning, Vec2 _playerFeet, bool _bPlayerAlive, ProjectileSystem& _projectiles, CombatSystem& _combat);

    int ApplyEvents(Span<const HitEvent> _events, const SandboxTuning& _tuning);

    void Tick(const SandboxTuning& _tuning);

    [[nodiscard]] Span<const TrainingDummy> GetDummies() const;
    [[nodiscard]] Span<TrainingDummy>       GetDummies();
    [[nodiscard]] const TrainingDummy*      Find(uint32_t _id) const;
    [[nodiscard]] int                       GetKillCount() const;
    [[nodiscard]] uint64_t                  HashState() const;

private:
    void Revive_(TrainingDummy& _dummy, const SandboxTuning& _tuning) const;

    Vector<TrainingDummy> m_dummies   = {};
    uint32_t              m_nextId    = kDummyOwnerBase;
    int                   m_killCount = 0;
};

[[nodiscard]] Vec2        GetDummySize(eDummyKind _kind);
[[nodiscard]] int         GetDummyHp(eDummyKind _kind, const SandboxTuning& _tuning);
[[nodiscard]] const char* GetDummyName(eDummyKind _kind);
