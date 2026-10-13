#include "Game/Enemies/BusEnemy.h"

#include <algorithm>
#include <cmath>
#include <format>

#include "Game/Enemies/EnemyCommon.h"
#include "Game/Enemies/EnemyDefParse.h"
#include "Game/Enemies/VehicleCommon.h"

// 더블데커 버스 중간보스

constexpr int kFlyTicks  = 32;   // 계단 비행 8 프레임 x 4 틱
constexpr int kLandTicks = 6;    // 계단 착지 3 프레임 x 2 틱
constexpr int kPersist   = 1 << 20;

constexpr uint32_t kStairSoldierTag  = 0x42555300u;
constexpr int      kVariantClimb     = 2;
constexpr int      kVariantClimbStay = 3;

namespace
{
[[nodiscard]] VECTOR2 Local_(
    const BusConfig& _config,
    const VECTOR2    _feet,
    const VECTOR2    _at)
{
    return { _feet.x + (_at.x - _config.anchor.x) * _config.pxPerSheetPx, _feet.y + (_at.y - _config.anchor.y) * _config.pxPerSheetPx };
}

void SubmitPart_(
    const EnemyRenderEnv& _render,
    const EnemyPartDepth& _depth,
    const StringView      _clip,
    const VECTOR2         _at,
    const int             _ticks,
    const bool            _bLoop)
{
    SubmitNamedPart(_render, _clip, _at, _ticks, _bLoop, _depth.sort, _depth.depth);
}

// 차체 단계
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
    EnemyEnv&         _env,
    const EnemyState& _e,
    const EnemyDef&   _def,
    const int         _stage,
    const bool        _bDeath)
{
    const BusConfig& config = _def.bus;
    const VECTOR2    feet   = _e.body.position;
    if (_bDeath)
    {
        for (const BusBurstDef& fx: config.deathBursts)
        {
            RequestFxAt(_env, fx.clip.c_str(), Local_(config, feet, fx.at), _e.facing, config.fxDepth, fx.delaySec);
        }
        return;
    }
    for (const BusBurstDef& fx: config.bursts)
    {
        if (fx.stage == _stage)
        {
            RequestFxAt(_env, fx.clip.c_str(), Local_(config, feet, fx.at), _e.facing, config.fxDepth, fx.delaySec);
        }
    }
}

// 계단 병사 한 명을 버스 오른쪽 화면 밖에 낸다

void SubmitDamaged_(
    const EnemyRenderEnv& _render,
    const BusConfig&      _config,
    const int             _level,
    const VECTOR2         _feet,
    const bool            _bCover)
{
    const String back = Named_("Bus_BackD", _level);
    SubmitPart_(_render, _config.partBack, back, _feet, 0, false);
    SubmitPart_(_render, _config.partBody, Named_("Bus_BodyD", _level), _feet, 0, false);
    if (!_bCover)
    {
        return;
    }
    if (const AnimClip* pBack = _render.anims.FindClip(back); pBack != nullptr && !pBack->frames.empty() && !pBack->frames[0].IsEmpty())
    {
        AnimFrame cover = pBack->frames[0];
        cover.w         = Max(1, Min(cover.w, cover.pivotX + static_cast<int>(_config.coverMaxX)));
        SpriteDrawOptions options;
        options.flashAmount = _render.flash;
        _render.sprites.Submit(_config.partCover.depth + _render.zOffset, _config.partCover.sort, cover, _feet, options);
    }
}

}   // namespace

void BusEnemy::Init(
    const EnemyDef& _def,
    EnemyEnv&       _env)
{
    EnemyState& _e = GetMutableState();
    (void)_env;
    PrepareBody(_e, _def);
    _e.facing      = -1;
    _e.state       = static_cast<int>(eBusState::Parked);
    m_elapsedTicks = 0;
    m_damageStage  = 0;
    m_stageStartTicks.fill(0);
    m_nextSoldierTick = 0;
    _e.clip           = -1;
}

