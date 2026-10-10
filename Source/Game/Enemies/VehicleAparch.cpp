#include <cmath>
#include <string>

#include "Base/Math.h"

#include "Game/Enemies/EnemyCommon.h"
#include "Game/Enemies/EnemyDefParse.h"
#include "Game/Enemies/VehicleCommon.h"
#include "World/Constants.h"

namespace slug
{
namespace
{
    enum eAparchState : int
    {
        kApproach,
        kStop,
        kIdle,
        kMissGun,
        kMissMove,
        kMissShoot,
        kMissEnd,
        kBomb,
        kTurn
    };

    [[nodiscard]] int ClipOf_(const Enemy& _e, const AparchCfg& _c)
    {
        switch (_e.state)
        {
            case kApproach: return _c.clipMove;
            case kStop: return _c.clipMoveStop;
            case kMissGun: return _c.clipGun;
            case kMissShoot: return _c.clipGunHold;
            case kMissEnd: return _c.clipGunEnd;
            case kTurn: return _c.clipTurn;
            default: return _c.clipIdle;
        }
    }

    void Go_(Enemy& _e, const EnemyDef& _def, const int _state)
    {
        EnterState(_e, _state);
        SetClip(_e, ClipOf_(_e, _def.aparch), true);
    }

    [[nodiscard]] bool Done_(const Enemy& _e, const EnemyDef& _def)
    {
        return _e.clip >= 0 && _e.stateTicks >= _def.GetClip(_e.clip).GetDurationTicks();
    }

    void GlideTo_(Enemy& _e, const Vec2 _target, const int _remainingTicks)
    {
        if (_remainingTicks <= 1)
        {
            _e.body.position = _target;
            return;
        }
        const float inv = 1.f / static_cast<float>(_remainingTicks);
        _e.body.position = { _e.body.position.x + (_target.x - _e.body.position.x) * inv, _e.body.position.y + (_target.y - _e.body.position.y) * inv };
    }

    void Init_(Enemy& _e, const EnemyDef& _def, EnemyEnv& _env)
    {
        (void)_env;
        PrepareBody(_e, _def);
        _e.facing = -1;   // 항상 좌향
        _e.aux4   = _e.facing;   // 마지막으로 본 향 (향이 바뀌면 kIdle 에서 Turning 연출)
        Go_(_e, _def, kApproach);
    }

