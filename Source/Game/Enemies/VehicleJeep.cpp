#include "Game/Enemies/EnemyCommon.h"
#include "Game/Enemies/EnemyDefParse.h"
#include "Game/Enemies/VehicleCommon.h"

namespace slug
{

// ===========================================
//  Jeep — 이동 로켓 지프 (Docs/EnemySpecs.md §2.1)
//   화면 좌/우 가장자리(+edgeInset) 사이를 오가며, 플레이어가 발사 쪽(바라보는 방향의 뒤)에 있을 때 로켓 4발을 쏜다.
//   클립 접미 = 주행 방향 (_Left 는 좌향 주행). 로켓은 항상 뒤로 나간다 (레거시 규약, 스프라이트 확인 필요).
//   사격 패턴: 2발 연속 → 2 s 휴지 → 2발 연속 → 2 s 휴지 → 재조준 (발수 4/3/1 은 즉시, 2/0 은 휴지 후).
// ===========================================

namespace
{
    enum eJeepState : int
    {
        kWait,           // 활성 전
        kIdleAndMove,    // 1 s 대기 후 목표 가장자리로 출발
        kMove,
        kTurn,           // 선회 후 반전 → Idle
        kTurnToMove,     // 선회 후 반전 → Move
        kIdle,
        kAimming,
        kShotIdle,
        kShotReady,
        kShot,
        kReAim,
        kTurnAfterShot   // 선회 후 반전 → IdleAndMove
    };

    // e.aux: 목표 가장자리 (-1 좌 / +1 우). e.counter: 남은 로켓 수.

    [[nodiscard]] int ClipOf_(const Enemy& _e, const JeepCfg& _c)
    {
        switch (_e.state)
        {
            case kMove: return _c.clipMove;
            case kTurn:
            case kTurnToMove:
            case kTurnAfterShot: return _c.clipTurn;
            case kAimming: return _c.clipAimming;
            case kShotIdle: return _c.clipReady;
            case kShotReady: return _c.clipShotReady;
            case kShot: return _c.clipShot;
            case kReAim: return _c.clipReAim;
            default: return _c.clipIdle;
        }
    }

    void Go_(Enemy& _e, const EnemyDef& _def, const int _state)
    {
        EnterState(_e, _state);
        SetClip(_e, ClipOf_(_e, _def.jeep), true);
    }

    [[nodiscard]] bool Done_(const Enemy& _e, const EnemyDef& _def)
    {
        return _e.clip >= 0 && _e.stateTicks >= _def.GetClip(_e.clip).GetDurationTicks();
    }

    [[nodiscard]] float TargetX_(const Enemy& _e, const JeepCfg& _c, const Aabb& _view)
    {
        return _e.aux < 0 ? _view.min.x + _c.edgeInset : _view.max.x - _c.edgeInset;
    }

    // 발사 쪽 = 바라보는 방향의 반대편
    [[nodiscard]] bool PlayerOnFireSide_(const Enemy& _e, const EnemyEnv& _env)
    {
        return DxToPlayer(_e, _env) * static_cast<float>(_e.facing) < 0.f;
    }

    void Init_(Enemy& _e, const EnemyDef& _def, EnemyEnv& _env)
    {
        (void)_env;
        PrepareBody(_e, _def);
        Go_(_e, _def, kWait);
    }

