#include <algorithm>
#include <cmath>
#include <string>
#include <vector>

#include "Base/SlugError.h"
#include "Base/Types.h"
#include "Core/AssetPath.h"
#include "Game/Stage/StageData.h"
#include "PropTestUtil.h"

constexpr float kDt = kStepSec;

namespace
{
[[nodiscard]] float Width_(
    const Aabb& _box)
{
    return _box.max.x - _box.min.x;
}

void HitN_(
    PropWorld&       _world,
    const PropHandle _id,
    const int        _times,
    const int        _damage,
    const eHitKind   _kind = eHitKind::Bullet)
{
    for (int i = 0; i < _times; ++i)
    {
        const HitEvent ev = MakeHit(_id, _damage, _kind);
        _world.OnHitEvents(Span<const HitEvent> { &ev, 1 });
    }
}

void TickN_(
    PropWorld&             _world,
    const int              _ticks,
    const PropTickContext* _pCtx = nullptr)
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

[[nodiscard]] bool HasEvent_(
    const Vector<PropEvent>& _events,
    const ePropEventKind     _kind)
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

[[nodiscard]] int CountEvents_(
    const Vector<PropEvent>& _events,
    const ePropEventKind     _kind)
{
    int n = 0;
    for (const PropEvent& ev: _events)
    {
        n += ev.kind == _kind ? 1 : 0;
    }
    return n;
}

[[nodiscard]] const Vector<HitEvent>* ResolveHitbox_(
    CombatSystem& _combat,
    PropWorld&    _world,
    const Hitbox& _hitbox)
{
    _combat.BeginTick();
    _world.RegisterHurtboxes(_combat);
    _combat.AddHitbox(_hitbox);
    const Vector<HitEvent>& events = _combat.Resolve();
    _world.OnHitEvents(events);
    return &events;
}

[[nodiscard]] uint64_t HashAfterTrucks_(
    const uint64_t _seed)
{
    PropWorld world { &GetPropLib(), _seed };
    for (int i = 0; i < 6; ++i)
    {
        world.Spawn("truck", { 580.f + 100.f * static_cast<float>(i), 342.f });
    }
    const PropHandle boat = world.Spawn("mob_boat", { 4140.f, 545.f });
    HitN_(world, boat, 1, 60);
    TickN_(world, 120);
    return world.HashState();
}
}   // namespace

TEST(PropWorld, UnknownDefSpawnsNothing)
{
    PropWorld world { &GetPropLib(), 1 };
    EXPECT_EQ(world.Spawn("no_such_def", { 0.f, 0.f }), kNoProp);
    EXPECT_EQ(world.GetCount(), 0u);
    EXPECT_TRUE(world.IsDestroyed(kNoProp));
}

TEST(PropWorld, ShopDestroyedAfterFourPistolHits)
{
    PropWorld        world { &GetPropLib(), 1 };
    const PropHandle shop = world.Spawn("shop_0", { 1274.f, 228.f });
    ASSERT_NE(shop, kNoProp);
    EXPECT_GE(shop, kPropOwnerBase);

    HitN_(world, shop, 3, 3);
    EXPECT_FALSE(world.IsDestroyed(shop));
    EXPECT_EQ(world.GetHp(shop), 3);

    HitN_(world, shop, 1, 3);
    EXPECT_TRUE(world.IsDestroyed(shop));
    EXPECT_FALSE(world.IsDestroyFinished(shop));

    TickN_(world, 31);
    EXPECT_TRUE(world.IsDestroyFinished(shop));
    EXPECT_EQ(world.GetCount(), 1u);
}

