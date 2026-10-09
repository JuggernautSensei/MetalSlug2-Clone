#include <cmath>

#include "Game/Enemies/EnemyCommon.h"
#include "Game/Enemies/EnemyDefParse.h"

namespace slug
{
namespace
{
    enum eHangState : int
    {
        kIdle,
        kThrow
    };

    void Go_(Enemy& _e, const EnemyDef& _def, EnemyEnv& _env, const int _state)
    {
        const HangCfg& cfg = _def.hang;
        EnterState(_e, _state);
        if (_state == kIdle)
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

    void Init_(Enemy& _e, const EnemyDef& _def, EnemyEnv& _env)
    {
        PrepareBody(_e, _def);
        _e.body.bUseGravity = false;
        Go_(_e, _def, _env, kIdle);
    }

    void Tick_(Enemy& _e, const EnemyDef& _def, EnemyEnv& _env)
    {
        const HangCfg& cfg = _def.hang;
        _e.body.velocity   = { 0.f, 0.f };

        if (_e.state == kIdle)
        {
            FacePlayer(_e, _env);
            if (std::abs(DxToPlayer(_e, _env)) <= cfg.detectX && _e.bActive)
            {
                ++_e.aux;
                if (_e.aux >= cfg.throwTicks)
                {
                    Go_(_e, _def, _env, kThrow);
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
                Go_(_e, _def, _env, kIdle);
            }
        }
    }

    void OnDeath_(Enemy& _e, const EnemyDef& _def, EnemyEnv& _env)
    {
        _e.bGrounded = false;
        BeginSoldierDeath(_e, _def, _env);
    }
}

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
}
