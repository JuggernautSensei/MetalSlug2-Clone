#include "Game/Enemies/Enemy.h"
#include "World/Combat.h"
#include "Game/Enemies/BigShieeEnemy.h"
#include "Game/Enemies/BusEnemy.h"
#include "Game/Enemies/BzkEnemy.h"
#include "Game/Enemies/FlyingBzkEnemy.h"
#include "Game/Enemies/HangEnemy.h"
#include "Game/Enemies/HeliEnemy.h"
#include "Game/Enemies/HeliBombEnemy.h"
#include "Game/Enemies/PowEnemy.h"
#include "Game/Enemies/ShielderEnemy.h"
#include "Game/Enemies/SoldierEnemy.h"
#include "Game/Enemies/BTankEnemy.h"
#include "Game/Enemies/JeepEnemy.h"
#include "Game/Enemies/MissileTankEnemy.h"
#include "Game/Enemies/SarubiaEnemy.h"

bool Enemy::OnHit(const EnemyDef&, EnemyEnv&, const HitEvent&)
{
    return false;
}

void Enemy::OnDeath(const EnemyDef&, EnemyEnv&)
{
}

void Enemy::TickDying(const EnemyDef&, EnemyEnv&)
{
    m_state.life = eEnemyLife::Gone;
}

void Enemy::RegisterHurtboxes(const EnemyDef& _def, CombatSystem& _combat) const
{
    Hurtbox hurtbox;
    hurtbox.box     = _def.hurtbox.At(m_state.body.position, m_state.facing);
    hurtbox.team    = _def.team;
    hurtbox.owner   = m_state.id;
    hurtbox.accepts = AcceptsFor(_def.bHuman);
    _combat.AddHurtbox(hurtbox);
}

void Enemy::SubmitParts(const EnemyDef&, const EnemyRenderEnv&) const
{
}

bool Enemy::DeathDone(const EnemyDef&) const
{
    return false;
}

const AnimFrame* Enemy::BodyFrame(const EnemyDef&, const AnimationLibrary&) const
{
    return nullptr;
}

Scoped<Enemy> MakeEnemy(const eEnemyBehavior _behavior)
{
    switch (_behavior)
    {
        case eEnemyBehavior::BigShiee: return MakeScoped<BigShieeEnemy>();
        case eEnemyBehavior::Bus: return MakeScoped<BusEnemy>();
        case eEnemyBehavior::Bzk: return MakeScoped<BzkEnemy>();
        case eEnemyBehavior::FlyingBzk: return MakeScoped<FlyingBzkEnemy>();
        case eEnemyBehavior::Hang: return MakeScoped<HangEnemy>();
        case eEnemyBehavior::Heli: return MakeScoped<HeliEnemy>();
        case eEnemyBehavior::HeliBomb: return MakeScoped<HeliBombEnemy>();
        case eEnemyBehavior::Pow: return MakeScoped<PowEnemy>();
        case eEnemyBehavior::Shielder: return MakeScoped<ShielderEnemy>();
        case eEnemyBehavior::Soldier: return MakeScoped<SoldierEnemy>();
        case eEnemyBehavior::BTank: return MakeScoped<BTankEnemy>();
        case eEnemyBehavior::Jeep: return MakeScoped<JeepEnemy>();
        case eEnemyBehavior::MissileTank: return MakeScoped<MissileTankEnemy>();
        case eEnemyBehavior::Sarubia: return MakeScoped<SarubiaEnemy>();
        case eEnemyBehavior::Count: break;
    }
    JUG_ASSERT(false, "Invalid enemy behavior.\n");
    return nullptr;
}

void Enemy::HashActionState(Fnv1a& _hash) const
{
    HashActionState_(_hash, 0, 0);
}

void Enemy::HashActionState_(Fnv1a& _hash, const int _shots, const int _progress, const int _secondary, const int _tertiary, const int _quaternary) const
{
    // 기존 체크포인트 형식은 진행 값 다섯 개를 64비트 정수로 기록한다.
    const int               progress = m_state.deathStart == eDeathStart::Blast    ? m_state.deathLaunchY
                                     : m_state.deathStart == eDeathStart::Standing ? m_state.deathFallFrame
                                                                                   : _progress;
    const ARRAY<int64_t, 5> values   = { _shots, progress, _secondary, _tertiary, _quaternary };
    _hash.Mix(values);
}
