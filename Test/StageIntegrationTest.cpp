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

// ===========================================
//  stage0 통합 테스트 (헤드리스). 실제 Stage0.json + 실제 정의(Enemies/Vehicles/Boss/Props) + 실제 지면으로
//   StageState 를 SimRunner 로 돌린다. 봇은 오른쪽으로 걸으며 주기적으로 쏘고, 칼은 자동 발동에 맡긴다.
//   피해는 무적(디버그 ExtendInvincibility)으로 막아 진행을 본다. 무적이 없을 때의 사망 경로는 별도 검사한다.
// ===========================================

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
        m_ctx.assetRoot  = slug::FilePath { SLUG_ASSET_DIR };
        m_ctx.bDebugDraw = _bDebugDraw;
        m_pState         = new StageState;
        m_runner         = std::unique_ptr<SimRunner> { new SimRunner { m_ctx, std::unique_ptr<IGameState> { m_pState } } };
    }

    [[nodiscard]] StageState&  State() { return *m_pState; }
    [[nodiscard]] IGameState*  Current() const { return m_runner->GetState(); }   // 끝 장면 후에는 스테이지가 교체되므로 포인터 비교만 한다
    [[nodiscard]] const IGameState* StagePtr() const { return m_pState; }
    [[nodiscard]] GameContext& Ctx() { return m_ctx; }

    // 봇 한 틱: 무적 유지, 오른쪽 이동, _bShoot 이면 사격 (24 틱 주기로 3 틱씩 누른다)
    // 벽(상자 충돌체/고체 소품)에 막혀 20 틱 동안 전진하지 못하면: 뒤로 물러난 뒤 달려가며 점프한다 (달리기 점프)
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

            // 앞쪽 40px 안에 키 큰 벽(충돌체/소품 벽)이 있으면 달리며 점프한다 (정점에서 벽 위로 넘어가도록 점프를 누른 채 유지)
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

    // 디버그 워프 요청 (F5 = 1, F6 = 2). 다음 틱에 소비된다.
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
    // 발 앞 46px 안에 발판보다 위로 솟은 벽(상자 충돌체 wall / 소품 벽)이 있는가
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
    StageState*     m_pState   = nullptr;   // m_runner 가 소유한다
    uint64_t        m_botTick  = 0;
    float           m_lastX    = -1.f;
    int             m_stuckTicks = 0;
    int             m_jumpTicks  = 0;
    int             m_backTicks  = 0;
    std::unique_ptr<SimRunner> m_runner = nullptr;   // 가장 마지막에 파괴되어야 OnExit 이 m_ctx 를 본다
};

