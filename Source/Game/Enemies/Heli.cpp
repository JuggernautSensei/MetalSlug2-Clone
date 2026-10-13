#include <algorithm>
#include <cmath>

#include "Base/Math.h"
#include "Game/Enemies/EnemyCommon.h"
#include "Game/Enemies/EnemyDefParse.h"
#include "Game/Enemies/VehicleCommon.h"

// 헬기 상태
enum class eHeliState
{
    Bomb,
    Low,
    PodOut,
    Move,
    Volley,
    MoveBack,
    Turn   // 방향 전환
};

constexpr int kExtStartX   = 0;
constexpr int kExtStartY   = 1;
constexpr int kExtTargetX  = 2;
constexpr int kExtTargetY  = 3;
constexpr int kExtBombTime = 4;
constexpr int kExtTurnFlip = 5;
constexpr int kExtPodTicks = 6;   // 포드 전개 후 경과 틱
constexpr int kExtCycle    = 7;
constexpr int kExtPassDir  = 8;
constexpr int kExtIdleTick = 9;   // 호버 idle 위상

constexpr float kFixedScale = 16.f;
constexpr int   kCurveSteps = 12;

namespace
{
[[nodiscard]] int Pack_(
    const float _value)
{
    return static_cast<int>(std::lround(static_cast<double>(_value * kFixedScale)));
}

[[nodiscard]] float Unpack_(
    const int _value)
{
    return static_cast<float>(_value) / kFixedScale;
}

// 곡선 표 선형 보간
[[nodiscard]] float SampleCurve_(
    const ARRAY<float, 13>& _curve,
    const float             _u)
{
    const float u     = Clamp(_u, 0.f, 1.f) * static_cast<float>(kCurveSteps);
    const int   index = Min(static_cast<int>(u), kCurveSteps - 1);
    const float t     = u - static_cast<float>(index);
    return _curve[static_cast<size_t>(index)] + (_curve[static_cast<size_t>(index) + 1] - _curve[static_cast<size_t>(index)]) * t;
}

[[nodiscard]] float Sign_(
    const int _facing)
{
    return _facing < 0 ? -1.f : 1.f;
}

[[nodiscard]] float HoverY_(
    const HeliCfg&  _c,
    const EnemyEnv& _env)
{
    return _env.ctx.cameraView.min.y + _c.hoverDy;
}

[[nodiscard]] float FollowTargetX_(
    const HeliCfg&  _c,
    const EnemyEnv& _env)
{
    return _env.ctx.player.feet.x + _c.followDx;
}

void SetPodTicks_(
    Enemy&    _e,
    const int _ticks)
{
    _e.ext[kExtPodTicks] = _ticks;
}

void Go_(
    Enemy&           _e,
    const EnemyDef&  _def,
    const eHeliState _state)
{
    const HeliCfg& c = _def.heli;
    EnterState(_e, static_cast<int>(_state));
    switch (_state)
    {
        case eHeliState::Turn: SetClip(_e, c.clipTurnIn, true); break;
        case eHeliState::Low:
        {
            const bool bForward = (_e.ext[kExtPassDir] < 0) == (_e.facing < 0);
            SetClip(_e, bForward ? c.clipPitch : c.clipFly, true);
            break;
        }
        default: SetClip(_e, c.clipFly, true); break;
    }
}

void BeginMove_(
    Enemy&           _e,
    const EnemyDef&  _def,
    const EnemyEnv&  _env,
    const eHeliState _state,
    const VECTOR2    _target)
{
    Go_(_e, _def, _state);
    _e.ext[kExtStartX]  = Pack_(_e.body.position.x);
    _e.ext[kExtStartY]  = Pack_(_e.body.position.y);
    _e.ext[kExtTargetX] = Pack_(_target.x);
    _e.ext[kExtTargetY] = Pack_(_target.y);
    (void)_env;
}

[[nodiscard]] bool StepMove_(
    Enemy&         _e,
    const HeliCfg& _c)
{
    const float   u      = static_cast<float>(_e.stateTicks) / static_cast<float>(Max(1, _c.moveTicks));
    const VECTOR2 start  = { Unpack_(_e.ext[kExtStartX]), Unpack_(_e.ext[kExtStartY]) };
    const VECTOR2 target = { Unpack_(_e.ext[kExtTargetX]), Unpack_(_e.ext[kExtTargetY]) };
    _e.body.position     = { start.x + (target.x - start.x) * SampleCurve_(_c.xCurve, u), start.y + (target.y - start.y) * SampleCurve_(_c.yCurve, u) };
    return _e.stateTicks >= _c.moveTicks;
}

void StepFollow_(
    Enemy&          _e,
    const HeliCfg&  _c,
    const EnemyEnv& _env)
{
    const float hoverY = HoverY_(_c, _env);
    if (_e.body.position.y < hoverY)
    {
        _e.body.position.y = Min(_e.body.position.y + _c.enterSpeedY * kStepSec, hoverY);
    }
    const float dx = FollowTargetX_(_c, _env) - _e.body.position.x;
    if (Abs(dx) > _c.followDeadzone)
    {
        const float step = Min(_c.followSpeed * kStepSec, Abs(dx));
        _e.body.position.x += (dx < 0.f ? -step : step);
    }
}

void DropBomb_(
    Enemy&          _e,
    const EnemyDef& _def,
    EnemyEnv&       _env)
{
    const HeliCfg& c         = _def.heli;
    const int      bombIndex = _env.lib.FindIndex(c.bombDef);
    if (bombIndex < 0)
    {
        return;
    }
    SpawnParams params;
    params.facing     = _e.facing;
    params.bActivated = true;
    const VECTOR2 at  = { _e.body.position.x + c.bombMuzzle.x * Sign_(_e.facing), _e.body.position.y + c.bombMuzzle.y };
    (void)_env.world.SpawnDeferred(bombIndex, at, params);
}

[[nodiscard]] int BurstPeriod_(
    const HeliCfg& _c)
{
    return Max(1, _c.burstCount * _c.bombIntervalTicks + _c.burstRestTicks);
}

// _first 틱부터 연속 투하를 시작해 _bursts 번 하고 마지막 휴지까지 끝나는 틱
[[nodiscard]] int BurstPhaseEnd_(
    const HeliCfg& _c,
    const int      _first,
    const int      _bursts)
{
    return _first + _bursts * BurstPeriod_(_c);
}

[[nodiscard]] bool IsBombTick_(
    const HeliCfg& _c,
    const int      _ticks,
    const int      _first)
{
    const int t = _ticks - _first;
    if (t < 0)
    {
        return false;
    }
    const int k = t % BurstPeriod_(_c);
    return k < _c.burstCount * _c.bombIntervalTicks && k % _c.bombIntervalTicks == 0;
}

[[nodiscard]] int FirstBombTicks_(
    const Enemy&   _e,
    const HeliCfg& _c)
{
    return _e.ext[kExtCycle] == 0 ? _c.firstBombTicks : _c.reBombTicks;
}

void StepBombing_(
    Enemy&          _e,
    const EnemyDef& _def,
    EnemyEnv&       _env,
    const int       _first,
    const int       _bursts)
{
    const HeliCfg& c = _def.heli;
    if (!_env.ctx.player.bAlive || _e.stateTicks >= BurstPhaseEnd_(c, _first, _bursts) || !IsBombTick_(c, _e.stateTicks, _first))
    {
        return;
    }
    if (_e.body.position.y < _env.ctx.cameraView.min.y + c.bombMinY)
    {
        return;
    }
    DropBomb_(_e, _def, _env);
}

[[nodiscard]] float LowTargetX_(
    const Enemy&    _e,
    const HeliCfg&  _c,
    const EnemyEnv& _env)
{
    const Aabb& view = _env.ctx.cameraView;
    return _e.ext[kExtPassDir] > 0 ? view.max.x - _c.lowMargin : view.min.x + _c.lowMargin;
}

void StepLow_(
    Enemy&          _e,
    const HeliCfg&  _c,
    const EnemyEnv& _env)
{
    const float lowY = _env.ctx.cameraView.min.y + _c.lowDy;
    if (_e.body.position.y < lowY)
    {
        _e.body.position.y = Min(_e.body.position.y + _c.descendSpeedY * kStepSec, lowY);
    }
    const float dx   = LowTargetX_(_e, _c, _env) - _e.body.position.x;
    const float step = Min(_c.lowSpeed * kStepSec, Abs(dx));
    _e.body.position.x += (dx < 0.f ? -step : step);
}

void BeginLow_(
    Enemy&          _e,
    const EnemyDef& _def,
    const EnemyEnv& _env)
{
    const Aabb& view    = _env.ctx.cameraView;
    _e.ext[kExtPassDir] = _e.body.position.x < (view.min.x + view.max.x) * 0.5f ? 1 : -1;
    Go_(_e, _def, eHeliState::Low);
}

[[nodiscard]] bool WantsTurn_(
    Enemy&          _e,
    const HeliCfg&  _c,
    const EnemyEnv& _env)
{
    const float behind = -DxToPlayer(_e, _env) * Sign_(_e.facing);
    if (behind > _c.turnDx)
    {
        ++_e.aux3;
    }
    else
    {
        _e.aux3 = 0;
    }
    return _e.aux3 >= _c.turnWaitTicks;
}

void Init_(
    Enemy&          _e,
    const EnemyDef& _def,
    EnemyEnv&       _env)
{
    PrepareBody(_e, _def);
    _e.body.bUseGravity = false;
    SetPodTicks_(_e, -1);
    if (_def.heli.startLow != 0)
    {
        BeginLow_(_e, _def, _env);
        return;
    }
    Go_(_e, _def, eHeliState::Bomb);
}

void Tick_(
    Enemy&          _e,
    const EnemyDef& _def,
    EnemyEnv&       _env)
{
    const HeliCfg& c = _def.heli;
    _e.body.velocity = { 0.f, 0.f };
    if (!_e.bActive)
    {
        return;
    }
    if (_e.ext[kExtPodTicks] >= 0)
    {
        ++_e.ext[kExtPodTicks];
    }
    ++_e.ext[kExtIdleTick];

    switch (static_cast<eHeliState>(_e.state))
    {
        case eHeliState::Bomb:
            StepFollow_(_e, c, _env);
            StepBombing_(_e, _def, _env, FirstBombTicks_(_e, c), c.highBursts);
            if (_e.stateTicks >= BurstPhaseEnd_(c, FirstBombTicks_(_e, c), c.highBursts))
            {
                BeginLow_(_e, _def, _env);
            }
            else if (WantsTurn_(_e, c, _env))
            {
                _e.aux3              = 0;
                _e.ext[kExtBombTime] = _e.stateTicks;
                _e.ext[kExtTurnFlip] = 0;
                Go_(_e, _def, eHeliState::Turn);
            }
            break;

        case eHeliState::Low:
            StepLow_(_e, c, _env);
            StepBombing_(_e, _def, _env, c.reBombTicks, c.lowBursts);
            if (_e.stateTicks >= BurstPhaseEnd_(c, c.reBombTicks, c.lowBursts))
            {
                SetPodTicks_(_e, 0);
                Go_(_e, _def, eHeliState::PodOut);
            }
            break;

        case eHeliState::Turn:
        {
            StepFollow_(_e, c, _env);
            const bool bFlipped = _e.ext[kExtTurnFlip] != 0;
            if (!bFlipped && ClipFinished(_e, _def))
            {
                _e.facing            = -_e.facing;
                _e.ext[kExtTurnFlip] = 1;
                SetClip(_e, c.clipTurnOut, true);
            }
            else if (bFlipped && ClipFinished(_e, _def))
            {
                Go_(_e, _def, eHeliState::Bomb);
                _e.stateTicks = _e.ext[kExtBombTime];
            }
            break;
        }

        case eHeliState::PodOut:
            if (_e.stateTicks >= c.podOutTicks)
            {
                const Aabb& view    = _env.ctx.cameraView;
                const float targetX = _e.facing < 0 ? view.max.x - c.sideMargin : view.min.x + c.sideMargin;
                const float targetY = Clamp(_env.ctx.player.center.y + c.targetDyPlayer, view.min.y + c.targetMinY, view.min.y + c.targetMaxY);
                BeginMove_(_e, _def, _env, eHeliState::Move, { targetX, targetY });
            }
            break;

        case eHeliState::Move:
            if (StepMove_(_e, c))
            {
                _e.counter = 0;
                Go_(_e, _def, eHeliState::Volley);
                RequestFx(_env, FxDef { _def.GetClip(c.clipPuff).name, c.puffOffset, 0.f, c.partPuff.depth }, _e.body.position, _e.facing);
            }
            break;

        case eHeliState::Volley:
        {
            const int t = _e.stateTicks;
            if (t > 0 && t % c.missileInterval == 0 && t / c.missileInterval < c.missiles)
            {
                RequestFx(_env, FxDef { _def.GetClip(c.clipPuff).name, c.puffOffset, 0.f, c.partPuff.depth }, _e.body.position, _e.facing);
            }
            const int sinceLaunch = t - c.puffToMissile;
            if (sinceLaunch >= 0 && sinceLaunch % c.missileInterval == 0 && sinceLaunch / c.missileInterval < c.missiles)
            {
                (void)FireWeapon(_e, _def, _env, c.weaponMissile);
            }
            if (t >= (c.missiles - 1) * c.missileInterval + c.volleyEndTicks)
            {
                SetPodTicks_(_e, -1);
                BeginMove_(_e, _def, _env, eHeliState::MoveBack, { Clamp(FollowTargetX_(c, _env), _env.ctx.cameraView.min.x + 60.f, _env.ctx.cameraView.max.x - 60.f), HoverY_(c, _env) });
            }
            break;
        }

        case eHeliState::MoveBack:
            if (StepMove_(_e, c))
            {
                ++_e.ext[kExtCycle];
                Go_(_e, _def, eHeliState::Bomb);
            }
            break;
    }
}

void OnDeath_(
    Enemy&          _e,
    const EnemyDef& _def,
    EnemyEnv&       _env)
{
    BeginVehicleDeath(_e, _def, _env, -1);
    _e.bVisible = false;
    SetPodTicks_(_e, -1);
}

// 지금 몸통이 보여 주는 Pitch 프레임
[[nodiscard]] int ShownPitchFrame_(
    const Enemy&            _e,
    const EnemyDef&         _def,
    const AnimationLibrary& _anims)
{
    const HeliCfg& c = _def.heli;
    if (_e.clip == c.clipFly && c.idlePitchTo > c.idlePitchFrom)
    {
        const int span = c.idlePitchTo - c.idlePitchFrom;
        const int m    = (_e.ext[kExtIdleTick] / Max(1, c.idleStepTicks)) % (span * 2);
        return c.idlePitchFrom + (m <= span ? m : span * 2 - m);
    }
    if (_e.clip == c.clipPitch)
    {
        const AnimClip* pClip = _anims.FindClip(ResolveSide(_def.GetClip(c.clipPitch).name, _e.facing));
        if (pClip && !pClip->frames.empty() && pClip->GetFrameDurationUnits() > 0)
        {
            const int index = static_cast<int>(static_cast<int64_t>(_e.clipTicks) * kAnimUnitsPerTick / pClip->GetFrameDurationUnits());
            return pClip->bLoop ? index % pClip->GetFrameCount() : Min(index, pClip->GetFrameCount() - 1);
        }
    }
    return -1;
}

const AnimFrame* BodyFrame_(
    const Enemy&            _e,
    const EnemyDef&         _def,
    const AnimationLibrary& _anims)
{
    const HeliCfg& c     = _def.heli;
    const int      shown = ShownPitchFrame_(_e, _def, _anims);
    if (_e.clip != c.clipFly || shown < 0)
    {
        return nullptr;
    }
    const AnimClip* pClip = _anims.FindClip(ResolveSide(_def.GetClip(c.clipPitch).name, _e.facing));
    if (!pClip || pClip->frames.empty())
    {
        return nullptr;
    }
    return &pClip->frames[static_cast<size_t>(Min(shown, pClip->GetFrameCount() - 1))];
}

void SubmitParts_(
    const Enemy&          _e,
    const EnemyDef&       _def,
    const EnemyRenderEnv& _render)
{
    const HeliCfg& c     = _def.heli;
    const float    sign  = Sign_(_e.facing);
    const bool     bTurn = static_cast<eHeliState>(_e.state) == eHeliState::Turn;
    if (bTurn)
    {
        return;
    }

    const int feetKey = static_cast<int>(_render.feet.y);
    if (_e.ext[kExtPodTicks] >= 0)
    {
        const VECTOR2 at = { _render.feet.x + c.podOffset.x * sign, _render.feet.y + c.podOffset.y };
        (void)SubmitNamedPart(_render, ResolveSide(_def.GetClip(c.clipPod).name, _e.facing), at, _e.ext[kExtPodTicks], false, feetKey + c.partPod.sort, c.partPod.depth);
    }
    const VECTOR2 rotor     = { _render.feet.x + c.rotorOffset.x * sign, _render.feet.y + c.rotorOffset.y };
    const int     shown     = ShownPitchFrame_(_e, _def, _render.anims);
    const String  rotorName = c.rotorRowPrefix.empty() || shown < 0 ? ResolveSide(_def.GetClip(c.clipRotor).name, _e.facing) : c.rotorRowPrefix + std::to_string(shown);
    (void)SubmitNamedPart(_render, rotorName, rotor, _e.clipTicks, true, feetKey + c.partRotor.sort, c.partRotor.depth);
}
}   // namespace

