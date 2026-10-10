#include <algorithm>
#include <cmath>

#include "Base/Math.h"
#include "Game/Enemies/EnemyCommon.h"
#include "Game/Enemies/EnemyDefParse.h"
#include "Game/Enemies/VehicleCommon.h"

// 더블데커 버스 중간보스 (원작 MS2 Mission 4, Docs/Spec_Bus.md).
// 월드에 정지해 있고 카메라 락이 걸린 뒤 시간표대로 개방 -> 계단 전개 -> 체력 단계별 파손 -> 대폭발 -> 잔해 순으로 진행한다.
// 시각은 시트 33825 에서 자른 Bus_* 클립을 겹쳐 그린다 (뒤 조각 -> 본체 -> 창 파편). 앵커 = 바퀴 바닥 중앙.

namespace slug
{

namespace
{
    // 상태: 락 대기 -> 진행 (aux = 락 이후 틱) -> 사망 (life == Dying)
    enum class eBusState
    {
        Parked,
        Active
    };

    constexpr float kPxPerSheetPx = 2.f;    // 시트 1x -> 게임 px
    constexpr float kAnchorU      = 52.f;   // 버스 로컬 좌표 (아케이드 px, 시트 B 프레임 기준) 의 앵커
    constexpr float kAnchorV      = 130.f;
    constexpr int   kOpenTicks    = 36;     // 개방 9 프레임 x 4 틱
    constexpr int   kFlyTicks     = 32;     // 계단 비행 8 프레임 x 4 틱
    constexpr int   kLandTicks    = 6;      // 계단 착지 3 프레임 x 2 틱
    constexpr int   kPersist      = 1 << 20;
    constexpr int   kSortBase     = 0;

    // 단계 진입 시 터지는 폭발 묶음. 좌표는 버스 로컬 (아케이드 px), delay 는 틱.
    struct BurstFx
    {
        int         stage;
        const char* clip;
        float       u;
        float       v;
        int         delay;
    };

    constexpr ARRAY<BurstFx, 12> kBursts = {
        BurstFx { 2, "Explosion_Large", 51.f, -5.f, 0 },
        BurstFx { 2, "Explosion_Large", 42.f, 86.f, 0 },
        BurstFx { 2, "Explosion_Large", 87.f, 70.f, 0 },
        BurstFx { 3, "Explosion_Large", 21.f, 72.f, 0 },
        BurstFx { 3, "Explosion_Mid", 91.f, 113.f, 0 },
        BurstFx { 4, "Explosion_Large", 50.f, 95.f, 0 },
        BurstFx { 4, "Explosion_Mid", 80.f, 100.f, 6 },
        BurstFx { 5, "Explosion_Large", 42.f, 101.f, 0 },
        BurstFx { 6, "Explosion_Mid", 45.f, 100.f, 0 },
        BurstFx { 7, "Explosion_Mid", 9.f, 116.f, 0 },
        BurstFx { 7, "Explosion_Mid", 78.f, 95.f, 3 },
        BurstFx { 6, "Explosion_Small", 80.f, 40.f, 4 }
    };

    // 사망 대폭발: 큰 폭발이 위쪽으로 6~8 개 솟는다 (원작 1:18.0 ~ 1:18.5).
    constexpr ARRAY<BurstFx, 10> kDeathBursts = {
        BurstFx { 0, "Explosion_Large", 20.f, -10.f, 0 },
        BurstFx { 0, "Explosion_Large", 75.f, 0.f, 0 },
        BurstFx { 0, "Explosion_Large", 48.f, 35.f, 2 },
        BurstFx { 0, "Explosion_Large", 10.f, 60.f, 3 },
        BurstFx { 0, "Explosion_Large", 90.f, 55.f, 3 },
        BurstFx { 0, "Explosion_Large", 30.f, 95.f, 5 },
        BurstFx { 0, "Explosion_Large", 70.f, 100.f, 6 },
        BurstFx { 0, "Explosion_Large", 48.f, 125.f, 8 },
        BurstFx { 0, "Explosion_Mid", 40.f, 20.f, 14 },
        BurstFx { 0, "Explosion_Mid", 60.f, 80.f, 18 }
    };

    // 사망 때 튀는 파편. 위치/속도는 게임 px (앵커 기준), 중력 1000.
    struct DebrisFx
    {
        const char* clip;
        float       u;
        float       v;
        float       vx;
        float       vy;
        float       fps;
    };

