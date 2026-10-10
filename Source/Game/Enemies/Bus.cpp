#include "Game/Enemies/EnemyCommon.h"
#include "Base/Math.h"
#include "Game/Enemies/EnemyDefParse.h"

namespace slug
{
namespace
{
    // 상태: 주차 (무적, 벽) -> 카메라 팬 대기 -> 개방 연출 -> 전투 (피격 가능, 파손 단계 진행).
    // 사망은 life == Dying 으로 표현하고 잔해를 계속 그린다.
    enum eBusState : int
    {
        kParked,
        kReveal,
        kOpening,
        kBattle
    };

    constexpr int kStageCount    = 8;         // HP / 8 마다 한 단계. 0..7 = 살아 있는 단계. 사망 시각은 ext[kStageCount] 에 둔다
    constexpr int kPersistTicks  = 1 << 20;   // 끝난 비루프 연출은 마지막 프레임에 머문다
    constexpr int kBigBeginTicks = 25;        // Bus_BigWin_Begin 5 프레임 @ 12 fps

    // 단계별 본체 스프라이트 (0 = 개방 직후, 7 = 최종). 개방 중에는 Bus_Opening 클립이 본체를 대신한다.
    constexpr const char* kBodyNames[kStageCount] = {
        "Bus_Open", "Bus_Damaged_1", "Bus_Damaged_2", "Bus_Ladder_2",
        "Bus_Ladder_3", "Bus_Damaged_3", "Bus_Smoke", "Bus_Smoke"
    };

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

    [[nodiscard]] Vec2 ChainCenter_(
        const Vec2 _feet)
    {
        return { _feet.x - 25.f, _feet.y - 150.f };   // 본체 상단 (전면 좌측 창 근처)
    }

    void Fx_(
        EnemyEnv&   _env,
        const char* _clip,
        const Vec2  _at,
        const int   _delayTicks = 0)
    {
        EnemyFxRequest request;
        request.clip       = _clip;
        request.position   = _at;
        request.delayTicks = _delayTicks;
        _env.world.RequestFx(request);
    }

    [[nodiscard]] Vec2 Scatter_(
        EnemyEnv&   _env,
        const Vec2  _c,
        const float _half)
    {
        const int   span = static_cast<int>(_half) * 2 + 1;
        const float dx   = static_cast<float>(Roll(_env, span) - static_cast<int>(_half));
        const float dy   = static_cast<float>(Roll(_env, span) - static_cast<int>(_half));
        return { _c.x + dx, _c.y + dy };
    }

    // 사망 연쇄 (Small 2, Mid 4, Large 4, 0.5 s 안에 끝난다). Docs/Spec_Bus.md 1절: 사망 시 대폭발 후 잔해.
    void DeathChain_(
        const Enemy& _e,
        EnemyEnv&    _env)
    {
        const Vec2 center = ChainCenter_(_e.body.position);
        for (int i = 0; i < 2; ++i)
        {
            Fx_(_env, "Explosion_Small", Scatter_(_env, center, 100.f), i * 4);
        }
        for (int i = 0; i < 4; ++i)
        {
            Fx_(_env, "Explosion_Mid", Scatter_(_env, center, 100.f), 8 + i * 4);
            Fx_(_env, "Explosion_Large", Scatter_(_env, center, 100.f), 12 + i * 4);
        }
    }

    // 단계 진입 기록 (부품 시계). 6 단계 (약 77.6 s) 는 작은 폭발, 7 단계 (약 78.8 s) 는 중간/큰 폭발.
    void EnterStage_(
        Enemy&    _e,
        EnemyEnv& _env,
        const int _stage)
    {
        _e.ext[_stage] = _e.stateTicks;
        if (_stage == 6)
        {
            Fx_(_env, "Explosion_Small", Scatter_(_env, ChainCenter_(_e.body.position), 60.f));
        }
        else if (_stage == 7)
        {
            Fx_(_env, "Explosion_Mid", Scatter_(_env, ChainCenter_(_e.body.position), 60.f));
            Fx_(_env, "Explosion_Large", Scatter_(_env, ChainCenter_(_e.body.position), 60.f), 18);
        }
    }

    void UpdateStage_(
        Enemy&    _e,
        EnemyEnv& _env)
    {
        const int stage = StageOf_(_e.health);
        if (stage <= _e.aux)
        {
            return;
        }
        for (int s = _e.aux + 1; s <= stage; ++s)
        {
            EnterStage_(_e, _env, s);
        }
        _e.aux = stage;
    }

    void Init_(
        Enemy&          _e,
        const EnemyDef& _def,
        EnemyEnv&       _env)
    {
        (void)_env;
        PrepareBody(_e, _def);
        _e.facing = 1;
        _e.aux    = 0;   // state 기본값 = kParked (명령이 초기화 전에 도착해도 덮어쓰지 않는다)
    }

    void Tick_(
        Enemy&          _e,
        const EnemyDef& _def,
        EnemyEnv&       _env)
    {
        const BusCfg& c  = _def.bus;
        _e.facing        = 1;
        _e.body.velocity = { 0.f, 0.f };
        UpdateStage_(_e, _env);

        switch (_e.state)
        {
        case kReveal:
            if (_e.stateTicks >= c.revealTicks)
            {
                EnterState(_e, kOpening);
            }
            break;
        case kOpening:
            if (_e.stateTicks >= c.openTicks)
            {
                EnterState(_e, kBattle);
            }
            break;
        default:
            break;
        }
    }

    // 스테이지 명령. startBattle 은 카메라 팬 (lockCamera glide) 과 같이 한 번만 온다.
    void OnCommand_(
        Enemy&                 _e,
        const EnemyDef&        _def,
        const slug::StringView _command)
    {
        (void)_def;
        if (_command == "startBattle" && _e.state == kParked)
        {
            EnterState(_e, kReveal);
        }
    }

