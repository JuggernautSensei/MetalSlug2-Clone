#include <cmath>
#include "Base/Math.h"

#include "Game/Enemies/EnemyCommon.h"
#include "Game/Enemies/EnemyDefParse.h"

namespace slug
{

// ===========================================
//  Pow — 포로 (레거시 Older, Docs/EnemySpecs.md §4.3)
//   묶여 있다가 플레이어의 어떤 공격(총알/칼/폭발) 에든 풀려난다 → 순찰 → 플레이어와 겹치면 아이템을 건네고 왼쪽으로 도주한다.
//   Neutral 팀이라 적 탄에는 맞지 않는다. 풀려난 뒤에는 Hurtbox 가 없다 (칼 자동 발동 대상이 아니다).
//   아이템은 ItemDropped 이벤트 (drop = SpawnParams.drop) 로 알린다.
// ===========================================

namespace
{
    enum ePowState : int
    {
        kStuck,
        kEscape,
        kWalk,
        kMoveDown,
        kGlide,
        kClose,
        kRun
    };

    // e.aux  = 방향 반전 타이머 (틱)
    // e.aux2 = 다음 반전까지 간격 (틱)
    // e.aux3 = 순찰 누적 틱

    void Go_(Enemy& _e, const EnemyDef& _def, EnemyEnv& _env, const int _state)
    {
        const PowCfg& cfg = _def.pow;
        EnterState(_e, _state);
        switch (_state)
        {
            case kStuck:
                SetClip(_e, cfg.clipStuck, true);
                _e.body.velocity.x = 0.f;
                break;
            case kEscape:
                SetClip(_e, cfg.clipEscape, true);
                _e.body.velocity.x = 0.f;
                break;
            case kWalk:
                SetClip(_e, cfg.clipMove, true);
                _e.aux  = 0;
                _e.aux2 = cfg.firstFlipTicks;
                break;
            case kMoveDown:
                SetClip(_e, cfg.clipMoveDown, true);
                _e.body.velocity.x  = 0.f;
                _e.body.bUseGravity = false;
                break;
            case kGlide:
                SetClip(_e, cfg.clipGlide, true);
                _e.body.bUseGravity = true;
                break;
            case kClose:
            {
                // 플레이어 반대쪽으로 비켜서 아이템을 건넨다
                const float side = DxToPlayer(_e, _env) < 0.f ? 1.f : -1.f;
                _e.body.position.x += side * cfg.closeNudge;
                _e.facing = DxToPlayer(_e, _env) < 0.f ? -1 : 1;
                SetClip(_e, cfg.clipItem, true);
                _e.body.velocity.x = 0.f;
                break;
            }
            case kRun:
                _e.facing = -1;
                SetClip(_e, cfg.clipRun, true);
                break;
            default: break;
        }
    }

    void Init_(Enemy& _e, const EnemyDef& _def, EnemyEnv& _env)
    {
        PrepareBody(_e, _def);
        Go_(_e, _def, _env, kStuck);
    }

    void Tick_(Enemy& _e, const EnemyDef& _def, EnemyEnv& _env)
    {
        const PowCfg& cfg = _def.pow;

        // 활강 중에는 낙하 속도 상한을 낮춘다
        if (_e.state == kGlide)
        {
            _env.phys.terminalVelocity = cfg.glideMaxFall;
        }

        switch (_e.state)
        {
            case kStuck:
                _e.body.velocity.x = 0.f;
                FacePlayer(_e, _env);
                break;

            case kEscape:
                _e.body.velocity.x = 0.f;
                if (ClipFinished(_e, _def))
                {
                    Go_(_e, _def, _env, kWalk);
                }
                break;

            case kWalk:
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
                    Go_(_e, _def, _env, kGlide);
                }
                else if (!FloorAhead(_e, _env, cfg.edgeProbeDist, cfg.edgeProbeDrop))
                {
                    Go_(_e, _def, _env, kGlide);   // 낭떠러지: 뛰어내려 활강
                    _e.body.velocity.x = static_cast<float>(_e.facing) * cfg.walkSpeed;
                }
                else if (_e.bOnOneWay && _e.aux3 >= cfg.dropMinPatrolTicks && DyToPlayer(_e, _env) >= cfg.dropDy)
                {
                    Go_(_e, _def, _env, kMoveDown);
                }
                else if (_env.ctx.player.bAlive && _e.body.GetBox().Overlaps(_env.ctx.player.bodyBox))
                {
                    Go_(_e, _def, _env, kClose);
                }
                break;
            }

            case kMoveDown:
                _e.body.velocity.x       = 0.f;
                _e.body.velocity.y       = _def.physics.dropSpeed;
                _e.body.dropThroughSteps = 2;
                if (ClipFinished(_e, _def))
                {
                    _e.body.bUseGravity = true;
                    _e.body.velocity.y  = 0.f;
                    Go_(_e, _def, _env, kGlide);
                }
                break;

            case kGlide:
                _e.body.velocity.x = static_cast<float>(_e.facing) * cfg.walkSpeed;
                break;

            case kClose:
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
                    Go_(_e, _def, _env, kRun);
                }
                break;

            case kRun:
            {
                _e.body.velocity.x = -cfg.runSpeed;
                if (_e.stateTicks >= cfg.blinkAtTicks && _e.stateTicks < cfg.blinkAtTicks + cfg.blinkForTicks)
                {
                    _e.bVisible = (((_e.stateTicks - cfg.blinkAtTicks) / slug::Max(1, cfg.blinkPeriodTicks)) % 2) == 0;
                }
                else
                {
                    _e.bVisible = true;
                }
                const float x = _e.body.position.x;
                if (_e.stateTicks >= cfg.removeAtTicks || x < _env.ctx.cameraView.min.x - cfg.offscreenMargin || x > _env.ctx.cameraView.max.x + cfg.offscreenMargin)
                {
                    _e.life = eEnemyLife::Gone;
                }
                break;
            }

            default: break;
        }

        const MoveResult move = StepPhysics(_e, _def, _env);

        if (_e.state == kWalk && (move.bHitWallLeft || move.bHitWallRight))
        {
            _e.facing = -_e.facing;
            _e.aux    = 0;
        }
        if ((_e.state == kGlide || _e.state == kMoveDown) && _e.bGrounded && _e.stateTicks > 1)
        {
            _e.body.bUseGravity = true;
            Go_(_e, _def, _env, kWalk);
        }
    }

    bool OnHit_(Enemy& _e, const EnemyDef&, EnemyEnv& _env, const HitEvent&)
    {
        // 묶여 있는 동안에만 의미가 있다 (풀린 뒤에는 Hurtbox 도 없다). 피해는 항상 무시한다.
        if (_e.state == kStuck)
        {
            Go_(_e, _env.lib.Get(_e.defIndex), _env, kEscape);
            _env.world.PushEvent(_env.world.MakeEvent(_e, eEnemyEventKind::Freed));
        }
        return true;
    }

    void RegisterHurtboxes_(const Enemy& _e, const EnemyDef& _def, CombatSystem& _combat)
    {
        // 떨어지는 중에는 피격되지 않는다: 풀려나면 활강(수평 이동)으로 바뀌므로 착지 후에만 묶인 포로가 된다
        if (_e.state != kStuck || !_e.bGrounded)
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
}

}   // namespace slug