// 스테이지 종료 경로 검사용: 실제 정의 라이브러리 + StageWorldHost + StageRunner (JSON 스테이지)
class GateFixture
{
public:
    explicit GateFixture(const std::string& _json)
        : data(slugtest::MustLoad(_json))
        , rng(uint64_t { 1 })
        , runner(data, host, camera, rng)
    {
        slug::String message;
        enemyLib = EnemyDefLibrary::LoadDirectory(EnemyDefLibrary::GetDefaultDirectory(slug::FilePath { SLUG_ASSET_DIR }), &message).Take();
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

}   // namespace

// ===========================================
//  정의와 엔티티 배치
// ===========================================

TEST(StageIntegration, Stage0UsesRealEnemiesAndPropsInsteadOfPlaceholders)
{
    IntegrationSim sim;
    sim.Step(2);

    const StageWorldHost& host = sim.State().GetHost();
    EXPECT_GT(sim.State().GetEnemies().CountAlive("soldier_bomb"), 0);
    EXPECT_GT(sim.State().GetProps().GetCount(), 0u);   // intro 의 트럭 두 대

    // 정의가 있는 적/소품은 플레이스홀더로 남지 않는다
    for (const PlaceholderEntity& entity: host.GetPlaceholders().GetEntities())
    {
        EXPECT_EQ(entity.kind, eEntityKind::Decor) << entity.def;   // 현재 정의가 없는 것은 장식(anim_decor/sprite_decor) 뿐이다
    }
}

TEST(StageIntegration, GroundPatchMaskAndApplyClearTheDestroyedHouseFootprint)
{
    // 집 픽셀맵(Stage0_House_4_PixelMap): 레거시 규칙상 고체(파란 픽셀)가 없고 전부 공기다.
    // 파괴 패치는 발자국 안의 지면을 비워야 하며, 발자국 밖은 그대로 남아야 한다.
    const std::optional<GroundMap> mask = LoadGroundPatchMask(slug::FilePath { SLUG_ASSET_DIR } / "Baked" / "Textures" / "Stage0_House_4_PixelMap.png");
    ASSERT_TRUE(mask.has_value());
    EXPECT_EQ(mask->GetWidth(), 1306);
    EXPECT_EQ(mask->GetHeight(), 751);
    EXPECT_FALSE(mask->IsSolid(0, 0));

    GroundMap ground = GroundMap::MakeEmpty(2000, 1000);
    ground.FillRect(0, 0, 1999, 999, true);
    ApplyGroundPatch(ground, { 100.f, 100.f }, *mask);
    EXPECT_FALSE(ground.IsSolid(105, 105));    // 발자국 안: 공기
    EXPECT_TRUE(ground.IsSolid(50, 50));       // 발자국 밖: 그대로 고체

    EXPECT_FALSE(LoadGroundPatchMask(slug::FilePath { SLUG_ASSET_DIR } / "Baked" / "Textures" / "no_such_map.png").has_value());
}

TEST(StageIntegration, SpawnedEnemiesStandOnTheGroundAtTheirScriptedFeet)
{
    IntegrationSim sim;
    sim.Step(2);
    // 첫 섹션(intro) 의 soldier_bomb (x 900) 은 지면 y 378 에 서 있어야 한다
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
    // 워프 없이 intro → street 를 걷는다. x≈1119 의 box_collider(벽) 는 점프(정점 약 72px)로 넘을 수 있어야 한다.
    // (레거시 90px 높이는 새 점프로 못 넘어 48px 로 낮췄다 — Stage0.json)
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

// 전 구간 연쇄 워프 (F5 를 섹션 수만큼 눌러 모든 섹션을 silent/일반 실행) + 봇 120 틱씩 → F6 보스 → 보스 처치 → 흐름 종료.
// 워프 경로에서도 스폰/게이트/소품 지붕/지면 교체가 충돌 없이 돌아감을 확인한다.
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

// ===========================================
//  판정 / 점수 / 게이트
// ===========================================

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
    IntegrationSim sim(true);   // 워프 요청은 F1(디버그) 상태에서만 소비된다
    // 거리 섹션(street)까지 워프 → 상점 지붕이 생긴다 (소품은 silent 실행으로도 놓인다)
    sim.Warp(1, 2);
    sim.Step(2);

    // shop_5 (x 2513, 중심 y 181) 의 지붕 위로 떨어뜨린다
    sim.State().GetPlayer().Reset(Tuning {}, { 2513.f, 60.f });
    sim.Step(240);
    const Vec2 feet = sim.State().GetPlayer().GetFeet();
    EXPECT_TRUE(sim.State().GetPlayer().IsGrounded());
    EXPECT_NEAR(feet.x, 2513.f, 80.f);
    EXPECT_LT(feet.y, 300.f);   // 지면(378)이 아니라 지붕 위
}

// ===========================================
//  끝까지: 워프 + 보스 처치 → MISSION COMPLETE → 타이틀
// ===========================================

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
    // 보스 사망 → 사망 연출 → endStage → MISSION COMPLETE 흐름 → 타이틀 상태로 교체
    EXPECT_NE(sim.Current(), pStage);
}

TEST(StageIntegration, GameOverRunsTheGameOverFlowAndReturnsToTitle)
{
    // 무적이 없을 때의 실제 사망 경로: 목숨이 다하면 GAME OVER 흐름을 거쳐 타이틀로 간다
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

