// StageRunner 테스트: 트리거, 스폰, 웨이브, 게이트, 워프, 타이밍 등 섹션 액션 진행 검증
#include <cmath>
#include <optional>
#include <string>
#include <utility>
#include <vector>

#include <gtest/gtest.h>

#include "Base/Math.h"
#include "Base/Random.h"
#include "StageTestUtil.h"

using namespace slug;
using namespace slugtest;

namespace
{
[[nodiscard]] std::string OneSection_(
    const std::string& _actions,
    const std::string& _camera = R"J({"initialBounds":[0,0,4000,448]})J")
{
    return WrapStage(R"J([{"id":"s0","triggerX":0,"actions":)J" + _actions + "}]", _camera);
}

constexpr const char* kTallBounds = R"J({"initialBounds":[0,0,4000,1000]})J";

// (x == 100 에만 바닥 352 가 있는) 플레이어 시작 바닥 질의
[[nodiscard]] std::optional<float> PlayerStartGround_(
    const float _x,
    const float)
{
    return _x == 100.f ? std::optional<float> { 352.f } : std::nullopt;
}

// x < 1000 이고 fromY <= 300 일 때만 바닥 300 이 있는 질의 (그 밖은 바닥 없음)
[[nodiscard]] std::optional<float> SnapGround_(
    const float _x,
    const float _fromY)
{
    if (_x < 1000.f && _fromY <= 300.f)
    {
        return 300.f;
    }
    return std::nullopt;
}

// 시드를 바꿔 웨이브를 70틱 돌린 뒤 스폰된 x 좌표 목록
[[nodiscard]] std::vector<float> CollectWaveXs_(
    const std::string& _json,
    const uint64_t     _seed)
{
    RunnerFixture fx { _json };
    fx.rng.Reseed(_seed);
    fx.StepAt(5.f, 70);
    std::vector<float> xs;
    for (const SpawnLogEntry& entry: fx.host.spawns)
    {
        xs.push_back(entry.request.position.x);
    }
    return xs;
}
}   // namespace

TEST(StageRunnerBegin, AppliesInitialGroundBackdropAndBgm)
{
    const std::string json = R"J({"version":1,"name":"T","world":{"size":[4000,1000],"background":"bg"},
"bgm":{"name":"Stage1_BGM","loop":true},
"ground":{"initial":"G0","maps":["G0"]},
"backdrop":{"initial":"a","modes":{"a":{"sprite":"s","offset":[0,0],"parallaxX":0.1}}},
"camera":{"initialBounds":[0,0,4000,448],"initialOffset":[7,8]},
"sections":[{"id":"s0","triggerX":0,"actions":[]}]})J";
    RunnerFixture     fx { json };
    EXPECT_EQ(fx.host.currentGround, "G0");
    EXPECT_EQ(fx.host.currentBackdrop, "a");
    ASSERT_NE(fx.host.FindEvent("bgm:Stage1_BGM:loop"), nullptr);
    EXPECT_FLOAT_EQ(fx.runner.GetCameraOffset().x, 7.f);
    EXPECT_TRUE(fx.camera.IsForwardOnly());
    EXPECT_TRUE(fx.camera.IsForwardOnlyY());
    EXPECT_EQ(fx.camera.GetBounds(), Aabb::FromMinMax({ 0.f, 0.f }, { 4000.f, 448.f }));
}

TEST(StageRunnerBegin, ResolvesPlayerStartWithGroundSnap)
{
    const std::string json = R"J({"version":1,"name":"T","world":{"size":[4000,1000],"background":"bg"},
"ground":{"initial":"G0","maps":["G0"]},
"player":{"x":100,"groundSnap":true,"fromY":10},
"sections":[{"id":"s0","triggerX":0,"actions":[]}]})J";
    RunnerFixture     fx { json };
    fx.host.groundFn                = PlayerStartGround_;
    const std::optional<Vec2> start = fx.runner.ResolvePlayerStart();
    ASSERT_TRUE(start.has_value());
    EXPECT_FLOAT_EQ(start->x, 100.f);
    EXPECT_FLOAT_EQ(start->y, 352.f);
    ASSERT_FALSE(fx.host.groundQueries.empty());
    EXPECT_FLOAT_EQ(fx.host.groundQueries.back().second, 10.f);
}

TEST(StageRunnerTrigger, FiresOnlyWhenStrictlyGreaterThanTriggerX)
{
    RunnerFixture fx { WrapStage(R"J([{"id":"a","triggerX":100,"actions":[{"type":"banner","text":"A","seconds":1}]}])J") };
    fx.StepAt(100.f, 5);
    EXPECT_EQ(fx.host.CountEvents("banner"), 0);
    EXPECT_EQ(fx.runner.GetNextSectionIndex(), 0);
    ASSERT_TRUE(fx.runner.GetNextTriggerX().has_value());
    EXPECT_FLOAT_EQ(*fx.runner.GetNextTriggerX(), 100.f);

    fx.StepAt(100.01f);
    EXPECT_EQ(fx.host.CountEvents("banner"), 1);
    EXPECT_EQ(fx.runner.GetNextSectionIndex(), 1);
    EXPECT_FALSE(fx.runner.GetNextTriggerX().has_value());
}

TEST(StageRunnerTrigger, EachSectionFiresExactlyOnce)
{
    RunnerFixture fx { WrapStage(R"J([{"id":"a","triggerX":10,"actions":[{"type":"banner","text":"A","seconds":1},{"type":"spawn","kind":"prop","def":"p","x":1,"y":1}]}])J") };
    fx.StepAt(50.f, 30);
    fx.StepAt(5.f, 10);    // 되돌아갔다가
    fx.StepAt(60.f, 10);   // 다시 지나가도
    EXPECT_EQ(fx.host.CountEvents("banner"), 1);
    EXPECT_EQ(fx.host.spawns.size(), 1u);
    EXPECT_EQ(fx.host.CountEvents("section:"), 1);
}

