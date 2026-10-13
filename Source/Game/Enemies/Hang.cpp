#include <cmath>

#include "Game/Enemies/EnemyCommon.h"
#include "Game/Enemies/EnemyDefParse.h"

enum class eHangState
{
    Idle,
    Throw
};

namespace
{
void Go_(
    Enemy&          _e,
    const EnemyDef& _def,
    EnemyEnv&       _env,
    eHangState      _state)
{
    const HangCfg& cfg = _def.hang;
    EnterState(_e, static_cast<int>(_state));
    if (_state == eHangState::Idle)
    {
        FacePlayer(_e, _env);
        SetClip(_e, cfg.clipIdle, true);
        _e.aux = 0;
    }
    else
    {
        FacePlayer(_e, _env);
        SetClip(_e, cfg.clipThrow, true);
    }
}

void Init_(
    Enemy&          _e,
    const EnemyDef& _def,
    EnemyEnv&       _env)
{
    PrepareBody(_e, _def);
    _e.body.bUseGravity = false;
    Go_(_e, _def, _env, eHangState::Idle);
}

void Tick_(
    Enemy&          _e,
    const EnemyDef& _def,
    EnemyEnv&       _env)
{
    const HangCfg& cfg = _def.hang;
    _e.body.velocity   = { 0.f, 0.f };

    if (_e.state == static_cast<int>(eHangState::Idle))
    {
        FacePlayer(_e, _env);
        if (Abs(DxToPlayer(_e, _env)) <= cfg.detectX && _e.bActive)
        {
            ++_e.aux;
            if (_e.aux >= cfg.throwTicks)
            {
                Go_(_e, _def, _env, eHangState::Throw);
            }
        }
    }
    else
    {
        if (ClipReachedFrame(_e, _def, cfg.fireFrame))
        {
            FireWeapon(_e, _def, _env, cfg.weaponBomb);
        }
        if (ClipFinished(_e, _def))
        {
            Go_(_e, _def, _env, eHangState::Idle);
        }
    }
}

void OnDeath_(
    Enemy&          _e,
    const EnemyDef& _def,
    EnemyEnv&       _env)
{
    _e.bGrounded = false;
    BeginSoldierDeath(_e, _def, _env);
}
}   // namespace

const BehaviorOps kHangOps = {
    .init              = &Init_,
    .tick              = &Tick_,
    .onHit             = nullptr,
    .onDeath           = &OnDeath_,
    .tickDying         = &TickSoldierDeath,
    .registerHurtboxes = nullptr,
};

void ParseHangCfg(
    DefReader& _r,
    EnemyDef&  _def)
{
    HangCfg& c = _def.hang;

    c.detectX    = _r.Num("detectX");
    c.throwTicks = _r.Ticks("throwDelaySec");
    c.fireFrame  = _r.Int("fireFrame");
    c.clipIdle   = _r.Clip("idle");
    c.clipThrow  = _r.Clip("throw");
    c.weaponBomb = _r.Weapon("bomb");
}
