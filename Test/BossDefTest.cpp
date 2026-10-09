#include <gtest/gtest.h>
#include "Base/Types.h"
#include "Base/Result.h"

#include "Game/Enemies/BattleShip.h"
#include "Game/Enemies/EnemyCommon.h"
#include "Game/Enemies/EnemyDef.h"
#include "Game/Enemies/EnemyTypes.h"
#include "BossTestUtil.h"
#include "PlayerTestUtil.h"

using namespace slug;
using namespace slugtest;

TEST(BossDef, BothBossDefinitionsParseWithSpecHitPoints)
{
    BossRig rig;
    ASSERT_TRUE(rig.IsReady());
    const EnemyDefLibrary& lib = rig.GetLib();

    const EnemyDef* pBus = lib.Find("mb_truck");
    const EnemyDef* pShip = lib.Find("battleship");
    ASSERT_NE(pBus, nullptr);
    ASSERT_NE(pShip, nullptr);
    EXPECT_EQ(pBus->behavior, eEnemyBehavior::Bus);
    EXPECT_EQ(pShip->behavior, eEnemyBehavior::BattleShip);
    EXPECT_EQ(pBus->hp, 250);    // Docs/EnemySpecs.md 7 (TTK 25 s)
    EXPECT_EQ(pShip->hp, 900);   // TTK 90 s
    EXPECT_EQ(pBus->team, eTeam::Enemy);
    EXPECT_EQ(pShip->team, eTeam::Enemy);
    EXPECT_TRUE(pBus->bNoDespawn);
    EXPECT_TRUE(pShip->bNoDespawn);
    EXPECT_GT(pShip->score, pBus->score);
}

TEST(BossDef, BusTimingsAndDropLimitsMatchTheSpec)
{
    BossRig rig;
    ASSERT_TRUE(rig.IsReady());
    const BusCfg& c = rig.GetLib().Find("mb_truck")->bus;
    EXPECT_EQ(c.openTicks, 210);             // MainTruck_Begin 3.5 s
    EXPECT_EQ(c.shellFirstTicks, 120);       // 첫 포탄 2 s
    EXPECT_EQ(c.shellIntervalTicks, 180);    // 이후 3 s
    EXPECT_EQ(c.soldierFirstTicks, 180);     // 첫 투하 3 s
    EXPECT_EQ(c.soldierIntervalTicks, 300);  // 이후 5 s
    EXPECT_EQ(c.soldierAliveMax, 2);
    EXPECT_EQ(c.soldierTotalMax, 8);
    EXPECT_TRUE(c.soldierDef == "soldier_bomb");
    EXPECT_GE(c.weaponShell, 0);
    EXPECT_FLOAT_EQ(c.originDepth, 123.f);
    EXPECT_FLOAT_EQ(c.trigger.cx, -215.f);
    EXPECT_GT(c.trigger.w, 0.f);
}

TEST(BossDef, BattleShipConfigMatchesTheSpec)
{
    BossRig rig;
    ASSERT_TRUE(rig.IsReady());
    const BattleShipCfg& c = rig.GetLib().Find("battleship")->battleShip;
    EXPECT_FLOAT_EQ(c.originDepth, 132.f);
    EXPECT_FLOAT_EQ(c.entranceOffsetX, -300.f);
    EXPECT_FLOAT_EQ(c.railSlot[0], 30.f);    // back
    EXPECT_FLOAT_EQ(c.railSlot[1], 100.f);   // home
    EXPECT_FLOAT_EQ(c.railSlot[2], 190.f);   // forward
    EXPECT_EQ(c.homeSlot, 1);
    EXPECT_EQ(c.idleTicks, 96);              // 1.6 s
    EXPECT_EQ(c.idleEnragedTicks, 48);       // 0.8 s
    EXPECT_EQ(c.cannonShots, 3);
    EXPECT_EQ(c.cannonShotsEnraged, 5);
    EXPECT_EQ(c.turretHp, 40);
    EXPECT_FLOAT_EQ(c.hullBlockDx, 270.f);
    EXPECT_FLOAT_EQ(c.turretOffset[0].x, -200.f);
    EXPECT_FLOAT_EQ(c.turretOffset[1].y, -120.f);
    EXPECT_FLOAT_EQ(c.bow.cx, 175.f);
    EXPECT_FLOAT_EQ(c.bridge.w, 280.f);
    EXPECT_FLOAT_EQ(c.aft.cy, 10.f);
    EXPECT_FLOAT_EQ(c.destroyFinishSec, 3.6f);
    EXPECT_FLOAT_EQ(c.destroyChainSec, 2.8f);
    EXPECT_FLOAT_EQ(c.turretMuzzle[1].y, -35.f);
    EXPECT_TRUE(c.bulletProjectile == "battleship_bullet");
    EXPECT_TRUE(c.turretProjectile == "btank_shell");
}