TEST(StageRunnerTrigger, SeveralSectionsFireInOrderInOneTick)
{
    RunnerFixture fx { WrapStage(R"J([
        {"id":"a","triggerX":100,"actions":[{"type":"banner","text":"A","seconds":1}]},
        {"id":"b","triggerX":200,"actions":[{"type":"banner","text":"B","seconds":1}]},
        {"id":"c","triggerX":300,"actions":[{"type":"banner","text":"C","seconds":1}]}])J") };
    fx.StepAt(500.f);
    ASSERT_EQ(fx.host.events.size(), 2u + 6u);
    std::vector<std::string> order;
    for (const HostEvent& event: fx.host.events)
    {
        if (event.text.rfind("banner:", 0) == 0 || event.text.rfind("section:", 0) == 0)
        {
            order.push_back(event.text.substr(0, 9));
            EXPECT_EQ(event.tick, 0);
        }
    }
    ASSERT_EQ(order.size(), 6u);
    EXPECT_EQ(order[0], "section:0");
    EXPECT_EQ(order[1], "banner:A:");
    EXPECT_EQ(order[2], "section:1");
    EXPECT_EQ(order[3], "banner:B:");
    EXPECT_EQ(order[4], "section:2");
    EXPECT_EQ(order[5], "banner:C:");
}

TEST(StageRunnerTrigger, SameTriggerXKeepsArrayOrder)
{
    RunnerFixture fx { WrapStage(R"J([
        {"id":"a","triggerX":50,"actions":[{"type":"banner","text":"first","seconds":1}]},
        {"id":"b","triggerX":50,"actions":[{"type":"banner","text":"second","seconds":1}]}])J") };
    fx.StepAt(51.f);
    const HostEvent* pFirst  = fx.host.FindEvent("banner:first");
    const HostEvent* pSecond = fx.host.FindEvent("banner:second");
    ASSERT_NE(pFirst, nullptr);
    ASSERT_NE(pSecond, nullptr);
    EXPECT_LT(pFirst, pSecond);
}

TEST(StageRunnerTrigger, ActionsRunInArrayOrder)
{
    RunnerFixture fx { OneSection_(R"J([
        {"type":"spawn","kind":"prop","def":"first","x":1,"y":1},
        {"type":"spawn","kind":"prop","def":"second","x":1,"y":1},
        {"type":"spawn","kind":"prop","def":"third","x":1,"y":1}])J") };
    fx.StepAt(5.f);
    ASSERT_EQ(fx.host.spawns.size(), 3u);
    EXPECT_EQ(fx.host.spawns[0].request.def, "first");
    EXPECT_EQ(fx.host.spawns[1].request.def, "second");
    EXPECT_EQ(fx.host.spawns[2].request.def, "third");
    EXPECT_EQ(fx.host.spawns[0].id, 1u);
    EXPECT_EQ(fx.host.spawns[2].id, 3u);
}

TEST(StageRunnerTiming, DelaysAreExactTicks)
{
    RunnerFixture fx { OneSection_(R"J([
        {"type":"banner","text":"now","seconds":1},
        {"type":"banner","text":"half","seconds":1,"delay":0.5},
        {"type":"banner","text":"one","seconds":1,"delay":1.0},
        {"type":"banner","text":"odd","seconds":1,"delay":6.7},
        {"type":"banner","text":"long","seconds":1,"delay":13.0}])J") };
    fx.StepAt(5.f, 900);
    ASSERT_NE(fx.host.FindEvent("banner:now"), nullptr);
    EXPECT_EQ(fx.host.FindEvent("banner:now")->tick, 0);
    EXPECT_EQ(fx.host.FindEvent("banner:half")->tick, 30);
    EXPECT_EQ(fx.host.FindEvent("banner:one")->tick, 60);
    EXPECT_EQ(fx.host.FindEvent("banner:odd")->tick, 402);
    EXPECT_EQ(fx.host.FindEvent("banner:long")->tick, 780);
}

TEST(StageRunnerTiming, DelayCountsFromTheTickTheSectionFired)
{
    RunnerFixture fx { WrapStage(R"J([{"id":"a","triggerX":100,"actions":[{"type":"banner","text":"b","seconds":1,"delay":1.0}]}])J") };
    fx.StepAt(0.f, 25);
    fx.StepAt(101.f);   // tick 25 에 발동
    fx.StepAt(101.f, 100);
    EXPECT_EQ(fx.host.FindEvent("banner:b")->tick, 25 + 60);
}

TEST(StageRunnerTiming, SameDueTickKeepsScheduleOrder)
{
    RunnerFixture fx { OneSection_(R"J([
        {"type":"banner","text":"X","seconds":1,"delay":1.0},
        {"type":"banner","text":"Y","seconds":1,"delay":1.0},
        {"type":"banner","text":"Z","seconds":1,"delay":0.5}])J") };
    fx.StepAt(5.f, 100);
    EXPECT_EQ(fx.host.FindEvent("banner:Z")->tick, 30);
    ASSERT_NE(fx.host.FindEvent("banner:X"), nullptr);
    EXPECT_EQ(fx.host.FindEvent("banner:X")->tick, 60);
    EXPECT_EQ(fx.host.FindEvent("banner:Y")->tick, 60);
    EXPECT_LT(fx.host.FindEvent("banner:X"), fx.host.FindEvent("banner:Y"));
}

