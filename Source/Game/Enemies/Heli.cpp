#include "Game/Enemies/HeliEnemy.h"

#include <algorithm>
#include <cmath>

#include "Game/Enemies/EnemyCommon.h"
#include "Game/Enemies/EnemyDefParse.h"
#include "Game/Enemies/VehicleCommon.h"

// 헬기 상태

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
    const HeliConfig& _config,
    const EnemyEnv&   _env)
{
    return _env.ctx.cameraView.min.y + _config.hoverDy;
}

[[nodiscard]] float FollowTargetX_(
    const HeliConfig& _config,
    const EnemyEnv&   _env)
{
    return _env.ctx.player.feet.x + _config.followDx;
}

[[nodiscard]] int BurstPeriod_(
    const HeliConfig& _config)
{
    return Max(1, _config.burstCount * _config.bombIntervalTicks + _config.burstRestTicks);
}

// _first 틱부터 연속 투하를 시작해 _bursts 번 하고 마지막 휴지까지 끝나는 틱
[[nodiscard]] int BurstPhaseEnd_(
    const HeliConfig& _config,
    const int         _first,
    const int         _bursts)
{
    return _first + _bursts * BurstPeriod_(_config);
}

[[nodiscard]] bool IsBombTick_(
    const HeliConfig& _config,
    const int         _ticks,
    const int         _first)
{
    const int t = _ticks - _first;
    if (t < 0)
    {
        return false;
    }
    const int k = t % BurstPeriod_(_config);
    return k < _config.burstCount * _config.bombIntervalTicks && k % _config.bombIntervalTicks == 0;
}

// 지금 몸통이 보여 주는 Pitch 프레임

}   // namespace

void HeliEnemy::Init(
    const EnemyDef& _def,
    EnemyEnv&       _env)
{
    EnemyState& _e = GetMutableState();
    PrepareBody(_e, _def);
    _e.body.bUseGravity = false;
    SetPodTicks_(-1);
    if (_def.heli.startLow != 0)
    {
        BeginLow_(_def, _env);
        return;
    }
    Go_(_def, eHeliState::Bomb);
}

