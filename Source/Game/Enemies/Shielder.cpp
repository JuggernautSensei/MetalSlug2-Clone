#include <cmath>

#include "Game/Enemies/EnemyCommon.h"
#include "Game/Enemies/EnemyDefParse.h"

namespace slug
{

// ===========================================
//  Shielder — 방패병 (Docs/EnemySpecs.md §1.5)
//   방패는 별도 Hurtbox (소유자 = 적 ID | kShieldOwnerBit, 받는 종류 = Bullet 만) 로 표현한다.
//   몸통보다 먼저 등록하므로 정면에서 오는 탄은 방패가 먼저 받아 소모된다. 칼(Melee)/폭탄(Blast) 은 방패가 받지 않아 몸통에 닿고,
//   관통 샷건 탄은 방패와 몸통을 모두 맞춘다 (소유자가 다르므로).
//   방패는 사격 자세(ShotReady 이후)에서는 내려간다 (취약).
//   레거시 구멍 수정: -100 < dy < -50 구간도 수평으로 취급한다.
// ===========================================

namespace
{
    enum eShielderState : int
    {
        kWait,
        kFall,
        kIdle,
        kMove,
        kMoveStop,
        kShotStart,
        kShotReady,
        kShot,
        kShotEnd,
        kLookStart,
        kLookIdle,
        kLookEnd
    };

    // e.shieldDir: 0 앞 / 1 위 / 2 아래 (조준/방패 방향). e.counter: 남은 발수.

    [[nodiscard]] int PickDir_(const int _dir, const int _fwd, const int _up, const int _down)
    {
        return _dir == 1 ? _up : (_dir == 2 ? _down : _fwd);
    }

    [[nodiscard]] int StateClip_(const Enemy& _e, const ShielderCfg& _c)
    {
        switch (_e.state)
        {
            case kWait:
            case kIdle: return _c.clipIdle;
            case kFall: return _c.clipFall;
            case kMove: return _c.clipMove;
            case kMoveStop: return _c.clipMoveStop;
            case kShotStart: return PickDir_(_e.shieldDir, _c.clipShotStart, _c.clipShotStartUp, _c.clipShotStartDown);
            case kShotReady: return PickDir_(_e.shieldDir, _c.clipShotReady, _c.clipShotReadyUp, _c.clipShotReadyDown);
            case kShot: return PickDir_(_e.shieldDir, _c.clipShot, _c.clipShotUp, _c.clipShotDown);
            case kShotEnd: return PickDir_(_e.shieldDir, _c.clipShotEnd, _c.clipShotEndUp, _c.clipShotEndDown);
            case kLookStart: return PickDir_(_e.shieldDir, _c.clipIdle, _c.clipLookUpStart, _c.clipLookDownStart);
            case kLookIdle: return PickDir_(_e.shieldDir, _c.clipIdle, _c.clipLookUpIdle, _c.clipLookDownIdle);
            case kLookEnd: return PickDir_(_e.shieldDir, _c.clipIdle, _c.clipLookUpEnd, _c.clipLookDownEnd);
            default: return _c.clipIdle;
        }
    }

    [[nodiscard]] bool StateDone_(const Enemy& _e, const EnemyDef& _def)
    {
        return _e.stateTicks >= _def.GetClip(StateClip_(_e, _def.shielder)).GetDurationTicks();
    }

