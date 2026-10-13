#include "Game/Enemies/SarubiaEnemy.h"

#include <cmath>

#include "Game/Enemies/EnemyCommon.h"
#include "Game/Enemies/EnemyDefParse.h"
#include "Game/Enemies/VehicleCommon.h"

void SarubiaEnemy::Init(
    const EnemyDef& _def,
    EnemyEnv&       _env)
{
    EnemyState& _e = GetMutableState();
    (void)_env;
    PrepareBody(_e, _def);
    Go_(_def, eSarubiaState::Idle);
}

void SarubiaEnemy::Tick(
    const EnemyDef& _def,
    EnemyEnv&       _env)
{
    EnemyState&          _e     = GetMutableState();
    const SarubiaConfig& config = _def.sarubia;
    _e.body.velocity.x          = 0.f;

    if (!_e.bActive)
    {
        (void)StepPhysics(_e, _def, _env);
        return;
    }

    const float dx      = DxToPlayer(_e, _env);
    const float absDx   = Abs(dx);
    const bool  bBehind = dx * static_cast<float>(_e.facing) < 0.f;

    switch (static_cast<eSarubiaState>(_e.state))
    {
        case eSarubiaState::Idle:
            if (_e.stateTicks >= config.cooldownTicks)
            {
                if (bBehind)
                {
                    Go_(_def, eSarubiaState::Turn);
                }
                else if (absDx > config.chaseDx)
                {
                    Go_(_def, eSarubiaState::MoveStart);
                }
                else
                {
                    Go_(_def, eSarubiaState::ShotStart);
                }
            }
            break;

        case eSarubiaState::Turn:
            if (IsClipDone(_e, _def))
            {
                _e.facing = -_e.facing;
                Go_(_def, eSarubiaState::Idle);
            }
            break;

        case eSarubiaState::MoveStart:
            if (IsClipDone(_e, _def))
            {
                Go_(_def, eSarubiaState::Move);
            }
            break;

        case eSarubiaState::Move:
            _e.body.velocity.x = static_cast<float>(_e.facing) * _def.physics.speed;
            if (_e.stateTicks >= config.moveMaxTicks || absDx <= config.stopDx)
            {
                Go_(_def, eSarubiaState::MoveStop);
            }
            break;

        case eSarubiaState::MoveStop:
            if (IsClipDone(_e, _def))
            {
                Go_(_def, eSarubiaState::Idle);
            }
            break;

        case eSarubiaState::ShotStart:
            if (_e.stateTicks >= config.shotReadyTicks)
            {
                Go_(_def, eSarubiaState::Shot);
            }
            break;

        case eSarubiaState::Shot:
            if (ClipReachedFrame(_e, _def, config.fireFrame))
            {
                (void)FireWeapon(_e, _def, _env, config.weaponGrenade);
            }
            if (IsClipDone(_e, _def))
            {
                Go_(_def, eSarubiaState::Idle);
            }
            break;

        default: break;
    }

    (void)StepPhysics(_e, _def, _env);
}

void SarubiaEnemy::OnDeath(
    const EnemyDef& _def,
    EnemyEnv&       _env)
{
    EnemyState& _e = GetMutableState();
    BeginVehicleDeath(_e, _def, _env, _def.sarubia.clipDestroy);
}

void SarubiaEnemy::TickDying(
    const EnemyDef& _def,
    EnemyEnv&       _env)
{
    TickVehicleDeath(GetMutableState(), _def, _env);
}

void ParseSarubiaConfig(
    DefReader& _r,
    EnemyDef&  _def)
{
    SarubiaConfig& config = _def.sarubia;

    config.cooldownTicks  = _r.Ticks("cooldownSec");
    config.chaseDx        = _r.Num("chaseDx");
    config.stopDx         = _r.Num("stopDx");
    config.moveMaxTicks   = _r.Ticks("moveMaxSec");
    config.shotReadyTicks = _r.Ticks("shotReadySec");
    config.fireFrame      = _r.Int("fireFrame");

    config.clipIdle      = _r.Clip("idle");
    config.clipTurn      = _r.Clip("turn");
    config.clipMoveStart = _r.Clip("moveStart");
    config.clipMove      = _r.Clip("move");
    config.clipMoveStop  = _r.Clip("moveStop");
    config.clipShotStart = _r.Clip("shotStart");
    config.clipShot      = _r.Clip("shot");
    config.clipDestroy   = _r.Clip("destroy");

    config.weaponGrenade = _r.Weapon("grenade");
}

[[nodiscard]] int SarubiaEnemy::ClipOf_(
    const SarubiaConfig& _config) const
{
    const EnemyState& _e = GetState();
    switch (static_cast<eSarubiaState>(_e.state))
    {
        case eSarubiaState::Turn: return _config.clipTurn;
        case eSarubiaState::MoveStart: return _config.clipMoveStart;
        case eSarubiaState::Move: return _config.clipMove;
        case eSarubiaState::MoveStop: return _config.clipMoveStop;
        case eSarubiaState::ShotStart: return _config.clipShotStart;
        case eSarubiaState::Shot: return _config.clipShot;
        default: return _config.clipIdle;
    }
}

void SarubiaEnemy::Go_(
    const EnemyDef&     _def,
    const eSarubiaState _state)
{
    EnemyState& _e = GetMutableState();
    EnterState(_e, static_cast<int>(_state));
    SetClip(_e, ClipOf_(_def.sarubia), true);
}