TEST(StageRunnerWave, EntriesSpawnAtTheirDelayWithCameraAnchoredPositions)
{
    RunnerFixture fx { OneSection_(R"J([{"type":"spawnWave","entries":[
        {"delay":3.5,"kind":"enemy","def":"soldier_bomb","camera":{"ax":"right","dx":[-149,-50],"ay":"top","dy":47.5}},
        {"delay":9.0,"kind":"enemy","def":"soldier_sword","camera":{"ax":"left","dx":10,"ay":"bottom","dy":-5}}]}])J") };
    fx.StepAt(5.f, 1);
    EXPECT_EQ(fx.runner.GetPendingWaveCount(), 2);
    EXPECT_TRUE(fx.host.spawns.empty());

    fx.StepAt(5.f, 209);   // tick 209 까지
    EXPECT_TRUE(fx.host.spawns.empty());
    fx.StepAt(5.f);   // tick 210
    ASSERT_EQ(fx.host.spawns.size(), 1u);
    EXPECT_EQ(fx.host.spawns[0].tick, 210);
    const SpawnRequest& first = fx.host.spawns[0].request;
    EXPECT_TRUE(first.bWave);
    EXPECT_TRUE(first.bFromCamera);
    EXPECT_EQ(first.positionMode, ePositionMode::Feet);
    EXPECT_GE(first.position.x, 640.f - 149.f);
    EXPECT_LE(first.position.x, 640.f - 50.f);
    EXPECT_FLOAT_EQ(first.position.y, 47.5f);

    fx.StepAt(5.f, 330);   // 540 까지
    ASSERT_EQ(fx.host.spawns.size(), 2u);
    EXPECT_EQ(fx.host.spawns[1].tick, 540);
    EXPECT_FLOAT_EQ(fx.host.spawns[1].request.position.x, 10.f);
    EXPECT_FLOAT_EQ(fx.host.spawns[1].request.position.y, 448.f - 5.f);
    EXPECT_EQ(fx.runner.GetPendingWaveCount(), 0);
}

TEST(StageRunnerWave, AnchorUsesTheCameraAtSpawnTime)
{
    RunnerFixture fx { OneSection_(R"J([{"type":"spawnWave","entries":[
        {"delay":2.0,"kind":"enemy","def":"e","camera":{"ax":"right","dx":-100,"ay":"top","dy":0}}]}])J") };
    fx.camera.SetBounds(Aabb::FromMinMax({ 0.f, 0.f }, { 4000.f, 448.f }));
    fx.StepAt(5.f, 1);
    fx.camera.SnapTo({ 2000.f, 224.f });   // 스폰 시점 전에 카메라가 이동
    fx.StepAt(2000.f, 130);
    ASSERT_EQ(fx.host.spawns.size(), 1u);
    const float expectedRight = fx.camera.GetPositionNoShake().x + 640.f - 100.f;
    EXPECT_NEAR(fx.host.spawns[0].request.position.x, expectedRight, 1e-3f);
    EXPECT_GT(fx.host.spawns[0].request.position.x, 2000.f);
}

TEST(StageRunnerWave, RandomDxIsSeededInRangeAndFixedDxDoesNotDrawRng)
{
    const std::string        json = OneSection_(R"J([{"type":"spawnWave","entries":[
        {"delay":1.0,"kind":"enemy","def":"e","camera":{"ax":"left","dx":[100,200],"ay":"top","dy":0},"repeat":{"count":40,"stepX":0}}]}])J");
    const std::vector<float> a    = CollectWaveXs_(json, 7);
    const std::vector<float> a2   = CollectWaveXs_(json, 7);
    const std::vector<float> b    = CollectWaveXs_(json, 8);
    ASSERT_EQ(a.size(), 40u);
    EXPECT_EQ(a, a2);
    EXPECT_NE(a, b);
    bool bVaried = false;
    for (const float x: a)
    {
        EXPECT_GE(x, 100.f);
        EXPECT_LE(x, 200.f);
        bVaried = bVaried || x != a.front();
    }
    EXPECT_TRUE(bVaried);

    RunnerFixture fixed { OneSection_(R"J([{"type":"spawnWave","entries":[
        {"delay":1.0,"kind":"enemy","def":"e","camera":{"ax":"left","dx":123,"ay":"top","dy":0}}]}])J") };
    fixed.StepAt(5.f, 70);
    slug::RngMt19937 fresh { uint64_t { 1 } };
    EXPECT_EQ(fixed.rng.Generate(), fresh.Generate());
}

TEST(StageRunnerWave, WaveEntryRepeatSpawnsCountInstances)
{
    RunnerFixture fx { OneSection_(R"J([{"type":"spawnWave","entries":[
        {"delay":1.0,"kind":"enemy","def":"e","x":100,"y":50,"repeat":{"count":3,"stepX":10,"stepY":1}}]}])J") };
    fx.StepAt(5.f, 100);
    ASSERT_EQ(fx.host.spawns.size(), 3u);
    EXPECT_FLOAT_EQ(fx.host.spawns[2].request.position.x, 120.f);
    EXPECT_FLOAT_EQ(fx.host.spawns[2].request.position.y, 52.f);
    EXPECT_EQ(fx.host.spawns[2].request.repeatIndex, 2);
}

TEST(StageRunnerSpawn, RepeatStepsPositionAndKeepsOriginMode)
{
    RunnerFixture fx { OneSection_(R"J([{"type":"spawn","kind":"prop","def":"fence","x":100,"y":50,"repeat":{"count":3,"stepX":64,"stepY":2}}])J") };
    fx.StepAt(5.f);
    ASSERT_EQ(fx.host.spawns.size(), 3u);
    EXPECT_FLOAT_EQ(fx.host.spawns[0].request.position.x, 100.f);
    EXPECT_FLOAT_EQ(fx.host.spawns[1].request.position.x, 164.f);
    EXPECT_FLOAT_EQ(fx.host.spawns[2].request.position.x, 228.f);
    EXPECT_FLOAT_EQ(fx.host.spawns[2].request.position.y, 54.f);
    EXPECT_EQ(fx.host.spawns[0].request.positionMode, ePositionMode::LegacyCenter);
    EXPECT_EQ(fx.host.spawns[1].request.repeatIndex, 1);
}

