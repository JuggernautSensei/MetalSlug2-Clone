#include <cmath>
#include "Base/Types.h"
#include "Base/Math.h"
#include <string>

#include "Game/Player/PlayerController.h"
#include "PropTestUtil.h"

using namespace slugtest;

namespace
{

constexpr float kDt = kStepSec;

PhysicsWorld FloorWorld(const GroundMap& _ground)
{
    PhysicsWorld world;
    world.pGround = &_ground;
    return world;
}

void TickItems(ItemWorld& _items, const PhysicsWorld& _world, const int _ticks, const float _camLeft = -1.0e9f)
{
    for (int i = 0; i < _ticks; ++i)
    {
        _items.Tick(_world, kDt, _camLeft, nullptr);
    }
}

bool IsFruit(const PropDefLibrary& _lib, const slug::String& _name)
{
    const PropItemGroup* pGroup = _lib.FindGroup("random_fruit");
    if (pGroup == nullptr)
    {
        return false;
    }
    for (const slug::String& name: pGroup->names)
    {
        if (name == _name)
        {
            return true;
        }
    }
    return false;
}

// 플레이어 몸 상자: 발 (x, y) 기준 24x40
Aabb PlayerBoxAt(const Vec2 _feet)
{
    return Aabb::FromBottomCenter(_feet, { 24.f, 40.f });
}

}   // namespace

// ===========================================
//  스폰 / 물리
// ===========================================

TEST(PropItem, UnknownNameSpawnsNothing)
{
    ItemWorld items { &GetPropLib(), 1 };
    EXPECT_EQ(items.Spawn("no_such_item", { 0.f, 0.f }), 0u);
    EXPECT_EQ(items.GetCount(), 0u);
}

TEST(PropItem, SpawnResolvesItemDefinition)
{
    ItemWorld items { &GetPropLib(), 1 };
    const uint32_t id = items.Spawn("item_letter", { 100.f, 140.f });
    ASSERT_NE(id, 0u);
    const PropItem* pItem = items.Find(id);
    ASSERT_NE(pItem, nullptr);
    EXPECT_EQ(GetPropLib().GetItems()[static_cast<size_t>(pItem->defIndex)].name.c_str(), std::string { "item_letter" });
}

TEST(PropItem, RandomGroupPicksOnlyMembers)
{
    ItemWorld items { &GetPropLib(), 2 };
    for (int i = 0; i < 60; ++i)
    {
        const uint32_t id = items.Spawn("random_fruit", { 100.f, 140.f });
        ASSERT_NE(id, 0u);
        const PropItem* pItem = items.Find(id);
        EXPECT_TRUE(IsFruit(GetPropLib(), GetPropLib().GetItems()[static_cast<size_t>(pItem->defIndex)].name));
    }
}

TEST(PropItem, RandomGroupSequenceIsDeterministicPerSeed)
{
    auto names = [](const uint64_t _seed)
    {
        ItemWorld items { &GetPropLib(), _seed };
        std::string out;
        for (int i = 0; i < 20; ++i)
        {
            const PropItem* pItem = items.Find(items.Spawn("random_score", { 100.f, 140.f }));
            out += GetPropLib().GetItems()[static_cast<size_t>(pItem->defIndex)].name.c_str();
            out += ';';
        }
        return out;
    };
    EXPECT_EQ(names(77), names(77));
    EXPECT_NE(names(77), names(78));
}

TEST(PropItem, GroundedItemLandsOnFloorAndStays)
{
    const GroundMap ground = MakeFloorMap(400, 200, 140);
    ItemWorld       items { &GetPropLib(), 1 };
    const uint32_t  id = items.Spawn("item_letter", { 100.f, 0.f });
    TickItems(items, FloorWorld(ground), 120);
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
    TickItems(items, FloorWorld(ground), 880);
    EXPECT_EQ(items.GetCount(), 1u);
    TickItems(items, FloorWorld(ground), 25);
    EXPECT_EQ(items.GetCount(), 0u);
}

TEST(PropItem, ItemsBehindCameraAreRemoved)
{
    const GroundMap ground = MakeFloorMap(400, 200, 140);
    ItemWorld       items { &GetPropLib(), 1 };
    items.Spawn("item_letter", { 100.f, 140.f });
    TickItems(items, FloorWorld(ground), 1, 200.f);   // 100 < 200 - 50
    EXPECT_EQ(items.GetCount(), 0u);
}

TEST(PropItem, BounceSupplyLaunchesUpAndSideways)
{
    ItemWorld items { &GetPropLib(), 4 };
    const PropItem* pItem = items.Find(items.Spawn("bomb_supple", { 100.f, 100.f }, true));
    ASSERT_NE(pItem, nullptr);
    EXPECT_FLOAT_EQ(pItem->body.velocity.y, -400.f);
    EXPECT_FLOAT_EQ(std::abs(pItem->body.velocity.x), 75.f);
}

