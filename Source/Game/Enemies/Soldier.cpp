#include <cmath>

#include "Base/Types.h"
#include "Game/Enemies/EnemyCommon.h"
#include "Game/Enemies/EnemyDefParse.h"

enum class eSoldierState
{
    Wait,
    Fall,
    Land,
    Idle,
    Move,
    MoveDown,
    HJump,
    Throw,
    Slash,
    JumpSlashReady,
    JumpSlash,
    JumpSlashLand,
    Climb
};

// aiVariant: 1 = 제자리 (이동 없음), 2 = 버스 계단 병사 (Climb 뒤 일반 AI), 3 = 계단 병사 + 제자리 (탐지 거리 무시)
constexpr int kVariantStay      = 1;
constexpr int kVariantClimb     = 2;
constexpr int kVariantClimbStay = 3;

static void Go_(
    Enemy&          _e,
    const EnemyDef& _def,
    EnemyEnv&       _env,
    eSoldierState   _state);

[[nodiscard]] static float AbsDx_(
    const Enemy&    _e,
    const EnemyEnv& _env)
{
    return std::abs(DxToPlayer(_e, _env));
}

static void Reposition_(
    Enemy&              _e,
    const EnemyDef&     _def,
    EnemyEnv&           _env,
    const eSoldierState _fallback)
{
    const SoldierCfg& cfg = _def.soldier;
    const float       dy  = DyToPlayer(_e, _env);
    if (_e.bGrounded && _e.bOnOneWay && dy > cfg.dropDy)
    {
        Go_(_e, _def, _env, eSoldierState::MoveDown);
    }
    else if (_e.bGrounded && dy < -cfg.jumpDy)
    {
        Go_(_e, _def, _env, eSoldierState::HJump);
    }
    else
    {
        Go_(_e, _def, _env, _fallback);
    }
}

static void Go_(
    Enemy&              _e,
    const EnemyDef&     _def,
    EnemyEnv&           _env,
    const eSoldierState _requestedState)
{
    const SoldierCfg& cfg   = _def.soldier;
    eSoldierState     state = _requestedState;
    if (state == eSoldierState::Move && (_e.aiVariant == kVariantStay || _e.aiVariant == kVariantClimbStay))
    {
        state = eSoldierState::Idle;
    }
    if (state == eSoldierState::JumpSlashReady && !CanLeaveRoof(_e))
    {
        state = eSoldierState::Move;   // 지붕 한가운데서 뛰면 버스 몸통 안에 내린다: 왼쪽 끝까지 걸어간 뒤에 뛴다
    }

    EnterState(_e, static_cast<int>(state));
    switch (state)
    {
        case eSoldierState::Wait:
            SetClip(_e, cfg.clipIdle, true);
            _e.body.velocity.x = 0.f;
            break;
        case eSoldierState::Fall:
            SetClip(_e, cfg.clipFall, true);
            break;
        case eSoldierState::Land:
            SetClip(_e, cfg.clipLand, true);
            _e.body.velocity.x = 0.f;
            break;
        case eSoldierState::Idle:
            FacePlayer(_e, _env);
            SetClip(_e, cfg.clipIdle, true);
            _e.body.velocity.x = 0.f;
            break;
        case eSoldierState::Move:
            FacePlayer(_e, _env);
            SetClip(_e, cfg.clipMove, true);
            break;
        case eSoldierState::MoveDown:
            SetClip(_e, cfg.clipMoveDown, true);
            _e.body.velocity.x  = 0.f;
            _e.body.bUseGravity = false;
            break;
        case eSoldierState::HJump:
            FacePlayer(_e, _env);
            SetClip(_e, cfg.clipHJump, true);
            _e.body.velocity.y = _def.physics.jumpVy;
            _e.body.bGrounded  = false;
            _e.bGrounded       = false;
            break;
        case eSoldierState::Throw:
            FacePlayer(_e, _env);
            SetClip(_e, cfg.clipThrow, true);
            _e.body.velocity.x = 0.f;
            break;
        case eSoldierState::Slash:
            FacePlayer(_e, _env);
            SetClip(_e, cfg.clipSlash, true);
            _e.body.velocity.x = 0.f;
            break;
        case eSoldierState::JumpSlashReady:
            FacePlayer(_e, _env);
            SetClip(_e, cfg.clipJumpSlashReady, true);
            _e.body.velocity.y = _def.physics.jumpVy;
            _e.body.bGrounded  = false;
            _e.bGrounded       = false;
            break;
        case eSoldierState::JumpSlash:
            SetClip(_e, cfg.clipJumpSlash, true);
            break;
        case eSoldierState::JumpSlashLand:
            SetClip(_e, cfg.clipJumpSlashLand, true);
            _e.body.velocity.x = 0.f;
            break;
        case eSoldierState::Climb:
            SetClip(_e, cfg.clipMove, true);
            _e.body.velocity = { 0.f, 0.f };
            _e.aux           = 0;
            break;
        default: break;
    }
}