TEST(PropWorld, KnifeDoesNotHitTrucks)
{
    PropWorld        world { &GetPropLib(), 1 };
    const PropHandle truck = world.Spawn("truck", { 580.f, 342.f });
    ASSERT_NE(truck, kNoProp);
    CombatSystem probe;
    probe.BeginTick();
    world.RegisterHurtboxes(probe);
    ASSERT_FALSE(probe.GetHurtboxes().empty());

    for (const Hurtbox& hb: probe.GetHurtboxes())
    {
        Hitbox knife;
        knife.box    = hb.box;
        knife.team   = eTeam::Player;
        knife.kind   = eHitKind::Melee;
        knife.damage = 8;
        knife.owner  = 1;
        knife.id     = 1;
        CombatSystem            combat;
        const Vector<HitEvent>* pEvents = ResolveHitbox_(combat, world, knife);
        EXPECT_TRUE(pEvents->empty());
    }
    EXPECT_FALSE(world.IsDestroyed(truck));
    EXPECT_EQ(world.GetHp(truck), 9);
}

TEST(PropWorld, PistolThreeShotsDestroyATruck)
{
    PropWorld        world { &GetPropLib(), 1 };
    const PropHandle truck = world.Spawn("truck", { 580.f, 342.f });
    HitN_(world, truck, 2, 3);
    EXPECT_FALSE(world.IsDestroyed(truck));
    HitN_(world, truck, 1, 3);
    EXPECT_TRUE(world.IsDestroyed(truck));
}

TEST(PropWorld, BombOneHitKillsATruckAndTwoHitsABigHouse)
{
    PropWorld        world { &GetPropLib(), 1 };
    const PropHandle truck = world.Spawn("truck", { 580.f, 342.f });
    HitN_(world, truck, 1, 20, eHitKind::Blast);
    EXPECT_TRUE(world.IsDestroyed(truck));

    const PropHandle house = world.Spawn("big_house", { 2000.f, 342.f });
    ASSERT_NE(house, kNoProp);
    HitN_(world, house, 1, 20, eHitKind::Blast);
    EXPECT_FALSE(world.IsDestroyed(house));
    HitN_(world, house, 1, 20, eHitKind::Blast);
    EXPECT_TRUE(world.IsDestroyed(house));
}

TEST(PropWorld, FenceDiesToOneHitAndIsRemovedAfterFinish)
{
    PropWorld        world { &GetPropLib(), 1 };
    const PropHandle fence = world.Spawn("fence", { 3665.f, 342.f }, PropSpawnParams {});
    ASSERT_NE(fence, kNoProp);
    HitN_(world, fence, 1, 2);
    EXPECT_TRUE(world.IsDestroyed(fence));
    TickN_(world, 1);
    EXPECT_EQ(world.GetCount(), 0u);
    EXPECT_TRUE(world.IsDestroyFinished(fence));
}

TEST(PropWorld, ImmortalGuardPostTakesNoDamageAndHasNoHurtbox)
{
    PropWorld        world { &GetPropLib(), 1 };
    const PropHandle post = world.Spawn("guard_post", { 5800.f, 541.f });
    CombatSystem     combat;
    combat.BeginTick();
    world.RegisterHurtboxes(combat);
    EXPECT_TRUE(combat.GetHurtboxes().empty());
    HitN_(world, post, 5, 100);
    EXPECT_FALSE(world.IsDestroyed(post));
    EXPECT_EQ(world.GetPlatforms().size(), 2u);
}

TEST(PropWorld, HurtboxesAreNeutralWithPropOwnerAndNoHazard)
{
    PropWorld        world { &GetPropLib(), 1 };
    const PropHandle shop = world.Spawn("shop_0", { 1274.f, 228.f });
    CombatSystem     combat;
    combat.BeginTick();
    world.RegisterHurtboxes(combat);
    ASSERT_EQ(combat.GetHurtboxes().size(), 1u);
    const Hurtbox& hb = combat.GetHurtboxes()[0];
    EXPECT_EQ(hb.team, eTeam::Neutral);
    EXPECT_EQ(hb.owner, shop);
    EXPECT_TRUE(hb.accepts.Has(eHitKind::Bullet));
    EXPECT_FALSE(hb.accepts.Has(eHitKind::Melee));
    EXPECT_TRUE(hb.accepts.Has(eHitKind::Blast));
    EXPECT_FALSE(hb.accepts.Has(eHitKind::Hazard));
    EXPECT_FLOAT_EQ(hb.box.min.x, 1274.f - 110.f);
    EXPECT_FLOAT_EQ(hb.box.max.y, 228.f - 50.f + 80.f);
}

