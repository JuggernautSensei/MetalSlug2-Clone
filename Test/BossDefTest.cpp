#include "Core/AssetPath.h"
#include <gtest/gtest.h>
#include "Base/Types.h"
#include "Base/Result.h"

#include "Game/Enemies/BossShiee.h"
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
    const EnemyDef* pBoss = lib.Find("boss_shiee");
    ASSERT_NE(pBus, nullptr);
    ASSERT_NE(pBoss, nullptr);
    EXPECT_EQ(pBus->behavior, eEnemyBehavior::Bus);
    EXPECT_EQ(pBoss->behavior, eEnemyBehavior::BossShiee);
    EXPECT_EQ(pBus->hp, 380);     // 1P HMG 40 dps x 전투 9.5 s (Docs/Spec_Bus.md)
    EXPECT_EQ(pBoss->hp, 1000);   // 캐논 헤드 체력 (= bossShiee.cannonHp). 포탑 7 x 120 은 파트별 (권총 3 dmg 기준 pistol-only 60~90 s)
    EXPECT_EQ(pBoss->team, eTeam::Enemy);
    EXPECT_TRUE(pBus->bNoDespawn);
    EXPECT_TRUE(pBoss->bNoDespawn);
    EXPECT_TRUE(pBoss->bHitFlash);
    EXPECT_GT(pBoss->score, pBus->score);
    EXPECT_EQ(lib.Find("bigshiee"), nullptr);       // 구 최종보스 정의는 제거되었다
    EXPECT_EQ(lib.FindProjectile("bigshiee_tracer"), nullptr);
}

TEST(BossDef, BusTimingsAndDropLimitsMatchTheSpec)
{
    BossRig rig;
    ASSERT_TRUE(rig.IsReady());
    const BusCfg& c = rig.GetLib().Find("mb_truck")->bus;
    EXPECT_EQ(c.revealTicks, 114);   // 카메라 팬 1.9 s
    EXPECT_EQ(c.openTicks, 86);      // Bus_Opening 10 프레임 @ 7 fps (1.43 s)
    EXPECT_EQ(c.chainTicks, 30);     // 사망 연쇄 0.5 s
    EXPECT_FLOAT_EQ(rig.GetLib().Find("mb_truck")->solid.w, 204.f);   // Bus_Idle 스프라이트 폭 (게임 px)
}

