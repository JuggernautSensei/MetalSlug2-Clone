#include <cmath>
#include "Base/Types.h"

#include "Game/Enemies/EnemyCommon.h"
#include "Game/Enemies/EnemyDefParse.h"

namespace slug
{
namespace
{
    enum eSoldierState : int
    {
        kWait,
        kFall,
        kLand,
        kIdle,
        kMove,
        kMoveDown,
        kHJump,
        kThrow,
        kSlash,
        kJumpSlashReady,
        kJumpSlash,
        kJumpSlashLand
    };

    void Go_(Enemy& _e, const EnemyDef& _def, EnemyEnv& _env, int _state);

    [[nodiscard]] float AbsDx_(const Enemy& _e, const EnemyEnv& _env)
    {
        return std::abs(DxToPlayer(_e, _env));
    }

    void Reposition_(Enemy& _e, const EnemyDef& _def, EnemyEnv& _env, const int _fallback)
    {
        const SoldierCfg& cfg  = _def.soldier;
        const float       dy   = DyToPlayer(_e, _env);
        if (_e.bGrounded && _e.bOnOneWay && dy > cfg.dropDy)
        {
            Go_(_e, _def, _env, kMoveDown);
        }
        else if (_e.bGrounded && dy < -cfg.jumpDy)
        {
            Go_(_e, _def, _env, kHJump);
        }
        else
        {
            Go_(_e, _def, _env, _fallback);
        }
    }

    void Go_(Enemy& _e, const EnemyDef& _def, EnemyEnv& _env, int _state)
    {
        const SoldierCfg& cfg = _def.soldier;
        if (_state == kMove && _e.aiVariant == 1)
        {
            _state = kIdle;
        }

        EnterState(_e, _state);
        switch (_state)
        {
            case kWait:
                SetClip(_e, cfg.clipIdle, true);
                _e.body.velocity.x = 0.f;
                break;
            case kFall:
                SetClip(_e, cfg.clipFall, true);
                break;
            case kLand:
                SetClip(_e, cfg.clipLand, true);
                _e.body.velocity.x = 0.f;
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
            case kMoveDown:
                SetClip(_e, cfg.clipMoveDown, true);
                _e.body.velocity.x  = 0.f;
                _e.body.bUseGravity = false;
                break;
            case kHJump:
                FacePlayer(_e, _env);
                SetClip(_e, cfg.clipHJump, true);
                _e.body.velocity.y = _def.physics.jumpVy;
                _e.body.bGrounded  = false;
                _e.bGrounded       = false;
                break;
            case kThrow:
                FacePlayer(_e, _env);
                SetClip(_e, cfg.clipThrow, true);
                _e.body.velocity.x = 0.f;
                break;
            case kSlash:
                FacePlayer(_e, _env);
                SetClip(_e, cfg.clipSlash, true);
                _e.body.velocity.x = 0.f;
                break;
            case kJumpSlashReady:
                FacePlayer(_e, _env);
                SetClip(_e, cfg.clipJumpSlashReady, true);
                _e.body.velocity.y = _def.physics.jumpVy;
                _e.body.bGrounded  = false;
                _e.bGrounded       = false;
                break;
            case kJumpSlash:
                SetClip(_e, cfg.clipJumpSlash, true);
                break;
            case kJumpSlashLand:
                SetClip(_e, cfg.clipJumpSlashLand, true);
                _e.body.velocity.x = 0.f;
                break;
            default: break;
        }
    }

