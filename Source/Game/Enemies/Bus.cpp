#include <algorithm>
#include <cmath>
#include <format>

#include "Base/Math.h"
#include "Game/Enemies/EnemyCommon.h"
#include "Game/Enemies/EnemyDefParse.h"
#include "Game/Enemies/VehicleCommon.h"

// 더블데커 버스 중간보스 (원작 MS2 Mission 4, Docs/Spec_Bus.md).
// 스폰 위치(앵커)에 처음부터 정지해 있다. 카메라 뷰 오른쪽 끝이 버스 왼쪽 끝에 닿으면 시간표대로 개방 -> 계단 전개 -> 계단 병사 출격 ->
// 체력 단계별 파손 -> 대폭발 -> 잔해 순으로 진행한다. 버스 자체는 공격하지 않는다 (병사만 공격). 개방 전에는 무적/비공격.
// 시각은 시트 33825 에서 자른 Bus_* 클립을 겹쳐 그린다 (뒤 조각 -> 본체 -> 창 파편). 앵커 = 바퀴 바닥 중앙.
// 좌표/폭발/파편/부품 깊이 표는 Assets/Defs/Bus.json (BusCfg).

// 상태: 개방 대기 (무적) -> 진행 (aux = 개방 트리거 이후 틱) -> 사망 (life == Dying)
enum class eBusState
{
    Parked,
    Active
};

constexpr int kOpenTicks = 36;   // 개방 9 프레임 x 4 틱
constexpr int kFlyTicks  = 32;   // 계단 비행 8 프레임 x 4 틱
constexpr int kLandTicks = 6;    // 계단 착지 3 프레임 x 2 틱
constexpr int kPersist   = 1 << 20;

// ext 슬롯 0..7 = 단계 진입 틱. 그 뒤는 병사 타이머.
constexpr int kExtNextSoldier = 8;    // 다음 병사를 낼 수 있는 틱

constexpr uint32_t kStairSoldierTag  = 0x42555300u;   // 계단 병사 (생존 수 세기)
constexpr int      kVariantClimb     = 2;             // Soldier.cpp: 계단 오른 뒤 일반 AI
constexpr int      kVariantClimbStay = 3;             // 계단 오른 뒤 승강대에서 제자리 투척

[[nodiscard]] static Vec2 Local_(
    const BusCfg& _c,
    const Vec2    _feet,
    const Vec2    _at)
{
    return { _feet.x + (_at.x - _c.anchor.x) * _c.pxPerSheetPx, _feet.y + (_at.y - _c.anchor.y) * _c.pxPerSheetPx };
}

static void SubmitPart_(
    const EnemyRenderEnv& _render,
    const EnemyPartDepth&   _depth,
    const StringView      _clip,
    const Vec2            _at,
    const int             _ticks,
    const bool            _bLoop)
{
    SubmitNamedPart(_render, _clip, _at, _ticks, _bLoop, _depth.sort, _depth.depth);
}