TEST(StageRunnerSpawn, OriginConversions)
{
    RunnerFixture fx { OneSection_(R"J([
        {"type":"spawn","kind":"enemy","def":"big_vehicle","id":"big","x":2995,"y":255,"origin":"legacyCenter","halfHeight":130},
        {"type":"spawn","kind":"prop","def":"truck","x":580,"y":342},
        {"type":"spawn","kind":"enemy","def":"soldier_bomb","x":900,"y":300},
        {"type":"spawn","kind":"enemy","def":"flyer","origin":"legacyCenter","halfHeight":50,"camera":{"ax":"right","dx":250,"ay":"top","dy":125}},
        {"type":"spawn","kind":"npc","def":"pow","x":10,"y":20,"settle":true,"drop":"gunitem_mg","facing":"left","hp":4}])J") };
    fx.StepAt(5.f);
    ASSERT_EQ(fx.host.spawns.size(), 5u);

    const SpawnRequest& big = fx.host.spawns[0].request;
    EXPECT_EQ(big.positionMode, ePositionMode::Feet);
    EXPECT_FLOAT_EQ(big.position.y, 385.f);
    EXPECT_FLOAT_EQ(big.halfHeight, 130.f);
    EXPECT_EQ(big.stageId, "big");

    const SpawnRequest& truck = fx.host.spawns[1].request;
    EXPECT_EQ(truck.positionMode, ePositionMode::LegacyCenter);
    EXPECT_FLOAT_EQ(truck.position.y, 342.f);

    const SpawnRequest& soldier = fx.host.spawns[2].request;
    EXPECT_EQ(soldier.positionMode, ePositionMode::Feet);
    EXPECT_FLOAT_EQ(soldier.position.y, 300.f);

    const SpawnRequest& flyer = fx.host.spawns[3].request;
    EXPECT_EQ(flyer.positionMode, ePositionMode::Feet);
    EXPECT_FLOAT_EQ(flyer.position.x, 640.f + 250.f);
    EXPECT_FLOAT_EQ(flyer.position.y, 125.f + 50.f);
    EXPECT_TRUE(flyer.bFromCamera);

    const SpawnRequest& pow = fx.host.spawns[4].request;
    EXPECT_TRUE(pow.bSettle);
    EXPECT_EQ(pow.drop, "gunitem_mg");
    EXPECT_EQ(pow.facing, -1);
    EXPECT_TRUE(pow.bHasHp);
    EXPECT_EQ(pow.hp, 4);
}

TEST(StageRunnerSpawn, GroundSnapUsesHostAndSkipsWhenNoGround)
{
    RunnerFixture fx { OneSection_(R"J([
        {"type":"spawn","kind":"enemy","def":"a","x":500,"groundSnap":true},
        {"type":"spawn","kind":"enemy","def":"b","id":"skipme","x":1500,"groundSnap":true},
        {"type":"spawn","kind":"enemy","def":"c","x":600,"groundSnap":true,"fromY":400},
        {"type":"gate","id":"g","waitFor":["skipme"],"actions":[{"type":"banner","text":"opened","seconds":1}]}])J") };
    fx.host.groundFn = SnapGround_;
    fx.StepAt(5.f, 3);
    ASSERT_EQ(fx.host.spawns.size(), 1u);
    EXPECT_EQ(fx.host.spawns[0].request.def, "a");
    EXPECT_FLOAT_EQ(fx.host.spawns[0].request.position.y, 300.f);
    EXPECT_EQ(fx.runner.GetSkippedCount(), 2);
    EXPECT_EQ(fx.runner.GetSpawnedCount(), 1);
    EXPECT_TRUE(fx.runner.WasSpawnSkipped("skipme"));
    const std::vector<std::pair<float, float>>& queries = fx.host.groundQueries;
    ASSERT_EQ(queries.size(), 3u);
    EXPECT_FLOAT_EQ(queries[2].second, 400.f);   // fromY 전달

    EXPECT_NE(fx.host.FindEvent("banner:opened"), nullptr);
    EXPECT_EQ(fx.runner.GetOpenGateCount(), 0);
}

TEST(StageRunnerSpawn, HostRefusalCountsAsSkipped)
{
    RunnerFixture fx { OneSection_(R"J([{"type":"spawn","kind":"enemy","def":"a","id":"x","x":1,"y":1}])J") };
    fx.host.bRefuseSpawns = true;
    fx.StepAt(5.f);
    EXPECT_EQ(fx.runner.GetSkippedCount(), 1);
    EXPECT_TRUE(fx.runner.WasSpawnSkipped("x"));
}

TEST(StageRunnerSpawn, ParamsPointerAndLayerArePassedThrough)
{
    RunnerFixture fx { OneSection_(R"J([{"type":"spawn","kind":"decor","def":"anim_decor","x":1,"y":2,"anim":"Parker","layer":"farBackground","cullMargin":100,"params":{"velocity":[70,-50]}}])J") };
    fx.StepAt(5.f);
    ASSERT_EQ(fx.host.spawns.size(), 1u);
    const SpawnRequest& request = fx.host.spawns[0].request;
    EXPECT_EQ(request.anim, "Parker");
    EXPECT_TRUE(request.bHasLayer);
    EXPECT_EQ(request.layer, eStageLayer::FarBackground);
    EXPECT_FLOAT_EQ(request.cullMargin, 100.f);
    ASSERT_NE(request.pParams, nullptr);
    EXPECT_FLOAT_EQ(request.pParams->GetVec2("velocity").x, 70.f);
}

namespace
{
const char* kBossGate = R"J([
    {"type":"spawn","kind":"enemy","def":"boss","id":"boss","x":1,"y":1},
    {"type":"gate","id":"g","waitFor":["boss"],"actions":[
        {"type":"banner","text":"now","seconds":1},
        {"type":"banner","text":"later","seconds":1,"delay":0.5}]}])J";
}

TEST(StageRunnerGate, RunsActionsWhenTargetIsDefeatedWithDelaysRelativeToThatTick)
{
    RunnerFixture fx { OneSection_(kBossGate) };
    fx.StepAt(5.f, 50);
    EXPECT_EQ(fx.runner.GetOpenGateCount(), 1);
    EXPECT_EQ(fx.runner.GetOpenGateIds().front(), "g");
    EXPECT_EQ(fx.host.CountEvents("banner"), 0);

    fx.host.defeated.insert(fx.runner.FindSpawnIds("boss").front());
    fx.StepAt(5.f);   // tick 50
    EXPECT_EQ(fx.host.FindEvent("banner:now")->tick, 50);
    EXPECT_EQ(fx.runner.GetOpenGateCount(), 0);
    fx.StepAt(5.f, 40);
    EXPECT_EQ(fx.host.FindEvent("banner:later")->tick, 80);
    EXPECT_EQ(fx.host.CountEvents("banner"), 2);   // 한 번만
}

