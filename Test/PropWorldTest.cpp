#include <cmath>
#include <iterator>
#include <string>

#include "PropTestUtil.h"

using namespace slugtest;

namespace
{

constexpr float kDt = kStepSec;

float Width(const Aabb& _box)
{
    return _box.max.x - _box.min.x;
}

// 배치한 소품 하나를 기준으로 피격 결과를 한 번에 만든다 (판정 경로 없이 이벤트만 적용)
void HitN(PropWorld& _world, const PropHandle _id, const int _times, const int _damage, const eHitKind _kind = eHitKind::Bullet)
{
    for (int i = 0; i < _times; ++i)
    {
        const HitEvent ev = MakeHit(_id, _damage, _kind);
        _world.OnHitEvents(jug::Span<const HitEvent> { &ev, 1 });
    }
}

void TickN(PropWorld& _world, const int _ticks, const PropTickContext* _pCtx = nullptr)
{
    PropTickContext ctx;
    if (_pCtx != nullptr)
    {
        ctx = *_pCtx;
    }
    for (int i = 0; i < _ticks; ++i)
    {
        _world.Tick(ctx);
    }
}

bool HasEvent(const jug::Vector<PropEvent>& _events, const ePropEventKind _kind)
{
    for (const PropEvent& ev: _events)
    {
        if (ev.kind == _kind)
        {
            return true;
        }
    }
    return false;
}

int CountEvents(const jug::Vector<PropEvent>& _events, const ePropEventKind _kind)
{
    int n = 0;
    for (const PropEvent& ev: _events)
    {
        n += ev.kind == _kind ? 1 : 0;
    }
    return n;
}

// 판정 전체 경로: 소품 Hurtbox 등록 → Hitbox 1개 → Resolve → OnHitEvents
const jug::Vector<HitEvent>* ResolveHitbox(CombatSystem& _combat, PropWorld& _world, const Hitbox& _hitbox)
{
    _combat.BeginTick();
    _world.RegisterHurtboxes(_combat);
    _combat.AddHitbox(_hitbox);
    const jug::Vector<HitEvent>& events = _combat.Resolve();
    _world.OnHitEvents(events);
    return &events;
}

}   // namespace

// ===========================================
//  스폰 / 피격 단계
// ===========================================

TEST(PropWorld, UnknownDefSpawnsNothing)
{
    PropWorld world { &GetPropLib(), 1 };
    EXPECT_EQ(world.Spawn("no_such_def", { 0.f, 0.f }), kNoProp);
    EXPECT_EQ(world.GetCount(), 0u);
    EXPECT_TRUE(world.IsDestroyed(kNoProp));   // 없는 핸들은 파괴된 것으로 본다
}

TEST(PropWorld, ShopDestroyedAfterFifteenPistolHits)
{
    PropWorld world { &GetPropLib(), 1 };
    const PropHandle shop = world.Spawn("shop_0", { 1274.f, 228.f });
    ASSERT_NE(shop, kNoProp);
    EXPECT_GE(shop, kPropOwnerBase);   // 소품 owner id 범위

    HitN(world, shop, 14, 2);
    EXPECT_FALSE(world.IsDestroyed(shop));
    EXPECT_EQ(world.GetHp(shop), 2);

    HitN(world, shop, 1, 2);
    EXPECT_TRUE(world.IsDestroyed(shop));
    EXPECT_FALSE(world.IsDestroyFinished(shop));   // 파괴 연출 0.5 s 동안은 아직

    TickN(world, 31);
    EXPECT_TRUE(world.IsDestroyFinished(shop));
    EXPECT_EQ(world.GetCount(), 1u);   // 상점 잔해는 남는다 (remove 아님)
}

TEST(PropWorld, KnifeNeedsEightHitsOnTruck)
{
    PropWorld world { &GetPropLib(), 1 };
    const PropHandle truck = world.Spawn("truck", { 580.f, 342.f });
    ASSERT_NE(truck, kNoProp);
    HitN(world, truck, 7, 4, eHitKind::Melee);   // 칼 4 x7 = 28 (HP 30)
    EXPECT_FALSE(world.IsDestroyed(truck));
    HitN(world, truck, 1, 4, eHitKind::Melee);   // 8번째에서 파괴
    EXPECT_TRUE(world.IsDestroyed(truck));
}

