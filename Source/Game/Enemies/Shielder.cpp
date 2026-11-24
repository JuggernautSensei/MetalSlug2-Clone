#include "Game/Enemies/ShielderEnemy.h"

#include <cmath>

#include "Game/Enemies/EnemyCommon.h"
#include "Game/Enemies/EnemyDefParse.h"

namespace
{
[[nodiscard]] int PickDir_(
    const int _dir,
    const int _fwd,
    const int _up,
    const int _down)
{
    return _dir == 1 ? _up : (_dir == 2 ? _down : _fwd);
}

}   // namespace

void ShielderEnemy::Init(
    const EnemyDef& _def,
    EnemyEnv&       _env)
{
    EnemyState& _e = GetMutableState();
    PrepareBody(_e, _def);
    Go_(_def, _env, eShielderState::Wait);
}

void ShielderEnemy::Tick(
    const EnemyDef& _def,
    EnemyEnv&       _env)
{
    EnemyState&           _e     = GetMutableState();
    const ShielderConfig& config = _def.shielder;
    const float           speed  = _def.physics.speed;

    const float dy    = DyToPlayer(_e, _env);
    const float absDx = Abs(DxToPlayer(_e, _env));

    switch (static_cast<eShielderState>(_e.state))
    {
        case eShielderState::Wait:
            _e.body.velocity.x = 0.f;
            if (_e.bActive)
            {
                Go_(_def, _env, eShielderState::Idle);
            }
            break;

        case eShielderState::Fall:
            _e.body.velocity.x = 0.f;
            break;

        case eShielderState::Idle:
            if (!_e.bGrounded)
            {
                Go_(_def, _env, eShielderState::Fall);
                break;
            }
            _e.body.velocity.x = 0.f;
            FacePlayer(_e, _env);
            if (_e.stateTicks >= config.cooldownTicks)
            {
                if (dy < -config.upDy)
                {
                    if (absDx <= config.guard)
                    {
                        Go_(_def, _env, eShielderState::LookStart, 1);
                    }
                    else
                    {
                        Go_(_def, _env, eShielderState::Move);
                    }
                }
                else if (dy > config.levelDy)
                {
                    if (absDx <= config.guard)
                    {
                        Go_(_def, _env, eShielderState::LookStart, 2);
                    }
                    else
                    {
                        Go_(_def, _env, eShielderState::Move);
                    }
                }
                else if (absDx >= config.detect)
                {
                    Go_(_def, _env, eShielderState::Move);
                }
                else
                {
                    Go_(_def, _env, Roll(_env, 2) == 0 ? eShielderState::Idle : eShielderState::ShotStart);
                }
            }
            break;

        case eShielderState::Move:
            if (!_e.bGrounded)
            {
                Go_(_def, _env, eShielderState::Fall);
                break;
            }
            FacePlayer(_e, _env);
            if (!FloorAhead(_e, _env, config.edgeProbeDist, config.edgeProbeDrop)
                || WallAhead(_e, _env, config.wallProbeW, config.wallProbeH, config.wallProbeX, config.wallProbeY))
            {
                Go_(_def, _env, eShielderState::MoveStop);
            }
            else if (_e.stateTicks >= config.moveMaxTicks)
            {
                Go_(_def, _env, Roll(_env, 2) == 0 ? eShielderState::MoveStop : eShielderState::Move);
            }
            else if (absDx <= config.detect && Abs(dy) <= config.moveDy)
            {
                Go_(_def, _env, eShielderState::ShotStart);
            }
            else if (absDx <= config.guard && dy < -config.moveDy)
            {
                Go_(_def, _env, eShielderState::LookStart, 1);
            }
            else if (absDx <= config.guard && dy > config.moveDy)
            {
                Go_(_def, _env, eShielderState::LookStart, 2);
            }
            else
            {
                _e.body.velocity.x = static_cast<float>(_e.facing) * speed;
            }
            break;

        case eShielderState::MoveStop:
            _e.body.velocity.x = 0.f;
            if (StateDone_(_def))
            {
                Go_(_def, _env, eShielderState::Idle);
            }
            break;

        case eShielderState::ShotStart:
            _e.body.velocity.x = 0.f;
            if (_e.stateTicks >= config.startTicks)
            {
                Go_(_def, _env, eShielderState::ShotReady);
            }
            break;

        case eShielderState::ShotReady:
            _e.body.velocity.x = 0.f;
            if (_e.stateTicks >= config.readyTicks)
            {
                if (m_shotsRemaining <= 0)
                {
                    Go_(_def, _env, eShielderState::ShotEnd);
                }
                else
                {
                    --m_shotsRemaining;
                    Go_(_def, _env, eShielderState::Shot);
                }
            }
            break;

        case eShielderState::Shot:
            _e.body.velocity.x = 0.f;
            if (_e.stateTicks == _def.GetClip(StateClip_(config)).TickOfFrame(config.fireFrame))
            {
                if (_e.shieldDir == 1)
                {
                    FireWeapon(_e, _def, _env, config.weaponUp, { 0.f, -1.f });
                }
                else if (_e.shieldDir == 2)
                {
                    FireWeapon(_e, _def, _env, config.weaponDown, { 0.f, 1.f });
                }
                else
                {
                    FireWeapon(_e, _def, _env, config.weaponForward);
                }
            }
            if (StateDone_(_def))
            {
                Go_(_def, _env, eShielderState::ShotReady);
            }
            break;

        case eShielderState::ShotEnd:
            _e.body.velocity.x = 0.f;
            if (StateDone_(_def))
            {
                Go_(_def, _env, eShielderState::Idle);
            }
            break;

        case eShielderState::LookStart:
            _e.body.velocity.x = 0.f;
            if (StateDone_(_def))
            {
                Go_(_def, _env, eShielderState::LookIdle);
            }
            break;

        case eShielderState::LookIdle:
            _e.body.velocity.x = 0.f;
            if (_e.stateTicks >= config.cooldownTicks)
            {
                if (absDx <= config.guard)
                {
                    Go_(_def, _env, eShielderState::ShotReady);
                }
                else
                {
                    Go_(_def, _env, eShielderState::LookEnd);
                }
            }
            break;

        case eShielderState::LookEnd:
            _e.body.velocity.x = 0.f;
            if (StateDone_(_def))
            {
                Go_(_def, _env, eShielderState::Idle);
            }
            break;

        default: break;
    }

    (void)StepPhysics(_e, _def, _env);

    if (_e.state == static_cast<int>(eShielderState::Fall) && _e.bGrounded)
    {
        Go_(_def, _env, _e.bActive ? eShielderState::Idle : eShielderState::Wait);
    }

    if (_e.defenceTicks > 0)
    {
        --_e.defenceTicks;
    }
    const int wantClip = (_e.defenceTicks > 0 && _e.bShieldUp) ? PickDir_(_e.shieldDir, config.clipDefence, config.clipDefenceUp, config.clipDefenceDown) : StateClip_(config);
    SetClip(_e, wantClip);
}

