#include <cmath>

#include <gtest/gtest.h>

#include "Core/Input.h"
#include "Game/SimRunner.h"
#include "Game/Stage/StageState.h"
#include "Game/Stage/StagePlaceholders.h"

// ===========================================
//  stage0 상태 (헤드리스). Assets/Stage/Stage0.json + Baked 지면을 실제로 로드해 SimRunner 로 돌린다.
//   Baked 가 없으면 StageState 가 평지로 대체하므로 지면 y 검사는 느슨하게 둔다.
// ===========================================

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
        m_ctx.assetRoot  = jug::FilePath { SLUG_ASSET_DIR };
        m_ctx.bDebugDraw = _bDebugDraw;
        m_pState         = new StageState;
        m_runner         = jug::Scoped<SimRunner> { new SimRunner { m_ctx, jug::Scoped<IGameState> { m_pState } } };
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
    jug::RngMt19937 m_rng;
    GameContext     m_ctx = {};
    InputFrame      m_frame = {};
    StageState*     m_pState = nullptr;   // m_runner 가 소유한다
    jug::Scoped<SimRunner> m_runner = nullptr;   // 가장 마지막에 파괴되어야 OnExit 이 m_ctx 를 본다
};

SpawnRequest MakeRequest(
    const char*          _def,
    const eEntityKind    _kind,
    const jug::VECTOR2   _position)
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

}   // namespace

// ===========================================
//  로드 / 시작
// ===========================================

TEST(StageState, LoadsStage0AndStartsAtTheScriptedPlayerStart)
{
    StageSim sim;
    sim.Step();   // OnEnter 가 끝난 뒤 첫 틱 (섹션 0 "intro" 가 발동)

    StageState& state = sim.State();
    ASSERT_TRUE(state.IsLoaded());
    EXPECT_FALSE(state.IsFinished());
    EXPECT_EQ(state.GetData().name, "Stage0");
    EXPECT_EQ(state.GetData().sections.size(), 21u);
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
        sim.Step();
        const float camX = sim.State().GetCamera().GetPositionNoShake().x;
        ASSERT_GE(camX, lastCamX - 1e-3f) << "tick " << i;
        lastCamX = camX;
    }
    EXPECT_GT(sim.State().GetPlayer().GetFeet().x, 750.f);
    EXPECT_GE(sim.State().GetRunner().GetNextSectionIndex(), 2);   // intro, street 발동
}

// ===========================================
//  디버그 워프 (F1 켜진 상태에서만)
// ===========================================

TEST(StageStateDebug, F5WarpsToTheNextSectionOnlyWithDebugDraw)
{
    StageSim sim { true };
    sim.Step();                       // 섹션 0 발동 → 다음은 1 ("street", triggerX 750)
    ASSERT_EQ(sim.State().GetRunner().GetNextSectionIndex(), 1);

    sim.Ctx().debugWarp = 1;          // F5
    sim.Step();
    EXPECT_EQ(sim.Ctx().debugWarp, 0);   // 소비됨
    EXPECT_GT(sim.State().GetPlayer().GetFeet().x, 750.f);
    EXPECT_EQ(sim.State().GetRunner().GetNextSectionIndex(), 2);
    EXPECT_EQ(sim.State().GetHost().GetPlaceholders().GetLastSectionId(), "street");
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

TEST(StageStateDebug, F6WarpsToTheBossArena)
{
    StageSim sim { true };
    sim.Step();
    sim.Ctx().debugWarp = 2;          // F6
    sim.Step();
    EXPECT_GT(sim.State().GetPlayer().GetFeet().x, 13500.f);
    EXPECT_TRUE(sim.State().GetRunner().GetNextSectionIndex() >= 20);
}

// ===========================================
//  StagePlaceholders 단위
// ===========================================

TEST(StagePlaceholders, HitsKillAtZeroHpThenTheEntityIsRemovedAfterTheDeathWindow)
{
    StagePlaceholders host;
    SpawnRequest      request = MakeRequest("soldier_bomb", eEntityKind::Enemy, { 500.f, 378.f });
    request.bHasHp = true;
    request.hp     = 2;
    const SpawnId id = host.SpawnEntity(request);

    EXPECT_NE(id, kInvalidSpawnId);
    EXPECT_FALSE(host.IsDefeated(id));

    const jug::Vector<HitEvent> events = { MakeHit(id, 1) };
    EXPECT_EQ(host.ApplyEvents(events), 0);
    EXPECT_FALSE(host.IsDefeated(id));

    const jug::Vector<HitEvent> killing = { MakeHit(id, 5) };
    EXPECT_EQ(host.ApplyEvents(killing), 1);
    EXPECT_TRUE(host.IsDefeated(id));
    EXPECT_FALSE(host.IsDestroyFinished(id));

    // 죽은 뒤에는 다시 맞아도 처치로 세지 않는다
    EXPECT_EQ(host.ApplyEvents(killing), 0);

    for (int i = 0; i < kPlaceholderDeathTicks; ++i)
    {
        host.Tick();
    }
    EXPECT_TRUE(host.IsDestroyFinished(id));
    EXPECT_TRUE(host.GetEntities().empty());
}

TEST(StagePlaceholders, NonHittableKindsAreNeverDefeatedUntilCleared)
{
    StagePlaceholders host;
    const SpawnId     decor = host.SpawnEntity(MakeRequest("anim_decor", eEntityKind::Decor, { 10.f, 10.f }));
    const SpawnId     item  = host.SpawnEntity(MakeRequest("random_score", eEntityKind::Item, { 20.f, 20.f }));

    const jug::Vector<HitEvent> events = { MakeHit(decor, 99), MakeHit(item, 99) };
    EXPECT_EQ(host.ApplyEvents(events), 0);
    EXPECT_FALSE(host.IsDefeated(decor));
    EXPECT_FALSE(host.IsDefeated(item));

    EntityFilter filter;
    filter.layerMask = LayerBit(eStageLayer::Item);
    host.ClearEntities(filter);
    EXPECT_TRUE(host.IsDefeated(item));   // 제거된 ID 는 격파로 본다
    EXPECT_FALSE(host.IsDefeated(decor));
}

TEST(StagePlaceholders, LegacyCenterPositionsPutTheFeetAtTheBottomOfTheBox)
{
    StagePlaceholders host;
    SpawnRequest      request = MakeRequest("crate", eEntityKind::Prop, { 300.f, 100.f });
    request.positionMode      = ePositionMode::LegacyCenter;
    host.SpawnEntity(request);

    ASSERT_EQ(host.GetEntities().size(), 1u);
    const PlaceholderEntity& entity = host.GetEntities()[0];
    EXPECT_FLOAT_EQ(entity.feet.x, 300.f);
    EXPECT_FLOAT_EQ(entity.feet.y, 100.f + entity.size.y * 0.5f);
    EXPECT_TRUE(entity.bHittable);
}

TEST(StagePlaceholders, GroundLookupFollowsTheActiveMap)
{
    StagePlaceholders host;
    GroundMap         low = GroundMap::MakeEmpty(200, 100);
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
