#include "Game/Enemies/JeepEnemy.h"

#include "Game/Enemies/EnemyCommon.h"
#include "Game/Enemies/EnemyDefParse.h"
#include "Game/Enemies/VehicleCommon.h"

void JeepEnemy::Init(
    const EnemyDef& _def,
    EnemyEnv&       _env)
{
    EnemyState& _e = GetMutableState();
    (void)_env;
    PrepareBody(_e, _def);
    Go_(_def, eJeepState::Wait);
}

void JeepEnemy::Tick(
    const EnemyDef& _def,
    EnemyEnv&       _env)
{
    EnemyState&       _e     = GetMutableState();
    const JeepConfig& config = _def.jeep;
    const Aabb&       view   = _env.ctx.cameraView;
    _e.body.velocity.x       = 0.f;

    switch (static_cast<eJeepState>(_e.state))
    {
        case eJeepState::Wait:
            if (_e.bActive)
            {
                const int park = ParkSide_(config, view);
                if (park != 0)
                {
                    m_parkSide = park;
                    _e.facing  = -park;
                    Go_(_def, eJeepState::Move);
                }
                else
                {
                    Go_(_def, eJeepState::IdleAndMove);
                }
            }
            break;

        case eJeepState::IdleAndMove:
            if (_e.stateTicks >= config.idleTicks)
            {
                const int park = m_hasTurned == 0 ? ParkSide_(config, view) : 0;
                m_parkSide     = park != 0 ? park : (DxToPlayer(_e, _env) < 0.f ? -1 : 1);
                const int dir  = park != 0 ? -park : m_parkSide;
                Go_(_def, _e.facing == dir ? eJeepState::Move : eJeepState::TurnToMove);
            }
            break;

        case eJeepState::TurnToMove:
            if (IsClipDone(_e, _def))
            {
                _e.facing = -_e.facing;
                Go_(_def, eJeepState::Move);
            }
            break;

        case eJeepState::Move:
        {
            _e.body.velocity.x    = static_cast<float>(_e.facing) * config.speed;
            const float remaining = (TargetX_(config, view) - _e.body.position.x) * static_cast<float>(_e.facing);
            if (remaining <= config.targetTol)
            {
                m_hasTurned = 1;
                Go_(_def, eJeepState::Turn);
            }
            break;
        }

        case eJeepState::Turn:
            if (IsClipDone(_e, _def))
            {
                _e.facing = -_e.facing;
                Go_(_def, eJeepState::Idle);
            }
            break;

        case eJeepState::Idle:
            if (_e.stateTicks >= config.idleTicks)
            {
                if (PlayerOnFireSide_(_env))
                {
                    m_shotsRemaining = config.bullets;
                    Go_(_def, eJeepState::Aimming);
                }
                else
                {
                    Go_(_def, eJeepState::Turn);
                }
            }
            break;

        case eJeepState::Aimming:
            if (IsClipDone(_e, _def))
            {
                Go_(_def, eJeepState::ShotIdle);
            }
            break;

        case eJeepState::ShotIdle:
            if (m_shotsRemaining == 2 || m_shotsRemaining == 0 || m_shotsRemaining == config.bullets)
            {
                if (_e.stateTicks >= (m_shotsRemaining == config.bullets ? config.firstWaitTicks : config.waitTicks))
                {
                    Go_(_def, m_shotsRemaining == 0 ? eJeepState::ReAim : eJeepState::ShotReady);
                }
            }
            else
            {
                Go_(_def, eJeepState::ShotReady);
            }
            break;

        case eJeepState::ShotReady:
            if (IsClipDone(_e, _def))
            {
                Go_(_def, eJeepState::Shot);
            }
            break;

        case eJeepState::Shot:
            if (ClipReachedFrame(_e, _def, config.fireFrame))
            {
                const float   facing = static_cast<float>(_e.facing);
                const VECTOR2 back   = { -facing, 0.f };
                const VECTOR2 shift  = { config.rocketShiftX * static_cast<float>(m_shotsRemaining & 1) * -facing, 0.f };
                (void)FireWeapon(_e, _def, _env, config.weaponRocket, back, shift);
            }
            if (IsClipDone(_e, _def))
            {
                --m_shotsRemaining;
                Go_(_def, eJeepState::ShotIdle);
            }
            break;

        case eJeepState::ReAim:
            if (IsClipDone(_e, _def))
            {
                Go_(_def, eJeepState::TurnAfterShot);
            }
            break;

        case eJeepState::TurnAfterShot:
            if (IsClipDone(_e, _def))
            {
                _e.facing = -_e.facing;
                Go_(_def, eJeepState::IdleAndMove);
            }
            break;

        default: break;
    }

    (void)StepPhysics(_e, _def, _env);
}