    // 주차 / 팬 중에는 피격을 흡수한다 (벽: 피해 없음). 개방 연출부터 피해를 받는다.
    bool OnHit_(
        Enemy&          _e,
        const EnemyDef& _def,
        EnemyEnv&       _env,
        const HitEvent& _event)
    {
        (void)_def;
        (void)_env;
        (void)_event;
        return _e.state < kOpening;
    }

    void OnDeath_(
        Enemy&          _e,
        const EnemyDef& _def,
        EnemyEnv&       _env)
    {
        (void)_def;
        for (int s = _e.aux + 1; s < kStageCount; ++s)
        {
            _e.ext[s] = _e.stateTicks;
        }
        _e.aux              = kStageCount;
        _e.ext[kStageCount] = _e.stateTicks;
        DeathChain_(_e, _env);
    }

    void TickDying_(
        Enemy&,
        const EnemyDef&,
        EnemyEnv&)
    {
    }

    // 잔해는 사망 후 계속 남는다. solid 는 Alive 에서만 적용되므로 잔해 위로 걸어갈 수 있다.
    bool DeathDone_(
        const Enemy&,
        const EnemyDef&)
    {
        return true;
    }

    // 단계 진입 뒤 한 번 재생되는 부품. 창 파손 (2~5), 파편 (3, 5, 6, 7), 최종 창 (5).
    void SubmitStageExtras_(
        const Enemy&          _e,
        const EnemyRenderEnv& _render,
        const Vec2            _feet,
        const int             _sortKey)
    {
        const int stage = _e.aux;
        const int layer = _render.layer;
        auto      since = [&_e](const int _s) { return _e.stateTicks - _e.ext[_s]; };

        if (stage >= 2)
        {
            SubmitNamedPart(_render, "Bus_WinLeftUp", _feet, since(2), false, _sortKey + 1, layer);
        }
        if (stage >= 3)
        {
            SubmitNamedPart(_render, "Bus_WinRightUp", _feet, since(3), false, _sortKey + 1, layer);
            SubmitNamedPart(_render, "Bus_Deco", _feet, since(3), false, _sortKey + 2, layer);
        }
        if (stage >= 4)
        {
            SubmitNamedPart(_render, "Bus_WinLeftDown", _feet, since(4), false, _sortKey + 1, layer);
        }
        if (stage >= 5)
        {
            const int t = since(5);
            if (t < kBigBeginTicks)
            {
                SubmitNamedPart(_render, "Bus_BigWin_Begin", _feet, t, false, _sortKey + 2, layer);
            }
            else
            {
                SubmitNamedPart(_render, "Bus_BigWin_End", _feet, t - kBigBeginTicks, false, _sortKey + 2, layer);
            }
            SubmitNamedPart(_render, "Bus_WinRightDown", _feet, since(5), false, _sortKey + 1, layer);
        }
        if (stage >= 6)
        {
            SubmitNamedPart(_render, "Bus_Bump", _feet, since(6), false, _sortKey + 2, layer);
        }
        if (stage >= 7)
        {
            SubmitNamedPart(_render, "Bus_Sideparts", _feet, since(7), false, _sortKey + 2, layer);
        }
    }

    void SubmitParts_(
        const Enemy&          _e,
        const EnemyDef&       _def,
        const EnemyRenderEnv& _render)
    {
        const Vec2 feet    = _render.feet;
        const int  sortKey = static_cast<int>(feet.y);
        const int  layer   = _render.layer;

        if (_e.life == eEnemyLife::Dying)
        {
            const int sinceDeath = _e.stateTicks - _e.ext[kStageCount];
            if (sinceDeath < _def.bus.chainTicks)
            {
                SubmitNamedPart(_render, kBodyNames[kStageCount - 1], feet, kPersistTicks, false, sortKey, layer);
            }
            else
            {
                SubmitNamedPart(_render, "Bus_Wreck", feet, kPersistTicks, false, sortKey, layer);
                SubmitNamedPart(_render, "Bus_WreckFront", { feet.x + 12.f, feet.y }, kPersistTicks, false, sortKey + 1, layer);
            }
            return;
        }

        if (_e.state == kParked || _e.state == kReveal)
        {
            SubmitNamedPart(_render, "Bus_Idle", feet, kPersistTicks, false, sortKey, layer);
            return;
        }
        if (_e.state == kOpening)
        {
            SubmitNamedPart(_render, "Bus_Opening", feet, _e.stateTicks, false, sortKey, layer);
            return;
        }

        const int  stage = _e.aux;
        const bool bLoop = stage >= kStageCount - 2;   // 연기 본체 (6, 7 단계) 는 반복 재생
        SubmitNamedPart(_render, kBodyNames[stage], feet, bLoop ? _e.stateTicks : kPersistTicks, bLoop, sortKey, layer);
        SubmitStageExtras_(_e, _render, feet, sortKey);
    }
}

const BehaviorOps kBusOps = {
    .init              = &Init_,
    .tick              = &Tick_,
    .onHit             = &OnHit_,
    .onDeath           = &OnDeath_,
    .tickDying         = &TickDying_,
    .registerHurtboxes = nullptr,
    .submitParts       = &SubmitParts_,
    .deathDone         = &DeathDone_,
    .onCommand         = &OnCommand_,
};

void ParseBusCfg(
    DefReader& _r,
    EnemyDef&  _def)
{
    BusCfg& c = _def.bus;

    c.revealTicks = _r.Ticks("revealSec");
    c.openTicks   = _r.Ticks("openSec");
    c.chainTicks  = _r.Ticks("chainSec");
}
}