TEST(PropItem, RollingFruitSlowsDownAndStops)
{
    const GroundMap ground = MakeFloorMap(400, 200, 140);
    ItemWorld       items { &GetPropLib(), 6 };
    const uint32_t  id = items.Spawn("item_apple", { 100.f, 140.f });
    TickItems(items, FloorWorld(ground), 1);
    const float startX = items.Find(id)->feet.x;
    TickItems(items, FloorWorld(ground), 600);
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
        TickItems(items, FloorWorld(ground), 1);
        EXPECT_FLOAT_EQ(items.Find(id)->feet.x, 100.f) << "tick " << i;
    }
    EXPECT_TRUE(items.Find(id)->body.bGrounded);
    TickItems(items, FloorWorld(ground), 600);
    EXPECT_NE(items.Find(id)->feet.x, 100.f);
}

// ===========================================
//  습득 / 점수 팝업
// ===========================================

TEST(PropItem, ScoreItemCollectedGivesPickupAndPopup)
{
    const GroundMap ground = MakeFloorMap(400, 200, 140);
    ItemWorld       items { &GetPropLib(), 1 };
    items.Spawn("item_letter", { 100.f, 140.f });
    TickItems(items, FloorWorld(ground), 2);

    const slug::Vector<ItemPickup> pickups = items.Collect(PlayerBoxAt({ 100.f, 140.f }));
    ASSERT_EQ(pickups.size(), 1u);
    EXPECT_EQ(pickups[0].kind, eItemKind::Score);
    EXPECT_EQ(pickups[0].score, 1100);
    EXPECT_EQ(items.GetCount(), 0u);
    EXPECT_EQ(items.GetPopups().size(), 1u);
    EXPECT_EQ(items.GetPopups()[0].score, 1100);
    EXPECT_TRUE(items.Collect(PlayerBoxAt({ 100.f, 140.f })).empty());   // 두 번 줍지 못한다
}

TEST(PropItem, GunItemShowsPickupEffectThenDisappears)
{
    const GroundMap ground = MakeFloorMap(400, 200, 140);
    ItemWorld       items { &GetPropLib(), 1 };
    items.Spawn("gunitem_mg", { 100.f, 140.f });
    TickItems(items, FloorWorld(ground), 60);

    const slug::Vector<ItemPickup> pickups = items.Collect(PlayerBoxAt({ 100.f, 140.f }));
    ASSERT_EQ(pickups.size(), 1u);
    EXPECT_EQ(pickups[0].kind, eItemKind::Gun);
    EXPECT_EQ(pickups[0].weapon, eWeapon::Hmg);
    EXPECT_EQ(items.GetCount(), 1u);   // Disappear 연출 중
    const slug::Vector<ItemFxRequest> fx = items.TakeFx();
    ASSERT_EQ(fx.size(), 1u);
    EXPECT_EQ(fx[0].clip.c_str(), std::string { "Get_Item_Effect" });

    TickItems(items, FloorWorld(ground), 5);   // 0.06 s 이후
    EXPECT_EQ(items.GetCount(), 0u);
}

TEST(PropItem, PopupRisesAndExpiresAfterSpec)
{
    ItemWorld items { &GetPropLib(), 1 };
    items.Collect(PlayerBoxAt({ 100.f, 140.f }));   // 빈 상태: 팝업 없음
    items.Spawn("item_apple", { 100.f, 140.f });
    (void)items.Collect(PlayerBoxAt({ 100.f, 140.f }));   // 공중 아이템은 상자 안이면 줍힌다
    const GroundMap ground = MakeFloorMap(400, 200, 140);
    ASSERT_EQ(items.GetPopups().size(), 1u);
    TickItems(items, FloorWorld(ground), 150);   // 2.5 s
    EXPECT_EQ(items.GetPopups().size(), 1u);
    TickItems(items, FloorWorld(ground), 20);    // 2.8 s 이후
    EXPECT_TRUE(items.GetPopups().empty());
}

TEST(PropItem, PickupGrantsScoreToPlayer)
{
    const Tuning tuning {};
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
    const Tuning tuning {};
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
    const Tuning tuning {};
    PlayerController player;
    player.Reset(tuning, { 100.f, 100.f });

    ItemPickup bullet;
    bullet.kind   = eItemKind::Supply;
    bullet.supply = eSupplyKind::Bullet;
    ApplyItemPickup(player, bullet, tuning);   // 권총: 변화 없음
    EXPECT_EQ(player.GetWeapon().weapon, eWeapon::Pistol);
    EXPECT_EQ(player.GetWeapon().ammo, 0);

    ItemPickup gun;
    gun.kind   = eItemKind::Gun;
    gun.weapon = eWeapon::Hmg;
    ApplyItemPickup(player, gun, tuning);
    const int afterGun = player.GetWeapon().ammo;
    ApplyItemPickup(player, bullet, tuning);   // HMG 탄 보충
    EXPECT_EQ(player.GetWeapon().ammo, afterGun + tuning.weapons.hmgPickupAmmo);
}

TEST(PropItem, SameOperationsGiveSameHashAndDifferentOnesDiffer)
{
    auto run = [](const uint64_t _seed, const char* _item)
    {
        const GroundMap ground = MakeFloorMap(400, 200, 140);
        ItemWorld       items { &GetPropLib(), _seed };
        for (int i = 0; i < 5; ++i)
        {
            items.Spawn(_item, { 100.f + 20.f * static_cast<float>(i), 0.f });
        }
        TickItems(items, FloorWorld(ground), 200);
        return items.HashState();
    };
    EXPECT_EQ(run(3, "random_score"), run(3, "random_score"));
    EXPECT_NE(run(3, "random_score"), run(4, "random_score"));
}
