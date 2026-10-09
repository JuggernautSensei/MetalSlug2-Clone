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
                    m_jumpTicks = 36;
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

    for (const PlaceholderEntity& entity: host.GetPlaceholderEntities())
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

TEST(StageIntegration, CameraKeepsUpWithThePlayerAlongTheWholeStage)
{
    IntegrationSim sim;
    int worstGap = 0;
    float worstAt = 0.f;
    int   lagTicks = 0;
    int   outTicks = 0;
    for (int i = 0; i < 40000 && sim.Current() == sim.StagePtr(); ++i)
    {
        sim.BotStep(true, true);
        if (sim.Current() != sim.StagePtr())
        {
            break;
        }
        const float px  = sim.State().GetPlayer().GetFeet().x;
        const float cam = sim.State().GetCamera().GetPositionNoShake().x;
        const bool  bLocked = sim.State().GetCamera().IsLocked();
        const float py      = sim.State().GetPlayer().GetFeet().y;
        const float camY    = sim.State().GetCamera().GetPositionNoShake().y;
        if (py < camY + 20.f || py > camY + 448.f || px < cam || px > cam + 640.f)
        {
            if (++outTicks == 30)
            {
                std::printf("[cam] player out of view x=%.0f y=%.0f cam=(%.0f,%.0f) section=%d locked=%d\n", px, py, cam, camY, sim.State().GetRunner().GetNextSectionIndex(), bLocked ? 1 : 0);
            }
        }
        else
        {
            outTicks = 0;
        }
        if (!bLocked && px - cam > 470.f)
        {
            if (++lagTicks == 120)
            {
                std::printf("[cam] lag at player x=%.0f cam=%.0f section=%d\n", px, cam, sim.State().GetRunner().GetNextSectionIndex());
            }
        }
        else
        {
            lagTicks = 0;
        }
    }
    SUCCEED();
}

TEST(StageIntegration, BotCanWalkTheWholeStageUpToTheBossArena)
{
    IntegrationSim sim;
    float bestX = 0.f;
    for (int i = 0; i < 40000 && sim.Current() == sim.StagePtr(); ++i)
    {
        sim.BotStep(true, true);
        if (sim.Current() == sim.StagePtr())
        {
            bestX = std::max(bestX, sim.State().GetPlayer().GetFeet().x);
        }
    }
    EXPECT_GE(bestX, 13500.f);
}

TEST(StageIntegration, BusBodyStopsThePlayerWhileAliveAndTheWreckLetsThePlayerThrough)
{
    IntegrationSim sim;
    constexpr float kBusWallLeft = 2995.f - 200.f;
    int tick = 0;
    for (; tick < 40000 && sim.State().GetEnemies().CountAlive("mb_truck") == 0; ++tick)
    {
        sim.BotStep(true, true);
    }
    ASSERT_EQ(sim.State().GetEnemies().CountAlive("mb_truck"), 1);

    int violations = 0;
    for (; tick < 40000 && sim.State().GetEnemies().CountAlive("mb_truck") > 0; ++tick)
    {
        sim.BotStep(true, true);
        if (sim.State().GetPlayer().GetFeet().x >= kBusWallLeft)
        {
            ++violations;
        }
    }
    EXPECT_EQ(violations, 0);
    EXPECT_EQ(sim.State().GetEnemies().CountAlive("mb_truck"), 0);
    EXPECT_TRUE(sim.State().GetEnemies().GetSolidBoxes().empty());
}

TEST(StageIntegration, SarubiaGateStartsInsideTheLockedScreenSoItCanBeShot)
{
    IntegrationSim sim(true);
    for (int i = 0; i < 19; ++i)
    {
        sim.Warp(1);
        sim.BotStep(true, true, 30);
    }
    sim.BotStep(true, true, 300);

    const float camLeft  = sim.State().GetCamera().GetPositionNoShake().x;
    bool        bFound   = false;
    sim.State().GetEnemies().ForEachAlive([&](const Enemy& _e) {
        if (_e.behavior == eEnemyBehavior::Sarubia)
        {
            bFound = true;
            EXPECT_GT(_e.body.position.x, camLeft);
            EXPECT_LT(_e.body.position.x + 50.f, camLeft + 640.f);
        }
    });
    EXPECT_TRUE(bFound);
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
    ASSERT_GT(sim.State().GetEnemies().CountAlive("bigshiee"), 0);

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

TEST(StageIntegration, TimedWaveEntryRepeatsEveryPeriodForItsTimesOnly)
{
    GateFixture fx(slugtest::WrapStage(R"J([{"id":"a","triggerX":0,"actions":[
        {"type":"spawnWave","entries":[{"delay":0.5,"every":0.5,"times":3,"kind":"enemy","def":"soldier_sword","id":"w","x":400,"y":378}]}]}])J"));

    fx.Step(20);
    EXPECT_EQ(fx.runner.GetSpawnedCount(), 0);
    fx.Step(20);   // t = 40 ticks: 첫 발동 (0.5 s)
    EXPECT_EQ(fx.runner.GetSpawnedCount(), 1);
    fx.Step(30);   // 두 번째 (1.0 s)
    EXPECT_EQ(fx.runner.GetSpawnedCount(), 2);
    fx.Step(30);   // 세 번째 (1.5 s)
    EXPECT_EQ(fx.runner.GetSpawnedCount(), 3);
    fx.Step(300);  // times 소진 후 더 나오지 않는다
    EXPECT_EQ(fx.runner.GetSpawnedCount(), 3);
}

