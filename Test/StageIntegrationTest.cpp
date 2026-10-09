#include "Core/AssetPath.h"
#include <cstdint>
#include "Base/Types.h"
#include "Base/Math.h"
#include "Base/Random.h"
#include <optional>
#include <vector>

#include <gtest/gtest.h>

#include "Core/Input.h"
#include "Core/Tuning.h"
#include "Game/Enemies/EnemyWorld.h"
#include "Game/Player/PlayerController.h"
#include "Game/Props/PropWorld.h"
#include "Game/SimRunner.h"
#include "Game/Stage/StageMapping.h"
#include "Game/Stage/StageRunner.h"
#include "Game/Stage/StageState.h"
#include "Game/Stage/StageWorldHost.h"
#include "Game/Ui/Hud.h"
#include "StageTestUtil.h"

namespace
{
using namespace slug;

class IntegrationSim
{
public:
    explicit IntegrationSim(
        const bool _bDebugDraw = false)
        : m_rng(uint64_t { 1 })
    {
        m_ctx.pRng       = &m_rng;
        m_ctx.assetRoot  = slug::FindAssetRoot();
        m_ctx.bDebugDraw = _bDebugDraw;
        m_pState         = new StageState;
        m_runner         = std::unique_ptr<SimRunner> { new SimRunner { m_ctx, std::unique_ptr<IGameState> { m_pState } } };
    }

    [[nodiscard]] StageState&  State() { return *m_pState; }
    [[nodiscard]] IGameState*  Current() const { return m_runner->GetState(); }
    [[nodiscard]] const IGameState* StagePtr() const { return m_pState; }
    [[nodiscard]] GameContext& Ctx() { return m_ctx; }

    void BotStep(const bool _bRight, const bool _bShoot, const int _count = 1)
    {
        for (int i = 0; i < _count; ++i)
        {
            PlayerController& player = m_pState->GetPlayer();
            player.ExtendInvincibility(600);
            const float x = player.GetFeet().x;
            m_stuckTicks  = (x - m_lastX < 0.5f) ? m_stuckTicks + 1 : 0;
            m_lastX       = x;
            if (m_stuckTicks >= 20 && m_backTicks == 0 && m_jumpTicks == 0)
            {
                m_backTicks  = 40;
                m_stuckTicks = 0;
            }

            if (m_backTicks == 0 && m_jumpTicks == 0 && player.IsGrounded() && IsWallAhead_(player.GetFeet()))
            {
                m_jumpTicks = 36;
            }

            bool bRight = _bRight;
            bool bJump  = false;
            if (m_backTicks > 0)
            {
                bRight = false;
                if (--m_backTicks == 0)
                {
                    m_jumpTicks = 36;   // 점프 컷(놓으면 높이 감소)을 피하려고 정점까지 누른다
                }
            }
            else if (m_jumpTicks > 0)
            {
                bJump = true;
                --m_jumpTicks;
            }
            m_frame.Set(eAction::Right, bRight);
            m_frame.Set(eAction::Left, m_backTicks > 0);
            m_frame.Set(eAction::Shot, _bShoot && (m_botTick % 24) < 3);
            m_frame.Set(eAction::Jump, bJump);
            m_runner->Step(m_frame);
            ++m_botTick;
        }
    }

    void Step(const int _count = 1)
    {
        for (int i = 0; i < _count; ++i)
        {
            m_frame = {};
            m_runner->Step(m_frame);
        }
    }

    void Hold(const eAction _action) { m_frame.Set(_action, true); }

    void Warp(const int _code, const int _count = 1)
    {
        for (int i = 0; i < _count; ++i)
        {
            m_ctx.debugWarp = _code;
            m_frame         = {};
            m_runner->Step(m_frame);
        }
    }

private:
    [[nodiscard]] bool IsWallAhead_(
        const Vec2 _feet) const
    {
        constexpr float kRange = 46.f;
        for (const Wall& wall: m_pState->GetProps().GetWalls())
        {
            if (wall.box.min.x > _feet.x && wall.box.min.x - _feet.x <= kRange && wall.box.min.y < _feet.y - 8.f)
            {
                return true;
            }
        }
        for (const StageCollider& collider: m_pState->GetHost().GetColliders())
        {
            if (collider.layer == eCollisionLayer::Wall && collider.box.min.x > _feet.x && collider.box.min.x - _feet.x <= kRange && collider.box.min.y < _feet.y - 8.f)
            {
                return true;
            }
        }
        return false;
    }

