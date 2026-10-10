#include "Core/AssetPath.h"
#include <cmath>
#include "Base/Types.h"
#include "Base/Math.h"
#include "Base/Random.h"

#include <gtest/gtest.h>

#include "Core/Input.h"
#include "Game/SimRunner.h"
#include "Game/Stage/StageState.h"
#include "Game/Stage/StageWorldHost.h"

namespace
{
using namespace slug;

class StageSim
{
public:
    explicit StageSim(
        const bool _bDebugDraw = false)
        : m_rng(uint64_t { 1 })
    {
        m_ctx.pRng       = &m_rng;
        m_ctx.assetRoot  = slug::FindAssetRoot();
        m_ctx.bDebugDraw = _bDebugDraw;
        m_pState         = new StageState;
        m_runner         = std::unique_ptr<SimRunner> { new SimRunner { m_ctx, std::unique_ptr<IGameState> { m_pState } } };
    }

    [[nodiscard]] StageState& State() { return *m_pState; }
    [[nodiscard]] GameContext& Ctx() { return m_ctx; }

    void Hold(const eAction _action)
    {
        m_frame.Set(_action, true);
    }

    void Release(const eAction _action)
    {
        m_frame.Set(_action, false);
    }

    void Step(const int _count = 1)
    {
        for (int i = 0; i < _count; ++i)
        {
            m_runner->Step(m_frame);
        }
    }

private:
    slug::RngMt19937 m_rng;
    GameContext     m_ctx = {};
    InputFrame      m_frame = {};
    StageState*     m_pState = nullptr;
    std::unique_ptr<SimRunner> m_runner = nullptr;
};

SpawnRequest MakeRequest(
    const char*          _def,
    const eEntityKind    _kind,
    const Vec2   _position)
{
    SpawnRequest request;
    request.def      = _def;
    request.kind     = _kind;
    request.position = _position;
    return request;
}

HitEvent MakeHit(
    const SpawnId _target,
    const int     _damage)
{
    HitEvent event;
    event.target = _target;
    event.damage = _damage;
    return event;
}
}

TEST(StageState, LoadsStage0AndStartsAtTheScriptedPlayerStart)
{
    StageSim sim;
    sim.Step();

    StageState& state = sim.State();
    ASSERT_TRUE(state.IsLoaded());
    EXPECT_FALSE(state.IsFinished());
    EXPECT_EQ(state.GetData().name, "Stage0");
    EXPECT_EQ(state.GetData().sections.size(), 20u);
    EXPECT_NEAR(state.GetPlayer().GetFeet().x, 100.f, 2.f);   // 스크립트 player.x
    EXPECT_GT(state.GetRunner().GetSpawnedCount(), 0);        // intro 의 소품들
    EXPECT_NE(state.GetHost().GetActiveGround(), nullptr);
}

TEST(StageState, PlayerLandsOnTheGroundAfterTheStart)
{
    StageSim sim;
    for (int i = 0; i < 60; ++i)
    {
        sim.Step();
    }
    EXPECT_TRUE(sim.State().GetPlayer().IsGrounded());
    EXPECT_GT(sim.State().GetPlayer().GetFeet().y, 300.f);
}

TEST(StageState, HoldingRightAdvancesThroughSectionsAndTheCameraNeverScrollsBack)
{
    StageSim sim;
    sim.Hold(eAction::Right);
    float lastCamX = -1.f;
    for (int i = 0; i < 900; ++i)
    {
        sim.State().GetPlayer().ExtendInvincibility(600);
        if (i % 24 < 3)
        {
            sim.Hold(eAction::Shot);
        }
        else
        {
            sim.Release(eAction::Shot);
        }
        sim.Step();
        const float camX = sim.State().GetCamera().GetPositionNoShake().x;
        ASSERT_GE(camX, lastCamX - 1e-3f) << "tick " << i;
        lastCamX = camX;
    }
    EXPECT_GT(sim.State().GetPlayer().GetFeet().x, 750.f);
    EXPECT_GE(sim.State().GetRunner().GetNextSectionIndex(), 2);   // intro, street 발동
}

TEST(StageStateDebug, F5WarpsToTheNextSectionOnlyWithDebugDraw)
{
    StageSim sim { true };
    sim.Step();
    ASSERT_EQ(sim.State().GetRunner().GetNextSectionIndex(), 1);

    sim.Ctx().debugWarp = 1;          // F5
    sim.Step();
    EXPECT_EQ(sim.Ctx().debugWarp, 0);   // 소비됨
    EXPECT_GT(sim.State().GetPlayer().GetFeet().x, 750.f);
    EXPECT_EQ(sim.State().GetRunner().GetNextSectionIndex(), 2);
    EXPECT_EQ(sim.State().GetHost().GetLastSectionId(), "street");
}

TEST(StageStateDebug, WarpRequestIsIgnoredWithoutDebugDraw)
{
    StageSim sim { false };
    sim.Step();
    sim.Ctx().debugWarp = 1;
    sim.Step();
    EXPECT_LT(sim.State().GetPlayer().GetFeet().x, 750.f);
    EXPECT_EQ(sim.State().GetRunner().GetNextSectionIndex(), 1);
}