TEST(PropWorld, BombTwoHitsOnTruck)
{
    PropWorld world { &GetPropLib(), 1 };
    const PropHandle truck = world.Spawn("truck", { 580.f, 342.f });
    HitN(world, truck, 1, 20, eHitKind::Blast);
    EXPECT_FALSE(world.IsDestroyed(truck));
    HitN(world, truck, 1, 20, eHitKind::Blast);
    EXPECT_TRUE(world.IsDestroyed(truck));
}

TEST(PropWorld, FenceDiesToOneHitAndIsRemovedAfterFinish)
{
    PropWorld world { &GetPropLib(), 1 };
    const PropHandle fence = world.Spawn("fence", { 3665.f, 342.f }, PropSpawnParams {});
    ASSERT_NE(fence, kNoProp);
    HitN(world, fence, 1, 2);
    EXPECT_TRUE(world.IsDestroyed(fence));
    TickN(world, 1);
    EXPECT_EQ(world.GetCount(), 0u);
    EXPECT_TRUE(world.IsDestroyFinished(fence));
}

TEST(PropWorld, ImmortalGuardPostTakesNoDamageAndHasNoHurtbox)
{
    PropWorld world { &GetPropLib(), 1 };
    const PropHandle post = world.Spawn("guard_post", { 5800.f, 541.f });
    CombatSystem combat;
    combat.BeginTick();
    world.RegisterHurtboxes(combat);
    EXPECT_TRUE(combat.GetHurtboxes().empty());
    HitN(world, post, 5, 100);
    EXPECT_FALSE(world.IsDestroyed(post));
    EXPECT_EQ(world.GetPlatforms().size(), 2u);   // 가드 포스트 발판 2개는 항상 있다
}

// ===========================================
//  판정 등록 / 실제 Hitbox 경로
// ===========================================

TEST(PropWorld, HurtboxesAreNeutralWithPropOwnerAndNoHazard)
{
    PropWorld world { &GetPropLib(), 1 };
    const PropHandle shop = world.Spawn("shop_0", { 1274.f, 228.f });
    CombatSystem combat;
    combat.BeginTick();
    world.RegisterHurtboxes(combat);
    ASSERT_EQ(combat.GetHurtboxes().size(), 1u);
    const Hurtbox& hb = combat.GetHurtboxes()[0];
    EXPECT_EQ(hb.team, eTeam::Neutral);
    EXPECT_EQ(hb.owner, shop);
    EXPECT_TRUE(hb.accepts.Has(eHitKind::Bullet));
    EXPECT_TRUE(hb.accepts.Has(eHitKind::Melee));
    EXPECT_TRUE(hb.accepts.Has(eHitKind::Blast));
    EXPECT_FALSE(hb.accepts.Has(eHitKind::Hazard));
    EXPECT_FLOAT_EQ(hb.box.min.x, 1274.f - 75.f);
    EXPECT_FLOAT_EQ(hb.box.max.y, 228.f - 100.f + 37.5f);
}

TEST(PropWorld, PlayerHitboxResolvesToPropEvent)
{
    PropWorld   world { &GetPropLib(), 1 };
    CombatSystem combat;
    const PropHandle shop = world.Spawn("shop_0", { 1274.f, 228.f });

    Hitbox hb;
    hb.box   = Aabb::FromCenterSize({ 1274.f, 128.f }, { 10.f, 10.f });
    hb.team  = eTeam::Player;
    hb.kind  = eHitKind::Bullet;
    hb.damage = 2;
    hb.owner = 1;
    hb.id    = combat.NewHitId();
    const jug::Vector<HitEvent>* pEvents = ResolveHitbox(combat, world, hb);

    ASSERT_EQ(pEvents->size(), 1u);
    EXPECT_EQ((*pEvents)[0].target, shop);
    EXPECT_EQ(world.GetHp(shop), 28);
    const jug::Vector<PropEvent> events = world.TakeEvents();
    EXPECT_TRUE(HasEvent(events, ePropEventKind::Damaged));
}