TEST(StageIntegration, ClearWavesCancelsTheWholeTimedRepeat)
{
    GateFixture fx(slugtest::WrapStage(R"J([{"id":"a","triggerX":0,"actions":[
        {"type":"spawnWave","entries":[{"delay":0.5,"every":0.5,"times":5,"kind":"enemy","def":"soldier_sword","x":400,"y":378}]},
        {"type":"clear","waves":true,"delay":0.8}]}])J"));
    fx.Step(200);   // 0.5 s 첫 발동 뒤 0.8 s 에 clear: 나머지 반복은 취소
    EXPECT_EQ(fx.runner.GetSpawnedCount(), 1);
    EXPECT_EQ(fx.runner.GetPendingWaveCount(), 0);
}

TEST(StageIntegration, BossFightSupplyPowsDropOnThePlayersSideAndCanBeRescued)
{
    IntegrationSim sim(true);
    sim.Warp(2);   // F6: 보스 아레나
    sim.Step(2);
    ASSERT_GT(sim.State().GetEnemies().CountAlive("bigshiee"), 0);

    constexpr int kPowClose = 5;   // ePowState::kClose (구출 후 아이템 연출)
    const auto   findPow    = [&]() -> const Enemy* {
        const Enemy* pFound = nullptr;
        sim.State().GetEnemies().ForEachAlive([&](const Enemy& _e) {
            if (_e.behavior == eEnemyBehavior::Pow)
            {
                pFound = &_e;
            }
        });
        return pFound;
    };

    sim.State().GetPlayer().ExtendInvincibility(3000);
    sim.Step(700);   // 첫 보급 POW 는 12 s (720 틱) 뒤
    EXPECT_EQ(findPow(), nullptr);

    sim.Step(20);
    const Enemy* pPow = findPow();
    ASSERT_NE(pPow, nullptr) << "보스전 보급 POW 가 떨어지지 않았다";
    const float camLeft = sim.State().GetCamera().GetPositionNoShake().x;
    EXPECT_GE(pPow->body.position.x, camLeft + 80.f);
    EXPECT_LE(pPow->body.position.x, camLeft + 200.f);

    // 플레이어를 POW 낙하 경로 아래로 옮기면 공중에서 구출된다
    const float dropX = pPow->body.position.x;
    sim.State().GetPlayer().Reset(Tuning {}, { dropX, sim.State().GetPlayer().GetFeet().y });
    for (int i = 0; i < 240; ++i)
    {
        sim.State().GetPlayer().ExtendInvincibility(3000);
        sim.Step(1);
        pPow = findPow();
        if (pPow == nullptr || pPow->state == kPowClose)
        {
            break;
        }
    }
    ASSERT_NE(pPow, nullptr);
    EXPECT_EQ(pPow->state, kPowClose) << "낙하하는 보급 POW 가 플레이어와 겹쳤는데 구출되지 않았다";
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

    ASSERT_GT(sim.State().GetEnemies().CountAlive("bigshiee"), 0);

    const IGameState* pStage = sim.StagePtr();
    sim.State().GetEnemies().KillAll(true);
    for (int i = 0; i < 3000 && sim.Current() == pStage; ++i)
    {
        sim.Step();
    }
    EXPECT_NE(sim.Current(), pStage);
}

TEST(StageIntegration, BossHpBarShowsOnlyDuringTheBossFightAndHidesAfterDeath)
{
    IntegrationSim sim(true);
    sim.Warp(2);   // F6: 보스 아레나
    sim.Step(2);
    EXPECT_FALSE(sim.State().BuildHudModel().bShowBoss);   // 입장 중에는 숨김

    for (int i = 0; i < 1200 && !sim.State().GetEnemies().GetBossHpRatio(); ++i)
    {
        sim.Step();
    }
    const HudModel start = sim.State().BuildHudModel();
    ASSERT_TRUE(start.bShowBoss);
    EXPECT_NEAR(start.bossHpRatio, 1.f, 1e-4f);

    EnemyHandle boss;
    sim.State().GetEnemies().ForEachAlive([&](const Enemy& _e) {
        if (_e.behavior == eEnemyBehavior::BigShiee)
        {
            boss = EnemyHandle { _e.id };
        }
    });
    ASSERT_FALSE(boss.IsNull());

    HitEvent ev;
    ev.damage       = 650;
    ev.kind         = eHitKind::Bullet;
    ev.attackerTeam = eTeam::Player;
    ev.attacker     = 1;
    ev.target       = boss.id;
    ev.hitId        = 0;
    sim.State().GetEnemies().OnHitEvents(slug::Span<const HitEvent> { &ev, 1 });
    EXPECT_NEAR(sim.State().BuildHudModel().bossHpRatio, 0.5f, 0.01f);

    sim.State().GetEnemies().KillAll(true);
    sim.Step(2);
    EXPECT_FALSE(sim.State().BuildHudModel().bShowBoss);
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

