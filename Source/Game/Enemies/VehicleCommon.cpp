#include "Game/Enemies/VehicleCommon.h"

#include "Base/Types.h"

void BeginVehicleDeath(
    EnemyState&     _e,
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
    EnemyState&     _e,
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
    const VECTOR2    _at,
    const int        _facing,
    const int        _depth,
    const float      _delaySec)
{
    FxDef fx;
    fx.clip     = String(_clip);
    fx.offset   = { 0.f, 0.f };
    fx.delaySec = _delaySec;
    fx.depth    = _depth;
    RequestFx(_env, fx, _at, _facing);
}

bool IsClipDone(
    const EnemyState& _e,
    const EnemyDef&   _def)
{
    return _e.clip >= 0 && _e.stateTicks >= _def.GetClip(_e.clip).GetDurationTicks();
}
