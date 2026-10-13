// 탈것 샌드박스 테스트: 샌드박스에 탈것을 스폰했을 때 결정성 검증
#include <cmath>
#include <vector>

#include "VehicleTestUtil.h"

constexpr float kGroundY = 378.f;

struct VehicleDirectState
{
    GameContext  ctx = {};
    SandboxState state { MakeEnemyConfig() };
    InputHistory input = {};

    VehicleDirectState()
    {
        state.OnEnter(ctx);
    }

    ~VehicleDirectState() { g_debug = {}; }
};

namespace
{
// 모든 탈것을 세워 두고 1200틱 동안 매 틱의 상태 해시를 _hashes 에 모은다
void RunHashes_(
    Vector<uint64_t>& _hashes)
{
    Sim   sim { MakeEnemyConfig({ 100.f, kGroundY }) };
    float x = 300.f;
    for (const char* name: kVehicleNames)
    {
        (void)sim.State().SpawnEnemy(name, { x, kGroundY });
        x += 150.f;
    }
    for (int i = 0; i < 1200; ++i)
    {
        sim.Step(1);
        _hashes.push_back(sim.State().HashState());
    }
}
}   // namespace

TEST(VehicleSandbox, KeyNineSpawnsTheFourVehiclesInTurnAndWraps)
{
    VehicleDirectState d;
    for (int press = 0; press < 5; ++press)
    {
        g_debug.spawn = 9;
        d.state.Tick(d.ctx, d.input);
        EXPECT_EQ(g_debug.spawn, 0);
    }
    const EnemyWorld& enemies = d.state.GetEnemies();
    EXPECT_EQ(enemies.CountAlive("jeep"), 2) << "다섯 번째는 다시 지프";
    EXPECT_EQ(enemies.CountAlive("btank"), 1);
    EXPECT_EQ(enemies.CountAlive("missle_tank"), 1);
    EXPECT_EQ(enemies.CountAlive("sarubia"), 1);
}

TEST(VehicleSandbox, KeyTenStillDoesNothing)
{
    VehicleDirectState d;
    g_debug.spawn = 10;
    d.state.Tick(d.ctx, d.input);
    EXPECT_EQ(d.state.GetEnemies().GetCount(), 0u);
}

TEST(VehicleSandbox, GroundVehiclesRestOnTheGround)
{
    Sim sim { MakeEnemyConfig() };
    sim.Step(2);
    SpawnParams left;
    left.facing               = -1;
    const EnemyHandle jeep    = sim.State().SpawnEnemy("jeep", { 400.f, kGroundY }, left);
    const EnemyHandle btank   = sim.State().SpawnEnemy("btank", { 700.f, kGroundY }, left);
    const EnemyHandle missile = sim.State().SpawnEnemy("missle_tank", { 800.f, kGroundY }, left);
    const EnemyHandle saru    = sim.State().SpawnEnemy("sarubia", { 900.f, kGroundY }, left);
    sim.Step(30);
    for (const EnemyHandle h: { jeep, btank, missile, saru })
    {
        const Enemy* pEnemy = sim.State().GetEnemies().Get(h);
        ASSERT_NE(pEnemy, nullptr);
        EXPECT_NEAR(pEnemy->body.position.y, kGroundY, 0.5f);
    }
    EXPECT_TRUE(sim.State().GetEnemies().Get(jeep)->bGrounded);
    EXPECT_TRUE(sim.State().GetEnemies().Get(saru)->bGrounded);
}

TEST(VehicleSandbox, SandboxHashIsDeterministicWithAllVehiclesPresent)
{
    Vector<uint64_t> first;
    Vector<uint64_t> second;
    RunHashes_(first);
    RunHashes_(second);
    EXPECT_EQ(first, second);
}

TEST(VehicleSandbox, PistolBurstsWearDownAMissleTankAtThreeDamagePerHit)
{
    Sim sim { MakeEnemyConfig() };
    sim.Step(2);
    const EnemyHandle h = sim.State().SpawnEnemy("missle_tank", { sim.X() + 240.f, kGroundY });
    sim.Hold(eAction::Shot);
    sim.Step(40);
    sim.Release(eAction::Shot);
    sim.Step(2);
    const Enemy* pEnemy = sim.State().GetEnemies().Get(h);
    ASSERT_NE(pEnemy, nullptr);
    EXPECT_LT(pEnemy->health.hp, 100);
    EXPECT_EQ(pEnemy->health.hp % 3, 100 % 3) << "피해는 3 의 배수로만 들어온다";
}
