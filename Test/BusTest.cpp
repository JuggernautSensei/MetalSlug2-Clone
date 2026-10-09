#include <gtest/gtest.h>

#include "BossTestUtil.h"

using namespace slug;
using namespace slugtest;

namespace
{
    constexpr float kBusX      = 600.f;
    constexpr float kBusFeetY  = 378.f;
    constexpr float kFarX      = 100.f;   // 트리거에서 떨어진 플레이어 위치
    constexpr float kTriggerX  = 385.f;   // 트리거에 닿는 플레이어 발 x

    EnemyHandle SpawnBus(BossRig& _rig)
    {
        SpawnParams params;
        params.facing = -1;
        return _rig.SpawnAt("mb_truck", { kBusX, kBusFeetY }, params);
    }

    void OpenBus(BossRig& _rig)
    {
        _rig.SetPlayer({ kTriggerX, kBusFeetY });
        _rig.Frame();
        _rig.SetPlayer({ kFarX, kBusFeetY });
    }

    int SoldiersAlive(BossRig& _rig, const EnemyHandle _bus)
    {
        EnemyFilter filter;
        filter.defIndex = _rig.GetLib().FindIndex("soldier_bomb");
        filter.userTag  = _bus.id;
        return _rig.World().CountAlive(filter);
    }

    std::vector<EnemyHandle> SoldierHandles(BossRig& _rig, const EnemyHandle _bus)
    {
        std::vector<EnemyHandle> out;
        const int defIndex = _rig.GetLib().FindIndex("soldier_bomb");
        for (const Enemy& e: _rig.World().GetEnemies())
        {
            if (e.defIndex == defIndex && e.userTag == _bus.id && e.life == eEnemyLife::Alive)
            {
                out.push_back({ e.id });
            }
        }
        return out;
    }

    void HitTick(BossRig& _rig, const uint32_t _id, const int _damage)
    {
        _rig.Hit(_id, _damage);
        _rig.Frame();
    }

    int CountDiedWithTag(BossRig& _rig, const uint32_t _tag)
    {
        int n = 0;
        for (const EnemyEvent& ev: _rig.Events())
        {
            n += (ev.kind == eEnemyEventKind::Died && ev.userTag == _tag) ? 1 : 0;
        }
        return n;
    }
}

TEST(Bus, StaysClosedUntilThePlayerTouchesTheTrigger)
{
    BossRig rig;
    ASSERT_TRUE(rig.IsReady());
    const EnemyHandle bus = SpawnBus(rig);
    rig.SetPlayer({ kFarX, kBusFeetY });
    rig.Run(300);
    const Enemy* pBus = rig.Find(bus);
    ASSERT_NE(pBus, nullptr);
    EXPECT_EQ(pBus->state, 0);            // kClosed
    EXPECT_EQ(pBus->aux2, 0);             // 포탄 0발
    EXPECT_EQ(pBus->shotsFired, 0);
}

TEST(Bus, TouchingTheTriggerOpensTheDoorAndStartsTheBattle)
{
    BossRig rig;
    ASSERT_TRUE(rig.IsReady());
    const EnemyHandle bus = SpawnBus(rig);
    rig.Run(5);
    OpenBus(rig);
    const Enemy* pBus = rig.Find(bus);
    ASSERT_NE(pBus, nullptr);
    EXPECT_EQ(pBus->state, 1);            // kBattle
    EXPECT_LE(pBus->stateTicks, 1);       // 전투 시계는 0 에서 시작
}

TEST(Bus, TheTriggerOnlyCountsWhileThePlayerIsAlive)
{
    BossRig rig;
    ASSERT_TRUE(rig.IsReady());
    const EnemyHandle bus = SpawnBus(rig);
    rig.Run(2);
    rig.SetPlayer({ kTriggerX, kBusFeetY }, false);
    rig.Frame();
    EXPECT_EQ(rig.Find(bus)->state, 0);
}