TEST(StageRunnerGate, UntilDestroyFinishedNeedsTheSecondCondition)
{
    RunnerFixture fx { OneSection_(R"J([
        {"type":"spawn","kind":"enemy","def":"boss","id":"boss","x":1,"y":1},
        {"type":"gate","id":"g","waitFor":["boss"],"until":"destroyFinished","actions":[{"type":"banner","text":"done","seconds":1}]}])J") };
    fx.StepAt(5.f);
    const SpawnId id = fx.runner.FindSpawnIds("boss").front();
    fx.host.defeated.insert(id);
    fx.StepAt(5.f, 10);
    EXPECT_EQ(fx.host.CountEvents("banner"), 0);   // 격파됐지만 연출이 안 끝남
    fx.host.destroyed.insert(id);
    fx.StepAt(5.f);
    EXPECT_EQ(fx.host.CountEvents("banner"), 1);
}

TEST(StageRunnerGate, WaitsForEveryInstanceOfARepeatedSpawn)
{
    RunnerFixture fx { OneSection_(R"J([
        {"type":"spawn","kind":"enemy","def":"t","id":"t","x":1,"y":1,"repeat":{"count":3,"stepX":10}},
        {"type":"gate","id":"g","waitFor":["t"],"actions":[{"type":"banner","text":"all","seconds":1}]}])J") };
    fx.StepAt(5.f);
    ASSERT_EQ(fx.runner.FindSpawnIds("t").size(), 3u);
    fx.host.defeated.insert(fx.runner.FindSpawnIds("t")[0]);
    fx.host.defeated.insert(fx.runner.FindSpawnIds("t")[1]);
    fx.StepAt(5.f, 5);
    EXPECT_EQ(fx.host.CountEvents("banner"), 0);
    fx.host.defeated.insert(fx.runner.FindSpawnIds("t")[2]);
    fx.StepAt(5.f);
    EXPECT_EQ(fx.host.CountEvents("banner"), 1);
}

TEST(StageRunnerGate, WaitsForSpawnsThatHaveNotHappenedYet)
{
    RunnerFixture fx { WrapStage(R"J([
        {"id":"a","triggerX":0,"actions":[{"type":"gate","id":"g","waitFor":["late"],"actions":[{"type":"banner","text":"opened","seconds":1}]}]},
        {"id":"b","triggerX":1000,"actions":[{"type":"spawn","kind":"enemy","def":"x","id":"late","x":1,"y":1}]}])J") };
    fx.StepAt(5.f, 20);
    EXPECT_EQ(fx.host.CountEvents("banner"), 0);   // 아직 스폰 전: 열리지 않는다
    fx.StepAt(1500.f);
    ASSERT_EQ(fx.runner.FindSpawnIds("late").size(), 1u);
    fx.StepAt(1500.f, 10);
    EXPECT_EQ(fx.host.CountEvents("banner"), 0);
    fx.host.defeated.insert(fx.runner.FindSpawnIds("late").front());
    fx.StepAt(1500.f);
    EXPECT_EQ(fx.host.CountEvents("banner"), 1);
}

TEST(StageRunnerGate, AbortOnGameOverCancelsAnOpenGate)
{
    RunnerFixture fx { OneSection_(R"J([
        {"type":"spawn","kind":"enemy","def":"boss","id":"boss","x":1,"y":1},
        {"type":"gate","id":"g","waitFor":["boss"],"abortOnGameOver":true,"actions":[{"type":"banner","text":"finale","seconds":1}]}])J") };
    fx.StepAt(5.f, 5);
    fx.host.bGameOver = true;
    fx.StepAt(5.f);
    EXPECT_EQ(fx.runner.GetOpenGateCount(), 0);
    fx.host.bGameOver = false;
    fx.host.defeated.insert(fx.runner.FindSpawnIds("boss").front());
    fx.StepAt(5.f, 5);
    EXPECT_EQ(fx.host.CountEvents("banner"), 0);
}

TEST(StageRunnerGate, AbortOnGameOverCancelsDelayedFinaleActionsOnly)
{
    RunnerFixture fx { OneSection_(R"J([
        {"type":"spawn","kind":"enemy","def":"boss","id":"boss","x":1,"y":1},
        {"type":"banner","text":"outside","seconds":1,"delay":3.0},
        {"type":"gate","id":"g","waitFor":["boss"],"abortOnGameOver":true,"actions":[
            {"type":"banner","text":"early","seconds":1},
            {"type":"banner","text":"cancelled","seconds":1,"delay":2.0},
            {"type":"endStage","next":"title","delay":3.0}]}])J") };
    fx.StepAt(5.f, 2);
    fx.host.defeated.insert(fx.runner.FindSpawnIds("boss").front());
    fx.StepAt(5.f, 10);
    EXPECT_NE(fx.host.FindEvent("banner:early"), nullptr);
    fx.host.bGameOver = true;
    fx.StepAt(5.f, 400);
    EXPECT_EQ(fx.host.FindEvent("banner:cancelled"), nullptr);
    EXPECT_FALSE(fx.host.bEnded);
    EXPECT_FALSE(fx.runner.IsFinished());
    EXPECT_NE(fx.host.FindEvent("banner:outside"), nullptr);
}

TEST(StageRunnerGate, WithoutAbortFlagGameOverDoesNotCancel)
{
    RunnerFixture fx { OneSection_(R"J([
        {"type":"spawn","kind":"enemy","def":"boss","id":"boss","x":1,"y":1},
        {"type":"gate","id":"g","waitFor":["boss"],"actions":[{"type":"banner","text":"opens","seconds":1}]}])J") };
    fx.StepAt(5.f);
    fx.host.bGameOver = true;
    fx.StepAt(5.f, 5);
    fx.host.defeated.insert(fx.runner.FindSpawnIds("boss").front());
    fx.StepAt(5.f);
    EXPECT_EQ(fx.host.CountEvents("banner"), 1);
}