TEST(PropWorld, EnemyHitboxDoesNotHitProps)
{
    PropWorld    world { &GetPropLib(), 1 };
    CombatSystem combat;
    const PropHandle shop = world.Spawn("shop_0", { 1274.f, 228.f });

    Hitbox hb;
    hb.box    = Aabb::FromCenterSize({ 1274.f, 128.f }, { 10.f, 10.f });
    hb.team   = eTeam::Enemy;
    hb.kind   = eHitKind::Bullet;
    hb.damage = 99;
    hb.owner  = 9;
    hb.id     = combat.NewHitId();
    const jug::Vector<HitEvent>* pEvents = ResolveHitbox(combat, world, hb);
    EXPECT_TRUE(pEvents->empty());
    EXPECT_EQ(world.GetHp(shop), 30);
}

TEST(PropWorld, ParamHitboxOverrideChangesHurtbox)
{
    PropWorld world { &GetPropLib(), 1 };
    PropSpawnParams params;
    params.hitbox = PropBoxOverride { { 50.f, 50.f }, { 0.f, 0.f } };
    params.hp     = 15;
    const PropHandle shop = world.Spawn("shop_0", { 100.f, 100.f }, params);
    EXPECT_EQ(world.GetHp(shop), 15);
    CombatSystem combat;
    combat.BeginTick();
    world.RegisterHurtboxes(combat);
    ASSERT_EQ(combat.GetHurtboxes().size(), 1u);
    EXPECT_FLOAT_EQ(Width(combat.GetHurtboxes()[0].box), 50.f);
}

// ===========================================
//  정적 충돌 출력
// ===========================================

TEST(PropWorld, ShopStandBoxIsOneWayPlatformAndGoesAwayOnDestroy)
{
    PropWorld world { &GetPropLib(), 1 };
    const PropHandle shop = world.Spawn("shop_0", { 200.f, 100.f });   // standBox 220x20 @(30,10) → 중심 (230,110), 윗면 y=100
    ASSERT_EQ(world.GetPlatforms().size(), 1u);
    const Platform& p = world.GetPlatforms()[0];
    EXPECT_TRUE(p.bOneWay);
    EXPECT_FLOAT_EQ(p.box.min.y, 100.f);
    EXPECT_TRUE(world.GetEnemyPlatforms().empty());

    HitN(world, shop, 15, 2);
    TickN(world, 1);
    EXPECT_TRUE(world.GetPlatforms().empty());
}

TEST(PropWorld, ShopFiveReplacesStandBoxWhenDestroyed)
{
    PropWorld world { &GetPropLib(), 1 };
    const PropHandle shop = world.Spawn("shop_5", { 2513.f, 181.f });
    ASSERT_EQ(world.GetPlatforms().size(), 1u);
    EXPECT_FLOAT_EQ(Width(world.GetPlatforms()[0].box), 230.f);

    HitN(world, shop, 1, 30);
    ASSERT_EQ(world.GetPlatforms().size(), 1u);
    EXPECT_FLOAT_EQ(Width(world.GetPlatforms()[0].box), 220.f);
    EXPECT_FLOAT_EQ(world.GetPlatforms()[0].box.GetCenter().y, 181.f + 130.f);
}

TEST(PropWorld, HouseRoofOnlyBlocksEnemies)
{
    PropWorld world { &GetPropLib(), 1 };
    const PropHandle house = world.Spawn("house_upper", { 7189.f, 717.f });
    EXPECT_TRUE(world.GetPlatforms().empty());
    ASSERT_EQ(world.GetEnemyPlatforms().size(), 1u);
    EXPECT_FLOAT_EQ(world.GetEnemyPlatforms()[0].box.min.y, 717.f + 3.f - 15.f);

    HitN(world, house, 10, 2);   // HP 20 → 파괴
    TickN(world, 1);
    EXPECT_TRUE(world.GetEnemyPlatforms().empty());   // 지붕이 사라져 위의 적이 떨어진다
}

TEST(PropWorld, BridgeBlockWallShrinksOnDestroy)
{
    PropWorld world { &GetPropLib(), 1 };
    const PropHandle block = world.Spawn("bridge_block", { 10098.f, 1274.f });
    ASSERT_EQ(world.GetWalls().size(), 1u);
    EXPECT_FLOAT_EQ(Width(world.GetWalls()[0].box), 135.f);
    HitN(world, block, 10, 2);
    ASSERT_EQ(world.GetWalls().size(), 1u);
    EXPECT_FLOAT_EQ(Width(world.GetWalls()[0].box), 72.f);
    EXPECT_FLOAT_EQ(world.GetWalls()[0].box.GetCenter().x, 10098.f + 30.f);
}

