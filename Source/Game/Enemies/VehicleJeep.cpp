#include "Base/Math.h"
#include "Game/Enemies/EnemyCommon.h"
#include "Game/Enemies/EnemyDefParse.h"
#include "Game/Enemies/VehicleCommon.h"

enum class eJeepState
{
    Wait,
    IdleAndMove,
    Move,
    Turn,
    TurnToMove,
    Idle,
    Aimming,
    ShotIdle,
    ShotReady,
    Shot,
    ReAim,
    TurnAfterShot
};

namespace
{
[[nodiscard]] int ClipOf_(
    const Enemy&   _e,
    const JeepCfg& _c)
{
    switch (static_cast<eJeepState>(_e.state))
    {
        case eJeepState::Move: return _c.clipMove;
        case eJeepState::Turn:
        case eJeepState::TurnToMove:
        case eJeepState::TurnAfterShot: return _c.clipTurn;
        case eJeepState::Aimming: return _c.clipAimming;
        case eJeepState::ShotIdle: return _c.clipReady;
        case eJeepState::ShotReady: return _c.clipShotReady;
        case eJeepState::Shot: return _c.clipShot;
        case eJeepState::ReAim: return _c.clipReAim;
        default: return _c.clipIdle;
    }
}

void Go_(
    Enemy&           _e,
    const EnemyDef&  _def,
    const eJeepState _state)
{
    EnterState(_e, static_cast<int>(_state));
    SetClip(_e, ClipOf_(_e, _def.jeep), true);
}

[[nodiscard]] float TargetX_(
    const Enemy&   _e,
    const JeepCfg& _c,
    const Aabb&    _view)
{
    return _e.aux < 0 ? _view.min.x + _c.edgeInset : _view.max.x - _c.edgeInset;
}

// 주차 지점(가장자리 + edgeInset) 이거나 그보다 바깥이면 그쪽 가장자리 (-1 왼쪽 / +1 오른쪽), 화면 안쪽이면 0
[[nodiscard]] int ParkSide_(
    const Enemy&   _e,
    const JeepCfg& _c,
    const Aabb&    _view)
{
    const float x = _e.body.position.x;
    if (x <= _view.min.x + _c.edgeInset + _c.targetTol)
    {
        return -1;
    }
    return x >= _view.max.x - _c.edgeInset - _c.targetTol ? 1 : 0;
}

[[nodiscard]] bool PlayerOnFireSide_(
    const Enemy&    _e,
    const EnemyEnv& _env)
{
    return DxToPlayer(_e, _env) * static_cast<float>(_e.facing) < 0.f;
}

void Init_(
    Enemy&          _e,
    const EnemyDef& _def,
    EnemyEnv&       _env)
{
    (void)_env;
    PrepareBody(_e, _def);
    Go_(_e, _def, eJeepState::Wait);
}

void Tick_(
    Enemy&          _e,
    const EnemyDef& _def,
    EnemyEnv&       _env)
{
    const JeepCfg& c    = _def.jeep;
    const Aabb&    view = _env.ctx.cameraView;
    _e.body.velocity.x  = 0.f;

    switch (static_cast<eJeepState>(_e.state))
    {
        case eJeepState::Wait:
            if (_e.bActive)
            {
                const int park = ParkSide_(_e, c, view);
                if (park != 0)
                {
                    _e.aux    = park;
                    _e.facing = -park;
                    Go_(_e, _def, eJeepState::Move);
                }
                else
                {
                    Go_(_e, _def, eJeepState::IdleAndMove);
                }
            }
            break;

        case eJeepState::IdleAndMove:
            if (_e.stateTicks >= c.idleTicks)
            {
                const int park = _e.aux2 == 0 ? ParkSide_(_e, c, view) : 0;
                _e.aux         = park != 0 ? park : (DxToPlayer(_e, _env) < 0.f ? -1 : 1);
                const int dir  = park != 0 ? -park : _e.aux;
                Go_(_e, _def, _e.facing == dir ? eJeepState::Move : eJeepState::TurnToMove);
            }
            break;

        case eJeepState::TurnToMove:
            if (IsClipDone(_e, _def))
            {
                _e.facing = -_e.facing;
                Go_(_e, _def, eJeepState::Move);
            }
            break;

        case eJeepState::Move:
        {
            _e.body.velocity.x    = static_cast<float>(_e.facing) * c.speed;
            const float remaining = (TargetX_(_e, c, view) - _e.body.position.x) * static_cast<float>(_e.facing);
            if (remaining <= c.targetTol)
            {
                _e.aux2 = 1;
                Go_(_e, _def, eJeepState::Turn);
            }
            break;
        }

        case eJeepState::Turn:
            if (IsClipDone(_e, _def))
            {
                _e.facing = -_e.facing;
                Go_(_e, _def, eJeepState::Idle);
            }
            break;

        case eJeepState::Idle:
            if (_e.stateTicks >= c.idleTicks)
            {
                if (PlayerOnFireSide_(_e, _env))
                {
                    _e.counter = c.bullets;
                    Go_(_e, _def, eJeepState::Aimming);
                }
                else
                {
                    Go_(_e, _def, eJeepState::Turn);
                }
            }
            break;

        case eJeepState::Aimming:
            if (IsClipDone(_e, _def))
            {
                Go_(_e, _def, eJeepState::ShotIdle);
            }
            break;

        case eJeepState::ShotIdle:
            if (_e.counter == 2 || _e.counter == 0 || _e.counter == c.bullets)
            {
                if (_e.stateTicks >= (_e.counter == c.bullets ? c.firstWaitTicks : c.waitTicks))
                {
                    Go_(_e, _def, _e.counter == 0 ? eJeepState::ReAim : eJeepState::ShotReady);
                }
            }
            else
            {
                Go_(_e, _def, eJeepState::ShotReady);
            }
            break;

        case eJeepState::ShotReady:
            if (IsClipDone(_e, _def))
            {
                Go_(_e, _def, eJeepState::Shot);
            }
            break;

        case eJeepState::Shot:
            if (ClipReachedFrame(_e, _def, c.fireFrame))
            {
                const float facing = static_cast<float>(_e.facing);
                const Vec2  back   = { -facing, 0.f };
                const Vec2  shift  = { c.rocketShiftX * static_cast<float>(_e.counter & 1) * -facing, 0.f };
                (void)FireWeapon(_e, _def, _env, c.weaponRocket, back, shift);
            }
            if (IsClipDone(_e, _def))
            {
                --_e.counter;
                Go_(_e, _def, eJeepState::ShotIdle);
            }
            break;

        case eJeepState::ReAim:
            if (IsClipDone(_e, _def))
            {
                Go_(_e, _def, eJeepState::TurnAfterShot);
            }
            break;

        case eJeepState::TurnAfterShot:
            if (IsClipDone(_e, _def))
            {
                _e.facing = -_e.facing;
                Go_(_e, _def, eJeepState::IdleAndMove);
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
    BeginVehicleDeath(_e, _def, _env, _def.jeep.clipDestroy);
}
}   // namespace

const BehaviorOps kJeepOps = {
    .init      = &Init_,
    .tick      = &Tick_,
    .onDeath   = &OnDeath_,
    .tickDying = &TickVehicleDeath,
};

void ParseJeepCfg(
    DefReader& _r,
    EnemyDef&  _def)
{
    JeepCfg& c = _def.jeep;

    c.speed          = _r.Num("speed");
    c.targetTol      = _r.Num("targetTol");
    c.edgeInset      = _r.Num("edgeInset");
    c.idleTicks      = _r.Ticks("idleSec");
    c.waitTicks      = _r.Ticks("waitSec");
    c.firstWaitTicks = _r.Ticks("firstWaitSec", 0.f);
    c.bullets        = _r.Int("bullets");
    c.fireFrame      = _r.Int("fireFrame");
    c.rocketShiftX   = _r.Num("rocketShiftX", c.rocketShiftX);

    c.clipIdle      = _r.Clip("idle");
    c.clipMove      = _r.Clip("move");
    c.clipTurn      = _r.Clip("turn");
    c.clipAimming   = _r.Clip("aimming");
    c.clipReady     = _r.Clip("ready");
    c.clipShotReady = _r.Clip("shotReady");
    c.clipShot      = _r.Clip("shot");
    c.clipReAim     = _r.Clip("reAim");
    c.clipDestroy   = _r.Clip("destroy");

    c.weaponRocket = _r.Weapon("rocket");
}
