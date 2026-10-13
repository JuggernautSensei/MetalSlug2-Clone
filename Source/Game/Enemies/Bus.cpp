#include <algorithm>
#include <cmath>
#include <format>

#include "Base/Math.h"
#include "Game/Enemies/EnemyCommon.h"
#include "Game/Enemies/EnemyDefParse.h"
#include "Game/Enemies/VehicleCommon.h"

// 더블데커 버스 중간보스

enum class eBusState
{
    Parked,
    Active
};

constexpr int kOpenTicks = 36;   // 개방 9 프레임 x 4 틱
constexpr int kFlyTicks  = 32;   // 계단 비행 8 프레임 x 4 틱
constexpr int kLandTicks = 6;    // 계단 착지 3 프레임 x 2 틱
constexpr int kPersist   = 1 << 20;

constexpr int kExtNextSoldier = 8;

constexpr uint32_t kStairSoldierTag  = 0x42555300u;
constexpr int      kVariantClimb     = 2;
constexpr int      kVariantClimbStay = 3;

namespace
{
[[nodiscard]] VECTOR2 Local_(
    const BusCfg& _c,
    VECTOR2       _feet,
    VECTOR2       _at)
{
    return { _feet.x + (_at.x - _c.anchor.x) * _c.pxPerSheetPx, _feet.y + (_at.y - _c.anchor.y) * _c.pxPerSheetPx };
}

void SubmitPart_(
    const EnemyRenderEnv& _render,
    const EnemyPartDepth& _depth,
    StringView            _clip,
    VECTOR2               _at,
    int                   _ticks,
    bool                  _bLoop)
{
    SubmitNamedPart(_render, _clip, _at, _ticks, _bLoop, _depth.sort, _depth.depth);
}

// 차체 단계
[[nodiscard]] int BodyLevel_(
    int _stage)
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
    int         _level)
{
    return String(_base) + std::to_string(_level);
}

void SpawnBursts_(
    EnemyEnv&       _env,
    const Enemy&    _e,
    const EnemyDef& _def,
    int             _stage,
    bool            _bDeath)
{
    const BusCfg& c    = _def.bus;
    const VECTOR2 feet = _e.body.position;
    if (_bDeath)
    {
        for (const BusBurstDef& fx: c.deathBursts)
        {
            RequestFxAt(_env, fx.clip.c_str(), Local_(c, feet, fx.at), _e.facing, c.fxDepth, fx.delaySec);
        }
        return;
    }
    for (const BusBurstDef& fx: c.bursts)
    {
        if (fx.stage == _stage)
        {
            RequestFxAt(_env, fx.clip.c_str(), Local_(c, feet, fx.at), _e.facing, c.fxDepth, fx.delaySec);
        }
    }
}

void EnterStage_(
    Enemy&          _e,
    const EnemyDef& _def,
    EnemyEnv&       _env,
    int             _stage)
{
    _e.ext[static_cast<size_t>(_stage)] = _e.aux;
    SpawnBursts_(_env, _e, _def, _stage, false);
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

// 계단 병사 한 명을 버스 오른쪽 화면 밖에 낸다
void SpawnStairSoldier_(
    Enemy&          _e,
    const EnemyDef& _def,
    EnemyEnv&       _env)
{
    const BusCfg& c        = _def.bus;
    const int     roll     = Roll(_env, 100);
    const String* pDefName = &c.soldierBombDef;
    int           variant  = kVariantClimbStay;
    if (roll < c.swordPercent)
    {
        pDefName = &c.soldierSwordDef;
        variant  = kVariantClimb;
    }
    else if (roll < c.swordPercent + c.bazookaPercent)
    {
        pDefName = &c.soldierBzkDef;
        variant  = kVariantClimb;
    }
    const int defIndex = _env.lib.FindIndex(*pDefName);
    if (defIndex < 0)
    {
        return;
    }
    const VECTOR2 feet = _e.body.position;
    SpawnParams   params;
    params.facing     = -1;
    params.bActivated = true;
    params.aiVariant  = variant;
    params.userTag    = kStairSoldierTag;
    params.arena.min  = { feet.x + c.stairsBase.x, feet.y + c.stairsBase.y };
    params.arena.max  = { feet.x + c.stairsTop.x, feet.y + c.stairsTop.y };
    if (c.roofTop.y != 0.f)
    {
        params.climbEnd = { feet.x + c.roofTop.x, feet.y + c.roofTop.y };
        params.roofSpan = { feet.x + c.roofSpan.x, feet.x + c.roofSpan.y };
    }
    (void)_env.world.SpawnDeferred(defIndex, { feet.x + c.soldierSpawn.x, feet.y + c.soldierSpawn.y }, params);
}

void TickSoldiers_(
    Enemy&          _e,
    const EnemyDef& _def,
    EnemyEnv&       _env)
{
    const BusCfg& c         = _def.bus;
    const int     readyTick = c.openDelayTicks + c.stairsDelayTicks + kFlyTicks + kLandTicks;
    if (_e.aux2 > c.soldierLastStage || _e.aux < readyTick + c.soldierFirstTicks || _e.aux < _e.ext[kExtNextSoldier])
    {
        return;
    }
    EnemyFilter filter;
    filter.userTag = kStairSoldierTag;
    if (_env.world.CountAlive(filter) >= c.soldierMax)
    {
        return;
    }
    SpawnStairSoldier_(_e, _def, _env);
    _e.ext[kExtNextSoldier] = _e.aux + c.soldierGapTicks;
}

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
        _e.health = Health::Make(_def.hp);
        if (_env.ctx.cameraView.max.x >= _e.body.position.x + c.openViewDx)
        {
            EnterState(_e, static_cast<int>(eBusState::Active));
            _e.aux = 0;
        }
        return;
    }

    ++_e.aux;
    UpdateStage_(_e, _def, _env);
    TickSoldiers_(_e, _def, _env);
}