TEST(PropWorld, PlayerHitboxResolvesToPropEvent)
{
    PropWorld        world { &GetPropLib(), 1 };
    CombatSystem     combat;
    const PropHandle shop = world.Spawn("shop_0", { 1274.f, 228.f });

    Hitbox hb;
    hb.box                          = Aabb::FromCenterSize({ 1274.f, 128.f }, { 10.f, 10.f });
    hb.team                         = eTeam::Player;
    hb.kind                         = eHitKind::Bullet;
    hb.damage                       = 2;
    hb.owner                        = 1;
    hb.id                           = combat.NewHitId();
    const Vector<HitEvent>* pEvents = ResolveHitbox_(combat, world, hb);

    ASSERT_EQ(pEvents->size(), 1u);
    EXPECT_EQ((*pEvents)[0].target, shop);
    EXPECT_EQ(world.GetHp(shop), 10);
    const Vector<PropEvent> events = world.TakeEvents();
    EXPECT_TRUE(HasEvent_(events, ePropEventKind::Damaged));
}

TEST(PropWorld, EnemyHitboxDoesNotHitProps)
{
    PropWorld        world { &GetPropLib(), 1 };
    CombatSystem     combat;
    const PropHandle shop = world.Spawn("shop_0", { 1274.f, 228.f });

    Hitbox hb;
    hb.box                          = Aabb::FromCenterSize({ 1274.f, 128.f }, { 10.f, 10.f });
    hb.team                         = eTeam::Enemy;
    hb.kind                         = eHitKind::Bullet;
    hb.damage                       = 99;
    hb.owner                        = 9;
    hb.id                           = combat.NewHitId();
    const Vector<HitEvent>* pEvents = ResolveHitbox_(combat, world, hb);
    EXPECT_TRUE(pEvents->empty());
    EXPECT_EQ(world.GetHp(shop), 12);
}

TEST(PropWorld, ParamHitboxOverrideChangesHurtbox)
{
    PropWorld       world { &GetPropLib(), 1 };
    PropSpawnParams params;
    params.hitbox = PropBoxOverride {
        { 50.f, 50.f },
        {  0.f,  0.f }
    };
    params.hp             = 15;
    const PropHandle shop = world.Spawn("shop_0", { 100.f, 100.f }, params);
    EXPECT_EQ(world.GetHp(shop), 15);
    CombatSystem combat;
    combat.BeginTick();
    world.RegisterHurtboxes(combat);
    ASSERT_EQ(combat.GetHurtboxes().size(), 1u);
    EXPECT_FLOAT_EQ(Width_(combat.GetHurtboxes()[0].box), 50.f);
}

TEST(PropWorld, ShopStandBoxIsOneWayPlatformAndGoesAwayOnDestroy)
{
    PropWorld        world { &GetPropLib(), 1 };
    const PropHandle shop = world.Spawn("shop_0", { 200.f, 100.f });
    ASSERT_EQ(world.GetPlatforms().size(), 1u);
    const Platform& p = world.GetPlatforms()[0];
    EXPECT_TRUE(p.bOneWay);
    EXPECT_FLOAT_EQ(p.box.min.y, 100.f);
    EXPECT_TRUE(world.GetEnemyPlatforms().empty());

    HitN_(world, shop, 15, 2);
    TickN_(world, 1);
    EXPECT_TRUE(world.GetPlatforms().empty());
}

TEST(PropWorld, ShopFiveReplacesStandBoxWhenDestroyed)
{
    PropWorld        world { &GetPropLib(), 1 };
    const PropHandle shop = world.Spawn("shop_5", { 2513.f, 181.f });
    ASSERT_EQ(world.GetPlatforms().size(), 1u);
    EXPECT_FLOAT_EQ(Width_(world.GetPlatforms()[0].box), 230.f);

    HitN_(world, shop, 1, 30);
    ASSERT_EQ(world.GetPlatforms().size(), 1u);
    EXPECT_FLOAT_EQ(Width_(world.GetPlatforms()[0].box), 220.f);
    EXPECT_FLOAT_EQ(world.GetPlatforms()[0].box.GetCenter().y, 181.f + 130.f);
}

