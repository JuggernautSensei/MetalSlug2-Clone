
#include <cmath>
#include <cstring>
#include <vector>

#include "EnemyTestUtil.h"

constexpr float kGroundY = 378.f;

struct EnemyDirectState
{
    SandboxState state { MakeEnemyConfig() };
    InputHistory input = {};

    EnemyDirectState()
    {
        state.OnEnter();
    }

    ~EnemyDirectState() { g_debug = {}; }
};

namespace
{
[[nodiscard]] int StepUntilPlayerDead_(
    Sim&      _sim,
    const int _maxTicks)
{
    for (int i = 0; i < _maxTicks; ++i)
    {
        if (!_sim.Player().IsAlive())
        {
            return i;
        }
        _sim.Step();
    }
    return !_sim.Player().IsAlive() ? _maxTicks : -1;
}

void RunHashes_(
    Vector<uint64_t>& _hashes)
{
    Sim   sim { MakeEnemyConfig({ 100.f, kGroundY }) };
    float x = 300.f;
    for (const char* name: kInfantryNames)
    {
        (void)sim.State().SpawnEnemy(name, { x, kGroundY });
        x += 50.f;
    }
    for (int i = 0; i < 1200; ++i)
    {
        sim.Step(1);
        _hashes.push_back(sim.State().HashState());
    }
}
}   // namespace

TEST(EnemySandbox, LoadsTheDefinitionFileAndSpawnsEveryInfantryKind)
{
    Sim sim { MakeEnemyConfig() };
    sim.Step(1);
    EXPECT_GT(sim.State().GetEnemyLib().GetDefCount(), 0u);

    float x = 260.f;
    for (const char* name: kInfantryNames)
    {
        const float       y = std::strcmp(name, "flying_bzk") == 0 ? 150.f : kGroundY;
        const EnemyHandle h = sim.State().SpawnEnemy(name, { x, y });
        EXPECT_FALSE(h.IsNull()) << name;
        x += 60.f;
    }
    sim.Step(120);
    EXPECT_EQ(sim.State().GetEnemies().GetCount(), 8u);
    for (const char* name: kInfantryNames)
    {
        EXPECT_GE(sim.State().GetEnemies().CountAlive(name), 1) << name;
    }
}

TEST(EnemySandbox, GroundedKindsRestOnTheGround)
{
    Sim                 sim { MakeEnemyConfig() };
    float               x = 260.f;
    Vector<EnemyHandle> handles;
    Vector<const char*> names;
    for (const char* name: { "soldier_bomb", "soldier_sword", "soldier_missile", "soldier_bzk", "shielder", "pow" })
    {
        handles.push_back(sim.State().SpawnEnemy(name, { x, 300.f }));
        names.push_back(name);
        x += 60.f;
    }
    Vector<bool> seen(handles.size(), false);
    for (int tick = 0; tick < 120; ++tick)
    {
        sim.Step(1);
        for (size_t i = 0; i < handles.size(); ++i)
        {
            seen[i] = seen[i] || OnGround(sim.State().GetEnemies().Get(handles[i]));
        }
    }
    for (size_t i = 0; i < handles.size(); ++i)
    {
        EXPECT_TRUE(seen[i]) << names[i] << " never stood on the ground";
    }
}

TEST(EnemySandbox, HangHoversAtItsSpawnHeight)
{
    Sim               sim { MakeEnemyConfig() };
    const EnemyHandle h = sim.State().SpawnEnemy("soldier_hang", { 300.f, 260.f });
    sim.Step(120);
    const EnemyState* pHang = sim.State().GetEnemies().Get(h);
    ASSERT_NE(pHang, nullptr);
    EXPECT_NEAR(pHang->body.position.y, 260.f, 0.01f);
}

TEST(EnemySandbox, FlyingBzkDescendsAndTurnsIntoASoldierOnLanding)
{
    Sim               sim { MakeEnemyConfig() };
    const EnemyHandle para = sim.State().SpawnEnemy("flying_bzk", { 300.f, 150.f });
    sim.Step(600);
    const EnemyHandle heir = sim.State().GetEnemies().ResolveHandle(para);
    ASSERT_NE(heir.id, para.id);
    const EnemyState* pHeir = sim.State().GetEnemies().Get(heir);
    ASSERT_NE(pHeir, nullptr);
    EXPECT_EQ(sim.State().GetEnemyLib().Get(pHeir->defIndex).name, "soldier_bzk");
}

TEST(EnemySandbox, InactiveMissileSoldierStaysSilentWhileFarAway)
{
    Sim               sim { MakeEnemyConfig() };
    const EnemyHandle h = sim.State().SpawnEnemy("soldier_missile", { 1200.f, kGroundY });
    sim.Step(600);
    const EnemyState* pEnemy = sim.State().GetEnemies().Get(h);
    ASSERT_NE(pEnemy, nullptr);
    EXPECT_FALSE(pEnemy->bActive);
    EXPECT_EQ(sim.State().GetProjectiles().CountShots(h.id), 0);
}

TEST(EnemySandbox, WithoutADefinitionFileNothingSpawns)
{
    SandboxConfig config = MakeFlatConfig();
    config.enemyDefPath  = "Z:/no/such/defs";
    Sim sim { config };
    sim.Step(1);
    EXPECT_EQ(sim.State().GetEnemyLib().GetDefCount(), 0u);
    EXPECT_TRUE(sim.State().SpawnEnemy("soldier_bomb", { 300.f, kGroundY }).IsNull());
}

