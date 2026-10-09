#include <cmath>

#include "Game/Enemies/EnemyCommon.h"
#include "Game/Enemies/EnemyDefParse.h"
#include "Game/Enemies/VehicleCommon.h"

namespace slug
{

// ===========================================
//  MissleTank — 미사일 전차 (Docs/EnemySpecs.md §2.4)
//   고정 포대. 조준 모드 Down(직사, 근거리) / Up(곡사, 원거리 |dx|>farDx). 모드 전환에는 조준 클립 한 번이 든다.
//   플레이어가 rangeDx 안에 있는 동안만 Idle 타이머가 간다 (벗어나면 리셋). 발사 준비 2 s → 미사일 5발 연속.
// ===========================================

namespace
{
    enum eMissleTankState : int
    {
        kIdle,    // 대기 (1 s 누적 후 판단)
        kAim,     // 조준 모드 전환 (aux 토글)
        kReady,   // 발사 준비 (2 s)
        kShot     // 미사일 1발 (counter 발 남음)
    };

    // e.aux: 조준 모드 (0 Down 직사 / 1 Up 곡사). e.counter: 남은 발수.
    constexpr int kModeDown = 0;
    constexpr int kModeUp   = 1;

    [[nodiscard]] bool IsUp_(const Enemy& _e)
    {
        return _e.aux == kModeUp;
    }

    [[nodiscard]] int ClipOf_(const Enemy& _e, const MissleTankCfg& _c)
    {
        const bool bUp = IsUp_(_e);
        switch (_e.state)
        {
            case kAim: return _e.aux == kModeDown ? _c.clipAimUp : _c.clipAimDown;   // 직사 → 곡사 전환은 AimmingUp
            case kReady: return bUp ? _c.clipReadyUp : _c.clipReadyDown;
            case kShot: return bUp ? _c.clipShotUp : _c.clipShotDown;
            default: return bUp ? _c.clipIdleUp : _c.clipIdleDown;
        }
    }

    void Go_(Enemy& _e, const EnemyDef& _def, const int _state)
    {
        EnterState(_e, _state);
        SetClip(_e, ClipOf_(_e, _def.missleTank), true);
    }

    void Init_(Enemy& _e, const EnemyDef& _def, EnemyEnv& _env)
    {
        (void)_env;
        PrepareBody(_e, _def);
        _e.aux = kModeDown;   // 초기 조준은 직사
        Go_(_e, _def, kIdle);
    }

    void Tick_(Enemy& _e, const EnemyDef& _def, EnemyEnv& _env)
    {
        const MissleTankCfg& c = _def.missleTank;
        _e.body.velocity.x     = 0.f;
        const float dx         = std::abs(DxToPlayer(_e, _env));

        switch (_e.state)
        {
            case kIdle:
                if (dx > c.rangeDx)
                {
                    _e.stateTicks = 0;   // 사거리 밖: 타이머 리셋
                    break;
                }
                if (_e.stateTicks >= c.detectTicks)
                {
                    const bool bWantUp = dx > c.farDx;
                    if (bWantUp != IsUp_(_e))
                    {
                        Go_(_e, _def, kAim);
                    }
                    else
                    {
                        Go_(_e, _def, kReady);
                    }
                }
                break;

            case kAim:
                if (_e.clip >= 0 && _e.stateTicks >= _def.GetClip(_e.clip).GetDurationTicks())
                {
                    _e.aux = IsUp_(_e) ? kModeDown : kModeUp;   // 조준 모드 토글
                    Go_(_e, _def, kIdle);
                }
                break;

            case kReady:
                if (_e.stateTicks >= c.readyTicks)
                {
                    _e.counter = c.bullets;
                    Go_(_e, _def, kShot);
                }
                break;

            case kShot:
                if (ClipReachedFrame(_e, _def, c.fireFrame))
                {
                    // 머즐 y 는 레거시 rand[0,24) 를 그대로 더한다 (탄 높이 분산)
                    const jug::VECTOR2 jitter = { 0.f, static_cast<float>(_env.world.Roll(24)) };
                    (void)FireWeapon(_e, _def, _env, IsUp_(_e) ? c.weaponUp : c.weaponDown, { 0.f, 0.f }, jitter);
                }
                if (_e.clip >= 0 && _e.stateTicks >= _def.GetClip(_e.clip).GetDurationTicks())
                {
                    --_e.counter;
                    if (_e.counter > 0)
                    {
                        Go_(_e, _def, kShot);   // 연속 발사 (준비 없이)
                    }
                    else
                    {
                        Go_(_e, _def, kIdle);
                    }
                }
                break;

            default: break;
        }
    }

    void OnDeath_(Enemy& _e, const EnemyDef& _def, EnemyEnv& _env)
    {
        BeginVehicleDeath(_e, _def, _env, IsUp_(_e) ? _def.missleTank.clipDestroyUp : _def.missleTank.clipDestroyDown);
    }
}   // namespace

const BehaviorOps kMissleTankOps = {
    .init       = &Init_,
    .tick       = &Tick_,
    .onDeath    = &OnDeath_,
    .tickDying  = &TickVehicleDeath,
};

void ParseMissleTankCfg(
    DefReader& _r,
    EnemyDef&  _def)
{
    MissleTankCfg& c = _def.missleTank;

    c.rangeDx     = _r.Num("rangeDx");
    c.farDx       = _r.Num("farDx");
    c.detectTicks = _r.Ticks("detectSec");
    c.readyTicks  = _r.Ticks("readySec");
    c.bullets     = _r.Int("bullets");
    c.fireFrame   = _r.Int("fireFrame");

    c.clipIdleUp     = _r.Clip("idleUp");
    c.clipIdleDown   = _r.Clip("idleDown");
    c.clipAimUp      = _r.Clip("aimUp");
    c.clipAimDown    = _r.Clip("aimDown");
    c.clipReadyUp    = _r.Clip("readyUp");
    c.clipReadyDown  = _r.Clip("readyDown");
    c.clipShotUp     = _r.Clip("shotUp");
    c.clipShotDown   = _r.Clip("shotDown");
    c.clipDestroyUp  = _r.Clip("destroyUp");
    c.clipDestroyDown = _r.Clip("destroyDown");

    c.weaponUp   = _r.Weapon("up");
    c.weaponDown = _r.Weapon("down");
}

}   // namespace slug
