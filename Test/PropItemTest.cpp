// 아이템 테스트: 아이템 낙하, 줍기, 효과 적용과 드롭 결정성 검증
#include <cmath>
#include <string>

#include "Base/Math.h"
#include "Base/Types.h"
#include "Game/Player/PlayerController.h"
#include "PropTestUtil.h"

constexpr float kDt = kStepSec;

namespace
{
[[nodiscard]] PhysicsWorld FloorWorld_(
    const GroundMap& _ground)
{
    PhysicsWorld world;
    world.pGround = &_ground;
    return world;
}

void TickItems_(
    ItemWorld&          _items,
    const PhysicsWorld& _world,
    const int           _ticks,
    const float         _camLeft = -1.0e9f)
{
    for (int i = 0; i < _ticks; ++i)
    {
        _items.Tick(_world, kDt, _camLeft, nullptr);
    }
}

[[nodiscard]] bool IsFruit_(
    const PropDefLibrary& _lib,
    const String&         _name)
{
    const PropItemGroup* pGroup = _lib.FindGroup("random_fruit");
    if (pGroup == nullptr)
    {
        return false;
    }
    for (const String& name: pGroup->names)
    {
        if (name == _name)
        {
            return true;
        }
    }
    return false;
}

[[nodiscard]] Aabb PlayerBoxAt_(
    const VECTOR2 _feet)
{
    return Aabb::FromBottomCenter(_feet, { 24.f, 40.f });
}
// 같은 시드로 random_score 그룹에서 20개를 뽑은 이름 나열
[[nodiscard]] String NamesFromSeed_(
    const uint64_t _seed)
{
    ItemWorld items { &GetPropLib(), _seed };
    String    out;
    for (int i = 0; i < 20; ++i)
    {
        const PropItem* pItem = items.Find(items.Spawn("random_score", { 100.f, 140.f }));
        out += GetPropLib().GetItems()[static_cast<size_t>(pItem->defIndex)].name.c_str();
        out += ';';
    }
    return out;
}

// 아이템 5개를 떨어뜨려 200틱 진행한 뒤의 상태 해시
[[nodiscard]] uint64_t HashAfterSpawn_(
    const uint64_t _seed,
    const char*    _item)
{
    const GroundMap ground = MakeFloorMap(400, 200, 140);
    ItemWorld       items { &GetPropLib(), _seed };
    for (int i = 0; i < 5; ++i)
    {
        items.Spawn(_item, { 100.f + 20.f * static_cast<float>(i), 0.f });
    }
    TickItems_(items, FloorWorld_(ground), 200);
    return items.HashState();
}
}   // namespace

TEST(PropItem, UnknownNameSpawnsNothing)
{
    ItemWorld items { &GetPropLib(), 1 };
    EXPECT_EQ(items.Spawn("no_such_item", { 0.f, 0.f }), 0u);
    EXPECT_EQ(items.GetCount(), 0u);
}

TEST(PropItem, SpawnResolvesItemDefinition)
{
    ItemWorld      items { &GetPropLib(), 1 };
    const uint32_t id = items.Spawn("item_letter", { 100.f, 140.f });
    ASSERT_NE(id, 0u);
    const PropItem* pItem = items.Find(id);
    ASSERT_NE(pItem, nullptr);
    EXPECT_EQ(GetPropLib().GetItems()[static_cast<size_t>(pItem->defIndex)].name.c_str(), String { "item_letter" });
}

TEST(PropItem, RandomGroupPicksOnlyMembers)
{
    ItemWorld items { &GetPropLib(), 2 };
    for (int i = 0; i < 60; ++i)
    {
        const uint32_t id = items.Spawn("random_fruit", { 100.f, 140.f });
        ASSERT_NE(id, 0u);
        const PropItem* pItem = items.Find(id);
        EXPECT_TRUE(IsFruit_(GetPropLib(), GetPropLib().GetItems()[static_cast<size_t>(pItem->defIndex)].name));
    }
}

TEST(PropItem, RandomGroupSequenceIsDeterministicPerSeed)
{
    EXPECT_EQ(NamesFromSeed_(77), NamesFromSeed_(77));
    EXPECT_NE(NamesFromSeed_(77), NamesFromSeed_(78));
}