// 버스 계단 병사: 스폰 위치 -> arena.min (계단 아래) -> arena.max (계단 위) 를 일정 속도로 걷는다 (물리/중력 없음). 끝나면 낙하로 넘어가 일반 AI.
static void TickClimb_(
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
    Go_(_e, _def, _env, eSoldierState::Fall);
    ClimbLeap(_e, _def);
}

static void DecideBomb_(
    Enemy&          _e,
    const EnemyDef& _def,
    EnemyEnv&       _env)
{
    const SoldierCfg& cfg   = _def.soldier;
    const float       absDx = AbsDx_(_e, _env);

    if (_e.state == static_cast<int>(eSoldierState::Idle))
    {
        if (_e.stateTicks < cfg.cooldownTicks)
        {
            return;
        }
        if (absDx >= cfg.detect && _e.aiVariant != kVariantClimbStay)
        {
            if (Roll(_env, 2) == 0)
            {
                Go_(_e, _def, _env, eSoldierState::Move);
            }
            else
            {
                Reposition_(_e, _def, _env, eSoldierState::Move);
            }
        }
        else
        {
            switch (Roll(_env, 3))
            {
                case 0: Go_(_e, _def, _env, eSoldierState::Throw); break;
                case 1: Reposition_(_e, _def, _env, eSoldierState::Throw); break;
                default: Go_(_e, _def, _env, eSoldierState::Move); break;
            }
        }
        return;
    }

    if (_e.stateTicks >= cfg.cooldownTicks)
    {
        switch (Roll(_env, 3))
        {
            case 0: Go_(_e, _def, _env, eSoldierState::Idle); break;
            case 1: Reposition_(_e, _def, _env, eSoldierState::Throw); break;
            default: Go_(_e, _def, _env, eSoldierState::Throw); break;
        }
    }
    else if (absDx <= cfg.closeStop)
    {
        Go_(_e, _def, _env, Roll(_env, 2) == 0 ? eSoldierState::Idle : eSoldierState::Throw);
    }
    else if (WallAhead(_e, _env, cfg.wallProbeW, cfg.wallProbeH, cfg.wallProbeX, cfg.wallProbeY))
    {
        Go_(_e, _def, _env, Roll(_env, 2) == 0 ? eSoldierState::HJump : eSoldierState::Throw);
    }
}