bool ShielderEnemy::OnHit(
    const EnemyDef& _def,
    EnemyEnv&,
    const HitEvent& _event)
{
    EnemyState& _e = GetMutableState();
    if ((_event.target & kShieldOwnerBit) != 0)
    {
        _e.defenceTicks = _def.shielder.defenceTicks;
        return true;
    }
    return false;
}

void ShielderEnemy::OnDeath(
    const EnemyDef& _def,
    EnemyEnv&       _env)
{
    EnemyState& _e = GetMutableState();
    _e.bShieldUp   = false;
    BeginSoldierDeath(_e, _def, _env);
}

void ShielderEnemy::TickDying(
    const EnemyDef& _def,
    EnemyEnv&       _env)
{
    TickSoldierDeath(GetMutableState(), _def, _env);
}

void ShielderEnemy::RegisterHurtboxes(
    const EnemyDef& _def,
    CombatSystem&   _combat) const
{
    const EnemyState&     _e     = GetState();
    const ShielderConfig& config = _def.shielder;
    if (_e.bShieldUp)
    {
        const BoxDef& box = _e.shieldDir == 1 ? config.shieldUp : (_e.shieldDir == 2 ? config.shieldDown : config.shieldFront);
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

void ParseShielderConfig(
    DefReader& _r,
    EnemyDef&  _def)
{
    ShielderConfig& config = _def.shielder;

    config.cooldownTicks = _r.Ticks("cooldown");
    config.detect        = _r.Num("detect");
    config.guard         = _r.Num("guard");
    config.levelDy       = _r.Num("levelDy");
    config.upDy          = _r.Num("upDy");
    config.moveDy        = _r.Num("moveDy");
    config.bullets       = _r.Int("bullets");
    config.startTicks    = _r.Ticks("startSec");
    config.readyTicks    = _r.Ticks("readySec");
    config.moveMaxTicks  = _r.Ticks("moveMaxSec");
    config.fireFrame     = _r.Int("fireFrame");
    config.edgeProbeDist = _r.Num("edgeProbeDist");
    config.edgeProbeDrop = _r.Num("edgeProbeDrop");
    config.defenceTicks  = _r.Ticks("defenceSec");
    {
        const BoxDef probe = _r.Box("wallProbe");
        config.wallProbeW  = probe.w;
        config.wallProbeH  = probe.h;
        config.wallProbeX  = probe.cx;
        config.wallProbeY  = probe.cy;
    }
    config.shieldFront = _r.Box("shieldFront");
    config.shieldUp    = _r.Box("shieldUp");
    config.shieldDown  = _r.Box("shieldDown");

    config.clipIdle          = _r.Clip("idle");
    config.clipMove          = _r.Clip("move");
    config.clipMoveStop      = _r.Clip("moveStop");
    config.clipFall          = _r.Clip("fall");
    config.clipShotStart     = _r.Clip("shotStart");
    config.clipShotReady     = _r.Clip("shotReady");
    config.clipShot          = _r.Clip("shot");
    config.clipShotEnd       = _r.Clip("shotEnd");
    config.clipShotStartUp   = _r.Clip("shotStartUp");
    config.clipShotReadyUp   = _r.Clip("shotReadyUp");
    config.clipShotUp        = _r.Clip("shotUp");
    config.clipShotEndUp     = _r.Clip("shotEndUp");
    config.clipShotStartDown = _r.Clip("shotStartDown");
    config.clipShotReadyDown = _r.Clip("shotReadyDown");
    config.clipShotDown      = _r.Clip("shotDown");
    config.clipShotEndDown   = _r.Clip("shotEndDown");
    config.clipLookUpStart   = _r.Clip("lookUpStart");
    config.clipLookUpIdle    = _r.Clip("lookUpIdle");
    config.clipLookUpEnd     = _r.Clip("lookUpEnd");
    config.clipLookDownStart = _r.Clip("lookDownStart");
    config.clipLookDownIdle  = _r.Clip("lookDownIdle");
    config.clipLookDownEnd   = _r.Clip("lookDownEnd");
    config.clipDefence       = _r.Clip("defence");
    config.clipDefenceUp     = _r.Clip("defenceUp");
    config.clipDefenceDown   = _r.Clip("defenceDown");

    config.weaponForward = _r.Weapon("forward");
    config.weaponUp      = _r.Weapon("up");
    config.weaponDown    = _r.Weapon("down");
}

[[nodiscard]] int ShielderEnemy::StateClip_(
    const ShielderConfig& _config) const
{
    const EnemyState& _e = GetState();
    switch (static_cast<eShielderState>(_e.state))
    {
        case eShielderState::Wait:
        case eShielderState::Idle: return _config.clipIdle;
        case eShielderState::Fall: return _config.clipFall;
        case eShielderState::Move: return _config.clipMove;
        case eShielderState::MoveStop: return _config.clipMoveStop;
        case eShielderState::ShotStart: return PickDir_(_e.shieldDir, _config.clipShotStart, _config.clipShotStartUp, _config.clipShotStartDown);
        case eShielderState::ShotReady: return PickDir_(_e.shieldDir, _config.clipShotReady, _config.clipShotReadyUp, _config.clipShotReadyDown);
        case eShielderState::Shot: return PickDir_(_e.shieldDir, _config.clipShot, _config.clipShotUp, _config.clipShotDown);
        case eShielderState::ShotEnd: return PickDir_(_e.shieldDir, _config.clipShotEnd, _config.clipShotEndUp, _config.clipShotEndDown);
        case eShielderState::LookStart: return PickDir_(_e.shieldDir, _config.clipIdle, _config.clipLookUpStart, _config.clipLookDownStart);
        case eShielderState::LookIdle: return PickDir_(_e.shieldDir, _config.clipIdle, _config.clipLookUpIdle, _config.clipLookDownIdle);
        case eShielderState::LookEnd: return PickDir_(_e.shieldDir, _config.clipIdle, _config.clipLookUpEnd, _config.clipLookDownEnd);
        default: return _config.clipIdle;
    }
}

[[nodiscard]] bool ShielderEnemy::StateDone_(
    const EnemyDef& _def) const
{
    const EnemyState& _e = GetState();
    return _e.stateTicks >= _def.GetClip(StateClip_(_def.shielder)).GetDurationTicks();
}

void ShielderEnemy::Go_(
    const EnemyDef&      _def,
    EnemyEnv&            _env,
    const eShielderState _state,
    const int            _dir)
{
    EnemyState&           _e     = GetMutableState();
    const ShielderConfig& config = _def.shielder;
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
            m_shotsRemaining = config.bullets;
            break;
        case eShielderState::LookStart:
            _e.bShieldUp = true;
            FacePlayer(_e, _env);
            m_shotsRemaining = config.bullets;
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
    SetClip(_e, StateClip_(config), true);
}

void ShielderEnemy::HashActionState(Fnv1a& _hash) const
{
    HashActionState_(_hash, m_shotsRemaining, 0);
}
