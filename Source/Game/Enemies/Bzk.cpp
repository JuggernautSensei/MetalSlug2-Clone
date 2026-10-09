#include <cmath>
#include <format>

#include <JugX/Math.h>

#include "Game/Enemies/EnemyCommon.h"
#include "Game/Enemies/EnemyDefParse.h"

namespace slug
{

// ===========================================
//  Bzk — 바주카 병사 (Docs/EnemySpecs.md §1.2)
//   플레이어 방향을 5구간 자세(수평 / 22.5 / 45 / 67.5 / 90 도, 위 또는 아래) 로 조준하고 2발을 쏜다.
//   두 번째 발은 각도를 다시 계산하지 않는다 (레거시 의도 유지).
// ===========================================

namespace
{
    enum eBzkState : int
    {
        kWait,
        kFall,
        kIdle,
        kMove,
        kHJump,
        kLanding,
        kAim,
        kShotReady,
        kShot,
        kRecover
    };

    // e.aux  = 자세 (0 수평, 1..4 = 22.5 x n 도)
    // e.aux2 = 0 위쪽 / 1 아래쪽
    // e.aux3 = HJump 도약 여부
    // e.counter = 남은 발수

    void Go_(Enemy& _e, const EnemyDef& _def, EnemyEnv& _env, const int _state)
    {
        const BzkCfg& cfg = _def.bzk;
        EnterState(_e, _state);
        switch (_state)
        {
            case kWait:
                SetClip(_e, cfg.clipIdle, true);
                _e.body.velocity.x = 0.f;
                break;
            case kFall:
                SetClip(_e, cfg.clipHJump, true);
                break;
            case kIdle:
                FacePlayer(_e, _env);
                SetClip(_e, cfg.clipIdle, true);
                _e.body.velocity.x = 0.f;
                break;
            case kMove:
                FacePlayer(_e, _env);
                SetClip(_e, cfg.clipMove, true);
                break;
            case kHJump:
                FacePlayer(_e, _env);
                SetClip(_e, cfg.clipHJump, true);
                _e.aux3 = 0;
                _e.body.velocity.x = 0.f;
                break;
            case kLanding:
                SetClip(_e, cfg.clipStandUp, true);
                _e.body.velocity.x = 0.f;
                break;
            case kAim:
                FacePlayer(_e, _env);
                _e.aux2 = DyToPlayer(_e, _env) > 0.f ? 1 : 0;
                SetClip(_e, _e.aux2 == 1 ? cfg.clipAimDown : cfg.clipAimUp, true);
                _e.body.velocity.x = 0.f;
                break;
            case kShotReady:
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
            case kShot:
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
            case kRecover:
                SetClip(_e, _e.aux2 == 1 ? cfg.clipRecoverDown : cfg.clipRecoverUp, true);
                break;
            default: break;
        }
    }

    // 플레이어 방향 → 자세 (0..4). 중심(몸통 중심) 에서 플레이어 중심까지의 수평 대비 각도.
    [[nodiscard]] int ChoosePosture_(const Enemy& _e, const BzkCfg& _cfg, const EnemyEnv& _env, bool& _bDown)
    {
        const float cx = _e.body.position.x;
        const float cy = _e.body.position.y + _cfg.muzzleBase.y;   // 총구 높이 기준
        const float dx = std::abs(_env.ctx.player.center.x - cx);
        const float dy = _env.ctx.player.center.y - cy;
        _bDown         = dy > 0.f;
        const float deg = std::atan2(std::abs(dy), jug::Max(dx, 0.001f)) * jug::kRad2Deg;
        for (int i = 0; i < 4; ++i)
        {
            if (deg < _cfg.angleStep[i])
            {
                return i;
            }
        }
        return 4;
    }

    void Init_(Enemy& _e, const EnemyDef& _def, EnemyEnv& _env)
    {
        PrepareBody(_e, _def);
        Go_(_e, _def, _env, kWait);
    }