// ===========================================
//  물리: 발판 위에 서고, 벽에 막힌다
// ===========================================

TEST(PropWorld, PlayerLandsOnOneWayShopPlatform)
{
    PropWorld world { &GetPropLib(), 1 };
    world.Spawn("shop_0", { 200.f, 100.f });   // 윗면 y = 100
    const GroundMap  ground = MakeFloorMap(400, 160, 140);
    const PhysicsWorld pw { &ground, world.GetPlatforms(), world.GetWalls() };

    Body body = MakeBody({ 230.f, 0.f });
    for (int i = 0; i < 180; ++i)
    {
        (void)Physics::Step(body, pw, PhysicsTuning {}, kDt);
    }
    EXPECT_TRUE(body.bGrounded);
    EXPECT_NEAR(body.position.y, 100.f, 0.5f);
}

TEST(PropWorld, PlayerIsBlockedByBridgeWall)
{
    PropWorld world { &GetPropLib(), 1 };
    world.Spawn("bridge_house", { 200.f, 90.f });   // 벽 150x100 → x [125,275], y [40,140]
    const GroundMap    ground = MakeFloorMap(400, 200, 160);
    const PhysicsWorld pw { &ground, world.GetPlatforms(), world.GetWalls() };

    Body body = MakeBody({ 50.f, 160.f });
    bool  bHitWall = false;
    for (int i = 0; i < 200; ++i)
    {
        body.velocity.x = 120.f;
        const MoveResult r = Physics::Step(body, pw, PhysicsTuning {}, kDt);
        bHitWall           = bHitWall || r.bHitWallRight;
    }
    EXPECT_TRUE(bHitWall);
    EXPECT_LT(body.position.x, 125.f);
    EXPECT_GT(body.position.x, 100.f);
}

// ===========================================
//  드롭 / 2차 폭발 / 낙하
// ===========================================

TEST(PropWorld, MobBoatDropsSixItemsAndBurstsOnDeath)
{
    PropWorld world { &GetPropLib(), 3 };
    const PropHandle boat = world.Spawn("mob_boat", { 4140.f, 545.f });
    HitN(world, boat, 1, 60);
    EXPECT_TRUE(world.IsDestroyed(boat));

    const jug::Vector<PropEvent> events = world.TakeEvents();
    EXPECT_EQ(CountEvents(events, ePropEventKind::DropSpawned), 6);
    EXPECT_EQ(world.GetItems().GetCount(), 6u);
    EXPECT_EQ(world.GetDebris().size(), 3u);   // onDestroy 목재 3
}

TEST(PropWorld, ExplodeOnDeathBoatBurstsAfterTwoSecondsThenRemoved)
{
    PropWorld world { &GetPropLib(), 3 };
    PropSpawnParams params;
    params.bExplodeOnDeath = true;
    const PropHandle boat = world.Spawn("mob_boat", { 4140.f, 545.f }, params);
    HitN(world, boat, 1, 60);
    (void)world.TakeEvents();

    TickN(world, 110);
    EXPECT_EQ(world.GetCount(), 1u);   // 아직 2 s 전
    TickN(world, 15);
    EXPECT_EQ(world.GetCount(), 0u);   // 2차 폭발 후 제거
    const jug::Vector<PropEvent> events = world.TakeEvents();
    EXPECT_TRUE(HasEvent(events, ePropEventKind::SecondaryBurst));
    EXPECT_TRUE(HasEvent(events, ePropEventKind::Removed));
    EXPECT_EQ(world.GetDebris().size(), 8u);   // 3 + 5 (5 s 수명 이내)
}

TEST(PropWorld, PlainBoatLeavesWreckWithoutSecondaryBurst)
{
    PropWorld world { &GetPropLib(), 3 };
    const PropHandle boat = world.Spawn("mob_boat", { 4634.f, 545.f });
    HitN(world, boat, 1, 60);
    TickN(world, 300);
    EXPECT_EQ(world.GetCount(), 1u);
    EXPECT_TRUE(world.IsDestroyFinished(boat));
    EXPECT_FALSE(HasEvent(world.TakeEvents(), ePropEventKind::SecondaryBurst));
}

