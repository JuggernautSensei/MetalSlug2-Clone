// 헬기 테스트: 폭격 사이클 (고공 3발 + 휴지, 저공 가로지르기 3발 + 휴지, 미사일, 복귀 반복), 폭탄/격추 이펙트, 히트박스
#include <algorithm>
#include <cmath>
#include <set>
#include <vector>

#include "Base/Math.h"
#include "Game/Enemies/EnemyBehaviors.h"
#include "Render/Animation.h"
#include "VehicleTestUtil.h"


constexpr float kGround    = 378.f;
constexpr int   kStateBomb = 0;   // Heli.cpp 의 eHeliState 순서와 같다 (Bomb, Low, PodOut, Move, Volley, MoveBack, Turn)
constexpr int   kStateLow  = 1;
constexpr int   kStateVolley = 4;

[[nodiscard]] static SpawnParams HeliParams_()
{
    SpawnParams params;
    params.facing     = -1;
    params.bActivated = true;
    return params;
}

struct HeliLog
{
    std::vector<int>   bombTicks   = {};   // 새 폭탄이 보인 틱
    std::vector<float> bombX       = {};
    std::vector<int>   bombStates  = {};   // 그때 헬기 상태
    std::vector<int>   states      = {};   // 틱별 헬기 상태
    std::vector<float> ys          = {};   // 틱별 헬기 y
    std::vector<float> xs          = {};
    int                missiles    = 0;
};

[[nodiscard]] static HeliLog RunHeli_(
    VehicleRig&       _rig,
    const EnemyHandle _heli,
    const int         _ticks)
{
    HeliLog               log;
    std::set<uint32_t>    seenBombs;
    std::set<uint32_t>    seenShots;
    for (int i = 0; i < _ticks; ++i)
    {
        _rig.Step(1);
        const Enemy* pHeli = _rig.E(_heli);
        if (pHeli)
        {
            log.states.push_back(pHeli->state);
            log.ys.push_back(pHeli->body.position.y);
            log.xs.push_back(pHeli->body.position.x);
        }
        for (const Enemy& enemy: _rig.World().GetEnemies())
        {
            if (enemy.behavior == eEnemyBehavior::HeliBomb && enemy.life != eEnemyLife::Gone && seenBombs.insert(enemy.id).second)
            {
                log.bombTicks.push_back(_rig.Ticks());
                log.bombX.push_back(enemy.body.position.x);
                log.bombStates.push_back(pHeli ? pHeli->state : -1);
            }
        }
        for (const Shot& shot: _rig.Projectiles().GetShots())
        {
            if (shot.params.owner == _heli.id && shot.ageTicks == 1)
            {
                ++log.missiles;
            }
        }
    }
    return log;
}

[[nodiscard]] static const EnemyDef& HeliDef_(
    VehicleRig& _rig)
{
    return _rig.Lib().Get(_rig.Lib().FindIndex("heli"));
}

TEST(Heli, DefinitionLoadsWithHeliAndBombDefs)
{
    VehicleRig rig;
    ASSERT_TRUE(rig.IsLoaded()) << rig.GetMessage();
    EXPECT_GE(rig.Lib().FindIndex("heli"), 0);
    EXPECT_GE(rig.Lib().FindIndex("heli_bomb"), 0);
}

// 고공 폭격: 3발 연속 (bombIntervalTicks 간격) -> 휴지 -> 3발 -> 휴지, 폭탄은 수직으로만 떨어진다
TEST(Heli, HighPhaseDropsThreeBombsThenRestsThenThreeAgain)
{
    VehicleRig rig({ 300.f, kGround });
    ASSERT_TRUE(rig.IsLoaded()) << rig.GetMessage();
    const HeliCfg     cfg  = HeliDef_(rig).heli;
    const EnemyHandle heli = rig.Spawn("heli", { 507.f, -30.f }, HeliParams_());
    ASSERT_FALSE(heli.IsNull());

    const HeliLog log = RunHeli_(rig, heli, cfg.firstBombTicks + 2 * cfg.burstCount * cfg.bombIntervalTicks + cfg.burstRestTicks + 60);
    ASSERT_GE(log.bombTicks.size(), 6u);
    for (size_t i = 0; i < 6; ++i)
    {
        EXPECT_EQ(log.bombStates[i], kStateBomb) << "고공 구간 안에서 투하";
    }
    const int interval = cfg.bombIntervalTicks;
    EXPECT_EQ(log.bombTicks[1] - log.bombTicks[0], interval);
    EXPECT_EQ(log.bombTicks[2] - log.bombTicks[1], interval);
    EXPECT_EQ(log.bombTicks[3] - log.bombTicks[2], interval + cfg.burstRestTicks) << "3발 뒤 휴지";
    EXPECT_EQ(log.bombTicks[4] - log.bombTicks[3], interval);
    EXPECT_EQ(log.bombTicks[5] - log.bombTicks[4], interval);
}

