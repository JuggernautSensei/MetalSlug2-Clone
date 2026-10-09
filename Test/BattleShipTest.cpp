#include <cstdio>

#include <gtest/gtest.h>

#include "BossTestUtil.h"
#include "PlayerTestUtil.h"

using namespace slug;
using namespace slugtest;

namespace
{
    constexpr float kGroundY = 378.f;

    EnemyHandle SpawnShip(BossRig& _rig)
    {
        SpawnParams params;
        params.facing = 1;
        params.arena  = BossArena();
        return _rig.SpawnAt("battleship", { BossArena().min.x - 300.f, kGroundY }, params);
    }

    void RunUntilFight(BossRig& _rig, const EnemyHandle _ship, const int _maxTicks = 900)
    {
        for (int i = 0; i < _maxTicks; ++i)
        {
            if (battleship::IsFightStarted(*_rig.Find(_ship)))
            {
                return;
            }
            _rig.Frame();
        }
    }

    void KillTurrets(BossRig& _rig, const EnemyHandle _ship)
    {
        for (int i = 0; i < battleship::kTurretCount; ++i)
        {
            _rig.Hit(battleship::TurretOwner(_ship.id, i), 40);
        }
    }

    struct Phase
    {
        int state;
        int ticks;
    };

    std::vector<Phase> RecordPhases(BossRig& _rig, const EnemyHandle _ship, const int _frames)
    {
        std::vector<Phase> out;
        int                current = -1;
        int                dwell   = 0;
        for (int i = 0; i < _frames; ++i)
        {
            _rig.Frame();
            const int state = _rig.Find(_ship)->state;
            if (state != current)
            {
                if (current >= 0)
                {
                    out.push_back({ current, dwell });
                }
                current = state;
                dwell   = 0;
            }
            ++dwell;
        }
        return out;
    }
}

TEST(BattleShip, EntranceRollsInFromLeftOfTheArenaInAboutFiveSeconds)
{
    BossRig rig;
    ASSERT_TRUE(rig.IsReady());
    const EnemyHandle ship = SpawnShip(rig);
    rig.Frame();
    EXPECT_NEAR(rig.Find(ship)->body.position.x, -300.f, 1.f);   // 아레나 왼쪽 밖에서 시작

    int ticks = 0;
    while (!battleship::IsFightStarted(*rig.Find(ship)) && ticks < 1000)
    {
        rig.Frame();
        ++ticks;
    }
    EXPECT_GE(ticks, 270);   // 4.5 s 이상
    EXPECT_LE(ticks, 390);   // 6.5 s 이내
    EXPECT_NEAR(rig.Find(ship)->body.position.x, 100.f, 0.6f);   // 홈 슬롯 = 아레나 왼쪽 + 100
}

TEST(BattleShip, FightStartsOnlyWhenTheEntranceEndsInIdle)
{
    BossRig rig;
    ASSERT_TRUE(rig.IsReady());
    const EnemyHandle ship = SpawnShip(rig);
    rig.Run(30);
    EXPECT_FALSE(battleship::IsFightStarted(*rig.Find(ship)));
    EXPECT_EQ(rig.Find(ship)->state, battleship::kEntrance);
    RunUntilFight(rig, ship);
    EXPECT_TRUE(battleship::IsFightStarted(*rig.Find(ship)));
    EXPECT_EQ(rig.Find(ship)->state, battleship::kIdle);
}

TEST(BattleShip, IdleLastsOneAndAHalfSecondsBeforeTheNextPattern)
{
    BossRig rig;
    ASSERT_TRUE(rig.IsReady());
    const EnemyHandle ship = SpawnShip(rig);
    RunUntilFight(rig, ship);
    EXPECT_EQ(battleship::GetPatternCount(*rig.Find(ship)), 0);
    rig.Run(90);
    EXPECT_EQ(battleship::GetPatternCount(*rig.Find(ship)), 0);   // 1.5 s 로는 아직
    rig.Run(10);
    EXPECT_EQ(battleship::GetPatternCount(*rig.Find(ship)), 1);   // 1.6 s 에 첫 패턴 (캐논)
    EXPECT_EQ(rig.Find(ship)->state, battleship::kHeadOpen);
}

