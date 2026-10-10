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

#include <gtest/gtest.h>

#include "BossTestUtil.h"

using namespace slug;
using namespace slugtest;

// 버스 (mb_truck): 주차 -> startBattle (카메라 팬 1.9 s) -> 개방 1.43 s -> 전투 -> 사망 연쇄 0.5 s -> 잔해.
// 원작 확인 사항 (Docs/Spec_Bus.md): 포탄/병사/GO! 없음, 주차 중 피격 없음, 잔해는 걸을 수 있다.

namespace
{
    constexpr int   kParked     = 0;
    constexpr int   kReveal     = 1;
    constexpr int   kOpening    = 2;
    constexpr int   kBattle     = 3;
    constexpr int   kRevealTicks = 114;   // 1.9 s
    constexpr int   kOpenTicks   = 86;    // 1.43 s
    constexpr int   kMaxHp       = 380;

    constexpr float kBusX     = 600.f;
    constexpr float kBusFeetY = 378.f;

    EnemyHandle SpawnBus(BossRig& _rig)
    {
        SpawnParams params;
        params.facing = -1;
        return _rig.SpawnAt("mb_truck", { kBusX, kBusFeetY }, params);
    }

    void StartBattle(BossRig& _rig, const EnemyHandle _bus)
    {
        _rig.World().SendCommand(_bus, "startBattle");
        _rig.Frame();
    }

    // 명령 후 팬 + 개방이 끝날 때까지 진행한다 (전투 상태 진입).
    void ToBattle(BossRig& _rig, const EnemyHandle _bus)
    {
        StartBattle(_rig, _bus);
        _rig.Run(kRevealTicks + kOpenTicks + 2);
        ASSERT_EQ(_rig.Find(_bus)->state, kBattle);
    }

    void HitTick(BossRig& _rig, const uint32_t _id, const int _damage)
    {
        _rig.Hit(_id, _damage);
        _rig.Frame();
    }

    int CountDiedWithTag(BossRig& _rig, const EnemyHandle _bus)
    {
        int n = 0;
        for (const EnemyEvent& ev: _rig.Events())
        {
            n += (ev.kind == eEnemyEventKind::Died && ev.handle == _bus) ? 1 : 0;
        }
        return n;
    }
}

TEST(Bus, ParkedBusIgnoresDamageAndStaysSolid)
{
    BossRig rig;
    ASSERT_TRUE(rig.IsReady());
    const EnemyHandle bus = SpawnBus(rig);
    rig.Run(5);
    HitTick(rig, bus.id, 50);
    EXPECT_EQ(rig.Find(bus)->state, kParked);
    EXPECT_EQ(rig.Find(bus)->health.hp, kMaxHp);
    EXPECT_EQ(rig.World().GetSolidBoxes().size(), 1u);
}

TEST(Bus, StartBattleWaitsForTheCameraPanThenOpens)
{
    BossRig rig;
    ASSERT_TRUE(rig.IsReady());
    const EnemyHandle bus = SpawnBus(rig);
    rig.Run(5);
    StartBattle(rig, bus);
    EXPECT_EQ(rig.Find(bus)->state, kReveal);
    rig.Run(kRevealTicks - 2);
    EXPECT_EQ(rig.Find(bus)->state, kReveal);
    rig.Run(2);
    EXPECT_EQ(rig.Find(bus)->state, kOpening);
}

TEST(Bus, OpeningLastsOneAndAHalfSecondsThenTheBattleStarts)
{
    BossRig rig;
    ASSERT_TRUE(rig.IsReady());
    const EnemyHandle bus = SpawnBus(rig);
    rig.Run(5);
    StartBattle(rig, bus);
    rig.Run(kRevealTicks);
    ASSERT_EQ(rig.Find(bus)->state, kOpening);
    rig.Run(kOpenTicks - 2);
    EXPECT_EQ(rig.Find(bus)->state, kOpening);
    rig.Run(2);
    EXPECT_EQ(rig.Find(bus)->state, kBattle);
    EXPECT_LE(rig.Find(bus)->stateTicks, 1);
}

TEST(Bus, OnlyTheStartBattleCommandStartsTheReveal)
{
    BossRig rig;
    ASSERT_TRUE(rig.IsReady());
    const EnemyHandle bus = SpawnBus(rig);
    rig.Run(5);
    rig.World().SendCommand(bus, "somethingElse");
    rig.Frame();
    EXPECT_EQ(rig.Find(bus)->state, kParked);
}

TEST(Bus, DamageIsAcceptedOnlyOnceTheOpeningHasBegun)
{
    BossRig rig;
    ASSERT_TRUE(rig.IsReady());
    const EnemyHandle bus = SpawnBus(rig);
    rig.Run(5);
    StartBattle(rig, bus);
    rig.Run(kRevealTicks - 2);
    HitTick(rig, bus.id, 10);
    EXPECT_EQ(rig.Find(bus)->health.hp, kMaxHp);   // 팬 중에는 무적
    rig.Run(4);
    HitTick(rig, bus.id, 10);
    EXPECT_EQ(rig.Find(bus)->health.hp, kMaxHp - 10);
}

