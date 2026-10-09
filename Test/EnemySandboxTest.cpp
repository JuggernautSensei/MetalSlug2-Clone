#include <cmath>
#include <cstring>
#include <vector>

#include "EnemyTestUtil.h"

using namespace slugtest;

namespace
{
    constexpr float kGroundY = 378.f;

    bool OnGround(const Enemy* _pEnemy)
    {
        return _pEnemy && _pEnemy->bGrounded && std::abs(_pEnemy->body.position.y - kGroundY) < 0.5f;
    }

    // 샌드박스 상태를 GameContext 로 직접 돌린다 (숫자키 디버그 스폰 경로 검증용)
    struct DirectState
    {
        GameContext    ctx   = {};
        SandboxState   state { MakeEnemyConfig() };
        InputHistory   input = {};

        DirectState()
        {
            state.OnEnter(ctx);
        }
    };
}   // namespace

// ===========================================
//  배치 / 지면 / 활성
// ===========================================

TEST(EnemySandbox, LoadsTheDefinitionFileAndSpawnsEveryInfantryKind)
{
    Sim sim { MakeEnemyConfig() };
    sim.Step(1);
    EXPECT_GT(sim.State().GetEnemyLib().GetDefCount(), 0u);

    float x = 260.f;
    for (const char* name: kInfantryNames)
    {
        // 낙하산 병은 공중에서 내려와야 하므로 높이를 준다 (지면에 두면 바로 착지해 변신한다)
        const float y = std::strcmp(name, "flying_bzk") == 0 ? 150.f : kGroundY;
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
    Sim sim { MakeEnemyConfig() };
    float x = 260.f;
    std::vector<EnemyHandle> handles;
    std::vector<const char*> names;
    for (const char* name: { "soldier_bomb", "soldier_sword", "soldier_missile", "soldier_bzk", "shielder", "pow" })
    {
        handles.push_back(sim.State().SpawnEnemy(name, { x, 300.f }));
        names.push_back(name);
        x += 60.f;
    }
    // 병사는 전투 행동(점프 베기 등)을 하므로 "120 틱 안에 한 번이라도 지면에 서 있었다" 로 확인한다
    std::vector<bool> seen(handles.size(), false);
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
    Sim sim { MakeEnemyConfig() };
    const EnemyHandle h = sim.State().SpawnEnemy("soldier_hang", { 300.f, 260.f });
    sim.Step(120);
    const Enemy* pHang = sim.State().GetEnemies().Get(h);
    ASSERT_NE(pHang, nullptr);
    EXPECT_NEAR(pHang->body.position.y, 260.f, 0.01f);   // 중력 없이 매달린다
}

TEST(EnemySandbox, FlyingBzkDescendsAndTurnsIntoASoldierOnLanding)
{
    Sim sim { MakeEnemyConfig() };
    const EnemyHandle para = sim.State().SpawnEnemy("flying_bzk", { 300.f, 150.f });
    sim.Step(600);
    const EnemyHandle heir = sim.State().GetEnemies().ResolveHandle(para);
    ASSERT_NE(heir.id, para.id);
    const Enemy* pHeir = sim.State().GetEnemies().Get(heir);
    ASSERT_NE(pHeir, nullptr);
    EXPECT_EQ(sim.State().GetEnemyLib().Get(pHeir->defIndex).name, "soldier_bzk");
}

TEST(EnemySandbox, InactiveMissileSoldierStaysSilentWhileFarAway)
{
    Sim sim { MakeEnemyConfig() };
    const EnemyHandle h = sim.State().SpawnEnemy("soldier_missile", { 1200.f, kGroundY });
    sim.Step(600);
    const Enemy* pEnemy = sim.State().GetEnemies().Get(h);
    ASSERT_NE(pEnemy, nullptr);
    EXPECT_FALSE(pEnemy->bActive);
    EXPECT_EQ(sim.State().GetProjectiles().CountShots(h.id), 0);
}

TEST(EnemySandbox, WithoutADefinitionFileNothingSpawns)
{
    SandboxConfig config = MakeFlatConfig();   // enemyDefPath 없음, assetRoot 없음
    Sim sim { config };
    sim.Step(1);
    EXPECT_EQ(sim.State().GetEnemyLib().GetDefCount(), 0u);
    EXPECT_TRUE(sim.State().SpawnEnemy("soldier_bomb", { 300.f, kGroundY }).IsNull());
}

// ===========================================
//  플레이어와의 교전
// ===========================================

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
    const EnemyHandle h = sim.State().SpawnEnemy("soldier_bomb", { sim.X() + 200.f, kGroundY });
    const int scoreBefore = sim.Player().GetScore();
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
    EXPECT_GE(sim.StepUntil(1500, [&] { return !sim.Player().IsAlive(); }), 0);
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

// ===========================================
//  디버그 스폰 (숫자키 1..8 → 보병 정의 8종)
// ===========================================

TEST(EnemySandbox, DebugSpawnSlotsSpawnTheEightInfantryKinds)
{
    for (int slot = 1; slot <= 8; ++slot)
    {
        DirectState d;
        d.ctx.debugSpawn = slot;
        d.state.Tick(d.ctx, d.input);
        EXPECT_EQ(d.ctx.debugSpawn, 0) << slot;   // 소비
        EXPECT_EQ(d.state.GetEnemies().CountAlive(kInfantryNames[slot - 1]), 1) << kInfantryNames[slot - 1];
        EXPECT_EQ(d.state.GetEnemies().GetCount(), 1u) << slot;
    }
}

TEST(EnemySandbox, DebugSpawnOutOfRangeDoesNothing)
{
    DirectState d;
    d.ctx.debugSpawn = 10;   // 9 는 이제 차량 순환 키 (VehicleSandboxTest 가 검증)
    d.state.Tick(d.ctx, d.input);
    EXPECT_EQ(d.state.GetEnemies().GetCount(), 0u);
}

// ===========================================
//  결정성
// ===========================================

TEST(EnemySandbox, SandboxHashIsDeterministicWithEnemiesPresent)
{
    const auto Run = [](std::vector<uint64_t>& _hashes) {
        Sim sim { MakeEnemyConfig({ 100.f, kGroundY }) };
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
    };
    std::vector<uint64_t> first;
    std::vector<uint64_t> second;
    Run(first);
    Run(second);
    EXPECT_EQ(first, second);
}
