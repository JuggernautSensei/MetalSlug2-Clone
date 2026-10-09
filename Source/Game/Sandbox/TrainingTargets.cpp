#include "Game/Sandbox/TrainingTargets.h"

#include <cmath>
#include <cstring>

namespace slug
{

jug::VECTOR2 GetDummySize(
    const eDummyKind _kind)
{
    switch (_kind)
    {
        case eDummyKind::Jeep: return { 90.f, 50.f };
        case eDummyKind::Bus: return { 170.f, 90.f };
        default: return { 20.f, 56.f };
    }
}

int GetDummyHp(
    const eDummyKind     _kind,
    const SandboxTuning& _tuning)
{
    switch (_kind)
    {
        case eDummyKind::Jeep: return _tuning.jeepHp;
        case eDummyKind::Bus: return _tuning.busHp;
        default: return _tuning.gruntHp;
    }
}

const char* GetDummyName(
    const eDummyKind _kind)
{
    switch (_kind)
    {
        case eDummyKind::Jeep: return "JEEP";
        case eDummyKind::Bus: return "BUS";
        default: return "GRUNT";
    }
}

void TrainingTargets::Clear()
{
    m_dummies.clear();
    m_nextId    = kDummyOwnerBase;
    m_killCount = 0;
}

uint32_t TrainingTargets::Add(
    const eDummyKind     _kind,
    const jug::VECTOR2   _feet,
    const bool           _bShooter,
    const SandboxTuning& _tuning)
{
    TrainingDummy dummy;
    dummy.id         = ++m_nextId;
    dummy.kind       = _kind;
    dummy.bShooter   = _bShooter;
    dummy.feet       = _feet;
    dummy.size       = GetDummySize(_kind);
    dummy.health     = Health::Make(GetDummyHp(_kind, _tuning));
    dummy.shootTimer = _tuning.shooterIntervalTicks;
    m_dummies.push_back(dummy);
    return dummy.id;
}

const TrainingDummy* TrainingTargets::Find(
    const uint32_t _id) const
{
    for (const TrainingDummy& dummy: m_dummies)
    {
        if (dummy.id == _id)
        {
            return &dummy;
        }
    }
    return nullptr;
}

void TrainingTargets::RegisterHurtboxes(
    CombatSystem& _combat) const
{
    for (const TrainingDummy& dummy: m_dummies)
    {
        if (!dummy.IsAlive())
        {
            continue;
        }
        Hurtbox hurtbox;
        hurtbox.box   = Aabb::FromBottomCenter(dummy.feet, dummy.size);
        hurtbox.team  = eTeam::Enemy;
        hurtbox.owner = dummy.id;
        _combat.AddHurtbox(hurtbox);
    }
}

void TrainingTargets::TickShooters(
    const SandboxTuning& _tuning,
    const jug::VECTOR2   _playerFeet,
    const bool           _bPlayerAlive,
    ProjectileSystem&    _projectiles,
    CombatSystem&        _combat)
{
    for (TrainingDummy& dummy: m_dummies)
    {
        if (!dummy.bShooter || !dummy.IsAlive())
        {
            continue;
        }

        const float dx = _playerFeet.x - dummy.feet.x;
        dummy.facing   = dx < 0.f ? -1 : 1;
        if (!_bPlayerAlive || std::abs(dx) > _tuning.shooterRange)
        {
            dummy.shootTimer = _tuning.shooterIntervalTicks;
            continue;
        }

        if (--dummy.shootTimer > 0)
        {
            continue;
        }
        dummy.shootTimer = _tuning.shooterIntervalTicks;

        BulletSpawn spawn;
        spawn.team      = eTeam::Enemy;
        spawn.owner     = dummy.id;
        spawn.position  = { dummy.feet.x + static_cast<float>(dummy.facing) * 16.f, dummy.feet.y - 30.f };
        spawn.sweepFrom = { dummy.feet.x, dummy.feet.y - 30.f };
        spawn.direction = { static_cast<float>(dummy.facing), 0.f };
        spawn.speed     = _tuning.enemyBulletSpeed;
        spawn.damage    = _tuning.enemyBulletDamage;
        spawn.size      = { 10.f, 8.f };
        spawn.range     = _tuning.shooterRange + 160.f;
        spawn.bPierce   = false;
        spawn.tag       = kEnemyBulletTag;
        spawn.kind      = eHitKind::Bullet;
        _projectiles.SpawnBullet(spawn, _combat);
    }
}

int TrainingTargets::ApplyEvents(
    const jug::Span<const HitEvent> _events,
    const SandboxTuning&            _tuning)
{
    (void)_tuning;
    int score = 0;
    for (const HitEvent& event: _events)
    {
        if (event.target < kDummyOwnerBase)
        {
            continue;
        }
        for (TrainingDummy& dummy: m_dummies)
        {
            if (dummy.id != event.target || !dummy.IsAlive())
            {
                continue;
            }

            // 모든 표적이 같은 경로: Health 에 피해만 적용한다 (종류/무기별 분기 없음)
            const int applied = dummy.health.ApplyDamage(event.damage);
            if (applied > 0)
            {
                ++dummy.hitsTaken;
                dummy.lastDamage = applied;
                dummy.lastKind   = event.kind;
                dummy.flashTicks = 5;
            }
            if (dummy.health.IsDead() && dummy.deadTicks < 0)
            {
                dummy.deadTicks = 0;
                ++m_killCount;
                score += dummy.kind == eDummyKind::Bus ? 1000 : (dummy.kind == eDummyKind::Jeep ? 500 : 100);
            }
            break;
        }
    }
    return score;
}

void TrainingTargets::Revive_(
    TrainingDummy&       _dummy,
    const SandboxTuning& _tuning) const
{
    _dummy.health     = Health::Make(GetDummyHp(_dummy.kind, _tuning));
    _dummy.deadTicks  = -1;
    _dummy.flashTicks = 0;
    _dummy.shootTimer = _tuning.shooterIntervalTicks;
    _dummy.anim       = {};
}

void TrainingTargets::Tick(
    const SandboxTuning& _tuning)
{
    for (TrainingDummy& dummy: m_dummies)
    {
        if (dummy.flashTicks > 0)
        {
            --dummy.flashTicks;
        }
        if (dummy.deadTicks >= 0)
        {
            ++dummy.deadTicks;
            if (dummy.deadTicks >= _tuning.dummyRespawnTicks)
            {
                Revive_(dummy, _tuning);
            }
        }
    }
}

uint64_t TrainingTargets::HashState() const
{
    uint64_t   hash = 14695981039346656037ull;
    const auto Mix  = [&hash](const int _value) {
        unsigned char bytes[sizeof(int)];
        std::memcpy(bytes, &_value, sizeof(int));
        for (const unsigned char byte: bytes)
        {
            hash ^= byte;
            hash *= 1099511628211ull;
        }
    };
    for (const TrainingDummy& dummy: m_dummies)
    {
        Mix(static_cast<int>(dummy.id));
        Mix(dummy.health.hp);
        Mix(dummy.deadTicks);
        Mix(dummy.hitsTaken);
    }
    Mix(m_killCount);
    return hash;
}

}   // namespace slug
