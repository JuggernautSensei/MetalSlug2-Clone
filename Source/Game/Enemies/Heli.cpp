#include <algorithm>
#include <cmath>

#include "Base/Math.h"
#include "Game/Enemies/EnemyCommon.h"
#include "Game/Enemies/EnemyDefParse.h"
#include "Game/Enemies/VehicleCommon.h"

namespace slug
{

namespace
{
    // 헬기 상태 (Enemy::state 에 int 로 저장). 시간/수치는 Docs/Spec_Helicopter.md
    enum class eHeliState
    {
        Bomb,       // 진입 + 폭격: 위 오른쪽에서 내려와 플레이어 위에서 폭탄 투하
        PodOut,     // 미사일 포드 전개 (제자리)
        Move,       // 바라보는 반대쪽 화면 가장자리 아래로 이동
        Volley,     // 발사 연기 → 미사일 (일제 사격)
        MoveBack,   // 폭격 위치로 복귀 (포드 접힘)
        Turn        // 방향 전환 (원작 미관측: 시트의 Turning 프레임 기반)
    };

    // Enemy::ext 슬롯 (좌표는 1/16 px 고정소수점으로 저장)
    constexpr int kExtStartX   = 0;
    constexpr int kExtStartY   = 1;
    constexpr int kExtTargetX  = 2;
    constexpr int kExtTargetY  = 3;
    constexpr int kExtBombTime = 4;   // 선회로 중단된 폭격의 경과 틱
    constexpr int kExtTurnFlip = 5;   // 선회 전반부가 끝나 방향을 뒤집었는가
    constexpr int kExtPodTicks = 6;   // 포드 전개 후 경과 틱 (-1 = 접힘)

    constexpr float kFixedScale = 16.f;
    constexpr int   kCurveSteps = 12;

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

