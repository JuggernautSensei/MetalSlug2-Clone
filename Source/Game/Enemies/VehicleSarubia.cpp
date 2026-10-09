#include <cmath>

#include "Game/Enemies/EnemyCommon.h"
#include "Game/Enemies/EnemyDefParse.h"
#include "Game/Enemies/VehicleCommon.h"

namespace slug
{

// ===========================================
//  Sarubia — 보행 전차, 중간보스 게이트 (Docs/EnemySpecs.md §2.5)
//   Idle 2 s 후: 플레이어가 뒤에 있으면 선회, 멀면(|dx|>chaseDx) 전진, 아니면 수류탄 사격.
//   수류탄(sarubia_grenade)은 수직으로 떨어져 착지한 뒤 지면을 따라 구르고, 수명 후 시각 전용 폭발로 사라진다.
// ===========================================

namespace
{
    enum eSarubiaState : int
    {
        kIdle,
        kTurn,
        kMoveStart,
        kMove,
        kMoveStop,
        kShotStart,   // 사격 준비 (1.5 s)
        kShot
    };

    [[nodiscard]] int ClipOf_(const Enemy& _e, const SarubiaCfg& _c)
    {
        switch (_e.state)
        {
            case kTurn: return _c.clipTurn;
            case kMoveStart: return _c.clipMoveStart;
            case kMove: return _c.clipMove;
            case kMoveStop: return _c.clipMoveStop;
            case kShotStart: return _c.clipShotStart;
            case kShot: return _c.clipShot;
            default: return _c.clipIdle;
        }
    }

    void Go_(Enemy& _e, const EnemyDef& _def, const int _state)
    {
        EnterState(_e, _state);
        SetClip(_e, ClipOf_(_e, _def.sarubia), true);
    }

    [[nodiscard]] bool Done_(const Enemy& _e, const EnemyDef& _def)
    {
        return _e.clip >= 0 && _e.stateTicks >= _def.GetClip(_e.clip).GetDurationTicks();
    }

    void Init_(Enemy& _e, const EnemyDef& _def, EnemyEnv& _env)
    {
        (void)_env;
        PrepareBody(_e, _def);
        Go_(_e, _def, kIdle);
    }

    void Tick_(Enemy& _e, const EnemyDef& _def, EnemyEnv& _env)
    {
        const SarubiaCfg& c = _def.sarubia;
        _e.body.velocity.x  = 0.f;

        if (!_e.bActive)
        {
            (void)StepPhysics(_e, _def, _env);
            return;
        }

        const float dx    = DxToPlayer(_e, _env);
        const float absDx = std::abs(dx);
        const bool  bBehind = dx * static_cast<float>(_e.facing) < 0.f;   // 플레이어가 바라보는 방향의 반대편

        switch (_e.state)
        {
            case kIdle:
                if (_e.stateTicks >= c.cooldownTicks)
                {
                    if (bBehind)
                    {
                        Go_(_e, _def, kTurn);
                    }
                    else if (absDx > c.chaseDx)
                    {
                        Go_(_e, _def, kMoveStart);
                    }
                    else
                    {
                        Go_(_e, _def, kShotStart);
                    }
                }
                break;

            case kTurn:
                if (Done_(_e, _def))
                {
                    _e.facing = -_e.facing;
                    Go_(_e, _def, kIdle);
                }
                break;

            case kMoveStart:
                if (Done_(_e, _def))
                {
                    Go_(_e, _def, kMove);
                }
                break;

            case kMove:
                _e.body.velocity.x = static_cast<float>(_e.facing) * _def.physics.speed;
                if (_e.stateTicks >= c.moveMaxTicks || absDx <= c.stopDx)
                {
                    Go_(_e, _def, kMoveStop);
                }
                break;

            case kMoveStop:
                if (Done_(_e, _def))
                {
                    Go_(_e, _def, kIdle);
                }
                break;

            case kShotStart:
                if (_e.stateTicks >= c.shotReadyTicks)
                {
                    Go_(_e, _def, kShot);
                }
                break;

            case kShot:
                if (ClipReachedFrame(_e, _def, c.fireFrame))
                {
                    (void)FireWeapon(_e, _def, _env, c.weaponGrenade);
                }
                if (Done_(_e, _def))
                {
                    Go_(_e, _def, kIdle);
                }
                break;

            default: break;
        }

        (void)StepPhysics(_e, _def, _env);
    }

    void OnDeath_(Enemy& _e, const EnemyDef& _def, EnemyEnv& _env)
    {
        BeginVehicleDeath(_e, _def, _env, _def.sarubia.clipDestroy);
    }
}   // namespace

const BehaviorOps kSarubiaOps = {
    .init       = &Init_,
    .tick       = &Tick_,
    .onDeath    = &OnDeath_,
    .tickDying  = &TickVehicleDeath,
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

}   // namespace slug