    slug::RngMt19937 m_rng;
    GameContext     m_ctx      = {};
    InputFrame      m_frame    = {};
    StageState*     m_pState   = nullptr;
    uint64_t        m_botTick  = 0;
    float           m_lastX    = -1.f;
    int             m_stuckTicks = 0;
    int             m_jumpTicks  = 0;
    int             m_backTicks  = 0;
    std::unique_ptr<SimRunner> m_runner = nullptr;
};

class GateFixture
{
public:
    explicit GateFixture(const std::string& _json)
        : data(slugtest::MustLoad(_json))
        , rng(uint64_t { 1 })
        , runner(data, host, camera, rng)
    {
        slug::String message;
        enemyLib = EnemyDefLibrary::LoadDirectory(EnemyDefLibrary::GetDefaultDirectory(slug::FindAssetRoot()), &message).Take();
        enemies.SetLibrary(&enemyLib);
        props.SetLibrary(&propLib);

        host.AddGroundMap("G0", MakeFlat_());
        host.AddGroundMap("G1", MakeFlat_());
        host.BindWorld(&enemies, &props);
        host.BindPlayer(&player, &tuning);
        camera.SetBounds(Aabb::FromMinMax({ 0.f, 0.f }, { 4000.f, 448.f }));
        player.Reset(tuning, { 100.f, 378.f });
        runner.Begin();
    }

    void Step(const int _count = 1)
    {
        for (int i = 0; i < _count; ++i)
        {
            runner.Tick();
        }
    }

    static GroundMap MakeFlat_()
    {
        GroundMap map = GroundMap::MakeEmpty(4000, 448);
        map.FillRect(0, 378, 3999, 447, true);
        return map;
    }

    StageData         data;
    StageWorldHost    host;
    Camera            camera;
    slug::RngMt19937   rng;
    StageRunner       runner;
    EnemyDefLibrary   enemyLib;
    PropDefLibrary    propLib;
    EnemyWorld        enemies { nullptr };
    PropWorld         props { nullptr, 1 };
    PlayerController  player;
    Tuning            tuning;
};
}

TEST(StageIntegration, Stage0UsesRealEnemiesAndPropsInsteadOfPlaceholders)
{
    IntegrationSim sim;
    sim.Step(2);

    const StageWorldHost& host = sim.State().GetHost();
    EXPECT_GT(sim.State().GetEnemies().CountAlive("soldier_bomb"), 0);
    EXPECT_GT(sim.State().GetProps().GetCount(), 0u);   // intro 의 트럭 두 대

    for (const PlaceholderEntity& entity: host.GetPlaceholders().GetEntities())
    {
        EXPECT_EQ(entity.kind, eEntityKind::Decor) << entity.def;
    }
}

TEST(StageIntegration, GroundPatchMaskAndApplyClearTheDestroyedHouseFootprint)
{
    const std::optional<GroundMap> mask = LoadGroundPatchMask(slug::FindAssetRoot() / "Baked" / "Textures" / "Stage0_House_4_PixelMap.png");
    ASSERT_TRUE(mask.has_value());
    EXPECT_EQ(mask->GetWidth(), 1306);
    EXPECT_EQ(mask->GetHeight(), 751);
    EXPECT_FALSE(mask->IsSolid(0, 0));

    GroundMap ground = GroundMap::MakeEmpty(2000, 1000);
    ground.FillRect(0, 0, 1999, 999, true);
    ApplyGroundPatch(ground, { 100.f, 100.f }, *mask);
    EXPECT_FALSE(ground.IsSolid(105, 105));    // 발자국 안: 공기
    EXPECT_TRUE(ground.IsSolid(50, 50));       // 발자국 밖: 그대로 고체

    EXPECT_FALSE(LoadGroundPatchMask(slug::FindAssetRoot() / "Baked" / "Textures" / "no_such_map.png").has_value());
}

TEST(StageIntegration, SpawnedEnemiesStandOnTheGroundAtTheirScriptedFeet)
{
    IntegrationSim sim;
    sim.Step(2);
    bool bFound = false;
    for (const Enemy& enemy: sim.State().GetEnemies().GetEnemies())
    {
        const Vec2 feet = enemy.body.position;
        if (enemy.life == eEnemyLife::Alive && feet.x > 880.f && feet.x < 920.f)
        {
            bFound = true;
            EXPECT_NEAR(feet.y, 378.f, 1.5f);
        }
    }
    EXPECT_TRUE(bFound);
}

TEST(StageIntegration, LongWalkThroughTheFirstSectionsWithoutWarpReachesTheStreet)
{
    IntegrationSim sim;
    for (int i = 0; i < 6000 && sim.State().GetPlayer().GetFeet().x < 1300.f; ++i)
    {
        sim.BotStep(true, true);
        ASSERT_FALSE(sim.State().IsFinished()) << "tick " << i;
    }
    EXPECT_GE(sim.State().GetPlayer().GetFeet().x, 1300.f);
    EXPECT_GE(sim.State().GetRunner().GetNextSectionIndex(), 2);   // intro, street 발동
    EXPECT_GT(sim.State().GetEnemies().GetCount(), 0u);            // 실제 적이 스폰되어 있다
}

