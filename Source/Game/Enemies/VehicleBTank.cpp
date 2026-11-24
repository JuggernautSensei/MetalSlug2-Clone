#include "Game/Enemies/BTankEnemy.h"

#include <algorithm>
#include <cmath>

#include "Game/Enemies/EnemyCommon.h"
#include "Game/Enemies/EnemyDefParse.h"
#include "Game/Enemies/VehicleCommon.h"

void BTankEnemy::Init(
    const EnemyDef& _def,
    EnemyEnv&       _env)
{
    EnemyState& _e = GetMutableState();
    (void)_env;
    PrepareBody(_e, _def);
    _e.facing = -1;
    Go_(_def, eBTankState::Closed);
}

void BTankEnemy::Tick(
    const EnemyDef& _def,
    EnemyEnv&       _env)
{
    EnemyState&        _e     = GetMutableState();
    const BTankConfig& config = _def.btank;
    _e.body.velocity.x        = 0.f;

    if (!_e.bActive)
    {
        if (_env.ctx.player.feet.x >= _e.body.position.x - config.activateDx)
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
        case eBTankState::Closed: Go_(_def, eBTankState::Open); break;

        case eBTankState::Open:
            if (_e.clip >= 0 && _e.stateTicks >= _def.GetClip(_e.clip).GetDurationTicks())
            {
                Go_(_def, eBTankState::Idle);
            }
            break;

        case eBTankState::Idle:
            if (_e.stateTicks >= config.idleTicks)
            {
                m_shotsRemaining = config.shotsPerCycle;
                Go_(_def, eBTankState::Fire);
            }
            break;

        case eBTankState::Fire:
            if (ClipReachedFrame(_e, _def, config.fireFrame))
            {
                (void)FireWeapon(_e, _def, _env, config.weaponShell);
            }
            if (_e.clip >= 0 && _e.stateTicks >= _def.GetClip(_e.clip).GetDurationTicks())
            {
                --m_shotsRemaining;
                if (m_shotsRemaining > 0)
                {
                    Go_(_def, eBTankState::Fire);
                }
                else
                {
                    Go_(_def, eBTankState::Idle);
                }
            }
            break;

        default: break;
    }

    (void)StepPhysics(_e, _def, _env);
}

void BTankEnemy::OnDeath(
    const EnemyDef& _def,
    EnemyEnv&       _env)
{
    EnemyState& _e = GetMutableState();
    BeginVehicleDeath(_e, _def, _env, _def.btank.clipDestroy);
}

void BTankEnemy::TickDying(
    const EnemyDef& _def,
    EnemyEnv&       _env)
{
    TickVehicleDeath(GetMutableState(), _def, _env);
}

int BTankEnemy::GetShotCounter() const noexcept
{
    return m_shotsRemaining;
}

void ParseBTankConfig(
    DefReader& _r,
    EnemyDef&  _def)
{
    BTankConfig& config = _def.btank;

    config.activateDx    = _r.Num("activateDx");
    config.idleTicks     = _r.Ticks("idleSec");
    config.shotsPerCycle = _r.Int("shotsPerCycle");
    config.fireFrame     = _r.Int("fireFrame");

    config.clipIdle    = _r.Clip("idle");
    config.clipOpen    = _r.Clip("open");
    config.clipFire    = _r.Clip("fire");
    config.clipDestroy = _r.Clip("destroy");

    config.weaponShell = _r.Weapon("shell");
}

[[nodiscard]] int BTankEnemy::ClipOf_(
    const BTankConfig& _config) const
{
    const EnemyState& _e = GetState();
    switch (static_cast<eBTankState>(_e.state))
    {
        case eBTankState::Open:
        case eBTankState::Idle: return _config.clipOpen;
        case eBTankState::Fire: return _config.clipFire;
        default: return _config.clipIdle;
    }
}

void BTankEnemy::Go_(
    const EnemyDef&   _def,
    const eBTankState _state)
{
    EnemyState&        _e     = GetMutableState();
    const BTankConfig& config = _def.btank;
    EnterState(_e, static_cast<int>(_state));
    const int clip = ClipOf_(config);
    SetClip(_e, clip, _state != eBTankState::Idle);
    if (_state == eBTankState::Idle && clip >= 0)
    {
        _e.clipTicks = Max(_e.clipTicks, _def.GetClip(clip).GetDurationTicks());
    }
}

void BTankEnemy::HashActionState(Fnv1a& _hash) const
{
    HashActionState_(_hash, m_shotsRemaining, 0);
}
