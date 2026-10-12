#include <cmath>

#include "Base/Math.h"
#include "Game/Enemies/EnemyCommon.h"
#include "Game/Enemies/EnemyDefParse.h"
#include "Game/Enemies/VehicleCommon.h"

enum class eMissleTankState
{
    Idle,
    Aim,
    Ready,
    Shot
};

constexpr int kModeDown = 0;
constexpr int kModeUp   = 1;

[[nodiscard]] static bool IsUp_(
    const Enemy& _e)
{
    return _e.aux == kModeUp;
}

[[nodiscard]] static int ClipOf_(
    const Enemy&         _e,
    const MissleTankCfg& _c)
{
    const bool bUp = IsUp_(_e);
    switch (static_cast<eMissleTankState>(_e.state))
    {
        case eMissleTankState::Aim: return _e.aux == kModeDown ? _c.clipAimUp : _c.clipAimDown;
        case eMissleTankState::Ready: return bUp ? _c.clipReadyUp : _c.clipReadyDown;
        case eMissleTankState::Shot: return bUp ? _c.clipShotUp : _c.clipShotDown;
        default: return bUp ? _c.clipIdleUp : _c.clipIdleDown;
    }
}

static void Go_(
    Enemy&                 _e,
    const EnemyDef&        _def,
    const eMissleTankState _state)
{
    EnterState(_e, static_cast<int>(_state));
    SetClip(_e, ClipOf_(_e, _def.missleTank), true);
}

static void Init_(
    Enemy&          _e,
    const EnemyDef& _def,
    EnemyEnv&       _env)
{
    (void)_env;
    PrepareBody(_e, _def);
    _e.aux = kModeDown;
    Go_(_e, _def, eMissleTankState::Idle);
    if (_def.missleTank.clipAppear >= 0)
    {
        SetClip(_e, _def.missleTank.clipAppear, true);   // 스폰 연출 (Idle 첫 1 s 안에 끝난다)
    }
}

static void Tick_(
    Enemy&          _e,
    const EnemyDef& _def,
    EnemyEnv&       _env)
{
    const MissleTankCfg& c = _def.missleTank;
    _e.body.velocity.x     = 0.f;
    const float dx         = std::abs(DxToPlayer(_e, _env));

    switch (static_cast<eMissleTankState>(_e.state))
    {
        case eMissleTankState::Idle:
            if (_e.clip == c.clipAppear && c.clipAppear >= 0 && _e.stateTicks >= _def.GetClip(c.clipAppear).GetDurationTicks())
            {
                SetClip(_e, ClipOf_(_e, c), true);   // 스폰 연출 끝: 일반 Idle 로 전환 (상태/타이머는 그대로)
            }
            if (dx > c.rangeDx)
            {
                _e.stateTicks = 0;   // 사거리 밖: 타이머 리셋
                break;
            }
            if (_e.stateTicks >= c.detectTicks)
            {
                const bool bWantUp = dx > c.farDx;
                if (bWantUp != IsUp_(_e))
                {
                    Go_(_e, _def, eMissleTankState::Aim);
                }
                else
                {
                    Go_(_e, _def, eMissleTankState::Ready);
                }
            }
            break;

        case eMissleTankState::Aim:
            if (_e.clip >= 0 && _e.stateTicks >= _def.GetClip(_e.clip).GetDurationTicks())
            {
                _e.aux = IsUp_(_e) ? kModeDown : kModeUp;
                Go_(_e, _def, eMissleTankState::Idle);
            }
            break;

        case eMissleTankState::Ready:
            if (_e.stateTicks >= c.readyTicks)
            {
                _e.counter = c.bullets;
                Go_(_e, _def, eMissleTankState::Shot);
            }
            break;

        case eMissleTankState::Shot:
            if (ClipReachedFrame(_e, _def, c.fireFrame))
            {
                const Vec2 jitter = { 0.f, static_cast<float>(_env.world.Roll(24)) };
                (void)FireWeapon(_e, _def, _env, IsUp_(_e) ? c.weaponUp : c.weaponDown, { 0.f, 0.f }, jitter);
            }
            if (_e.clip >= 0 && _e.stateTicks >= _def.GetClip(_e.clip).GetDurationTicks())
            {
                --_e.counter;
                if (_e.counter > 0)
                {
                    Go_(_e, _def, eMissleTankState::Shot);
                }
                else
                {
                    Go_(_e, _def, eMissleTankState::Idle);
                }
            }
            break;

        default: break;
    }

    (void)StepPhysics(_e, _def, _env);
}

static void OnDeath_(
    Enemy&          _e,
    const EnemyDef& _def,
    EnemyEnv&       _env)
{
    BeginVehicleDeath(_e, _def, _env, IsUp_(_e) ? _def.missleTank.clipDestroyUp : _def.missleTank.clipDestroyDown);
}

const BehaviorOps kMissleTankOps = {
    .init      = &Init_,
    .tick      = &Tick_,
    .onDeath   = &OnDeath_,
    .tickDying = &TickVehicleDeath,
};

void ParseMissleTankCfg(
    DefReader& _r,
    EnemyDef&  _def)
{
    MissleTankCfg& c = _def.missleTank;

    c.rangeDx     = _r.Num("rangeDx");
    c.farDx       = _r.Num("farDx");
    c.detectTicks = _r.Ticks("detectSec");
    c.readyTicks  = _r.Ticks("readySec");
    c.bullets     = _r.Int("bullets");
    c.fireFrame   = _r.Int("fireFrame");

    c.clipIdleUp      = _r.Clip("idleUp");
    c.clipIdleDown    = _r.Clip("idleDown");
    c.clipAimUp       = _r.Clip("aimUp");
    c.clipAimDown     = _r.Clip("aimDown");
    c.clipReadyUp     = _r.Clip("readyUp");
    c.clipReadyDown   = _r.Clip("readyDown");
    c.clipShotUp      = _r.Clip("shotUp");
    c.clipShotDown    = _r.Clip("shotDown");
    c.clipDestroyUp   = _r.Clip("destroyUp");
    c.clipDestroyDown = _r.Clip("destroyDown");
    c.clipAppear      = _r.ClipOpt("appear");

    c.weaponUp   = _r.Weapon("up");
    c.weaponDown = _r.Weapon("down");
}