static void DecideSword_(
    Enemy&          _e,
    const EnemyDef& _def,
    EnemyEnv&       _env)
{
    const SoldierCfg& cfg   = _def.soldier;
    const float       absDx = AbsDx_(_e, _env);

    if (_e.state == static_cast<int>(eSoldierState::Idle))
    {
        if (_e.stateTicks < cfg.cooldownTicks)
        {
            return;
        }
        if (absDx >= cfg.detect)
        {
            if (Roll(_env, 2) == 0)
            {
                Go_(_e, _def, _env, eSoldierState::Move);
            }
            else
            {
                Reposition_(_e, _def, _env, eSoldierState::Move);
            }
        }
        else if (absDx >= cfg.jumpAttack)
        {
            Reposition_(_e, _def, _env, Roll(_env, 2) == 0 ? eSoldierState::JumpSlashReady : eSoldierState::Move);
        }
        else
        {
            switch (Roll(_env, 3))
            {
                case 0: Go_(_e, _def, _env, eSoldierState::Slash); break;
                case 1:
                    if (_e.bGrounded && _e.bOnOneWay && DyToPlayer(_e, _env) > cfg.dropDy)
                    {
                        Go_(_e, _def, _env, eSoldierState::MoveDown);
                    }
                    else
                    {
                        Go_(_e, _def, _env, eSoldierState::HJump);
                    }
                    break;
                default: Go_(_e, _def, _env, eSoldierState::Move); break;
            }
        }
        return;
    }

    if (_e.stateTicks >= cfg.cooldownTicks)
    {
        eSoldierState next = eSoldierState::Move;
        switch (Roll(_env, 3))
        {
            case 0: next = eSoldierState::Idle; break;
            case 1: next = eSoldierState::JumpSlashReady; break;
            default: next = eSoldierState::Move; break;
        }
        Reposition_(_e, _def, _env, next);
    }
    else if (absDx <= cfg.jumpAttack)
    {
        if (Roll(_env, 2) == 0)
        {
            Go_(_e, _def, _env, eSoldierState::Idle);
        }
        else
        {
            Go_(_e, _def, _env, absDx <= cfg.swordNear ? eSoldierState::Slash : eSoldierState::JumpSlashReady);
        }
    }
    else if (WallAhead(_e, _env, cfg.wallProbeW, cfg.wallProbeH, cfg.wallProbeX, cfg.wallProbeY))
    {
        Go_(_e, _def, _env, eSoldierState::HJump);
    }
}

static void DecideMissile_(
    Enemy&          _e,
    const EnemyDef& _def,
    EnemyEnv&       _env)
{
    const SoldierCfg& cfg   = _def.soldier;
    const float       absDx = AbsDx_(_e, _env);

    if (_e.state == static_cast<int>(eSoldierState::Idle))
    {
        if (_e.stateTicks < cfg.cooldownTicks)
        {
            return;
        }
        if (absDx >= cfg.detect)
        {
            if (Roll(_env, 2) == 0)
            {
                Go_(_e, _def, _env, eSoldierState::Move);
            }
            else
            {
                Reposition_(_e, _def, _env, eSoldierState::Move);
            }
        }
        else
        {
            Reposition_(_e, _def, _env, Roll(_env, 2) == 0 ? eSoldierState::Throw : eSoldierState::Move);
        }
        return;
    }

    if (_e.stateTicks >= cfg.cooldownTicks)
    {
        Reposition_(_e, _def, _env, Roll(_env, 2) == 0 ? eSoldierState::Throw : eSoldierState::Idle);
    }
    else if (WallAhead(_e, _env, cfg.wallProbeW, cfg.wallProbeH, cfg.wallProbeX, cfg.wallProbeY))
    {
        Go_(_e, _def, _env, eSoldierState::HJump);
    }
}

static void Init_(
    Enemy&          _e,
    const EnemyDef& _def,
    EnemyEnv&       _env)
{
    PrepareBody(_e, _def);
    Go_(_e, _def, _env, _e.aiVariant >= kVariantClimb ? eSoldierState::Climb : eSoldierState::Fall);
}