    void Go_(Enemy& _e, const EnemyDef& _def, EnemyEnv& _env, const int _state, const int _dir = -1)
    {
        const ShielderCfg& cfg = _def.shielder;
        EnterState(_e, _state);
        if (_dir >= 0)
        {
            _e.shieldDir = _dir;
        }
        switch (_state)
        {
            case kIdle:
            case kMove:
            case kMoveStop:
                _e.shieldDir = 0;
                _e.bShieldUp = true;
                FacePlayer(_e, _env);
                break;
            case kWait:
            case kFall:
                _e.shieldDir = 0;
                _e.bShieldUp = true;
                break;
            case kShotStart:
                _e.shieldDir = 0;
                _e.bShieldUp = true;   // 사격 준비 동안은 아직 방어 중
                FacePlayer(_e, _env);
                _e.counter = cfg.bullets;
                break;
            case kLookStart:
                _e.bShieldUp = true;
                FacePlayer(_e, _env);
                _e.counter = cfg.bullets;
                break;
            case kShotReady:
            case kShot:
            case kShotEnd:
                _e.bShieldUp = false;   // 사격 중에는 취약
                break;
            default: break;
        }
        if (_state == kMove || _state == kMoveStop || _state == kIdle || _state == kWait)
        {
            _e.body.velocity.x = 0.f;
        }
        SetClip(_e, StateClip_(_e, cfg), true);
    }

    void Init_(Enemy& _e, const EnemyDef& _def, EnemyEnv& _env)
    {
        PrepareBody(_e, _def);
        Go_(_e, _def, _env, kWait);
    }

