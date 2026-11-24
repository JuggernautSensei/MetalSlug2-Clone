#include "Game/Enemies/BzkEnemy.h"

#include <cmath>
#include <format>

#include "Game/Enemies/EnemyCommon.h"
#include "Game/Enemies/EnemyDefParse.h"

constexpr int kVariantClimb = 2;

void BzkEnemy::Init(
    const EnemyDef& _def,
    EnemyEnv&       _env)
{
    EnemyState& _e = GetMutableState();
    PrepareBody(_e, _def);
    Go_(_def, _env, _e.aiVariant >= kVariantClimb ? eBzkState::Climb : eBzkState::Wait);
}

void BzkEnemy::Tick(
    const EnemyDef& _def,
    EnemyEnv&       _env)
{
    EnemyState&      _e     = GetMutableState();
    const BzkConfig& config = _def.bzk;
    const float      speed  = _def.physics.speed;

    switch (static_cast<eBzkState>(_e.state))
    {
        case eBzkState::Wait:
            if (_e.bActive)
            {
                Go_(_def, _env, eBzkState::Idle);
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
                Go_(_def, _env, eBzkState::Fall);
                break;
            }
            FacePlayer(_e, _env);
            if (_e.stateTicks >= config.cooldownTicks)
            {
                if (DistToPlayer(_e, _env) > config.detect)
                {
                    Go_(_def, _env, eBzkState::Move);
                }
                else
                {
                    Go_(_def, _env, Roll(_env, 3) == 2 ? eBzkState::Move : eBzkState::Aim);
                }
            }
            break;

        case eBzkState::Move:
            if (!_e.bGrounded)
            {
                Go_(_def, _env, eBzkState::Fall);
                break;
            }
            FacePlayer(_e, _env);
            if (DistToPlayer(_e, _env) <= config.detect || AtRoofEdge(_e))
            {
                Go_(_def, _env, eBzkState::Aim);
            }
            else if (WallAhead(_e, _env, config.wallProbeW, config.wallProbeH, config.wallProbeX, config.wallProbeY))
            {
                Go_(_def, _env, eBzkState::HJump);
            }
            else if (_e.stateTicks >= config.cooldownTicks)
            {
                Go_(_def, _env, eBzkState::Idle);
            }
            else
            {
                _e.body.velocity.x = static_cast<float>(_e.facing) * speed;
            }
            break;

        case eBzkState::HJump:
            if (m_jumpStarted == 0)
            {
                _e.body.velocity.x = 0.f;
                if (ClipReachedFrame(_e, _def, config.jumpFrame))
                {
                    m_jumpStarted      = 1;
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
                Go_(_def, _env, eBzkState::Idle);
            }
            break;

        case eBzkState::Aim:
            _e.body.velocity.x = 0.f;
            if (ClipFinished(_e, _def))
            {
                bool bDown       = false;
                m_aimPosture     = ChoosePosture_(config, _env, bDown);
                m_bHasAimPosture = true;
                m_aimDown        = bDown ? 1 : 0;
                m_shotsRemaining = config.bullets;
                Go_(_def, _env, eBzkState::ShotReady);
            }
            break;

        case eBzkState::ShotReady:
            _e.body.velocity.x = 0.f;
            if (_e.stateTicks >= config.readyTicks)
            {
                Go_(_def, _env, eBzkState::Shot);
            }
            break;

        case eBzkState::Shot:
            _e.body.velocity.x = 0.f;
            if (ClipReachedFrame(_e, _def, config.fireFrame))
            {
                const float   deg = 22.5f * static_cast<float>(m_aimPosture);
                const float   rad = deg * kDeg2Rad;
                const VECTOR2 dir = { std::cos(rad) * static_cast<float>(_e.facing), std::sin(rad) * (m_aimDown == 1 ? 1.f : -1.f) };
                FireWeapon(_e, _def, _env, config.weaponRocket, dir, { dir.x * config.muzzleDist, dir.y * config.muzzleDist });
                --m_shotsRemaining;
            }
            if (ClipFinished(_e, _def))
            {
                Go_(_def, _env, m_shotsRemaining > 0 ? eBzkState::ShotReady : eBzkState::Recover);
            }
            break;

        case eBzkState::Recover:
            _e.body.velocity.x = 0.f;
            if (ClipFinished(_e, _def))
            {
                Go_(_def, _env, eBzkState::Idle);
            }
            break;

        case eBzkState::Climb:
            TickClimb_(_def, _env);
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
                Go_(_def, _env, _e.bActive ? eBzkState::Landing : eBzkState::Wait);
            }
            break;
        case eBzkState::HJump:
            if (m_jumpStarted == 1 && _e.bGrounded && _e.stateTicks > 0)
            {
                Go_(_def, _env, eBzkState::Landing);
            }
            break;
        default: break;
    }
}

void BzkEnemy::OnDeath(
    const EnemyDef& _def,
    EnemyEnv&       _env)
{
    EnemyState& _e = GetMutableState();
    BeginSoldierDeath(_e, _def, _env);
}

void BzkEnemy::TickDying(
    const EnemyDef& _def,
    EnemyEnv&       _env)
{
    TickSoldierDeath(GetMutableState(), _def, _env);
}