TEST(BattleShip, CannonSequenceFollowsTheClipTiming)
{
    BossRig rig;
    ASSERT_TRUE(rig.IsReady());
    const EnemyHandle ship = SpawnShip(rig);
    RunUntilFight(rig, ship);
    std::vector<Phase> phases = RecordPhases(rig, ship, 1000);
    ASSERT_FALSE(phases.empty());
    ASSERT_EQ(phases[0].state, battleship::kIdle);
    phases.erase(phases.begin());

    std::vector<int> want = { battleship::kHeadOpen, battleship::kCannonStart };
    for (int k = 0; k < 3; ++k)
    {
        want.push_back(battleship::kCannonIdle);
        want.push_back(battleship::kCannonShot);
    }
    want.push_back(battleship::kCannonEnd);
    want.push_back(battleship::kHeadClose);
    ASSERT_GE(phases.size(), want.size());
    for (size_t i = 0; i < want.size(); ++i)
    {
        EXPECT_EQ(phases[i].state, want[i]) << "phase " << i;
    }
    EXPECT_NEAR(phases[0].ticks, 86, 1);    // HeadOpen (1.44 s)
    EXPECT_NEAR(phases[1].ticks, 96, 1);    // CannonStart (1.6 s)
    EXPECT_NEAR(phases[2].ticks, 60, 1);    // CannonIdle (1.0 s)
    EXPECT_NEAR(phases[3].ticks, 34, 1);    // CannonShot (0.56 s)
    EXPECT_NEAR(phases[8].ticks, 67, 1);    // CannonEnd (1.12 s)
    EXPECT_NEAR(phases[9].ticks, 101, 1);   // HeadClose (1.68 s)
}

TEST(BattleShip, CannonFiresThreeShotsWhenNotEnraged)
{
    BossRig rig;
    ASSERT_TRUE(rig.IsReady());
    const EnemyHandle ship = SpawnShip(rig);
    RunUntilFight(rig, ship);
    KillTurrets(rig, ship);
    const int before = rig.Find(ship)->shotsFired;
    rig.Run(600);
    EXPECT_EQ(rig.Find(ship)->shotsFired - before, 3);
}

TEST(BattleShip, CannonFiresFiveShotsWhenEnraged)
{
    BossRig rig;
    ASSERT_TRUE(rig.IsReady());
    const EnemyHandle ship = SpawnShip(rig);
    RunUntilFight(rig, ship);
    rig.Hit(ship.id, 450);   // HP 450 = 최대의 1/2 → 격앙
    ASSERT_TRUE(battleship::IsEnraged(*rig.Find(ship)));
    KillTurrets(rig, ship);
    const int before = rig.Find(ship)->shotsFired;
    rig.Run(600);
    EXPECT_EQ(rig.Find(ship)->shotsFired - before, 5);
}

TEST(BattleShip, EnragedIdleIsShorter)
{
    BossRig rig;
    ASSERT_TRUE(rig.IsReady());
    const EnemyHandle ship = SpawnShip(rig);
    RunUntilFight(rig, ship);
    rig.Hit(ship.id, 450);
    rig.Run(48);
    EXPECT_EQ(battleship::GetPatternCount(*rig.Find(ship)), 1);   // 0.8 s
}

TEST(BattleShip, CannonTracksThePlayerWithinItsAngleRange)
{
    BossRig rig;
    ASSERT_TRUE(rig.IsReady());
    const EnemyHandle ship = SpawnShip(rig);
    RunUntilFight(rig, ship);
    rig.SetPlayer({ 500.f, kGroundY });
    rig.Run(100);
    rig.Frame();
    const float angle = battleship::GetCannonAngle(*rig.Find(ship));
    EXPECT_GE(angle, -135.f);
    EXPECT_LE(angle, -45.f);
}

