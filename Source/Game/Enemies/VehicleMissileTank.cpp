#include "Game/Enemies/MissileTankEnemy.h"

#include <cmath>

#include "Game/Enemies/EnemyCommon.h"
#include "Game/Enemies/EnemyDefParse.h"
#include "Game/Enemies/VehicleCommon.h"

constexpr int kModeDown = 0;
constexpr int kModeUp   = 1;

void MissileTankEnemy::Init(
    const EnemyDef& _def,
    EnemyEnv&       _env)
{
    EnemyState& _e = GetMutableState();
    (void)_env;
    PrepareBody(_e, _def);
    m_launcherPosition = kModeDown;
    Go_(_def, eMissileTankState::Idle);
    if (_def.missileTank.clipAppear >= 0)
    {
        SetClip(_e, _def.missileTank.clipAppear, true);   // 스폰 연출
    }
}

void MissileTankEnemy::Tick(
    const EnemyDef& _def,
    EnemyEnv&       _env)
{
    EnemyState&              _e     = GetMutableState();
    const MissileTankConfig& config = _def.missileTank;
    _e.body.velocity.x              = 0.f;
    const float dx                  = Abs(DxToPlayer(_e, _env));

    switch (static_cast<eMissileTankState>(_e.state))
    {
        case eMissileTankState::Idle:
            if (_e.clip == config.clipAppear && config.clipAppear >= 0 && _e.stateTicks >= _def.GetClip(config.clipAppear).GetDurationTicks())
            {
                SetClip(_e, ClipOf_(config), true);
            }
            if (dx > config.rangeDx)
            {
                _e.stateTicks = 0;
                break;
            }
            if (_e.stateTicks >= config.detectTicks)
            {
                const bool bWantUp = dx > config.farDx;
                if (bWantUp != IsUp_())
                {
                    Go_(_def, eMissileTankState::Aim);
                }
                else
                {
                    Go_(_def, eMissileTankState::Ready);
                }
            }
            break;

        case eMissileTankState::Aim:
            if (_e.clip >= 0 && _e.stateTicks >= _def.GetClip(_e.clip).GetDurationTicks())
            {
                m_launcherPosition = IsUp_() ? kModeDown : kModeUp;
                Go_(_def, eMissileTankState::Idle);
            }
            break;

        case eMissileTankState::Ready:
            if (_e.stateTicks >= config.readyTicks)
            {
                m_shotsRemaining = config.bullets;
                Go_(_def, eMissileTankState::Shot);
            }
            break;

        case eMissileTankState::Shot:
            if (ClipReachedFrame(_e, _def, config.fireFrame))
            {
                const VECTOR2 jitter = { 0.f, static_cast<float>(_env.world.Roll(24)) };
                (void)FireWeapon(_e, _def, _env, IsUp_() ? config.weaponUp : config.weaponDown, { 0.f, 0.f }, jitter);
            }
            if (_e.clip >= 0 && _e.stateTicks >= _def.GetClip(_e.clip).GetDurationTicks())
            {
                --m_shotsRemaining;
                if (m_shotsRemaining > 0)
                {
                    Go_(_def, eMissileTankState::Shot);
                }
                else
                {
                    Go_(_def, eMissileTankState::Idle);
                }
            }
            break;

        default: break;
    }

    (void)StepPhysics(_e, _def, _env);
}

void MissileTankEnemy::OnDeath(
    const EnemyDef& _def,
    EnemyEnv&       _env)
{
    EnemyState& _e = GetMutableState();
    BeginVehicleDeath(_e, _def, _env, IsUp_() ? _def.missileTank.clipDestroyUp : _def.missileTank.clipDestroyDown);
}

void MissileTankEnemy::TickDying(
    const EnemyDef& _def,
    EnemyEnv&       _env)
{
    TickVehicleDeath(GetMutableState(), _def, _env);
}

void ParseMissileTankConfig(
    DefReader& _r,
    EnemyDef&  _def)
{
    MissileTankConfig& config = _def.missileTank;

    config.rangeDx     = _r.Num("rangeDx");
    config.farDx       = _r.Num("farDx");
    config.detectTicks = _r.Ticks("detectSec");
    config.readyTicks  = _r.Ticks("readySec");
    config.bullets     = _r.Int("bullets");
    config.fireFrame   = _r.Int("fireFrame");

    config.clipIdleUp      = _r.Clip("idleUp");
    config.clipIdleDown    = _r.Clip("idleDown");
    config.clipAimUp       = _r.Clip("aimUp");
    config.clipAimDown     = _r.Clip("aimDown");
    config.clipReadyUp     = _r.Clip("readyUp");
    config.clipReadyDown   = _r.Clip("readyDown");
    config.clipShotUp      = _r.Clip("shotUp");
    config.clipShotDown    = _r.Clip("shotDown");
    config.clipDestroyUp   = _r.Clip("destroyUp");
    config.clipDestroyDown = _r.Clip("destroyDown");
    config.clipAppear      = _r.ClipOpt("appear");

    config.weaponUp   = _r.Weapon("up");
    config.weaponDown = _r.Weapon("down");
}

[[nodiscard]] bool MissileTankEnemy::IsUp_() const
{
    return m_launcherPosition == kModeUp;
}

[[nodiscard]] int MissileTankEnemy::ClipOf_(
    const MissileTankConfig& _config) const
{
    const EnemyState& _e  = GetState();
    const bool        bUp = IsUp_();
    switch (static_cast<eMissileTankState>(_e.state))
    {
        case eMissileTankState::Aim: return m_launcherPosition == kModeDown ? _config.clipAimUp : _config.clipAimDown;
        case eMissileTankState::Ready: return bUp ? _config.clipReadyUp : _config.clipReadyDown;
        case eMissileTankState::Shot: return bUp ? _config.clipShotUp : _config.clipShotDown;
        default: return bUp ? _config.clipIdleUp : _config.clipIdleDown;
    }
}

void MissileTankEnemy::Go_(
    const EnemyDef&         _def,
    const eMissileTankState _state)
{
    EnemyState& _e = GetMutableState();
    EnterState(_e, static_cast<int>(_state));
    SetClip(_e, ClipOf_(_def.missileTank), true);
}

void MissileTankEnemy::HashActionState(Fnv1a& _hash) const
{
    HashActionState_(_hash, m_shotsRemaining, m_launcherPosition);
}