TEST(BossDef, BossProjectilesParseWithTheirBallisticsAndExplosions)
{
    BossRig rig;
    ASSERT_TRUE(rig.IsReady());
    const EnemyDefLibrary& lib = rig.GetLib();

    const ProjectileDef* pBullet = lib.FindProjectile("battleship_bullet");
    ASSERT_NE(pBullet, nullptr);
    EXPECT_FLOAT_EQ(pBullet->speed, 300.f);
    EXPECT_FLOAT_EQ(pBullet->size.x, 30.f);
    EXPECT_FLOAT_EQ(pBullet->gravity, 0.f);
    EXPECT_TRUE(pBullet->explosion.clip == "Explosion_Huge");
    EXPECT_FLOAT_EQ(pBullet->explosion.offset.y, -20.f);

    const ProjectileDef* pShell = lib.FindProjectile("btank_shell");
    ASSERT_NE(pShell, nullptr);
    EXPECT_FLOAT_EQ(pShell->gravity, 500.f);
    EXPECT_FLOAT_EQ(pShell->v0.y, -200.f);

    const ProjectileDef* pBus = lib.FindProjectile("bus_shell");
    ASSERT_NE(pBus, nullptr);
    EXPECT_FLOAT_EQ(pBus->gravity, 500.f);
    EXPECT_GE(lib.FindProjectileIndex("battleship_bullet"), 0);
}

TEST(BossDef, BehaviorNamesAndLookupRoundTrip)
{
    EXPECT_STREQ(GetBehaviorName(eEnemyBehavior::Bus), "bus");
    EXPECT_STREQ(GetBehaviorName(eEnemyBehavior::BattleShip), "battleship");
    ASSERT_TRUE(FindBehavior("bus").has_value());
    EXPECT_EQ(*FindBehavior("bus"), eEnemyBehavior::Bus);
    ASSERT_TRUE(FindBehavior("battleship").has_value());
    EXPECT_EQ(*FindBehavior("battleship"), eEnemyBehavior::BattleShip);
}

TEST(BossDef, PartOwnersKeepTheEnemyIdAndTheTurretIndex)
{
    const uint32_t id = 2345;
    for (int i = 0; i < 2; ++i)
    {
        const uint32_t owner = battleship::TurretOwner(id, i);
        EXPECT_TRUE(IsEnemyOwner(owner));
        EXPECT_EQ(owner & kEnemyIdMask, id);
        EXPECT_EQ(GetPartIndex(owner), i);
        EXPECT_EQ(owner & kShieldOwnerBit, 0u);
    }
    EXPECT_EQ(GetPartIndex(id), -1);                 // 본체는 파트가 아니다
    EXPECT_EQ(GetPartIndex(id | kShieldOwnerBit), -1);   // 방패 소유자도 파트가 아니다
    EXPECT_NE(battleship::TurretOwner(id, 0), battleship::TurretOwner(id, 1));
}

