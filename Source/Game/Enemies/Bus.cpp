#include "Game/Enemies/EnemyCommon.h"
#include "Base/Math.h"
#include "Game/Enemies/EnemyDefParse.h"

namespace slug
{
namespace
{
    enum eBusState : int
    {
        kClosed,
        kBattle
    };

    constexpr int   kStageCount    = 8;       // HP / 8 마다 한 단계. 사망 = 8
    constexpr float kSoldierFeetDy = 37.5f;   // 병사 발 = 중심 + 37.5 (보병 몸통 높이 75 의 절반)
    constexpr int   kPersistTicks  = 1 << 20; // 이미 끝난 비루프 연출을 마지막 프레임에 머물게 하는 값

    [[nodiscard]] Vec2 OriginOf_(
        const Enemy&   _e,
        const BusCfg& _c)
    {
        return { _e.body.position.x, _e.body.position.y - _c.originDepth };
    }

    [[nodiscard]] int StageOf_(
        const Health& _h)
    {
        if (_h.hp <= 0)
        {
            return kStageCount;
        }
        int stage = 0;
        for (int s = 1; s < kStageCount; ++s)
        {
            if (_h.hp * kStageCount <= _h.maxHp * (kStageCount - s))
            {
                stage = s;
            }
        }
        return stage;
    }

    void Fx_(
        EnemyEnv&         _env,
        const char*       _clip,
        const Vec2 _at,
        const int         _delayTicks = 0)
    {
        EnemyFxRequest request;
        request.clip       = _clip;
        request.position   = _at;
        request.delayTicks = _delayTicks;
        _env.world.RequestFx(request);
    }

    [[nodiscard]] Vec2 Scatter_(
        EnemyEnv&         _env,
        const Vec2 _c,
        const float       _half)
    {
        const int span = static_cast<int>(_half) * 2 + 1;
        const float dx = static_cast<float>(Roll(_env, span) - static_cast<int>(_half));
        const float dy = static_cast<float>(Roll(_env, span) - static_cast<int>(_half));
        return { _c.x + dx, _c.y + dy };
    }

    void EnterStage_(
        Enemy&          _e,
        const EnemyDef& _def,
        EnemyEnv&       _env,
        const int       _stage)
    {
        const BusCfg&     c      = _def.bus;
        const Vec2 origin = OriginOf_(_e, c);
        if (_stage == kStageCount)
        {
            const Vec2 center = { origin.x - 25.f, origin.y - 50.f };
            for (int i = 0; i < 2; ++i)
            {
                Fx_(_env, "Explosion_Small", Scatter_(_env, center, 100.f), i * 4);
            }
            for (int i = 0; i < 4; ++i)
            {
                Fx_(_env, "Explosion_Mid", Scatter_(_env, center, 100.f), 8 + i * 4);
                Fx_(_env, "Explosion_Large", Scatter_(_env, center, 100.f), 12 + i * 4);
            }
            return;
        }
        const Vec2 center = { origin.x - 50.f, origin.y - 50.f };
        Fx_(_env, "Explosion_Small", Scatter_(_env, center, 75.f));
        Fx_(_env, "Explosion_Mid", Scatter_(_env, center, 75.f));
        Fx_(_env, "Explosion_Large", Scatter_(_env, center, 75.f));
        switch (_stage)
        {
            case 1: Fx_(_env, "LeftUp_Win_Destroy", { origin.x - 20.f, origin.y - 71.f }); break;
            case 2:
                Fx_(_env, "RightUp_Win_Destroy", { origin.x + 40.f, origin.y - 65.f });
                Fx_(_env, "LeftDown_Win_Destroy", { origin.x - 20.f, origin.y + 7.f });
                break;
            case 4: Fx_(_env, "RightDown_Win_Destroy", { origin.x + 119.f, origin.y + 36.f }); break;
            default: break;
        }
    }

