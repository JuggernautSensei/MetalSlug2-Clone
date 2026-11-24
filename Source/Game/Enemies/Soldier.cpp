#include "Game/Enemies/SoldierEnemy.h"

#include <cmath>

#include "Base/Types.h"
#include "Game/Enemies/EnemyCommon.h"
#include "Game/Enemies/EnemyDefParse.h"

constexpr int kVariantStay      = 1;
constexpr int kVariantClimb     = 2;
constexpr int kVariantClimbStay = 3;

void SoldierEnemy::Init(
    const EnemyDef& _def,
    EnemyEnv&       _env)
{
    EnemyState& _e = GetMutableState();
    PrepareBody(_e, _def);
    Go_(_def, _env, _e.aiVariant >= kVariantClimb ? eSoldierState::Climb : eSoldierState::Fall);
}

void SoldierEnemy::Tick(
    const EnemyDef& _def,
    EnemyEnv&       _env)
{
    EnemyState&          _e     = GetMutableState();
    const SoldierConfig& config = _def.soldier;
    const float          speed  = _def.physics.speed;
    const float          face   = static_cast<float>(_e.facing);

    switch (static_cast<eSoldierState>(_e.state))
    {
        case eSoldierState::Wait:
            if (_e.bActive)
            {
                Go_(_def, _env, eSoldierState::Idle);
            }
            break;

        case eSoldierState::Fall:
            if (!_e.bLeap)
            {
                _e.body.velocity.x = 0.f;
            }
            break;

        case eSoldierState::Land:
            if (ClipFinished(_e, _def))
            {
                Go_(_def, _env, eSoldierState::Idle);
            }
            break;

        case eSoldierState::Idle:
            if (!_e.bGrounded)
            {
                Go_(_def, _env, eSoldierState::Fall);
                break;
            }
            _e.body.velocity.x = 0.f;
            switch (config.ai)
            {
                case eSoldierAi::Bomb: DecideBomb_(_def, _env); break;
                case eSoldierAi::Sword: DecideSword_(_def, _env); break;
                case eSoldierAi::Missile: DecideMissile_(_def, _env); break;
            }
            break;

        case eSoldierState::Move:
            if (!_e.bGrounded)
            {
                Go_(_def, _env, eSoldierState::Fall);
                break;
            }
            FacePlayer(_e, _env);
            switch (config.ai)
            {
                case eSoldierAi::Bomb: DecideBomb_(_def, _env); break;
                case eSoldierAi::Sword: DecideSword_(_def, _env); break;
                case eSoldierAi::Missile: DecideMissile_(_def, _env); break;
            }
            if (_e.state == static_cast<int>(eSoldierState::Move))
            {
                const bool bRetreat = config.retreatBelow > 0.f && AbsDx_(_env) < config.retreatBelow;
                _e.body.velocity.x  = static_cast<float>(_e.facing) * speed * (bRetreat ? -1.f : 1.f);
            }
            break;

        case eSoldierState::MoveDown:
            _e.body.velocity.x       = 0.f;
            _e.body.velocity.y       = _def.physics.dropSpeed;
            _e.body.dropThroughSteps = 2;
            if (ClipFinished(_e, _def))
            {
                _e.body.bUseGravity = true;
                _e.body.velocity.y  = 0.f;
                Go_(_def, _env, eSoldierState::Fall);
            }
            break;

        case eSoldierState::HJump:
            _e.body.velocity.x = face * speed;
            break;

        case eSoldierState::Throw:
            _e.body.velocity.x = 0.f;
            if (ClipReachedFrame(_e, _def, config.fireFrame))
            {
                FireWeapon(_e, _def, _env, config.weaponAttack);
            }
            if (ClipFinished(_e, _def))
            {
                Go_(_def, _env, eSoldierState::Idle);
            }
            break;

        case eSoldierState::Slash:
            _e.body.velocity.x = 0.f;
            if (ClipReachedFrame(_e, _def, config.slashFrame))
            {
                StartMelee(_e, _def, _env, config.weaponSlash);
            }
            if (ClipFinished(_e, _def))
            {
                Go_(_def, _env, eSoldierState::Idle);
            }
            break;

        case eSoldierState::JumpSlashReady:
            _e.body.velocity.x = face * speed;
            if (_e.stateTicks >= config.jumpReadyTicks)
            {
                Go_(_def, _env, eSoldierState::JumpSlash);
            }
            break;

        case eSoldierState::JumpSlash:
            _e.body.velocity.x = face * speed;
            if (ClipReachedFrame(_e, _def, config.jumpSlashFrame))
            {
                StartMelee(_e, _def, _env, config.weaponJumpSlash);
            }
            break;

        case eSoldierState::JumpSlashLand:
            _e.body.velocity.x = 0.f;
            if (ClipFinished(_e, _def))
            {
                Go_(_def, _env, eSoldierState::Idle);
            }
            break;

        case eSoldierState::Climb:
            TickClimb_(_def, _env);
            break;

        default: break;
    }

    if (static_cast<eSoldierState>(_e.state) != eSoldierState::Climb)
    {
        (void)StepPhysics(_e, _def, _env);
        HoldOnRoof(_e);
    }

    switch (static_cast<eSoldierState>(_e.state))
    {
        case eSoldierState::Fall:
            if (_e.bGrounded)
            {
                _e.bLeap = false;
                Go_(_def, _env, _e.bActive ? eSoldierState::Land : eSoldierState::Wait);
            }
            break;
        case eSoldierState::MoveDown:
            if (_e.bGrounded && _e.stateTicks > 1)
            {
                _e.body.bUseGravity = true;
                Go_(_def, _env, eSoldierState::Land);
            }
            break;
        case eSoldierState::HJump:
            if (_e.bGrounded && _e.stateTicks > 0)
            {
                Go_(_def, _env, eSoldierState::Land);
            }
            break;
        case eSoldierState::JumpSlash:
            if (_e.bGrounded && _e.stateTicks > 0)
            {
                Go_(_def, _env, eSoldierState::JumpSlashLand);
            }
            break;
        default: break;
    }

    TickMelee(_e, _def, _env);
}