TEST(BattleShip, EvenPatternMovesTheRailToAnotherSlotAndStops)
{
    BossRig rig;
    ASSERT_TRUE(rig.IsReady());
    const EnemyHandle ship = SpawnShip(rig);
    RunUntilFight(rig, ship);
    KillTurrets(rig, ship);
    for (int i = 0; i < 2000 && rig.Find(ship)->state != battleship::kRailMove; ++i)
    {
        rig.Frame();
    }
    ASSERT_EQ(rig.Find(ship)->state, battleship::kRailMove);
    const int target = rig.Find(ship)->ext[battleship::kExtRailTarget];
    EXPECT_NE(target, 1);   // 홈이 아닌 다른 슬롯

    for (int i = 0; i < 1000 && rig.Find(ship)->state == battleship::kRailMove; ++i)
    {
        rig.Frame();
    }
    const Enemy* pShip = rig.Find(ship);
    EXPECT_EQ(pShip->state, battleship::kIdle);
    EXPECT_EQ(pShip->ext[battleship::kExtRailSlot], target);
    const float slotX = BossArena().min.x + (target == 0 ? 30.f : 190.f);
    EXPECT_NEAR(pShip->body.position.x, slotX, 0.6f);
    EXPECT_NEAR(pShip->body.velocity.x, 0.f, 0.001f);
}

TEST(BattleShip, TurretsAreDestroyableWhileTheShipStaysAttackable)
{
    BossRig rig;
    ASSERT_TRUE(rig.IsReady());
    const EnemyHandle ship = SpawnShip(rig);
    RunUntilFight(rig, ship);
    EXPECT_EQ(battleship::GetTurretHp(*rig.Find(ship), 0), 40);

    rig.Hit(battleship::TurretOwner(ship.id, 0), 40);
    EXPECT_EQ(battleship::GetTurretHp(*rig.Find(ship), 0), 0);
    EXPECT_EQ(battleship::GetTurretHp(*rig.Find(ship), 1), 40);   // 다른 포탑은 그대로
    EXPECT_EQ(rig.Find(ship)->health.hp, 900);                    // 본체는 그대로

    rig.Hit(ship.id, 10);   // 본체는 여전히 맞는다
    EXPECT_EQ(rig.Find(ship)->health.hp, 890);
    EXPECT_EQ(battleship::GetTurretHp(*rig.Find(ship), 0), 0);
}

TEST(BattleShip, TurretDamageNeverReachesTheHullHitPoints)
{
    BossRig rig;
    ASSERT_TRUE(rig.IsReady());
    const EnemyHandle ship = SpawnShip(rig);
    RunUntilFight(rig, ship);
    rig.Hit(battleship::TurretOwner(ship.id, 1), 15);
    EXPECT_EQ(battleship::GetTurretHp(*rig.Find(ship), 1), 25);
    EXPECT_EQ(rig.Find(ship)->health.hp, 900);
    EXPECT_EQ(rig.CountEvents(eEnemyEventKind::Died), 0);
}

TEST(BattleShip, TurretsFireArcingShellsOnceTheFightStarts)
{
    BossRig rig;
    ASSERT_TRUE(rig.IsReady());
    const EnemyHandle ship = SpawnShip(rig);
    RunUntilFight(rig, ship);
    rig.SetPlayer({ 420.f, kGroundY });
    rig.Run(120);
    bool bShell = false;
    for (const Shot& shot: rig.Projectiles().GetShots())
    {
        if (shot.params.owner == ship.id && shot.params.gravity > 0.f)
        {
            bShell = true;
            EXPECT_FLOAT_EQ(shot.params.velocity.y, -200.f);   // 포물선 v0
        }
    }
    EXPECT_TRUE(bShell);
}