void JeepEnemy::OnDeath(
    const EnemyDef& _def,
    EnemyEnv&       _env)
{
    EnemyState& _e = GetMutableState();
    BeginVehicleDeath(_e, _def, _env, _def.jeep.clipDestroy);
}

void JeepEnemy::TickDying(
    const EnemyDef& _def,
    EnemyEnv&       _env)
{
    TickVehicleDeath(GetMutableState(), _def, _env);
}

void ParseJeepConfig(
    DefReader& _r,
    EnemyDef&  _def)
{
    JeepConfig& config = _def.jeep;

    config.speed          = _r.Num("speed");
    config.targetTol      = _r.Num("targetTol");
    config.edgeInset      = _r.Num("edgeInset");
    config.idleTicks      = _r.Ticks("idleSec");
    config.waitTicks      = _r.Ticks("waitSec");
    config.firstWaitTicks = _r.Ticks("firstWaitSec", 0.f);
    config.bullets        = _r.Int("bullets");
    config.fireFrame      = _r.Int("fireFrame");
    config.rocketShiftX   = _r.Num("rocketShiftX", config.rocketShiftX);

    config.clipIdle      = _r.Clip("idle");
    config.clipMove      = _r.Clip("move");
    config.clipTurn      = _r.Clip("turn");
    config.clipAimming   = _r.Clip("aimming");
    config.clipReady     = _r.Clip("ready");
    config.clipShotReady = _r.Clip("shotReady");
    config.clipShot      = _r.Clip("shot");
    config.clipReAim     = _r.Clip("reAim");
    config.clipDestroy   = _r.Clip("destroy");

    config.weaponRocket = _r.Weapon("rocket");
}

[[nodiscard]] int JeepEnemy::ClipOf_(
    const JeepConfig& _config) const
{
    const EnemyState& _e = GetState();
    switch (static_cast<eJeepState>(_e.state))
    {
        case eJeepState::Move: return _config.clipMove;
        case eJeepState::Turn:
        case eJeepState::TurnToMove:
        case eJeepState::TurnAfterShot: return _config.clipTurn;
        case eJeepState::Aimming: return _config.clipAimming;
        case eJeepState::ShotIdle: return _config.clipReady;
        case eJeepState::ShotReady: return _config.clipShotReady;
        case eJeepState::Shot: return _config.clipShot;
        case eJeepState::ReAim: return _config.clipReAim;
        default: return _config.clipIdle;
    }
}

void JeepEnemy::Go_(
    const EnemyDef&  _def,
    const eJeepState _state)
{
    EnemyState& _e = GetMutableState();
    EnterState(_e, static_cast<int>(_state));
    SetClip(_e, ClipOf_(_def.jeep), true);
}

[[nodiscard]] float JeepEnemy::TargetX_(
    const JeepConfig& _config,
    const Aabb&       _view) const
{
    return m_parkSide < 0 ? _view.min.x + _config.edgeInset : _view.max.x - _config.edgeInset;
}

[[nodiscard]] int JeepEnemy::ParkSide_(
    const JeepConfig& _config,
    const Aabb&       _view) const
{
    const EnemyState& _e = GetState();
    const float       x  = _e.body.position.x;
    if (x <= _view.min.x + _config.edgeInset + _config.targetTol)
    {
        return -1;
    }
    return x >= _view.max.x - _config.edgeInset - _config.targetTol ? 1 : 0;
}

[[nodiscard]] bool JeepEnemy::PlayerOnFireSide_(
    const EnemyEnv& _env) const
{
    const EnemyState& _e = GetState();
    return DxToPlayer(_e, _env) * static_cast<float>(_e.facing) < 0.f;
}

void JeepEnemy::HashActionState(Fnv1a& _hash) const
{
    HashActionState_(_hash, m_shotsRemaining, m_parkSide, m_hasTurned);
}