void HeliEnemy::Tick(
    const EnemyDef& _def,
    EnemyEnv&       _env)
{
    EnemyState&       _e     = GetMutableState();
    const HeliConfig& config = _def.heli;
    _e.body.velocity         = { 0.f, 0.f };
    if (!_e.bActive)
    {
        return;
    }
    if (m_flight.podTicks >= 0)
    {
        ++m_flight.podTicks;
    }
    ++m_flight.idleTicks;

    switch (static_cast<eHeliState>(_e.state))
    {
        case eHeliState::Bomb:
            StepFollow_(config, _env);
            StepBombing_(_def, _env, FirstBombTicks_(config), config.highBursts);
            if (_e.stateTicks >= BurstPhaseEnd_(config, FirstBombTicks_(config), config.highBursts))
            {
                BeginLow_(_def, _env);
            }
            else if (WantsTurn_(config, _env))
            {
                m_turnWaitTicks   = 0;
                m_flight.bombTime = _e.stateTicks;
                m_flight.turnFlip = 0;
                Go_(_def, eHeliState::Turn);
            }
            break;

        case eHeliState::Low:
            StepLow_(config, _env);
            StepBombing_(_def, _env, config.reBombTicks, config.lowBursts);
            if (_e.stateTicks >= BurstPhaseEnd_(config, config.reBombTicks, config.lowBursts))
            {
                SetPodTicks_(0);
                Go_(_def, eHeliState::PodOut);
            }
            break;

        case eHeliState::Turn:
        {
            StepFollow_(config, _env);
            const bool bFlipped = m_flight.turnFlip != 0;
            if (!bFlipped && ClipFinished(_e, _def))
            {
                _e.facing         = -_e.facing;
                m_flight.turnFlip = 1;
                SetClip(_e, config.clipTurnOut, true);
            }
            else if (bFlipped && ClipFinished(_e, _def))
            {
                Go_(_def, eHeliState::Bomb);
                _e.stateTicks = m_flight.bombTime;
            }
            break;
        }

        case eHeliState::PodOut:
            if (_e.stateTicks >= config.podOutTicks)
            {
                const Aabb& view    = _env.ctx.cameraView;
                const float targetX = _e.facing < 0 ? view.max.x - config.sideMargin : view.min.x + config.sideMargin;
                const float targetY = Clamp(_env.ctx.player.center.y + config.targetDyPlayer, view.min.y + config.targetMinY, view.min.y + config.targetMaxY);
                BeginMove_(_def, _env, eHeliState::Move, { targetX, targetY });
            }
            break;

        case eHeliState::Move:
            if (StepMove_(config))
            {
                Go_(_def, eHeliState::Volley);
                RequestFx(_env, FxDef { _def.GetClip(config.clipPuff).name, config.puffOffset, 0.f, config.partPuff.depth }, _e.body.position, _e.facing);
            }
            break;

        case eHeliState::Volley:
        {
            const int t = _e.stateTicks;
            if (t > 0 && t % config.missileInterval == 0 && t / config.missileInterval < config.missiles)
            {
                RequestFx(_env, FxDef { _def.GetClip(config.clipPuff).name, config.puffOffset, 0.f, config.partPuff.depth }, _e.body.position, _e.facing);
            }
            const int sinceLaunch = t - config.puffToMissile;
            if (sinceLaunch >= 0 && sinceLaunch % config.missileInterval == 0 && sinceLaunch / config.missileInterval < config.missiles)
            {
                (void)FireWeapon(_e, _def, _env, config.weaponMissile);
            }
            if (t >= (config.missiles - 1) * config.missileInterval + config.volleyEndTicks)
            {
                SetPodTicks_(-1);
                BeginMove_(_def, _env, eHeliState::MoveBack, { Clamp(FollowTargetX_(config, _env), _env.ctx.cameraView.min.x + 60.f, _env.ctx.cameraView.max.x - 60.f), HoverY_(config, _env) });
            }
            break;
        }

        case eHeliState::MoveBack:
            if (StepMove_(config))
            {
                ++m_flight.cycle;
                Go_(_def, eHeliState::Bomb);
            }
            break;
    }
}

void HeliEnemy::OnDeath(
    const EnemyDef& _def,
    EnemyEnv&       _env)
{
    EnemyState& _e = GetMutableState();
    BeginVehicleDeath(_e, _def, _env, -1);
    _e.bVisible = false;
    SetPodTicks_(-1);
}

void HeliEnemy::TickDying(
    const EnemyDef& _def,
    EnemyEnv&       _env)
{
    TickVehicleDeath(GetMutableState(), _def, _env);
}

void HeliEnemy::SubmitParts(
    const EnemyDef&       _def,
    const EnemyRenderEnv& _render) const
{
    const EnemyState& _e     = GetState();
    const HeliConfig& config = _def.heli;
    const float       sign   = Sign_(_e.facing);
    const bool        bTurn  = static_cast<eHeliState>(_e.state) == eHeliState::Turn;
    if (bTurn)
    {
        return;
    }

    const int feetKey = static_cast<int>(_render.feet.y);
    if (m_flight.podTicks >= 0)
    {
        const VECTOR2 at = { _render.feet.x + config.podOffset.x * sign, _render.feet.y + config.podOffset.y };
        (void)SubmitNamedPart(_render, ResolveSide(_def.GetClip(config.clipPod).name, _e.facing), at, m_flight.podTicks, false, feetKey + config.partPod.sort, config.partPod.depth);
    }
    const VECTOR2 rotor     = { _render.feet.x + config.rotorOffset.x * sign, _render.feet.y + config.rotorOffset.y };
    const int     shown     = ShownPitchFrame_(_def, _render.anims);
    const String  rotorName = config.rotorRowPrefix.empty() || shown < 0 ? ResolveSide(_def.GetClip(config.clipRotor).name, _e.facing) : config.rotorRowPrefix + std::to_string(shown);
    (void)SubmitNamedPart(_render, rotorName, rotor, _e.clipTicks, true, feetKey + config.partRotor.sort, config.partRotor.depth);
}