    void Tick_(Enemy& _e, const EnemyDef& _def, EnemyEnv& _env)
    {
        const AparchCfg& c = _def.aparch;
        if (!_e.bActive)
        {
            return;
        }

        switch (_e.state)
        {
            case kApproach:
            {
                const Vec2 target = ScreenToWorld(_env, { c.approachX, c.approachY });
                const Vec2 d      = { target.x - _e.body.position.x, target.y - _e.body.position.y };
                const float        dist2  = d.x * d.x + d.y * d.y;
                if (dist2 <= c.arriveSq)
                {
                    _e.body.position   = target;
                    _e.body.velocity   = { 0.f, 0.f };
                    Go_(_e, _def, kStop);
                    break;
                }
                const float curSpeed = std::sqrt(_e.body.velocity.x * _e.body.velocity.x + _e.body.velocity.y * _e.body.velocity.y);
                const float speed    = dist2 <= c.decelSq ? slug::Max(c.minSpeed, curSpeed - c.decel * kStepSec) : c.speed;
                const float len      = std::sqrt(dist2);
                _e.body.velocity     = { d.x / len * speed, d.y / len * speed };
                _e.body.position     = { _e.body.position.x + _e.body.velocity.x * kStepSec, _e.body.position.y + _e.body.velocity.y * kStepSec };
                break;
            }

            case kStop:
                _e.body.velocity = { 0.f, 0.f };
                if (Done_(_e, _def))
                {
                    Go_(_e, _def, kIdle);
                }
                break;

            case kIdle:
                _e.body.velocity = { 0.f, 0.f };
                if (_e.aux4 != _e.facing)
                {
                    _e.aux4 = _e.facing;
                    Go_(_e, _def, kTurn);
                    break;
                }
                if (_e.stateTicks >= c.idleTicks)
                {
                    if (_e.aux2 == 0)
                    {
                        _e.aux2 = 1;
                        _e.aux  = _env.world.Roll(2);   // 시작 패리티는 무작위
                    }
                    const bool bBomb = (_e.aux & 1) == 1;
                    ++_e.aux;
                    _e.counter = 0;
                    if (bBomb)
                    {
                        _e.aux3 = c.idleTicks;   // 레거시: 드롭 진입 시 누적 타이머는 PATTERN_TIME(2 s) 그대로 이월된다
                    }
                    Go_(_e, _def, bBomb ? kBomb : kMissGun);
                }
                break;

            case kTurn:
                _e.body.velocity = { 0.f, 0.f };
                if (Done_(_e, _def))
                {
                    Go_(_e, _def, kIdle);
                }
                break;

            case kMissGun:
                _e.body.velocity = { 0.f, 0.f };
                if (_e.stateTicks >= c.gunTicks)
                {
                    Go_(_e, _def, kMissMove);
                }
                break;

            case kMissMove:
                _e.body.velocity = { 0.f, 0.f };
                GlideTo_(_e, ScreenToWorld(_env, { c.missileX, c.missileY }), c.moveTicks - _e.stateTicks);
                if (_e.stateTicks >= c.moveTicks)
                {
                    _e.counter = 0;
                    Go_(_e, _def, kMissShoot);
                }
                break;

            case kMissShoot:
                _e.body.velocity = { 0.f, 0.f };
                if (_e.counter < c.missileShots && _e.stateTicks >= _e.counter * c.missileIntervalTicks)
                {
                    (void)FireWeapon(_e, _def, _env, c.weaponMissile);
                    ++_e.counter;
                }
                if (_e.stateTicks >= c.missileShots * c.missileIntervalTicks)
                {
                    Go_(_e, _def, kMissEnd);
                }
                break;

            case kMissEnd:
                _e.body.velocity = { 0.f, 0.f };
                GlideTo_(_e, ScreenToWorld(_env, { c.homeX, c.homeY }), c.endTicks - _e.stateTicks);
                if (_e.stateTicks >= c.endTicks)
                {
                    Go_(_e, _def, kIdle);
                }
                break;

            case kBomb:
            {
                const float dx = DxToPlayer(_e, _env);
                // 호버 높이는 접근 때 정해진 y 를 그대로 유지한다. x 만 플레이어를 따라간다 (y 추적 기믹 없음)
                _e.body.velocity = { dx * c.followGain, 0.f };
                _e.body.position = { _e.body.position.x + _e.body.velocity.x * kStepSec, _e.body.position.y + _e.body.velocity.y * kStepSec };

                // 레거시 DROP_MISSLE 과 같은 순서: 타이머 +1 틱, 0.5 s 마다 카운터 +1 (틱당 최대 1회), 홀수(mod 7) 카운터면 투하 후 +1
                ++_e.aux3;
                if (_e.aux3 >= c.bombStepTicks)
                {
                    _e.aux3 -= c.bombStepTicks;
                    ++_e.counter;
                }
                if (((_e.counter % 7) & 1) != 0)
                {
                    (void)FireWeapon(_e, _def, _env, c.weaponDrop);
                    ++_e.counter;
                }
                if (_e.counter > c.bombSteps)
                {
                    Go_(_e, _def, kIdle);
                }
                break;
            }

            default: break;
        }
    }

    void OnDeath_(Enemy& _e, const EnemyDef& _def, EnemyEnv& _env)
    {
        (void)_env;
        const AparchCfg& c = _def.aparch;
        SetClip(_e, c.clipDestroy, true);
        _e.deathTicks      = 0;
        _e.body.velocity   = { c.driftX, c.driftY };
    }