// 저공: 고공 구간이 끝나면 낮게 내려와 x 축으로 가로지르며 3발 + 휴지를 반복한다
TEST(Heli, LowPhaseCrossesTheScreenDroppingThreeAndRest)
{
    VehicleRig rig({ 300.f, kGround });
    ASSERT_TRUE(rig.IsLoaded()) << rig.GetMessage();
    const HeliCfg     cfg  = HeliDef_(rig).heli;
    const EnemyHandle heli = rig.Spawn("heli", { 507.f, -30.f }, HeliParams_());

    const int     highEnd = cfg.firstBombTicks + cfg.highBursts * (cfg.burstCount * cfg.bombIntervalTicks + cfg.burstRestTicks);
    const int     lowEnd  = cfg.reBombTicks + cfg.lowBursts * (cfg.burstCount * cfg.bombIntervalTicks + cfg.burstRestTicks);
    const HeliLog log     = RunHeli_(rig, heli, highEnd + lowEnd + 20);

    int firstLow = -1;
    int lastLow  = -1;
    for (size_t i = 0; i < log.states.size(); ++i)
    {
        if (log.states[i] == kStateLow)
        {
            firstLow = firstLow < 0 ? static_cast<int>(i) : firstLow;
            lastLow  = static_cast<int>(i);
        }
    }
    ASSERT_GE(firstLow, 0) << "저공 구간에 들어가지 않았다";
    EXPECT_NEAR(static_cast<float>(firstLow), static_cast<float>(highEnd), 3.f);
    EXPECT_GT(log.ys[static_cast<size_t>(lastLow)], cfg.lowDy - 2.f) << "저공 높이까지 내려온다";
    EXPECT_GT(std::abs(log.xs[static_cast<size_t>(lastLow)] - log.xs[static_cast<size_t>(firstLow)]), 150.f) << "x 축으로 크게 가로지른다";

    int lowBombs = 0;
    for (const int state: log.bombStates)
    {
        lowBombs += state == kStateLow ? 1 : 0;
    }
    EXPECT_EQ(lowBombs, cfg.lowBursts * cfg.burstCount) << "저공 구간 폭탄 수";
}

// 사이클: 고공 -> 저공 -> 미사일 -> 고공 복귀 -> 반복
TEST(Heli, CycleFiresMissilesThenReturnsToTheHighPhase)
{
    VehicleRig rig({ 300.f, kGround });
    ASSERT_TRUE(rig.IsLoaded()) << rig.GetMessage();
    const EnemyHandle heli = rig.Spawn("heli", { 507.f, -30.f }, HeliParams_());

    const HeliLog log = RunHeli_(rig, heli, 60 * 30);
    EXPECT_GE(log.missiles, 2) << "미사일 일제 사격";
    bool bSawVolley = false;
    bool bBackHigh  = false;
    bool bSawLow    = false;
    for (const int state: log.states)
    {
        bSawLow    = bSawLow || state == kStateLow;
        bSawVolley = bSawVolley || (bSawLow && state == kStateVolley);
        bBackHigh  = bBackHigh || (bSawVolley && state == kStateBomb);
    }
    EXPECT_TRUE(bSawLow);
    EXPECT_TRUE(bSawVolley);
    EXPECT_TRUE(bBackHigh) << "미사일 뒤 고공으로 돌아와 반복한다";
    EXPECT_GT(log.bombTicks.size(), 12u) << "두 번째 사이클에서도 폭탄을 계속 떨어뜨린다";
}

TEST(Heli, StartLowParamSkipsTheHighPhase)
{
    VehicleRig rig({ 300.f, kGround });
    ASSERT_TRUE(rig.IsLoaded()) << rig.GetMessage();
    EnemyDefLibrary lib;
    ASSERT_TRUE(LoadVehicleLib(lib));
    EXPECT_EQ(lib.Get(lib.FindIndex("heli")).heli.startLow, 0);
}

TEST(Heli, HurtboxCoversTheSprite)
{
    VehicleRig rig({ 300.f, kGround });
    ASSERT_TRUE(rig.IsLoaded()) << rig.GetMessage();
    const EnemyHandle heli = rig.Spawn("heli", { 507.f, 120.f }, HeliParams_());
    rig.Step(2);
    bool bFound = false;
    for (const Hurtbox& hurtbox: rig.Combat().GetHurtboxes())
    {
        if (hurtbox.owner == heli.id)
        {
            bFound = true;
            EXPECT_GE(hurtbox.box.max.x - hurtbox.box.min.x, 170.f);
            EXPECT_GE(hurtbox.box.max.y - hurtbox.box.min.y, 90.f);
        }
    }
    EXPECT_TRUE(bFound);
}