TEST(Bus, ShellsFireTwoSecondsIntoTheBattleThenEveryThreeSeconds)
{
    BossRig rig;
    ASSERT_TRUE(rig.IsReady());
    const EnemyHandle bus = SpawnBus(rig);
    rig.Run(5);
    OpenBus(rig);

    int firstAt  = -1;
    int secondAt = -1;
    for (int i = 0; i < 400 && secondAt < 0; ++i)
    {
        rig.Frame();
        const Enemy* pBus = rig.Find(bus);
        if (pBus->shotsFired >= 1 && firstAt < 0)
        {
            firstAt = pBus->stateTicks;
        }
        if (pBus->shotsFired >= 2 && secondAt < 0)
        {
            secondAt = pBus->stateTicks;
        }
    }
    EXPECT_GE(firstAt, 119);    // 2 s
    EXPECT_LE(firstAt, 123);
    EXPECT_GE(secondAt - firstAt, 178);   // 이후 3 s 간격
    EXPECT_LE(secondAt - firstAt, 182);
}

TEST(Bus, ShellIsLaunchedLeftwardAtThePlayerWithTheBallisticArc)
{
    BossRig rig;
    ASSERT_TRUE(rig.IsReady());
    const EnemyHandle bus = SpawnBus(rig);
    rig.Run(5);
    OpenBus(rig);
    rig.Run(130);
    ASSERT_GE(rig.Find(bus)->shotsFired, 1);

    bool bFound = false;
    for (const Shot& shot: rig.Projectiles().GetShots())
    {
        if (shot.params.owner != bus.id)
        {
            continue;
        }
        bFound = true;
        EXPECT_LT(shot.params.velocity.x, 0.f);                 // 플레이어 쪽(왼쪽)
        EXPECT_LE(std::abs(shot.params.velocity.x), 420.f + 0.5f);
        EXPECT_GE(std::abs(shot.params.velocity.x), 80.f - 0.5f);
        EXPECT_FLOAT_EQ(shot.params.velocity.y, -200.f);       // 포물선 v0
        EXPECT_FLOAT_EQ(shot.params.gravity, 500.f);
        EXPECT_EQ(shot.params.team, eTeam::Enemy);
        break;
    }
    EXPECT_TRUE(bFound);
}

TEST(Bus, ASoldierIsDroppedThreeSecondsIntoTheBattle)
{
    BossRig rig;
    ASSERT_TRUE(rig.IsReady());
    const EnemyHandle bus = SpawnBus(rig);
    rig.Run(5);
    OpenBus(rig);

    int spawnedAt = -1;
    for (int i = 0; i < 300 && spawnedAt < 0; ++i)
    {
        rig.Frame();
        if (SoldiersAlive(rig, bus) >= 1)
        {
            spawnedAt = rig.Find(bus)->stateTicks;
        }
    }
    EXPECT_GE(spawnedAt, 179);   // 3 s
    EXPECT_LE(spawnedAt, 184);
}

TEST(Bus, AtMostTwoDropsAreAliveAtOnceAndKillingThemFreesSlots)
{
    BossRig rig;
    ASSERT_TRUE(rig.IsReady());
    const EnemyHandle bus = SpawnBus(rig);
    rig.Run(5);
    OpenBus(rig);
    rig.Run(1200);   // 3 s + 5 s 간격 여러 번
    EXPECT_EQ(SoldiersAlive(rig, bus), 2);   // 생존 상한 2

    for (const EnemyHandle h: SoldierHandles(rig, bus))
    {
        rig.Hit(h.id, 1);
    }
    EXPECT_EQ(SoldiersAlive(rig, bus), 0);
    rig.Run(360);
    EXPECT_GE(SoldiersAlive(rig, bus), 1);
}

TEST(Bus, TotalDropsAreCappedAtEightEvenWhenAllAreKilled)
{
    BossRig rig;
    ASSERT_TRUE(rig.IsReady());
    const EnemyHandle bus = SpawnBus(rig);
    rig.Run(5);
    OpenBus(rig);
    for (int i = 0; i < 60 * 60; ++i)
    {
        for (const EnemyHandle h: SoldierHandles(rig, bus))
        {
            rig.Hit(h.id, 1);
        }
        rig.Frame();
    }
    EXPECT_EQ(rig.Find(bus)->aux3, 8);       // 누적 투하 8
    EXPECT_EQ(CountDiedWithTag(rig, bus.id), 8);
    EXPECT_EQ(SoldiersAlive(rig, bus), 0);
}

