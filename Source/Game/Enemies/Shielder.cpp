#include <cmath>

#include "Game/Enemies/EnemyCommon.h"
#include "Game/Enemies/EnemyDefParse.h"

enum class eShielderState
{
    Wait,
    Fall,
    Idle,
    Move,
    MoveStop,
    ShotStart,
    ShotReady,
    Shot,
    ShotEnd,
    LookStart,
    LookIdle,
    LookEnd
};

namespace
{
[[nodiscard]] int PickDir_(
    int _dir,
    int _fwd,
    int _up,
    int _down)
{
    return _dir == 1 ? _up : (_dir == 2 ? _down : _fwd);
}

[[nodiscard]] int StateClip_(
    const Enemy&       _e,
    const ShielderCfg& _c)
{
    switch (static_cast<eShielderState>(_e.state))
    {
        case eShielderState::Wait:
        case eShielderState::Idle: return _c.clipIdle;
        case eShielderState::Fall: return _c.clipFall;
        case eShielderState::Move: return _c.clipMove;
        case eShielderState::MoveStop: return _c.clipMoveStop;
        case eShielderState::ShotStart: return PickDir_(_e.shieldDir, _c.clipShotStart, _c.clipShotStartUp, _c.clipShotStartDown);
        case eShielderState::ShotReady: return PickDir_(_e.shieldDir, _c.clipShotReady, _c.clipShotReadyUp, _c.clipShotReadyDown);
        case eShielderState::Shot: return PickDir_(_e.shieldDir, _c.clipShot, _c.clipShotUp, _c.clipShotDown);
        case eShielderState::ShotEnd: return PickDir_(_e.shieldDir, _c.clipShotEnd, _c.clipShotEndUp, _c.clipShotEndDown);
        case eShielderState::LookStart: return PickDir_(_e.shieldDir, _c.clipIdle, _c.clipLookUpStart, _c.clipLookDownStart);
        case eShielderState::LookIdle: return PickDir_(_e.shieldDir, _c.clipIdle, _c.clipLookUpIdle, _c.clipLookDownIdle);
        case eShielderState::LookEnd: return PickDir_(_e.shieldDir, _c.clipIdle, _c.clipLookUpEnd, _c.clipLookDownEnd);
        default: return _c.clipIdle;
    }
}

[[nodiscard]] bool StateDone_(
    const Enemy&    _e,
    const EnemyDef& _def)
{
    return _e.stateTicks >= _def.GetClip(StateClip_(_e, _def.shielder)).GetDurationTicks();
}

void Go_(
    Enemy&          _e,
    const EnemyDef& _def,
    EnemyEnv&       _env,
    eShielderState  _state,
    int             _dir = -1)
{
    const ShielderCfg& cfg = _def.shielder;
    EnterState(_e, static_cast<int>(_state));
    if (_dir >= 0)
    {
        _e.shieldDir = _dir;
    }
    switch (_state)
    {
        case eShielderState::Idle:
        case eShielderState::Move:
        case eShielderState::MoveStop:
            _e.shieldDir = 0;
            _e.bShieldUp = true;
            FacePlayer(_e, _env);
            break;
        case eShielderState::Wait:
        case eShielderState::Fall:
            _e.shieldDir = 0;
            _e.bShieldUp = true;
            break;
        case eShielderState::ShotStart:
            _e.shieldDir = 0;
            _e.bShieldUp = true;
            FacePlayer(_e, _env);
            _e.counter = cfg.bullets;
            break;
        case eShielderState::LookStart:
            _e.bShieldUp = true;
            FacePlayer(_e, _env);
            _e.counter = cfg.bullets;
            break;
        case eShielderState::ShotReady:
        case eShielderState::Shot:
        case eShielderState::ShotEnd:
            _e.bShieldUp = false;
            break;
        default: break;
    }
    if (_state == eShielderState::Move
        || _state == eShielderState::MoveStop
        || _state == eShielderState::Idle
        || _state == eShielderState::Wait)
    {
        _e.body.velocity.x = 0.f;
    }
    SetClip(_e, StateClip_(_e, cfg), true);
}

void Init_(
    Enemy&          _e,
    const EnemyDef& _def,
    EnemyEnv&       _env)
{
    PrepareBody(_e, _def);
    Go_(_e, _def, _env, eShielderState::Wait);
}

void Tick_(
    Enemy&          _e,
    const EnemyDef& _def,
    EnemyEnv&       _env)
{
    const ShielderCfg& cfg   = _def.shielder;
    const float        speed = _def.physics.speed;

    const float dy    = DyToPlayer(_e, _env);
    const float absDx = Abs(DxToPlayer(_e, _env));

    switch (static_cast<eShielderState>(_e.state))
    {
        case eShielderState::Wait:
            _e.body.velocity.x = 0.f;
            if (_e.bActive)
            {
                Go_(_e, _def, _env, eShielderState::Idle);
            }
            break;

        case eShielderState::Fall:
            _e.body.velocity.x = 0.f;
            break;

        case eShielderState::Idle:
            if (!_e.bGrounded)
            {
                Go_(_e, _def, _env, eShielderState::Fall);
                break;
            }
            _e.body.velocity.x = 0.f;
            FacePlayer(_e, _env);
            if (_e.stateTicks >= cfg.cooldownTicks)
            {
                if (dy < -cfg.upDy)
                {
                    if (absDx <= cfg.guard)
                    {
                        Go_(_e, _def, _env, eShielderState::LookStart, 1);
                    }
                    else
                    {
                        Go_(_e, _def, _env, eShielderState::Move);
                    }
                }
                else if (dy > cfg.levelDy)
                {
                    if (absDx <= cfg.guard)
                    {
                        Go_(_e, _def, _env, eShielderState::LookStart, 2);
                    }
                    else
                    {
                        Go_(_e, _def, _env, eShielderState::Move);
                    }
                }
                else if (absDx >= cfg.detect)
                {
                    Go_(_e, _def, _env, eShielderState::Move);
                }
                else
                {
                    Go_(_e, _def, _env, Roll(_env, 2) == 0 ? eShielderState::Idle : eShielderState::ShotStart);
                }
            }
            break;

        case eShielderState::Move:
            if (!_e.bGrounded)
            {
                Go_(_e, _def, _env, eShielderState::Fall);
                break;
            }
            FacePlayer(_e, _env);
            if (!FloorAhead(_e, _env, cfg.edgeProbeDist, cfg.edgeProbeDrop)
                || WallAhead(_e, _env, cfg.wallProbeW, cfg.wallProbeH, cfg.wallProbeX, cfg.wallProbeY))
            {
                Go_(_e, _def, _env, eShielderState::MoveStop);
            }
            else if (_e.stateTicks >= cfg.moveMaxTicks)
            {
                Go_(_e, _def, _env, Roll(_env, 2) == 0 ? eShielderState::MoveStop : eShielderState::Move);
            }
            else if (absDx <= cfg.detect && Abs(dy) <= cfg.moveDy)
            {
                Go_(_e, _def, _env, eShielderState::ShotStart);
            }
            else if (absDx <= cfg.guard && dy < -cfg.moveDy)
            {
                Go_(_e, _def, _env, eShielderState::LookStart, 1);
            }
            else if (absDx <= cfg.guard && dy > cfg.moveDy)
            {
                Go_(_e, _def, _env, eShielderState::LookStart, 2);
            }
            else
            {
                _e.body.velocity.x = static_cast<float>(_e.facing) * speed;
            }
            break;

        case eShielderState::MoveStop:
            _e.body.velocity.x = 0.f;
            if (StateDone_(_e, _def))
            {
                Go_(_e, _def, _env, eShielderState::Idle);
            }
            break;

        case eShielderState::ShotStart:
            _e.body.velocity.x = 0.f;
            if (_e.stateTicks >= cfg.startTicks)
            {
                Go_(_e, _def, _env, eShielderState::ShotReady);
            }
            break;

        case eShielderState::ShotReady:
            _e.body.velocity.x = 0.f;
            if (_e.stateTicks >= cfg.readyTicks)
            {
                if (_e.counter <= 0)
                {
                    Go_(_e, _def, _env, eShielderState::ShotEnd);
                }
                else
                {
                    --_e.counter;
                    Go_(_e, _def, _env, eShielderState::Shot);
                }
            }
            break;

        case eShielderState::Shot:
            _e.body.velocity.x = 0.f;
            if (_e.stateTicks == _def.GetClip(StateClip_(_e, cfg)).TickOfFrame(cfg.fireFrame))
            {
                if (_e.shieldDir == 1)
                {
                    FireWeapon(_e, _def, _env, cfg.weaponUp, { 0.f, -1.f });
                }
                else if (_e.shieldDir == 2)
                {
                    FireWeapon(_e, _def, _env, cfg.weaponDown, { 0.f, 1.f });
                }
                else
                {
                    FireWeapon(_e, _def, _env, cfg.weaponForward);
                }
            }
            if (StateDone_(_e, _def))
            {
                Go_(_e, _def, _env, eShielderState::ShotReady);
            }
            break;

        case eShielderState::ShotEnd:
            _e.body.velocity.x = 0.f;
            if (StateDone_(_e, _def))
            {
                Go_(_e, _def, _env, eShielderState::Idle);
            }
            break;

        case eShielderState::LookStart:
            _e.body.velocity.x = 0.f;
            if (StateDone_(_e, _def))
            {
                Go_(_e, _def, _env, eShielderState::LookIdle);
            }
            break;

        case eShielderState::LookIdle:
            _e.body.velocity.x = 0.f;
            if (_e.stateTicks >= cfg.cooldownTicks)
            {
                if (absDx <= cfg.guard)
                {
                    Go_(_e, _def, _env, eShielderState::ShotReady);
                }
                else
                {
                    Go_(_e, _def, _env, eShielderState::LookEnd);
                }
            }
            break;

        case eShielderState::LookEnd:
            _e.body.velocity.x = 0.f;
            if (StateDone_(_e, _def))
            {
                Go_(_e, _def, _env, eShielderState::Idle);
            }
            break;

        default: break;
    }

    (void)StepPhysics(_e, _def, _env);

    if (_e.state == static_cast<int>(eShielderState::Fall) && _e.bGrounded)
    {
        Go_(_e, _def, _env, _e.bActive ? eShielderState::Idle : eShielderState::Wait);
    }

    if (_e.defenceTicks > 0)
    {
        --_e.defenceTicks;
    }
    const int wantClip = (_e.defenceTicks > 0 && _e.bShieldUp) ? PickDir_(_e.shieldDir, cfg.clipDefence, cfg.clipDefenceUp, cfg.clipDefenceDown) : StateClip_(_e, cfg);
    SetClip(_e, wantClip);
}

bool OnHit_(
    Enemy&          _e,
    const EnemyDef& _def,
    EnemyEnv&,
    const HitEvent& _event)
{
    if ((_event.target & kShieldOwnerBit) != 0)
    {
        _e.defenceTicks = _def.shielder.defenceTicks;
        return true;
    }
    return false;
}

void OnDeath_(
    Enemy&          _e,
    const EnemyDef& _def,
    EnemyEnv&       _env)
{
    _e.bShieldUp = false;
    BeginSoldierDeath(_e, _def, _env);
}

void RegisterHurtboxes_(
    const Enemy&    _e,
    const EnemyDef& _def,
    CombatSystem&   _combat)
{
    const ShielderCfg& cfg = _def.shielder;
    if (_e.bShieldUp)
    {
        const BoxDef& box = _e.shieldDir == 1 ? cfg.shieldUp : (_e.shieldDir == 2 ? cfg.shieldDown : cfg.shieldFront);
        Hurtbox       shield;
        shield.box     = box.At(_e.body.position, _e.facing);
        shield.team    = _def.team;
        shield.owner   = _e.id | kShieldOwnerBit;
        shield.accepts = HitKindMask { eHitKind::Bullet };
        _combat.AddHurtbox(shield);
    }

    Hurtbox body;
    body.box   = _def.hurtbox.At(_e.body.position, _e.facing);
    body.team  = _def.team;
    body.owner = _e.id;
    _combat.AddHurtbox(body);
}
}   // namespace