TEST(PropWorld, HouseRoofOnlyBlocksEnemies)
{
    PropWorld        world { &GetPropLib(), 1 };
    const PropHandle house = world.Spawn("house_upper", { 7189.f, 717.f });
    EXPECT_TRUE(world.GetPlatforms().empty());
    ASSERT_EQ(world.GetEnemyPlatforms().size(), 1u);
    EXPECT_FLOAT_EQ(world.GetEnemyPlatforms()[0].box.min.y, 717.f + 3.f - 15.f);

    HitN_(world, house, 10, 2);
    TickN_(world, 1);
    EXPECT_TRUE(world.GetEnemyPlatforms().empty());
}

TEST(PropWorld, BridgeBlockWallShrinksOnDestroy)
{
    PropWorld        world { &GetPropLib(), 1 };
    const PropHandle block = world.Spawn("bridge_block", { 10098.f, 1274.f });
    ASSERT_EQ(world.GetWalls().size(), 1u);
    EXPECT_FLOAT_EQ(Width_(world.GetWalls()[0].box), 135.f);
    HitN_(world, block, 10, 2);
    ASSERT_EQ(world.GetWalls().size(), 1u);
    EXPECT_FLOAT_EQ(Width_(world.GetWalls()[0].box), 72.f);
    EXPECT_FLOAT_EQ(world.GetWalls()[0].box.GetCenter().x, 10098.f + 30.f);
}

TEST(PropWorld, PlayerLandsOnOneWayShopPlatform)
{
    PropWorld world { &GetPropLib(), 1 };
    world.Spawn("shop_0", { 200.f, 100.f });
    const GroundMap    ground = MakeFloorMap(400, 160, 140);
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
    world.Spawn("bridge_house", { 200.f, 90.f });
    const GroundMap    ground = MakeFloorMap(400, 200, 160);
    const PhysicsWorld pw { &ground, world.GetPlatforms(), world.GetWalls() };

    Body body     = MakeBody({ 50.f, 160.f });
    bool bHitWall = false;
    for (int i = 0; i < 200; ++i)
    {
        body.velocity.x    = 120.f;
        const MoveResult r = Physics::Step(body, pw, PhysicsTuning {}, kDt);
        bHitWall           = bHitWall || r.bHitWallRight;
    }
    EXPECT_TRUE(bHitWall);
    EXPECT_LT(body.position.x, 125.f);
    EXPECT_GT(body.position.x, 100.f);
}

TEST(PropWorld, MobBoatDropsSixItemsAndBurstsOnDeath)
{
    PropWorld        world { &GetPropLib(), 3 };
    const PropHandle boat = world.Spawn("mob_boat", { 4140.f, 545.f });
    HitN_(world, boat, 1, 60);
    EXPECT_TRUE(world.IsDestroyed(boat));

    const Vector<PropEvent> events = world.TakeEvents();
    EXPECT_EQ(CountEvents_(events, ePropEventKind::DropSpawned), 6);
    EXPECT_EQ(world.GetItems().GetCount(), 6u);
    EXPECT_EQ(world.GetDebris().size(), 3u);
}

TEST(PropWorld, ExplodeOnDeathBoatBurstsAfterTwoSecondsThenRemoved)
{
    PropWorld       world { &GetPropLib(), 3 };
    PropSpawnParams params;
    params.bExplodeOnDeath = true;
    const PropHandle boat  = world.Spawn("mob_boat", { 4140.f, 545.f }, params);
    HitN_(world, boat, 1, 60);
    (void)world.TakeEvents();

    TickN_(world, 110);
    EXPECT_EQ(world.GetCount(), 1u);
    TickN_(world, 15);
    EXPECT_EQ(world.GetCount(), 0u);
    const Vector<PropEvent> events = world.TakeEvents();
    EXPECT_TRUE(HasEvent_(events, ePropEventKind::SecondaryBurst));
    EXPECT_TRUE(HasEvent_(events, ePropEventKind::Removed));
    EXPECT_EQ(world.GetDebris().size(), 8u);
}

