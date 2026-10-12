#include <cmath>

#include "Game/Enemies/EnemyCommon.h"
#include "Game/Enemies/EnemyDefParse.h"
#include "Game/Enemies/VehicleCommon.h"

enum class eSarubiaState
{
    Idle,
    Turn,
    MoveStart,
    Move,
    MoveStop,
    ShotStart,
    Shot
};

namespace
{
[[nodiscard]] int ClipOf_(
    const Enemy&      _e,
    const SarubiaCfg& _c)
{
    switch (static_cast<eSarubiaState>(_e.state))
    {
        case eSarubiaState::Turn: return _c.clipTurn;
        case eSarubiaState::MoveStart: return _c.clipMoveStart;
        case eSarubiaState::Move: return _c.clipMove;
        case eSarubiaState::MoveStop: return _c.clipMoveStop;
        case eSarubiaState::ShotStart: return _c.clipShotStart;
        case eSarubiaState::Shot: return _c.clipShot;
        default: return _c.clipIdle;
    }
}

void Go_(
    Enemy&              _e,
    const EnemyDef&     _def,
    const eSarubiaState _state)
{
    EnterState(_e, static_cast<int>(_state));
    SetClip(_e, ClipOf_(_e, _def.sarubia), true);
}

void Init_(
    Enemy&          _e,
    const EnemyDef& _def,
    EnemyEnv&       _env)
{
    (void)_env;
    PrepareBody(_e, _def);
    Go_(_e, _def, eSarubiaState::Idle);
}

void Tick_(
    Enemy&          _e,
    const EnemyDef& _def,
    EnemyEnv&       _env)
{
    const SarubiaCfg& c = _def.sarubia;
    _e.body.velocity.x  = 0.f;

    if (!_e.bActive)
    {
        (void)StepPhysics(_e, _def, _env);
        return;
    }

    const float dx      = DxToPlayer(_e, _env);
    const float absDx   = std::abs(dx);
    const bool  bBehind = dx * static_cast<float>(_e.facing) < 0.f;

    switch (static_cast<eSarubiaState>(_e.state))
    {
        case eSarubiaState::Idle:
            if (_e.stateTicks >= c.cooldownTicks)
            {
                if (bBehind)
                {
                    Go_(_e, _def, eSarubiaState::Turn);
                }
                else if (absDx > c.chaseDx)
                {
                    Go_(_e, _def, eSarubiaState::MoveStart);
                }
                else
                {
                    Go_(_e, _def, eSarubiaState::ShotStart);
                }
            }
            break;

        case eSarubiaState::Turn:
            if (IsClipDone(_e, _def))
            {
                _e.facing = -_e.facing;
                Go_(_e, _def, eSarubiaState::Idle);
            }
            break;

        case eSarubiaState::MoveStart:
            if (IsClipDone(_e, _def))
            {
                Go_(_e, _def, eSarubiaState::Move);
            }
            break;

        case eSarubiaState::Move:
            _e.body.velocity.x = static_cast<float>(_e.facing) * _def.physics.speed;
            if (_e.stateTicks >= c.moveMaxTicks || absDx <= c.stopDx)
            {
                Go_(_e, _def, eSarubiaState::MoveStop);
            }
            break;

        case eSarubiaState::MoveStop:
            if (IsClipDone(_e, _def))
            {
                Go_(_e, _def, eSarubiaState::Idle);
            }
            break;

        case eSarubiaState::ShotStart:
            if (_e.stateTicks >= c.shotReadyTicks)
            {
                Go_(_e, _def, eSarubiaState::Shot);
            }
            break;

        case eSarubiaState::Shot:
            if (ClipReachedFrame(_e, _def, c.fireFrame))
            {
                (void)FireWeapon(_e, _def, _env, c.weaponGrenade);
            }
            if (IsClipDone(_e, _def))
            {
                Go_(_e, _def, eSarubiaState::Idle);
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
    BeginVehicleDeath(_e, _def, _env, _def.sarubia.clipDestroy);
}
}   // namespace

const BehaviorOps kSarubiaOps = {
    .init      = &Init_,
    .tick      = &Tick_,
    .onDeath   = &OnDeath_,
    .tickDying = &TickVehicleDeath,
};

void ParseSarubiaCfg(
    DefReader& _r,
    EnemyDef&  _def)
{
    SarubiaCfg& c = _def.sarubia;

    c.cooldownTicks  = _r.Ticks("cooldownSec");
    c.chaseDx        = _r.Num("chaseDx");
    c.stopDx         = _r.Num("stopDx");
    c.moveMaxTicks   = _r.Ticks("moveMaxSec");
    c.shotReadyTicks = _r.Ticks("shotReadySec");
    c.fireFrame      = _r.Int("fireFrame");

    c.clipIdle      = _r.Clip("idle");
    c.clipTurn      = _r.Clip("turn");
    c.clipMoveStart = _r.Clip("moveStart");
    c.clipMove      = _r.Clip("move");
    c.clipMoveStop  = _r.Clip("moveStop");
    c.clipShotStart = _r.Clip("shotStart");
    c.clipShot      = _r.Clip("shot");
    c.clipDestroy   = _r.Clip("destroy");

    c.weaponGrenade = _r.Weapon("grenade");
}