TEST(StageRunnerGate, NestedGatesChain)
{
    RunnerFixture fx { OneSection_(R"J([
        {"type":"spawn","kind":"enemy","def":"a","id":"a","x":1,"y":1},
        {"type":"spawn","kind":"enemy","def":"b","id":"b","x":1,"y":1},
        {"type":"gate","id":"g1","waitFor":["a"],"actions":[
            {"type":"banner","text":"first","seconds":1},
            {"type":"gate","id":"g2","waitFor":["b"],"actions":[{"type":"banner","text":"second","seconds":1}]}]}])J") };
    fx.StepAt(5.f);
    fx.host.defeated.insert(fx.runner.FindSpawnIds("a").front());
    fx.StepAt(5.f, 3);
    EXPECT_EQ(fx.host.CountEvents("banner"), 1);
    EXPECT_EQ(fx.runner.GetOpenGateCount(), 1);
    fx.host.defeated.insert(fx.runner.FindSpawnIds("b").front());
    fx.StepAt(5.f);
    EXPECT_EQ(fx.host.CountEvents("banner"), 2);
}

TEST(StageRunnerCommand, SendsToEveryInstance)
{
    RunnerFixture fx { OneSection_(R"J([
        {"type":"spawn","kind":"enemy","def":"big_vehicle","id":"big","x":1,"y":1,"repeat":{"count":2,"stepX":5}},
        {"type":"command","target":"big","command":"startBattle"}])J") };
    fx.StepAt(5.f);
    ASSERT_EQ(fx.host.commands.size(), 2u);
    EXPECT_EQ(fx.host.commands[0].second, "startBattle");
    EXPECT_NE(fx.host.commands[0].first, fx.host.commands[1].first);
}

TEST(StageRunnerClear, ClearWavesCancelsWavesAndGatesButKeepsOtherDelayedActions)
{
    RunnerFixture fx { OneSection_(R"J([
        {"type":"spawn","kind":"enemy","def":"e","id":"e","x":1,"y":1},
        {"type":"gate","id":"g","waitFor":["e"],"actions":[{"type":"banner","text":"gate","seconds":1}]},
        {"type":"spawnWave","entries":[{"delay":5.0,"kind":"enemy","def":"w","x":1,"y":1},{"delay":0.5,"kind":"enemy","def":"w","x":1,"y":1}]},
        {"type":"banner","text":"kept","seconds":1,"delay":5.0},
        {"type":"clear","waves":true,"delay":1.0}])J") };
    fx.StepAt(5.f, 1);
    EXPECT_EQ(fx.runner.GetPendingWaveCount(), 2);
    EXPECT_EQ(fx.runner.GetPendingActionCount(), 2);   // kept + clear
    fx.StepAt(5.f, 100);
    EXPECT_EQ(fx.runner.GetPendingWaveCount(), 0);
    EXPECT_EQ(fx.runner.GetOpenGateCount(), 0);
    EXPECT_EQ(fx.host.CountSpawnsOf("w"), 1);   // 0.5 s 웨이브는 clear(1.0 s) 전에 이미 스폰됨
    fx.StepAt(5.f, 400);
    EXPECT_EQ(fx.host.CountSpawnsOf("w"), 1);   // 5 s 웨이브는 취소됨
    EXPECT_NE(fx.host.FindEvent("banner:kept"), nullptr);
}

TEST(StageRunnerClear, ClearLayersGoesToTheHostWithMask)
{
    RunnerFixture fx { OneSection_(R"J([{"type":"clear","layers":["monster","item"]}])J") };
    fx.StepAt(5.f);
    ASSERT_EQ(fx.host.clears.size(), 1u);
    EXPECT_EQ(fx.host.clears[0], LayerBit(eStageLayer::Monster) | LayerBit(eStageLayer::Item));
}

TEST(StageRunnerFlow, EndStageFinishesTheRunner)
{
    RunnerFixture fx { OneSection_(R"J([{"type":"banner","text":"bye","seconds":1},{"type":"endStage","next":"title","delay":0.5},{"type":"banner","text":"never","seconds":1,"delay":2.0}])J") };
    fx.StepAt(5.f, 30);   // tick 0..29
    EXPECT_FALSE(fx.runner.IsFinished());
    fx.StepAt(5.f);   // tick 30
    EXPECT_TRUE(fx.runner.IsFinished());
    EXPECT_TRUE(fx.host.bEnded);
    EXPECT_EQ(fx.host.endNext, "title");
    const int64_t tickAfter = fx.runner.GetTick();
    fx.StepAt(5.f, 200);
    EXPECT_EQ(fx.runner.GetTick(), tickAfter);
    EXPECT_EQ(fx.host.FindEvent("banner:never"), nullptr);
}

TEST(StageRunnerFlow, EndStageDefaultsToTitle)
{
    RunnerFixture fx { OneSection_(R"J([{"type":"endStage"}])J") };
    fx.StepAt(5.f);
    EXPECT_EQ(fx.host.endNext, "title");
}

TEST(StageRunnerActions, SimpleActionsAreForwardedToTheHost)
{
    RunnerFixture fx { OneSection_(R"J([
        {"type":"setGround","name":"G1"},
        {"type":"setBackdrop","mode":"b"},
        {"type":"setCameraOffset","offset":[50,-140]},
        {"type":"playBgm","name":"Boss_BGM","loop":false},
        {"type":"playerInvincible","seconds":10},
        {"type":"fade","to":"black","seconds":1},
        {"type":"banner","text":"MISSION\nCOMPLETE","seconds":4}])J") };
    fx.StepAt(5.f);
    EXPECT_EQ(fx.host.currentGround, "G1");
    EXPECT_EQ(fx.runner.GetGroundName(), "G1");
    EXPECT_EQ(fx.host.currentBackdrop, "b");
    EXPECT_EQ(fx.runner.GetBackdropMode(), "b");
    EXPECT_FLOAT_EQ(fx.runner.GetCameraOffset().x, 50.f);
    EXPECT_FLOAT_EQ(fx.runner.GetCameraOffset().y, -140.f);
    EXPECT_NE(fx.host.FindEvent("bgm:Boss_BGM:once"), nullptr);
    EXPECT_FLOAT_EQ(fx.host.invincibleSeconds, 10.f);
    EXPECT_NE(fx.host.FindEvent("fade:black"), nullptr);
    EXPECT_NE(fx.host.FindEvent("banner:MISSION\nCOMPLETE"), nullptr);

    const Vec2 target = fx.runner.ComputeCameraTarget({ 1000.f, 378.f });
    EXPECT_FLOAT_EQ(target.x, 1050.f);
    EXPECT_FLOAT_EQ(target.y, 378.f - kStageCameraTargetLift - 140.f);
}

