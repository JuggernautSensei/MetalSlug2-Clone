// AI 탭의 UI 없는 부분: 스폰/전멸/재스폰/정의 리로드와 일시정지-스텝 제어.

#include <cmath>
#include <memory>

#include <gtest/gtest.h>

#include "Core/AssetPath.h"
#include "Editor/EnemyLab.h"
#include "EnemyTestUtil.h"
#include "Game/SimRunner.h"

TEST(EnemyLabTest, SpawnByNameCreatesOneEnemyAtThePosition)
{
    Sim               sim { MakeEnemyConfig() };
    EnemyLab          lab;
    const size_t      before = sim.State().GetEnemies().GetCount();
    const EnemyHandle h      = lab.Spawn(sim.State(), "soldier_sword", { 400.f, 378.f }, 1);
    ASSERT_FALSE(h.IsNull());
    EXPECT_EQ(sim.State().GetEnemies().GetCount(), before + 1);
    const Enemy* pEnemy = sim.State().GetEnemies().Get(h);
    ASSERT_NE(pEnemy, nullptr);
    EXPECT_NEAR(pEnemy->body.position.x, 400.f, 0.5f);
    EXPECT_NEAR(pEnemy->body.position.y, 378.f, 0.5f);
    EXPECT_EQ(pEnemy->facing, 1);
    EXPECT_EQ(lab.GetSetCount(), 1u);
}

TEST(EnemyLabTest, UnknownDefSpawnsNothing)
{
    Sim      sim { MakeEnemyConfig() };
    EnemyLab lab;
    EXPECT_TRUE(lab.Spawn(sim.State(), "no_such_def", { 400.f, 378.f }, 1).IsNull());
    EXPECT_EQ(sim.State().GetEnemies().GetCount(), 0u);
    EXPECT_EQ(lab.GetSetCount(), 0u);
}

TEST(EnemyLabTest, SpawnNearPlayerUsesTheOffset)
{
    Sim               sim { MakeEnemyConfig() };
    EnemyLab          lab;
    const Vec2        feet   = sim.State().GetPlayer().GetFeet();
    const EnemyHandle h      = lab.SpawnNearPlayer(sim.State(), "soldier_sword", 150.f, -1);
    const Enemy*      pEnemy = sim.State().GetEnemies().Get(h);
    ASSERT_NE(pEnemy, nullptr);
    EXPECT_NEAR(pEnemy->body.position.x, feet.x + 150.f, 0.5f);
}

TEST(EnemyLabTest, KillAllAndClearEmptyTheWorld)
{
    Sim      sim { MakeEnemyConfig() };
    EnemyLab lab;
    (void)lab.Spawn(sim.State(), "soldier_sword", { 400.f, 378.f }, -1);
    (void)lab.Spawn(sim.State(), "soldier_bomb", { 460.f, 378.f }, -1);
    EXPECT_EQ(sim.State().GetEnemies().CountAlive(), 2);
    lab.KillAll(sim.State());
    EXPECT_EQ(sim.State().GetEnemies().CountAlive(), 0);

    (void)lab.Spawn(sim.State(), "soldier_sword", { 400.f, 378.f }, -1);
    lab.Clear(sim.State());
    EXPECT_EQ(sim.State().GetEnemies().GetCount(), 0u);
}

TEST(EnemyLabTest, RespawnLastRestoresTheSet)
{
    Sim      sim { MakeEnemyConfig() };
    EnemyLab lab;
    (void)lab.Spawn(sim.State(), "soldier_sword", { 400.f, 378.f }, -1);
    (void)lab.Spawn(sim.State(), "soldier_bomb", { 460.f, 378.f }, 1);
    lab.Clear(sim.State());
    EXPECT_EQ(lab.RespawnLast(sim.State()), 2);
    EXPECT_EQ(sim.State().GetEnemies().GetCount(), 2u);
    EXPECT_EQ(lab.GetSetCount(), 2u);
}

TEST(EnemyLabTest, SelectAtPicksTheEnemyUnderThePoint)
{
    Sim               sim { MakeEnemyConfig() };
    EnemyLab          lab;
    const EnemyHandle h = lab.Spawn(sim.State(), "soldier_sword", { 400.f, 378.f }, -1);
    lab.SelectAt(sim.State(), { 400.f, 360.f });
    EXPECT_EQ(lab.GetSelected(), h);
    lab.SelectAt(sim.State(), { 900.f, 100.f });
    EXPECT_TRUE(lab.GetSelected().IsNull());
}

TEST(EnemyLabTest, PauseAndStepAdvanceExactlyTheRequestedTicks)
{
    EnemyLab lab;
    EXPECT_EQ(lab.TakeSteps(3), 3);
    lab.SetPaused(true);
    EXPECT_EQ(lab.TakeSteps(3), 0);
    lab.RequestSteps(1);
    EXPECT_EQ(lab.TakeSteps(3), 1);
    EXPECT_EQ(lab.TakeSteps(3), 0);
    lab.RequestSteps(10);
    EXPECT_EQ(lab.TakeSteps(0), 10);
    EXPECT_EQ(lab.TakeSteps(0), 0);
    lab.SetPaused(false);
    EXPECT_EQ(lab.TakeSteps(2), 2);
}

TEST(EnemyLabTest, SteppingTheWorldMovesTicksByExactlyN)
{
    Sim      sim { MakeEnemyConfig() };
    EnemyLab lab;
    lab.SetPaused(true);
    lab.RequestSteps(10);
    const uint64_t before = sim.State().GetTickCount();
    sim.Step(lab.TakeSteps(5));
    EXPECT_EQ(sim.State().GetTickCount(), before + 10);
    sim.Step(lab.TakeSteps(5));
    EXPECT_EQ(sim.State().GetTickCount(), before + 10);
}

TEST(EnemyLabTest, ReloadDefsKeepsTheWorldValidAndRespawnsTheSet)
{
    Sim      sim { MakeEnemyConfig() };
    EnemyLab lab;
    lab.Init(nullptr, "sandbox", FindAssetRoot());
    (void)lab.Spawn(sim.State(), "soldier_sword", { 400.f, 378.f }, -1);
    sim.Step(5);
    ASSERT_TRUE(lab.ReloadDefs(sim.State()));
    EXPECT_GT(sim.State().GetEnemyLib().GetDefCount(), 0u);
    EXPECT_EQ(sim.State().GetEnemies().GetCount(), 1u);
    sim.Step(60);
    EXPECT_EQ(sim.State().GetEnemies().CountAlive(), 1);
    EXPECT_FALSE(lab.Spawn(sim.State(), "soldier_bomb", { 500.f, 378.f }, -1).IsNull());
}

TEST(EnemyLabTest, EnterAndLeaveSwitchTheRunnerState)
{
    SimRunner runner { Scoped<IGameState> { new SandboxState { MakeEnemyConfig() } } };
    EnemyLab  lab;
    lab.Init(&runner, "sandbox", FindAssetRoot());
    IGameState* pFirst = runner.GetState();
    ASSERT_NE(lab.GetSandbox(), nullptr);
    lab.EnterLab();
    EXPECT_NE(runner.GetState(), pFirst);
    ASSERT_NE(lab.GetSandbox(), nullptr);
    EXPECT_GT(lab.GetSandbox()->GetEnemies().GetCount(), 0u);
    lab.LeaveLab();
    ASSERT_NE(lab.GetSandbox(), nullptr);
}