TEST(PropItem, GroundedItemLandsOnFloorAndStays)
{
    const GroundMap ground = MakeFloorMap(400, 200, 140);
    ItemWorld       items { &GetPropLib(), 1 };
    const uint32_t  id = items.Spawn("item_letter", { 100.f, 0.f });
    TickItems_(items, FloorWorld_(ground), 120);
    const PropItem* pItem = items.Find(id);
    ASSERT_NE(pItem, nullptr);
    EXPECT_TRUE(pItem->body.bGrounded);
    EXPECT_NEAR(pItem->feet.y, 140.f, 0.5f);
}

TEST(PropItem, LifetimeIsFifteenSeconds)
{
    const GroundMap ground = MakeFloorMap(400, 200, 140);
    ItemWorld       items { &GetPropLib(), 1 };
    items.Spawn("item_letter", { 100.f, 140.f });
    TickItems_(items, FloorWorld_(ground), 880);
    EXPECT_EQ(items.GetCount(), 1u);
    TickItems_(items, FloorWorld_(ground), 25);
    EXPECT_EQ(items.GetCount(), 0u);
}

TEST(PropItem, ItemsBehindCameraAreRemoved)
{
    const GroundMap ground = MakeFloorMap(400, 200, 140);
    ItemWorld       items { &GetPropLib(), 1 };
    items.Spawn("item_letter", { 100.f, 140.f });
    TickItems_(items, FloorWorld_(ground), 1, 200.f);
    EXPECT_EQ(items.GetCount(), 0u);
}

TEST(PropItem, BounceSupplyLaunchesStraightUp)
{
    ItemWorld       items { &GetPropLib(), 4 };
    const PropItem* pItem = items.Find(items.Spawn("bomb_supple", { 100.f, 100.f }, true));
    ASSERT_NE(pItem, nullptr);
    EXPECT_FLOAT_EQ(pItem->body.velocity.y, -400.f);
    EXPECT_FLOAT_EQ(pItem->body.velocity.x, 0.f);
}

TEST(PropItem, BounceSupplyKeepsXWhileRisingAndFalling)
{
    const GroundMap ground = MakeFloorMap(400, 200, 140);
    ItemWorld       items { &GetPropLib(), 4 };
    const uint32_t  id   = items.Spawn("bomb_supple", { 100.f, 140.f }, true);
    float           minY = 1.0e9f;
    for (int i = 0; i < 240; ++i)
    {
        TickItems_(items, FloorWorld_(ground), 1);
        const PropItem* pItem = items.Find(id);
        ASSERT_NE(pItem, nullptr);
        EXPECT_FLOAT_EQ(pItem->feet.x, 100.f) << "tick " << i;
        if (pItem->feet.y < minY)
        {
            minY = pItem->feet.y;
        }
    }
    EXPECT_LT(minY, 100.f);
}

TEST(PropItem, RollingFruitSlowsDownAndStops)
{
    const GroundMap ground = MakeFloorMap(400, 200, 140);
    ItemWorld       items { &GetPropLib(), 6 };
    const uint32_t  id = items.Spawn("item_apple", { 100.f, 140.f });
    TickItems_(items, FloorWorld_(ground), 1);
    const float startX = items.Find(id)->feet.x;
    TickItems_(items, FloorWorld_(ground), 600);
    const PropItem* pItem = items.Find(id);
    ASSERT_NE(pItem, nullptr);
    EXPECT_FLOAT_EQ(pItem->body.velocity.x, 0.f);
    EXPECT_NE(pItem->feet.x, startX);
}

TEST(PropItem, RollingFruitFallsStraightDownBeforeItRolls)
{
    const GroundMap ground = MakeFloorMap(400, 200, 140);
    ItemWorld       items { &GetPropLib(), 6 };
    const uint32_t  id = items.Spawn("item_apple", { 100.f, 40.f });
    for (int i = 0; i < 120 && !items.Find(id)->body.bGrounded; ++i)
    {
        TickItems_(items, FloorWorld_(ground), 1);
        EXPECT_FLOAT_EQ(items.Find(id)->feet.x, 100.f) << "tick " << i;
    }
    EXPECT_TRUE(items.Find(id)->body.bGrounded);
    TickItems_(items, FloorWorld_(ground), 600);
    EXPECT_NE(items.Find(id)->feet.x, 100.f);
}