TEST(EnemySandbox, KnifeKillsASoldierInOneSlash)
{
    Sim sim { MakeEnemyConfig() };
    sim.Step(2);
    const EnemyHandle h = sim.State().SpawnEnemy("soldier_bomb", { sim.X() + 25.f, kGroundY });
    sim.Tap(eAction::Shot);
    sim.Step(30);
    EXPECT_TRUE(sim.State().GetEnemies().IsDefeated(h));
    EXPECT_EQ(sim.Player().GetStats().knifeSlashes, 1);
    EXPECT_EQ(sim.Player().GetStats().shotsFired, 0);
}

TEST(EnemySandbox, PistolBulletKillsASoldierAndAwardsItsScore)
{
    Sim sim { MakeEnemyConfig() };
    sim.Step(2);
    const EnemyHandle h           = sim.State().SpawnEnemy("soldier_bomb", { sim.X() + 200.f, kGroundY });
    const int         scoreBefore = sim.Player().GetScore();
    sim.Hold(eAction::Shot);
    sim.Step(60);
    sim.Release(eAction::Shot);
    EXPECT_TRUE(sim.State().GetEnemies().IsDefeated(h));
    EXPECT_EQ(sim.Player().GetScore() - scoreBefore, 100);
}

TEST(EnemySandbox, BombKillsASoldierStandingNearby)
{
    Sim sim { MakeEnemyConfig() };
    sim.Step(2);
    const EnemyHandle h = sim.State().SpawnEnemy("soldier_bomb", { sim.X() + 60.f, kGroundY });
    sim.Tap(eAction::Bomb);
    sim.Step(120);
    EXPECT_TRUE(sim.State().GetEnemies().IsDefeated(h));
}

TEST(EnemySandbox, ASwordSoldierCanKillThePlayer)
{
    Sim sim { MakeEnemyConfig() };
    sim.Step(2);
    (void)sim.State().SpawnEnemy("soldier_sword", { sim.X() + 45.f, kGroundY });
    EXPECT_GE(StepUntilPlayerDead_(sim, 1500), 0);
}

TEST(EnemySandbox, TrainingTargetsStillDieToTheKnifeWithEnemiesPresent)
{
    Sim sim { MakeEnemyConfig() };
    sim.Step(2);
    const uint32_t grunt = sim.State().AddTarget(eDummyKind::Grunt, { sim.X() + 25.f, kGroundY });
    (void)sim.State().SpawnEnemy("soldier_missile", { sim.X() + 400.f, kGroundY });
    sim.Tap(eAction::Shot);
    sim.Step(30);
    const TrainingDummy* pDummy = sim.State().GetTargets().Find(grunt);
    ASSERT_NE(pDummy, nullptr);
    EXPECT_FALSE(pDummy->IsAlive());
    EXPECT_EQ(sim.Player().GetStats().knifeSlashes, 1);
}

TEST(EnemySandbox, DebugSpawnSlotsSpawnTheEightInfantryKinds)
{
    for (int slot = 1; slot <= 8; ++slot)
    {
        EnemyDirectState d;
        g_debug.spawn = slot;
        d.state.Tick(d.input);
        EXPECT_EQ(g_debug.spawn, 0) << slot;
        EXPECT_EQ(d.state.GetEnemies().CountAlive(kInfantryNames[slot - 1]), 1) << kInfantryNames[slot - 1];
        EXPECT_EQ(d.state.GetEnemies().GetCount(), 1u) << slot;
    }
}

TEST(EnemySandbox, DebugSpawnOutOfRangeDoesNothing)
{
    EnemyDirectState d;
    g_debug.spawn = 10;
    d.state.Tick(d.input);
    EXPECT_EQ(d.state.GetEnemies().GetCount(), 0u);
}

TEST(EnemySandbox, SandboxHashIsDeterministicWithEnemiesPresent)
{
    Vector<uint64_t> first;
    Vector<uint64_t> second;
    RunHashes_(first);
    RunHashes_(second);
    EXPECT_EQ(first, second);
}

TEST(EnemySandbox, BloodSplatsAtTheChestOfAKnifedSoldier)
{
    SLUG_REQUIRE_BAKED(anims);
    Sim sim { MakeEnemyConfig() };
    sim.Step(2);
    const EnemyHandle h = sim.State().SpawnEnemy("soldier_bomb", { sim.X() + 25.f, kGroundY });
    sim.Tap(eAction::Shot);
    for (int i = 0; i < 30 && !sim.State().GetEnemies().IsDefeated(h); ++i)
    {
        sim.Step(1);
    }
    ASSERT_TRUE(sim.State().GetEnemies().IsDefeated(h));
    sim.Step(1);
    const EnemyState* pEnemy = sim.State().GetEnemies().Get(h);
    ASSERT_NE(pEnemy, nullptr);

    int bloodCount = 0;
    for (const Effect& fx: sim.State().GetEnemies().GetEffects().GetEffects())
    {
        if (fx.depth != 60)
        {
            continue;
        }
        ++bloodCount;
        EXPECT_NEAR(fx.position.x, pEnemy->body.position.x, 0.5f);
        const float chestDy = fx.position.y - pEnemy->body.position.y;
        EXPECT_GE(chestDy, -68.f);
        EXPECT_LE(chestDy, -62.f);
    }
    EXPECT_GE(bloodCount, 1);
}

TEST(EnemySandbox, KnifedSoldierLeavesNoCorpseOnceTheSlashClipEnds)
{
    Sim sim { MakeEnemyConfig() };
    sim.Step(2);
    const EnemyHandle h = sim.State().SpawnEnemy("soldier_bomb", { sim.X() + 25.f, kGroundY });
    sim.Tap(eAction::Shot);
    sim.Step(30);
    ASSERT_TRUE(sim.State().GetEnemies().IsDefeated(h));
    ASSERT_NE(sim.State().GetEnemies().Get(h), nullptr);

    sim.Step(90);
    EXPECT_EQ(sim.State().GetEnemies().Get(h), nullptr);
}
