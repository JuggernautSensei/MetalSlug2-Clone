// StageState 테스트: 스테이지 상태 전환, 호스트 자리표시자, 디버그 옵션 검증
#include <cmath>

#include <gtest/gtest.h>

#include "Base/Math.h"
#include "Base/Types.h"
#include "Core/AssetPath.h"
#include "Core/Input.h"
#include "Game/SimRunner.h"
#include "Game/Stage/StageState.h"
#include "Game/Stage/StageWorldHost.h"

class StageSim
{
public:
    explicit StageSim(
        const bool _bDebugDraw = false)
        : m_rng(uint64_t { 1 })
    {
        m_ctx.pRng       = &m_rng;
        m_ctx.assetRoot  = FindAssetRoot();
        m_ctx.bDebugDraw = _bDebugDraw;
        m_pState         = new StageState;
        m_runner         = Scoped<SimRunner> {
            new SimRunner { m_ctx, Scoped<IGameState> { m_pState } }
        };
    }

    [[nodiscard]] StageState&  State() { return *m_pState; }
    [[nodiscard]] GameContext& Ctx() { return m_ctx; }

    void Hold(
        const eAction _action)
    {
        m_frame.Set(_action, true);
    }

    void Release(
        const eAction _action)
    {
        m_frame.Set(_action, false);
    }

    void Step(
        const int _count = 1)
    {
        for (int i = 0; i < _count; ++i)
        {
            m_runner->Step(m_frame);
        }
    }

private:
    RngMt19937        m_rng    = RngMt19937 { std::mt19937_64::default_seed };
    GameContext       m_ctx    = {};
    InputFrame        m_frame  = {};
    StageState*       m_pState = nullptr;
    Scoped<SimRunner> m_runner = nullptr;
};

namespace
{
[[nodiscard]] SpawnRequest MakeRequest_(
    const char*       _def,
    const eEntityKind _kind,
    const Vec2        _position)
{
    SpawnRequest request;
    request.def      = _def;
    request.kind     = _kind;
    request.position = _position;
    return request;
}

[[nodiscard]] HitEvent MakeHit_(
    const SpawnId _target,
    const int     _damage)
{
    HitEvent event;
    event.target = _target;
    event.damage = _damage;
    return event;
}
}   // namespace

TEST(StageState, LoadsStage0AndStartsAtTheScriptedPlayerStart)
{
    StageSim sim;
    sim.Step();

    StageState& state = sim.State();
    ASSERT_TRUE(state.IsLoaded());
    EXPECT_FALSE(state.IsFinished());
    EXPECT_EQ(state.GetData().name, "Stage0");
    EXPECT_EQ(state.GetData().sections.size(), 22u);
    EXPECT_NEAR(state.GetPlayer().GetFeet().x, 100.f, 2.f);   // 스크립트 player.x
    EXPECT_GT(state.GetRunner().GetSpawnedCount(), 0);        // intro 의 소품들
    EXPECT_NE(state.GetHost().GetActiveGround(), nullptr);
}

TEST(StageState, PlayerLandsOnTheGroundAfterTheStart)
{
    // 시작 낙하산: 입력 없이 두면 낙하 (260px / 110px/s, 약 140틱) 후 착지하고 낙하산이 사라진다
    StageSim sim;
    EXPECT_EQ(sim.State().GetPlayer().GetChutePhase(), eChutePhase::Descend);
    for (int i = 0; i < 400; ++i)
    {
        sim.Step();
    }
    EXPECT_EQ(sim.State().GetPlayer().GetChutePhase(), eChutePhase::None);
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
        const float camX    = sim.State().GetCamera().GetPositionNoShake().x;
        const bool  bLocked = sim.State().GetRunner().GetLockDepth() > 0;
        // 카메라 락 (bus 등) 은 글라이드로 락 rect 쪽으로 옮겨 가므로 (rect 왼쪽이 현재 카메라보다 뒤일 수 있다) 잠긴 동안은 역행을 허용한다.
        // 락이 없는 구간에서는 카메라가 절대 뒤로 가지 않는다.
        if (!bLocked)
        {
            ASSERT_GE(camX, lastCamX - 1e-3f) << "tick " << i << " feet.x " << sim.State().GetPlayer().GetFeet().x;
        }
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

    sim.Ctx().debugWarp = 1;   // F5
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
    sim.Ctx().debugWarp = 2;   // F6
    sim.Step();
    EXPECT_GT(sim.State().GetPlayer().GetFeet().x, 13700.f);   // boss_arena 섹션 트리거 (마지막 구간)
    EXPECT_TRUE(sim.State().GetRunner().GetNextSectionIndex() >= 20);
}

TEST(StageHostPlaceholders, HitsKillAtZeroHpThenTheEntityIsRemovedAfterTheDeathWindow)
{
    StageWorldHost host;
    SpawnRequest   request = MakeRequest_("soldier_bomb", eEntityKind::Enemy, { 500.f, 378.f });
    request.bHasHp         = true;
    request.hp             = 2;
    const SpawnId id       = host.SpawnEntity(request);
    const SpawnId target   = host.GetPlaceholderEntities()[0].id;   // 피격은 플레이스홀더 자신의 id 로 들어온다

    EXPECT_NE(id, kInvalidSpawnId);
    EXPECT_FALSE(host.IsDefeated(id));

    const Vector<HitEvent> events = { MakeHit_(target, 1) };
    EXPECT_EQ(host.ApplyPlaceholderEvents(events), 0);
    EXPECT_FALSE(host.IsDefeated(id));

    const Vector<HitEvent> killing = { MakeHit_(target, 5) };
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
    const SpawnId  decor       = host.SpawnEntity(MakeRequest_("anim_decor", eEntityKind::Decor, { 10.f, 10.f }));
    const SpawnId  item        = host.SpawnEntity(MakeRequest_("random_score", eEntityKind::Item, { 20.f, 20.f }));
    const SpawnId  decorTarget = host.GetPlaceholderEntities()[0].id;
    const SpawnId  itemTarget  = host.GetPlaceholderEntities()[1].id;

    const Vector<HitEvent> events = { MakeHit_(decorTarget, 99), MakeHit_(itemTarget, 99) };
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
    SpawnRequest   request = MakeRequest_("crate", eEntityKind::Prop, { 300.f, 100.f });
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