TEST(PropItem, ScoreItemCollectedGivesPickupAndPopup)
{
    const GroundMap ground = MakeFloorMap(400, 200, 140);
    ItemWorld       items { &GetPropLib(), 1 };
    items.Spawn("item_letter", { 100.f, 140.f });
    TickItems_(items, FloorWorld_(ground), 2);

    const Vector<ItemPickup> pickups = items.Collect(PlayerBoxAt_({ 100.f, 140.f }));
    ASSERT_EQ(pickups.size(), 1u);
    EXPECT_EQ(pickups[0].kind, eItemKind::Score);
    EXPECT_EQ(pickups[0].score, 1100);
    EXPECT_EQ(items.GetCount(), 0u);
    EXPECT_EQ(items.GetPopups().size(), 1u);
    EXPECT_EQ(items.GetPopups()[0].score, 1100);
    EXPECT_TRUE(items.Collect(PlayerBoxAt_({ 100.f, 140.f })).empty());
}

TEST(PropItem, GunItemShowsPickupEffectThenDisappears)
{
    const GroundMap ground = MakeFloorMap(400, 200, 140);
    ItemWorld       items { &GetPropLib(), 1 };
    items.Spawn("gunitem_mg", { 100.f, 140.f });
    TickItems_(items, FloorWorld_(ground), 60);

    const Vector<ItemPickup> pickups = items.Collect(PlayerBoxAt_({ 100.f, 140.f }));
    ASSERT_EQ(pickups.size(), 1u);
    EXPECT_EQ(pickups[0].kind, eItemKind::Gun);
    EXPECT_EQ(pickups[0].weapon, eWeapon::Hmg);
    EXPECT_EQ(items.GetCount(), 1u);
    const Vector<ItemFxRequest> fx = items.TakeFx();
    ASSERT_EQ(fx.size(), 1u);
    EXPECT_EQ(fx[0].clip.c_str(), String { "Get_Item_Effect" });

    TickItems_(items, FloorWorld_(ground), 5);   // 0.06 s 이후
    EXPECT_EQ(items.GetCount(), 0u);
}

TEST(PropItem, PopupRisesAndExpiresAfterSpec)
{
    ItemWorld items { &GetPropLib(), 1 };
    (void)items.Collect(PlayerBoxAt_({ 100.f, 140.f }));   // 빈 상태: 팝업 없음
    items.Spawn("item_apple", { 100.f, 140.f });
    (void)items.Collect(PlayerBoxAt_({ 100.f, 140.f }));
    const GroundMap ground = MakeFloorMap(400, 200, 140);
    ASSERT_EQ(items.GetPopups().size(), 1u);
    TickItems_(items, FloorWorld_(ground), 150);   // 2.5 s
    EXPECT_EQ(items.GetPopups().size(), 1u);
    TickItems_(items, FloorWorld_(ground), 20);   // 2.8 s 이후
    EXPECT_TRUE(items.GetPopups().empty());
}

TEST(PropItem, PickupGrantsScoreToPlayer)
{
    const Tuning     tuning {};
    PlayerController player;
    player.Reset(tuning, { 100.f, 100.f });
    ItemPickup pickup;
    pickup.kind  = eItemKind::Score;
    pickup.score = 1100;
    ApplyItemPickup(player, pickup, tuning);
    EXPECT_EQ(player.GetScore(), 1100);
}

TEST(PropItem, PickupGrantsWeaponWithAmmoAndBombs)
{
    const Tuning     tuning {};
    PlayerController player;
    player.Reset(tuning, { 100.f, 100.f });
    const int bombsBefore = player.GetWeapon().bombs;

    ItemPickup gun;
    gun.kind   = eItemKind::Gun;
    gun.weapon = eWeapon::Hmg;
    ApplyItemPickup(player, gun, tuning);
    EXPECT_EQ(player.GetWeapon().weapon, eWeapon::Hmg);
    EXPECT_EQ(player.GetWeapon().ammo, tuning.weapons.hmgPickupAmmo);

    ItemPickup bomb;
    bomb.kind   = eItemKind::Supply;
    bomb.supply = eSupplyKind::Bomb;
    bomb.amount = 10;
    ApplyItemPickup(player, bomb, tuning);
    EXPECT_EQ(player.GetWeapon().bombs, bombsBefore + 10);
}