TEST(PropWorld, ShopFiveSpawnsSignThatFallsToGroundAndDetonates)
{
    PropWorld world { &GetPropLib(), 4 };
    const PropHandle shop = world.Spawn("shop_5", { 2513.f, 181.f });
    HitN(world, shop, 1, 30);
    EXPECT_EQ(world.GetCount(), 2u);   // 상점 + 낙하 간판

    const GroundMap   ground = MakeFloorMap(3000, 500, 400);
    PropTickContext   ctx;
    ctx.pGround = &ground;
    bool bRemoved = false;
    for (int i = 0; i < 400 && !bRemoved; ++i)
    {
        world.Tick(ctx);
        bRemoved = HasEvent(world.TakeEvents(), ePropEventKind::Removed);
    }
    EXPECT_TRUE(bRemoved);
    EXPECT_EQ(world.GetCount(), 1u);
    EXPECT_EQ(world.GetDebris().size(), 12u);   // 상점 6 + 간판 6 (5 s 이내 폭발)
}

TEST(PropWorld, GroundHouseDamageStagesAndGroundPatch)
{
    PropWorld world { &GetPropLib(), 5 };
    const jug::VECTOR2 center { 9830.f, 1269.f };
    const PropHandle   house = world.Spawn("ground_house", center);

    HitN(world, house, 10, 2);   // 20 dmg → HP 20 (damagedAtHp)
    EXPECT_EQ(world.GetHp(house), 20);
    EXPECT_EQ(world.GetDebris().size(), 5u);   // onDamaged 목재 5
    TickN(world, 1);
    EXPECT_EQ(world.Get(house)->visual, ePropVisual::Damaged);

    HitN(world, house, 10, 2);
    EXPECT_TRUE(world.IsDestroyed(house));
    const jug::Vector<PropEvent> events = world.TakeEvents();
    bool bPatch = false;
    for (const PropEvent& ev: events)
    {
        if (ev.kind == ePropEventKind::GroundPatch)
        {
            bPatch = true;
            EXPECT_EQ(ev.name.c_str(), std::string { "Stage0_House_4_PixelMap" });
            EXPECT_FLOAT_EQ(ev.pos.x, center.x - (653.f - 12.f));
            EXPECT_FLOAT_EQ(ev.pos.y, center.y - (375.5f - 5.f));
        }
    }
    EXPECT_TRUE(bPatch);
    EXPECT_EQ(CountEvents(events, ePropEventKind::Damaged), 20);
}

TEST(PropWorld, ApplyGroundPatchWritesOnlyPatchRegion)
{
    GroundMap map = GroundMap::MakeEmpty(10, 10);
    const GroundMap patch = GroundMap::MakeFromRows(std::array<jug::StringView, 3> { "###", "###", "###" });
    ApplyGroundPatch(map, { 2.f, 2.f }, patch);
    EXPECT_TRUE(map.IsSolid(2, 2));
    EXPECT_TRUE(map.IsSolid(4, 4));
    EXPECT_FALSE(map.IsSolid(1, 2));
    EXPECT_FALSE(map.IsSolid(5, 5));
    EXPECT_EQ(map.CountSolidPixels(), 9u);
}

// ===========================================
//  변종 / 결정성 / 컬링 / 정리
// ===========================================

TEST(PropWorld, TruckVariantPositionShiftMatchesLegacyFinalInit)
{
    PropWorld world { &GetPropLib(), 1 };
    PropSpawnParams params;
    params.variant = 1;   // loaded: dy -10
    const PropHandle truck = world.Spawn("truck", { 580.f, 342.f }, params);
    EXPECT_FLOAT_EQ(world.Get(truck)->pos.y, 332.f);
    EXPECT_EQ(world.Get(truck)->variant, 1);
    EXPECT_EQ(world.GetWalls().size(), 2u);
}