const AnimFrame* HeliEnemy::BodyFrame(
    const EnemyDef&         _def,
    const AnimationLibrary& _anims) const
{
    const EnemyState& _e     = GetState();
    const HeliConfig& config = _def.heli;
    const int         shown  = ShownPitchFrame_(_def, _anims);
    if (_e.clip != config.clipFly || shown < 0)
    {
        return nullptr;
    }
    const AnimClip* pClip = _anims.FindClip(ResolveSide(_def.GetClip(config.clipPitch).name, _e.facing));
    if (!pClip || pClip->frames.empty())
    {
        return nullptr;
    }
    return &pClip->frames[static_cast<size_t>(Min(shown, pClip->GetFrameCount() - 1))];
}

void ParseHeliConfig(
    DefReader& _r,
    EnemyDef&  _def)
{
    HeliConfig& config = _def.heli;

    config.enterSpeedY       = _r.Num("enterSpeedY", config.enterSpeedY);
    config.hoverDy           = _r.Num("hoverDy", config.hoverDy);
    config.followDx          = _r.Num("followDx", config.followDx);
    config.followSpeed       = _r.Num("followSpeed", config.followSpeed);
    config.followDeadzone    = _r.Num("followDeadzone", config.followDeadzone);
    config.firstBombTicks    = _r.Ticks("firstBombSec", 0.9f);
    config.bombIntervalTicks = Max(1, _r.Ticks("bombIntervalSec", 0.4f));
    config.reBombTicks       = _r.Ticks("reBombSec", 0.4f);
    config.burstCount        = Max(1, _r.Int("burstCount", config.burstCount));
    config.burstRestTicks    = Max(0, _r.Ticks("burstRestSec", 1.4f));
    config.highBursts        = Max(1, _r.Int("highBursts", config.highBursts));
    config.lowBursts         = Max(1, _r.Int("lowBursts", config.lowBursts));
    config.lowDy             = _r.Num("lowDy", config.lowDy);
    config.lowSpeed          = _r.Num("lowSpeed", config.lowSpeed);
    config.lowMargin         = _r.Num("lowMargin", config.lowMargin);
    config.descendSpeedY     = _r.Num("descendSpeedY", config.descendSpeedY);
    config.startLow          = _r.Int("startLow", config.startLow);
    config.bombMuzzle        = _r.ReadVec2("bombMuzzle", config.bombMuzzle);
    config.bombMinY          = _r.Num("bombMinY", config.bombMinY);

    config.podOutTicks    = _r.Ticks("podOutSec", 0.2f);
    config.moveTicks      = Max(1, _r.Ticks("moveSec", 1.17f));
    config.sideMargin     = _r.Num("sideMargin", config.sideMargin);
    config.targetDyPlayer = _r.Num("targetDyPlayer", config.targetDyPlayer);
    config.targetMinY     = _r.Num("targetMinY", config.targetMinY);
    config.targetMaxY     = _r.Num("targetMaxY", config.targetMaxY);
    (void)_r.NumArray("xCurve", config.xCurve);
    (void)_r.NumArray("yCurve", config.yCurve);
    config.missiles        = _r.Int("missiles", config.missiles);
    config.puffToMissile   = _r.Ticks("puffToMissileSec", 0.167f);
    config.missileInterval = Max(1, _r.Ticks("missileIntervalSec", 2.1f));
    config.volleyEndTicks  = _r.Ticks("volleyEndSec", 1.5f);
    config.puffOffset      = _r.ReadVec2("puffOffset", config.puffOffset);
    config.turnWaitTicks   = _r.Ticks("turnWaitSec", 0.4f);
    config.turnDx          = _r.Num("turnDx", config.turnDx);
    config.podOffset       = _r.ReadVec2("podOffset", config.podOffset);
    config.rotorOffset     = _r.ReadVec2("rotorOffset", config.rotorOffset);
    config.idlePitchFrom   = Max(0, _r.Int("idlePitchFrom", config.idlePitchFrom));
    config.idlePitchTo     = Max(0, _r.Int("idlePitchTo", config.idlePitchTo));
    config.idleStepTicks   = Max(1, _r.Ticks("idleStepSec", 0.2f));
    config.rotorRowPrefix  = _r.Str("rotorRowPrefix");

    config.clipFly       = _r.Clip("fly");
    config.clipPitch     = _r.Clip("pitch");
    config.clipTurnIn    = _r.Clip("turnIn");
    config.clipTurnOut   = _r.Clip("turnOut");
    config.clipPod       = _r.Clip("pod");
    config.clipRotor     = _r.Clip("rotor");
    config.clipPuff      = _r.Clip("puff");
    config.weaponMissile = _r.Weapon("missile");
    config.bombDef       = _r.Str("bombDef");
}