TEST(BossDef, BossShieeConfigMatchesTheTimeline)
{
    BossRig rig;
    ASSERT_TRUE(rig.IsReady());
    const BossShieeCfg& c = rig.GetLib().Find("boss_shiee")->bossShiee;
    EXPECT_FLOAT_EQ(c.originDepth, 208.f);            // 발 → 선체 중심 (416 / 2)
    EXPECT_FLOAT_EQ(c.stationX, 330.f);               // 카메라 왼쪽 기준 선체 중심 (원작 350~420 의 설계 보정)
    EXPECT_FLOAT_EQ(c.entranceFromX, -260.f);         // 등장 시작: 화면 왼쪽 밖
    EXPECT_EQ(c.entranceTicks, 960);                  // 16 s (원작 252~274 s 천천히 굴러 들어옴)
    EXPECT_FLOAT_EQ(c.scrollSpeed, 18.f);             // 엔드리스 드리프트 18 px/s (원작 16~21 game px/s, 끊김 없음)
    EXPECT_EQ(c.scrollOffTicks, 0);
    EXPECT_FLOAT_EQ(c.scrollBudget, 100000.f);        // 사실상 무제한 (엔드리스 아레나)
    EXPECT_EQ(c.turretHp, 120);
    EXPECT_EQ(c.turretTurnTicks + c.turretFireTicks, 75);   // 사격 예고 합계 1.25 s (>= 0.6 s)
    EXPECT_EQ(c.turretShootTicks, 60);
    EXPECT_FLOAT_EQ(c.streakSpeed, 200.f);            // 탄속 <= 260 px/s (느린 탄 = 예고 중 피하기 쉽다)
    EXPECT_EQ(c.deployAfterDown, 4);
    EXPECT_EQ(c.assaultMaxTicks, 1800);               // 30 s 뒤 강제 전개
    EXPECT_EQ(c.deployTicks, 160);                    // 4 단계 x 40 틱
    EXPECT_EQ(c.cannonHp, 1000);
    EXPECT_EQ(c.volleyMinTicks, 150);                 // 볼리 2.5 s ~ 3.5 s
    EXPECT_EQ(c.volleyMaxTicks, 210);
    EXPECT_EQ(c.volleyAimTicks, 45);                  // Gun_Aim 예고 0.75 s (>= 0.6 s)
    EXPECT_EQ(c.volleyStreaks, 3);
    EXPECT_EQ(c.streakStaggerTicks, 15);              // 트레이서 간격 0.25 s
    EXPECT_EQ(c.shellEvery, 2);
    EXPECT_FLOAT_EQ(c.shellFlightSec, 1.f);
    EXPECT_FLOAT_EQ(c.enragedRate, 1.25f);
    EXPECT_EQ(c.cannonBlastTicks, 150);               // 원작 340.4~343 s
    EXPECT_EQ(c.splitTicks, 420);                     // 원작 348~350 s (사망 7 s)
    EXPECT_EQ(c.wreckTicks, 570);
    EXPECT_EQ(c.finishTicks, 600);                    // 사망 10 s 에 IsDestroyFinished
    EXPECT_EQ(c.turretScore, 500);
    EXPECT_EQ(c.cannonScore, 2000);
    EXPECT_FALSE(c.tracerProjectile.empty());
    EXPECT_FALSE(c.shellProjectile.empty());
    EXPECT_FALSE(c.blastProjectile.empty());
}

TEST(BossDef, BossShieeTurretLayoutHasSevenPlacedTurrets)
{
    BossRig rig;
    ASSERT_TRUE(rig.IsReady());
    const BossShieeCfg& c = rig.GetLib().Find("boss_shiee")->bossShiee;
    // 포탑 A..G 는 서로 다른 위치 (같은 자리에 두 포탑이 겹치면 안 된다)
    for (int i = 0; i < kBossTurretCount; ++i)
    {
        for (int j = i + 1; j < kBossTurretCount; ++j)
        {
            const float dx = c.turretOffset[i].x - c.turretOffset[j].x;
            const float dy = c.turretOffset[i].y - c.turretOffset[j].y;
            EXPECT_GT(dx * dx + dy * dy, 40.f * 40.f) << "포탑 " << i << " 와 " << j << " 이 너무 가깝다";
        }
    }
}

TEST(BossDef, BossShieeProjectilesCarryTheShellAndTracerNumbers)
{
    BossRig rig;
    ASSERT_TRUE(rig.IsReady());
    const ProjectileDef* pTracer = rig.GetLib().FindProjectile("boss_tracer");
    const ProjectileDef* pShell  = rig.GetLib().FindProjectile("boss_shell");
    const ProjectileDef* pBlast  = rig.GetLib().FindProjectile("boss_blast");
    ASSERT_NE(pTracer, nullptr);
    ASSERT_NE(pShell, nullptr);
    ASSERT_NE(pBlast, nullptr);
    EXPECT_FLOAT_EQ(pShell->gravity, 1626.f);         // 발사 코드가 vy 를 비행 1.0 s 에서 구한다
    EXPECT_FLOAT_EQ(pTracer->size.x, 14.f);           // 트레이서 판정 14x14
    EXPECT_FLOAT_EQ(pTracer->size.y, 14.f);
    EXPECT_EQ(SecToTicks(pTracer->lifetimeSec), 96);  // 1.6 s
    EXPECT_FLOAT_EQ(pShell->size.x, 16.f);
    EXPECT_FLOAT_EQ(pBlast->size.x, 60.f);            // 착탄 판정 60x60
    EXPECT_FLOAT_EQ(pBlast->size.y, 60.f);
    EXPECT_NEAR(SecToTicks(pBlast->lifetimeSec), 18, 1);   // 0.3 s
    EXPECT_TRUE(pShell->explosion.clip == "Explosion_Mid");
}

