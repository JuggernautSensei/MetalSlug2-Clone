#include <cmath>
#include <format>

#include "Base/Math.h"
#include "Game/Enemies/EnemyCommon.h"
#include "Game/Enemies/EnemyDefParse.h"

enum class eBzkState
{
    Wait,
    Fall,
    Idle,
    Move,
    HJump,
    Landing,
    Aim,
    ShotReady,
    Shot,
    Recover,
    Climb
};

// aiVariant 2 = 버스 계단 바주카병: Climb (스폰 -> arena.min -> arena.max, 물리 없음) 뒤 Fall 로 승강대에 선다.
constexpr int kVariantClimb = 2;

namespace
{
void Go_(
    Enemy&          _e,
    const EnemyDef& _def,
    EnemyEnv&       _env,
    const eBzkState _state)
{
    const BzkCfg& cfg = _def.bzk;
    EnterState(_e, static_cast<int>(_state));
    switch (_state)
    {
        case eBzkState::Wait:
            SetClip(_e, cfg.clipIdle, true);
            _e.body.velocity.x = 0.f;
            break;
        case eBzkState::Fall:
            SetClip(_e, cfg.clipHJump, true);
            break;
        case eBzkState::Idle:
            FacePlayer(_e, _env);
            SetClip(_e, cfg.clipIdle, true);
            _e.body.velocity.x = 0.f;
            break;
        case eBzkState::Move:
            FacePlayer(_e, _env);
            SetClip(_e, cfg.clipMove, true);
            break;
        case eBzkState::HJump:
            FacePlayer(_e, _env);
            SetClip(_e, cfg.clipHJump, true);
            _e.aux3            = 0;
            _e.body.velocity.x = 0.f;
            break;
        case eBzkState::Landing:
            SetClip(_e, cfg.clipStandUp, true);
            _e.body.velocity.x = 0.f;
            break;
        case eBzkState::Aim:
            FacePlayer(_e, _env);
            _e.aux2 = DyToPlayer(_e, _env) > 0.f ? 1 : 0;
            SetClip(_e, _e.aux2 == 1 ? cfg.clipAimDown : cfg.clipAimUp, true);
            _e.body.velocity.x = 0.f;
            break;
        case eBzkState::ShotReady:
        {
            const int posture = _e.aux;
            int       clip    = cfg.clipReadyF;
            if (posture > 0)
            {
                clip = _e.aux2 == 1 ? cfg.clipReadyDown[posture - 1] : cfg.clipReadyUp[posture - 1];
            }
            SetClip(_e, clip, true);
            _e.body.velocity.x = 0.f;
            break;
        }
        case eBzkState::Shot:
        {
            const int posture = _e.aux;
            int       clip    = cfg.clipShotF;
            if (posture > 0)
            {
                clip = _e.aux2 == 1 ? cfg.clipShotDown[posture - 1] : cfg.clipShotUp[posture - 1];
            }
            SetClip(_e, clip, true);
            break;
        }
        case eBzkState::Recover:
            SetClip(_e, _e.aux2 == 1 ? cfg.clipRecoverDown : cfg.clipRecoverUp, true);
            break;
        case eBzkState::Climb:
            SetClip(_e, cfg.clipMove, true);
            _e.body.velocity = { 0.f, 0.f };
            _e.aux           = 0;
            break;
        default: break;
    }
}

void TickClimb_(
    Enemy&          _e,
    const EnemyDef& _def,
    EnemyEnv&       _env)
{
    if (!ClimbStep(_e, _def))
    {
        return;
    }
    _e.body.bGrounded = false;
    _e.bGrounded      = false;
    Go_(_e, _def, _env, eBzkState::Fall);
    ClimbLeap(_e, _def);
}

[[nodiscard]] int ChoosePosture_(
    const Enemy&    _e,
    const BzkCfg&   _cfg,
    const EnemyEnv& _env,
    bool&           _bDown)
{
    const float cx  = _e.body.position.x;
    const float cy  = _e.body.position.y + _cfg.muzzleBase.y;
    const float dx  = Abs(_env.ctx.player.center.x - cx);
    const float dy  = _env.ctx.player.center.y - cy;
    _bDown          = dy > 0.f;
    const float deg = ATan2(Abs(dy), Max(dx, 0.001f)) * kRad2Deg;
    for (int i = 0; i < 4; ++i)
    {
        if (deg < _cfg.angleStep[i])
        {
            return i;
        }
    }
    return 4;
}

void Init_(
    Enemy&          _e,
    const EnemyDef& _def,
    EnemyEnv&       _env)
{
    PrepareBody(_e, _def);
    Go_(_e, _def, _env, _e.aiVariant >= kVariantClimb ? eBzkState::Climb : eBzkState::Wait);
}

void Tick_(
    Enemy&          _e,
    const EnemyDef& _def,
    EnemyEnv&       _env)
{
    const BzkCfg& cfg   = _def.bzk;
    const float   speed = _def.physics.speed;

    switch (static_cast<eBzkState>(_e.state))
    {
        case eBzkState::Wait:
            if (_e.bActive)
            {
                Go_(_e, _def, _env, eBzkState::Idle);
            }
            break;

        case eBzkState::Fall:
            if (!_e.bLeap)
            {
                _e.body.velocity.x = 0.f;
            }
            break;

        case eBzkState::Idle:
            if (!_e.bGrounded)
            {
                Go_(_e, _def, _env, eBzkState::Fall);
                break;
            }
            FacePlayer(_e, _env);
            if (_e.stateTicks >= cfg.cooldownTicks)
            {
                if (DistToPlayer(_e, _env) > cfg.detect)
                {
                    Go_(_e, _def, _env, eBzkState::Move);
                }
                else
                {
                    Go_(_e, _def, _env, Roll(_env, 3) == 2 ? eBzkState::Move : eBzkState::Aim);
                }
            }
            break;

        case eBzkState::Move:
            if (!_e.bGrounded)
            {
                Go_(_e, _def, _env, eBzkState::Fall);
                break;
            }
            FacePlayer(_e, _env);
            if (DistToPlayer(_e, _env) <= cfg.detect || AtRoofEdge(_e))
            {
                Go_(_e, _def, _env, eBzkState::Aim);
            }
            else if (WallAhead(_e, _env, cfg.wallProbeW, cfg.wallProbeH, cfg.wallProbeX, cfg.wallProbeY))
            {
                Go_(_e, _def, _env, eBzkState::HJump);
            }
            else if (_e.stateTicks >= cfg.cooldownTicks)
            {
                Go_(_e, _def, _env, eBzkState::Idle);
            }
            else
            {
                _e.body.velocity.x = static_cast<float>(_e.facing) * speed;
            }
            break;

        case eBzkState::HJump:
            if (_e.aux3 == 0)
            {
                _e.body.velocity.x = 0.f;
                if (ClipReachedFrame(_e, _def, cfg.jumpFrame))
                {
                    _e.aux3            = 1;
                    _e.body.velocity.y = _def.physics.jumpVy;
                    _e.body.bGrounded  = false;
                    _e.bGrounded       = false;
                    _e.body.velocity.x = static_cast<float>(_e.facing) * speed;
                }
            }
            else
            {
                _e.body.velocity.x = static_cast<float>(_e.facing) * speed;
            }
            break;

        case eBzkState::Landing:
            if (ClipFinished(_e, _def))
            {
                Go_(_e, _def, _env, eBzkState::Idle);
            }
            break;

        case eBzkState::Aim:
            _e.body.velocity.x = 0.f;
            if (ClipFinished(_e, _def))
            {
                bool bDown = false;
                _e.aux     = ChoosePosture_(_e, cfg, _env, bDown);
                _e.aux2    = bDown ? 1 : 0;
                _e.counter = cfg.bullets;
                Go_(_e, _def, _env, eBzkState::ShotReady);
            }
            break;

        case eBzkState::ShotReady:
            _e.body.velocity.x = 0.f;
            if (_e.stateTicks >= cfg.readyTicks)
            {
                Go_(_e, _def, _env, eBzkState::Shot);
            }
            break;

        case eBzkState::Shot:
            _e.body.velocity.x = 0.f;
            if (ClipReachedFrame(_e, _def, cfg.fireFrame))
            {
                const float   deg = 22.5f * static_cast<float>(_e.aux);
                const float   rad = deg * kDeg2Rad;
                const VECTOR2 dir = { std::cos(rad) * static_cast<float>(_e.facing), std::sin(rad) * (_e.aux2 == 1 ? 1.f : -1.f) };
                FireWeapon(_e, _def, _env, cfg.weaponRocket, dir, { dir.x * cfg.muzzleDist, dir.y * cfg.muzzleDist });
                --_e.counter;
            }
            if (ClipFinished(_e, _def))
            {
                Go_(_e, _def, _env, _e.counter > 0 ? eBzkState::ShotReady : eBzkState::Recover);
            }
            break;

        case eBzkState::Recover:
            _e.body.velocity.x = 0.f;
            if (ClipFinished(_e, _def))
            {
                Go_(_e, _def, _env, eBzkState::Idle);
            }
            break;

        case eBzkState::Climb:
            TickClimb_(_e, _def, _env);
            break;

        default: break;
    }

    if (static_cast<eBzkState>(_e.state) != eBzkState::Climb)
    {
        (void)StepPhysics(_e, _def, _env);
        HoldOnRoof(_e);
    }

    switch (static_cast<eBzkState>(_e.state))
    {
        case eBzkState::Fall:
            if (_e.bGrounded)
            {
                _e.bLeap = false;
                Go_(_e, _def, _env, _e.bActive ? eBzkState::Landing : eBzkState::Wait);
            }
            break;
        case eBzkState::HJump:
            if (_e.aux3 == 1 && _e.bGrounded && _e.stateTicks > 0)
            {
                Go_(_e, _def, _env, eBzkState::Landing);
            }
            break;
        default: break;
    }
}

void OnDeath_(
    Enemy&          _e,
    const EnemyDef& _def,
    EnemyEnv&       _env)
{
    BeginSoldierDeath(_e, _def, _env);
}
}   // namespace