void OnDeath_(
    Enemy&          _e,
    const EnemyDef& _def,
    EnemyEnv&       _env)
{
    _e.deathTicks = 0;
    SpawnBursts_(_env, _e, _def, 0, true);
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
    const EnemyDef&       _def,
    const EnemyRenderEnv& _render)
{
    const BusCfg& c = _def.bus;
    const float   t = static_cast<float>(_e.deathTicks) / 60.f;
    for (const BusDebrisDef& d: c.debris)
    {
        const VECTOR2 start   = Local_(c, _render.feet, d.at);
        const float   groundY = _render.feet.y + c.debrisGroundDy;
        float         tFly    = t;
        float         y       = start.y + d.velocity.y * t + 0.5f * kDebrisGravity * t * t;
        if (y > groundY)
        {
            const float disc = d.velocity.y * d.velocity.y + 2.f * kDebrisGravity * (groundY - start.y);
            tFly             = disc > 0.f ? (-d.velocity.y + Sqrt(disc)) / kDebrisGravity : 0.f;
            y                = groundY;
        }
        const VECTOR2 at    = { start.x + d.velocity.x * tFly, y };
        const int     ticks = static_cast<int>(tFly * d.fps * 60.f / 12.f);
        SubmitPart_(_render, c.partDebris, d.clip, at, ticks, true);
    }
}

void SubmitDamaged_(
    const EnemyRenderEnv& _render,
    const BusCfg&         _c,
    int                   _level,
    VECTOR2               _feet,
    bool                  _bCover)
{
    const String back = Named_("Bus_BackD", _level);
    SubmitPart_(_render, _c.partBack, back, _feet, 0, false);
    SubmitPart_(_render, _c.partBody, Named_("Bus_BodyD", _level), _feet, 0, false);
    if (!_bCover)
    {
        return;
    }
    if (const AnimClip* pBack = _render.anims.FindClip(back); pBack != nullptr && !pBack->frames.empty() && !pBack->frames[0].IsEmpty())
    {
        AnimFrame cover = pBack->frames[0];
        cover.w         = Max(1, Min(cover.w, cover.pivotX + static_cast<int>(_c.coverMaxX)));
        SpriteDrawOptions options;
        options.flashAmount = _render.flash;
        _render.sprites.Submit(_c.partCover.depth + _render.zOffset, _c.partCover.sort, cover, _feet, options);
    }
}

