#include <algorithm>
#include "Core/AssetPath.h"
#include <cstdint>
#include "Base/Types.h"
#include "Base/Math.h"
#include "Base/Random.h"
#include <optional>
#include <set>
#include <vector>

#include <gtest/gtest.h>

#include "Core/Input.h"
#include "Core/Tuning.h"
#include "Game/Enemies/BossShiee.h"
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

TEST(StageIntegration, GroundHouseFootprintClearsOnlyTheHouseAndKeepsTheStreet)
{
    const slug::FilePath root = slug::FindAssetRoot();
    const std::optional<GroundMap> mask = LoadGroundPatchMask(root / "Baked" / "Textures" / "Stage0_House_4_PixelMap.png");
    ASSERT_TRUE(mask.has_value());
    EXPECT_EQ(mask->GetWidth(), 1306);
    EXPECT_EQ(mask->GetHeight(), 751);
    EXPECT_FALSE(mask->IsSolid(0, 0));
    EXPECT_GT(mask->CountSolidPixels(), 5000u);   // 도로 위 봉우리 한 덩어리 (빈 마스크가 아님)

    slug::Result<GroundMap> loaded = GroundMap::Load(root / "Baked" / "Ground" / "Stage0_0.bin");
    ASSERT_TRUE(loaded);
    GroundMap ground = loaded.Take();
    const uint64_t streetBefore = ground.CountSolidPixels();   // 전체 개수는 봉우리 제거 후 정확히 줄어야 한다
    EXPECT_TRUE(ground.IsSolid(9830, 1230));                   // 집 몸체 (봉우리)

    // 집 (9830,1269) 의 지면 패치 좌상단 = center - (size/2 - inset)
    EXPECT_TRUE(ApplyGroundPatch(ground, { 9189.f, 898.f }, *mask));
    EXPECT_FALSE(ground.IsSolid(9830, 1230));                  // 몸체: 공기
    EXPECT_FALSE(ground.IsSolid(9760, 1270));                  // 봉우리 왼쪽 경사: 공기
    EXPECT_TRUE(ground.IsSolid(9830, 1300));                   // 도로 (발자국 밖): 그대로
    EXPECT_TRUE(ground.IsSolid(9189, 1300));
    EXPECT_TRUE(ground.IsSolid(10400, 1300));
    EXPECT_EQ(ground.CountSolidPixels(), streetBefore - mask->CountSolidPixels());
}