TEST(StageIntegration, WarpChainThroughEverySectionReachesTheBossAndEndsTheStage)
{
    IntegrationSim sim(true);
    const int sectionCount = static_cast<int>(sim.State().GetData().sections.size());
    for (int i = 0; i < sectionCount - 2 && !sim.State().IsFinished(); ++i)
    {
        sim.Warp(1);
        for (int t = 0; t < 120; ++t)
        {
            sim.BotStep(true, true);
        }
    }
    sim.Warp(2);
    sim.Step(2);
    ASSERT_GT(sim.State().GetEnemies().CountAlive("battleship"), 0);

    const IGameState* pStage = sim.StagePtr();
    sim.State().GetEnemies().KillAll(true);
    for (int i = 0; i < 3000 && sim.Current() == pStage; ++i)
    {
        sim.Step();
    }
    EXPECT_NE(sim.Current(), pStage);
}

TEST(StageIntegration, EnemyDeathScoreReachesThePlayerAndTheHudModel)
{
    IntegrationSim sim;
    sim.Step(2);
    const int before = sim.State().GetPlayer().GetScore();
    sim.State().GetEnemies().KillAll(true);
    sim.Step(2);
    EXPECT_GT(sim.State().GetPlayer().GetScore(), before);
    EXPECT_EQ(MakeHudModel(sim.State().GetPlayer()).score, sim.State().GetPlayer().GetScore());
}

TEST(StageIntegration, GateOpensOnlyAfterTheSpawnedTargetDies)
{
    GateFixture fx(slugtest::WrapStage(R"J([{"id":"a","triggerX":0,"actions":[
        {"type":"spawn","kind":"enemy","def":"soldier_sword","id":"t","x":400,"y":378},
        {"type":"gate","id":"g","waitFor":["t"],"actions":[{"type":"banner","text":"opened","seconds":1}]}]}])J"));

    fx.Step(3);
    ASSERT_EQ(fx.runner.GetOpenGateCount(), 1);
    EXPECT_FALSE(fx.host.IsDefeated(fx.runner.FindSpawnIds("t")[0]));

    fx.enemies.KillAll(false);
    fx.Step(2);
    EXPECT_EQ(fx.runner.GetOpenGateCount(), 0);
}

TEST(StageIntegration, PropRoofCarriesThePlayerAboveTheGround)
{
    IntegrationSim sim(true);
    sim.Warp(1, 2);
    sim.Step(2);

    sim.State().GetPlayer().Reset(Tuning {}, { 2513.f, 60.f });
    sim.Step(240);
    const Vec2 feet = sim.State().GetPlayer().GetFeet();
    EXPECT_TRUE(sim.State().GetPlayer().IsGrounded());
    EXPECT_NEAR(feet.x, 2513.f, 80.f);
    EXPECT_LT(feet.y, 300.f);   // 지면(378)이 아니라 지붕 위
}

TEST(StageIntegration, DebugWarpToTheBossThenBossDeathEndsTheStageThroughTheFlow)
{
    IntegrationSim sim(true);
    sim.Warp(2);   // F6: 보스 아레나
    sim.Step(2);

    ASSERT_GT(sim.State().GetEnemies().CountAlive("battleship"), 0);

    const IGameState* pStage = sim.StagePtr();
    sim.State().GetEnemies().KillAll(true);
    for (int i = 0; i < 3000 && sim.Current() == pStage; ++i)
    {
        sim.Step();
    }
    EXPECT_NE(sim.Current(), pStage);
}

TEST(StageIntegration, GameOverRunsTheGameOverFlowAndReturnsToTitle)
{
    IntegrationSim sim;
    sim.Step(2);
    const IGameState* pStage = sim.StagePtr();
    for (int i = 0; i < 4000 && !sim.State().GetPlayer().IsGameOver(); ++i)
    {
        PlayerController& player = sim.State().GetPlayer();
        if (player.IsAlive() && player.GetInvincibleTicks() == 0)
        {
            player.Kill();
        }
        sim.Step();
    }
    ASSERT_TRUE(sim.State().GetPlayer().IsGameOver());
    for (int i = 0; i < 1000 && sim.Current() == pStage; ++i)
    {
        sim.Step();
    }
    EXPECT_NE(sim.Current(), pStage);
}

TEST(StageIntegration, ScriptedBotRunIsDeterministicOver3000Ticks)
{
    auto run = []() {
        IntegrationSim      sim;
        std::vector<uint64_t> hashes;
        hashes.reserve(3000);
        for (int i = 0; i < 3000; ++i)
        {
            sim.BotStep(true, true);
            hashes.push_back(sim.State().HashState());
        }
        return hashes;
    };
    const std::vector<uint64_t> first  = run();
    const std::vector<uint64_t> second = run();
    ASSERT_EQ(first.size(), second.size());
    EXPECT_EQ(first, second);
}