TEST(BattleShip, CannonShotsKillAnExposedPlayer)
{
    BossRig rig;
    ASSERT_TRUE(rig.IsReady());
    const EnemyHandle ship = SpawnShip(rig);
    RunUntilFight(rig, ship);
    rig.SetPlayerInvulnerable(false);
    rig.SetPlayer({ 430.f, kGroundY });
    rig.Run(900);

    bool bKilled = false;
    for (const HitEvent& ev: rig.PlayerHits())
    {
        bKilled = bKilled || (ev.attacker == ship.id && ev.attackerTeam == eTeam::Enemy);
    }
    EXPECT_TRUE(bKilled);
}

TEST(BattleShip, HullBlocksThePlayerAtItsXPlus270)
{
    BossRig rig;
    ASSERT_TRUE(rig.IsReady());
    const EnemyHandle ship = SpawnShip(rig);
    rig.Run(2);
    ASSERT_TRUE(rig.World().HasPlayerMinX());
    EXPECT_NEAR(rig.World().GetPlayerMinX(), rig.Find(ship)->body.position.x + 270.f, 0.01f);
}

TEST(BattleShip, HullConstraintStaysUntilTheDestroySequenceEnds)
{
    BossRig rig;
    ASSERT_TRUE(rig.IsReady());
    const EnemyHandle ship = SpawnShip(rig);
    RunUntilFight(rig, ship);
    rig.Hit(ship.id, 900);
    rig.Run(100);
    EXPECT_TRUE(rig.World().HasPlayerMinX());
    rig.Run(200);
    EXPECT_FALSE(rig.World().HasPlayerMinX());
}

TEST(BattleShip, DestroySequenceTakesExactlyTheSpecTicks)
{
    BossRig rig;
    ASSERT_TRUE(rig.IsReady());
    const EnemyHandle ship = SpawnShip(rig);
    RunUntilFight(rig, ship);
    EXPECT_FALSE(rig.World().IsDestroyFinished(ship));

    rig.Hit(ship.id, 900);
    EXPECT_EQ(rig.CountEventsOf(eEnemyEventKind::Died, ship), 1);   // Died 는 즉시
    EXPECT_TRUE(rig.World().IsDefeated(ship));
    EXPECT_FALSE(rig.World().IsDestroyFinished(ship));

    rig.Run(215);
    EXPECT_FALSE(rig.World().IsDestroyFinished(ship));
    rig.Run(1);
    EXPECT_TRUE(rig.World().IsDestroyFinished(ship));
}

TEST(BattleShip, DeathDestroysBothTurretsAndStopsTheAttacks)
{
    BossRig rig;
    ASSERT_TRUE(rig.IsReady());
    const EnemyHandle ship = SpawnShip(rig);
    RunUntilFight(rig, ship);
    rig.Hit(ship.id, 900);
    EXPECT_EQ(battleship::GetTurretHp(*rig.Find(ship), 0), 0);
    EXPECT_EQ(battleship::GetTurretHp(*rig.Find(ship), 1), 0);
    const int shots = rig.Find(ship)->shotsFired;
    rig.Run(400);
    EXPECT_EQ(rig.Find(ship)->shotsFired, shots);
    EXPECT_EQ(rig.CountEventsOf(eEnemyEventKind::Died, ship), 1);
}

TEST(BattleShip, IsDestroyFinishedIsFalseForAHandleThatIsStillAlive)
{
    BossRig rig;
    ASSERT_TRUE(rig.IsReady());
    const EnemyHandle ship = SpawnShip(rig);
    rig.Run(60);
    EXPECT_FALSE(rig.World().IsDestroyFinished(ship));
    EXPECT_TRUE(rig.World().IsDestroyFinished(EnemyHandle {}));
}

TEST(BattleShip, IdenticalScriptedRunsProduceIdenticalStateHashes)
{
    auto runOnce = []() {
        BossRig rig;
        const EnemyHandle ship = SpawnShip(rig);
        rig.SetPlayerInvulnerable(false);
        rig.SetPlayer({ 430.f, kGroundY });
        for (int i = 0; i < 1500; ++i)
        {
            if (i == 700)
            {
                rig.Hit(battleship::TurretOwner(ship.id, 0), 12);
            }
            rig.Frame();
        }
        return rig.World().HashState();
    };
    EXPECT_EQ(runOnce(), runOnce());
}