TEST(StageStateDebug, F6WarpsToTheStageEnd)
{
    StageSim sim { true };
    sim.Step();
    sim.Ctx().debugWarp = 2;          // F6
    sim.Step();
    EXPECT_GT(sim.State().GetPlayer().GetFeet().x, 12900.f);   // stage_end 섹션 트리거
    EXPECT_TRUE(sim.State().GetRunner().GetNextSectionIndex() >= 20);
}

TEST(StageHostPlaceholders, HitsKillAtZeroHpThenTheEntityIsRemovedAfterTheDeathWindow)
{
    StageWorldHost host;
    SpawnRequest   request = MakeRequest("soldier_bomb", eEntityKind::Enemy, { 500.f, 378.f });
    request.bHasHp         = true;
    request.hp             = 2;
    const SpawnId id       = host.SpawnEntity(request);
    const SpawnId target   = host.GetPlaceholderEntities()[0].id;   // 피격은 플레이스홀더 자신의 id 로 들어온다

    EXPECT_NE(id, kInvalidSpawnId);
    EXPECT_FALSE(host.IsDefeated(id));

    const slug::Vector<HitEvent> events = { MakeHit(target, 1) };
    EXPECT_EQ(host.ApplyPlaceholderEvents(events), 0);
    EXPECT_FALSE(host.IsDefeated(id));

    const slug::Vector<HitEvent> killing = { MakeHit(target, 5) };
    EXPECT_EQ(host.ApplyPlaceholderEvents(killing), 1);
    EXPECT_TRUE(host.IsDefeated(id));
    EXPECT_FALSE(host.IsDestroyFinished(id));

    EXPECT_EQ(host.ApplyPlaceholderEvents(killing), 0);

    for (int i = 0; i < kPlaceholderDeathTicks; ++i)
    {
        host.TickPlaceholders();
    }
    EXPECT_TRUE(host.IsDestroyFinished(id));
    EXPECT_TRUE(host.GetPlaceholderEntities().empty());
}

TEST(StageHostPlaceholders, NonHittableKindsAreNeverDefeatedUntilCleared)
{
    StageWorldHost host;
    const SpawnId  decor = host.SpawnEntity(MakeRequest("anim_decor", eEntityKind::Decor, { 10.f, 10.f }));
    const SpawnId  item  = host.SpawnEntity(MakeRequest("random_score", eEntityKind::Item, { 20.f, 20.f }));
    const SpawnId  decorTarget = host.GetPlaceholderEntities()[0].id;
    const SpawnId  itemTarget  = host.GetPlaceholderEntities()[1].id;

    const slug::Vector<HitEvent> events = { MakeHit(decorTarget, 99), MakeHit(itemTarget, 99) };
    EXPECT_EQ(host.ApplyPlaceholderEvents(events), 0);
    EXPECT_FALSE(host.IsDefeated(decor));
    EXPECT_FALSE(host.IsDefeated(item));

    EntityFilter filter;
    filter.layerMask = LayerBit(eStageLayer::Item);
    host.ClearEntities(filter);
    EXPECT_TRUE(host.IsDefeated(item));
    EXPECT_FALSE(host.IsDefeated(decor));
}

TEST(StageHostPlaceholders, LegacyCenterPositionsPutTheFeetAtTheBottomOfTheBox)
{
    StageWorldHost host;
    SpawnRequest   request = MakeRequest("crate", eEntityKind::Prop, { 300.f, 100.f });
    request.positionMode   = ePositionMode::LegacyCenter;
    host.SpawnEntity(request);

    ASSERT_EQ(host.GetPlaceholderEntities().size(), 1u);
    const PlaceholderEntity& entity = host.GetPlaceholderEntities()[0];
    EXPECT_FLOAT_EQ(entity.feet.x, 300.f);
    EXPECT_FLOAT_EQ(entity.feet.y, 100.f + entity.size.y * 0.5f);
    EXPECT_TRUE(entity.bHittable);
}

TEST(StageHostPlaceholders, GroundLookupFollowsTheActiveMap)
{
    StageWorldHost host;
    GroundMap      low = GroundMap::MakeEmpty(200, 100);
    low.FillRect(0, 60, 199, 99, true);
    GroundMap high = GroundMap::MakeEmpty(200, 100);
    high.FillRect(0, 30, 199, 99, true);
    host.AddGroundMap("low", std::move(low));
    host.AddGroundMap("high", std::move(high));

    host.SetGround("low");
    ASSERT_TRUE(host.GetGroundFindY(50.f, 0.f).has_value());
    EXPECT_FLOAT_EQ(*host.GetGroundFindY(50.f, 0.f), 60.f);

    host.SetGround("high");
    EXPECT_FLOAT_EQ(*host.GetGroundFindY(50.f, 0.f), 30.f);
    EXPECT_EQ(host.GetActiveGroundName(), "high");
}
