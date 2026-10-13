#include "Game/Enemies/PowEnemy.h"

#include <cmath>
#include <string>

#include "Game/Enemies/EnemyCommon.h"
#include "Game/Enemies/EnemyDefParse.h"

void PowEnemy::Init(
    const EnemyDef& _def,
    EnemyEnv&       _env)
{
    EnemyState& _e = GetMutableState();
    PrepareBody(_e, _def);
    Go_(_def, _env, ePowState::Stuck);
}

void PowEnemy::Tick(
    const EnemyDef& _def,
    EnemyEnv&       _env)
{
    EnemyState&      _e     = GetMutableState();
    const PowConfig& config = _def.pow;

    if (_e.state == static_cast<int>(ePowState::Glide))
    {
        _env.phys.terminalVelocity = config.glideMaxFall;
    }

    switch (static_cast<ePowState>(_e.state))
    {
        case ePowState::Stuck:
            _e.body.velocity.x = 0.f;
            FacePlayer(_e, _env);
            if (!_e.bGrounded && TouchingPlayer_(_env))
            {
                Go_(_def, _env, ePowState::Close);
            }
            break;

        case ePowState::Escape:
            _e.body.velocity.x = 0.f;
            if (ClipFinished(_e, _def))
            {
                Go_(_def, _env, ePowState::Walk);
            }
            break;

        case ePowState::Walk:
        {
            ++m_turnTicks;
            ++m_patrolTicks;
            if (m_turnTicks >= m_turnDelayTicks)
            {
                _e.facing        = -_e.facing;
                m_turnTicks      = 0;
                m_turnDelayTicks = config.flipMinTicks + Roll(_env, 3) * (config.flipRangeTicks / 2);
            }
            _e.body.velocity.x = static_cast<float>(_e.facing) * config.walkSpeed;

            if (!_e.bGrounded)
            {
                Go_(_def, _env, ePowState::Glide);
            }
            else if (!FloorAhead(_e, _env, config.edgeProbeDist, config.edgeProbeDrop))
            {
                Go_(_def, _env, ePowState::Glide);
                _e.body.velocity.x = static_cast<float>(_e.facing) * config.walkSpeed;
            }
            else if (_e.bOnOneWay && m_patrolTicks >= config.dropMinPatrolTicks && DyToPlayer(_e, _env) >= config.dropDy)
            {
                Go_(_def, _env, ePowState::MoveDown);
            }
            else if (_env.ctx.player.bAlive && _e.body.GetBox().Overlaps(_env.ctx.player.bodyBox))
            {
                Go_(_def, _env, ePowState::Close);
            }
            break;
        }

        case ePowState::MoveDown:
            if (TouchingPlayer_(_env))
            {
                Go_(_def, _env, ePowState::Close);
                break;
            }
            _e.body.velocity.x       = 0.f;
            _e.body.velocity.y       = _def.physics.dropSpeed;
            _e.body.dropThroughSteps = 2;
            if (ClipFinished(_e, _def))
            {
                _e.body.bUseGravity = true;
                _e.body.velocity.y  = 0.f;
                Go_(_def, _env, ePowState::Glide);
            }
            break;

        case ePowState::Glide:
            if (TouchingPlayer_(_env))
            {
                Go_(_def, _env, ePowState::Close);
                break;
            }
            _e.body.velocity.x = static_cast<float>(_e.facing) * config.walkSpeed;
            break;

        case ePowState::Close:
            _e.body.velocity.x = 0.f;
            if (ClipReachedFrame(_e, _def, config.itemFrame) && !_e.drop.empty())
            {
                EnemyEvent event = _env.world.MakeEvent(_e, eEnemyEventKind::ItemDropped);
                event.pos        = { _e.body.position.x + static_cast<float>(_e.facing) * config.itemOffsetX, _e.body.position.y };
                event.drop       = _e.drop;
                _env.world.PushEvent(std::move(event));
            }
            if (ClipFinished(_e, _def))
            {
                Go_(_def, _env, ePowState::Run);
            }
            break;

        case ePowState::Run:
        {
            _e.body.velocity.x = -config.runSpeed;
            if (_e.stateTicks >= config.blinkAtTicks && _e.stateTicks < config.blinkAtTicks + config.blinkForTicks)
            {
                _e.bVisible = (((_e.stateTicks - config.blinkAtTicks) / Max(1, config.blinkPeriodTicks)) % 2) == 0;
            }
            else
            {
                _e.bVisible = true;
            }
            const float x = _e.body.position.x;
            if (_e.stateTicks >= config.removeAtTicks
                || x < _env.ctx.cameraView.min.x - config.offscreenMargin
                || x > _env.ctx.cameraView.max.x + config.offscreenMargin)
            {
                _e.life = eEnemyLife::Gone;
            }
            break;
        }

        default: break;
    }

    const MoveResult move = StepPhysics(_e, _def, _env);

    if (_e.state == static_cast<int>(ePowState::Walk) && (move.bHitWallLeft || move.bHitWallRight))
    {
        _e.facing   = -_e.facing;
        m_turnTicks = 0;
    }
    if ((_e.state == static_cast<int>(ePowState::Glide) || _e.state == static_cast<int>(ePowState::MoveDown))
        && _e.bGrounded
        && _e.stateTicks > 1)
    {
        _e.body.bUseGravity = true;
        Go_(_def, _env, ePowState::Walk);
    }
}

bool PowEnemy::OnHit(
    const EnemyDef&,
    EnemyEnv& _env,
    const HitEvent&)
{
    EnemyState& _e = GetMutableState();
    if (_e.state == static_cast<int>(ePowState::Stuck))
    {
        Go_(_env.lib.Get(_e.defIndex), _env, ePowState::Escape);
        _env.world.PushEvent(_env.world.MakeEvent(_e, eEnemyEventKind::Freed));
    }
    return true;
}