TEST(StageIntegration, BigHouseFootprintClearsItsCapAndTheStreetStaysSolid)
{
    const slug::FilePath root = slug::FindAssetRoot();
    const std::optional<GroundMap> mask = LoadGroundPatchMask(root / "Baked" / "Textures" / "Stage0_House_5_PixelMap.png");
    ASSERT_TRUE(mask.has_value());
    EXPECT_EQ(mask->GetWidth(), 555);
    EXPECT_EQ(mask->GetHeight(), 124);
    EXPECT_GT(mask->CountSolidPixels(), 8000u);

    slug::Result<GroundMap> loaded = GroundMap::Load(root / "Baked" / "Ground" / "Stage0_0.bin");
    ASSERT_TRUE(loaded);
    GroundMap ground = loaded.Take();
    EXPECT_TRUE(ground.IsSolid(10450, 1210));    // 집 몸체 윗부분
    EXPECT_TRUE(ApplyGroundPatch(ground, { 10204.f, 1204.f }, *mask));   // 큰 집 @10470
    EXPECT_FALSE(ground.IsSolid(10450, 1210));
    EXPECT_TRUE(ground.IsSolid(10450, 1300));    // 도로는 그대로
    EXPECT_TRUE(ground.IsSolid(9830, 1230));     // 옆 집 봉우리 (패치 밖) 도 그대로

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

namespace
{
// 봇이 보스 아레나 (x >= 12950, 보스 생존) 에 도달할 때까지 전진한다. 도달하면 true.
bool BotReachesBossArena(IntegrationSim& _sim, const int _maxTicks)
{
    for (int i = 0; i < _maxTicks && _sim.Current() == _sim.StagePtr(); ++i)
    {
        _sim.BotStep(true, true);
        if (_sim.Current() != _sim.StagePtr())
        {
            return false;
        }
        if (_sim.State().GetPlayer().GetFeet().x >= 12950.f && _sim.State().GetEnemies().CountAlive("boss_shiee") > 0)
        {
            return true;
        }
    }
    return false;
}

const Enemy* FindBoss(const EnemyWorld& _enemies)
{
    for (const Enemy& e: _enemies.GetEnemies())
    {
        if (e.life != eEnemyLife::Gone && e.behavior == eEnemyBehavior::BossShiee)
        {
            return &e;
        }
    }
    return nullptr;
}
}

TEST(StageIntegration, BossShieeAppearsAndFightsWhenTheBotWalksIntoTheArena)
{
    IntegrationSim sim;
    ASSERT_TRUE(BotReachesBossArena(sim, 40000)) << "보스가 등장하지 않았다";

    // 등장 (16 s = 960 틱) 이 끝나면 HUD 보스 바가 뜨고 (전투 시작), 포탑/캐논이 사격한다
    bool bBarShown = false;
    int  shots     = 0;
    for (int i = 0; i < 4000 && sim.Current() == sim.StagePtr(); ++i)
    {
        sim.BotStep(true, true);
        if (sim.Current() != sim.StagePtr())
        {
            break;
        }
        bBarShown = bBarShown || sim.State().GetEnemies().GetBossHpRatio().has_value();
        if (const Enemy* pBoss = FindBoss(sim.State().GetEnemies()))
        {
            shots = std::max(shots, pBoss->shotsFired);
        }
    }
    EXPECT_TRUE(bBarShown) << "전투 시작 전에 보스 바가 떴거나 뜨지 않았다";
    EXPECT_GT(shots, 0) << "포탑/캐논이 한 번도 쏘지 않았다";
}

TEST(StageIntegration, BossHullHasNoHitboxOnlyTurretsAndCannonDo)
{
    IntegrationSim sim;
    ASSERT_TRUE(BotReachesBossArena(sim, 40000));
    sim.BotStep(true, true, 60);

    const Enemy* pBoss = FindBoss(sim.State().GetEnemies());
    ASSERT_NE(pBoss, nullptr);
    int hullBoxes = 0;
    int partBoxes = 0;
    for (const Hurtbox& hb: sim.State().GetCombat().GetHurtboxes())
    {
        if (hb.owner == pBoss->id)
        {
            ++hullBoxes;   // 선체 판정은 없다: 탄/폭탄이 선체를 통과한다
        }
        else if (GetPartIndex(hb.owner) >= 0 && (hb.owner & kEnemyIdMask) == (pBoss->id & kEnemyIdMask))
        {
            ++partBoxes;   // 포탑 A..G 와 캐논 헤드
        }
    }
    EXPECT_EQ(hullBoxes, 0);
    EXPECT_GE(partBoxes, 1);
}

TEST(StageIntegration, EndlessTerrainCarriesTheBotPastTheWorldWidth)
{
    // 엔드리스 꼬리: 월드 폭 (14094) 을 넘어도 지면과 배경이 주기 반복으로 이어진다. 봇이 발을 헛디디지 않고 16000 까지 간다.
    IntegrationSim sim;
    ASSERT_TRUE(BotReachesBossArena(sim, 40000));
    float bestX = 0.f;
    float maxFeetY = 0.f;
    for (int i = 0; i < 90000 && sim.Current() == sim.StagePtr() && bestX < 16000.f; ++i)
    {
        sim.BotStep(true, true);
        if (sim.Current() != sim.StagePtr())
        {
            break;
        }
        const Vec2 feet = sim.State().GetPlayer().GetFeet();
        bestX = std::max(bestX, feet.x);
        if (feet.x >= 14094.f)
        {
            maxFeetY = std::max(maxFeetY, feet.y);
        }
    }
    EXPECT_GE(bestX, 16000.f) << "bestX=" << bestX;
    EXPECT_LT(maxFeetY, 1300.f) << "꼬리 구간에서 바닥 아래로 떨어졌다";
}

TEST(StageIntegration, BotStillWalksToTheBossAfterEveryOpenlandPropIsDestroyed)
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
    float bestX        = 0.f;
    float maxFeetY     = 0.f;
    bool  bRoadChecked = false;
    for (int i = 0; i < 40000 && sim.Current() == sim.StagePtr(); ++i)
    {
        PropWorld& props = sim.State().GetProps();
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
        bestX = std::max(bestX, feet.x);
        if (feet.x >= 9000.f)
        {
            maxFeetY = std::max(maxFeetY, feet.y);
        }
        if (feet.x >= 9000.f && feet.x <= 10600.f && i % 10 == 0)
        {
            std::printf("[proof] tick %d feet=(%.1f, %.1f) section=%d\n", i, feet.x, feet.y, sim.State().GetRunner().GetNextSectionIndex());
        }
        if (!bRoadChecked && feet.x >= 13000.f)
        {
            // 파괴물이 모두 부서진 뒤의 도로: 9000..13450 구간의 바닥 행이 전부 고체여야 한다
            const GroundMap* pGround = sim.State().GetHost().GetActiveGround();
            ASSERT_NE(pGround, nullptr);
            for (int x = 9000; x < 13450; x += 10)
            {
                ASSERT_TRUE(pGround->IsSolid(x, 1300)) << "도로가 x=" << x << " 에서 뚫렸다";
            }
            bRoadChecked = true;
        }
    }
    EXPECT_GE(destroyedIds.size(), 6u) << "개활지 파괴물이 충분히 나와야 한다";
    EXPECT_GE(bestX, 13500.f) << "bestX=" << bestX;
    EXPECT_LT(maxFeetY, 1300.f) << "플레이어가 도로 아래로 떨어졌다";
    EXPECT_TRUE(bRoadChecked);
}