    void DecideBomb_(Enemy& _e, const EnemyDef& _def, EnemyEnv& _env)
    {
        const SoldierCfg& cfg   = _def.soldier;
        const float       absDx = AbsDx_(_e, _env);

        if (_e.state == kIdle)
        {
            if (_e.stateTicks < cfg.cooldownTicks)
            {
                return;
            }
            if (absDx >= cfg.detect)
            {
                if (Roll(_env, 2) == 0)
                {
                    Go_(_e, _def, _env, kMove);
                }
                else
                {
                    Reposition_(_e, _def, _env, kMove);
                }
            }
            else
            {
                switch (Roll(_env, 3))
                {
                    case 0: Go_(_e, _def, _env, kThrow); break;
                    case 1: Reposition_(_e, _def, _env, kThrow); break;
                    default: Go_(_e, _def, _env, kMove); break;
                }
            }
            return;
        }

        if (_e.stateTicks >= cfg.cooldownTicks)
        {
            switch (Roll(_env, 3))
            {
                case 0: Go_(_e, _def, _env, kIdle); break;
                case 1: Reposition_(_e, _def, _env, kThrow); break;
                default: Go_(_e, _def, _env, kThrow); break;
            }
        }
        else if (absDx <= cfg.closeStop)
        {
            Go_(_e, _def, _env, Roll(_env, 2) == 0 ? kIdle : kThrow);
        }
        else if (WallAhead(_e, _env, cfg.wallProbeW, cfg.wallProbeH, cfg.wallProbeX, cfg.wallProbeY))
        {
            Go_(_e, _def, _env, Roll(_env, 2) == 0 ? kHJump : kThrow);
        }
    }

    void DecideSword_(Enemy& _e, const EnemyDef& _def, EnemyEnv& _env)
    {
        const SoldierCfg& cfg   = _def.soldier;
        const float       absDx = AbsDx_(_e, _env);

        if (_e.state == kIdle)
        {
            if (_e.stateTicks < cfg.cooldownTicks)
            {
                return;
            }
            if (absDx >= cfg.detect)
            {
                if (Roll(_env, 2) == 0)
                {
                    Go_(_e, _def, _env, kMove);
                }
                else
                {
                    Reposition_(_e, _def, _env, kMove);
                }
            }
            else if (absDx >= cfg.jumpAttack)
            {
                Reposition_(_e, _def, _env, Roll(_env, 2) == 0 ? kJumpSlashReady : kMove);
            }
            else
            {
                switch (Roll(_env, 3))
                {
                    case 0: Go_(_e, _def, _env, kSlash); break;
                    case 1:
                        if (_e.bGrounded && _e.bOnOneWay && DyToPlayer(_e, _env) > cfg.dropDy)
                        {
                            Go_(_e, _def, _env, kMoveDown);
                        }
                        else
                        {
                            Go_(_e, _def, _env, kHJump);
                        }
                        break;
                    default: Go_(_e, _def, _env, kMove); break;
                }
            }
            return;
        }

        if (_e.stateTicks >= cfg.cooldownTicks)
        {
            int next = kMove;
            switch (Roll(_env, 3))
            {
                case 0: next = kIdle; break;
                case 1: next = kJumpSlashReady; break;
                default: next = kMove; break;
            }
            Reposition_(_e, _def, _env, next);
        }
        else if (absDx <= cfg.jumpAttack)
        {
            if (Roll(_env, 2) == 0)
            {
                Go_(_e, _def, _env, kIdle);
            }
            else
            {
                Go_(_e, _def, _env, absDx <= cfg.swordNear ? kSlash : kJumpSlashReady);
            }
        }
        else if (WallAhead(_e, _env, cfg.wallProbeW, cfg.wallProbeH, cfg.wallProbeX, cfg.wallProbeY))
        {
            Go_(_e, _def, _env, kHJump);
        }
    }

    void DecideMissile_(Enemy& _e, const EnemyDef& _def, EnemyEnv& _env)
    {
        const SoldierCfg& cfg   = _def.soldier;
        const float       absDx = AbsDx_(_e, _env);

        if (_e.state == kIdle)
        {
            if (_e.stateTicks < cfg.cooldownTicks)
            {
                return;
            }
            if (absDx >= cfg.detect)
            {
                if (Roll(_env, 2) == 0)
                {
                    Go_(_e, _def, _env, kMove);
                }
                else
                {
                    Reposition_(_e, _def, _env, kMove);
                }
            }
            else
            {
                Reposition_(_e, _def, _env, Roll(_env, 2) == 0 ? kThrow : kMove);
            }
            return;
        }

        if (_e.stateTicks >= cfg.cooldownTicks)
        {
            Reposition_(_e, _def, _env, Roll(_env, 2) == 0 ? kThrow : kIdle);
        }
        else if (WallAhead(_e, _env, cfg.wallProbeW, cfg.wallProbeH, cfg.wallProbeX, cfg.wallProbeY))
        {
            Go_(_e, _def, _env, kHJump);
        }
    }

