#include "Game/Enemies/VehicleCommon.h"

#include "Base/Math.h"
#include "Base/Types.h"

namespace slug
{

void BeginVehicleDeath(
    Enemy&          _e,
    const EnemyDef& _def,
    EnemyEnv&       _env,
    const int       _destroyClip)
{
    _e.body.velocity.x = 0.f;
    _e.deathTicks      = 0;
    if (_destroyClip >= 0)
    {
        SetClip(_e, _destroyClip, true);
    }
    for (const FxDef& fx: _def.death.fx)
    {
        RequestFx(_env, fx, _e.body.position, _e.facing);
    }
}

void TickVehicleDeath(
    Enemy&          _e,
    const EnemyDef& _def,
    EnemyEnv&       _env)
{
    (void)_env;
    _e.body.velocity.x = 0.f;
    ++_e.deathTicks;
    if (_e.deathTicks >= SecToTicks(_def.death.removeAtSec))
    {
        _e.life = eEnemyLife::Gone;
    }
}

void RequestFxAt(
    EnemyEnv&        _env,
    const StringView _clip,
    const Vec2       _at,
    const int        _facing,
    const int        _layer,
    const float      _delaySec)
{
    FxDef fx;
    fx.clip     = String(_clip);
    fx.offset   = { 0.f, 0.f };
    fx.delaySec = _delaySec;
    fx.layer    = _layer;
    RequestFx(_env, fx, _at, _facing);
}

Vec2 ScreenToWorld(
    const EnemyEnv& _env,
    const Vec2      _screen)
{
    const Aabb& view = _env.ctx.cameraView;
    return { view.min.x + _screen.x, view.min.y + _screen.y };
}

}   // namespace slug
