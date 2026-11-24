#include "Game/Enemies/HangEnemy.h"

#include <cmath>

#include "Game/Enemies/EnemyCommon.h"
#include "Game/Enemies/EnemyDefParse.h"

void HangEnemy::Init(
    const EnemyDef& _def,
    EnemyEnv&       _env)
{
    EnemyState& _e = GetMutableState();
    PrepareBody(_e, _def);
    _e.body.bUseGravity = false;
    Go_(_def, _env, eHangState::Idle);
}

void HangEnemy::Tick(
    const EnemyDef& _def,
    EnemyEnv&       _env)
{
    EnemyState&       _e     = GetMutableState();
    const HangConfig& config = _def.hang;
    _e.body.velocity         = { 0.f, 0.f };

    if (_e.state == static_cast<int>(eHangState::Idle))
    {
        FacePlayer(_e, _env);
        if (Abs(DxToPlayer(_e, _env)) <= config.detectX && _e.bActive)
        {
            ++m_throwTicks;
            if (m_throwTicks >= config.throwTicks)
            {
                Go_(_def, _env, eHangState::Throw);
            }
        }
    }
    else
    {
        if (ClipReachedFrame(_e, _def, config.fireFrame))
        {
            FireWeapon(_e, _def, _env, config.weaponBomb);
        }
        if (ClipFinished(_e, _def))
        {
            Go_(_def, _env, eHangState::Idle);
        }
    }
}

void HangEnemy::OnDeath(
    const EnemyDef& _def,
    EnemyEnv&       _env)
{
    EnemyState& _e = GetMutableState();
    _e.bGrounded   = false;
    BeginSoldierDeath(_e, _def, _env);
}

void HangEnemy::TickDying(
    const EnemyDef& _def,
    EnemyEnv&       _env)
{
    TickSoldierDeath(GetMutableState(), _def, _env);
}

void ParseHangConfig(
    DefReader& _r,
    EnemyDef&  _def)
{
    HangConfig& config = _def.hang;

    config.detectX    = _r.Num("detectX");
    config.throwTicks = _r.Ticks("throwDelaySec");
    config.fireFrame  = _r.Int("fireFrame");
    config.clipIdle   = _r.Clip("idle");
    config.clipThrow  = _r.Clip("throw");
    config.weaponBomb = _r.Weapon("bomb");
}

void HangEnemy::Go_(
    const EnemyDef&  _def,
    EnemyEnv&        _env,
    const eHangState _state)
{
    EnemyState&       _e     = GetMutableState();
    const HangConfig& config = _def.hang;
    EnterState(_e, static_cast<int>(_state));
    if (_state == eHangState::Idle)
    {
        FacePlayer(_e, _env);
        SetClip(_e, config.clipIdle, true);
        m_throwTicks = 0;
    }
    else
    {
        FacePlayer(_e, _env);
        SetClip(_e, config.clipThrow, true);
    }
}

void HangEnemy::HashActionState(Fnv1a& _hash) const
{
    HashActionState_(_hash, 0, m_throwTicks);
}