void SubmitLive_(
    const Enemy&          _e,
    const EnemyDef&       _def,
    const EnemyRenderEnv& _render)
{
    const BusCfg& c     = _def.bus;
    const VECTOR2 feet  = _render.feet;
    const int     L     = _e.aux;
    const int     stage = _e.aux2;

    for (const BusInnerSoldierDef& s: c.innerSoldiers)
    {
        if (stage >= s.stage)
        {
            SubmitPart_(_render, c.partInnerSoldier, "SD_Idle_Left", Local_(c, feet, s.at), _e.stateTicks + s.phase * 6, true);
        }
    }

    if (stage == 0)
    {
        if (static_cast<eBusState>(_e.state) == eBusState::Parked || L < c.openDelayTicks)
        {
            SubmitPart_(_render, c.partBody, "Bus_Idle", feet, 0, false);
            return;
        }
        const int openT   = L - c.openDelayTicks;
        const int stairsT = openT - c.stairsDelayTicks;
        if (stairsT >= 0)
        {
            if (stairsT < kFlyTicks)
            {
                SubmitPart_(_render, c.partStairs, "Bus_StairsFly", feet, stairsT, false);
            }
            else if (stairsT < kFlyTicks + kLandTicks)
            {
                SubmitPart_(_render, c.partStairs, "Bus_StairsLand", feet, stairsT - kFlyTicks, false);
            }
            else
            {
                SubmitPart_(_render, c.partStairs, "Bus_StairsRail", feet, stairsT - kFlyTicks - kLandTicks, false);
            }
        }
        SubmitPart_(_render, c.partBody, "Bus_OpenBody", feet, Min(openT, kPersist), false);
        SubmitPart_(_render, c.partCover, "Bus_OpenTop", feet, Min(openT, kPersist), false);
        return;
    }

    SubmitDamaged_(_render, c, BodyLevel_(stage), feet, true);

    for (const BusWindowDef& w: c.windows)
    {
        if (stage < w.stage)
        {
            continue;
        }
        const int t = L - _e.ext[static_cast<size_t>(w.stage)];
        if (t < GetNamedPartTicks(_render.anims, w.clip))
        {
            SubmitPart_(_render, { c.partCover.depth, c.partWindow.sort }, w.clip, feet, t, false);
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
        const BusCfg& c = _def.bus;
        if (_e.deathTicks < c.wreckAfterTicks)
        {
            SubmitDamaged_(_render, c, BodyLevel_(BusCfg::kStageCount), _render.feet, false);
        }
        else
        {
            SubmitPart_(_render, c.partWreck, "Bus_Wreck", _render.feet, 0, false);
        }
        SubmitDebris_(_e, _def, _render);
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

namespace
{
bool ReadNumField_(
    const JsonReader& _o,
    StringView        _key,
    float&            _out)
{
    StringView reason;
    return ReadFloatField(_o, _key, false, _out, reason);
}

bool ReadIntField_(
    const JsonReader& _o,
    StringView        _key,
    int&              _out)
{
    float value = static_cast<float>(_out);
    if (!ReadNumField_(_o, _key, value))
    {
        return false;
    }
    _out = static_cast<int>(std::lround(static_cast<double>(value)));
    return true;
}

bool ReadStrField_(
    const JsonReader& _o,
    StringView        _key,
    String&           _out)
{
    StringView reason;
    return ReadStringField(_o, _key, true, _out, reason);
}

bool ReadVecField_(
    const JsonReader& _o,
    StringView        _key,
    VECTOR2&          _out)
{
    StringView reason;
    return ReadVec2Field(_o, _key, false, _out, reason);
}

void ParseBursts_(
    DefReader&           _r,
    StringView           _key,
    Vector<BusBurstDef>& _out)
{
    const JsonReader* pList = _r.FindRaw(_key);
    if (!pList)
    {
        return;
    }
    if (!pList->IsArray())
    {
        _r.Fail(std::format("params.{}", _key), "객체 배열이 아닙니다");
        return;
    }
    Vector<BusBurstDef> list;
    for (const JsonReader& item: pList->GetArray())
    {
        BusBurstDef b;
        if (!ReadStrField_(item, "clip", b.clip) || !ReadIntField_(item, "stage", b.stage) || !ReadVecField_(item, "at", b.at) || !ReadNumField_(item, "delay", b.delaySec))
        {
            _r.Fail(std::format("params.{}", _key), "{clip, stage, at:[x,y], delay} 항목이 잘못되었습니다");
            return;
        }
        list.push_back(b);
    }
    _out = std::move(list);
}

void ParseDebris_(
    DefReader&            _r,
    Vector<BusDebrisDef>& _out)
{
    const JsonReader* pList = _r.FindRaw("debris");
    if (!pList)
    {
        return;
    }
    if (!pList->IsArray())
    {
        _r.Fail("params.debris", "객체 배열이 아닙니다");
        return;
    }
    Vector<BusDebrisDef> list;
    for (const JsonReader& item: pList->GetArray())
    {
        BusDebrisDef d;
        if (!ReadStrField_(item, "clip", d.clip) || !ReadVecField_(item, "at", d.at) || !ReadVecField_(item, "velocity", d.velocity) || !ReadNumField_(item, "fps", d.fps))
        {
            _r.Fail("params.debris", "{clip, at:[x,y], velocity:[x,y], fps} 항목이 잘못되었습니다");
            return;
        }
        list.push_back(d);
    }
    _out = std::move(list);
}

void ParseInnerSoldiers_(
    DefReader&                  _r,
    Vector<BusInnerSoldierDef>& _out)
{
    const JsonReader* pList = _r.FindRaw("innerSoldiers");
    if (!pList)
    {
        return;
    }
    if (!pList->IsArray())
    {
        _r.Fail("params.innerSoldiers", "객체 배열이 아닙니다");
        return;
    }
    Vector<BusInnerSoldierDef> list;
    for (const JsonReader& item: pList->GetArray())
    {
        BusInnerSoldierDef s;
        if (!ReadIntField_(item, "stage", s.stage) || !ReadVecField_(item, "at", s.at) || !ReadIntField_(item, "phase", s.phase))
        {
            _r.Fail("params.innerSoldiers", "{stage, at:[x,y], phase} 항목이 잘못되었습니다");
            return;
        }
        list.push_back(s);
    }
    _out = std::move(list);
}

void ParseWindows_(
    DefReader&            _r,
    Vector<BusWindowDef>& _out)
{
    const JsonReader* pList = _r.FindRaw("windows");
    if (!pList)
    {
        return;
    }
    if (!pList->IsArray())
    {
        _r.Fail("params.windows", "객체 배열이 아닙니다");
        return;
    }
    Vector<BusWindowDef> list;
    for (const JsonReader& item: pList->GetArray())
    {
        BusWindowDef w;
        if (!ReadStrField_(item, "clip", w.clip) || !ReadIntField_(item, "stage", w.stage) || w.stage < 1 || w.stage > BusCfg::kStageCount)
        {
            _r.Fail("params.windows", "clip 과 stage (1 이상 7 이하) 항목이 잘못되었습니다");
            return;
        }
        list.push_back(w);
    }
    _out = std::move(list);
}
}   // namespace

void ParseBusCfg(
    DefReader& _r,
    EnemyDef&  _def)
{
    BusCfg& c = _def.bus;

    c.openViewDx       = _r.Num("openViewDx", c.openViewDx);
    c.openDelayTicks   = _r.Ticks("openDelaySec", static_cast<float>(c.openDelayTicks) / 60.f);
    c.stairsDelayTicks = _r.Ticks("stairsDelaySec", static_cast<float>(c.stairsDelayTicks) / 60.f);
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

    c.pxPerSheetPx   = _r.Num("pxPerSheetPx", c.pxPerSheetPx);
    c.anchor         = _r.ReadVec2("anchor", c.anchor);
    c.debrisGroundDy = _r.Num("debrisGroundDy", c.debrisGroundDy);
    c.fxDepth        = _r.Int("fxDepth", c.fxDepth);
    ParseBursts_(_r, "bursts", c.bursts);
    ParseBursts_(_r, "deathBursts", c.deathBursts);
    ParseDebris_(_r, c.debris);
    ParseInnerSoldiers_(_r, c.innerSoldiers);
    ParseWindows_(_r, c.windows);

    if (_r.Has("soldierBombDef"))
    {
        c.soldierBombDef = _r.Str("soldierBombDef");
    }
    if (_r.Has("soldierSwordDef"))
    {
        c.soldierSwordDef = _r.Str("soldierSwordDef");
    }
    if (_r.Has("soldierBzkDef"))
    {
        c.soldierBzkDef = _r.Str("soldierBzkDef");
    }
    c.soldierSpawn      = _r.ReadVec2("soldierSpawn", c.soldierSpawn);
    c.stairsBase        = _r.ReadVec2("stairsBase", c.stairsBase);
    c.stairsTop         = _r.ReadVec2("stairsTop", c.stairsTop);
    c.landingW          = _r.Num("landingW", c.landingW);
    c.soldierFirstTicks = _r.Ticks("soldierFirstSec", static_cast<float>(c.soldierFirstTicks) / 60.f);
    c.soldierGapTicks   = _r.Ticks("soldierGapSec", static_cast<float>(c.soldierGapTicks) / 60.f);
    c.soldierMax        = _r.Int("soldierMax", c.soldierMax);
    c.swordPercent      = _r.Int("swordPercent", c.swordPercent);
    c.bazookaPercent    = _r.Int("bazookaPercent", c.bazookaPercent);
    c.soldierLastStage  = _r.Int("soldierLastStage", c.soldierLastStage);
    c.roofTop           = _r.ReadVec2("roofTop", c.roofTop);
    c.roofSpan          = _r.ReadVec2("roofSpan", c.roofSpan);
    c.coverMaxX         = _r.Num("coverMaxX", c.coverMaxX);
}