// 차체 단계 (0 = 개방 상태, 1..3 = 파손 D1..D3)
[[nodiscard]] static int BodyLevel_(
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

[[nodiscard]] static String Named_(
    const char* _base,
    const int   _level)
{
    return String(_base) + std::to_string(_level);
}

static void SpawnBursts_(
    EnemyEnv&       _env,
    const Enemy&    _e,
    const EnemyDef& _def,
    const int       _stage,
    const bool      _bDeath)
{
    const BusCfg& c    = _def.bus;
    const Vec2    feet = _e.body.position;
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

static void EnterStage_(
    Enemy&          _e,
    const EnemyDef& _def,
    EnemyEnv&       _env,
    const int       _stage)
{
    _e.ext[static_cast<size_t>(_stage)] = _e.aux;
    SpawnBursts_(_env, _e, _def, _stage, false);
}

static void Init_(
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

// 계단 병사 한 명을 버스 오른쪽 화면 밖에 낸다 (결정적 난수로 종류 선택).
// 대검병은 계단을 올라 지붕을 건너 내려오고, 폭탄병은 승강대에 서서 던지고, 바주카병도 계단을 올라 승강대(버스 위)에서 로켓을 쏜다 (Bzk.cpp Climb).
static void SpawnStairSoldier_(
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
    const Vec2  feet = _e.body.position;
    SpawnParams params;
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

static void TickSoldiers_(
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

// 체력이 단계 문턱 이하로 내려가면 단계를 올린다 (한 틱에 여러 단계 가능).
static void UpdateStage_(
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

static void Tick_(
    Enemy&          _e,
    const EnemyDef& _def,
    EnemyEnv&       _env)
{
    const BusCfg& c  = _def.bus;
    _e.body.velocity = { 0.f, 0.f };
    _e.facing        = -1;

    if (static_cast<eBusState>(_e.state) == eBusState::Parked)
    {
        // 개방 대기: 체력을 채워 피해를 무효화하고, 카메라 뷰 오른쪽 끝이 버스 왼쪽 끝에 닿으면 개방을 시작한다.
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

static void OnDeath_(
    Enemy&          _e,
    const EnemyDef& _def,
    EnemyEnv&       _env)
{
    _e.deathTicks = 0;
    SpawnBursts_(_env, _e, _def, 0, true);
}

static void TickDying_(
    Enemy&          _e,
    const EnemyDef& _def,
    EnemyEnv&       _env)
{
    (void)_def;
    (void)_env;
    _e.body.velocity = { 0.f, 0.f };
    ++_e.deathTicks;
}

static bool DeathDone_(
    const Enemy&    _e,
    const EnemyDef& _def)
{
    return _e.deathTicks >= _def.bus.destroyDoneTicks;
}

static void SubmitDebris_(
    const Enemy&          _e,
    const EnemyDef&       _def,
    const EnemyRenderEnv& _render)
{
    const BusCfg& c = _def.bus;
    const float   t = static_cast<float>(_e.deathTicks) / 60.f;
    for (const BusDebrisDef& d: c.debris)
    {
        const Vec2  start   = Local_(c, _render.feet, d.at);
        const float groundY = _render.feet.y + c.debrisGroundDy;
        float       tFly    = t;
        float       y       = start.y + d.velocity.y * t + 0.5f * kDebrisGravity * t * t;
        if (y > groundY)
        {
            // 지면에 닿은 시각을 구해 그 뒤로는 멈춘다
            const float disc = d.velocity.y * d.velocity.y + 2.f * kDebrisGravity * (groundY - start.y);
            tFly             = disc > 0.f ? (-d.velocity.y + std::sqrt(disc)) / kDebrisGravity : 0.f;
            y                = groundY;
        }
        const Vec2 at    = { start.x + d.velocity.x * tFly, y };
        const int  ticks = static_cast<int>(tFly * d.fps * 60.f / 12.f);   // fps 비율로 회전 속도를 바꾼다 (클립은 12 fps)
        SubmitPart_(_render, c.partDebris, d.clip, at, ticks, true);
    }
}

// 파손 단계 차체. 개방 단계와 같은 구조: 계단 (BackD 의 x >= coverMaxX) -> 몸통 (BodyD) -> 병사 -> 앞면 덮개 (BackD 의 x < coverMaxX = 개방 단계 Bus_OpenTop 에 해당).
// Bus_BackD* 는 이름과 달리 뒷벽이 아니라 앞면 덮개 + 계단 그림이다 (Bus_BodyD* 는 승강대 기둥/좌석까지 든 전체 몸통)
static void SubmitDamaged_(
    const EnemyRenderEnv& _render,
    const BusCfg&         _c,
    const int             _level,
    const Vec2            _feet,
    const bool            _bCover)
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
        cover.w         = std::max(1, std::min(cover.w, cover.pivotX + static_cast<int>(_c.coverMaxX)));
        SpriteDrawOptions options;
        options.flashAmount = _render.flash;
        _render.sprites.Submit(_c.partCover.depth + _render.zOffset, _c.partCover.sort, cover, _feet, options);
    }
}

static void SubmitLive_(
    const Enemy&          _e,
    const EnemyDef&       _def,
    const EnemyRenderEnv& _render)
{
    const BusCfg& c     = _def.bus;
    const Vec2    feet  = _render.feet;
    const int     L     = _e.aux;
    const int     stage = _e.aux2;

    // 창 안 병사: 차체보다 뒤
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
        SubmitPart_(_render, c.partBody, "Bus_OpenBody", feet, std::min(openT, kPersist), false);
        SubmitPart_(_render, c.partCover, "Bus_OpenTop", feet, std::min(openT, kPersist), false);
        return;
    }

    SubmitDamaged_(_render, c, BodyLevel_(stage), feet, true);

    // 창 파편 (한 번 재생하고 끝나면 그리지 않는다)
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

static void SubmitParts_(
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

const BehaviorOps kBusOps = {
    .init              = &Init_,
    .tick              = &Tick_,
    .onDeath           = &OnDeath_,
    .tickDying         = &TickDying_,
    .registerHurtboxes = nullptr,
    .submitParts       = &SubmitParts_,
    .deathDone         = &DeathDone_,
};

static bool ReadNumField_(
    const JsonReader& _o,
    const StringView  _key,
    float&            _out)
{
    if (!_o.HasField(_key))
    {
        return true;
    }
    const Result<double> value = _o.ReadField<double>(_key);
    if (!value)
    {
        return false;
    }
    _out = static_cast<float>(value.GetValue());
    return true;
}

static bool ReadIntField_(
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

static bool ReadStrField_(
    const JsonReader& _o,
    const StringView  _key,
    String&           _out)
{
    const Result<String> value = _o.ReadField<String>(_key);
    if (!value)
    {
        return false;
    }
    _out = value.GetValue();
    return true;
}

static bool ReadVecField_(
    const JsonReader& _o,
    const StringView  _key,
    Vec2&             _out)
{
    if (!_o.HasField(_key))
    {
        return true;
    }
    const Result<JsonReader> field = _o.FindField(_key);
    if (!field || !field->IsArray() || field->GetSize() != 2)
    {
        return false;
    }
    const Vector<JsonReader> items = field->GetArray();
    const Result<double>     x     = items[0].Read<double>();
    const Result<double>     y     = items[1].Read<double>();
    if (!x || !y)
    {
        return false;
    }
    _out = { static_cast<float>(x.GetValue()), static_cast<float>(y.GetValue()) };
    return true;
}

static void ParseBursts_(
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

static void ParseDebris_(
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

static void ParseInnerSoldiers_(
    DefReader&                   _r,
    Vector<BusInnerSoldierDef>&  _out)
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

static void ParseWindows_(
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