void HeliEnemy::SetPodTicks_(
    const int _ticks)
{
    m_flight.podTicks = _ticks;
}

void HeliEnemy::Go_(
    const EnemyDef&  _def,
    const eHeliState _state)
{
    EnemyState&       _e     = GetMutableState();
    const HeliConfig& config = _def.heli;
    EnterState(_e, static_cast<int>(_state));
    switch (_state)
    {
        case eHeliState::Turn: SetClip(_e, config.clipTurnIn, true); break;
        case eHeliState::Low:
        {
            const bool bForward = (m_flight.passDirection < 0) == (_e.facing < 0);
            SetClip(_e, bForward ? config.clipPitch : config.clipFly, true);
            break;
        }
        default: SetClip(_e, config.clipFly, true); break;
    }
}

void HeliEnemy::BeginMove_(
    const EnemyDef&  _def,
    const EnemyEnv&  _env,
    const eHeliState _state,
    const VECTOR2    _target)
{
    EnemyState& _e = GetMutableState();
    Go_(_def, _state);
    m_flight.startX  = Pack_(_e.body.position.x);
    m_flight.startY  = Pack_(_e.body.position.y);
    m_flight.targetX = Pack_(_target.x);
    m_flight.targetY = Pack_(_target.y);
    (void)_env;
}

[[nodiscard]] bool HeliEnemy::StepMove_(
    const HeliConfig& _config)
{
    EnemyState&   _e     = GetMutableState();
    const float   u      = static_cast<float>(_e.stateTicks) / static_cast<float>(Max(1, _config.moveTicks));
    const VECTOR2 start  = { Unpack_(m_flight.startX), Unpack_(m_flight.startY) };
    const VECTOR2 target = { Unpack_(m_flight.targetX), Unpack_(m_flight.targetY) };
    _e.body.position     = { start.x + (target.x - start.x) * SampleCurve_(_config.xCurve, u), start.y + (target.y - start.y) * SampleCurve_(_config.yCurve, u) };
    return _e.stateTicks >= _config.moveTicks;
}

void HeliEnemy::StepFollow_(
    const HeliConfig& _config,
    const EnemyEnv&   _env)
{
    EnemyState& _e     = GetMutableState();
    const float hoverY = HoverY_(_config, _env);
    if (_e.body.position.y < hoverY)
    {
        _e.body.position.y = Min(_e.body.position.y + _config.enterSpeedY * kStepSec, hoverY);
    }
    const float dx = FollowTargetX_(_config, _env) - _e.body.position.x;
    if (Abs(dx) > _config.followDeadzone)
    {
        const float step = Min(_config.followSpeed * kStepSec, Abs(dx));
        _e.body.position.x += (dx < 0.f ? -step : step);
    }
}

void HeliEnemy::DropBomb_(
    const EnemyDef& _def,
    EnemyEnv&       _env)
{
    EnemyState&       _e        = GetMutableState();
    const HeliConfig& config    = _def.heli;
    const int         bombIndex = _env.lib.FindIndex(config.bombDef);
    if (bombIndex < 0)
    {
        return;
    }
    SpawnParams params;
    params.facing     = _e.facing;
    params.bActivated = true;
    const VECTOR2 at  = { _e.body.position.x + config.bombMuzzle.x * Sign_(_e.facing), _e.body.position.y + config.bombMuzzle.y };
    (void)_env.world.SpawnDeferred(bombIndex, at, params);
}