TEST(BossDef, BossClipAndSpriteNamesExistInBakedAssets)
{
    SLUG_REQUIRE_BAKED(anims);
    slug::Result<SpriteLibrary> sprites = SpriteLibrary::LoadFromDirectory(slug::FilePath { SLUG_ASSET_DIR } / "Baked");
    ASSERT_TRUE(sprites) << "sprites.json 을 읽지 못했습니다";

    const char* kNames[] = {
        "MainTruck_Not_Begin", "MainTruck_Begin", "MainTruck_Idle_0", "MainTruck_Idle_1", "MainTruck_Idle_2", "MainTruck_Idle_3",
        "MainTruck_Ladder_0", "MainTruck_Ladder_1", "MainTruck_Ladder_2", "MainTruck_Ladder_3", "MainTruck_Deco_Destroy_Stop",
        "MainTruck_Deco_Destroy", "MainTruck_Big_Win_Destroy_Begin", "MainTruck_Bump", "MainTruck_Sideparts", "MainTruck_Die",
        "MainTruck_Die_Front", "LeftUp_Win_Destroy", "RightUp_Win_Destroy", "LeftDown_Win_Destroy", "RightDown_Win_Destroy",
        "BTank_Proj", "BTank_Proj_Effect", "BTank_Shot_Effect", "Explosion_Small", "Explosion_Mid", "Explosion_Large", "Explosion_Huge",
        "BattleShip_Body", "BattleShip_Tail", "BattleShip_Head", "BattleShip_Head_Swiching_Start", "BattleShip_Head_Swiching_End",
        "BattleShip_Antena", "BattleShip_Antena_Turn", "BattleShip_Cannon_Start", "BattleShip_Cannon_End", "BattleShip_Cannon_Idle_0",
        "BattleShip_Cannon_Idle_8", "BattleShip_Cannon_Shot_0", "BattleShip_Cannon_Shot_4", "BattleShip_Cannon_Deco",
        "BattleShip_Front_Rail_Idle", "BattleShip_Front_Rail_Forward", "BattleShip_Front_Rail_Backward", "BattleShip_Front_Rail_Forward_Fast",
        "BattleShip_Front_Rail_Backward_Fast", "BattleShip_Back_Rail_Idle", "BattleShip_Back_Rail_Forward", "BattleShip_Back_Rail_Backward",
        "BattleShip_Back_Rail_Forward_Fast", "BattleShip_Back_Rail_Backward_Fast", "BattleShip_DCannon_Left", "BattleShip_DCannon_Mid",
        "BattleShip_DCannon_Right", "BattleShip_DCannon_TurnLeft_0", "BattleShip_DCannon_TurnRight_0", "BattleShip_DCannon_Shot_Left",
        "BattleShip_DCannon_Shot_Mid", "BattleShip_DCannon_Shot_Right", "BattleShip_DCannon_Shot_Effect_Left",
        "BattleShip_DCannon_Shot_Effect_Mid", "BattleShip_DCannon_Shot_Effect_Right", "BattleShip_Cannon_Destroy_Type5",
        "BattleShip_Destroy", "BattleShip_Bullet_0", "BattleShip_Bullet_4",
    };
    for (const char* name: kNames)
    {
        const bool bAnim   = anims.FindClip(name) != nullptr;
        const bool bSprite = sprites.GetValue().Find(name) != nullptr;
        EXPECT_TRUE(bAnim || bSprite) << "클립/스프라이트 없음: " << name;
    }
}

TEST(BossDef, ClipDurationsAgreeWithTheBakedFrames)
{
    SLUG_REQUIRE_BAKED(anims);
    const int beginTicks = GetNamedPartTicks(anims, "MainTruck_Begin");
    EXPECT_NEAR(beginTicks, 210, 3);
    EXPECT_NEAR(GetNamedPartTicks(anims, "BattleShip_Cannon_Start"), 96, 3);
    EXPECT_NEAR(GetNamedPartTicks(anims, "BattleShip_DCannon_Shot_Left"), 67, 3);
    EXPECT_NEAR(GetNamedPartTicks(anims, "BattleShip_Head_Swiching_Start"), 86, 3);
    EXPECT_EQ(GetNamedPartTicks(anims, "BattleShip_Bullet_0"), 0);   // 정적 스프라이트는 길이 0
}
