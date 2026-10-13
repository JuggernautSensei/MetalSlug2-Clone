#include <algorithm>

#include "Base/Math.h"
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

void AddBombHitbox_(
    const Enemy&   _e,
    const VECTOR2  _size,
    const int      _damage,
    const eHitKind _kind,
    EnemyEnv&      _env)
{
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

void Init_(
    Enemy&          _e,
    const EnemyDef& _def,
    EnemyEnv&       _env)
{
    PrepareBody(_e, _def);
    _e.body.bUseGravity = false;
    _e.body.velocity    = { 0.f, _def.heliBomb.startVy };
    _e.meleeHitId       = _env.ctx.pCombat ? _env.ctx.pCombat->NewHitId() : kNoHitId;
    SetClip(_e, _def.heliBomb.clipFall, true);
}

void Tick_(
    Enemy&          _e,
    const EnemyDef& _def,
    EnemyEnv&       _env)
{
    const HeliBombCfg& c = _def.heliBomb;
    if (!_e.bActive)
    {
        return;
    }

    _e.body.velocity.y = Min(_e.body.velocity.y + c.gravity * kStepSec, c.maxFall);
    _e.body.position.y += _e.body.velocity.y * kStepSec;

    const VECTOR2 probeTo = { _e.body.position.x, _e.body.position.y + c.groundProbeY };
    // 플레이어 바로 위에서 떨어지는 폭탄은 그 머리 위 구조물에 막히지 않고 플레이어 발 높이까지 내려와 터진다
    const EnemyPlayerInfo& player   = _env.ctx.player;
    const bool             bAimed   = player.bAlive && Abs(probeTo.x - player.feet.x) <= c.hitSize.x * 0.5f + c.blastSize.x * 0.5f;
    const bool             bPassing = bAimed && probeTo.y < player.feet.y;
    const bool             bArrived = bAimed && probeTo.y >= player.feet.y;
    if (bArrived || (!bPassing && Touches_(_env.ctx.physics, probeTo)))
    {
        RequestFx(_env, FxDef {
                            c.groundFx, { 0.f, c.groundProbeY + c.groundFxDy },
                             0.f, 57
        },
                  _e.body.position,
                  _e.facing);
        (void)_e.health.ApplyDamage(_e.health.hp);
        return;
    }
    AddBombHitbox_(_e, c.hitSize, c.blastDamage, eHitKind::Bullet, _env);
}

void OnDeath_(
    Enemy&          _e,
    const EnemyDef& _def,
    EnemyEnv&       _env)
{
    BeginVehicleDeath(_e, _def, _env, -1);
    _e.bVisible = false;
    _e.aux      = 0;
}

void TickDying_(
    Enemy&          _e,
    const EnemyDef& _def,
    EnemyEnv&       _env)
{
    const HeliBombCfg& c = _def.heliBomb;
    if (_e.aux < c.blastTicks)
    {
        AddBombHitbox_(_e, c.blastSize, c.blastDamage, eHitKind::Blast, _env);
        ++_e.aux;
        return;
    }
    if (_e.meleeHitId != kNoHitId && _env.ctx.pCombat)
    {
        _env.ctx.pCombat->ReleaseHitId(_e.meleeHitId);
    }
    _e.meleeHitId = kNoHitId;
    _e.life       = eEnemyLife::Gone;
}
}   // namespace

const BehaviorOps kHeliBombOps = {
    .init      = &Init_,
    .tick      = &Tick_,
    .onDeath   = &OnDeath_,
    .tickDying = &TickDying_,
};

void ParseHeliBombCfg(
    DefReader& _r,
    EnemyDef&  _def)
{
    HeliBombCfg& c = _def.heliBomb;

    c.startVy      = _r.Num("startVy", c.startVy);
    c.gravity      = _r.Num("gravity", c.gravity);
    c.maxFall      = _r.Num("maxFall", c.maxFall);
    c.hitSize      = _r.ReadVec2("hitSize", c.hitSize);
    c.blastSize    = _r.ReadVec2("blastSize", c.blastSize);
    c.blastTicks   = _r.Ticks("blastSec", 0.13f);
    c.blastDamage  = _r.Int("blastDamage", c.blastDamage);
    c.groundProbeY = _r.Num("groundProbeY", c.groundProbeY);
    c.groundFx     = _r.Str("groundFx");
    c.groundFxDy   = _r.Num("groundFxDy", c.groundFxDy);

    c.clipFall = _r.Clip("fall");
}