TEST(PropWorld, SameSeedGivesSameVariantsDropsAndHash)
{
    auto run = [](const uint64_t _seed)
    {
        PropWorld world { &GetPropLib(), _seed };
        for (int i = 0; i < 6; ++i)
        {
            world.Spawn("truck", { 580.f + 100.f * static_cast<float>(i), 342.f });
        }
        const PropHandle boat = world.Spawn("mob_boat", { 4140.f, 545.f });
        HitN(world, boat, 1, 60);
        TickN(world, 120);
        return world.HashState();
    };
    EXPECT_EQ(run(42), run(42));
    EXPECT_NE(run(42), run(43));
}

TEST(PropWorld, CullingRemovesPropsBehindCameraWithEvents)
{
    PropWorld world { &GetPropLib(), 1 };
    world.Spawn("shop_0", { 1274.f, 228.f });   // cullMargin 200
    world.Spawn("truck", { 1810.f, 342.f });

    PropTickContext ctx;
    ctx.cameraLeft = 1100.f;   // 1274 > 900 → 남는다
    TickN(world, 1, &ctx);
    EXPECT_EQ(world.GetCount(), 2u);

    ctx.cameraLeft = 1500.f;   // 1274 < 1300 → 제거, 트럭(1810)은 남는다
    world.Tick(ctx);
    EXPECT_EQ(world.GetCount(), 1u);
    const jug::Vector<PropEvent> events = world.TakeEvents();
    EXPECT_EQ(CountEvents(events, ePropEventKind::Culled), 1);
    EXPECT_EQ(CountEvents(events, ePropEventKind::Removed), 1);
}

TEST(PropWorld, ClearByLayerAndResetRestoreIds)
{
    PropWorld world { &GetPropLib(), 1 };
    world.Spawn("shop_0", { 1274.f, 228.f });
    world.Spawn("truck", { 580.f, 342.f });
    world.ClearByLayer(true);    // front 없음
    EXPECT_EQ(world.GetCount(), 2u);
    world.ClearByLayer(false);
    EXPECT_EQ(world.GetCount(), 0u);
    EXPECT_TRUE(world.GetWalls().empty());

    world.Reset(1);
    EXPECT_EQ(world.Spawn("shop_0", { 1274.f, 228.f }), kPropOwnerBase);
}

TEST(PropWorld, HashChangesWithDamageAndIsStableWithoutIt)
{
    PropWorld a { &GetPropLib(), 9 };
    PropWorld b { &GetPropLib(), 9 };
    a.Spawn("tower", { 10808.f, 1175.f });
    b.Spawn("tower", { 10808.f, 1175.f });
    EXPECT_EQ(a.HashState(), b.HashState());
    HitN(a, a.GetProps()[0].id, 1, 2);
    EXPECT_NE(a.HashState(), b.HashState());
}

TEST(PropWorld, SubmitQueuesSpritesOnLayersFromDefinitions)
{
    jug::Result<SpriteLibrary> spriteLib = SpriteLibrary::LoadFromJson(R"({"version":1,"sprites":{"TestBox":{"atlas":0,"x":0,"y":0,"w":10,"h":10,"pivotX":5,"pivotY":5}}})");
    ASSERT_TRUE(static_cast<bool>(spriteLib));
    jug::Result<PropDefLibrary> defs = PropDefLibrary::Parse(R"({ "version": 1, "defs": {
        "box":   { "hp": 4, "idle": { "sprite": "TestBox" }, "parts": [ { "role": "hurt", "size": [10, 10] } ] },
        "front": { "hp": 0, "layer": "front", "idle": { "sprite": "TestBox" } } } })");
    ASSERT_TRUE(static_cast<bool>(defs));

    const PropDefLibrary boxLib = defs.Take();
    PropWorld world { &boxLib, 1 };
    world.Spawn("box", { 10.f, 20.f });
    world.Spawn("front", { 30.f, 40.f });
    AnimationLibrary anims;
    const SpriteLibrary sprites = spriteLib.Take();
    SpriteBatch      batch;
    world.Submit(batch, anims, sprites);
    const jug::Vector<SpriteCommand>& cmds = batch.GetPendingCommands();
    ASSERT_EQ(cmds.size(), 2u);
    EXPECT_EQ(cmds[0].layer, kPropLayerBack);
    EXPECT_EQ(cmds[1].layer, kPropLayerFront);
    EXPECT_FLOAT_EQ(cmds[1].origin.x, 30.f);
}