void SoldierEnemy::OnDeath(
    const EnemyDef& _def,
    EnemyEnv&       _env)
{
    EnemyState& _e = GetMutableState();
    BeginSoldierDeath(_e, _def, _env);
}

void SoldierEnemy::TickDying(
    const EnemyDef& _def,
    EnemyEnv&       _env)
{
    TickSoldierDeath(GetMutableState(), _def, _env);
}

void ParseSoldierConfig(
    DefReader& _r,
    EnemyDef&  _def)
{
    SoldierConfig& config = _def.soldier;

    const String ai = _r.Str("ai");
    if (ai == "bomb")
    {
        config.ai = eSoldierAi::Bomb;
    }
    else if (ai == "sword")
    {
        config.ai = eSoldierAi::Sword;
    }
    else if (ai == "missile")
    {
        config.ai = eSoldierAi::Missile;
    }
    else
    {
        _r.Fail("params.ai", "\"bomb\" / \"sword\" / \"missile\" 중 하나여야 합니다");
        return;
    }

    config.cooldownTicks = _r.Ticks("cooldown");
    config.detect        = _r.Num("detect");
    config.dropDy        = _r.Num("dropDy");
    config.jumpDy        = _r.Num("jumpDy");
    config.retreatBelow  = _r.Num("retreatBelow", 0.f);
    {
        const BoxDef probe = _r.Box("wallProbe");
        config.wallProbeW  = probe.w;
        config.wallProbeH  = probe.h;
        config.wallProbeX  = probe.cx;
        config.wallProbeY  = probe.cy;
    }

    config.clipIdle     = _r.Clip("idle");
    config.clipMove     = _r.Clip("move");
    config.clipMoveDown = _r.Clip("moveDown");
    config.clipFall     = _r.Clip("fall");
    config.clipHJump    = _r.Clip("hJump");
    config.clipLand     = _r.Clip("land");

    if (config.ai == eSoldierAi::Sword)
    {
        config.jumpAttack         = _r.Num("jumpAttack");
        config.swordNear          = _r.Num("swordNear");
        config.slashFrame         = _r.Int("slashFrame");
        config.jumpSlashFrame     = _r.Int("jumpSlashFrame");
        config.jumpReadyTicks     = _r.Ticks("jumpReadySec");
        config.clipSlash          = _r.Clip("slash");
        config.clipJumpSlashReady = _r.Clip("jumpSlashReady");
        config.clipJumpSlash      = _r.Clip("jumpSlash");
        config.clipJumpSlashLand  = _r.Clip("jumpSlashLand");
        config.weaponSlash        = _r.Weapon("slash");
        config.weaponJumpSlash    = _r.Weapon("jumpSlash");
    }
    else
    {
        config.fireFrame    = _r.Int("fireFrame");
        config.clipThrow    = _r.Clip("throw");
        config.weaponAttack = _r.Weapon("attack");
        if (config.ai == eSoldierAi::Bomb)
        {
            config.closeStop = _r.Num("closeStop");
        }
    }
}

[[nodiscard]] float SoldierEnemy::AbsDx_(
    const EnemyEnv& _env) const
{
    const EnemyState& _e = GetState();
    return Abs(DxToPlayer(_e, _env));
}