    void Tick_(Enemy& _e, const EnemyDef& _def, EnemyEnv& _env)
    {
        const BzkCfg& cfg   = _def.bzk;
        const float   speed = _def.physics.speed;

        switch (_e.state)
        {
            case kWait:
                if (_e.bActive)
                {
                    Go_(_e, _def, _env, kIdle);
                }
                break;

            case kFall:
                _e.body.velocity.x = 0.f;
                break;

            case kIdle:
                if (!_e.bGrounded)
                {
                    Go_(_e, _def, _env, kFall);
                    break;
                }
                FacePlayer(_e, _env);
                if (_e.stateTicks >= cfg.cooldownTicks)
                {
                    if (DistToPlayer(_e, _env) > cfg.detect)
                    {
                        Go_(_e, _def, _env, kMove);
                    }
                    else
                    {
                        Go_(_e, _def, _env, Roll(_env, 3) == 2 ? kMove : kAim);
                    }
                }
                break;

            case kMove:
                if (!_e.bGrounded)
                {
                    Go_(_e, _def, _env, kFall);
                    break;
                }
                FacePlayer(_e, _env);
                if (DistToPlayer(_e, _env) <= cfg.detect)
                {
                    Go_(_e, _def, _env, kAim);
                }
                else if (WallAhead(_e, _env, cfg.wallProbeW, cfg.wallProbeH, cfg.wallProbeX, cfg.wallProbeY))
                {
                    Go_(_e, _def, _env, kHJump);
                }
                else if (_e.stateTicks >= cfg.cooldownTicks)
                {
                    Go_(_e, _def, _env, kIdle);
                }
                else
                {
                    _e.body.velocity.x = static_cast<float>(_e.facing) * speed;
                }
                break;

            case kHJump:
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

            case kLanding:
                if (ClipFinished(_e, _def))
                {
                    Go_(_e, _def, _env, kIdle);
                }
                break;

            case kAim:
                _e.body.velocity.x = 0.f;
                if (ClipFinished(_e, _def))
                {
                    bool bDown = false;
                    _e.aux     = ChoosePosture_(_e, cfg, _env, bDown);
                    _e.aux2    = bDown ? 1 : 0;
                    _e.counter = cfg.bullets;
                    Go_(_e, _def, _env, kShotReady);
                }
                break;

            case kShotReady:
                _e.body.velocity.x = 0.f;
                if (_e.stateTicks >= cfg.readyTicks)
                {
                    Go_(_e, _def, _env, kShot);
                }
                break;

            case kShot:
                _e.body.velocity.x = 0.f;
                if (ClipReachedFrame(_e, _def, cfg.fireFrame))
                {
                    const float deg = 22.5f * static_cast<float>(_e.aux);
                    const float rad = deg * jug::kDeg2Rad;
                    const jug::VECTOR2 dir = { std::cos(rad) * static_cast<float>(_e.facing), std::sin(rad) * (_e.aux2 == 1 ? 1.f : -1.f) };
                    FireWeapon(_e, _def, _env, cfg.weaponRocket, dir, { dir.x * cfg.muzzleDist, dir.y * cfg.muzzleDist });
                    --_e.counter;
                }
                if (ClipFinished(_e, _def))
                {
                    Go_(_e, _def, _env, _e.counter > 0 ? kShotReady : kRecover);
                }
                break;

            case kRecover:
                _e.body.velocity.x = 0.f;
                if (ClipFinished(_e, _def))
                {
                    Go_(_e, _def, _env, kIdle);
                }
                break;

            default: break;
        }

        (void)StepPhysics(_e, _def, _env);

        switch (_e.state)
        {
            case kFall:
                if (_e.bGrounded)
                {
                    Go_(_e, _def, _env, _e.bActive ? kLanding : kWait);
                }
                break;
            case kHJump:
                if (_e.aux3 == 1 && _e.bGrounded && _e.stateTicks > 0)
                {
                    Go_(_e, _def, _env, kLanding);
                }
                break;
            default: break;
        }
    }

    void OnDeath_(Enemy& _e, const EnemyDef& _def, EnemyEnv& _env)
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
    (void)_r.NumArray("angleStep", c.angleStep, 4);
    c.muzzleBase    = _r.Vec2("muzzleBase", c.muzzleBase);
    c.muzzleDist    = _r.Num("muzzleDist");
    {
        const BoxDef probe = _r.Box("wallProbe");
        c.wallProbeW = probe.w;
        c.wallProbeH = probe.h;
        c.wallProbeX = probe.cx;
        c.wallProbeY = probe.cy;
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

}   // namespace slug