const BehaviorOps kBzkOps = {
    .init              = &Init_,
    .tick              = &Tick_,
    .onHit             = nullptr,
    .onDeath           = &OnDeath_,
    .tickDying         = &TickSoldierDeath,
    .registerHurtboxes = nullptr,
};

void ParseBzkCfg(
    DefReader& _r,
    EnemyDef&  _def)
{
    BzkCfg& c = _def.bzk;

    c.cooldownTicks = _r.Ticks("cooldown");
    c.detect        = _r.Num("detect");
    c.bullets       = _r.Int("bullets");
    c.readyTicks    = _r.Ticks("readySec");
    c.jumpFrame     = _r.Int("jumpFrame");
    c.fireFrame     = _r.Int("fireFrame");
    (void)_r.NumArray("angleStep", c.angleStep);
    c.muzzleBase = _r.ReadVec2("muzzleBase", c.muzzleBase);
    c.muzzleDist = _r.Num("muzzleDist");
    {
        const BoxDef probe = _r.Box("wallProbe");
        c.wallProbeW       = probe.w;
        c.wallProbeH       = probe.h;
        c.wallProbeX       = probe.cx;
        c.wallProbeY       = probe.cy;
    }

    c.clipIdle        = _r.Clip("idle");
    c.clipMove        = _r.Clip("move");
    c.clipHJump       = _r.Clip("hJump");
    c.clipStandUp     = _r.Clip("standUp");
    c.clipAimUp       = _r.Clip("aimUp");
    c.clipAimDown     = _r.Clip("aimDown");
    c.clipRecoverUp   = _r.Clip("recoverUp");
    c.clipRecoverDown = _r.Clip("recoverDown");
    c.clipReadyF      = _r.Clip("ready");
    c.clipShotF       = _r.Clip("shot");
    for (int i = 0; i < 4; ++i)
    {
        c.clipReadyUp[i]   = _r.Clip(std::format("readyUp{}", i));
        c.clipReadyDown[i] = _r.Clip(std::format("readyDown{}", i));
        c.clipShotUp[i]    = _r.Clip(std::format("shotUp{}", i));
        c.clipShotDown[i]  = _r.Clip(std::format("shotDown{}", i));
    }
    c.weaponRocket = _r.Weapon("rocket");
}
