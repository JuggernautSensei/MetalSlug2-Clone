#include <algorithm>
#include <cmath>

#include "Game/Enemies/EnemyCommon.h"
#include "Game/Enemies/EnemyDefParse.h"
#include "Game/Enemies/VehicleCommon.h"

namespace slug
{

// ===========================================
//  BTank — 고정 포탑 전차 (Docs/EnemySpecs.md §2.3)
//   이동하지 않고 항상 좌향. 플레이어가 전차 x - activateDx 를 넘으면 문을 연다 (정의의 활성 margin 은 음수: 행동이 직접 켠다).
//   열린 뒤 3 s 마다 포탄 2연발. 포탄은 포물선(v0 + 중력)이다.
// ===========================================

namespace
{
    enum eBTankState : int
    {
        kClosed,   // 닫힘 (활성 전)
        kOpen,     // 문 열림 애니메이션
        kIdle,     // 열린 채 대기 (마지막 프레임 유지)
        kFire      // 발사 (counter 발 남음)
    };

    [[nodiscard]] int ClipOf_(const Enemy& _e, const BTankCfg& _c)
    {
        switch (_e.state)
        {
            case kOpen:
            case kIdle: return _c.clipOpen;
            case kFire: return _c.clipFire;
            default: return _c.clipIdle;
        }
    }

    void Go_(Enemy& _e, const EnemyDef& _def, const int _state)
    {
        const BTankCfg& c = _def.btank;
        EnterState(_e, _state);
        const int clip = ClipOf_(_e, c);
        SetClip(_e, clip, _state != kIdle);   // 열림 → 대기는 이어서 (같은 클립), 나머지는 처음부터
        if (_state == kIdle && clip >= 0)
        {
            // 열린 채 멈춘 상태 = 열림 클립의 마지막 프레임 (재생을 끝까지 감아 둔다)
            _e.clipTicks = std::max(_e.clipTicks, _def.GetClip(clip).GetDurationTicks());
        }
    }

    void Init_(Enemy& _e, const EnemyDef& _def, EnemyEnv& _env)
    {
        (void)_env;
        PrepareBody(_e, _def);
        _e.facing = -1;   // 항상 좌향
        Go_(_e, _def, kClosed);
    }

    void Tick_(Enemy& _e, const EnemyDef& _def, EnemyEnv& _env)
    {
        const BTankCfg& c = _def.btank;
        _e.body.velocity.x = 0.f;

        if (!_e.bActive)
        {
            // 활성: 플레이어가 전차 x - activateDx 를 넘었다 (화면 margin 과 무관)
            if (_env.ctx.player.feet.x >= _e.body.position.x - c.activateDx)
            {
                _e.bActive = true;
            }
            else
            {
                return;
            }
        }

        switch (_e.state)
        {
            case kClosed:
                Go_(_e, _def, kOpen);
                break;

            case kOpen:
                if (_e.clip >= 0 && _e.stateTicks >= _def.GetClip(_e.clip).GetDurationTicks())
                {
                    Go_(_e, _def, kIdle);
                }
                break;

            case kIdle:
                if (_e.stateTicks >= c.idleTicks)
                {
                    _e.counter = c.shotsPerCycle;
                    Go_(_e, _def, kFire);
                }
                break;

            case kFire:
                if (ClipReachedFrame(_e, _def, c.fireFrame))
                {
                    (void)FireWeapon(_e, _def, _env, c.weaponShell);
                }
                if (_e.clip >= 0 && _e.stateTicks >= _def.GetClip(_e.clip).GetDurationTicks())
                {
                    --_e.counter;
                    if (_e.counter > 0)
                    {
                        Go_(_e, _def, kFire);   // 연발: 대기 없이 다음 발
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
        BeginVehicleDeath(_e, _def, _env, _def.btank.clipDestroy);
    }
}   // namespace

const BehaviorOps kBTankOps = {
    .init       = &Init_,
    .tick       = &Tick_,
    .onDeath    = &OnDeath_,
    .tickDying  = &TickVehicleDeath,
};

void ParseBTankCfg(
    DefReader& _r,
    EnemyDef&  _def)
{
    BTankCfg& c = _def.btank;

    c.activateDx    = _r.Num("activateDx");
    c.idleTicks     = _r.Ticks("idleSec");
    c.shotsPerCycle = _r.Int("shotsPerCycle");
    c.fireFrame     = _r.Int("fireFrame");

    c.clipIdle    = _r.Clip("idle");
    c.clipOpen    = _r.Clip("open");
    c.clipFire    = _r.Clip("fire");
    c.clipDestroy = _r.Clip("destroy");

    c.weaponShell = _r.Weapon("shell");
}

}   // namespace slug