    void UpdateStage_(
        Enemy&          _e,
        const EnemyDef& _def,
        EnemyEnv&       _env)
    {
        const int stage = StageOf_(_e.health);
        if (stage <= _e.aux)
        {
            return;
        }
        for (int s = _e.aux + 1; s <= stage; ++s)
        {
            EnterStage_(_e, _def, _env, s);
        }
        _e.aux     = stage;
        _e.ext[0]  = _e.stateTicks;   // 현재 단계 진입 시각 (파츠 연출 시계)
    }

    void Open_(
        Enemy& _e)
    {
        EnterState(_e, kBattle);
        _e.clipTicks = 0;          // 문 개방 연출 시계
    }

    void TickBattle_(
        Enemy&          _e,
        const EnemyDef& _def,
        EnemyEnv&       _env)
    {
        const BusCfg& c = _def.bus;
        if (!_env.ctx.player.bAlive)
        {
            return;
        }

        const int nextShell = c.shellFirstTicks + _e.aux2 * c.shellIntervalTicks;
        if (_e.stateTicks >= nextShell)
        {
            (void)FireWeapon(_e, _def, _env, c.weaponShell);
            ++_e.aux2;
        }

        const int nextSoldier = c.soldierFirstTicks + _e.aux4 * c.soldierIntervalTicks;
        if (_e.stateTicks >= nextSoldier)
        {
            ++_e.aux4;
            const int defIndex = _env.lib.FindIndex(c.soldierDef);
            if (defIndex >= 0 && _e.aux3 < c.soldierTotalMax)
            {
                EnemyFilter filter;
                filter.defIndex = defIndex;
                filter.userTag  = _e.id;
                if (_env.world.CountAlive(filter) < c.soldierAliveMax)
                {
                    const Vec2 origin = OriginOf_(_e, c);
                    SpawnParams        params;
                    params.facing     = -1;
                    params.bActivated = true;
                    params.userTag    = _e.id;
                    (void)_env.world.SpawnDeferred(defIndex, { origin.x + c.soldierOffset.x, origin.y + c.soldierOffset.y + kSoldierFeetDy }, params);
                    ++_e.aux3;
                }
            }
        }
    }

    void Init_(
        Enemy&          _e,
        const EnemyDef& _def,
        EnemyEnv&       _env)
    {
        PrepareBody(_e, _def);
        _e.facing  = 1;
        _e.ext[0]  = 0;
        _e.aux     = 0;
        EnterState(_e, kClosed);
        UpdateStage_(_e, _def, _env);
    }

    void Tick_(
        Enemy&          _e,
        const EnemyDef& _def,
        EnemyEnv&       _env)
    {
        _e.facing = 1;
        _e.body.velocity = { 0.f, 0.f };
        UpdateStage_(_e, _def, _env);

        if (_e.state == kClosed)
        {
            const Aabb trigger = _def.bus.trigger.At(OriginOf_(_e, _def.bus), 1);
            if (_env.ctx.player.bAlive && _env.ctx.player.bodyBox.Overlaps(trigger))
            {
                Open_(_e);
            }
            return;
        }
        TickBattle_(_e, _def, _env);
    }

    void OnDeath_(
        Enemy&          _e,
        const EnemyDef& _def,
        EnemyEnv&       _env)
    {
        for (int s = _e.aux + 1; s <= kStageCount; ++s)
        {
            EnterStage_(_e, _def, _env, s);
        }
        _e.aux    = kStageCount;
        _e.ext[0] = _e.stateTicks;
    }

    void TickDying_(
        Enemy&,
        const EnemyDef&,
        EnemyEnv&)
    {
    }

    bool DeathDone_(
        const Enemy&,
        const EnemyDef&)
    {
        return true;
    }