[[nodiscard]] int HeliEnemy::FirstBombTicks_(
    const HeliConfig& _config) const
{
    return m_flight.cycle == 0 ? _config.firstBombTicks : _config.reBombTicks;
}

void HeliEnemy::StepBombing_(
    const EnemyDef& _def,
    EnemyEnv&       _env,
    const int       _first,
    const int       _bursts)
{
    EnemyState&       _e     = GetMutableState();
    const HeliConfig& config = _def.heli;
    if (!_env.ctx.player.bAlive || _e.stateTicks >= BurstPhaseEnd_(config, _first, _bursts) || !IsBombTick_(config, _e.stateTicks, _first))
    {
        return;
    }
    if (_e.body.position.y < _env.ctx.cameraView.min.y + config.bombMinY)
    {
        return;
    }
    DropBomb_(_def, _env);
}

[[nodiscard]] float HeliEnemy::LowTargetX_(
    const HeliConfig& _config,
    const EnemyEnv&   _env) const
{
    const Aabb& view = _env.ctx.cameraView;
    return m_flight.passDirection > 0 ? view.max.x - _config.lowMargin : view.min.x + _config.lowMargin;
}

void HeliEnemy::StepLow_(
    const HeliConfig& _config,
    const EnemyEnv&   _env)
{
    EnemyState& _e   = GetMutableState();
    const float lowY = _env.ctx.cameraView.min.y + _config.lowDy;
    if (_e.body.position.y < lowY)
    {
        _e.body.position.y = Min(_e.body.position.y + _config.descendSpeedY * kStepSec, lowY);
    }
    const float dx   = LowTargetX_(_config, _env) - _e.body.position.x;
    const float step = Min(_config.lowSpeed * kStepSec, Abs(dx));
    _e.body.position.x += (dx < 0.f ? -step : step);
}

void HeliEnemy::BeginLow_(
    const EnemyDef& _def,
    const EnemyEnv& _env)
{
    EnemyState& _e         = GetMutableState();
    const Aabb& view       = _env.ctx.cameraView;
    m_flight.passDirection = _e.body.position.x < (view.min.x + view.max.x) * 0.5f ? 1 : -1;
    Go_(_def, eHeliState::Low);
}

[[nodiscard]] bool HeliEnemy::WantsTurn_(
    const HeliConfig& _config,
    const EnemyEnv&   _env)
{
    EnemyState& _e     = GetMutableState();
    const float behind = -DxToPlayer(_e, _env) * Sign_(_e.facing);
    if (behind > _config.turnDx)
    {
        ++m_turnWaitTicks;
    }
    else
    {
        m_turnWaitTicks = 0;
    }
    return m_turnWaitTicks >= _config.turnWaitTicks;
}

[[nodiscard]] int HeliEnemy::ShownPitchFrame_(
    const EnemyDef&         _def,
    const AnimationLibrary& _anims) const
{
    const EnemyState& _e     = GetState();
    const HeliConfig& config = _def.heli;
    if (_e.clip == config.clipFly && config.idlePitchTo > config.idlePitchFrom)
    {
        const int span = config.idlePitchTo - config.idlePitchFrom;
        const int m    = (m_flight.idleTicks / Max(1, config.idleStepTicks)) % (span * 2);
        return config.idlePitchFrom + (m <= span ? m : span * 2 - m);
    }
    if (_e.clip == config.clipPitch)
    {
        const AnimClip* pClip = _anims.FindClip(ResolveSide(_def.GetClip(config.clipPitch).name, _e.facing));
        if (pClip && !pClip->frames.empty() && pClip->GetFrameDurationUnits() > 0)
        {
            const int index = static_cast<int>(static_cast<int64_t>(_e.clipTicks) * kAnimUnitsPerTick / pClip->GetFrameDurationUnits());
            return pClip->bLoop ? index % pClip->GetFrameCount() : Min(index, pClip->GetFrameCount() - 1);
        }
    }
    return -1;
}

void HeliEnemy::HashActionState(Fnv1a& _hash) const
{
    HashActionState_(_hash, 0, 0, 0, m_turnWaitTicks);
}