TEST(PropWorld, PlainBoatLeavesWreckWithoutSecondaryBurst)
{
    PropWorld        world { &GetPropLib(), 3 };
    const PropHandle boat = world.Spawn("mob_boat", { 4634.f, 545.f });
    HitN_(world, boat, 1, 60);
    TickN_(world, 300);
    EXPECT_EQ(world.GetCount(), 1u);
    EXPECT_TRUE(world.IsDestroyFinished(boat));
    EXPECT_FALSE(HasEvent_(world.TakeEvents(), ePropEventKind::SecondaryBurst));
}

TEST(PropWorld, ShopFiveSpawnsSignThatFallsToGroundAndDetonates)
{
    PropWorld        world { &GetPropLib(), 4 };
    const PropHandle shop = world.Spawn("shop_5", { 2513.f, 181.f });
    HitN_(world, shop, 1, 30);
    EXPECT_EQ(world.GetCount(), 2u);

    const GroundMap ground = MakeFloorMap(3000, 500, 400);
    PropTickContext ctx;
    ctx.pGround   = &ground;
    bool bRemoved = false;
    for (int i = 0; i < 400 && !bRemoved; ++i)
    {
        world.Tick(ctx);
        bRemoved = HasEvent_(world.TakeEvents(), ePropEventKind::Removed);
    }
    EXPECT_TRUE(bRemoved);
    EXPECT_EQ(world.GetCount(), 1u);
    EXPECT_EQ(world.GetDebris().size(), 12u);
}

TEST(PropWorld, GroundHouseDamageStagesAndGroundPatch)
{
    PropWorld        world { &GetPropLib(), 5 };
    const VECTOR2    center { 9830.f, 1269.f };
    const PropHandle house = world.Spawn("ground_house", center);

    HitN_(world, house, 6, 2);
    EXPECT_EQ(world.GetHp(house), 12);
    EXPECT_EQ(world.GetDebris().size(), 5u);
    TickN_(world, 1);
    EXPECT_EQ(world.Get(house)->visual, ePropVisual::Damaged);

    HitN_(world, house, 6, 2);
    EXPECT_TRUE(world.IsDestroyed(house));
    const Vector<PropEvent> events = world.TakeEvents();
    bool                    bPatch = false;
    for (const PropEvent& ev: events)
    {
        if (ev.kind == ePropEventKind::GroundPatch)
        {
            bPatch = true;
            EXPECT_EQ(ev.name.c_str(), String { "Stage0_House_4_PixelMap" });
            EXPECT_FLOAT_EQ(ev.pos.x, center.x - (653.f - 12.f));
            EXPECT_FLOAT_EQ(ev.pos.y, center.y - (375.5f - 5.f));
        }
    }
    EXPECT_TRUE(bPatch);
    EXPECT_EQ(CountEvents_(events, ePropEventKind::Damaged), 12);
}

TEST(PropWorld, ApplyGroundPatchClearsOnlyTheFootprintAndKeepsTheFloor)
{
    GroundMap map = GroundMap::MakeEmpty(10, 10);
    map.FillRect(0, 7, 9, 9, true);
    map.FillRect(3, 4, 5, 6, true);
    GroundMap footprint = GroundMap::MakeEmpty(10, 10);
    footprint.FillRect(3, 4, 5, 6, true);
    EXPECT_TRUE(ApplyGroundPatch(map, { 0.f, 0.f }, footprint));
    EXPECT_FALSE(map.IsSolid(3, 4));
    EXPECT_FALSE(map.IsSolid(5, 6));
    EXPECT_TRUE(map.IsSolid(0, 7));
    EXPECT_TRUE(map.IsSolid(9, 9));
    EXPECT_EQ(map.CountSolidPixels(), 30u);
}

