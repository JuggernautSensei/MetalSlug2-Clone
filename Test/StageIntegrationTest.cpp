// 스테이지 통합 테스트: StageState 와 월드 호스트가 실제 에셋으로 스테이지를 진행하는지 검증
#include <algorithm>
#include <cstdint>
#include <optional>
#include <set>
#include <vector>

#include <gtest/gtest.h>

#include "Base/Math.h"
#include "Base/Random.h"
#include "Base/Types.h"
#include "Core/AssetPath.h"
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
        m_runner         = std::unique_ptr<SimRunner> {
            new SimRunner { m_ctx, std::unique_ptr<IGameState> { m_pState } }
        };
    }

    [[nodiscard]] StageState&       State() { return *m_pState; }
    [[nodiscard]] IGameState*       Current() const { return m_runner->GetState(); }
    [[nodiscard]] const IGameState* StagePtr() const { return m_pState; }
    [[nodiscard]] GameContext&      Ctx() { return m_ctx; }

    void BotStep(
        const bool _bRight,
        const bool _bShoot,
        const int  _count = 1)
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

    void Step(
        const int _count = 1)
    {
        for (int i = 0; i < _count; ++i)
        {
            m_frame = {};
            m_runner->Step(m_frame);
        }
    }

    void Hold(const eAction _action) { m_frame.Set(_action, true); }

    void Warp(
        const int _code,
        const int _count = 1)
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

    slug::RngMt19937           m_rng;
    GameContext                m_ctx        = {};
    InputFrame                 m_frame      = {};
    StageState*                m_pState     = nullptr;
    uint64_t                   m_botTick    = 0;
    float                      m_lastX      = -1.f;
    int                        m_stuckTicks = 0;
    int                        m_jumpTicks  = 0;
    int                        m_backTicks  = 0;
    std::unique_ptr<SimRunner> m_runner     = nullptr;
};

class GateFixture
{
public:
    explicit GateFixture(
        const std::string& _json)
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

    void Step(
        const int _count = 1)
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

    StageData        data;
    StageWorldHost   host   = {};
    Camera           camera = {};
    slug::RngMt19937 rng;
    StageRunner      runner;
    EnemyDefLibrary  enemyLib = {};
    PropDefLibrary   propLib  = {};
    EnemyWorld       enemies { nullptr };
    PropWorld        props { nullptr, 1 };
    PlayerController player = {};
    Tuning           tuning = {};
};
}   // namespace

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

TEST(StageIntegration, GroundHouseFootprintClearsOnlyTheHouseAndKeepsTheStreet)
{
    const slug::FilePath           root = slug::FindAssetRoot();
    const std::optional<GroundMap> mask = LoadGroundPatchMask(root / "Baked" / "Textures" / "Stage0_House_4_PixelMap.png");
    ASSERT_TRUE(mask.has_value());
    EXPECT_EQ(mask->GetWidth(), 1306);
    EXPECT_EQ(mask->GetHeight(), 751);
    EXPECT_FALSE(mask->IsSolid(0, 0));
    EXPECT_GT(mask->CountSolidPixels(), 5000u);   // 도로 위 봉우리 한 덩어리 (빈 마스크가 아님)

    slug::Result<GroundMap> loaded = GroundMap::Load(root / "Baked" / "Ground" / "Stage0_0.bin");
    ASSERT_TRUE(loaded);
    GroundMap      ground       = loaded.Take();
    const uint64_t streetBefore = ground.CountSolidPixels();   // 전체 개수는 봉우리 제거 후 정확히 줄어야 한다
    EXPECT_TRUE(ground.IsSolid(9830, 1230));                   // 집 몸체 (봉우리)

    // 집 (9830,1269) 의 지면 패치 좌상단 = center - (size/2 - inset)
    EXPECT_TRUE(ApplyGroundPatch(ground, { 9189.f, 898.f }, *mask));
    EXPECT_FALSE(ground.IsSolid(9830, 1230));   // 몸체: 공기
    EXPECT_FALSE(ground.IsSolid(9760, 1270));   // 봉우리 왼쪽 경사: 공기
    EXPECT_TRUE(ground.IsSolid(9830, 1300));    // 도로 (발자국 밖): 그대로
    EXPECT_TRUE(ground.IsSolid(9189, 1300));
    EXPECT_TRUE(ground.IsSolid(10400, 1300));
    EXPECT_EQ(ground.CountSolidPixels(), streetBefore - mask->CountSolidPixels());
}