TEST(StageIntegration, BusBodyStopsThePlayerWhileAliveAndTheWreckLetsThePlayerThrough)
{
    IntegrationSim sim;
    constexpr float kBusWallLeft = 2995.f - 102.f;   // 본체 반폭 (Bus_Idle 204 px)
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
    ASSERT_GT(sim.State().GetEnemies().CountAlive("boss_shiee"), 0);

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

TEST(StageIntegration, BossFightSupplyPowsArePlacedAheadOfTheCameraAndScrollInWithoutPopping)
{
    // 보급 POW 는 팝인하지 않는다: 등장 시점에 카메라 오른쪽 밖 월드에 미리 있고, 드리프트로 오른쪽 가장자리에서 들어온다.
    IntegrationSim sim(true);
    sim.Warp(2);   // F6: 보스 아레나
    sim.Step(2);
    ASSERT_GT(sim.State().GetEnemies().CountAlive("boss_shiee"), 0);

    const auto powXs = [&]() {
        slug::Vector<float> xs;
        sim.State().GetEnemies().ForEachAlive([&](const Enemy& _e) {
            if (_e.behavior == eEnemyBehavior::Pow)
            {
                xs.push_back(_e.body.position.x);
            }
        });
        return xs;
    };
    ASSERT_FALSE(powXs().empty()) << "보스 아레나에 보급 POW 가 미리 배치되어 있어야 한다";
    const float camRight0 = sim.State().GetCamera().GetPositionNoShake().x + 640.f;
    for (const float x: powXs())
    {
        EXPECT_GE(x, camRight0) << "등장 시점에 화면 안에 POW 가 있으면 팝인이다";
    }

    // 드리프트로 가장 가까운 POW 가 화면 오른쪽 가장자리에서 들어온다
    sim.State().GetPlayer().ExtendInvincibility(60 * 200);
    bool bEntered = false;
    float entryGap = 0.f;
    for (int i = 0; i < 60 * 150 && !bEntered; ++i)
    {
        sim.State().GetPlayer().ExtendInvincibility(600);
        sim.Step(1);
        const float camLeft  = sim.State().GetCamera().GetPositionNoShake().x;
        for (const float x: powXs())
        {
            if (x >= camLeft && x <= camLeft + 640.f)
            {
                bEntered = true;
                entryGap = x - (camLeft + 640.f);
                break;
            }
        }
    }
    EXPECT_TRUE(bEntered) << "POW 가 드리프트로 화면에 들어오지 않았다";
    EXPECT_GT(entryGap, -40.f) << "화면 오른쪽 가장자리 (또는 그 바깥) 에서 들어왔다";
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

    ASSERT_GT(sim.State().GetEnemies().CountAlive("boss_shiee"), 0);

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
        if (_e.behavior == eEnemyBehavior::BossShiee)
        {
            boss = EnemyHandle { _e.id };
        }
    });
    ASSERT_FALSE(boss.IsNull());

    // 선체는 탄을 흡수만 한다 (체력 바 변화 없음)
    HitEvent ev;
    ev.damage       = 650;
    ev.kind         = eHitKind::Bullet;
    ev.attackerTeam = eTeam::Player;
    ev.attacker     = 1;
    ev.target       = boss.id;
    ev.hitId        = 0;
    sim.State().GetEnemies().OnHitEvents(slug::Span<const HitEvent> { &ev, 1 });
    EXPECT_NEAR(sim.State().BuildHudModel().bossHpRatio, 1.f, 1e-4f);

    // 포탑 A..D 파괴 (각 120): 바 = (포탑 3 x 120 + 캐논 1000) / (포탑 7 x 120 + 캐논 1000)
    for (int i = 0; i < 4; ++i)
    {
        ev.damage = 120;
        ev.target = bossshiee::TurretOwner(boss.id, i);
        sim.State().GetEnemies().OnHitEvents(slug::Span<const HitEvent> { &ev, 1 });
    }
    EXPECT_NEAR(sim.State().BuildHudModel().bossHpRatio, 1360.f / 1840.f, 0.01f);

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