TEST(PropWorld, ApplyGroundPatchRefusesFootprintsThatCutTheFloorOrClearMostOfTheGround)
{
    GroundMap map = GroundMap::MakeEmpty(10, 10);
    map.FillRect(0, 7, 9, 9, true);
    map.FillRect(3, 4, 5, 6, true);
    const uint64_t before = map.CountSolidPixels();

    const GroundMap cutsFloor = GroundMap::MakeFromRows(ARRAY<StringView, 3> { "###", "###", "###" });
    EXPECT_FALSE(ApplyGroundPatch(map, { 0.f, 7.f }, cutsFloor));
    EXPECT_EQ(map.CountSolidPixels(), before);

    GroundMap everything = GroundMap::MakeEmpty(10, 10);
    everything.FillRect(0, 0, 9, 9, true);
    EXPECT_FALSE(ApplyGroundPatch(map, { 0.f, 0.f }, everything));
    EXPECT_EQ(map.CountSolidPixels(), before);
}

TEST(PropWorld, TruckVariantPositionShiftMatchesLegacyFinalInit)
{
    PropWorld       world { &GetPropLib(), 1 };
    PropSpawnParams params;
    params.variant         = 1;
    const PropHandle truck = world.Spawn("truck", { 580.f, 342.f }, params);
    EXPECT_FLOAT_EQ(world.Get(truck)->pos.y, 332.f);
    EXPECT_EQ(world.Get(truck)->variant, 1);
    EXPECT_EQ(world.GetWalls().size(), 2u);
}

TEST(PropWorld, SameSeedGivesSameVariantsDropsAndHash)
{
    EXPECT_EQ(HashAfterTrucks_(42), HashAfterTrucks_(42));
    EXPECT_NE(HashAfterTrucks_(42), HashAfterTrucks_(43));
}

TEST(PropWorld, CullingRemovesPropsBehindCameraWithEvents)
{
    PropWorld world { &GetPropLib(), 1 };
    world.Spawn("shop_0", { 1274.f, 228.f });
    world.Spawn("truck", { 1810.f, 342.f });

    PropTickContext ctx;
    ctx.cameraLeft = 1100.f;
    TickN_(world, 1, &ctx);
    EXPECT_EQ(world.GetCount(), 2u);

    ctx.cameraLeft = 1500.f;
    world.Tick(ctx);
    EXPECT_EQ(world.GetCount(), 1u);
    const Vector<PropEvent> events = world.TakeEvents();
    EXPECT_EQ(CountEvents_(events, ePropEventKind::Culled), 1);
    EXPECT_EQ(CountEvents_(events, ePropEventKind::Removed), 1);
}

TEST(PropWorld, CullingKeepsAWidePropWhileAnyPartOfItsSpriteIsOnScreen)
{
    AnimationLibrary anims;
    SpriteLibrary    sprites;
    ASSERT_TRUE(LoadLibs(anims, sprites));
    PropWorld world { &GetPropLib(), 1 };
    world.SetSpriteLibrary(&sprites);
    world.Spawn("big_house", { 1000.f, 1257.f });

    PropTickContext ctx;
    ctx.pAnims = &anims;
    TickN_(world, 1, &ctx);
    ASSERT_EQ(world.GetCount(), 1u);
    Aabb bounds = {};
    ASSERT_TRUE(world.GetVisualBounds(world.GetProps()[0], &anims, bounds));
    ASSERT_GT(bounds.max.x - bounds.min.x, 400.f);

    const float edge = bounds.max.x;
    ctx.cameraLeft   = edge - 1.f;
    ASSERT_LT(1000.f, ctx.cameraLeft - 150.f) << "앵커 규칙만으로는 지워질 위치여야 이 테스트가 의미가 있다";
    TickN_(world, 5, &ctx);
    EXPECT_EQ(world.GetCount(), 1u);
    EXPECT_EQ(CountEvents_(world.TakeEvents(), ePropEventKind::Culled), 0);

    ctx.cameraLeft = edge + 1.f;
    world.Tick(ctx);
    EXPECT_EQ(world.GetCount(), 0u);
    const Vector<PropEvent> events = world.TakeEvents();
    EXPECT_EQ(CountEvents_(events, ePropEventKind::Culled), 1);
    EXPECT_EQ(CountEvents_(events, ePropEventKind::Removed), 1);
}