    // 곡선 표 (균등 간격 13점) 선형 보간
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
            default: SetClip(_e, c.clipFly, true); break;
        }
    }

    // 현재 위치 → 목표로 이동 곡선을 시작한다
    void BeginMove_(
        Enemy&           _e,
        const EnemyDef&  _def,
        const EnemyEnv&  _env,
        const eHeliState _state,
        const Vec2       _target)
    {
        Go_(_e, _def, _state);
        _e.ext[kExtStartX]  = Pack_(_e.body.position.x);
        _e.ext[kExtStartY]  = Pack_(_e.body.position.y);
        _e.ext[kExtTargetX] = Pack_(_target.x);
        _e.ext[kExtTargetY] = Pack_(_target.y);
        (void)_env;
    }

    // 곡선 이동 한 틱. 끝났으면 true
    [[nodiscard]] bool StepMove_(
        Enemy&          _e,
        const HeliCfg&  _c)
    {
        const float u      = static_cast<float>(_e.stateTicks) / static_cast<float>(Max(1, _c.moveTicks));
        const Vec2  start  = { Unpack_(_e.ext[kExtStartX]), Unpack_(_e.ext[kExtStartY]) };
        const Vec2  target = { Unpack_(_e.ext[kExtTargetX]), Unpack_(_e.ext[kExtTargetY]) };
        _e.body.position   = { start.x + (target.x - start.x) * SampleCurve_(_c.xCurve, u), start.y + (target.y - start.y) * SampleCurve_(_c.yCurve, u) };
        return _e.stateTicks >= _c.moveTicks;
    }

    // 폭격 중 이동: 호버 높이까지 내려오고, 플레이어 x 를 천천히 따라간다
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
        if (std::abs(dx) > _c.followDeadzone)
        {
            const float step = Min(_c.followSpeed * kStepSec, std::abs(dx));
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
        const Vec2 at     = { _e.body.position.x + c.bombMuzzle.x * Sign_(_e.facing), _e.body.position.y + c.bombMuzzle.y };
        (void)_env.world.SpawnDeferred(bombIndex, at, params);
    }

    // 폭격: 첫 폭탄 이후 일정 간격
    void StepBombing_(
        Enemy&          _e,
        const EnemyDef& _def,
        EnemyEnv&       _env)
    {
        const HeliCfg& c = _def.heli;
        if (!_env.ctx.player.bAlive || _e.stateTicks < c.firstBombTicks || (_e.stateTicks - c.firstBombTicks) % c.bombIntervalTicks != 0)
        {
            return;
        }
        if (_e.body.position.y < _env.ctx.cameraView.min.y + c.bombMinY)
        {
            return;
        }
        DropBomb_(_e, _def, _env);
    }

    // 플레이어가 반대쪽에 머물면 선회한다
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
        (void)_env;
        PrepareBody(_e, _def);
        _e.body.bUseGravity = false;
        SetPodTicks_(_e, -1);
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

        switch (static_cast<eHeliState>(_e.state))
        {
            case eHeliState::Bomb:
                StepFollow_(_e, c, _env);
                StepBombing_(_e, _def, _env);
                if (_e.stateTicks >= c.bombRunTicks)
                {
                    SetPodTicks_(_e, 0);
                    Go_(_e, _def, eHeliState::PodOut);
                }
                else if (WantsTurn_(_e, c, _env))
                {
                    _e.aux3                = 0;
                    _e.ext[kExtBombTime]   = _e.stateTicks;
                    _e.ext[kExtTurnFlip]   = 0;
                    Go_(_e, _def, eHeliState::Turn);
                }
                break;

            case eHeliState::Turn:
            {
                StepFollow_(_e, c, _env);
                const bool bFlipped = _e.ext[kExtTurnFlip] != 0;
                if (!bFlipped && ClipFinished(_e, _def))
                {
                    _e.facing          = -_e.facing;
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
                    const Aabb& view   = _env.ctx.cameraView;
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
                    RequestFx(_env, FxDef { _def.GetClip(c.clipPuff).name, c.puffOffset, 0.f, 58 }, _e.body.position, _e.facing);
                }
                break;

            case eHeliState::Volley:
            {
                const int t = _e.stateTicks;
                if (t > 0 && t % c.missileInterval == 0 && t / c.missileInterval < c.missiles)
                {
                    RequestFx(_env, FxDef { _def.GetClip(c.clipPuff).name, c.puffOffset, 0.f, 58 }, _e.body.position, _e.facing);
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
        _e.bVisible = false;   // 원작: 추락 없이 즉시 대폭발로 사라진다
        SetPodTicks_(_e, -1);
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

        const int sortKey = static_cast<int>(_render.feet.y);
        if (_e.ext[kExtPodTicks] >= 0)
        {
            const Vec2 at = { _render.feet.x + c.podOffset.x * sign, _render.feet.y + c.podOffset.y };
            (void)SubmitNamedPart(_render, ResolveSide(_def.GetClip(c.clipPod).name, _e.facing), at, _e.ext[kExtPodTicks], false, sortKey, 16);
        }
        const Vec2 rotor = { _render.feet.x + c.rotorOffset.x * sign, _render.feet.y + c.rotorOffset.y };
        (void)SubmitNamedPart(_render, ResolveSide(_def.GetClip(c.clipRotor).name, _e.facing), rotor, _e.clipTicks, true, sortKey, 16);
    }

}   // namespace

const BehaviorOps kHeliOps = {
    .init              = &Init_,
    .tick              = &Tick_,
    .onDeath           = &OnDeath_,
    .tickDying         = &TickVehicleDeath,
    .registerHurtboxes = nullptr,
    .submitParts       = &SubmitParts_,
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
    c.bombRunTicks      = _r.Ticks("bombRunSec", 6.1f);
    c.bombMuzzle        = _r.ReadVec2("bombMuzzle", c.bombMuzzle);
    c.bombMinY          = _r.Num("bombMinY", c.bombMinY);

    c.podOutTicks     = _r.Ticks("podOutSec", 0.2f);
    c.moveTicks       = Max(1, _r.Ticks("moveSec", 1.17f));
    c.sideMargin      = _r.Num("sideMargin", c.sideMargin);
    c.targetDyPlayer  = _r.Num("targetDyPlayer", c.targetDyPlayer);
    c.targetMinY      = _r.Num("targetMinY", c.targetMinY);
    c.targetMaxY      = _r.Num("targetMaxY", c.targetMaxY);
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

    c.clipFly       = _r.Clip("fly");
    c.clipTurnIn    = _r.Clip("turnIn");
    c.clipTurnOut   = _r.Clip("turnOut");
    c.clipPod       = _r.Clip("pod");
    c.clipRotor     = _r.Clip("rotor");
    c.clipPuff      = _r.Clip("puff");
    c.weaponMissile = _r.Weapon("missile");
    c.bombDef       = _r.Str("bombDef");
}

}   // namespace slug