static void Tick_(
    Enemy&          _e,
    const EnemyDef& _def,
    EnemyEnv&       _env)
{
    const SoldierCfg& cfg   = _def.soldier;
    const float       speed = _def.physics.speed;
    const float       face  = static_cast<float>(_e.facing);

    switch (static_cast<eSoldierState>(_e.state))
    {
        case eSoldierState::Wait:
            if (_e.bActive)
            {
                Go_(_e, _def, _env, eSoldierState::Idle);
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
                Go_(_e, _def, _env, eSoldierState::Idle);
            }
            break;

        case eSoldierState::Idle:
            if (!_e.bGrounded)
            {
                Go_(_e, _def, _env, eSoldierState::Fall);
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

        case eSoldierState::Move:
            if (!_e.bGrounded)
            {
                Go_(_e, _def, _env, eSoldierState::Fall);
                break;
            }
            FacePlayer(_e, _env);
            switch (cfg.ai)
            {
                case eSoldierAi::Bomb: DecideBomb_(_e, _def, _env); break;
                case eSoldierAi::Sword: DecideSword_(_e, _def, _env); break;
                case eSoldierAi::Missile: DecideMissile_(_e, _def, _env); break;
            }
            if (_e.state == static_cast<int>(eSoldierState::Move))
            {
                const bool bRetreat = cfg.retreatBelow > 0.f && AbsDx_(_e, _env) < cfg.retreatBelow;
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
                Go_(_e, _def, _env, eSoldierState::Fall);
            }
            break;

        case eSoldierState::HJump:
            _e.body.velocity.x = face * speed;
            break;

        case eSoldierState::Throw:
            _e.body.velocity.x = 0.f;
            if (ClipReachedFrame(_e, _def, cfg.fireFrame))
            {
                FireWeapon(_e, _def, _env, cfg.weaponAttack);
            }
            if (ClipFinished(_e, _def))
            {
                Go_(_e, _def, _env, eSoldierState::Idle);
            }
            break;

        case eSoldierState::Slash:
            _e.body.velocity.x = 0.f;
            if (ClipReachedFrame(_e, _def, cfg.slashFrame))
            {
                StartMelee(_e, _def, _env, cfg.weaponSlash);
            }
            if (ClipFinished(_e, _def))
            {
                Go_(_e, _def, _env, eSoldierState::Idle);
            }
            break;

        case eSoldierState::JumpSlashReady:
            _e.body.velocity.x = face * speed;
            if (_e.stateTicks >= cfg.jumpReadyTicks)
            {
                Go_(_e, _def, _env, eSoldierState::JumpSlash);
            }
            break;

        case eSoldierState::JumpSlash:
            _e.body.velocity.x = face * speed;
            if (ClipReachedFrame(_e, _def, cfg.jumpSlashFrame))
            {
                StartMelee(_e, _def, _env, cfg.weaponJumpSlash);
            }
            break;

        case eSoldierState::JumpSlashLand:
            _e.body.velocity.x = 0.f;
            if (ClipFinished(_e, _def))
            {
                Go_(_e, _def, _env, eSoldierState::Idle);
            }
            break;

        case eSoldierState::Climb:
            TickClimb_(_e, _def, _env);
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
                Go_(_e, _def, _env, _e.bActive ? eSoldierState::Land : eSoldierState::Wait);
            }
            break;
        case eSoldierState::MoveDown:
            if (_e.bGrounded && _e.stateTicks > 1)
            {
                _e.body.bUseGravity = true;
                Go_(_e, _def, _env, eSoldierState::Land);
            }
            break;
        case eSoldierState::HJump:
            if (_e.bGrounded && _e.stateTicks > 0)
            {
                Go_(_e, _def, _env, eSoldierState::Land);
            }
            break;
        case eSoldierState::JumpSlash:
            if (_e.bGrounded && _e.stateTicks > 0)
            {
                Go_(_e, _def, _env, eSoldierState::JumpSlashLand);
            }
            break;
        default: break;
    }

    TickMelee(_e, _def, _env);
}

static void OnDeath_(
    Enemy&          _e,
    const EnemyDef& _def,
    EnemyEnv&       _env)
{
    BeginSoldierDeath(_e, _def, _env);
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

    const String ai = _r.Str("ai");
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
        c.wallProbeW       = probe.w;
        c.wallProbeH       = probe.h;
        c.wallProbeX       = probe.cx;
        c.wallProbeY       = probe.cy;
    }

    c.clipIdle     = _r.Clip("idle");
    c.clipMove     = _r.Clip("move");
    c.clipMoveDown = _r.Clip("moveDown");
    c.clipFall     = _r.Clip("fall");
    c.clipHJump    = _r.Clip("hJump");
    c.clipLand     = _r.Clip("land");

    if (c.ai == eSoldierAi::Sword)
    {
        c.jumpAttack         = _r.Num("jumpAttack");
        c.swordNear          = _r.Num("swordNear");
        c.slashFrame         = _r.Int("slashFrame");
        c.jumpSlashFrame     = _r.Int("jumpSlashFrame");
        c.jumpReadyTicks     = _r.Ticks("jumpReadySec");
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