    constexpr ARRAY<DebrisFx, 9> kDebris = {
        DebrisFx { "Bus_DebGrille", 52.f, 112.f, -60.f, -520.f, 14.f },
        DebrisFx { "Bus_DebBumper", 52.f, 124.f, 90.f, -430.f, 10.f },
        DebrisFx { "Bus_DebFrameA", 30.f, 30.f, -170.f, -640.f, 12.f },
        DebrisFx { "Bus_DebFrameB", 75.f, 28.f, 160.f, -600.f, 12.f },
        DebrisFx { "Bus_DebChunk", 20.f, 70.f, -220.f, -480.f, 16.f },
        DebrisFx { "Bus_DebChunk", 80.f, 60.f, 240.f, -520.f, 16.f },
        DebrisFx { "Bus_DebChunk", 50.f, 20.f, 30.f, -700.f, 16.f },
        DebrisFx { "Bus_DebBar", 15.f, 105.f, -140.f, -380.f, 10.f },
        DebrisFx { "Bus_DebBar", 90.f, 100.f, 130.f, -420.f, 10.f }
    };

    constexpr float kDebrisGravity = 1000.f;

    // 창 안 병사 (아케이드 px 의 발 위치). 단계가 올라가면 하나씩 나타난다.
    struct SoldierSpot
    {
        int   stage;
        float u;
        float v;
        int   phase;
    };

    constexpr ARRAY<SoldierSpot, 4> kSoldiers = {
        SoldierSpot { 3, 98.f, 56.f, 0 },
        SoldierSpot { 4, 55.f, 54.f, 7 },
        SoldierSpot { 4, 83.f, 54.f, 13 },
        SoldierSpot { 5, 22.f, 56.f, 3 }
    };

    [[nodiscard]] Vec2 Local_(
        const Vec2  _feet,
        const float _u,
        const float _v)
    {
        return { _feet.x + (_u - kAnchorU) * kPxPerSheetPx, _feet.y + (_v - kAnchorV) * kPxPerSheetPx };
    }

    // 차체 단계 (0 = 개방 상태, 1..3 = 파손 D1..D3)
    [[nodiscard]] int BodyLevel_(
        const int _stage)
    {
        if (_stage <= 0)
        {
            return 0;
        }
        if (_stage >= 6)
        {
            return 3;
        }
        return _stage >= 4 ? 2 : 1;
    }

    [[nodiscard]] String Named_(
        const char* _base,
        const int   _level)
    {
        return String(_base) + std::to_string(_level);
    }

    void SpawnBursts_(
        EnemyEnv&       _env,
        const Enemy&    _e,
        const int       _stage,
        const bool      _bDeath)
    {
        const Vec2 feet = _e.body.position;
        if (_bDeath)
        {
            for (const BurstFx& fx: kDeathBursts)
            {
                RequestFxAt(_env, fx.clip, Local_(feet, fx.u, fx.v), _e.facing, 60, static_cast<float>(fx.delay) / 60.f);
            }
            return;
        }
        for (const BurstFx& fx: kBursts)
        {
            if (fx.stage == _stage)
            {
                RequestFxAt(_env, fx.clip, Local_(feet, fx.u, fx.v), _e.facing, 60, static_cast<float>(fx.delay) / 60.f);
            }
        }
    }

    void EnterStage_(
        Enemy&          _e,
        const EnemyDef& _def,
        EnemyEnv&       _env,
        const int       _stage)
    {
        (void)_def;
        _e.ext[static_cast<size_t>(_stage)] = _e.aux;
        SpawnBursts_(_env, _e, _stage, false);
    }

    void Init_(
        Enemy&          _e,
        const EnemyDef& _def,
        EnemyEnv&       _env)
    {
        (void)_env;
        PrepareBody(_e, _def);
        _e.facing = -1;
        _e.state  = static_cast<int>(eBusState::Parked);
        _e.aux    = 0;
        _e.aux2   = 0;
        _e.aux3   = 0;
        _e.ext.fill(0);
        _e.clip = -1;
    }

    // 체력이 단계 문턱 이하로 내려가면 단계를 올린다 (한 틱에 여러 단계 가능).
    void UpdateStage_(
        Enemy&          _e,
        const EnemyDef& _def,
        EnemyEnv&       _env)
    {
        const BusCfg& c = _def.bus;
        while (_e.aux2 < BusCfg::kStageCount && _e.health.hp <= c.stageHp[static_cast<size_t>(_e.aux2)])
        {
            ++_e.aux2;
            EnterStage_(_e, _def, _env, _e.aux2);
        }
    }