TEST(BossDef, BehaviorNameRoundTripsForTheNewBoss)
{
    EXPECT_STREQ(GetBehaviorName(eEnemyBehavior::BossShiee), "boss_shiee");
    ASSERT_TRUE(FindBehavior("boss_shiee").has_value());
    EXPECT_EQ(*FindBehavior("boss_shiee"), eEnemyBehavior::BossShiee);
    EXPECT_FALSE(FindBehavior("bigshiee").has_value());
}

TEST(BossDef, TurretAndCannonOwnersAreDistinctParts)
{
    const uint32_t id = 2345;
    for (int i = 0; i < bossshiee::kTurretCount; ++i)
    {
        EXPECT_EQ(GetPartIndex(bossshiee::TurretOwner(id, i)), i);
        EXPECT_NE(bossshiee::TurretOwner(id, i), bossshiee::CannonOwner(id));
    }
    EXPECT_EQ(GetPartIndex(bossshiee::CannonOwner(id)), bossshiee::kCannonPart);
    EXPECT_EQ(GetPartIndex(id), -1);   // 선체는 파트가 아니다
}

TEST(BossDef, BossShieeClipNamesExistInBakedAssets)
{
    SLUG_REQUIRE_BAKED(anims);
    const char* kNames[] = {
        "Boss_Body_Idle", "Boss_Body_DriveForward", "Boss_Body_DriveBack",
        "Boss_Cannon_Deploy1", "Boss_Cannon_Deploy3", "Boss_Cannon_Deploy6", "Boss_Cannon_Deploy9",
        "Boss_Cannon_Idle", "Boss_Cannon_DriveForward", "Boss_Cannon_DriveBack",
        "Boss_Turret_A_Idle", "Boss_Turret_G_Idle", "Boss_Turret_A_Turning", "Boss_Turret_G_Turning",
        "Boss_Turret_A_Shooting", "Boss_Turret_G_Shooting", "Boss_Turret_A_Destroyed", "Boss_Turret_G_Destroyed",
        "Boss_Turret_B_Idle", "Boss_Turret_C_Idle", "Boss_Turret_D_Idle", "Boss_Turret_E_Idle", "Boss_Turret_F_Idle",
        "Boss_Gun_Aim", "Boss_Gun_Shoot_1", "Boss_Gun_Shoot_5",
        "Boss_Fire_ABCD_1", "Boss_Fire_EFG_1", "Boss_Projectile_Cyan", "Boss_Projectile_Orange",
        "Boss_Shell_Flame_1", "Boss_Destroyed_Body", "Boss_Destroyed_Cannon", "Boss_Wreck", "Boss_Smoke",
        "Explosion_Small", "Explosion_Mid", "Explosion_Large", "Explosion_Huge",
    };
    for (const char* name: kNames)
    {
        EXPECT_NE(anims.FindClip(name), nullptr) << "클립 없음: " << name;
    }
}

TEST(BossDef, GunAimHasNineAngleFramesAndShootRowsAreEightFrames)
{
    SLUG_REQUIRE_BAKED(anims);
    EXPECT_NEAR(GetNamedPartTicks(anims, "Boss_Gun_Aim"), 45, 3);        // 9 프레임 x 5 틱
    EXPECT_NEAR(GetNamedPartTicks(anims, "Boss_Gun_Shoot_1"), 34, 3);    // 8 프레임 @ 14 fps (약 34 틱)
    EXPECT_NEAR(GetNamedPartTicks(anims, "Boss_Cannon_Deploy1"), 40, 3);
}
