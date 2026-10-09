#include "Core/AssetPath.h"
#include <gtest/gtest.h>
#include "Base/Types.h"
#include "Base/Result.h"

#include "Game/Enemies/BigShiee.h"
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

    const EnemyDef* pBus  = lib.Find("mb_truck");
    const EnemyDef* pBoss = lib.Find("bigshiee");
    ASSERT_NE(pBus, nullptr);
    ASSERT_NE(pBoss, nullptr);
    EXPECT_EQ(pBus->behavior, eEnemyBehavior::Bus);
    EXPECT_EQ(pBoss->behavior, eEnemyBehavior::BigShiee);
    EXPECT_EQ(pBus->hp, 250);    // Docs/EnemySpecs.md 7 (TTK 25 s)
    EXPECT_EQ(pBoss->hp, 900);   // 선체 HP (TTK 90 s 기준)
    EXPECT_EQ(pBus->team, eTeam::Enemy);
    EXPECT_EQ(pBoss->team, eTeam::Enemy);
    EXPECT_TRUE(pBus->bNoDespawn);
    EXPECT_TRUE(pBoss->bNoDespawn);
    EXPECT_GT(pBoss->score, pBus->score);
    EXPECT_EQ(lib.Find("battleship"), nullptr);   // 구 보스 정의는 제거되었다
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
    EXPECT_FLOAT_EQ(c.trigger.cx, -335.f);
    EXPECT_GT(c.trigger.w, 0.f);
}

TEST(BossDef, BigShieeConfigMatchesTheTimeline)
{
    BossRig rig;
    ASSERT_TRUE(rig.IsReady());
    const BigShieeCfg& c = rig.GetLib().Find("bigshiee")->bigShiee;
    EXPECT_FLOAT_EQ(c.originDepth, 208.f);            // 발 → 프레임 중심 (416 / 2)
    EXPECT_FLOAT_EQ(c.homeX, 290.f);
    EXPECT_FLOAT_EQ(c.driveMinX, 205.f);
    EXPECT_FLOAT_EQ(c.driveMaxX, 290.f);
    EXPECT_EQ(c.entranceTicks, 150);                  // 2.5 s 등장
    EXPECT_EQ(c.assaultTicks, 720);                   // 포탑 연사 12 s
    EXPECT_EQ(c.deployTicks, 240);                    // 캐논 전개 4 s
    EXPECT_EQ(c.volleyMinTicks, 180);                 // 볼리 3 s ~ 4 s
    EXPECT_EQ(c.volleyMaxTicks, 240);
    EXPECT_EQ(c.telegraphTicks, 24);                  // 사격 예고 0.4 s
    EXPECT_EQ(c.volleyStreaks, 3);
    EXPECT_EQ(c.shellEvery, 2);
    EXPECT_FLOAT_EQ(c.hullBlockDx, -60.f);
    EXPECT_EQ(c.turretHp, 60);
    EXPECT_EQ(c.cannonHp, 300);
    EXPECT_FLOAT_EQ(c.turretOffset[0].x, -130.f);
    EXPECT_FLOAT_EQ(c.turretOffset[6].y, -90.f);
    EXPECT_FLOAT_EQ(c.destroyChainSec, 8.f);
    EXPECT_FLOAT_EQ(c.destroyFinishSec, 10.f);
    EXPECT_FALSE(c.tracerProjectile.empty());
    EXPECT_FALSE(c.shellProjectile.empty());
    EXPECT_FALSE(c.blastProjectile.empty());
}

TEST(BossDef, BigShieeProjectilesCarryTheShellAndTracerNumbers)
{
    BossRig rig;
    ASSERT_TRUE(rig.IsReady());
    const ProjectileDef* pTracer = rig.GetLib().FindProjectile("bigshiee_tracer");
    const ProjectileDef* pShell  = rig.GetLib().FindProjectile("bigshiee_shell");
    const ProjectileDef* pBlast  = rig.GetLib().FindProjectile("bigshiee_blast");
    ASSERT_NE(pTracer, nullptr);
    ASSERT_NE(pShell, nullptr);
    ASSERT_NE(pBlast, nullptr);
    EXPECT_FLOAT_EQ(pShell->v0.y, -300.f);
    EXPECT_FLOAT_EQ(pShell->gravity, 500.f);
    EXPECT_FLOAT_EQ(pBlast->size.x, 60.f);            // 착탄 판정 60x60
    EXPECT_FLOAT_EQ(pBlast->size.y, 60.f);
    EXPECT_NEAR(SecToTicks(pBlast->lifetimeSec), 18, 1);   // 0.3 s
    EXPECT_TRUE(pShell->explosion.clip == "Explosion_Mid");
}