void PowEnemy::RegisterHurtboxes(
    const EnemyDef& _def,
    CombatSystem&   _combat) const
{
    const EnemyState& _e = GetState();
    if (_e.state != static_cast<int>(ePowState::Stuck) || !_e.bGrounded)
    {
        return;
    }
    Hurtbox hurtbox;
    hurtbox.box   = _def.hurtbox.At(_e.body.position, _e.facing);
    hurtbox.team  = _def.team;
    hurtbox.owner = _e.id;
    _combat.AddHurtbox(hurtbox);
}

void ParsePowConfig(
    DefReader& _r,
    EnemyDef&  _def)
{
    PowConfig& config = _def.pow;

    config.walkSpeed          = _r.Num("walkSpeed");
    config.firstFlipTicks     = _r.Ticks("firstFlipSec");
    config.flipMinTicks       = _r.Ticks("flipMinSec");
    config.flipRangeTicks     = _r.Ticks("flipRangeSec");
    config.glideMaxFall       = _r.Num("glideMaxFall");
    config.dropDy             = _r.Num("dropDy");
    config.dropMinPatrolTicks = _r.Ticks("dropMinPatrolSec");
    config.itemFrame          = _r.Int("itemFrame");
    config.closeNudge         = _r.Num("closeNudge");
    config.itemOffsetX        = _r.Num("itemOffsetX");
    config.runSpeed           = _r.Num("runSpeed");
    config.blinkAtTicks       = _r.Ticks("blinkAtSec");
    config.blinkForTicks      = _r.Ticks("blinkForSec");
    config.blinkPeriodTicks   = _r.Ticks("blinkPeriodSec");
    config.removeAtTicks      = _r.Ticks("removeAtSec");
    config.offscreenMargin    = _r.Num("offscreenMargin");
    config.edgeProbeDist      = _r.Num("edgeProbeDist");
    config.edgeProbeDrop      = _r.Num("edgeProbeDrop");

    config.clipStuck    = _r.Clip("stuck");
    config.clipEscape   = _r.Clip("escape");
    config.clipMove     = _r.Clip("move");
    config.clipGlide    = _r.Clip("glide");
    config.clipMoveDown = _r.Clip("moveDown");
    config.clipItem     = _r.Clip("item");
    config.clipRun      = _r.Clip("run");

    constexpr ARRAY<const char*, PowConfig::kSlotCount> kSlotKeys = { "stuck", "escape", "move", "glide", "moveDown", "item", "run" };
    for (int v = 0; v < PowConfig::kVariantCount; ++v)
    {
        for (int s = 0; s < PowConfig::kSlotCount; ++s)
        {
            const String key          = String(kSlotKeys[s]) + "_v" + std::to_string(v + 1);
            config.clipVariants[v][s] = _r.ClipOpt(key);
        }
    }
}

[[nodiscard]] int PowEnemy::VariantClip_(
    const PowConfig& _config,
    const int        _slot,
    const int        _base) const
{
    const EnemyState& _e = GetState();
    const int         v  = _e.aiVariant;
    if (v >= 1 && v <= PowConfig::kVariantCount)
    {
        const int clip = _config.clipVariants[v - 1][_slot];
        if (clip >= 0)
        {
            return clip;
        }
    }
    return _base;
}

void PowEnemy::Go_(
    const EnemyDef& _def,
    EnemyEnv&       _env,
    const ePowState _state)
{
    EnemyState&      _e     = GetMutableState();
    const PowConfig& config = _def.pow;
    EnterState(_e, static_cast<int>(_state));
    switch (_state)
    {
        case ePowState::Stuck:
            SetClip(_e, VariantClip_(config, 0, config.clipStuck), true);
            _e.body.velocity.x = 0.f;
            break;
        case ePowState::Escape:
            SetClip(_e, VariantClip_(config, 1, config.clipEscape), true);
            _e.body.velocity.x = 0.f;
            break;
        case ePowState::Walk:
            SetClip(_e, VariantClip_(config, 2, config.clipMove), true);
            m_turnTicks      = 0;
            m_turnDelayTicks = config.firstFlipTicks;
            break;
        case ePowState::MoveDown:
            SetClip(_e, VariantClip_(config, 4, config.clipMoveDown), true);
            _e.body.velocity.x  = 0.f;
            _e.body.bUseGravity = false;
            break;
        case ePowState::Glide:
            SetClip(_e, VariantClip_(config, 3, config.clipGlide), true);
            _e.body.bUseGravity = true;
            break;
        case ePowState::Close:
        {
            const float side = DxToPlayer(_e, _env) < 0.f ? 1.f : -1.f;
            _e.body.position.x += side * config.closeNudge;
            _e.facing = DxToPlayer(_e, _env) < 0.f ? -1 : 1;
            SetClip(_e, VariantClip_(config, 5, config.clipItem), true);
            _e.body.velocity.x = 0.f;
            break;
        }
        case ePowState::Run:
            _e.facing = -1;
            SetClip(_e, VariantClip_(config, 6, config.clipRun), true);
            break;
        default: break;
    }
}

bool PowEnemy::TouchingPlayer_(
    EnemyEnv& _env)
{
    EnemyState& _e = GetMutableState();
    return _env.ctx.player.bAlive && _e.body.GetBox().Overlaps(_env.ctx.player.bodyBox);
}

void PowEnemy::HashActionState(Fnv1a& _hash) const
{
    HashActionState_(_hash, 0, m_turnTicks, m_turnDelayTicks, m_patrolTicks);
}