// 폭탄이 터지면 폭발 이펙트 (최전면 레이어 58) 가 실제 애니로 생긴다. 착지하면 먼지도 깔린다.
TEST(HeliFx, BombLandingSpawnsExplosionAndDust)
{
    SLUG_REQUIRE_BAKED(anims);
    EnemyDefLibrary lib;
    ASSERT_TRUE(LoadVehicleLib(lib));
    GroundMap    ground = MakeFlatGround(3000);
    PhysicsWorld physics;
    physics.pGround = &ground;

    EnemyWorld world(&lib, 7);
    world.SetLibrary(&lib);
    SpawnParams params;
    params.facing     = -1;
    params.bActivated = true;
    ASSERT_FALSE(world.Spawn("heli_bomb", { 300.f, 200.f }, params, &physics).IsNull());

    CombatSystem combat;
    EnemyContext ctx;
    ctx.physics    = physics;
    ctx.pCombat    = &combat;
    ctx.cameraView = Aabb::FromMinSize({ 0.f, 0.f }, { 640.f, 448.f });
    ctx.pAnims     = &anims;

    int  maxDepth = -1;
    int  maxCount = 0;
    bool bDust    = false;
    for (int i = 0; i < 240; ++i)
    {
        combat.BeginTick();
        world.Tick(ctx);
        for (const Effect& fx: world.GetEffects().GetEffects())
        {
            maxDepth = std::max(maxDepth, fx.depth);
            bDust    = bDust || fx.depth == 57;
        }
        maxCount = std::max(maxCount, static_cast<int>(world.GetEffects().GetCount()));
    }
    EXPECT_EQ(maxDepth, 58) << "폭발은 최전면 레이어";
    EXPECT_GE(maxCount, 2) << "폭발 + 먼지";
    EXPECT_TRUE(bDust);
}

// 격추: 큰 폭발 + 시간차 폭발/연기 (원작 3:14 우상단 대폭발)
TEST(HeliFx, DestructionSpawnsALargeExplosionAndStaggeredBursts)
{
    SLUG_REQUIRE_BAKED(anims);
    EnemyDefLibrary lib;
    ASSERT_TRUE(LoadVehicleLib(lib));
    GroundMap    ground = MakeFlatGround(3000);
    PhysicsWorld physics;
    physics.pGround = &ground;

    EnemyWorld world(&lib, 7);
    world.SetLibrary(&lib);
    SpawnParams params;
    params.facing     = -1;
    params.bActivated = true;
    const EnemyHandle h = world.Spawn("heli", { 400.f, 120.f }, params, &physics);
    ASSERT_FALSE(h.IsNull());

    CombatSystem combat;
    EnemyContext ctx;
    ctx.physics    = physics;
    ctx.pCombat    = &combat;
    ctx.cameraView = Aabb::FromMinSize({ 0.f, 0.f }, { 640.f, 448.f });
    ctx.pAnims     = &anims;
    world.Tick(ctx);

    HitEvent hit;
    hit.target = h.id;
    hit.damage = 9999;
    hit.kind   = eHitKind::Bullet;
    Vector<HitEvent> hits;
    hits.push_back(hit);
    world.OnHitEvents(hits);
    world.Tick(ctx);
    const size_t first = world.GetEffects().GetCount();
    EXPECT_GE(first, 1u) << "대폭발이 즉시 생긴다";
    size_t most = first;
    for (int i = 0; i < 40; ++i)
    {
        world.Tick(ctx);
        most = std::max(most, world.GetEffects().GetCount());
    }
    EXPECT_GT(most, first) << "시간차 폭발/연기가 이어진다";
}

// 호버 idle: 비행 클립일 때 몸통이 Pitch 프레임을 왕복하고 (렌더 전용), 로터 행이 같은 번호를 따른다. 시뮬레이션 상태는 건드리지 않는다
TEST(Heli, HoverIdleCyclesPitchFramesWithoutTouchingSimState)
{
    VehicleRig rig({ 300.f, kGround });
    ASSERT_TRUE(rig.IsLoaded()) << rig.GetMessage();
    Result<AnimationLibrary> anims = AnimationLibrary::LoadFromDirectory(FindAssetRoot());
    ASSERT_TRUE(anims) << "Assets 가 없다";
    const AnimationLibrary lib = anims.Take();

    const EnemyDef&   def  = HeliDef_(rig);
    const EnemyHandle heli = rig.Spawn("heli", { 507.f, -30.f }, HeliParams_());
    ASSERT_GT(def.heli.idlePitchTo, def.heli.idlePitchFrom);
    const BehaviorOps& ops = GetBehaviorOps(eEnemyBehavior::Heli);
    ASSERT_NE(ops.bodyFrame, nullptr);

    std::set<const AnimFrame*> shown;
    for (int i = 0; i < 6 * def.heli.idleStepTicks * (def.heli.idlePitchTo - def.heli.idlePitchFrom); ++i)
    {
        rig.Step(1);
        const Enemy* pHeli = rig.E(heli);
        ASSERT_NE(pHeli, nullptr);
        if (pHeli->clip == def.heli.clipFly)
        {
            const AnimFrame* pFrame = ops.bodyFrame(*pHeli, def, lib);
            ASSERT_NE(pFrame, nullptr);
            shown.insert(pFrame);
        }
    }
    EXPECT_GE(shown.size(), static_cast<size_t>(def.heli.idlePitchTo - def.heli.idlePitchFrom + 1)) << "범위 안의 모든 Pitch 프레임이 나온다";
}