TEST(StageIntegration, BigHouseFootprintClearsItsCapAndTheStreetStaysSolid)
{
    const slug::FilePath           root = slug::FindAssetRoot();
    const std::optional<GroundMap> mask = LoadGroundPatchMask(root / "Baked" / "Textures" / "Stage0_House_5_PixelMap.png");
    ASSERT_TRUE(mask.has_value());
    EXPECT_EQ(mask->GetWidth(), 555);
    EXPECT_EQ(mask->GetHeight(), 124);
    EXPECT_GT(mask->CountSolidPixels(), 8000u);

    slug::Result<GroundMap> loaded = GroundMap::Load(root / "Baked" / "Ground" / "Stage0_0.bin");
    ASSERT_TRUE(loaded);
    GroundMap ground = loaded.Take();
    EXPECT_TRUE(ground.IsSolid(10450, 1210));                            // 집 몸체 윗부분
    EXPECT_TRUE(ApplyGroundPatch(ground, { 10204.f, 1204.f }, *mask));   // 큰 집 @10470
    EXPECT_FALSE(ground.IsSolid(10450, 1210));
    EXPECT_TRUE(ground.IsSolid(10450, 1300));   // 도로는 그대로
    EXPECT_TRUE(ground.IsSolid(9830, 1230));    // 옆 집 봉우리 (패치 밖) 도 그대로

    EXPECT_FALSE(LoadGroundPatchMask(root / "Baked" / "Textures" / "no_such_map.png").has_value());
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
    int            worstGap = 0;
    float          worstAt  = 0.f;
    int            lagTicks = 0;
    int            outTicks = 0;
    for (int i = 0; i < 40000 && sim.Current() == sim.StagePtr(); ++i)
    {
        sim.BotStep(true, true);
        if (sim.Current() != sim.StagePtr())
        {
            break;
        }
        const float px      = sim.State().GetPlayer().GetFeet().x;
        const float cam     = sim.State().GetCamera().GetPositionNoShake().x;
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

TEST(StageIntegration, BotCanWalkTheWholeStageToTheStageEnd)
{
    IntegrationSim sim;
    float          bestX = 0.f;
    for (int i = 0; i < 40000 && sim.Current() == sim.StagePtr(); ++i)
    {
        sim.BotStep(true, true);
        if (sim.Current() == sim.StagePtr())
        {
            bestX = std::max(bestX, sim.State().GetPlayer().GetFeet().x);
        }
    }
    EXPECT_GE(bestX, 12900.f);
    EXPECT_NE(sim.Current(), sim.StagePtr()) << "마지막 섹션 (stage_end) 에서 스테이지가 끝나야 한다";
}

TEST(StageIntegration, BotStillWalksToTheStageEndAfterEveryOpenlandPropIsDestroyed)
{
    // 회귀: 개활지(집/큰 집/탑/다리 블록) 의 파괴물을 전부 부숴도 지면 패치가 도로를 뚫어 진행이 막히면 안 된다.
    // 섹션 트리거가 지나야 소품이 나오므로, 봇이 전진하면서 개활지 소품을 매 틱 맞춰 부순다.
    IntegrationSim sim(true);   // 워프(F5)는 디버그 모드에서만 동작한다
    for (int i = 0; i < 40 && sim.State().GetPlayer().GetFeet().x < 8990.f; ++i)
    {
        sim.Warp(1);   // F5: 다음 섹션 (개활지 시작까지)
    }
    ASSERT_GE(sim.State().GetPlayer().GetFeet().x, 8990.f);

    std::set<uint32_t> destroyedIds;
    float              bestX        = 0.f;
    float              maxFeetY     = 0.f;
    bool               bRoadChecked = false;
    for (int i = 0; i < 40000 && sim.Current() == sim.StagePtr(); ++i)
    {
        PropWorld&             props = sim.State().GetProps();
        slug::Vector<HitEvent> hits;
        for (const PropInstance& inst: props.GetProps())
        {
            if (inst.pos.x >= 8980.f && inst.pos.x < 13500.f && !inst.bDestroyed)
            {
                HitEvent ev;
                ev.damage       = 9999;
                ev.kind         = eHitKind::Bullet;
                ev.attackerTeam = eTeam::Player;
                ev.attacker     = 1;
                ev.target       = inst.id;
                hits.push_back(ev);
                destroyedIds.insert(inst.id);
            }
        }
        if (!hits.empty())
        {
            props.OnHitEvents(slug::Span<const HitEvent> { hits.data(), hits.size() });
        }

        sim.BotStep(true, true);
        if (sim.Current() != sim.StagePtr())
        {
            break;
        }
        const Vec2 feet = sim.State().GetPlayer().GetFeet();
        bestX           = std::max(bestX, feet.x);
        if (feet.x >= 9000.f)
        {
            maxFeetY = std::max(maxFeetY, feet.y);
        }
        if (feet.x >= 9000.f && feet.x <= 10600.f && i % 10 == 0)
        {
            std::printf("[proof] tick %d feet=(%.1f, %.1f) section=%d\n", i, feet.x, feet.y, sim.State().GetRunner().GetNextSectionIndex());
        }
        if (!bRoadChecked && feet.x >= 12850.f)
        {
            // 파괴물이 모두 부서진 뒤의 도로: 9000..12850 구간의 바닥 행이 전부 고체여야 한다
            const GroundMap* pGround = sim.State().GetHost().GetActiveGround();
            ASSERT_NE(pGround, nullptr);
            for (int x = 9000; x < 12850; x += 10)
            {
                ASSERT_TRUE(pGround->IsSolid(x, 1300)) << "도로가 x=" << x << " 에서 뚫렸다";
            }
            bRoadChecked = true;
        }
    }
    EXPECT_GE(destroyedIds.size(), 6u) << "개활지 파괴물이 충분히 나와야 한다";
    EXPECT_GE(bestX, 12900.f) << "bestX=" << bestX;
    EXPECT_LT(maxFeetY, 1300.f) << "플레이어가 도로 아래로 떨어졌다";
    EXPECT_TRUE(bRoadChecked);
}

TEST(StageIntegration, SarubiaGateStartsInsideTheLockedScreenSoItCanBeShot)
{
    IntegrationSim sim(true);
    for (int i = 0; i < 18; ++i)
    {
        sim.Warp(1);
        sim.BotStep(true, true, 30);
    }
    sim.BotStep(true, true, 300);

    const float camLeft = sim.State().GetCamera().GetPositionNoShake().x;
    bool        bFound  = false;
    for (const Enemy& enemy: sim.State().GetEnemies().GetEnemies())
    {
        if (enemy.life == eEnemyLife::Alive && enemy.behavior == eEnemyBehavior::Sarubia)
        {
            bFound = true;
            EXPECT_GT(enemy.body.position.x, camLeft);
            EXPECT_LT(enemy.body.position.x + 50.f, camLeft + 640.f);
        }
    }
    EXPECT_TRUE(bFound);
}

TEST(StageIntegration, WarpChainThroughEverySectionReachesTheStageEndAndEndsTheStage)
{
    IntegrationSim sim(true);
    const int      sectionCount = static_cast<int>(sim.State().GetData().sections.size());
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

    const IGameState* pStage = sim.StagePtr();
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
    fx.Step(300);   // times 소진 후 더 나오지 않는다
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

TEST(StageIntegration, DebugWarpToTheStageEndEndsTheStageThroughTheFlow)
{
    IntegrationSim sim(true);
    sim.Warp(2);   // F6: 마지막 구간 (stage_end)
    sim.Step(2);

    const IGameState* pStage = sim.StagePtr();
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

namespace
{
// 스크립트 봇을 3000틱 돌리며 매 틱의 상태 해시를 모은다
[[nodiscard]] std::vector<uint64_t> RunBotHashes_()
{
    IntegrationSim        sim;
    std::vector<uint64_t> hashes;
    hashes.reserve(3000);
    for (int i = 0; i < 3000; ++i)
    {
        sim.BotStep(true, true);
        hashes.push_back(sim.State().HashState());
    }
    return hashes;
}
}   // namespace

TEST(StageIntegration, ScriptedBotRunIsDeterministicOver3000Ticks)
{
    const std::vector<uint64_t> first  = RunBotHashes_();
    const std::vector<uint64_t> second = RunBotHashes_();
    ASSERT_EQ(first.size(), second.size());
    EXPECT_EQ(first, second);
}