    void Tick_(Enemy& _e, const EnemyDef& _def, EnemyEnv& _env)
    {
        const JeepCfg& c    = _def.jeep;
        const Aabb&    view = _env.ctx.cameraView;
        _e.body.velocity.x  = 0.f;

        switch (_e.state)
        {
            case kWait:
                if (_e.bActive)
                {
                    Go_(_e, _def, kIdleAndMove);
                }
                break;

            case kIdleAndMove:
                if (_e.stateTicks >= c.idleTicks)
                {
                    _e.aux = DxToPlayer(_e, _env) < 0.f ? -1 : 1;   // 플레이어가 왼쪽이면 좌측 가장자리
                    Go_(_e, _def, _e.facing == _e.aux ? kMove : kTurnToMove);
                }
                break;

            case kTurnToMove:
                if (Done_(_e, _def))
                {
                    _e.facing = -_e.facing;
                    Go_(_e, _def, kMove);
                }
                break;

            case kMove:
            {
                _e.body.velocity.x = static_cast<float>(_e.facing) * c.speed;
                // 목표까지 남은 거리 (바라보는 방향 기준). 목표를 지나쳤으면 도착이다
                const float remaining = (TargetX_(_e, c, view) - _e.body.position.x) * static_cast<float>(_e.facing);
                if (remaining <= c.targetTol)
                {
                    Go_(_e, _def, kTurn);
                }
                break;
            }

            case kTurn:
                if (Done_(_e, _def))
                {
                    _e.facing = -_e.facing;
                    Go_(_e, _def, kIdle);
                }
                break;

            case kIdle:
                if (_e.stateTicks >= c.idleTicks)
                {
                    if (PlayerOnFireSide_(_e, _env))
                    {
                        _e.counter = c.bullets;
                        Go_(_e, _def, kAimming);
                    }
                    else
                    {
                        Go_(_e, _def, kTurn);
                    }
                }
                break;

            case kAimming:
                if (Done_(_e, _def))
                {
                    Go_(_e, _def, kShotIdle);
                }
                break;

            case kShotIdle:
                if (_e.counter == 2 || _e.counter == 0)
                {
                    if (_e.stateTicks >= c.waitTicks)
                    {
                        Go_(_e, _def, _e.counter == 0 ? kReAim : kShotReady);
                    }
                }
                else
                {
                    Go_(_e, _def, kShotReady);
                }
                break;

            case kShotReady:
                if (Done_(_e, _def))
                {
                    Go_(_e, _def, kShot);
                }
                break;

            case kShot:
                if (ClipReachedFrame(_e, _def, c.fireFrame))
                {
                    // 로켓: 후방(-바라보는 방향)으로 발사. 발수 홀짝에 따라 머즐이 20px 씩 엇갈린다 (레거시 k = bullet & 1)
                    const float facing = static_cast<float>(_e.facing);
                    const jug::VECTOR2 back  = { -facing, 0.f };
                    const jug::VECTOR2 shift = { 20.f * static_cast<float>(_e.counter & 1) * -facing, 0.f };
                    (void)FireWeapon(_e, _def, _env, c.weaponRocket, back, shift);
                }
                if (Done_(_e, _def))
                {
                    --_e.counter;
                    Go_(_e, _def, kShotIdle);
                }
                break;

            case kReAim:
                if (Done_(_e, _def))
                {
                    Go_(_e, _def, kTurnAfterShot);
                }
                break;

            case kTurnAfterShot:
                if (Done_(_e, _def))
                {
                    _e.facing = -_e.facing;
                    Go_(_e, _def, kIdleAndMove);
                }
                break;

            default: break;
        }

        (void)StepPhysics(_e, _def, _env);
    }

    void OnDeath_(Enemy& _e, const EnemyDef& _def, EnemyEnv& _env)
    {
        BeginVehicleDeath(_e, _def, _env, _def.jeep.clipDestroy);
    }
}   // namespace

const BehaviorOps kJeepOps = {
    .init       = &Init_,
    .tick       = &Tick_,
    .onDeath    = &OnDeath_,
    .tickDying  = &TickVehicleDeath,
};

void ParseJeepCfg(
    DefReader& _r,
    EnemyDef&  _def)
{
    JeepCfg& c = _def.jeep;

    c.speed     = _r.Num("speed");
    c.targetTol = _r.Num("targetTol");
    c.edgeInset = _r.Num("edgeInset");
    c.idleTicks = _r.Ticks("idleSec");
    c.waitTicks = _r.Ticks("waitSec");
    c.bullets   = _r.Int("bullets");
    c.fireFrame = _r.Int("fireFrame");

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

}   // namespace slug
