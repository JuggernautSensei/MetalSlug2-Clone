#include "Game/Enemies/HeliBombEnemy.h"

#include <algorithm>

#include "Game/Enemies/EnemyCommon.h"
#include "Game/Enemies/EnemyDefParse.h"
#include "Game/Enemies/VehicleCommon.h"

namespace
{
[[nodiscard]] bool Touches_(
    const PhysicsWorld& _world,
    const VECTOR2       _to)
{
    if (_world.pGround && _world.pGround->IsSolidAt(_to))
    {
        return true;
    }
    for (const Wall& wall: _world.walls)
    {
        if (wall.box.Contains(_to))
        {
            return true;
        }
    }
    return false;
}

}   // namespace

void HeliBombEnemy::Init(
    const EnemyDef& _def,
    EnemyEnv&       _env)
{
    EnemyState& _e = GetMutableState();
    PrepareBody(_e, _def);
    _e.body.bUseGravity = false;
    _e.body.velocity    = { 0.f, _def.heliBomb.startVy };
    _e.meleeHitId       = _env.ctx.pCombat ? _env.ctx.pCombat->NewHitId() : kNoHitId;
    SetClip(_e, _def.heliBomb.clipFall, true);
}

void HeliBombEnemy::Tick(
    const EnemyDef& _def,
    EnemyEnv&       _env)
{
    EnemyState&           _e     = GetMutableState();
    const HeliBombConfig& config = _def.heliBomb;
    if (!_e.bActive)
    {
        return;
    }

    _e.body.velocity.y = Min(_e.body.velocity.y + config.gravity * kStepSec, config.maxFall);
    _e.body.position.y += _e.body.velocity.y * kStepSec;

    const VECTOR2 probeTo = { _e.body.position.x, _e.body.position.y + config.groundProbeY };
    // 플레이어 바로 위에서 떨어지는 폭탄은 그 머리 위 구조물에 막히지 않고 플레이어 발 높이까지 내려와 터진다
    const EnemyPlayerInfo& player   = _env.ctx.player;
    const bool             bAimed   = player.bAlive && Abs(probeTo.x - player.feet.x) <= config.hitSize.x * 0.5f + config.blastSize.x * 0.5f;
    const bool             bPassing = bAimed && probeTo.y < player.feet.y;
    const bool             bArrived = bAimed && probeTo.y >= player.feet.y;
    if (bArrived || (!bPassing && Touches_(_env.ctx.physics, probeTo)))
    {
        RequestFx(_env, FxDef {
                            config.groundFx, { 0.f, config.groundProbeY + config.groundFxDy },
                             0.f, 57
        },
                  _e.body.position,
                  _e.facing);
        (void)_e.health.ApplyDamage(_e.health.hp);
        return;
    }
    AddBombHitbox_(config.hitSize, config.blastDamage, eHitKind::Bullet, _env);
}

void HeliBombEnemy::OnDeath(
    const EnemyDef& _def,
    EnemyEnv&       _env)
{
    EnemyState& _e = GetMutableState();
    BeginVehicleDeath(_e, _def, _env, -1);
    _e.bVisible  = false;
    m_blastTicks = 0;
}

void HeliBombEnemy::TickDying(
    const EnemyDef& _def,
    EnemyEnv&       _env)
{
    EnemyState&           _e     = GetMutableState();
    const HeliBombConfig& config = _def.heliBomb;
    if (m_blastTicks < config.blastTicks)
    {
        AddBombHitbox_(config.blastSize, config.blastDamage, eHitKind::Blast, _env);
        ++m_blastTicks;
        return;
    }
    if (_e.meleeHitId != kNoHitId && _env.ctx.pCombat)
    {
        _env.ctx.pCombat->ReleaseHitId(_e.meleeHitId);
    }
    _e.meleeHitId = kNoHitId;
    _e.life       = eEnemyLife::Gone;
}

void ParseHeliBombConfig(
    DefReader& _r,
    EnemyDef&  _def)
{
    HeliBombConfig& config = _def.heliBomb;

    config.startVy      = _r.Num("startVy", config.startVy);
    config.gravity      = _r.Num("gravity", config.gravity);
    config.maxFall      = _r.Num("maxFall", config.maxFall);
    config.hitSize      = _r.ReadVec2("hitSize", config.hitSize);
    config.blastSize    = _r.ReadVec2("blastSize", config.blastSize);
    config.blastTicks   = _r.Ticks("blastSec", 0.13f);
    config.blastDamage  = _r.Int("blastDamage", config.blastDamage);
    config.groundProbeY = _r.Num("groundProbeY", config.groundProbeY);
    config.groundFx     = _r.Str("groundFx");
    config.groundFxDy   = _r.Num("groundFxDy", config.groundFxDy);

    config.clipFall = _r.Clip("fall");
}

void HeliBombEnemy::AddBombHitbox_(
    const VECTOR2  _size,
    const int      _damage,
    const eHitKind _kind,
    EnemyEnv&      _env) const
{
    const EnemyState& _e = GetState();
    if (!_env.ctx.pCombat || _e.meleeHitId == kNoHitId)
    {
        return;
    }
    Hitbox hitbox;
    hitbox.box     = Aabb::FromCenterSize(_e.body.position, _size);
    hitbox.team    = eTeam::Enemy;
    hitbox.kind    = _kind;
    hitbox.damage  = _damage;
    hitbox.bPierce = false;
    hitbox.owner   = _e.id;
    hitbox.id      = _e.meleeHitId;
    _env.ctx.pCombat->AddHitbox(hitbox);
}

void HeliBombEnemy::HashActionState(Fnv1a& _hash) const
{
    HashActionState_(_hash, 0, m_blastTicks);
}
