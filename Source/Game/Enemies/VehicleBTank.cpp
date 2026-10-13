#include <algorithm>
#include <cmath>

#include "Game/Enemies/EnemyCommon.h"
#include "Game/Enemies/EnemyDefParse.h"
#include "Game/Enemies/VehicleCommon.h"

enum class eBTankState
{
    Closed,
    Open,
    Idle,
    Fire
};

namespace
{
[[nodiscard]] int ClipOf_(
    const Enemy&    _e,
    const BTankCfg& _c)
{
    switch (static_cast<eBTankState>(_e.state))
    {
        case eBTankState::Open:
        case eBTankState::Idle: return _c.clipOpen;
        case eBTankState::Fire: return _c.clipFire;
        default: return _c.clipIdle;
    }
}

void Go_(
    Enemy&          _e,
    const EnemyDef& _def,
    eBTankState     _state)
{
    const BTankCfg& c = _def.btank;
    EnterState(_e, static_cast<int>(_state));
    const int clip = ClipOf_(_e, c);
    SetClip(_e, clip, _state != eBTankState::Idle);
    if (_state == eBTankState::Idle && clip >= 0)
    {
        _e.clipTicks = Max(_e.clipTicks, _def.GetClip(clip).GetDurationTicks());
    }
}

void Init_(
    Enemy&          _e,
    const EnemyDef& _def,
    EnemyEnv&       _env)
{
    (void)_env;
    PrepareBody(_e, _def);
    _e.facing = -1;
    Go_(_e, _def, eBTankState::Closed);
}

void Tick_(
    Enemy&          _e,
    const EnemyDef& _def,
    EnemyEnv&       _env)
{
    const BTankCfg& c  = _def.btank;
    _e.body.velocity.x = 0.f;

    if (!_e.bActive)
    {
        if (_env.ctx.player.feet.x >= _e.body.position.x - c.activateDx)
        {
            _e.bActive = true;
        }
        else
        {
            (void)StepPhysics(_e, _def, _env);
            return;
        }
    }

    switch (static_cast<eBTankState>(_e.state))
    {
        case eBTankState::Closed: Go_(_e, _def, eBTankState::Open); break;

        case eBTankState::Open:
            if (_e.clip >= 0 && _e.stateTicks >= _def.GetClip(_e.clip).GetDurationTicks())
            {
                Go_(_e, _def, eBTankState::Idle);
            }
            break;

        case eBTankState::Idle:
            if (_e.stateTicks >= c.idleTicks)
            {
                _e.counter = c.shotsPerCycle;
                Go_(_e, _def, eBTankState::Fire);
            }
            break;

        case eBTankState::Fire:
            if (ClipReachedFrame(_e, _def, c.fireFrame))
            {
                (void)FireWeapon(_e, _def, _env, c.weaponShell);
            }
            if (_e.clip >= 0 && _e.stateTicks >= _def.GetClip(_e.clip).GetDurationTicks())
            {
                --_e.counter;
                if (_e.counter > 0)
                {
                    Go_(_e, _def, eBTankState::Fire);
                }
                else
                {
                    Go_(_e, _def, eBTankState::Idle);
                }
            }
            break;

        default: break;
    }

    (void)StepPhysics(_e, _def, _env);
}

void OnDeath_(
    Enemy&          _e,
    const EnemyDef& _def,
    EnemyEnv&       _env)
{
    BeginVehicleDeath(_e, _def, _env, _def.btank.clipDestroy);
}
}   // namespace

const BehaviorOps kBTankOps = {
    .init      = &Init_,
    .tick      = &Tick_,
    .onDeath   = &OnDeath_,
    .tickDying = &TickVehicleDeath,
};

void ParseBTankCfg(
    DefReader& _r,
    EnemyDef&  _def)
{
    BTankCfg& c = _def.btank;

    c.activateDx    = _r.Num("activateDx");
    c.idleTicks     = _r.Ticks("idleSec");
    c.shotsPerCycle = _r.Int("shotsPerCycle");
    c.fireFrame     = _r.Int("fireFrame");

    c.clipIdle    = _r.Clip("idle");
    c.clipOpen    = _r.Clip("open");
    c.clipFire    = _r.Clip("fire");
    c.clipDestroy = _r.Clip("destroy");

    c.weaponShell = _r.Weapon("shell");
}
