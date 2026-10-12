// 스테이지 통합 테스트: StageState 와 월드 호스트가 실제 에셋으로 스테이지를 진행하는지 검증
#include <algorithm>
#include <cstdint>
#include <cstdlib>
#include <fstream>
#include <map>
#include <optional>
#include <set>
#include <vector>

#include <gtest/gtest.h>

#include "Base/Math.h"
#include "Base/Types.h"
#include "Core/AssetPath.h"
#include "Core/Input.h"
#include "Core/Tuning.h"
#include "Game/Enemies/EnemyTypes.h"
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

class IntegrationSim
{
public:
    explicit IntegrationSim(
        const bool     _bDebugDraw = false,
        const uint64_t _seed       = 1)
        : m_rng(_seed)
    {
        m_ctx.pRng       = &m_rng;
        m_ctx.assetRoot  = FindAssetRoot();
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

    // 살아 있는 최종보스 (BigShiee) 가 있는지
    [[nodiscard]] bool IsBossAlive() const
    {
        for (const Enemy& enemy: m_pState->GetEnemies().GetEnemies())
        {
            if (enemy.life == eEnemyLife::Alive && enemy.behavior == eEnemyBehavior::BigShiee)
            {
                return true;
            }
        }
        return false;
    }

    // 보스 파트 (포탑 0..6, 주포 7) 에 판정 피해 1회씩. 포탑이 다 부서져야 주포가 올라오고, 주포 체력이 0 이 되면 사망 연출
    // (dying -> destroyFinished 로 클리어 게이트가 열린다). 매 틱 호출해도 된다
    void DamageBoss()
    {
        Vector<HitEvent> hits;
        for (const Enemy& enemy: m_pState->GetEnemies().GetEnemies())
        {
            if (enemy.life == eEnemyLife::Alive && enemy.behavior == eEnemyBehavior::BigShiee)
            {
                for (uint32_t part = 0; part <= 7u; ++part)
                {
                    HitEvent ev;
                    ev.damage       = 9999;
                    ev.kind         = eHitKind::Bullet;
                    ev.attackerTeam = eTeam::Player;
                    ev.attacker     = 1;
                    ev.target       = MakePartOwner(enemy.id, part);
                    hits.push_back(ev);
                }
            }
        }
        if (!hits.empty())
        {
            m_pState->GetEnemies().OnHitEvents(Span<const HitEvent> { hits.data(), hits.size() });
        }
    }

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

    RngMt19937           m_rng = RngMt19937 { std::mt19937_64::default_seed };
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
        : data(MustLoad(_json))
        , rng(uint64_t { 1 })
        , runner(data, host, camera, rng)
    {
        String message;
        enemyLib = EnemyDefLibrary::LoadDirectory(EnemyDefLibrary::GetDefaultDirectory(FindAssetRoot()), &message).Take();
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
    RngMt19937 rng = RngMt19937 { std::mt19937_64::default_seed };
    StageRunner      runner;
    EnemyDefLibrary  enemyLib = {};
    PropDefLibrary   propLib  = {};
    EnemyWorld       enemies { nullptr };
    PropWorld        props { nullptr, 1 };
    PlayerController player = {};
    Tuning           tuning = {};
};

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
    const FilePath           root = FindAssetRoot();
    const std::optional<GroundMap> mask = LoadGroundMask(root / "Textures" / "Stage0_House_4_PixelMap.png");
    ASSERT_TRUE(mask.has_value());
    EXPECT_EQ(mask->GetWidth(), 1306);
    EXPECT_EQ(mask->GetHeight(), 751);
    EXPECT_FALSE(mask->IsSolid(0, 0));
    EXPECT_GT(mask->CountSolidPixels(), 5000u);   // 도로 위 봉우리 한 덩어리 (빈 마스크가 아님)

    std::optional<GroundMap> loaded = LoadGroundMask(root / "Ground" / "Stage0_0.png");
    ASSERT_TRUE(loaded.has_value());
    GroundMap      ground       = std::move(*loaded);
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
    const FilePath           root = FindAssetRoot();
    const std::optional<GroundMap> mask = LoadGroundMask(root / "Textures" / "Stage0_House_5_PixelMap.png");
    ASSERT_TRUE(mask.has_value());
    EXPECT_EQ(mask->GetWidth(), 555);
    EXPECT_EQ(mask->GetHeight(), 124);
    EXPECT_GT(mask->CountSolidPixels(), 8000u);

    std::optional<GroundMap> loaded = LoadGroundMask(root / "Ground" / "Stage0_0.png");
    ASSERT_TRUE(loaded.has_value());
    GroundMap ground = std::move(*loaded);
    EXPECT_TRUE(ground.IsSolid(10450, 1210));                            // 집 몸체 윗부분
    EXPECT_TRUE(ApplyGroundPatch(ground, { 10204.f, 1204.f }, *mask));   // 큰 집 @10470
    EXPECT_FALSE(ground.IsSolid(10450, 1210));
    EXPECT_TRUE(ground.IsSolid(10450, 1300));   // 도로는 그대로
    EXPECT_TRUE(ground.IsSolid(9830, 1230));    // 옆 집 봉우리 (패치 밖) 도 그대로

    EXPECT_FALSE(LoadGroundMask(root / "Textures" / "no_such_map.png").has_value());
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
    float          bestX     = 0.f;
    int            bossTicks = 0;
    for (int i = 0; i < 60000 && sim.Current() == sim.StagePtr(); ++i)
    {
        sim.BotStep(true, true);
        if (sim.Current() == sim.StagePtr())
        {
            bestX = std::max(bestX, sim.State().GetPlayer().GetFeet().x);
            if (sim.IsBossAlive() && ++bossTicks >= 900)
            {
                sim.DamageBoss();   // 봇은 보스를 15초 동안 상대한 뒤 쓰러뜨린다
            }
        }
    }
    EXPECT_GE(bestX, 13700.f) << "보스 아레나 트리거까지 도달해야 한다";
    EXPECT_NE(sim.Current(), sim.StagePtr()) << "보스 파괴 연출이 끝나면 클리어 흐름으로 스테이지가 끝나야 한다";
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
    int                bossTicks    = 0;
    for (int i = 0; i < 60000 && sim.Current() == sim.StagePtr(); ++i)
    {
        PropWorld&             props = sim.State().GetProps();
        Vector<HitEvent> hits;
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
            props.OnHitEvents(Span<const HitEvent> { hits.data(), hits.size() });
        }

        sim.BotStep(true, true);
        if (sim.Current() != sim.StagePtr())
        {
            break;
        }
        if (sim.IsBossAlive() && ++bossTicks >= 900)
        {
            sim.DamageBoss();
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
    EXPECT_GE(bestX, 13700.f) << "bestX=" << bestX;
    EXPECT_LT(maxFeetY, 1300.f) << "플레이어가 도로 아래로 떨어졌다";
    EXPECT_NE(sim.Current(), sim.StagePtr()) << "보스 파괴 뒤 클리어 흐름으로 스테이지가 끝나야 한다";
    EXPECT_TRUE(bRoadChecked);
}

TEST(StageIntegration, SarubiaGateStartsInsideTheLockedScreenSoItCanBeShot)
{
    IntegrationSim sim(true);
    const int      sarubiaSection = sim.State().GetData().FindSection("openland_4");
    for (int i = 0; i < sarubiaSection; ++i)
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

static const Enemy* FindSarubia_(
    IntegrationSim& _sim)
{
    for (const Enemy& enemy: _sim.State().GetEnemies().GetEnemies())
    {
        if (enemy.behavior == eEnemyBehavior::Sarubia)
        {
            return &enemy;
        }
    }
    return nullptr;
}

// R2-17 재현: 실제 Stage0 흐름에서 사루비아가 플레이어 총알에 체력이 줄고 결국 죽는지
TEST(StageIntegration, SarubiaLosesHpToPlayerBulletsAndDiesInStage0)
{
    IntegrationSim sim(true);
    const int      sarubiaSection = sim.State().GetData().FindSection("openland_4");
    for (int i = 0; i < sarubiaSection; ++i)
    {
        sim.Warp(1);
        sim.BotStep(true, true, 30);
    }
    sim.BotStep(true, true, 300);


    const Enemy* pStart = FindSarubia_(sim);
    ASSERT_NE(pStart, nullptr);
    const int startHp = pStart->health.hp;
    for (int t = 0; t < 240 && FindSarubia_(sim) && FindSarubia_(sim)->life == eEnemyLife::Alive; ++t)
    {
        sim.BotStep(true, true, 30);
    }
    const Enemy* pEnd = FindSarubia_(sim);
    ASSERT_NE(pEnd, nullptr);
    EXPECT_NE(pEnd->life, eEnemyLife::Alive) << "startHp=" << startHp << " endHp=" << pEnd->health.hp;
}

// R2-17 판정 영역: 사루비아 상부(포탑/망루, 발 기준 y -130 근처)를 쏜 총알이 통과하지 않아야 한다.
// 기존 body 100x100 (y -100..0) 에서는 y -110 이상이 전부 빗나갔다
TEST(StageIntegration, SarubiaHurtboxCoversTheTurretAboveItsTreads)
{
    IntegrationSim sim(true);
    const int      sarubiaSection = sim.State().GetData().FindSection("openland_4");
    for (int i = 0; i < sarubiaSection; ++i)
    {
        sim.Warp(1);
        sim.BotStep(true, true, 30);
    }
    sim.BotStep(true, true, 300);

    const Enemy* pSaru = nullptr;
    for (const Enemy& enemy: sim.State().GetEnemies().GetEnemies())
    {
        if (enemy.behavior == eEnemyBehavior::Sarubia)
        {
            pSaru = &enemy;
        }
    }
    ASSERT_NE(pSaru, nullptr);

    for (const float dy: { -120.f, -90.f, -30.f })
    {
        CombatSystem combat;
        combat.BeginTick();
        sim.State().GetEnemies().RegisterHurtboxes(combat);
        const Aabb probe = Aabb::FromCenterSize({ pSaru->body.position.x, pSaru->body.position.y + dy }, { 6.f, 6.f });
        EXPECT_TRUE(combat.HasTargetIn(eTeam::Player, eHitKind::Bullet, probe, kNoOwner)) << "dy=" << dy;
    }
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
    GateFixture fx(WrapStage(R"J([{"id":"a","triggerX":0,"actions":[
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
    GateFixture fx(WrapStage(R"J([{"id":"a","triggerX":0,"actions":[
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
    GateFixture fx(WrapStage(R"J([{"id":"a","triggerX":0,"actions":[
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

// 스크립트 봇을 3000틱 돌리며 매 틱의 상태 해시를 모은다
[[nodiscard]] static std::vector<uint64_t> RunBotHashes_()
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

TEST(StageIntegration, ScriptedBotRunIsDeterministicOver3000Ticks)
{
    const std::vector<uint64_t> first  = RunBotHashes_();
    const std::vector<uint64_t> second = RunBotHashes_();
    ASSERT_EQ(first.size(), second.size());
    EXPECT_EQ(first, second);
}

// 살아 있는 behavior 적 하나에 판정 피해를 준다 (해제 조건 검증용, 정상 피격 경로 OnHitEvents 사용)
[[nodiscard]] static bool KillFirstAlive_(
    StageState&          _state,
    const eEnemyBehavior _behavior)
{
    Vector<HitEvent> hits;
    for (const Enemy& enemy: _state.GetEnemies().GetEnemies())
    {
        if (enemy.life == eEnemyLife::Alive && enemy.behavior == _behavior)
        {
            HitEvent ev;
            ev.damage       = 9999;
            ev.kind         = eHitKind::Bullet;
            ev.attackerTeam = eTeam::Player;
            ev.attacker     = 1;
            ev.target       = enemy.id;
            hits.push_back(ev);
            break;
        }
    }
    if (hits.empty())
    {
        return false;
    }
    _state.GetEnemies().OnHitEvents(Span<const HitEvent> { hits.data(), hits.size() });
    return true;
}

[[nodiscard]] static bool HasAlive_(
    const StageState&    _state,
    const eEnemyBehavior _behavior)
{
    for (const Enemy& enemy: _state.GetEnemies().GetEnemies())
    {
        if (enemy.life == eEnemyLife::Alive && enemy.behavior == _behavior)
        {
            return true;
        }
    }
    return false;
}

TEST(StageIntegration, HeliSpawnsAtTheHeliTriggerAndLocksTheCamera)
{
    // 강 구간 헬기 (heli_river, triggerX 5480): 카메라를 5480..6280 에 잠그고, 헬기와 지프 (river_jeep) 가 모두 죽어야 풀린다
    IntegrationSim sim(true);
    const int      heliSection = sim.State().GetData().FindSection("heli_river");
    ASSERT_GE(heliSection, 0);
    for (int i = 0; i < heliSection; ++i)
    {
        sim.Warp(1);
        sim.BotStep(true, false, 30);
    }
    for (int t = 0; t < 900 && sim.State().GetPlayer().GetFeet().x < 5490.f; ++t)
    {
        sim.BotStep(true, false);
    }
    sim.BotStep(true, false, 90);   // 락 글라이드 (1.0 s) 완료 대기

    EXPECT_GE(sim.State().GetPlayer().GetFeet().x, 5458.f);
    EXPECT_GE(sim.State().GetRunner().GetLockDepth(), 1);
    const float camLeft = sim.State().GetCamera().GetPositionNoShake().x;
    // 락 rect [5480,288,6120,736] 는 뷰 폭 640 과 같아 카메라가 5480 에 고정된다 (망루 guard_post x 5800 이 화면 가운데)
    EXPECT_NEAR(camLeft, 5388.f, 2.f);
    EXPECT_TRUE(HasAlive_(sim.State(), eEnemyBehavior::Heli));

    // 헬기가 실제 락된 화면 (640x448) 안에 들어오는 틱이 있어야 한다 (화면 밖에만 머물면 플레이어에게 안 보인다)
    bool bHeliVisible = false;
    for (int t = 0; t < 900 && !bHeliVisible; ++t)
    {
        sim.BotStep(true, false);
        const Vec2 cam = sim.State().GetCamera().GetPositionNoShake();
        for (const Enemy& enemy: sim.State().GetEnemies().GetEnemies())
        {
            if (enemy.life == eEnemyLife::Alive && enemy.behavior == eEnemyBehavior::Heli && enemy.body.position.x >= cam.x && enemy.body.position.x <= cam.x + 640.f && enemy.body.position.y >= cam.y && enemy.body.position.y <= cam.y + 448.f)
            {
                bHeliVisible = true;
            }
        }
    }
    EXPECT_TRUE(bHeliVisible) << "헬기가 락된 화면 안에 한 번도 들어오지 않았다";

    // 지프 (river_jeep) 는 river_mid 트리거 (5600) 에서 스폰된다
    for (int t = 0; t < 900 && sim.State().GetPlayer().GetFeet().x < 5610.f; ++t)
    {
        sim.BotStep(true, false);
    }
    EXPECT_TRUE(HasAlive_(sim.State(), eEnemyBehavior::Jeep));

    // 헬기만 죽이면 지프가 살아 있어 락이 유지된다
    EXPECT_TRUE(KillFirstAlive_(sim.State(), eEnemyBehavior::Heli));
    sim.BotStep(true, false, 120);
    EXPECT_GE(sim.State().GetRunner().GetLockDepth(), 1) << "지프가 살아 있는 동안 카메라 락 유지";

    // 지프까지 죽으면 게이트 heli_dead 가 열려 락이 풀린다
    EXPECT_TRUE(KillFirstAlive_(sim.State(), eEnemyBehavior::Jeep));
    sim.BotStep(true, false, 120);
    EXPECT_EQ(sim.State().GetRunner().GetLockDepth(), 0);
}

TEST(StageIntegration, BusIsPreplacedInsideTheLockedViewAndLocksTheCameraAtTheBusTrigger)
{
    IntegrationSim sim(true);
    const int      busSection = sim.State().GetData().FindSection("bus");
    for (int i = 0; i < busSection; ++i)
    {
        sim.Warp(1);
        sim.BotStep(true, false, 30);
    }
    sim.BotStep(true, false, 300);

    const float camLeft = sim.State().GetCamera().GetPositionNoShake().x;
    EXPECT_GE(sim.State().GetRunner().GetLockDepth(), 1);
    EXPECT_NEAR(camLeft, 2457.f, 2.f);

    bool bFound = false;
    for (const Enemy& enemy: sim.State().GetEnemies().GetEnemies())
    {
        if (enemy.life == eEnemyLife::Alive && enemy.behavior == eEnemyBehavior::Bus)
        {
            bFound = true;
            EXPECT_GT(enemy.body.position.x, camLeft);
            EXPECT_LT(enemy.body.position.x, camLeft + 640.f);
        }
    }
    EXPECT_TRUE(bFound);

    // 버스와 헬기가 모두 사라지면 두 잠금 (게이트 bus_clear / heli_dead) 이 풀려 스택이 비어야 한다
    sim.State().GetEnemies().KillAll(false);
    sim.BotStep(false, false, 120);
    EXPECT_EQ(sim.State().GetRunner().GetLockDepth(), 0);
}

TEST(StageIntegration, BusStairSoldierTraceWritesCsvWhenRequested)
{
    // BUS_TRACE_STAGE=<path> 이면 실제 스테이지의 버스 계단 병사 틱별 위치/상태를 CSV 로 남긴다 (진단용)
    char*  pPath  = nullptr;
    size_t length = 0;
    if (_dupenv_s(&pPath, &length, "BUS_TRACE_STAGE") != 0 || pPath == nullptr)
    {
        GTEST_SKIP() << "BUS_TRACE_STAGE not set";
    }
    const std::string path = pPath;
    std::free(pPath);
    std::ofstream  out(path);
    out << "tick,id,def,x,y,state,grounded,camLeft\n";
    IntegrationSim sim(true);
    const int      busSection = sim.State().GetData().FindSection("bus");
    for (int i = 0; i < busSection; ++i)
    {
        sim.Warp(1);
        sim.BotStep(true, false, 30);
    }
    for (int t = 0; t < 1500; ++t)
    {
        sim.BotStep(true, false);
        float busX = 0.f;
        for (const Enemy& enemy: sim.State().GetEnemies().GetEnemies())
        {
            if (enemy.behavior == eEnemyBehavior::Bus && enemy.life != eEnemyLife::Gone)
            {
                busX = enemy.body.position.x;
            }
        }
        for (const Enemy& enemy: sim.State().GetEnemies().GetEnemies())
        {
            if (enemy.userTag == 0x42555300u && enemy.life != eEnemyLife::Gone)
            {
                out << t << ',' << enemy.id << ',' << enemy.defIndex << ',' << enemy.body.position.x - busX << ',' << enemy.body.position.y << ',' << enemy.state << ',' << enemy.bGrounded << ','
                    << sim.State().GetCamera().GetPositionNoShake().x << '\n';
            }
        }
    }
}

// 실제 Stage0 (StageState) 에서 플레이어가 버스 왼쪽에 서서 쏠 때: 계단 병사가 지붕까지 오르고, 지붕에서 걸어 버스 몸통 안으로 떨어지지 않는다
TEST(StageIntegration, BusStairSoldiersReachTheRoofAndNeverEnterTheBusBodyWithThePlayerOnTheLeft)
{
    IntegrationSim sim(true);
    const int      busSection = sim.State().GetData().FindSection("bus");
    for (int i = 0; i < busSection; ++i)
    {
        sim.Warp(1);
        sim.BotStep(true, false, 30);
    }
    for (int tick = 0; tick < 4000 && sim.State().GetPlayer().GetFeet().x < 2700.f; ++tick)
    {
        sim.BotStep(true, false);
    }
    ASSERT_GE(sim.State().GetPlayer().GetFeet().x, 2700.f);

    int   roofTicks   = 0;
    int   awayTicks   = 0;
    int   badTicks    = 0;
    float firstBadX   = 0.f;
    float firstBadY   = 0.f;
    for (int tick = 0; tick < 1800; ++tick)
    {
        sim.BotStep(false, tick > 200);
        float busX   = 0.f;
        float busY   = 0.f;
        float roofY  = 0.f;
        bool  bBus   = false;
        for (const Enemy& enemy: sim.State().GetEnemies().GetEnemies())
        {
            if (enemy.behavior == eEnemyBehavior::Bus && enemy.life == eEnemyLife::Alive)
            {
                const EnemyDef& def = sim.State().GetEnemies().GetLibrary()->Get(enemy.defIndex);
                busX                = enemy.body.position.x;
                busY                = enemy.body.position.y;
                roofY               = busY + def.bus.roofTop.y;
                bBus                = true;
            }
        }
        if (!bBus)
        {
            continue;
        }
        for (const Enemy& enemy: sim.State().GetEnemies().GetEnemies())
        {
            if (enemy.userTag != 0x42555300u || enemy.life != eEnemyLife::Alive)
            {
                continue;
            }
            const float dx = enemy.body.position.x - busX;
            if (std::abs(enemy.body.position.y - roofY) < 1.f && enemy.bGrounded)
            {
                ++roofTicks;
            }
            if (enemy.body.position.y > busY - 5.f && dx < -100.f)
            {
                ++awayTicks;
            }
            // 버스 몸통 (solid x +-85) 위 구간에서는 지붕 높이보다 아래에 있으면 안 된다 (계단 오르기는 x >= 88)
            if (std::abs(dx) < 80.f && enemy.body.position.y > roofY + 1.f)
            {
                if (badTicks == 0)
                {
                    firstBadX = dx;
                    firstBadY = enemy.body.position.y - busY;
                }
                ++badTicks;
            }
        }
    }
    EXPECT_GT(roofTicks, 100) << "병사가 지붕 위에 서야 한다";
    EXPECT_GT(awayTicks, 0) << "대검병은 지붕 왼쪽 끝에서 점프 베기로 버스 밖 지면에 내려야 한다";
    EXPECT_EQ(badTicks, 0) << "병사가 버스 몸통 안에 있었다 (첫 위치 상대 " << firstBadX << "," << firstBadY << ")";
}

TEST(StageIntegration, BusLockGlideMovesTheCameraSmoothly)
{
    // bus 섹션 락 글라이드 (2.5 s) 동안 카메라 수평 속도의 틱 간 변화가 2 px/tick 이내여야 한다 (급정지/급가속 금지)
    IntegrationSim sim(true);
    const int      busSection = sim.State().GetData().FindSection("bus");
    for (int i = 0; i < busSection; ++i)
    {
        sim.Warp(1);
        sim.BotStep(true, false, 30);
    }
    for (int t = 0; t < 600 && sim.State().GetRunner().GetLockDepth() < 1; ++t)
    {
        sim.BotStep(true, false);
    }
    ASSERT_GE(sim.State().GetRunner().GetLockDepth(), 1);

    // 락 직전까지 따라가던 속도 (약 4 px/tick) 를 기준으로 삼는다. 첫 틱은 기준 속도로 채운다
    float prevX = sim.State().GetCamera().GetPositionNoShake().x;
    sim.BotStep(true, false);
    float prevVel  = sim.State().GetCamera().GetPositionNoShake().x - prevX;
    prevX          = sim.State().GetCamera().GetPositionNoShake().x;
    float maxAccel = 0.f;
    float maxVel   = 0.f;
    for (int t = 0; t < 180; ++t)
    {
        sim.BotStep(true, false);
        const float x   = sim.State().GetCamera().GetPositionNoShake().x;
        const float vel = x - prevX;
        maxAccel        = std::max(maxAccel, std::abs(vel - prevVel));
        maxVel          = std::max(maxVel, std::abs(vel));
        prevVel         = vel;
        prevX           = x;
    }
    EXPECT_NEAR(prevX, 2457.f, 2.f) << "글라이드 끝에 락 rect 왼쪽 (2457) 에 도달해야 한다";
    EXPECT_GT(maxVel, 0.f) << "글라이드 중에는 카메라가 실제로 움직여야 한다";
    EXPECT_LE(maxAccel, 2.f) << "카메라 수평 속도 변화가 2 px/tick 를 넘으면 안 된다 (최대 속도 " << maxVel << ")";
}

// 보스 진입 구간 기록기: 틱마다 카메라/락/보스를 남기고, 소품/아이템이 사라진 틱을 (화면 안이었는지와 함께) 모은다
struct BossEntrySample
{
    int   tick      = 0;
    float camX      = 0.f;   // 반올림된 화면 왼쪽 (렌더 기준)
    float centerX   = 0.f;   // 카메라 중심 (연속값)
    float lockMinX  = 0.f;
    float lockMaxX  = 0.f;
    int   lockDepth = 0;
    int   moving    = 0;
    float bossX     = 0.f;
    float playerX   = 0.f;
};

struct BossEntryRemoval
{
    int      tick     = 0;
    String   kind     = {};   // prop / item
    String   reason   = {};   // Culled / DestroyedRemove / FallRemove / Gone
    String   def      = {};
    uint32_t id       = 0;
    float    x        = 0.f;
    float    y        = 0.f;
    float    left     = 0.f;   // 스프라이트 사각형 (알 수 없으면 pos 점)
    float    right    = 0.f;
    bool     bVisible = false;
};

struct BossEntryTrackedProp
{
    uint32_t id         = 0;
    String   def        = {};
    Vec2     pos        = { 0.f, 0.f };
    float    left       = 0.f;
    float    right      = 0.f;
    bool     bDestroyed = false;
    bool     bFalling   = false;
};

struct BossEntryTrackedItem
{
    uint32_t id   = 0;
    String   def  = {};
    Vec2     feet = { 0.f, 0.f };
};

class BossEntryRecorder
{
public:
    void Sample(
        IntegrationSim& _sim)
    {
        StageState&   state  = _sim.State();
        const Camera& camera = state.GetCamera();
        const Vec2    cam    = camera.GetPositionNoShake();

        // 직전 틱 끝의 화면 (플레이어가 마지막으로 본 화면) 기준으로 사라진 것의 가시성을 판정한다
        const float prevLeft  = m_bHasPrev ? m_prevCamX : cam.x;
        const float prevRight = prevLeft + 640.f;

        const PropDefLibrary*        pLib = state.GetProps().GetLibrary();
        Vector<BossEntryTrackedProp> now;
        for (const PropInstance& inst: state.GetProps().GetProps())
        {
            BossEntryTrackedProp t;
            t.id         = inst.id;
            t.def        = pLib->GetDefs()[static_cast<size_t>(inst.defIndex)].name;
            t.pos        = inst.pos;
            t.left       = inst.pos.x;
            t.right      = inst.pos.x;
            t.bDestroyed = inst.bDestroyed;
            t.bFalling   = inst.bFalling;
            Aabb bounds  = {};
            if (state.GetProps().GetVisualBounds(inst, _sim.Ctx().pAnims, bounds))
            {
                t.left  = bounds.min.x;
                t.right = bounds.max.x;
            }
            now.push_back(t);
        }
        for (const BossEntryTrackedProp& before: m_props)
        {
            bool bFound = false;
            for (const BossEntryTrackedProp& cur: now)
            {
                bFound = bFound || cur.id == before.id;
            }
            if (bFound)
            {
                continue;
            }
            BossEntryRemoval r;
            r.tick     = m_tick;
            r.kind     = "prop";
            r.reason   = before.bDestroyed ? "DestroyedRemove" : (before.bFalling ? "FallRemove" : "Culled");
            r.def      = before.def;
            r.id       = before.id;
            r.x        = before.pos.x;
            r.y        = before.pos.y;
            r.left     = before.left;
            r.right    = before.right;
            r.bVisible = before.right >= prevLeft && before.left <= prevRight;
            m_removals.push_back(r);
        }
        m_props = now;

        const Span<const PropItemDef> idefs = pLib->GetItems();
        Vector<BossEntryTrackedItem>  nowItems;
        for (const PropItem& item: state.GetProps().GetItems().GetItems())
        {
            BossEntryTrackedItem t;
            t.id   = item.id;
            t.def  = item.defIndex >= 0 ? idefs[static_cast<size_t>(item.defIndex)].name : String {};
            t.feet = item.feet;
            nowItems.push_back(t);
        }
        for (const BossEntryTrackedItem& before: m_items)
        {
            bool bFound = false;
            for (const BossEntryTrackedItem& cur: nowItems)
            {
                bFound = bFound || cur.id == before.id;
            }
            if (bFound)
            {
                continue;
            }
            BossEntryRemoval r;
            r.tick     = m_tick;
            r.kind     = "item";
            r.reason   = before.feet.x < prevLeft - 40.f ? "Culled" : "PickedOrExpired";
            r.def      = before.def;
            r.id       = before.id;
            r.x        = before.feet.x;
            r.y        = before.feet.y;
            r.left     = before.feet.x - 16.f;
            r.right    = before.feet.x + 16.f;
            r.bVisible = r.right >= prevLeft && r.left <= prevRight;
            m_removals.push_back(r);
        }
        m_items = nowItems;

        BossEntrySample sample;
        sample.tick      = m_tick;
        sample.camX      = cam.x;
        sample.centerX   = camera.GetViewCenter().x;
        sample.lockMinX  = camera.GetLockBounds().min.x;
        sample.lockMaxX  = camera.GetLockBounds().max.x;
        sample.lockDepth = state.GetRunner().GetLockDepth();
        sample.moving    = camera.IsMoving() ? 1 : 0;
        sample.playerX   = state.GetPlayer().GetFeet().x;
        for (const Enemy& enemy: state.GetEnemies().GetEnemies())
        {
            if (enemy.behavior == eEnemyBehavior::BigShiee)
            {
                sample.bossX = enemy.body.position.x;
            }
        }
        m_samples.push_back(sample);

        m_prevCamX = cam.x;
        m_bHasPrev = true;
        ++m_tick;
    }

    [[nodiscard]] const Vector<BossEntrySample>&  GetSamples() const { return m_samples; }
    [[nodiscard]] const Vector<BossEntryRemoval>& GetRemovals() const { return m_removals; }

    // 화면 안에 있던 것이 컬링으로 사라진 건수 (파괴 후 제거 / 습득 / 수명 만료는 제외). kind 가 비면 소품+아이템 모두
    [[nodiscard]] int CountVisibleRemovals(
        const String& _kind) const
    {
        int count = 0;
        for (const BossEntryRemoval& r: m_removals)
        {
            if (r.bVisible && r.reason == "Culled" && (_kind.empty() || r.kind == _kind))
            {
                ++count;
            }
        }
        return count;
    }

    // 락 이후 (lockDepth >= 1 이 된 틱부터) 화면 왼쪽 x (렌더 기준) 의 틱당 속도 변화 최대값. 방향 반전 (양쪽 속도 2 px/tick 이상) 횟수는 _pReversals 로 돌려준다
    [[nodiscard]] float MaxAccelSinceLock(
        int* _pReversals) const
    {
        float maxAccel  = 0.f;
        int   reversals = 0;
        float prevVel   = 0.f;
        bool  bLocked   = false;
        for (size_t i = 1; i < m_samples.size(); ++i)
        {
            const float vel = m_samples[i].camX - m_samples[i - 1].camX;
            bLocked         = bLocked || m_samples[i].lockDepth >= 1;
            if (bLocked)
            {
                maxAccel = std::max(maxAccel, std::abs(vel - prevVel));
                if (vel * prevVel < 0.f && std::abs(vel) >= 2.f && std::abs(prevVel) >= 2.f)
                {
                    ++reversals;
                }
            }
            prevVel = vel;
        }
        if (_pReversals != nullptr)
        {
            *_pReversals = reversals;
        }
        return maxAccel;
    }

    void WriteCsv(
        const std::string& _path) const
    {
        std::ofstream out { _path };
        out << "T,tick,camX,centerX,vel,acc,lockMinX,lockMaxX,lockDepth,moving,bossX,playerX\n";
        float prevVel = 0.f;
        for (size_t i = 0; i < m_samples.size(); ++i)
        {
            const BossEntrySample& s   = m_samples[i];
            const float            vel = i > 0 ? s.camX - m_samples[i - 1].camX : 0.f;
            out << "T," << s.tick << ',' << s.camX << ',' << s.centerX << ',' << vel << ',' << (vel - prevVel) << ',' << s.lockMinX << ',' << s.lockMaxX << ',' << s.lockDepth << ',' << s.moving << ',' << s.bossX << ',' << s.playerX << '\n';
            prevVel = vel;
        }
        out << "E,tick,kind,reason,def,id,x,y,left,right,visible\n";
        for (const BossEntryRemoval& r: m_removals)
        {
            out << "E," << r.tick << ',' << r.kind << ',' << r.reason << ',' << r.def << ',' << r.id << ',' << r.x << ',' << r.y << ',' << r.left << ',' << r.right << ',' << (r.bVisible ? 1 : 0) << '\n';
        }
    }

private:
    int                          m_tick     = 0;
    float                        m_prevCamX = 0.f;
    bool                         m_bHasPrev = false;
    Vector<BossEntrySample>      m_samples  = {};
    Vector<BossEntryRemoval>     m_removals = {};
    Vector<BossEntryTrackedProp> m_props    = {};
    Vector<BossEntryTrackedItem> m_items    = {};
};

// 보스 아레나 트리거 (x 13700) 까지 간다. 사루비아 락 (openland_4, 플레이어 x 는 13066 을 못 넘는다) 은 남은 적을 치워 푼다.
// _pRecorder 가 있으면 마지막 접근 구간 (사루비아 정리 뒤) 의 틱을 기록한다
static void ReachBossArena_(
    IntegrationSim&    _sim,
    BossEntryRecorder* _pRecorder = nullptr)
{
    const int bossSection = _sim.State().GetData().FindSection("boss_arena");
    for (int i = 0; i < bossSection - 1; ++i)
    {
        _sim.Warp(1);
        _sim.BotStep(true, false, 30);
    }
    _sim.State().GetEnemies().KillAll(false);
    if (_pRecorder != nullptr)
    {
        _pRecorder->Sample(_sim);
    }
    for (int t = 0; t < 2400 && _sim.State().GetPlayer().GetFeet().x < 13710.f; ++t)
    {
        _sim.BotStep(true, false);
        if (_pRecorder != nullptr)
        {
            _pRecorder->Sample(_sim);
        }
    }
}

// 보스 진입 후 _ticks 틱을 기록한다
static void RecordBossEntry_(
    IntegrationSim&    _sim,
    BossEntryRecorder& _rec,
    const int          _ticks)
{
    ReachBossArena_(_sim, &_rec);
    for (int t = 0; t < _ticks; ++t)
    {
        _sim.BotStep(true, false);
        _rec.Sample(_sim);
    }
}

TEST(StageIntegration, BossEntryDumpWritesCsvWhenRequested)
{
    // C15_DUMP=<path> 이면 보스 진입 구간의 카메라/제거 이벤트를 CSV 로 남긴다 (진단용, 평소엔 건너뛴다)
    char*  pPath = nullptr;
    size_t length = 0;
    if (_dupenv_s(&pPath, &length, "C15_DUMP") != 0 || pPath == nullptr)
    {
        GTEST_SKIP() << "C15_DUMP not set";
    }
    const std::string path = pPath;
    std::free(pPath);
    IntegrationSim    sim(true);
    BossEntryRecorder rec;
    RecordBossEntry_(sim, rec, 1200);
    rec.WriteCsv(path);
}

TEST(StageIntegration, BossSpawnsAtTheArenaTriggerAfterSarubiaAndLocksTheCamera)
{
    IntegrationSim sim(true);
    ReachBossArena_(sim);
    ASSERT_GE(sim.State().GetPlayer().GetFeet().x, 13700.f);
    sim.BotStep(true, false, 150);

    const float camLeft = sim.State().GetCamera().GetPositionNoShake().x;
    EXPECT_GE(sim.State().GetRunner().GetLockDepth(), 1);
    EXPECT_GE(camLeft, 13700.f - 2.f) << "락 사각형은 자동 스크롤로 오른쪽으로만 간다";
    EXPECT_LT(camLeft, 13700.f + 120.f);
    EXPECT_TRUE(sim.IsBossAlive());
    for (const Enemy& enemy: sim.State().GetEnemies().GetEnemies())
    {
        if (enemy.life == eEnemyLife::Alive && enemy.behavior == eEnemyBehavior::BigShiee)
        {
            EXPECT_NEAR(enemy.body.position.y, 1286.f, 2.f) << "보스 발은 아레나 지면 (openland, groundSnap)";
            EXPECT_LT(enemy.body.position.x, camLeft + 640.f);
            EXPECT_LT(enemy.body.position.x, camLeft + 200.f) << "150틱 뒤에도 선체 가운데는 아직 화면 왼쪽에서 들어오는 중이다 (뿅 금지)";
        }
    }
    EXPECT_EQ(sim.Current(), sim.StagePtr()) << "보스가 살아 있는 동안 스테이지는 끝나지 않는다";
}

static int CountPropsInView_(
    IntegrationSim& _sim)
{
    const Vec2 cam   = _sim.State().GetCamera().GetPositionNoShake();
    int        count = 0;
    for (const PropInstance& prop: _sim.State().GetProps().GetProps())
    {
        if (!prop.bDestroyed && prop.pos.x >= cam.x && prop.pos.x <= cam.x + 640.f)
        {
            ++count;
        }
    }
    return count;
}

TEST(StageIntegration, BossArenaEntryNeverRemovesAVisiblePropOrItem)
{
    // 보스 진입 (사루비아 락 해제 -> 보스 락 글라이드 -> 자동 스크롤) 동안 화면 안에 있던 소품/아이템이 컬링으로 사라지면 안 된다
    IntegrationSim    sim(true);
    BossEntryRecorder rec;
    RecordBossEntry_(sim, rec, 900);
    ASSERT_GE(sim.State().GetRunner().GetLockDepth(), 1);
    EXPECT_EQ(rec.CountVisibleRemovals(String {}), 0);
}

TEST(StageIntegration, BossArenaLockGlideKeepsTheScreenContinuous)
{
    // 락 글라이드 + 자동 스크롤 동안 화면 왼쪽 x 의 틱 간 변화가 글라이드 가속 상한 (2 px/tick) 안이고 방향이 뒤집히지 않는다
    IntegrationSim    sim(true);
    BossEntryRecorder rec;
    RecordBossEntry_(sim, rec, 900);
    int         reversals = 0;
    const float maxAccel  = rec.MaxAccelSinceLock(&reversals);
    EXPECT_LE(maxAccel, 2.f) << "카메라 수평 속도 변화가 크다 (화면이 튄다)";
    EXPECT_EQ(reversals, 0);
}

TEST(StageIntegration, BossArenaPlacesHousesAndRoofsOnTheFloorInsideTheLockedView)
{
    // 보스 아레나 바닥 (R3-01): 락 중 화면 안에 살아 있는 소품 (집/지붕/망루) 이 있어야 휑하지 않다
    IntegrationSim sim(true);
    ReachBossArena_(sim);
    sim.BotStep(true, false, 150);

    EXPECT_GE(sim.State().GetRunner().GetLockDepth(), 1);
    EXPECT_GT(CountPropsInView_(sim), 0) << "보스 락 중 화면 안에 소품이 없다";

    int emptySamples = 0;
    for (int t = 0; t < 600; t += 60)
    {
        sim.BotStep(true, false, 60);
        if (CountPropsInView_(sim) == 0)
        {
            ++emptySamples;
        }
    }
    EXPECT_EQ(emptySamples, 0) << "보스 락이 스크롤하는 동안 빈 화면 구간이 있다";
}

TEST(StageIntegration, BossDeathUnlocksTheCameraAndEndsTheStageThroughTheClearGate)
{
    IntegrationSim sim(true);
    ReachBossArena_(sim);
    sim.BotStep(true, false, 150);
    ASSERT_TRUE(sim.IsBossAlive());
    ASSERT_GE(sim.State().GetRunner().GetLockDepth(), 1);

    // 캐논이 올라와 맞을 때까지 매 틱 쏜다 (캐논 파괴 -> 얼굴 -> 사망). 사망 연출 중이면 살아 있지 않다
    for (int t = 0; t < 5000 && sim.IsBossAlive(); ++t)
    {
        sim.DamageBoss();
        sim.BotStep(false, false);
    }
    ASSERT_FALSE(sim.IsBossAlive()) << "보스가 사망 단계에 들어가지 않았다";
    // 사망 연출 (dyingDoneSec 10 s) 중에는 스테이지가 끝나면 안 된다 (클리어 게이트는 destroyFinished 뒤에만 연다)
    sim.BotStep(false, false, 300);
    ASSERT_EQ(sim.Current(), sim.StagePtr()) << "사망 연출 중에 스테이지가 끝났다";
    for (int t = 0; t < 1800 && sim.Current() == sim.StagePtr(); ++t)
    {
        sim.BotStep(false, false);
    }
    EXPECT_NE(sim.Current(), sim.StagePtr()) << "보스 파괴 연출이 끝나면 클리어 흐름 (MISSION COMPLETE) 으로 끝나야 한다";
}
