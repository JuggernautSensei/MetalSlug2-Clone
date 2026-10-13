#include <cmath>
#include <string>

#include "Base/Math.h"
#include "Game/Enemies/EnemyCommon.h"
#include "Game/Enemies/EnemyDefParse.h"

enum class ePowState
{
    Stuck,
    Escape,
    Walk,
    MoveDown,
    Glide,
    Close,
    Run
};

namespace
{
// 외형 변형 클립
[[nodiscard]] int VariantClip_(
    const Enemy&  _e,
    const PowCfg& _c,
    int           _slot,
    int           _base)
{
    const int v = _e.aiVariant;
    if (v >= 1 && v <= PowCfg::kVariantCount)
    {
        const int clip = _c.clipVariants[v - 1][_slot];
        if (clip >= 0)
        {
            return clip;
        }
    }
    return _base;
}

void Go_(
    Enemy&          _e,
    const EnemyDef& _def,
    EnemyEnv&       _env,
    ePowState       _state)
{
    const PowCfg& cfg = _def.pow;
    EnterState(_e, static_cast<int>(_state));
    switch (_state)
    {
        case ePowState::Stuck:
            SetClip(_e, VariantClip_(_e, cfg, 0, cfg.clipStuck), true);
            _e.body.velocity.x = 0.f;
            break;
        case ePowState::Escape:
            SetClip(_e, VariantClip_(_e, cfg, 1, cfg.clipEscape), true);
            _e.body.velocity.x = 0.f;
            break;
        case ePowState::Walk:
            SetClip(_e, VariantClip_(_e, cfg, 2, cfg.clipMove), true);
            _e.aux  = 0;
            _e.aux2 = cfg.firstFlipTicks;
            break;
        case ePowState::MoveDown:
            SetClip(_e, VariantClip_(_e, cfg, 4, cfg.clipMoveDown), true);
            _e.body.velocity.x  = 0.f;
            _e.body.bUseGravity = false;
            break;
        case ePowState::Glide:
            SetClip(_e, VariantClip_(_e, cfg, 3, cfg.clipGlide), true);
            _e.body.bUseGravity = true;
            break;
        case ePowState::Close:
        {
            const float side = DxToPlayer(_e, _env) < 0.f ? 1.f : -1.f;
            _e.body.position.x += side * cfg.closeNudge;
            _e.facing = DxToPlayer(_e, _env) < 0.f ? -1 : 1;
            SetClip(_e, VariantClip_(_e, cfg, 5, cfg.clipItem), true);
            _e.body.velocity.x = 0.f;
            break;
        }
        case ePowState::Run:
            _e.facing = -1;
            SetClip(_e, VariantClip_(_e, cfg, 6, cfg.clipRun), true);
            break;
        default: break;
    }
}

bool TouchingPlayer_(
    Enemy&    _e,
    EnemyEnv& _env)
{
    return _env.ctx.player.bAlive && _e.body.GetBox().Overlaps(_env.ctx.player.bodyBox);
}

void Init_(
    Enemy&          _e,
    const EnemyDef& _def,
    EnemyEnv&       _env)
{
    PrepareBody(_e, _def);
    Go_(_e, _def, _env, ePowState::Stuck);
}

void Tick_(
    Enemy&          _e,
    const EnemyDef& _def,
    EnemyEnv&       _env)
{
    const PowCfg& cfg = _def.pow;

    if (_e.state == static_cast<int>(ePowState::Glide))
    {
        _env.phys.terminalVelocity = cfg.glideMaxFall;
    }

    switch (static_cast<ePowState>(_e.state))
    {
        case ePowState::Stuck:
            _e.body.velocity.x = 0.f;
            FacePlayer(_e, _env);
            if (!_e.bGrounded && TouchingPlayer_(_e, _env))
            {
                Go_(_e, _def, _env, ePowState::Close);
            }
            break;

        case ePowState::Escape:
            _e.body.velocity.x = 0.f;
            if (ClipFinished(_e, _def))
            {
                Go_(_e, _def, _env, ePowState::Walk);
            }
            break;

        case ePowState::Walk:
        {
            ++_e.aux;
            ++_e.aux3;
            if (_e.aux >= _e.aux2)
            {
                _e.facing = -_e.facing;
                _e.aux    = 0;
                _e.aux2   = cfg.flipMinTicks + Roll(_env, 3) * (cfg.flipRangeTicks / 2);
            }
            _e.body.velocity.x = static_cast<float>(_e.facing) * cfg.walkSpeed;

            if (!_e.bGrounded)
            {
                Go_(_e, _def, _env, ePowState::Glide);
            }
            else if (!FloorAhead(_e, _env, cfg.edgeProbeDist, cfg.edgeProbeDrop))
            {
                Go_(_e, _def, _env, ePowState::Glide);
                _e.body.velocity.x = static_cast<float>(_e.facing) * cfg.walkSpeed;
            }
            else if (_e.bOnOneWay && _e.aux3 >= cfg.dropMinPatrolTicks && DyToPlayer(_e, _env) >= cfg.dropDy)
            {
                Go_(_e, _def, _env, ePowState::MoveDown);
            }
            else if (_env.ctx.player.bAlive && _e.body.GetBox().Overlaps(_env.ctx.player.bodyBox))
            {
                Go_(_e, _def, _env, ePowState::Close);
            }
            break;
        }

        case ePowState::MoveDown:
            if (TouchingPlayer_(_e, _env))
            {
                Go_(_e, _def, _env, ePowState::Close);
                break;
            }
            _e.body.velocity.x       = 0.f;
            _e.body.velocity.y       = _def.physics.dropSpeed;
            _e.body.dropThroughSteps = 2;
            if (ClipFinished(_e, _def))
            {
                _e.body.bUseGravity = true;
                _e.body.velocity.y  = 0.f;
                Go_(_e, _def, _env, ePowState::Glide);
            }
            break;

        case ePowState::Glide:
            if (TouchingPlayer_(_e, _env))
            {
                Go_(_e, _def, _env, ePowState::Close);
                break;
            }
            _e.body.velocity.x = static_cast<float>(_e.facing) * cfg.walkSpeed;
            break;

        case ePowState::Close:
            _e.body.velocity.x = 0.f;
            if (ClipReachedFrame(_e, _def, cfg.itemFrame) && !_e.drop.empty())
            {
                EnemyEvent event = _env.world.MakeEvent(_e, eEnemyEventKind::ItemDropped);
                event.pos        = { _e.body.position.x + static_cast<float>(_e.facing) * cfg.itemOffsetX, _e.body.position.y };
                event.drop       = _e.drop;
                _env.world.PushEvent(std::move(event));
            }
            if (ClipFinished(_e, _def))
            {
                Go_(_e, _def, _env, ePowState::Run);
            }
            break;

        case ePowState::Run:
        {
            _e.body.velocity.x = -cfg.runSpeed;
            if (_e.stateTicks >= cfg.blinkAtTicks && _e.stateTicks < cfg.blinkAtTicks + cfg.blinkForTicks)
            {
                _e.bVisible = (((_e.stateTicks - cfg.blinkAtTicks) / Max(1, cfg.blinkPeriodTicks)) % 2) == 0;
            }
            else
            {
                _e.bVisible = true;
            }
            const float x = _e.body.position.x;
            if (_e.stateTicks >= cfg.removeAtTicks
                || x < _env.ctx.cameraView.min.x - cfg.offscreenMargin
                || x > _env.ctx.cameraView.max.x + cfg.offscreenMargin)
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
        _e.facing = -_e.facing;
        _e.aux    = 0;
    }
    if ((_e.state == static_cast<int>(ePowState::Glide) || _e.state == static_cast<int>(ePowState::MoveDown))
        && _e.bGrounded
        && _e.stateTicks > 1)
    {
        _e.body.bUseGravity = true;
        Go_(_e, _def, _env, ePowState::Walk);
    }
}

bool OnHit_(
    Enemy& _e,
    const EnemyDef&,
    EnemyEnv& _env,
    const HitEvent&)
{
    if (_e.state == static_cast<int>(ePowState::Stuck))
    {
        Go_(_e, _env.lib.Get(_e.defIndex), _env, ePowState::Escape);
        _env.world.PushEvent(_env.world.MakeEvent(_e, eEnemyEventKind::Freed));
    }
    return true;
}

void RegisterHurtboxes_(
    const Enemy&    _e,
    const EnemyDef& _def,
    CombatSystem&   _combat)
{
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
}   // namespace

const BehaviorOps kPowOps = {
    .init              = &Init_,
    .tick              = &Tick_,
    .onHit             = &OnHit_,
    .onDeath           = nullptr,
    .tickDying         = nullptr,
    .registerHurtboxes = &RegisterHurtboxes_,
};

void ParsePowCfg(
    DefReader& _r,
    EnemyDef&  _def)
{
    PowCfg& c = _def.pow;

    c.walkSpeed          = _r.Num("walkSpeed");
    c.firstFlipTicks     = _r.Ticks("firstFlipSec");
    c.flipMinTicks       = _r.Ticks("flipMinSec");
    c.flipRangeTicks     = _r.Ticks("flipRangeSec");
    c.glideMaxFall       = _r.Num("glideMaxFall");
    c.dropDy             = _r.Num("dropDy");
    c.dropMinPatrolTicks = _r.Ticks("dropMinPatrolSec");
    c.itemFrame          = _r.Int("itemFrame");
    c.closeNudge         = _r.Num("closeNudge");
    c.itemOffsetX        = _r.Num("itemOffsetX");
    c.runSpeed           = _r.Num("runSpeed");
    c.blinkAtTicks       = _r.Ticks("blinkAtSec");
    c.blinkForTicks      = _r.Ticks("blinkForSec");
    c.blinkPeriodTicks   = _r.Ticks("blinkPeriodSec");
    c.removeAtTicks      = _r.Ticks("removeAtSec");
    c.offscreenMargin    = _r.Num("offscreenMargin");
    c.edgeProbeDist      = _r.Num("edgeProbeDist");
    c.edgeProbeDrop      = _r.Num("edgeProbeDrop");

    c.clipStuck    = _r.Clip("stuck");
    c.clipEscape   = _r.Clip("escape");
    c.clipMove     = _r.Clip("move");
    c.clipGlide    = _r.Clip("glide");
    c.clipMoveDown = _r.Clip("moveDown");
    c.clipItem     = _r.Clip("item");
    c.clipRun      = _r.Clip("run");

    constexpr ARRAY<const char*, PowCfg::kSlotCount> kSlotKeys = { "stuck", "escape", "move", "glide", "moveDown", "item", "run" };
    for (int v = 0; v < PowCfg::kVariantCount; ++v)
    {
        for (int s = 0; s < PowCfg::kSlotCount; ++s)
        {
            const String key     = String(kSlotKeys[s]) + "_v" + std::to_string(v + 1);
            c.clipVariants[v][s] = _r.ClipOpt(key);
        }
    }
}