void SoldierEnemy::Reposition_(
    const EnemyDef&     _def,
    EnemyEnv&           _env,
    const eSoldierState _fallback)
{
    EnemyState&          _e     = GetMutableState();
    const SoldierConfig& config = _def.soldier;
    const float          dy     = DyToPlayer(_e, _env);
    if (_e.bGrounded && _e.bOnOneWay && dy > config.dropDy)
    {
        Go_(_def, _env, eSoldierState::MoveDown);
    }
    else if (_e.bGrounded && dy < -config.jumpDy)
    {
        Go_(_def, _env, eSoldierState::HJump);
    }
    else
    {
        Go_(_def, _env, _fallback);
    }
}

void SoldierEnemy::Go_(
    const EnemyDef&     _def,
    EnemyEnv&           _env,
    const eSoldierState _requestedState)
{
    EnemyState&          _e     = GetMutableState();
    const SoldierConfig& config = _def.soldier;
    eSoldierState        state  = _requestedState;
    if (state == eSoldierState::Move && (_e.aiVariant == kVariantStay || _e.aiVariant == kVariantClimbStay))
    {
        state = eSoldierState::Idle;
    }
    if (state == eSoldierState::JumpSlashReady && !CanLeaveRoof(_e))
    {
        state = eSoldierState::Move;
    }

    EnterState(_e, static_cast<int>(state));
    switch (state)
    {
        case eSoldierState::Wait:
            SetClip(_e, config.clipIdle, true);
            _e.body.velocity.x = 0.f;
            break;
        case eSoldierState::Fall:
            SetClip(_e, config.clipFall, true);
            break;
        case eSoldierState::Land:
            SetClip(_e, config.clipLand, true);
            _e.body.velocity.x = 0.f;
            break;
        case eSoldierState::Idle:
            FacePlayer(_e, _env);
            SetClip(_e, config.clipIdle, true);
            _e.body.velocity.x = 0.f;
            break;
        case eSoldierState::Move:
            FacePlayer(_e, _env);
            SetClip(_e, config.clipMove, true);
            break;
        case eSoldierState::MoveDown:
            SetClip(_e, config.clipMoveDown, true);
            _e.body.velocity.x  = 0.f;
            _e.body.bUseGravity = false;
            break;
        case eSoldierState::HJump:
            FacePlayer(_e, _env);
            SetClip(_e, config.clipHJump, true);
            _e.body.velocity.y = _def.physics.jumpVy;
            _e.body.bGrounded  = false;
            _e.bGrounded       = false;
            break;
        case eSoldierState::Throw:
            FacePlayer(_e, _env);
            SetClip(_e, config.clipThrow, true);
            _e.body.velocity.x = 0.f;
            break;
        case eSoldierState::Slash:
            FacePlayer(_e, _env);
            SetClip(_e, config.clipSlash, true);
            _e.body.velocity.x = 0.f;
            break;
        case eSoldierState::JumpSlashReady:
            FacePlayer(_e, _env);
            SetClip(_e, config.clipJumpSlashReady, true);
            _e.body.velocity.y = _def.physics.jumpVy;
            _e.body.bGrounded  = false;
            _e.bGrounded       = false;
            break;
        case eSoldierState::JumpSlash:
            SetClip(_e, config.clipJumpSlash, true);
            break;
        case eSoldierState::JumpSlashLand:
            SetClip(_e, config.clipJumpSlashLand, true);
            _e.body.velocity.x = 0.f;
            break;
        case eSoldierState::Climb:
            SetClip(_e, config.clipMove, true);
            _e.body.velocity = { 0.f, 0.f };
            _e.climbSegment  = 0;
            break;
        default: break;
    }
}

void SoldierEnemy::TickClimb_(
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
    Go_(_def, _env, eSoldierState::Fall);
    ClimbLeap(_e, _def);
}

void SoldierEnemy::DecideBomb_(
    const EnemyDef& _def,
    EnemyEnv&       _env)
{
    EnemyState&          _e     = GetMutableState();
    const SoldierConfig& config = _def.soldier;
    const float          absDx  = AbsDx_(_env);

    if (_e.state == static_cast<int>(eSoldierState::Idle))
    {
        if (_e.stateTicks < config.cooldownTicks)
        {
            return;
        }
        if (absDx >= config.detect && _e.aiVariant != kVariantClimbStay)
        {
            if (Roll(_env, 2) == 0)
            {
                Go_(_def, _env, eSoldierState::Move);
            }
            else
            {
                Reposition_(_def, _env, eSoldierState::Move);
            }
        }
        else
        {
            switch (Roll(_env, 3))
            {
                case 0: Go_(_def, _env, eSoldierState::Throw); break;
                case 1: Reposition_(_def, _env, eSoldierState::Throw); break;
                default: Go_(_def, _env, eSoldierState::Move); break;
            }
        }
        return;
    }

    if (_e.stateTicks >= config.cooldownTicks)
    {
        switch (Roll(_env, 3))
        {
            case 0: Go_(_def, _env, eSoldierState::Idle); break;
            case 1: Reposition_(_def, _env, eSoldierState::Throw); break;
            default: Go_(_def, _env, eSoldierState::Throw); break;
        }
    }
    else if (absDx <= config.closeStop)
    {
        Go_(_def, _env, Roll(_env, 2) == 0 ? eSoldierState::Idle : eSoldierState::Throw);
    }
    else if (WallAhead(_e, _env, config.wallProbeW, config.wallProbeH, config.wallProbeX, config.wallProbeY))
    {
        Go_(_def, _env, Roll(_env, 2) == 0 ? eSoldierState::HJump : eSoldierState::Throw);
    }
}

