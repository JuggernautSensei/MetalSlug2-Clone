#include <cmath>
#include <string>

#include "Base/Math.h"

#include "Game/Enemies/EnemyCommon.h"
#include "Game/Enemies/EnemyDefParse.h"
#include "Game/Enemies/VehicleCommon.h"
#include "World/Constants.h"

namespace slug
{

// ===========================================
//  Aparch — 공격 헬기 (Docs/EnemySpecs.md §2.6)
//   원점 = 기체 중심 (지면 개념 없음, 중력 없음). 항상 좌향.
//   활성 후 화면 (approachX, approachY) 로 접근 → 2 s 대기 → 패턴을 번갈아 한다.
//     미사일: 포 전개 1 s → 자세 이동 1 s → 미사일 3발 (1 s 간격) → 수납/복귀 1 s
//     폭탄:   플레이어 위를 따라다니며 6 s 동안 폭탄을 투하 (0.5 s 틱: 드롭 시각 0.5/1.0/1.5, 2.5/3.0/3.5, 4.5/5.0/5.5 s)
//   사망: 표류하며 1.5 s 동안 연쇄 폭발, 끝에 대형 폭발과 금속 파편 후 제거.
// ===========================================

namespace
{
    enum eAparchState : int
    {
        kApproach,   // 접근 (목표 화면 지점)
        kStop,       // 정지 클립 후 대기
        kIdle,
        kMissGun,    // 포 전개 (포는 별도 오버레이라 몸체 클립을 그대로 쓴다: 오버레이 슬롯은 로터 하나뿐)
        kMissMove,   // 미사일 자세로 이동
        kMissShoot,  // 미사일 3발 (counter = 발사 수)
        kMissEnd,    // 포 수납 + 복귀
        kBomb        // 폭탄 패턴 (counter = 0.5 s 틱 리듬 카운터)
    };

    // e.aux: 패턴 카운터 (홀/짝으로 미사일/폭탄 교대). e.aux2: 시작 패리티 선택 여부.

    [[nodiscard]] int ClipOf_(const Enemy& _e, const AparchCfg& _c)
    {
        switch (_e.state)
        {
            case kApproach: return _c.clipMove;
            case kStop: return _c.clipMoveStop;
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

    // 목표 쪽으로 남은 거리의 일부만큼 이동 (duration 틱 안에 도달하는 부드러운 이동)
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
                // 접근 속도: 기본 speed, 가까워지면 감속 (v = max(minSpeed, v - decel*dt))
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
                    Go_(_e, _def, bBomb ? kBomb : kMissGun);
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
                // 발사 시각: 0, 1 s, 2 s (직전 발로부터 1 s 간격). 마지막 발 뒤 1 s 에 수납으로 넘어간다
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
                // 플레이어 위 followOffsetY 를 따라다닌다
                const float dx = DxToPlayer(_e, _env);
                const float dy = DyToPlayer(_e, _env) - c.followOffsetY;
                _e.body.velocity = { dx * c.followGain, dy * c.followGain };
                _e.body.position = { _e.body.position.x + _e.body.velocity.x * kStepSec, _e.body.position.y + _e.body.velocity.y * kStepSec };

                // 0.5 s 틱: counter 를 올리고, 홀수(mod 7 기준)면 투하 후 한 번 더 올린다 (레거시 t%7&1 리듬)
                if (_e.stateTicks > 0 && _e.stateTicks % c.bombStepTicks == 0)
                {
                    ++_e.counter;
                    if (((_e.counter % 7) & 1) != 0)
                    {
                        (void)FireWeapon(_e, _def, _env, c.weaponDrop);
                        ++_e.counter;
                    }
                }
                if (_e.stateTicks >= c.bombSteps * c.bombStepTicks)
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
        _e.body.velocity   = { c.driftX, c.driftY };   // 표류
    }

    void TickDying_(Enemy& _e, const EnemyDef& _def, EnemyEnv& _env)
    {
        const AparchCfg& c = _def.aparch;
        _e.body.position   = { _e.body.position.x + _e.body.velocity.x * kStepSec, _e.body.position.y + _e.body.velocity.y * kStepSec };

        // 연쇄 폭발: 0, 0.3 s, ... 마다 S+M (중심 (-30,-20), 반경 ±50 균등)
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
}   // namespace

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
    c.followOffsetY        = _r.Num("followOffsetY");
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

    c.weaponMissile = _r.Weapon("missile");
    c.weaponDrop    = _r.Weapon("drop");
}

}   // namespace slug