TEST(Bus, DamageStagesAdvanceEveryEighthOfItsHitPoints)
{
    BossRig rig;
    ASSERT_TRUE(rig.IsReady());
    const EnemyHandle bus = SpawnBus(rig);
    ToBattle(rig, bus);
    ASSERT_EQ(rig.Find(bus)->health.maxHp, kMaxHp);
    EXPECT_EQ(rig.Find(bus)->aux, 0);

    HitTick(rig, bus.id, 47);   // hp 333: 아직 0단계 (333 * 8 > 380 * 7)
    EXPECT_EQ(rig.Find(bus)->aux, 0);
    HitTick(rig, bus.id, 1);    // hp 332: 1단계 (<= 7/8)
    EXPECT_EQ(rig.Find(bus)->aux, 1);
    HitTick(rig, bus.id, 47);   // hp 285: 2단계 (<= 6/8)
    EXPECT_EQ(rig.Find(bus)->aux, 2);
    HitTick(rig, bus.id, 95);   // hp 190: 4단계 (<= 4/8)
    EXPECT_EQ(rig.Find(bus)->health.hp, 190);
    EXPECT_EQ(rig.Find(bus)->aux, 4);
}

TEST(Bus, DeathStopsEverythingAndEmitsOneDiedEvent)
{
    BossRig rig;
    ASSERT_TRUE(rig.IsReady());
    const EnemyHandle bus = SpawnBus(rig);
    ToBattle(rig, bus);
    rig.Run(30);

    HitTick(rig, bus.id, kMaxHp);
    EXPECT_EQ(CountDiedWithTag(rig, bus), 1);
    const Enemy* pBus = rig.Find(bus);
    ASSERT_NE(pBus, nullptr);
    EXPECT_EQ(pBus->life, eEnemyLife::Dying);
    EXPECT_EQ(pBus->aux, 8);
    EXPECT_TRUE(rig.World().IsDefeated(bus));

    rig.Run(300);
    EXPECT_EQ(CountDiedWithTag(rig, bus), 1);
    EXPECT_EQ(rig.Find(bus)->shotsFired, 0);
}

TEST(Bus, DeathCarriesTheConfiguredScore)
{
    BossRig rig;
    ASSERT_TRUE(rig.IsReady());
    const EnemyHandle bus = SpawnBus(rig);
    ToBattle(rig, bus);
    rig.Hit(bus.id, kMaxHp);
    bool bFound = false;
    for (const EnemyEvent& ev: rig.Events())
    {
        if (ev.kind == eEnemyEventKind::Died && ev.handle == bus)
        {
            bFound = true;
            EXPECT_EQ(ev.score, 2000);
            EXPECT_TRUE(ev.defName == "mb_truck");
        }
    }
    EXPECT_TRUE(bFound);
}

TEST(Bus, WreckIsWalkableAndTheStageGateSeesTheDeathAtOnce)
{
    BossRig rig;
    ASSERT_TRUE(rig.IsReady());
    const EnemyHandle bus = SpawnBus(rig);
    ToBattle(rig, bus);
    ASSERT_EQ(rig.World().GetSolidBoxes().size(), 1u);

    rig.Hit(bus.id, kMaxHp);
    rig.Run(1);
    EXPECT_TRUE(rig.World().GetSolidBoxes().empty());   // 잔해는 벽이 아니다
    EXPECT_TRUE(rig.World().IsDefeated(bus));
    EXPECT_TRUE(rig.World().IsDestroyFinished(bus));    // 잔해는 남지만 연출 대기는 없다
}

TEST(Bus, SolidBoxCoversTheBodyOnlyWhileTheBusIsAlive)
{
    BossRig rig;
    ASSERT_TRUE(rig.IsReady());
    const EnemyHandle bus = SpawnBus(rig);
    rig.Run(2);

    const slug::Vector<Aabb> boxes = rig.World().GetSolidBoxes();
    ASSERT_EQ(boxes.size(), 1u);
    EXPECT_FLOAT_EQ(boxes[0].min.x, kBusX - 102.f);
    EXPECT_FLOAT_EQ(boxes[0].max.x, kBusX + 102.f);
    EXPECT_FLOAT_EQ(boxes[0].min.y, kBusFeetY - 260.f);
    EXPECT_FLOAT_EQ(boxes[0].max.y, kBusFeetY);

    rig.Hit(bus.id, 1);
    rig.Frame();
    EXPECT_EQ(rig.World().GetSolidBoxes().size(), 1u);   // 주차 중에도 벽
}

TEST(Bus, SolidStaysThroughTheOpeningAndTheBattle)
{
    BossRig rig;
    ASSERT_TRUE(rig.IsReady());
    const EnemyHandle bus = SpawnBus(rig);
    ToBattle(rig, bus);
    EXPECT_EQ(rig.World().GetSolidBoxes().size(), 1u);
}

TEST(Bus, NoShellsOrSoldiersAreEverFiredDuringTheBattle)
{
    BossRig rig;
    ASSERT_TRUE(rig.IsReady());
    const EnemyHandle bus = SpawnBus(rig);
    ToBattle(rig, bus);
    rig.SetPlayerInvulnerable(false);
    rig.SetPlayer({ kBusX - 300.f, kBusFeetY });
    rig.Run(900);
    EXPECT_EQ(rig.Find(bus)->shotsFired, 0);
    EXPECT_EQ(rig.World().CountAlive(EnemyFilter {}), 1);   // 버스 자신뿐
}