TEST(PropWorld, ResetClearsPropsAndRestoresIds)
{
    PropWorld world { &GetPropLib(), 1 };
    world.Spawn("shop_0", { 1274.f, 228.f });
    world.Spawn("truck", { 580.f, 342.f });
    EXPECT_EQ(world.GetCount(), 2u);

    world.Reset(1);
    EXPECT_EQ(world.GetCount(), 0u);
    EXPECT_TRUE(world.GetWalls().empty());
    EXPECT_EQ(world.Spawn("shop_0", { 1274.f, 228.f }), kPropOwnerBase);
}

TEST(PropWorld, HashChangesWithDamageAndIsStableWithoutIt)
{
    PropWorld a { &GetPropLib(), 9 };
    PropWorld b { &GetPropLib(), 9 };
    a.Spawn("tower", { 10808.f, 1175.f });
    b.Spawn("tower", { 10808.f, 1175.f });
    EXPECT_EQ(a.HashState(), b.HashState());
    HitN_(a, a.GetProps()[0].id, 1, 2);
    EXPECT_NE(a.HashState(), b.HashState());
}

TEST(PropWorld, SubmitQueuesSpritesOnLayersFromDefinitions)
{
    Result<SpriteLibrary> spriteLib = SpriteLibrary::LoadFromJson(R"({"version":1,"sprites":{"TestBox":{"atlas":0,"x":0,"y":0,"w":10,"h":10,"pivotX":5,"pivotY":5}}})");
    ASSERT_TRUE(static_cast<bool>(spriteLib));
    Result<PropDefLibrary> defs = PropDefLibrary::Parse(R"({ "version": 1, "defs": {
        "box":   { "hp": 4, "idle": { "sprite": "TestBox" }, "parts": [ { "role": "hurt", "size": [10, 10] } ] },
        "front": { "hp": 0, "depth": 38, "idle": { "sprite": "TestBox" } } } })");
    ASSERT_TRUE(static_cast<bool>(defs));

    const PropDefLibrary boxLib = defs.Take();
    PropWorld            world { &boxLib, 1 };
    world.Spawn("box", { 10.f, 20.f });
    world.Spawn("front", { 30.f, 40.f });
    AnimationLibrary    anims;
    const SpriteLibrary sprites = spriteLib.Take();
    SpriteBatch         batch;
    world.Submit(batch, anims, sprites);
    const Vector<SpriteCommand>& cmds = batch.GetPendingCommands();
    ASSERT_EQ(cmds.size(), 2u);
    EXPECT_EQ(cmds[0].depth, kPropDepthBack);
    EXPECT_EQ(cmds[1].depth, kPropDepthFront);
    EXPECT_FLOAT_EQ(cmds[1].origin.x, 30.f);
}

TEST(PropWorld, CrateIsOneWayPlatformNotWall)
{
    PropWorld        world { &GetPropLib(), 1 };
    const PropHandle crate = world.Spawn("crate", { 1119.f, 350.f });
    ASSERT_NE(crate, kNoProp);
    EXPECT_EQ(world.GetWalls().size(), 0u);
    ASSERT_EQ(world.GetPlatforms().size(), 1u);
    const Platform& p = world.GetPlatforms()[0];
    EXPECT_TRUE(p.bOneWay);
    EXPECT_FLOAT_EQ(p.box.min.y, 305.f);
}