const BehaviorOps kShielderOps = {
    .init              = &Init_,
    .tick              = &Tick_,
    .onHit             = &OnHit_,
    .onDeath           = &OnDeath_,
    .tickDying         = &TickSoldierDeath,
    .registerHurtboxes = &RegisterHurtboxes_,
};

void ParseShielderCfg(
    DefReader& _r,
    EnemyDef&  _def)
{
    ShielderCfg& c = _def.shielder;

    c.cooldownTicks = _r.Ticks("cooldown");
    c.detect        = _r.Num("detect");
    c.guard         = _r.Num("guard");
    c.levelDy       = _r.Num("levelDy");
    c.upDy          = _r.Num("upDy");
    c.moveDy        = _r.Num("moveDy");
    c.bullets       = _r.Int("bullets");
    c.startTicks    = _r.Ticks("startSec");
    c.readyTicks    = _r.Ticks("readySec");
    c.moveMaxTicks  = _r.Ticks("moveMaxSec");
    c.fireFrame     = _r.Int("fireFrame");
    c.edgeProbeDist = _r.Num("edgeProbeDist");
    c.edgeProbeDrop = _r.Num("edgeProbeDrop");
    c.defenceTicks  = _r.Ticks("defenceSec");
    {
        const BoxDef probe = _r.Box("wallProbe");
        c.wallProbeW       = probe.w;
        c.wallProbeH       = probe.h;
        c.wallProbeX       = probe.cx;
        c.wallProbeY       = probe.cy;
    }
    c.shieldFront = _r.Box("shieldFront");
    c.shieldUp    = _r.Box("shieldUp");
    c.shieldDown  = _r.Box("shieldDown");

    c.clipIdle          = _r.Clip("idle");
    c.clipMove          = _r.Clip("move");
    c.clipMoveStop      = _r.Clip("moveStop");
    c.clipFall          = _r.Clip("fall");
    c.clipShotStart     = _r.Clip("shotStart");
    c.clipShotReady     = _r.Clip("shotReady");
    c.clipShot          = _r.Clip("shot");
    c.clipShotEnd       = _r.Clip("shotEnd");
    c.clipShotStartUp   = _r.Clip("shotStartUp");
    c.clipShotReadyUp   = _r.Clip("shotReadyUp");
    c.clipShotUp        = _r.Clip("shotUp");
    c.clipShotEndUp     = _r.Clip("shotEndUp");
    c.clipShotStartDown = _r.Clip("shotStartDown");
    c.clipShotReadyDown = _r.Clip("shotReadyDown");
    c.clipShotDown      = _r.Clip("shotDown");
    c.clipShotEndDown   = _r.Clip("shotEndDown");
    c.clipLookUpStart   = _r.Clip("lookUpStart");
    c.clipLookUpIdle    = _r.Clip("lookUpIdle");
    c.clipLookUpEnd     = _r.Clip("lookUpEnd");
    c.clipLookDownStart = _r.Clip("lookDownStart");
    c.clipLookDownIdle  = _r.Clip("lookDownIdle");
    c.clipLookDownEnd   = _r.Clip("lookDownEnd");
    c.clipDefence       = _r.Clip("defence");
    c.clipDefenceUp     = _r.Clip("defenceUp");
    c.clipDefenceDown   = _r.Clip("defenceDown");

    c.weaponForward = _r.Weapon("forward");
    c.weaponUp      = _r.Weapon("up");
    c.weaponDown    = _r.Weapon("down");
}