TEST(PropItem, BulletSupplyRefillsOnlyFiniteWeapons)
{
    const Tuning     tuning {};
    PlayerController player;
    player.Reset(tuning, { 100.f, 100.f });

    ItemPickup bullet;
    bullet.kind   = eItemKind::Supply;
    bullet.supply = eSupplyKind::Bullet;
    ApplyItemPickup(player, bullet, tuning);
    EXPECT_EQ(player.GetWeapon().weapon, eWeapon::Pistol);
    EXPECT_EQ(player.GetWeapon().ammo, 0);

    ItemPickup gun;
    gun.kind   = eItemKind::Gun;
    gun.weapon = eWeapon::Hmg;
    ApplyItemPickup(player, gun, tuning);
    const int afterGun = player.GetWeapon().ammo;
    ApplyItemPickup(player, bullet, tuning);
    EXPECT_EQ(player.GetWeapon().ammo, afterGun + tuning.weapons.hmgPickupAmmo);
}

TEST(PropItem, SameOperationsGiveSameHashAndDifferentOnesDiffer)
{
    EXPECT_EQ(HashAfterSpawn_(3, "random_score"), HashAfterSpawn_(3, "random_score"));
    EXPECT_NE(HashAfterSpawn_(3, "random_score"), HashAfterSpawn_(4, "random_score"));
}

// 굴러가는 아이템: 회전(spinScale)은 감속과 함께 줄고, 멈추면 0 으로 고정된다
TEST(PropItem, RollingSpinSlowsDownAndFreezesWhenStopped)
{
    const GroundMap ground = MakeFloorMap(400, 200, 140);
    ItemWorld       items { &GetPropLib(), 6 };
    const uint32_t  id   = items.Spawn("item_apple", { 100.f, 140.f });
    float           prev = 2.f;
    for (int i = 0; i < 600; ++i)
    {
        TickItems_(items, FloorWorld_(ground), 1);
        const PropItem* pItem = items.Find(id);
        ASSERT_NE(pItem, nullptr);
        EXPECT_LE(pItem->spinScale, prev + 1e-6f) << "tick " << i;
        prev = pItem->spinScale;
    }
    EXPECT_FLOAT_EQ(items.Find(id)->spinScale, 0.f);
}

TEST(PropItem, RollingSpinIsFullWhileAirborne)
{
    const GroundMap ground = MakeFloorMap(400, 200, 140);
    ItemWorld       items { &GetPropLib(), 6 };
    const uint32_t  id = items.Spawn("item_apple", { 100.f, 40.f });
    TickItems_(items, FloorWorld_(ground), 1);
    EXPECT_FALSE(items.Find(id)->body.bGrounded);
    EXPECT_FLOAT_EQ(items.Find(id)->spinScale, 1.f);
}

TEST(PropItem, NonRollingItemKeepsFullSpin)
{
    const GroundMap ground = MakeFloorMap(400, 200, 140);
    ItemWorld       items { &GetPropLib(), 6 };
    const uint32_t  id = items.Spawn("item_letter", { 100.f, 140.f });
    TickItems_(items, FloorWorld_(ground), 120);
    EXPECT_FLOAT_EQ(items.Find(id)->spinScale, 1.f);
}

// ItemTuning 이 감속률을 실제로 바꾼다 (0 이면 속도가 줄지 않는다)
TEST(PropItem, RollFrictionComesFromItemTuning)
{
    const GroundMap ground = MakeFloorMap(400, 200, 140);
    ItemWorld       items { &GetPropLib(), 6 };
    const uint32_t  id = items.Spawn("item_apple", { 100.f, 140.f });
    ItemTuning      tuning;
    tuning.rollFrictionPerSec = 0.f;
    for (int i = 0; i < 5 && !items.Find(id)->body.bGrounded; ++i)
    {
        items.Tick(FloorWorld_(ground), kDt, -1.0e9f, nullptr, &tuning);
    }
    ASSERT_TRUE(items.Find(id)->body.bGrounded);
    items.Tick(FloorWorld_(ground), kDt, -1.0e9f, nullptr, &tuning);
    const float v0 = items.Find(id)->body.velocity.x;
    ASSERT_GT(Abs(v0), 1.f);
    for (int i = 0; i < 300; ++i)
    {
        items.Tick(FloorWorld_(ground), kDt, -1.0e9f, nullptr, &tuning);
    }
    EXPECT_FLOAT_EQ(items.Find(id)->body.velocity.x, v0);
}