TEST(BossDef, BehaviorNameRoundTripsForTheNewBoss)
{
    EXPECT_STREQ(GetBehaviorName(eEnemyBehavior::BigShiee), "bigshiee");
    ASSERT_TRUE(FindBehavior("bigshiee").has_value());
    EXPECT_EQ(*FindBehavior("bigshiee"), eEnemyBehavior::BigShiee);
    EXPECT_FALSE(FindBehavior("battleship").has_value());
}

TEST(BossDef, TurretAndCannonOwnersAreDistinctParts)
{
    const uint32_t id = 2345;
    for (int i = 0; i < bigshiee::kTurretCount; ++i)
    {
        EXPECT_EQ(GetPartIndex(bigshiee::TurretOwner(id, i)), i);
        EXPECT_NE(bigshiee::TurretOwner(id, i), bigshiee::CannonOwner(id));
    }
    EXPECT_EQ(GetPartIndex(bigshiee::CannonOwner(id)), bigshiee::kCannonPart);
    EXPECT_EQ(GetPartIndex(id), -1);   // 선체는 파트가 아니다
}

TEST(BossDef, BigShieeClipNamesExistInBakedAssets)
{
    SLUG_REQUIRE_BAKED(anims);
    const char* kNames[] = {
        "BigShiee_Body_Idle", "BigShiee_Body_DriveForward", "BigShiee_Body_DriveBack",
        "BigShiee_Cannon_Deploy1", "BigShiee_Cannon_Deploy3", "BigShiee_Cannon_Deploy6", "BigShiee_Cannon_Deploy9",
        "BigShiee_Cannon_Idle", "BigShiee_Cannon_DriveForward", "BigShiee_Cannon_DriveBack",
        "BigShiee_Turret_A_Idle", "BigShiee_Turret_G_Idle", "BigShiee_Turret_A_Shooting", "BigShiee_Turret_G_Shooting",
        "BigShiee_Turret_A_Destroyed", "BigShiee_Turret_G_Destroyed",
        "BigShiee_Gun_Aim", "BigShiee_Gun_Shoot_1", "BigShiee_Gun_Shoot_5",
        "BigShiee_Fire_ABCD_1", "BigShiee_Fire_EFG_1", "BigShiee_Projectile_Cyan", "BigShiee_Projectile_Orange",
        "BigShiee_Shell_Flame_1", "BigShiee_Destroyed_Body", "BigShiee_Destroyed_Cannon", "BigShiee_Smoke", "BigShiee_Emblem_Spin_A",
    };
    for (const char* name: kNames)
    {
        EXPECT_NE(anims.FindClip(name), nullptr) << "클립 없음: " << name;
    }
}

TEST(BossDef, GunAimHasNineAngleFramesAndShootRowsAreEightFrames)
{
    SLUG_REQUIRE_BAKED(anims);
    EXPECT_NEAR(GetNamedPartTicks(anims, "BigShiee_Gun_Aim"), 45, 3);        // 9 프레임 x 5 틱
    EXPECT_NEAR(GetNamedPartTicks(anims, "BigShiee_Gun_Shoot_1"), 40, 3);    // 8 프레임 x 5 틱
    EXPECT_NEAR(GetNamedPartTicks(anims, "BigShiee_Turret_A_Shooting"), 70, 4);   // 14 프레임 (사격 예고 0.4 s 는 그 앞부분)
    EXPECT_NEAR(GetNamedPartTicks(anims, "BigShiee_Cannon_Deploy1"), 40, 3);
}