namespace
{
    SandboxConfig MakeBossLabConfig()
    {
        SandboxConfig config = MakeConfig(MakeFlatGround(640, 448, 378), { 540.f, 378.f });
        config.cameraBounds = Aabb::FromMinMax({ 0.f, 0.f }, { 640.f, 448.f });
        config.bBossLab     = true;
        config.bossArena    = BossArena();
        config.bossGroundY  = 378.f;
        config.enemyDefPath = BossDefsDir();
        return config;
    }

    const Enemy* FindShip(SandboxState& _state, EnemyHandle& _out)
    {
        const int defIndex = _state.GetEnemyLib().FindIndex("battleship");
        for (const Enemy& e: _state.GetEnemies().GetEnemies())
        {
            if (e.defIndex == defIndex)
            {
                _out = { e.id };
                return &e;
            }
        }
        return nullptr;
    }
}

TEST(BossLab, HullKeepsThePlayerOnTheFarSideOfTheBow)
{
    SandboxConfig config = MakeBossLabConfig();
    if (config.enemyDefPath.empty())
    {
        GTEST_SKIP() << "Boss.json 임시 폴더를 만들지 못했습니다";
    }
    Sim sim(config);
    EnemyHandle ship;
    ASSERT_NE(FindShip(sim.State(), ship), nullptr) << "bosslab 에 battleship 이 없습니다 (Defs 로딩 실패?)";

    sim.Hold(eAction::Left);
    sim.Step(300);
    EnemyHandle again;
    const Enemy* pShip = FindShip(sim.State(), again);
    ASSERT_NE(pShip, nullptr);
    EXPECT_GE(sim.X(), pShip->body.position.x + 270.f - 1.f);
}

TEST(BossLab, ThePistolCanDefeatTheBattleshipThroughThePlayerController)
{
    SandboxConfig config = MakeBossLabConfig();
    if (config.enemyDefPath.empty())
    {
        GTEST_SKIP() << "Boss.json 임시 폴더를 만들지 못했습니다";
    }
    Sim sim(config);
    sim.Tune().weapons.pistol.maxAlive = 0;
    sim.Tune().player.respawnInvincible = 1000000;
    sim.Tune().player.respawnOffsetX    = 400.f;

    EnemyHandle ship;
    ASSERT_NE(FindShip(sim.State(), ship), nullptr);

    int ticks = 0;
    for (; ticks < 9000; ++ticks)
    {
        sim.Hold(eAction::Left);
        sim.Hold(eAction::Shot);
        sim.Step();
        if (sim.State().GetEnemies().IsDestroyFinished(ship))
        {
            break;
        }
    }
    EXPECT_TRUE(sim.State().GetEnemies().IsDestroyFinished(ship));
    EXPECT_GT(ticks, 60 * 15);
    EXPECT_LT(ticks, 9000);
    std::printf("[boss-lab] pistol kill after %d ticks (%.1f s)\n", ticks, static_cast<double>(ticks) / 60.0);
}

TEST(BattleShip, BossBarReadsTheShipHealthThroughTheHandle)
{
    BossRig rig;
    ASSERT_TRUE(rig.IsReady());
    const EnemyHandle ship = SpawnShip(rig);
    rig.Run(2);
    EXPECT_EQ(rig.World().GetHealth(ship).hp, 900);
    EXPECT_EQ(rig.World().GetHealth(ship).maxHp, 900);
    rig.Hit(ship.id, 100);
    EXPECT_EQ(rig.World().GetHealth(ship).hp, 800);
    EXPECT_EQ(rig.World().GetHealth(EnemyHandle {}).maxHp, 0);   // 빈 핸들
}