void BusEnemy::Tick(
    const EnemyDef& _def,
    EnemyEnv&       _env)
{
    EnemyState&      _e     = GetMutableState();
    const BusConfig& config = _def.bus;
    _e.body.velocity        = { 0.f, 0.f };
    _e.facing               = -1;

    if (static_cast<eBusState>(_e.state) == eBusState::Parked)
    {
        _e.health = Health::Make(_def.hp);
        if (_env.ctx.cameraView.max.x >= _e.body.position.x + config.openViewDx)
        {
            EnterState(_e, static_cast<int>(eBusState::Active));
            m_elapsedTicks = 0;
        }
        return;
    }

    ++m_elapsedTicks;
    UpdateStage_(_def, _env);
    TickSoldiers_(_def, _env);
}

void BusEnemy::OnDeath(
    const EnemyDef& _def,
    EnemyEnv&       _env)
{
    EnemyState& _e = GetMutableState();
    _e.deathTicks  = 0;
    SpawnBursts_(_env, _e, _def, 0, true);
}

void BusEnemy::TickDying(
    const EnemyDef& _def,
    EnemyEnv&       _env)
{
    EnemyState& _e = GetMutableState();
    (void)_def;
    (void)_env;
    _e.body.velocity = { 0.f, 0.f };
    ++_e.deathTicks;
}

void BusEnemy::SubmitParts(
    const EnemyDef&       _def,
    const EnemyRenderEnv& _render) const
{
    const EnemyState& _e = GetState();
    if (_e.life == eEnemyLife::Dying)
    {
        const BusConfig& config = _def.bus;
        if (_e.deathTicks < config.wreckAfterTicks)
        {
            SubmitDamaged_(_render, config, BodyLevel_(BusConfig::kStageCount), _render.feet, false);
        }
        else
        {
            SubmitPart_(_render, config.partWreck, "Bus_Wreck", _render.feet, 0, false);
        }
        SubmitDebris_(_def, _render);
        return;
    }
    SubmitLive_(_def, _render);
}

bool BusEnemy::DeathDone(
    const EnemyDef& _def) const
{
    const EnemyState& _e = GetState();
    return _e.deathTicks >= _def.bus.destroyDoneTicks;
}

namespace
{
bool ReadNumField_(
    const JsonReader& _o,
    const StringView  _key,
    float&            _out)
{
    StringView reason;
    return ReadFloatField(_o, _key, false, _out, reason);
}

bool ReadIntField_(
    const JsonReader& _o,
    const StringView  _key,
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
    const StringView  _key,
    String&           _out)
{
    StringView reason;
    return ReadStringField(_o, _key, true, _out, reason);
}

bool ReadVecField_(
    const JsonReader& _o,
    const StringView  _key,
    VECTOR2&          _out)
{
    StringView reason;
    return ReadVec2Field(_o, _key, false, _out, reason);
}

void ParseBursts_(
    DefReader&           _r,
    const StringView     _key,
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
        if (!ReadStrField_(item, "clip", w.clip) || !ReadIntField_(item, "stage", w.stage) || w.stage < 1 || w.stage > BusConfig::kStageCount)
        {
            _r.Fail("params.windows", "clip 과 stage (1 이상 7 이하) 항목이 잘못되었습니다");
            return;
        }
        list.push_back(w);
    }
    _out = std::move(list);
}
}   // namespace