TEST(PropWorld, CrateLetsBodyWalkThroughAndLandOnTop)
{
    PropWorld world { &GetPropLib(), 1 };
    world.Spawn("crate", { 1119.f, 350.f });
    const GroundMap  ground = MakeFloorMap(2000, 500, 378);
    Vector<Platform> platforms;
    for (const Platform& p: world.GetPlatforms())
    {
        platforms.push_back(p);
    }
    PhysicsWorld pw;
    pw.pGround   = &ground;
    pw.platforms = platforms;

    Body walker = MakeBody({ 1000.f, 378.f });
    for (int i = 0; i < 120; ++i)
    {
        walker.velocity.x = 150.f;
        (void)Physics::Step(walker, pw, PhysicsTuning {}, kDt);
    }
    EXPECT_GT(walker.position.x, 1200.f);

    Body faller = MakeBody({ 1119.f, 250.f });
    for (int i = 0; i < 90 && !faller.bGrounded; ++i)
    {
        (void)Physics::Step(faller, pw, PhysicsTuning {}, kDt);
    }
    EXPECT_TRUE(faller.bGrounded);
    EXPECT_NEAR(faller.position.y, 305.f, 0.5f);
}

TEST(PropWorld, OtherWallPropsKeepTheirWalls)
{
    PropWorld world { &GetPropLib(), 1 };
    world.Spawn("mob_boat", { 1000.f, 300.f });
    world.Spawn("crate", { 1119.f, 350.f });
    EXPECT_EQ(world.GetWalls().size(), 1u);
}

TEST(PropWorld, BuildingSortKeyPutsSmallerXInFront)
{
    PropWorld        world { &GetPropLib(), 1 };
    const PropHandle s1 = world.Spawn("shop_1", { 1489.f, 230.f });
    const PropHandle s2 = world.Spawn("shop_2", { 1661.f, 252.f });
    ASSERT_NE(world.Get(s1), nullptr);
    ASSERT_NE(world.Get(s2), nullptr);
    EXPECT_GT(PropWorld::SortKey(*world.Get(s1)), PropWorld::SortKey(*world.Get(s2)));
    EXPECT_EQ(world.Get(s2)->sortOffsetY, 0.f);
}

TEST(PropWorld, BuildingSortKeyTiesBreakByY)
{
    PropWorld        world { &GetPropLib(), 1 };
    const PropHandle back  = world.Spawn("shop_0", { 1274.f, 200.f });
    const PropHandle front = world.Spawn("shop_0", { 1274.f, 260.f });
    ASSERT_NE(world.Get(back), nullptr);
    ASSERT_NE(world.Get(front), nullptr);
    EXPECT_LT(PropWorld::SortKey(*world.Get(back)), PropWorld::SortKey(*world.Get(front)));
}

TEST(PropWorldStage0, ShopsDrawFrontToBackByX)
{
    String            diag;
    Result<StageData> result = LoadStageFromFile(FindAssetRoot() / "Stage" / "Stage0.json", &diag);
    ASSERT_TRUE(result) << diag;
    const StageData data = result.Take();

    Vector<SpawnSpec> shops;
    for (const StageSection& section: data.sections)
    {
        for (const StageAction& action: section.actions)
        {
            if (action.type == eActionType::Spawn && action.spawn.kind == eEntityKind::Prop && action.spawn.def.starts_with("shop_"))
            {
                shops.push_back(action.spawn);
            }
        }
    }
    ASSERT_EQ(shops.size(), 6u);
    std::sort(shops.begin(), shops.end(), [](const SpawnSpec& _a, const SpawnSpec& _b) { return _a.x < _b.x; });

    PropWorld   world { &GetPropLib(), 1 };
    Vector<int> keys;
    for (const SpawnSpec& shop: shops)
    {
        const PropHandle h = world.Spawn(shop.def.c_str(), { shop.x, shop.y });
        ASSERT_NE(world.Get(h), nullptr);
        keys.push_back(PropWorld::SortKey(*world.Get(h)));
    }
    for (size_t i = 1; i < keys.size(); ++i)
    {
        EXPECT_GT(keys[i - 1], keys[i]) << "shop index " << i;
    }
}

TEST(PropWorld, SortKeyWithoutOffsetIsCenterY)
{
    PropWorld        world { &GetPropLib(), 1 };
    const PropHandle boat = world.Spawn("mob_boat", { 1000.f, 300.f });
    ASSERT_NE(world.Get(boat), nullptr);
    EXPECT_EQ(PropWorld::SortKey(*world.Get(boat)), 300);
}