namespace
{
constexpr const char* kWarpStage = R"J([
    {"id":"s0","triggerX":0,"actions":[
        {"type":"spawn","kind":"enemy","def":"e0","x":1,"y":1},
        {"type":"spawn","kind":"prop","def":"p0","x":1,"y":1},
        {"type":"spawn","kind":"npc","def":"n0","x":1,"y":1,"settle":true},
        {"type":"spawn","kind":"item","def":"i0","x":1,"y":1},
        {"type":"setGround","name":"G1","when":"silent"},
        {"type":"banner","text":"S0","seconds":1},
        {"type":"banner","text":"late","seconds":1,"delay":1.0,"when":"always"},
        {"type":"setBounds","rect":[0,0,3000,448]},
        {"type":"setBackdrop","mode":"b"},
        {"type":"setCameraOffset","offset":[5,6]},
        {"type":"lockCamera","rect":[0,0,640,448]}]},
    {"id":"s1","triggerX":100,"actions":[
        {"type":"banner","text":"S1","seconds":1}]},
    {"id":"boss","triggerX":200,"actions":[
        {"type":"banner","text":"BOSS","seconds":1}]}])J";
}

TEST(StageRunnerWarp, SilentRunsOnlyWhenAlwaysOrSilentActions)
{
    RunnerFixture fx { WrapStage(kWarpStage) };
    ASSERT_TRUE(fx.runner.WarpToSection(1));

    EXPECT_EQ(fx.host.CountSpawnsOf("e0"), 0);
    EXPECT_EQ(fx.host.CountSpawnsOf("p0"), 1);
    EXPECT_EQ(fx.host.CountSpawnsOf("n0"), 1);
    EXPECT_EQ(fx.host.CountSpawnsOf("i0"), 1);
    for (const SpawnLogEntry& entry: fx.host.spawns)
    {
        EXPECT_TRUE(entry.request.bSilent);
    }
    EXPECT_EQ(fx.host.currentGround, "G1");
    EXPECT_EQ(fx.host.currentBackdrop, "b");
    EXPECT_FLOAT_EQ(fx.runner.GetCameraOffset().x, 5.f);
    EXPECT_EQ(fx.runner.GetBaseBounds(), Aabb::FromMinMax({ 0.f, 0.f }, { 3000.f, 448.f }));
    EXPECT_EQ(fx.host.CountEvents("banner:S0"), 0);
    EXPECT_EQ(fx.host.CountEvents("banner:late"), 1);   // always 는 silent 에서 지연 없이 즉시
    EXPECT_EQ(fx.runner.GetLockDepth(), 0);             // lockCamera 는 normal

    ASSERT_EQ(fx.host.warps.size(), 1u);
    EXPECT_FLOAT_EQ(fx.host.warps[0].x, 101.f);
    EXPECT_FLOAT_EQ(fx.host.warps[0].y, 378.f);
    EXPECT_EQ(fx.runner.GetNextSectionIndex(), 1);
    EXPECT_EQ(fx.runner.GetTick(), 0);
    EXPECT_EQ(fx.host.CountEvents("banner:S1"), 0);
    fx.Step();
    EXPECT_EQ(fx.host.CountEvents("banner:S1"), 1);
    EXPECT_EQ(fx.host.CountEvents("section:1"), 1);
    EXPECT_EQ(fx.host.CountEvents("section:0"), 0);   // 건너뛴 섹션은 OnSectionEntered 없음
}

TEST(StageRunnerWarp, CannotWarpBackwardOrOutOfRange)
{
    RunnerFixture fx { WrapStage(kWarpStage) };
    fx.StepAt(5.f);   // s0 발동
    EXPECT_FALSE(fx.runner.WarpToSection(0));
    EXPECT_FALSE(fx.runner.WarpToSection(-1));
    EXPECT_FALSE(fx.runner.WarpToSection(3));
    EXPECT_FALSE(fx.runner.WarpToSection("nope"));
    EXPECT_TRUE(fx.runner.WarpToSection("s1"));
}

TEST(StageRunnerWarp, WarpToBossPicksBossSectionOrLast)
{
    RunnerFixture fx { WrapStage(kWarpStage) };
    ASSERT_TRUE(fx.runner.WarpToBoss());
    EXPECT_EQ(fx.runner.GetNextSectionIndex(), 2);
    EXPECT_FLOAT_EQ(fx.host.warps.back().x, 201.f);

    RunnerFixture noBoss { WrapStage(R"J([{"id":"a","triggerX":0,"actions":[]},{"id":"z","triggerX":50,"actions":[]}])J") };
    ASSERT_TRUE(noBoss.runner.WarpToBoss());
    EXPECT_EQ(noBoss.runner.GetNextSectionIndex(), 1);
}