    void Tick_(Enemy& _e, const EnemyDef& _def, EnemyEnv& _env)
    {
        const ShielderCfg& cfg   = _def.shielder;
        const float        speed = _def.physics.speed;

        const float dy    = DyToPlayer(_e, _env);
        const float absDx = std::abs(DxToPlayer(_e, _env));

        switch (_e.state)
        {
            case kWait:
                _e.body.velocity.x = 0.f;
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
                _e.body.velocity.x = 0.f;
                FacePlayer(_e, _env);
                if (_e.stateTicks >= cfg.cooldownTicks)
                {
                    if (dy < -cfg.upDy)
                    {
                        if (absDx <= cfg.guard)
                        {
                            Go_(_e, _def, _env, kLookStart, 1);
                        }
                        else
                        {
                            Go_(_e, _def, _env, kMove);
                        }
                    }
                    else if (dy > cfg.levelDy)
                    {
                        if (absDx <= cfg.guard)
                        {
                            Go_(_e, _def, _env, kLookStart, 2);
                        }
                        else
                        {
                            Go_(_e, _def, _env, kMove);
                        }
                    }
                    else if (absDx >= cfg.detect)
                    {
                        Go_(_e, _def, _env, kMove);
                    }
                    else
                    {
                        Go_(_e, _def, _env, Roll(_env, 2) == 0 ? kIdle : kShotStart);
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
                if (!FloorAhead(_e, _env, cfg.edgeProbeDist, cfg.edgeProbeDrop) || WallAhead(_e, _env, cfg.wallProbeW, cfg.wallProbeH, cfg.wallProbeX, cfg.wallProbeY))
                {
                    Go_(_e, _def, _env, kMoveStop);   // 낭떠러지/벽 앞에서는 멈춘다 (레거시: 그대로 떨어짐)
                }
                else if (_e.stateTicks >= cfg.moveMaxTicks)
                {
                    Go_(_e, _def, _env, Roll(_env, 2) == 0 ? kMoveStop : kMove);
                }
                else if (absDx <= cfg.detect && std::abs(dy) <= cfg.moveDy)
                {
                    Go_(_e, _def, _env, kShotStart);
                }
                else if (absDx <= cfg.guard && dy < -cfg.moveDy)
                {
                    Go_(_e, _def, _env, kLookStart, 1);
                }
                else if (absDx <= cfg.guard && dy > cfg.moveDy)
                {
                    Go_(_e, _def, _env, kLookStart, 2);
                }
                else
                {
                    _e.body.velocity.x = static_cast<float>(_e.facing) * speed;
                }
                break;

            case kMoveStop:
                _e.body.velocity.x = 0.f;
                if (StateDone_(_e, _def))
                {
                    Go_(_e, _def, _env, kIdle);
                }
                break;

            case kShotStart:
                _e.body.velocity.x = 0.f;
                if (_e.stateTicks >= cfg.startTicks)
                {
                    Go_(_e, _def, _env, kShotReady);
                }
                break;

            case kShotReady:
                _e.body.velocity.x = 0.f;
                if (_e.stateTicks >= cfg.readyTicks)
                {
                    if (_e.counter <= 0)
                    {
                        Go_(_e, _def, _env, kShotEnd);
                    }
                    else
                    {
                        --_e.counter;
                        Go_(_e, _def, _env, kShot);
                    }
                }
                break;

            case kShot:
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
                    Go_(_e, _def, _env, kShotReady);
                }
                break;

            case kShotEnd:
                _e.body.velocity.x = 0.f;
                if (StateDone_(_e, _def))
                {
                    Go_(_e, _def, _env, kIdle);
                }
                break;

            case kLookStart:
                _e.body.velocity.x = 0.f;
                if (StateDone_(_e, _def))
                {
                    Go_(_e, _def, _env, kLookIdle);
                }
                break;

            case kLookIdle:
                _e.body.velocity.x = 0.f;
                if (_e.stateTicks >= cfg.cooldownTicks)
                {
                    if (absDx <= cfg.guard)
                    {
                        Go_(_e, _def, _env, kShotReady);   // 수직 사격 (Look 자세에서 바로)
                    }
                    else
                    {
                        Go_(_e, _def, _env, kLookEnd);
                    }
                }
                break;

            case kLookEnd:
                _e.body.velocity.x = 0.f;
                if (StateDone_(_e, _def))
                {
                    Go_(_e, _def, _env, kIdle);
                }
                break;

            default: break;
        }

        (void)StepPhysics(_e, _def, _env);

        if (_e.state == kFall && _e.bGrounded)
        {
            Go_(_e, _def, _env, _e.bActive ? kIdle : kWait);
        }

        // 방어 연출: 방패가 올라와 있는 동안 막으면 잠깐 방어 클립을 덮어쓴다 (상태 타이머는 클립이 아니라 stateTicks 로 센다)
        if (_e.defenceTicks > 0)
        {
            --_e.defenceTicks;
        }
        const int wantClip = (_e.defenceTicks > 0 && _e.bShieldUp) ? PickDir_(_e.shieldDir, cfg.clipDefence, cfg.clipDefenceUp, cfg.clipDefenceDown) : StateClip_(_e, cfg);
        SetClip(_e, wantClip);
    }

    bool OnHit_(Enemy& _e, const EnemyDef& _def, EnemyEnv&, const HitEvent& _event)
    {
        if ((_event.target & kShieldOwnerBit) != 0)
        {
            _e.defenceTicks = _def.shielder.defenceTicks;   // 방패가 막음: 피해 없음
            return true;
        }
        return false;
    }

    void OnDeath_(Enemy& _e, const EnemyDef& _def, EnemyEnv& _env)
    {
        _e.bShieldUp = false;
        BeginSoldierDeath(_e, _def, _env);
    }

    void RegisterHurtboxes_(const Enemy& _e, const EnemyDef& _def, CombatSystem& _combat)
    {
        const ShielderCfg& cfg = _def.shielder;
        if (_e.bShieldUp)
        {
            const BoxDef& box = _e.shieldDir == 1 ? cfg.shieldUp : (_e.shieldDir == 2 ? cfg.shieldDown : cfg.shieldFront);
            Hurtbox shield;
            shield.box     = box.At(_e.body.position, _e.facing);
            shield.team    = _def.team;
            shield.owner   = _e.id | kShieldOwnerBit;
            shield.accepts = HitKindMask { eHitKind::Bullet };   // 총알만 받아 소모시킨다
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
        c.wallProbeW = probe.w;
        c.wallProbeH = probe.h;
        c.wallProbeX = probe.cx;
        c.wallProbeY = probe.cy;
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

}   // namespace slug