    void Tick_(
        Enemy&          _e,
        const EnemyDef& _def,
        EnemyEnv&       _env)
    {
        const BusCfg& c  = _def.bus;
        _e.body.velocity = { 0.f, 0.f };
        _e.facing        = -1;

        if (static_cast<eBusState>(_e.state) == eBusState::Parked)
        {
            // 카메라가 락 위치에 멈추면 버스가 화면 안 정해진 곳에 선다
            const float screenX = _e.body.position.x - _env.ctx.cameraView.min.x;
            if (screenX <= c.lockScreenX + c.lockSlackPx)
            {
                EnterState(_e, static_cast<int>(eBusState::Active));
                _e.aux = 0;
            }
            UpdateStage_(_e, _def, _env);
            return;
        }

        ++_e.aux;
        UpdateStage_(_e, _def, _env);

        if (_e.aux3 == 0 && _e.aux2 >= c.bombStage && _e.aux - _e.ext[static_cast<size_t>(c.bombStage)] >= c.bombDelayTicks)
        {
            _e.aux3 = 1;
            (void)FireWeapon(_e, _def, _env, c.weaponBomb);
        }
    }

    void OnDeath_(
        Enemy&          _e,
        const EnemyDef& _def,
        EnemyEnv&       _env)
    {
        (void)_def;
        _e.deathTicks = 0;
        SpawnBursts_(_env, _e, 0, true);
    }

    void TickDying_(
        Enemy&          _e,
        const EnemyDef& _def,
        EnemyEnv&       _env)
    {
        (void)_def;
        (void)_env;
        _e.body.velocity = { 0.f, 0.f };
        ++_e.deathTicks;
    }

    bool DeathDone_(
        const Enemy&    _e,
        const EnemyDef& _def)
    {
        return _e.deathTicks >= _def.bus.destroyDoneTicks;
    }

    void SubmitDebris_(
        const Enemy&          _e,
        const EnemyRenderEnv& _render)
    {
        const float t = static_cast<float>(_e.deathTicks) / 60.f;
        for (const DebrisFx& d: kDebris)
        {
            const Vec2  start  = Local_(_render.feet, d.u, d.v);
            const float groundY = _render.feet.y - 8.f;
            float       tFly   = t;
            float       y      = start.y + d.vy * t + 0.5f * kDebrisGravity * t * t;
            if (y > groundY)
            {
                // 지면에 닿은 시각을 구해 그 뒤로는 멈춘다
                const float disc = d.vy * d.vy + 2.f * kDebrisGravity * (groundY - start.y);
                tFly             = disc > 0.f ? (-d.vy + std::sqrt(disc)) / kDebrisGravity : 0.f;
                y                = groundY;
            }
            const Vec2 at = { start.x + d.vx * tFly, y };
            const int  ticks = static_cast<int>(tFly * d.fps * 60.f / 12.f);   // fps 비율로 회전 속도를 바꾼다 (클립은 12 fps)
            SubmitNamedPart(_render, d.clip, at, ticks, true, kSortBase + 20, _render.layer);
        }
    }

