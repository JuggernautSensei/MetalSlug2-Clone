// 샌드박스 상태에서 적 스폰, 전투, 디버그 스폰 슬롯과 상태 해시 결정성을 검증한다.

#include <cmath>
#include <cstring>
#include <vector>

#include "EnemyTestUtil.h"


constexpr float kGroundY = 378.f;

[[nodiscard]] static bool OnGround_(
    const Enemy* _pEnemy)
{
    return _pEnemy && _pEnemy->bGrounded && std::abs(_pEnemy->body.position.y - kGroundY) < 0.5f;
}

struct EnemyDirectState
{
    GameContext  ctx = {};
    SandboxState state { MakeEnemyConfig() };
    InputHistory input = {};

    EnemyDirectState()
    {
        state.OnEnter(ctx);
    }
};

// 플레이어가 죽을 때까지 최대 _maxTicks 틱을 진행한다 (Sim::StepUntil 과 같은 규약: 경과 틱, 실패 -1)
[[nodiscard]] static int StepUntilPlayerDead_(
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

// 보병 전 종류를 세워 두고 1200틱 동안 샌드박스 상태 해시를 기록한다
static void RunHashes_(
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
    Sim                       sim { MakeEnemyConfig() };
    float                     x = 260.f;
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
            seen[i] = seen[i] || OnGround_(sim.State().GetEnemies().Get(handles[i]));
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
    const Enemy* pHang = sim.State().GetEnemies().Get(h);
    ASSERT_NE(pHang, nullptr);
    EXPECT_NEAR(pHang->body.position.y, 260.f, 0.01f);   // 중력 없이 매달린다
}

TEST(EnemySandbox, FlyingBzkDescendsAndTurnsIntoASoldierOnLanding)
{
    Sim               sim { MakeEnemyConfig() };
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
    Sim               sim { MakeEnemyConfig() };
    const EnemyHandle h = sim.State().SpawnEnemy("soldier_missile", { 1200.f, kGroundY });
    sim.Step(600);
    const Enemy* pEnemy = sim.State().GetEnemies().Get(h);
    ASSERT_NE(pEnemy, nullptr);
    EXPECT_FALSE(pEnemy->bActive);
    EXPECT_EQ(sim.State().GetProjectiles().CountShots(h.id), 0);
}

TEST(EnemySandbox, WithoutADefinitionFileNothingSpawns)
{
    SandboxConfig config = MakeFlatConfig();
    Sim           sim { config };
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
        d.ctx.debugSpawn = slot;
        d.state.Tick(d.ctx, d.input);
        EXPECT_EQ(d.ctx.debugSpawn, 0) << slot;   // 소비
        EXPECT_EQ(d.state.GetEnemies().CountAlive(kInfantryNames[slot - 1]), 1) << kInfantryNames[slot - 1];
        EXPECT_EQ(d.state.GetEnemies().GetCount(), 1u) << slot;
    }
}

TEST(EnemySandbox, DebugSpawnOutOfRangeDoesNothing)
{
    EnemyDirectState d;
    d.ctx.debugSpawn = 10;
    d.state.Tick(d.ctx, d.input);
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

// 혈흔은 발 원점이 아니라 피격 상자 중심(가슴, body.cy = -37.5)에서 터진다
TEST(EnemySandbox, BloodSplatsAtTheChestOfAKnifedSoldier)
{
    SLUG_REQUIRE_BAKED(anims);   // 이펙트 연출은 애니 라이브러리가 있어야 생성된다
    Sim sim { MakeEnemyConfig(), &anims };
    sim.Step(2);
    const EnemyHandle h = sim.State().SpawnEnemy("soldier_bomb", { sim.X() + 25.f, kGroundY });
    sim.Tap(eAction::Shot);
    for (int i = 0; i < 30 && !sim.State().GetEnemies().IsDefeated(h); ++i)
    {
        sim.Step(1);
    }
    ASSERT_TRUE(sim.State().GetEnemies().IsDefeated(h));
    sim.Step(1);   // 사망 틱 다음에 혈흔 연출이 생성된다 (수명 약 0.44 s 이므로 바로 확인)
    const Enemy* pEnemy = sim.State().GetEnemies().Get(h);
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
        // 혈흔 y 보정 상수 (kBloodOffsetY) 는 소스 익명 네임스페이스라 값 대신 상체 윗부분 (발 기준 -68 ~ -62, 가슴~어깨) 으로 검증한다
        const float chestDy = fx.position.y - pEnemy->body.position.y;
        EXPECT_GE(chestDy, -68.f);
        EXPECT_LE(chestDy, -62.f);
    }
    EXPECT_GE(bloodCount, 1);
}

// 칼 사망은 베는 클립이 끝나면 시체를 남기지 않는다 (마지막 프레임 고정 방지)
TEST(EnemySandbox, KnifedSoldierLeavesNoCorpseOnceTheSlashClipEnds)
{
    Sim sim { MakeEnemyConfig() };
    sim.Step(2);
    const EnemyHandle h = sim.State().SpawnEnemy("soldier_bomb", { sim.X() + 25.f, kGroundY });
    sim.Tap(eAction::Shot);
    sim.Step(30);
    ASSERT_TRUE(sim.State().GetEnemies().IsDefeated(h));
    ASSERT_NE(sim.State().GetEnemies().Get(h), nullptr);   // 클립(0.96 s) 중에는 아직 있다

    sim.Step(90);
    EXPECT_EQ(sim.State().GetEnemies().Get(h), nullptr);   // 탭 후 약 2 s 에 제거 (기존: 사망 후 3.0 s 까지 마지막 프레임 고정)
}