    void TickDying_(Enemy& _e, const EnemyDef& _def, EnemyEnv& _env)
    {
        const AparchCfg& c = _def.aparch;
        _e.body.position   = { _e.body.position.x + _e.body.velocity.x * kStepSec, _e.body.position.y + _e.body.velocity.y * kStepSec };

        const int t = _e.deathTicks;
        if (t % c.chainIntervalTicks == 0 && t / c.chainIntervalTicks < c.chainCount)
        {
            const Vec2 base = { _e.body.position.x - 30.f, _e.body.position.y - 20.f };
            const auto         jitter = [&]() { return static_cast<float>(_env.world.Roll(100)) - 50.f; };
            RequestFxAt(_env, "Explosion_Small", { base.x + jitter(), base.y + jitter() }, _e.facing);
            RequestFxAt(_env, "Explosion_Mid", { base.x + jitter(), base.y + jitter() }, _e.facing);
        }

        if (t >= c.removeTicks)
        {
            RequestFxAt(_env, "Explosion_Large", _e.body.position, _e.facing);
            for (int i = 0; i < 5; ++i)
            {
                const Vec2 at = { _e.body.position.x + static_cast<float>(_env.world.Roll(120)) - 60.f, _e.body.position.y + static_cast<float>(_env.world.Roll(80)) - 80.f };
                RequestFxAt(_env, "Metal" + std::to_string(_env.world.Roll(12) + 1), at, _e.facing);
            }
            _e.life = eEnemyLife::Gone;
            return;
        }
        ++_e.deathTicks;
    }
}

const BehaviorOps kAparchOps = {
    .init       = &Init_,
    .tick       = &Tick_,
    .onDeath    = &OnDeath_,
    .tickDying  = &TickDying_,
};

void ParseAparchCfg(
    DefReader& _r,
    EnemyDef&  _def)
{
    AparchCfg& c = _def.aparch;

    const float decelDist  = _r.Num("decelDist");
    const float arriveDist = _r.Num("arriveDist");
    c.speed        = _r.Num("speed");
    c.decelSq      = decelDist * decelDist;
    c.arriveSq     = arriveDist * arriveDist;
    c.decel        = _r.Num("decel");
    c.minSpeed     = _r.Num("minSpeed");
    {
        const Vec2 approach = _r.ReadVec2("approach", { 445.f, 125.f });
        c.approachX = approach.x;
        c.approachY = approach.y;
        const Vec2 missile  = _r.ReadVec2("missilePose", { 540.f, 248.f });
        c.missileX  = missile.x;
        c.missileY  = missile.y;
        const Vec2 home     = _r.ReadVec2("home", { 540.f, 100.f });
        c.homeX     = home.x;
        c.homeY     = home.y;
    }
    c.idleTicks            = _r.Ticks("idleSec");
    c.gunTicks             = _r.Ticks("gunSec");
    c.moveTicks            = _r.Ticks("moveSec");
    c.missileIntervalTicks = _r.Ticks("missileIntervalSec");
    c.missileShots         = _r.Int("missileShots");
    c.endTicks             = _r.Ticks("endSec");
    c.followGain           = _r.Num("followGain");
    c.bombStepTicks        = _r.Ticks("bombStepSec");
    c.bombSteps            = _r.Int("bombSteps");
    {
        const Vec2 drift = _r.ReadVec2("drift", { 50.f, -70.f });
        c.driftX = drift.x;
        c.driftY = drift.y;
    }
    c.chainCount         = _r.Int("chainCount");
    c.chainIntervalTicks = _r.Ticks("chainIntervalSec");
    c.removeTicks        = _r.Ticks("removeSec");

    c.clipIdle     = _r.Clip("idle");
    c.clipMove     = _r.Clip("move");
    c.clipMoveStop = _r.Clip("moveStop");
    c.clipDestroy  = _r.Clip("destroy");
    c.clipGun      = _r.Clip("gun");
    c.clipGunHold  = _r.Clip("gunHold");
    c.clipGunEnd   = _r.Clip("gunEnd");
    c.clipTurn     = _r.Clip("turn");

    c.weaponMissile = _r.Weapon("missile");
    c.weaponDrop    = _r.Weapon("drop");
}
}
