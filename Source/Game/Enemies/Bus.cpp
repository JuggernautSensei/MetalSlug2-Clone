#include "Game/Enemies/EnemyCommon.h"
#include "Game/Enemies/EnemyDefParse.h"

namespace slug
{

// ===========================================
//  Bus — 버스 중간보스 MBTruck (Docs/EnemySpecs.md 3.1)
//   닫힘 → 플레이어 몸이 트리거에 닿으면 문이 열리고(MainTruck_Begin) 전투 시계가 시작된다.
//   전투: 포탄 첫 2 s 후 3 s 간격 (플레이어 조준, 포물선), 병사 투하 첫 3 s 후 5 s 간격 (생존 2 / 누적 8).
//   파손: HP 를 8 등분해 단계마다 스프라이트/이펙트를 바꾼다. 0 이면 사망 → 부모(EnemyWorld)가 Died 이벤트를 낸다.
//      스테이지는 Died 를 기다려 지면 픽셀맵을 바꾼다 (이 파일은 지면을 건드리지 않는다).
//   좌표: 트리거/투하 위치는 "원점" 기준(원점 = 발 - originDepth), 포탄 머즐은 무기 규약(발 기준). 방향은 항상 +1.
//   레거시 수정: 개방 판정은 트리거 상자와 플레이어 몸통이 겹칠 때만 (레거시는 모든 겹침).
// ===========================================

namespace
{
    enum eBusState : int
    {
        kClosed,   // 문 닫힘: 트리거 대기
        kBattle    // 문 열림 / 전투 시계 진행
    };

    constexpr int   kStageCount    = 8;       // HP / 8 마다 한 단계. 사망 = 8
    constexpr float kSoldierFeetDy = 37.5f;   // 병사 발 = 중심 + 37.5 (보병 몸통 높이 75 의 절반)
    constexpr int   kPersistTicks  = 1 << 20; // 이미 끝난 비루프 연출을 마지막 프레임에 머물게 하는 값

    // 원점 = 발 - (0, originDepth)
    [[nodiscard]] jug::VECTOR2 OriginOf_(
        const Enemy&   _e,
        const BusCfg& _c)
    {
        return { _e.body.position.x, _e.body.position.y - _c.originDepth };
    }

    // 현재 HP 가 속한 파손 단계 (1..7). 0 HP 는 사망(8)
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
        const jug::VECTOR2 _at,
        const int         _delayTicks = 0)
    {
        EnemyFxRequest request;
        request.clip       = _clip;
        request.position   = _at;
        request.delayTicks = _delayTicks;
        _env.world.RequestFx(request);
    }

    // 중심 _c 주변 ±_half 균등 난수 한 점
    [[nodiscard]] jug::VECTOR2 Scatter_(
        EnemyEnv&         _env,
        const jug::VECTOR2 _c,
        const float       _half)
    {
        const int span = static_cast<int>(_half) * 2 + 1;
        const float dx = static_cast<float>(Roll(_env, span) - static_cast<int>(_half));
        const float dy = static_cast<float>(Roll(_env, span) - static_cast<int>(_half));
        return { _c.x + dx, _c.y + dy };
    }

    // 파손 단계 진입 연출 (단계별 창틀/장식 파괴 이펙트, 중심 폭발). 사망(8)은 큰 폭발.
    void EnterStage_(
        Enemy&          _e,
        const EnemyDef& _def,
        EnemyEnv&       _env,
        const int       _stage)
    {
        const BusCfg&     c      = _def.bus;
        const jug::VECTOR2 origin = OriginOf_(_e, c);
        if (_stage == kStageCount)
        {
            // 사망: CreateExplosionEffect(pos,(-25,-50),200, S2 M4 L4) 상당
            const jug::VECTOR2 center = { origin.x - 25.f, origin.y - 50.f };
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
        // 기본 폭발 (1/1/1, 중심 (-50,-50), 균등 ±75)
        const jug::VECTOR2 center = { origin.x - 50.f, origin.y - 50.f };
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

    // 파손 단계 갱신 (전투 중과 닫힘 상태 모두). 단계가 올라갈 때마다 진입 연출.
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
        EnterState(_e, kBattle);   // stateTicks = 0: 전투 시계 시작
        _e.clipTicks = 0;          // 문 개방 연출 시계
    }

    // 플레이어 발사 / 병사 투하 타이머 (전투 시계 기준). 플레이어가 죽었으면 멈춘다.
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

        // 포탄: 첫 발 2 s 후, 이후 3 s 간격 (발사 횟수 aux2 로 다음 시각을 정한다)
        const int nextShell = c.shellFirstTicks + _e.aux2 * c.shellIntervalTicks;
        if (_e.stateTicks >= nextShell)
        {
            (void)FireWeapon(_e, _def, _env, c.weaponShell);
            ++_e.aux2;
        }

        // 병사 투하: 첫 3 s 후, 이후 5 s 간격 (aux4 = 지나간 투하 시점 수). 생존 2 / 누적 8 로 제한
        const int nextSoldier = c.soldierFirstTicks + _e.aux4 * c.soldierIntervalTicks;
        if (_e.stateTicks >= nextSoldier)
        {
            ++_e.aux4;
            const int defIndex = _env.lib.FindIndex(c.soldierDef);
            if (defIndex >= 0 && _e.aux3 < c.soldierTotalMax)
            {
                EnemyFilter filter;
                filter.defIndex = defIndex;
                filter.userTag  = _e.id;   // 이 버스가 낸 병사만 센다
                if (_env.world.CountAlive(filter) < c.soldierAliveMax)
                {
                    const jug::VECTOR2 origin = OriginOf_(_e, c);
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
        // 남은 파손 단계 연출을 한꺼번에 진행하고 사망 상태로 둔다 (잔해 유지, 판정 없음)
        for (int s = _e.aux + 1; s <= kStageCount; ++s)
        {
            EnterStage_(_e, _def, _env, s);
        }
        _e.aux    = kStageCount;
        _e.ext[0] = _e.stateTicks;
    }

    // 사망 후에는 잔해만 남는다 (Dying 상태 유지: Removed 이벤트 없음, 스테이지가 Died 로 처리)
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

    // ===========================================
    //  렌더 파츠
    // ===========================================

    void SubmitParts_(
        const Enemy&          _e,
        const EnemyDef&       _def,
        const EnemyRenderEnv& _render)
    {
        const BusCfg&     c      = _def.bus;
        const jug::VECTOR2 origin = { _render.feet.x, _render.feet.y - c.originDepth };
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

        // 본체: 닫힘(Not_Begin) → 개방 중(Begin 클립) → 파손 단계별 Idle
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

        // 파손 단계 파츠 (이전 단계 것은 끝난 상태로 남고, 현재 단계 것만 진행 시계로 재생한다)
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

}   // namespace

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
    c.soldierOffset        = _r.Vec2("soldierOffset", { 0.f, 0.f });
    c.soldierDef           = _r.Str("soldierDef");
    c.weaponShell          = _r.Weapon("shell");
}

}   // namespace slug