const BehaviorOps kHeliOps = {
    .init              = &Init_,
    .tick              = &Tick_,
    .onDeath           = &OnDeath_,
    .tickDying         = &TickVehicleDeath,
    .registerHurtboxes = nullptr,
    .submitParts       = &SubmitParts_,
    .bodyFrame         = &BodyFrame_,
};

void ParseHeliCfg(
    DefReader& _r,
    EnemyDef&  _def)
{
    HeliCfg& c = _def.heli;

    c.enterSpeedY       = _r.Num("enterSpeedY", c.enterSpeedY);
    c.hoverDy           = _r.Num("hoverDy", c.hoverDy);
    c.followDx          = _r.Num("followDx", c.followDx);
    c.followSpeed       = _r.Num("followSpeed", c.followSpeed);
    c.followDeadzone    = _r.Num("followDeadzone", c.followDeadzone);
    c.firstBombTicks    = _r.Ticks("firstBombSec", 0.9f);
    c.bombIntervalTicks = Max(1, _r.Ticks("bombIntervalSec", 0.4f));
    c.reBombTicks       = _r.Ticks("reBombSec", 0.4f);
    c.burstCount        = Max(1, _r.Int("burstCount", c.burstCount));
    c.burstRestTicks    = Max(0, _r.Ticks("burstRestSec", 1.4f));
    c.highBursts        = Max(1, _r.Int("highBursts", c.highBursts));
    c.lowBursts         = Max(1, _r.Int("lowBursts", c.lowBursts));
    c.lowDy             = _r.Num("lowDy", c.lowDy);
    c.lowSpeed          = _r.Num("lowSpeed", c.lowSpeed);
    c.lowMargin         = _r.Num("lowMargin", c.lowMargin);
    c.descendSpeedY     = _r.Num("descendSpeedY", c.descendSpeedY);
    c.startLow          = _r.Int("startLow", c.startLow);
    c.bombMuzzle        = _r.ReadVec2("bombMuzzle", c.bombMuzzle);
    c.bombMinY          = _r.Num("bombMinY", c.bombMinY);

    c.podOutTicks    = _r.Ticks("podOutSec", 0.2f);
    c.moveTicks      = Max(1, _r.Ticks("moveSec", 1.17f));
    c.sideMargin     = _r.Num("sideMargin", c.sideMargin);
    c.targetDyPlayer = _r.Num("targetDyPlayer", c.targetDyPlayer);
    c.targetMinY     = _r.Num("targetMinY", c.targetMinY);
    c.targetMaxY     = _r.Num("targetMaxY", c.targetMaxY);
    (void)_r.NumArray("xCurve", c.xCurve);
    (void)_r.NumArray("yCurve", c.yCurve);
    c.missiles        = _r.Int("missiles", c.missiles);
    c.puffToMissile   = _r.Ticks("puffToMissileSec", 0.167f);
    c.missileInterval = Max(1, _r.Ticks("missileIntervalSec", 2.1f));
    c.volleyEndTicks  = _r.Ticks("volleyEndSec", 1.5f);
    c.puffOffset      = _r.ReadVec2("puffOffset", c.puffOffset);
    c.turnWaitTicks   = _r.Ticks("turnWaitSec", 0.4f);
    c.turnDx          = _r.Num("turnDx", c.turnDx);
    c.podOffset       = _r.ReadVec2("podOffset", c.podOffset);
    c.rotorOffset     = _r.ReadVec2("rotorOffset", c.rotorOffset);
    c.idlePitchFrom   = Max(0, _r.Int("idlePitchFrom", c.idlePitchFrom));
    c.idlePitchTo     = Max(0, _r.Int("idlePitchTo", c.idlePitchTo));
    c.idleStepTicks   = Max(1, _r.Ticks("idleStepSec", 0.2f));
    c.rotorRowPrefix  = _r.Str("rotorRowPrefix");

    c.clipFly       = _r.Clip("fly");
    c.clipPitch     = _r.Clip("pitch");
    c.clipTurnIn    = _r.Clip("turnIn");
    c.clipTurnOut   = _r.Clip("turnOut");
    c.clipPod       = _r.Clip("pod");
    c.clipRotor     = _r.Clip("rotor");
    c.clipPuff      = _r.Clip("puff");
    c.weaponMissile = _r.Weapon("missile");
    c.bombDef       = _r.Str("bombDef");
}