    void SubmitParts_(
        const Enemy&          _e,
        const EnemyDef&       _def,
        const EnemyRenderEnv& _render)
    {
        const BusCfg&     c      = _def.bus;
        const Vec2 origin = { _render.feet.x, _render.feet.y - c.originDepth };
        const int         sortKey = static_cast<int>(_render.feet.y);
        const int         layer   = _render.layer;
        const int         stage   = _e.aux;
        const int         sinceStage = _e.stateTicks - _e.ext[0];

        if (stage >= kStageCount)
        {
            SubmitNamedPart(_render, "MainTruck_Die", { origin.x, origin.y + 60.f }, kPersistTicks, false, sortKey, layer);
            SubmitNamedPart(_render, "MainTruck_Die_Front", { origin.x + 12.f, origin.y + 88.f }, kPersistTicks, false, sortKey + 1, layer);
            return;
        }

        const bool bOpen = _e.state == kBattle;
        const int  idle  = stage < 3 ? stage : 3;   // Idle_0..3 / Ladder_0..3

        static constexpr const char* kIdleNames[]   = { "MainTruck_Idle_0", "MainTruck_Idle_1", "MainTruck_Idle_2", "MainTruck_Idle_3" };
        static constexpr const char* kLadderNames[] = { "MainTruck_Ladder_0", "MainTruck_Ladder_1", "MainTruck_Ladder_2", "MainTruck_Ladder_3" };
        if (!bOpen)
        {
            SubmitNamedPart(_render, "MainTruck_Not_Begin", origin, kPersistTicks, false, sortKey, layer);
        }
        else if (_e.clipTicks < c.openTicks)
        {
            SubmitNamedPart(_render, "MainTruck_Begin", { origin.x - 33.f, origin.y + 1.f }, _e.clipTicks, false, sortKey, layer);
        }
        else
        {
            SubmitNamedPart(_render, kIdleNames[idle], { origin.x - 33.f, origin.y + 1.f }, kPersistTicks, false, sortKey, layer);
        }
        if (bOpen)
        {
            SubmitNamedPart(_render, kLadderNames[idle], { origin.x + 54.f, origin.y + 6.f }, kPersistTicks, false, sortKey, layer);
        }

        if (stage == 2)
        {
            SubmitNamedPart(_render, "MainTruck_Deco_Destroy_Stop", origin, kPersistTicks, false, sortKey + 2, layer);
        }
        else if (stage >= 3)
        {
            SubmitNamedPart(_render, "MainTruck_Deco_Destroy", origin, stage == 3 ? sinceStage : kPersistTicks, false, sortKey + 2, layer);
        }
        if (stage >= 5)
        {
            SubmitNamedPart(_render, "MainTruck_Big_Win_Destroy_Begin", { origin.x - 18.f, origin.y + 60.f }, stage == 5 ? sinceStage : kPersistTicks, false, sortKey + 2, layer);
        }
        if (stage >= 6)
        {
            SubmitNamedPart(_render, "MainTruck_Bump", origin, stage == 6 ? sinceStage : kPersistTicks, false, sortKey + 3, layer);
        }
        if (stage >= 7)
        {
            SubmitNamedPart(_render, "MainTruck_Sideparts", origin, sinceStage, false, sortKey + 3, layer);
        }
    }
}

const BehaviorOps kBusOps = {
    .init              = &Init_,
    .tick              = &Tick_,
    .onHit             = nullptr,
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

    c.originDepth          = _r.Num("originDepth");
    c.trigger              = _r.Box("trigger");
    c.openTicks            = _r.Ticks("openSec");
    c.shellFirstTicks      = _r.Ticks("shellFirstSec");
    c.shellIntervalTicks   = _r.Ticks("shellIntervalSec");
    c.soldierFirstTicks    = _r.Ticks("soldierFirstSec");
    c.soldierIntervalTicks = _r.Ticks("soldierIntervalSec");
    c.soldierAliveMax      = _r.Int("soldierAliveMax");
    c.soldierTotalMax      = _r.Int("soldierTotalMax");
    c.soldierOffset        = _r.ReadVec2("soldierOffset", { 0.f, 0.f });
    c.soldierDef           = _r.Str("soldierDef");
    c.weaponShell          = _r.Weapon("shell");
}
}