void SoldierEnemy::DecideSword_(
    const EnemyDef& _def,
    EnemyEnv&       _env)
{
    EnemyState&          _e     = GetMutableState();
    const SoldierConfig& config = _def.soldier;
    const float          absDx  = AbsDx_(_env);

    if (_e.state == static_cast<int>(eSoldierState::Idle))
    {
        if (_e.stateTicks < config.cooldownTicks)
        {
            return;
        }
        if (absDx >= config.detect)
        {
            if (Roll(_env, 2) == 0)
            {
                Go_(_def, _env, eSoldierState::Move);
            }
            else
            {
                Reposition_(_def, _env, eSoldierState::Move);
            }
        }
        else if (absDx >= config.jumpAttack)
        {
            Reposition_(_def, _env, Roll(_env, 2) == 0 ? eSoldierState::JumpSlashReady : eSoldierState::Move);
        }
        else
        {
            switch (Roll(_env, 3))
            {
                case 0: Go_(_def, _env, eSoldierState::Slash); break;
                case 1:
                    if (_e.bGrounded && _e.bOnOneWay && DyToPlayer(_e, _env) > config.dropDy)
                    {
                        Go_(_def, _env, eSoldierState::MoveDown);
                    }
                    else
                    {
                        Go_(_def, _env, eSoldierState::HJump);
                    }
                    break;
                default: Go_(_def, _env, eSoldierState::Move); break;
            }
        }
        return;
    }

    if (_e.stateTicks >= config.cooldownTicks)
    {
        eSoldierState next = eSoldierState::Move;
        switch (Roll(_env, 3))
        {
            case 0: next = eSoldierState::Idle; break;
            case 1: next = eSoldierState::JumpSlashReady; break;
            default: next = eSoldierState::Move; break;
        }
        Reposition_(_def, _env, next);
    }
    else if (absDx <= config.jumpAttack)
    {
        if (Roll(_env, 2) == 0)
        {
            Go_(_def, _env, eSoldierState::Idle);
        }
        else
        {
            Go_(_def, _env, absDx <= config.swordNear ? eSoldierState::Slash : eSoldierState::JumpSlashReady);
        }
    }
    else if (WallAhead(_e, _env, config.wallProbeW, config.wallProbeH, config.wallProbeX, config.wallProbeY))
    {
        Go_(_def, _env, eSoldierState::HJump);
    }
}

void SoldierEnemy::DecideMissile_(
    const EnemyDef& _def,
    EnemyEnv&       _env)
{
    EnemyState&          _e     = GetMutableState();
    const SoldierConfig& config = _def.soldier;
    const float          absDx  = AbsDx_(_env);

    if (_e.state == static_cast<int>(eSoldierState::Idle))
    {
        if (_e.stateTicks < config.cooldownTicks)
        {
            return;
        }
        if (absDx >= config.detect)
        {
            if (Roll(_env, 2) == 0)
            {
                Go_(_def, _env, eSoldierState::Move);
            }
            else
            {
                Reposition_(_def, _env, eSoldierState::Move);
            }
        }
        else
        {
            Reposition_(_def, _env, Roll(_env, 2) == 0 ? eSoldierState::Throw : eSoldierState::Move);
        }
        return;
    }

    if (_e.stateTicks >= config.cooldownTicks)
    {
        Reposition_(_def, _env, Roll(_env, 2) == 0 ? eSoldierState::Throw : eSoldierState::Idle);
    }
    else if (WallAhead(_e, _env, config.wallProbeW, config.wallProbeH, config.wallProbeX, config.wallProbeY))
    {
        Go_(_def, _env, eSoldierState::HJump);
    }
}

void SoldierEnemy::HashActionState(Fnv1a& _hash) const
{
    HashActionState_(_hash, 0, GetState().climbSegment);
}