    void SubmitLive_(
        const Enemy&          _e,
        const EnemyDef&       _def,
        const EnemyRenderEnv& _render)
    {
        const BusCfg& c    = _def.bus;
        const Vec2    feet = _render.feet;
        const int     lay  = _render.layer;
        const int     L    = _e.aux;
        const int     stage = _e.aux2;

        // 창 안 병사: 차체보다 뒤
        for (const SoldierSpot& s: kSoldiers)
        {
            if (stage >= s.stage)
            {
                SubmitNamedPart(_render, "SD_Idle_Left", Local_(feet, s.u, s.v), _e.stateTicks + s.phase * 6, true, kSortBase, lay);
            }
        }

        if (stage == 0)
        {
            if (static_cast<eBusState>(_e.state) == eBusState::Parked || L < c.openDelayTicks)
            {
                SubmitNamedPart(_render, "Bus_Idle", feet, 0, false, kSortBase + 2, lay);
                return;
            }
            const int openT   = L - c.openDelayTicks;
            const int stairsT = openT - c.stairsDelayTicks;
            if (stairsT >= 0)
            {
                if (stairsT < kFlyTicks)
                {
                    SubmitNamedPart(_render, "Bus_StairsFly", feet, stairsT, false, kSortBase + 1, lay);
                }
                else if (stairsT < kFlyTicks + kLandTicks)
                {
                    SubmitNamedPart(_render, "Bus_StairsLand", feet, stairsT - kFlyTicks, false, kSortBase + 1, lay);
                }
                else
                {
                    SubmitNamedPart(_render, "Bus_StairsRail", feet, stairsT - kFlyTicks - kLandTicks, false, kSortBase + 1, lay);
                }
            }
            SubmitNamedPart(_render, "Bus_OpenBody", feet, std::min(openT, kPersist), false, kSortBase + 2, lay);
            SubmitNamedPart(_render, "Bus_OpenTop", feet, std::min(openT, kPersist), false, kSortBase + 3, lay);
            return;
        }

        const int level = BodyLevel_(stage);
        SubmitNamedPart(_render, Named_("Bus_BackD", level), feet, 0, false, kSortBase + 1, lay);
        SubmitNamedPart(_render, Named_("Bus_BodyD", level), feet, 0, false, kSortBase + 2, lay);

        // 창 파편 (한 번 재생하고 끝나면 그리지 않는다): 위 두 창은 1단계, 아래 창은 4 / 6 단계
        struct WinStage
        {
            const char* clip;
            int         stage;
        };
        const ARRAY<WinStage, 4> wins = { WinStage { "Bus_WinUL", 1 }, WinStage { "Bus_WinUR", 1 }, WinStage { "Bus_WinLL", 4 }, WinStage { "Bus_WinLR", 6 } };
        for (const WinStage& w: wins)
        {
            if (stage < w.stage)
            {
                continue;
            }
            const int t = L - _e.ext[static_cast<size_t>(w.stage)];
            if (t < GetNamedPartTicks(_render.anims, w.clip))
            {
                SubmitNamedPart(_render, w.clip, feet, t, false, kSortBase + 4, lay);
            }
        }
    }

    void SubmitParts_(
        const Enemy&          _e,
        const EnemyDef&       _def,
        const EnemyRenderEnv& _render)
    {
        if (_e.life == eEnemyLife::Dying)
        {
            if (_e.deathTicks < _def.bus.wreckAfterTicks)
            {
                const int level = BodyLevel_(BusCfg::kStageCount);
                SubmitNamedPart(_render, Named_("Bus_BackD", level), _render.feet, 0, false, kSortBase + 1, _render.layer);
                SubmitNamedPart(_render, Named_("Bus_BodyD", level), _render.feet, 0, false, kSortBase + 2, _render.layer);
            }
            else
            {
                SubmitNamedPart(_render, "Bus_Wreck", _render.feet, 0, false, kSortBase, _render.layer);
            }
            SubmitDebris_(_e, _render);
            return;
        }
        SubmitLive_(_e, _def, _render);
    }

}   // namespace

const BehaviorOps kBusOps = {
    .init              = &Init_,
    .tick              = &Tick_,
    .onDeath           = &OnDeath_,
    .tickDying         = &TickDying_,
    .registerHurtboxes = nullptr,
    .submitParts       = &SubmitParts_,
    .deathDone         = &DeathDone_,
};

void ParseBusCfg(
    DefReader& _r,
    EnemyDef&  _def)
{
    BusCfg& c = _def.bus;

    c.lockScreenX      = _r.Num("lockScreenX", c.lockScreenX);
    c.lockSlackPx      = _r.Num("lockSlackPx", c.lockSlackPx);
    c.openDelayTicks   = _r.Ticks("openDelaySec", static_cast<float>(c.openDelayTicks) / 60.f);
    c.stairsDelayTicks = _r.Ticks("stairsDelaySec", static_cast<float>(c.stairsDelayTicks) / 60.f);
    c.bombStage        = _r.Int("bombStage", c.bombStage);
    c.bombDelayTicks   = _r.Ticks("bombDelaySec", static_cast<float>(c.bombDelayTicks) / 60.f);
    c.wreckAfterTicks  = _r.Ticks("wreckAfterSec", static_cast<float>(c.wreckAfterTicks) / 60.f);
    c.destroyDoneTicks = _r.Ticks("destroyDoneSec", static_cast<float>(c.destroyDoneTicks) / 60.f);

    ARRAY<float, BusCfg::kStageCount> stageHp = {};
    if (_r.NumArray("stageHp", stageHp))
    {
        for (int i = 0; i < BusCfg::kStageCount; ++i)
        {
            c.stageHp[static_cast<size_t>(i)] = static_cast<int>(stageHp[static_cast<size_t>(i)]);
        }
    }

    c.weaponBomb = _r.Weapon("bomb");
}

}   // namespace slug