TEST(Bus, DamageStagesAdvanceEveryEighthOfItsHitPoints)
{
    BossRig rig;
    ASSERT_TRUE(rig.IsReady());
    const EnemyHandle bus = SpawnBus(rig);
    rig.Run(2);
    ASSERT_EQ(rig.Find(bus)->health.maxHp, 250);
    EXPECT_EQ(rig.Find(bus)->aux, 0);

    HitTick(rig, bus.id, 31);   // hp 219: 아직 1단계 전 (219 * 8 > 250 * 7)
    EXPECT_EQ(rig.Find(bus)->aux, 0);
    HitTick(rig, bus.id, 1);    // hp 218: 1단계 (<= 7/8)
    EXPECT_EQ(rig.Find(bus)->aux, 1);
    HitTick(rig, bus.id, 31);   // hp 187: 2단계 (<= 6/8 = 187.5)
    EXPECT_EQ(rig.Find(bus)->aux, 2);
    HitTick(rig, bus.id, 62);   // hp 125: 4단계 (<= 4/8)
    EXPECT_EQ(rig.Find(bus)->health.hp, 125);
    EXPECT_EQ(rig.Find(bus)->aux, 4);
}

TEST(Bus, DamageWorksWhileTheDoorIsStillClosed)
{
    BossRig rig;
    ASSERT_TRUE(rig.IsReady());
    const EnemyHandle bus = SpawnBus(rig);
    rig.Run(2);
    HitTick(rig, bus.id, 50);
    EXPECT_EQ(rig.Find(bus)->health.hp, 200);
    EXPECT_EQ(rig.Find(bus)->state, 0);    // 여전히 닫힘
    EXPECT_EQ(rig.Find(bus)->aux, 1);
}

TEST(Bus, DeathEmitsOneDiedEventAndStopsAllAttacks)
{
    BossRig rig;
    ASSERT_TRUE(rig.IsReady());
    const EnemyHandle bus = SpawnBus(rig);
    rig.Run(5);
    OpenBus(rig);
    rig.Run(130);
    const int shotsBefore = rig.Find(bus)->shotsFired;

    rig.Hit(bus.id, 250);
    EXPECT_EQ(rig.CountEventsOf(eEnemyEventKind::Died, bus), 1);
    const Enemy* pBus = rig.Find(bus);
    ASSERT_NE(pBus, nullptr);
    EXPECT_EQ(pBus->life, eEnemyLife::Dying);
    EXPECT_EQ(pBus->aux, 8);   // 사망 단계
    EXPECT_TRUE(rig.World().IsDefeated(bus));

    rig.Run(900);
    EXPECT_EQ(rig.Find(bus)->shotsFired, shotsBefore);
    EXPECT_EQ(rig.CountEventsOf(eEnemyEventKind::Died, bus), 1);   // Died 는 한 번
    EXPECT_EQ(SoldiersAlive(rig, bus), 0);
}

TEST(Bus, DeathCarriesTheConfiguredScore)
{
    BossRig rig;
    ASSERT_TRUE(rig.IsReady());
    const EnemyHandle bus = SpawnBus(rig);
    rig.Run(2);
    rig.Hit(bus.id, 250);
    for (const EnemyEvent& ev: rig.Events())
    {
        if (ev.kind == eEnemyEventKind::Died && ev.handle == bus)
        {
            EXPECT_EQ(ev.score, 2000);
            EXPECT_TRUE(ev.defName == "mb_truck");
        }
    }
}

TEST(Bus, DeathIsDestroyFinishedImmediatelyAsTheWreckStays)
{
    BossRig rig;
    ASSERT_TRUE(rig.IsReady());
    const EnemyHandle bus = SpawnBus(rig);
    rig.Run(2);
    rig.Hit(bus.id, 250);
    EXPECT_TRUE(rig.World().IsDestroyFinished(bus));
}

TEST(Bus, ShellKillsAnExposedPlayerStandingInItsPath)
{
    BossRig rig;
    ASSERT_TRUE(rig.IsReady());
    const EnemyHandle bus = SpawnBus(rig);
    rig.Run(5);
    OpenBus(rig);
    rig.SetPlayerInvulnerable(false);
    rig.SetPlayer({ 300.f, kBusFeetY });
    rig.Run(400);

    bool bKilledByBus = false;
    for (const HitEvent& ev: rig.PlayerHits())
    {
        bKilledByBus = bKilledByBus || (ev.attacker == bus.id && ev.kind == eHitKind::Bullet);
    }
    EXPECT_TRUE(bKilledByBus);
}