void ParseBusConfig(
    DefReader& _r,
    EnemyDef&  _def)
{
    BusConfig& config = _def.bus;

    config.openViewDx       = _r.Num("openViewDx", config.openViewDx);
    config.openDelayTicks   = _r.Ticks("openDelaySec", static_cast<float>(config.openDelayTicks) / 60.f);
    config.stairsDelayTicks = _r.Ticks("stairsDelaySec", static_cast<float>(config.stairsDelayTicks) / 60.f);
    config.wreckAfterTicks  = _r.Ticks("wreckAfterSec", static_cast<float>(config.wreckAfterTicks) / 60.f);
    config.destroyDoneTicks = _r.Ticks("destroyDoneSec", static_cast<float>(config.destroyDoneTicks) / 60.f);

    ARRAY<float, BusConfig::kStageCount> stageHp = {};
    if (_r.NumArray("stageHp", stageHp))
    {
        for (int i = 0; i < BusConfig::kStageCount; ++i)
        {
            config.stageHp[static_cast<size_t>(i)] = static_cast<int>(stageHp[static_cast<size_t>(i)]);
        }
    }

    config.pxPerSheetPx   = _r.Num("pxPerSheetPx", config.pxPerSheetPx);
    config.anchor         = _r.ReadVec2("anchor", config.anchor);
    config.debrisGroundDy = _r.Num("debrisGroundDy", config.debrisGroundDy);
    config.fxDepth        = _r.Int("fxDepth", config.fxDepth);
    ParseBursts_(_r, "bursts", config.bursts);
    ParseBursts_(_r, "deathBursts", config.deathBursts);
    ParseDebris_(_r, config.debris);
    ParseInnerSoldiers_(_r, config.innerSoldiers);
    ParseWindows_(_r, config.windows);

    if (_r.Has("soldierBombDef"))
    {
        config.soldierBombDef = _r.Str("soldierBombDef");
    }
    if (_r.Has("soldierSwordDef"))
    {
        config.soldierSwordDef = _r.Str("soldierSwordDef");
    }
    if (_r.Has("soldierBzkDef"))
    {
        config.soldierBzkDef = _r.Str("soldierBzkDef");
    }
    config.soldierSpawn      = _r.ReadVec2("soldierSpawn", config.soldierSpawn);
    config.stairsBase        = _r.ReadVec2("stairsBase", config.stairsBase);
    config.stairsTop         = _r.ReadVec2("stairsTop", config.stairsTop);
    config.landingW          = _r.Num("landingW", config.landingW);
    config.soldierFirstTicks = _r.Ticks("soldierFirstSec", static_cast<float>(config.soldierFirstTicks) / 60.f);
    config.soldierGapTicks   = _r.Ticks("soldierGapSec", static_cast<float>(config.soldierGapTicks) / 60.f);
    config.soldierMax        = _r.Int("soldierMax", config.soldierMax);
    config.swordPercent      = _r.Int("swordPercent", config.swordPercent);
    config.bazookaPercent    = _r.Int("bazookaPercent", config.bazookaPercent);
    config.soldierLastStage  = _r.Int("soldierLastStage", config.soldierLastStage);
    config.roofTop           = _r.ReadVec2("roofTop", config.roofTop);
    config.roofSpan          = _r.ReadVec2("roofSpan", config.roofSpan);
    config.coverMaxX         = _r.Num("coverMaxX", config.coverMaxX);
}

void BusEnemy::EnterStage_(
    const EnemyDef& _def,
    EnemyEnv&       _env,
    const int       _stage)
{
    EnemyState& _e                                 = GetMutableState();
    m_stageStartTicks[static_cast<size_t>(_stage)] = m_elapsedTicks;
    SpawnBursts_(_env, _e, _def, _stage, false);
}