TEST(StageRunnerWarp, WarpClearsPendingWavesGatesAndNonPermanentLocks)
{
    RunnerFixture fx { WrapStage(R"J([
        {"id":"a","triggerX":0,"actions":[
            {"type":"spawn","kind":"enemy","def":"e","id":"e","x":1,"y":1},
            {"type":"lockCamera","rect":[0,0,640,448]},
            {"type":"gate","id":"g","waitFor":["e"],"actions":[]},
            {"type":"spawnWave","entries":[{"delay":5.0,"kind":"enemy","def":"w","x":1,"y":1}]},
            {"type":"banner","text":"pending","seconds":1,"delay":9.0}]},
        {"id":"b","triggerX":100,"actions":[]}])J") };
    fx.StepAt(5.f, 3);
    EXPECT_EQ(fx.runner.GetLockDepth(), 1);
    ASSERT_TRUE(fx.runner.WarpToSection(1));
    EXPECT_EQ(fx.runner.GetLockDepth(), 0);
    EXPECT_FALSE(fx.camera.IsLocked());
    EXPECT_EQ(fx.runner.GetOpenGateCount(), 0);
    EXPECT_EQ(fx.runner.GetPendingWaveCount(), 0);
    EXPECT_EQ(fx.runner.GetPendingActionCount(), 0);
    fx.StepAt(101.f, 700);
    EXPECT_EQ(fx.host.CountSpawnsOf("w"), 0);
    EXPECT_EQ(fx.host.CountEvents("banner:pending"), 0);
}

namespace
{
// 보스 워프 후 30틱 진행한 뒤의 (러너 해시, 스폰 수)
[[nodiscard]] std::pair<uint64_t, size_t> WarpToBossAndHash_()
{
    RunnerFixture fx { WrapStage(kWarpStage) };
    EXPECT_TRUE(fx.runner.WarpToBoss());
    fx.Step(30);
    return std::pair<uint64_t, size_t> { fx.runner.HashState(), fx.host.spawns.size() };
}

// 적이 아닌(항상 생성되는) 스폰의 def 이름 목록
[[nodiscard]] std::vector<std::string> NonEnemyDefs_(
    const FakeStageHost& _host)
{
    std::vector<std::string> defs;
    for (const SpawnLogEntry& entry: _host.spawns)
    {
        if (entry.request.kind != eEntityKind::Enemy)
        {
            defs.push_back(entry.request.def);
        }
    }
    return defs;
}
}   // namespace

TEST(StageRunnerWarp, WarpingTwiceFromTheSameStartIsDeterministic)
{
    EXPECT_EQ(WarpToBossAndHash_(), WarpToBossAndHash_());
}

TEST(StageRunnerWarp, WarpingGivesTheSameAlwaysStateAsPlayingThere)
{
    const std::string json = WrapStage(R"J([
        {"id":"s0","triggerX":0,"actions":[
            {"type":"spawn","kind":"prop","def":"truck","x":5,"y":5},
            {"type":"spawn","kind":"enemy","def":"boss","id":"boss","x":1,"y":1},
            {"type":"setGround","name":"G1","when":"silent"},
            {"type":"lockCamera","rect":[0,0,640,448]},
            {"type":"setBounds","rect":[0,0,3000,448]},
            {"type":"gate","id":"g","waitFor":["boss"],"actions":[{"type":"setGround","name":"G1"},{"type":"unlockCamera"}]}]},
        {"id":"s1","triggerX":200,"actions":[
            {"type":"spawn","kind":"npc","def":"pow","x":9,"y":9,"settle":true},
            {"type":"setBackdrop","mode":"b"},
            {"type":"setCameraOffset","offset":[3,4]},
            {"type":"spawn","kind":"enemy","def":"soldier","x":9,"y":9}]},
        {"id":"s2","triggerX":400,"actions":[{"type":"banner","text":"X","seconds":1}]}])J");

    RunnerFixture warped { json };
    ASSERT_TRUE(warped.runner.WarpToSection(2));

    RunnerFixture played { json };
    played.StepAt(10.f);
    played.host.DefeatAll();
    played.StepAt(10.f, 3);
    played.StepAt(250.f, 3);
    ASSERT_EQ(played.runner.GetNextSectionIndex(), 2);

    EXPECT_EQ(warped.runner.GetGroundName(), played.runner.GetGroundName());
    EXPECT_EQ(warped.runner.GetBackdropMode(), played.runner.GetBackdropMode());
    EXPECT_EQ(warped.runner.GetCameraOffset(), played.runner.GetCameraOffset());
    EXPECT_EQ(warped.runner.GetBaseBounds(), played.runner.GetBaseBounds());
    EXPECT_EQ(warped.runner.GetLockDepth(), played.runner.GetLockDepth());
    EXPECT_EQ(warped.camera.IsLocked(), played.camera.IsLocked());

    EXPECT_EQ(NonEnemyDefs_(warped.host), NonEnemyDefs_(played.host));
}

TEST(StageRunnerWalls, LeftWallAlwaysAndRightWallOnlyWhenLockedOrAtTheEnd)
{
    RunnerFixture fx { OneSection_(R"J([{"type":"lockCamera","rect":[320,0,960,448],"delay":1.0}])J") };
    fx.camera.SetBounds(Aabb::FromMinMax({ 0.f, 0.f }, { 4000.f, 448.f }));
    fx.camera.SnapTo({ 1000.f, 224.f });
    fx.StepAt(1000.f, 1);

    ScreenWalls walls = fx.runner.MakeScreenWalls();
    const Aabb  view  = Aabb::FromMinSize(fx.camera.GetPositionNoShake(), { 640.f, 448.f });
    ASSERT_TRUE(walls.bHasLeft);
    EXPECT_FLOAT_EQ(walls.left.max.x, view.min.x + 14.f);
    EXPECT_FALSE(walls.bHasRight);

    fx.StepAt(1000.f, 70);   // 락 적용
    walls = fx.runner.MakeScreenWalls();
    EXPECT_TRUE(fx.camera.IsLocked());
    ASSERT_TRUE(walls.bHasRight);
    const Aabb lockedView = Aabb::FromMinSize(fx.camera.GetPositionNoShake(), { 640.f, 448.f });
    EXPECT_FLOAT_EQ(walls.right.min.x, lockedView.max.x - 14.f);
    EXPECT_FLOAT_EQ(walls.left.max.x, lockedView.min.x + 14.f);
}

TEST(StageRunnerWalls, RightWallAtTheBoundsEnd)
{
    RunnerFixture fx { OneSection_("[]") };
    fx.camera.SnapTo({ 3900.f, 224.f });   // 경계 오른쪽 끝(4000)으로 클램프
    fx.StepAt(3900.f, 2);
    EXPECT_TRUE(fx.runner.MakeScreenWalls().bHasRight);
}