void ParseBzkConfig(
    DefReader& _r,
    EnemyDef&  _def)
{
    BzkConfig& config = _def.bzk;

    config.cooldownTicks = _r.Ticks("cooldown");
    config.detect        = _r.Num("detect");
    config.bullets       = _r.Int("bullets");
    config.readyTicks    = _r.Ticks("readySec");
    config.jumpFrame     = _r.Int("jumpFrame");
    config.fireFrame     = _r.Int("fireFrame");
    (void)_r.NumArray("angleStep", config.angleStep);
    config.muzzleBase = _r.ReadVec2("muzzleBase", config.muzzleBase);
    config.muzzleDist = _r.Num("muzzleDist");
    {
        const BoxDef probe = _r.Box("wallProbe");
        config.wallProbeW  = probe.w;
        config.wallProbeH  = probe.h;
        config.wallProbeX  = probe.cx;
        config.wallProbeY  = probe.cy;
    }

    config.clipIdle        = _r.Clip("idle");
    config.clipMove        = _r.Clip("move");
    config.clipHJump       = _r.Clip("hJump");
    config.clipStandUp     = _r.Clip("standUp");
    config.clipAimUp       = _r.Clip("aimUp");
    config.clipAimDown     = _r.Clip("aimDown");
    config.clipRecoverUp   = _r.Clip("recoverUp");
    config.clipRecoverDown = _r.Clip("recoverDown");
    config.clipReadyF      = _r.Clip("ready");
    config.clipShotF       = _r.Clip("shot");
    for (int i = 0; i < 4; ++i)
    {
        config.clipReadyUp[i]   = _r.Clip(std::format("readyUp{}", i));
        config.clipReadyDown[i] = _r.Clip(std::format("readyDown{}", i));
        config.clipShotUp[i]    = _r.Clip(std::format("shotUp{}", i));
        config.clipShotDown[i]  = _r.Clip(std::format("shotDown{}", i));
    }
    config.weaponRocket = _r.Weapon("rocket");
}

void BzkEnemy::Go_(
    const EnemyDef& _def,
    EnemyEnv&       _env,
    const eBzkState _state)
{
    EnemyState&      _e     = GetMutableState();
    const BzkConfig& config = _def.bzk;
    EnterState(_e, static_cast<int>(_state));
    switch (_state)
    {
        case eBzkState::Wait:
            SetClip(_e, config.clipIdle, true);
            _e.body.velocity.x = 0.f;
            break;
        case eBzkState::Fall:
            SetClip(_e, config.clipHJump, true);
            break;
        case eBzkState::Idle:
            FacePlayer(_e, _env);
            SetClip(_e, config.clipIdle, true);
            _e.body.velocity.x = 0.f;
            break;
        case eBzkState::Move:
            FacePlayer(_e, _env);
            SetClip(_e, config.clipMove, true);
            break;
        case eBzkState::HJump:
            FacePlayer(_e, _env);
            SetClip(_e, config.clipHJump, true);
            m_jumpStarted      = 0;
            _e.body.velocity.x = 0.f;
            break;
        case eBzkState::Landing:
            SetClip(_e, config.clipStandUp, true);
            _e.body.velocity.x = 0.f;
            break;
        case eBzkState::Aim:
            FacePlayer(_e, _env);
            m_aimDown = DyToPlayer(_e, _env) > 0.f ? 1 : 0;
            SetClip(_e, m_aimDown == 1 ? config.clipAimDown : config.clipAimUp, true);
            _e.body.velocity.x = 0.f;
            break;
        case eBzkState::ShotReady:
        {
            const int posture = m_aimPosture;
            int       clip    = config.clipReadyF;
            if (posture > 0)
            {
                clip = m_aimDown == 1 ? config.clipReadyDown[posture - 1] : config.clipReadyUp[posture - 1];
            }
            SetClip(_e, clip, true);
            _e.body.velocity.x = 0.f;
            break;
        }
        case eBzkState::Shot:
        {
            const int posture = m_aimPosture;
            int       clip    = config.clipShotF;
            if (posture > 0)
            {
                clip = m_aimDown == 1 ? config.clipShotDown[posture - 1] : config.clipShotUp[posture - 1];
            }
            SetClip(_e, clip, true);
            break;
        }
        case eBzkState::Recover:
            SetClip(_e, m_aimDown == 1 ? config.clipRecoverDown : config.clipRecoverUp, true);
            break;
        case eBzkState::Climb:
            SetClip(_e, config.clipMove, true);
            _e.body.velocity = { 0.f, 0.f };
            _e.climbSegment  = 0;
            m_bHasAimPosture = false;
            break;
        default: break;
    }
}

void BzkEnemy::TickClimb_(
    const EnemyDef& _def,
    EnemyEnv&       _env)
{
    EnemyState& _e = GetMutableState();
    if (!ClimbStep(_e, _def))
    {
        return;
    }
    _e.body.bGrounded = false;
    _e.bGrounded      = false;
    Go_(_def, _env, eBzkState::Fall);
    ClimbLeap(_e, _def);
}

[[nodiscard]] int BzkEnemy::ChoosePosture_(
    const BzkConfig& _config,
    const EnemyEnv&  _env,
    bool&            _bDown) const
{
    const EnemyState& _e = GetState();
    const float       cx = _e.body.position.x;
    const float       cy = _e.body.position.y + _config.muzzleBase.y;
    const float       dx = Abs(_env.ctx.player.center.x - cx);
    const float       dy = _env.ctx.player.center.y - cy;
    _bDown               = dy > 0.f;
    const float deg      = ATan2(Abs(dy), Max(dx, 0.001f)) * kRad2Deg;
    for (int i = 0; i < 4; ++i)
    {
        if (deg < _config.angleStep[i])
        {
            return i;
        }
    }
    return 4;
}

void BzkEnemy::HashActionState(Fnv1a& _hash) const
{
    HashActionState_(_hash, m_shotsRemaining, m_bHasAimPosture ? m_aimPosture : GetState().climbSegment, m_aimDown, m_jumpStarted);
}