    void Init_(Enemy& _e, const EnemyDef& _def, EnemyEnv& _env)
    {
        PrepareBody(_e, _def);
        Go_(_e, _def, _env, kFall);
    }

    void Tick_(Enemy& _e, const EnemyDef& _def, EnemyEnv& _env)
    {
        const SoldierCfg& cfg   = _def.soldier;
        const float       speed = _def.physics.speed;
        const float       face  = static_cast<float>(_e.facing);

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

            case kLand:
                if (ClipFinished(_e, _def))
                {
                    Go_(_e, _def, _env, kIdle);
                }
                break;

            case kIdle:
                if (!_e.bGrounded)
                {
                    Go_(_e, _def, _env, kFall);
                    break;
                }
                _e.body.velocity.x = 0.f;
                switch (cfg.ai)
                {
                    case eSoldierAi::Bomb: DecideBomb_(_e, _def, _env); break;
                    case eSoldierAi::Sword: DecideSword_(_e, _def, _env); break;
                    case eSoldierAi::Missile: DecideMissile_(_e, _def, _env); break;
                }
                break;

            case kMove:
                if (!_e.bGrounded)
                {
                    Go_(_e, _def, _env, kFall);
                    break;
                }
                FacePlayer(_e, _env);
                switch (cfg.ai)
                {
                    case eSoldierAi::Bomb: DecideBomb_(_e, _def, _env); break;
                    case eSoldierAi::Sword: DecideSword_(_e, _def, _env); break;
                    case eSoldierAi::Missile: DecideMissile_(_e, _def, _env); break;
                }
                if (_e.state == kMove)
                {
                    const bool bRetreat = cfg.retreatBelow > 0.f && AbsDx_(_e, _env) < cfg.retreatBelow;
                    _e.body.velocity.x  = static_cast<float>(_e.facing) * speed * (bRetreat ? -1.f : 1.f);
                }
                break;

            case kMoveDown:
                _e.body.velocity.x     = 0.f;
                _e.body.velocity.y     = _def.physics.dropSpeed;
                _e.body.dropThroughSteps = 2;
                if (ClipFinished(_e, _def))
                {
                    _e.body.bUseGravity = true;
                    _e.body.velocity.y  = 0.f;
                    Go_(_e, _def, _env, kFall);
                }
                break;

            case kHJump:
                _e.body.velocity.x = face * speed;
                break;

            case kThrow:
                _e.body.velocity.x = 0.f;
                if (ClipReachedFrame(_e, _def, cfg.fireFrame))
                {
                    FireWeapon(_e, _def, _env, cfg.weaponAttack);
                }
                if (ClipFinished(_e, _def))
                {
                    Go_(_e, _def, _env, kIdle);
                }
                break;

            case kSlash:
                _e.body.velocity.x = 0.f;
                if (ClipReachedFrame(_e, _def, cfg.slashFrame))
                {
                    StartMelee(_e, _def, _env, cfg.weaponSlash);
                }
                if (ClipFinished(_e, _def))
                {
                    Go_(_e, _def, _env, kIdle);
                }
                break;

            case kJumpSlashReady:
                _e.body.velocity.x = face * speed;
                if (_e.stateTicks >= cfg.jumpReadyTicks)
                {
                    Go_(_e, _def, _env, kJumpSlash);
                }
                break;

            case kJumpSlash:
                _e.body.velocity.x = face * speed;
                if (ClipReachedFrame(_e, _def, cfg.jumpSlashFrame))
                {
                    StartMelee(_e, _def, _env, cfg.weaponJumpSlash);
                }
                break;

            case kJumpSlashLand:
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
                    Go_(_e, _def, _env, _e.bActive ? kLand : kWait);
                }
                break;
            case kMoveDown:
                if (_e.bGrounded && _e.stateTicks > 1)
                {
                    _e.body.bUseGravity = true;
                    Go_(_e, _def, _env, kLand);
                }
                break;
            case kHJump:
                if (_e.bGrounded && _e.stateTicks > 0)
                {
                    Go_(_e, _def, _env, kLand);
                }
                break;
            case kJumpSlash:
                if (_e.bGrounded && _e.stateTicks > 0)
                {
                    Go_(_e, _def, _env, kJumpSlashLand);
                }
                break;
            default: break;
        }