void BusEnemy::SpawnStairSoldier_(
    const EnemyDef& _def,
    EnemyEnv&       _env)
{
    EnemyState&      _e       = GetMutableState();
    const BusConfig& config   = _def.bus;
    const int        roll     = Roll(_env, 100);
    const String*    pDefName = &config.soldierBombDef;
    int              variant  = kVariantClimbStay;
    if (roll < config.swordPercent)
    {
        pDefName = &config.soldierSwordDef;
        variant  = kVariantClimb;
    }
    else if (roll < config.swordPercent + config.bazookaPercent)
    {
        pDefName = &config.soldierBzkDef;
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
    params.arena.min  = { feet.x + config.stairsBase.x, feet.y + config.stairsBase.y };
    params.arena.max  = { feet.x + config.stairsTop.x, feet.y + config.stairsTop.y };
    if (config.roofTop.y != 0.f)
    {
        params.climbEnd = { feet.x + config.roofTop.x, feet.y + config.roofTop.y };
        params.roofSpan = { feet.x + config.roofSpan.x, feet.x + config.roofSpan.y };
    }
    (void)_env.world.SpawnDeferred(defIndex, { feet.x + config.soldierSpawn.x, feet.y + config.soldierSpawn.y }, params);
}

void BusEnemy::TickSoldiers_(
    const EnemyDef& _def,
    EnemyEnv&       _env)
{
    const BusConfig& config    = _def.bus;
    const int        readyTick = config.openDelayTicks + config.stairsDelayTicks + kFlyTicks + kLandTicks;
    if (m_damageStage > config.soldierLastStage || m_elapsedTicks < readyTick + config.soldierFirstTicks || m_elapsedTicks < m_nextSoldierTick)
    {
        return;
    }
    EnemyFilter filter;
    filter.userTag = kStairSoldierTag;
    if (_env.world.CountAlive(filter) >= config.soldierMax)
    {
        return;
    }
    SpawnStairSoldier_(_def, _env);
    m_nextSoldierTick = m_elapsedTicks + config.soldierGapTicks;
}

void BusEnemy::UpdateStage_(
    const EnemyDef& _def,
    EnemyEnv&       _env)
{
    EnemyState&      _e     = GetMutableState();
    const BusConfig& config = _def.bus;
    while (m_damageStage < BusConfig::kStageCount && _e.health.hp <= config.stageHp[static_cast<size_t>(m_damageStage)])
    {
        ++m_damageStage;
        EnterStage_(_def, _env, m_damageStage);
    }
}

void BusEnemy::SubmitDebris_(
    const EnemyDef&       _def,
    const EnemyRenderEnv& _render) const
{
    const EnemyState& _e     = GetState();
    const BusConfig&  config = _def.bus;
    const float       t      = static_cast<float>(_e.deathTicks) / 60.f;
    for (const BusDebrisDef& d: config.debris)
    {
        const VECTOR2 start   = Local_(config, _render.feet, d.at);
        const float   groundY = _render.feet.y + config.debrisGroundDy;
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
        SubmitPart_(_render, config.partDebris, d.clip, at, ticks, true);
    }
}

void BusEnemy::SubmitLive_(
    const EnemyDef&       _def,
    const EnemyRenderEnv& _render) const
{
    const EnemyState& _e     = GetState();
    const BusConfig&  config = _def.bus;
    const VECTOR2     feet   = _render.feet;
    const int         L      = m_elapsedTicks;
    const int         stage  = m_damageStage;

    for (const BusInnerSoldierDef& s: config.innerSoldiers)
    {
        if (stage >= s.stage)
        {
            SubmitPart_(_render, config.partInnerSoldier, "SD_Idle_Left", Local_(config, feet, s.at), _e.stateTicks + s.phase * 6, true);
        }
    }

    if (stage == 0)
    {
        if (static_cast<eBusState>(_e.state) == eBusState::Parked || L < config.openDelayTicks)
        {
            SubmitPart_(_render, config.partBody, "Bus_Idle", feet, 0, false);
            return;
        }
        const int openT   = L - config.openDelayTicks;
        const int stairsT = openT - config.stairsDelayTicks;
        if (stairsT >= 0)
        {
            if (stairsT < kFlyTicks)
            {
                SubmitPart_(_render, config.partStairs, "Bus_StairsFly", feet, stairsT, false);
            }
            else if (stairsT < kFlyTicks + kLandTicks)
            {
                SubmitPart_(_render, config.partStairs, "Bus_StairsLand", feet, stairsT - kFlyTicks, false);
            }
            else
            {
                SubmitPart_(_render, config.partStairs, "Bus_StairsRail", feet, stairsT - kFlyTicks - kLandTicks, false);
            }
        }
        SubmitPart_(_render, config.partBody, "Bus_OpenBody", feet, Min(openT, kPersist), false);
        SubmitPart_(_render, config.partCover, "Bus_OpenTop", feet, Min(openT, kPersist), false);
        return;
    }

    SubmitDamaged_(_render, config, BodyLevel_(stage), feet, true);

    for (const BusWindowDef& w: config.windows)
    {
        if (stage < w.stage)
        {
            continue;
        }
        const int t = L - m_stageStartTicks[static_cast<size_t>(w.stage)];
        if (t < GetNamedPartTicks(_render.anims, w.clip))
        {
            SubmitPart_(_render, { config.partCover.depth, config.partWindow.sort }, w.clip, feet, t, false);
        }
    }
}

void BusEnemy::HashActionState(Fnv1a& _hash) const
{
    HashActionState_(_hash, 0, m_elapsedTicks, m_damageStage);
}