        TickMelee(_e, _def, _env);
    }

    void OnDeath_(Enemy& _e, const EnemyDef& _def, EnemyEnv& _env)
    {
        BeginSoldierDeath(_e, _def, _env);
    }
}

const BehaviorOps kSoldierOps = {
    .init              = &Init_,
    .tick              = &Tick_,
    .onHit             = nullptr,
    .onDeath           = &OnDeath_,
    .tickDying         = &TickSoldierDeath,
    .registerHurtboxes = nullptr,
};

void ParseSoldierCfg(
    DefReader& _r,
    EnemyDef&  _def)
{
    SoldierCfg& c = _def.soldier;

    const slug::String ai = _r.Str("ai");
    if (ai == "bomb")
    {
        c.ai = eSoldierAi::Bomb;
    }
    else if (ai == "sword")
    {
        c.ai = eSoldierAi::Sword;
    }
    else if (ai == "missile")
    {
        c.ai = eSoldierAi::Missile;
    }
    else
    {
        _r.Fail("params.ai", "\"bomb\" / \"sword\" / \"missile\" 중 하나여야 합니다");
        return;
    }

    c.cooldownTicks = _r.Ticks("cooldown");
    c.detect        = _r.Num("detect");
    c.dropDy        = _r.Num("dropDy");
    c.jumpDy        = _r.Num("jumpDy");
    c.retreatBelow  = _r.Num("retreatBelow", 0.f);
    {
        const BoxDef probe = _r.Box("wallProbe");
        c.wallProbeW = probe.w;
        c.wallProbeH = probe.h;
        c.wallProbeX = probe.cx;
        c.wallProbeY = probe.cy;
    }

    c.clipIdle     = _r.Clip("idle");
    c.clipMove     = _r.Clip("move");
    c.clipMoveDown = _r.Clip("moveDown");
    c.clipFall     = _r.Clip("fall");
    c.clipHJump    = _r.Clip("hJump");
    c.clipLand     = _r.Clip("land");

    if (c.ai == eSoldierAi::Sword)
    {
        c.jumpAttack     = _r.Num("jumpAttack");
        c.swordNear      = _r.Num("swordNear");
        c.slashFrame     = _r.Int("slashFrame");
        c.jumpSlashFrame = _r.Int("jumpSlashFrame");
        c.jumpReadyTicks = _r.Ticks("jumpReadySec");
        c.clipSlash          = _r.Clip("slash");
        c.clipJumpSlashReady = _r.Clip("jumpSlashReady");
        c.clipJumpSlash      = _r.Clip("jumpSlash");
        c.clipJumpSlashLand  = _r.Clip("jumpSlashLand");
        c.weaponSlash        = _r.Weapon("slash");
        c.weaponJumpSlash    = _r.Weapon("jumpSlash");
    }
    else
    {
        c.fireFrame    = _r.Int("fireFrame");
        c.clipThrow    = _r.Clip("throw");
        c.weaponAttack = _r.Weapon("attack");
        if (c.ai == eSoldierAi::Bomb)
        {
            c.closeStop = _r.Num("closeStop");
        }
    }
}
}
