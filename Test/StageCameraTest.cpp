#include <algorithm>
#include "Base/Types.h"
#include "Base/Result.h"
#include <cmath>

#include <gtest/gtest.h>

#include "StageTestUtil.h"

using namespace slug;
using namespace slugtest;

namespace
{
constexpr const char* kTall = R"J({"initialBounds":[0,0,4000,1000]})J";

std::string OneSection(
    const std::string& _actions,
    const std::string& _camera = R"J({"initialBounds":[0,0,4000,448]})J")
{
    return WrapStage(R"J([{"id":"s0","triggerX":0,"actions":)J" + _actions + "}]", _camera);
}

Camera MakeCamera(
    const Aabb& _bounds = Aabb::FromMinMax({ 0.f, 0.f }, { 4000.f, 1000.f }))
{
    Camera camera;
    camera.SetBounds(_bounds);
    return camera;
}
}

TEST(CameraStageExt, ForwardOnlyYNeverMovesUp)
{
    Camera camera = MakeCamera();
    camera.SetForwardOnlyY(true);
    camera.SetTarget({ 320.f, 700.f });
    camera.SnapToTarget();
    const float lowest = camera.GetViewCenter().y;
    camera.SetTarget({ 320.f, 100.f });   // 위로 올라간 플레이어
    for (int i = 0; i < 300; ++i)
    {
        camera.Update();
        ASSERT_GE(camera.GetViewCenter().y, lowest) << "tick " << i;
    }
}

TEST(CameraStageExt, ForwardOnlyYIsOffByDefaultAndBoundsStillWin)
{
    Camera camera = MakeCamera();
    EXPECT_FALSE(camera.IsForwardOnlyY());
    camera.SetTarget({ 320.f, 700.f });
    camera.SnapToTarget();
    camera.SetTarget({ 320.f, 100.f });
    for (int i = 0; i < 600; ++i)
    {
        camera.Update();
    }
    EXPECT_LT(camera.GetViewCenter().y, 400.f);   // 기본은 따라 올라간다

    camera.SetForwardOnlyY(true);
    camera.SetBounds(Aabb::FromMinMax({ 0.f, 0.f }, { 4000.f, 448.f }));   // 경계가 y 를 강제하면 전진 전용보다 우선
    camera.Update();
    EXPECT_FLOAT_EQ(camera.GetViewCenter().y, 224.f);
}

TEST(CameraStageExt, BeginMoveIsLinearAndTickExactThenTrackingResumes)
{
    Camera camera = MakeCamera();
    camera.SnapTo({ 320.f, 224.f });
    camera.SetTarget({ 9999.f, 224.f });   // 이동 중에는 무시되어야 한다
    camera.BeginMove({ 820.f, 424.f }, 0.5f);
    EXPECT_TRUE(camera.IsMoving());

    for (int k = 1; k <= 30; ++k)
    {
        camera.Update();
        if (k < 30)
        {
            ASSERT_TRUE(camera.IsMoving()) << k;
            EXPECT_NEAR(camera.GetViewCenter().x, 320.f + 500.f * static_cast<float>(k) / 30.f, 1e-3f) << k;
            EXPECT_NEAR(camera.GetViewCenter().y, 224.f + 200.f * static_cast<float>(k) / 30.f, 1e-3f) << k;
        }
    }
    EXPECT_FALSE(camera.IsMoving());
    EXPECT_FLOAT_EQ(camera.GetViewCenter().x, 820.f);
    EXPECT_FLOAT_EQ(camera.GetViewCenter().y, 424.f);

    camera.SetTarget({ 2000.f, 424.f });
    for (int i = 0; i < 200; ++i)
    {
        camera.Update();
    }
    EXPECT_GT(camera.GetViewCenter().x, 1500.f);   // 추적 재개
}

TEST(CameraStageExt, BeginMoveWithZeroSecondsSnapsAndMoveRespectsBounds)
{
    Camera camera = MakeCamera();
    camera.BeginMove({ 1500.f, 300.f }, 0.f);
    EXPECT_FALSE(camera.IsMoving());
    EXPECT_FLOAT_EQ(camera.GetViewCenter().x, 1500.f);

    camera.BeginMove({ 9000.f, 300.f }, 1.f);   // 경계 밖 목표: 이동 중 매 틱 경계로 클램프
    for (int i = 0; i < 60; ++i)
    {
        camera.Update();
        ASSERT_LE(camera.GetViewCenter().x, 3680.f + 1e-3f);
    }
    EXPECT_FALSE(camera.IsMoving());
    EXPECT_FLOAT_EQ(camera.GetViewCenter().x, 3680.f);
}

TEST(CameraStageExt, LockWithGlideMovesFirstThenLocksOnArrival)
{
    Camera camera = MakeCamera(Aabb::FromMinMax({ 0.f, 0.f }, { 4000.f, 448.f }));
    camera.SnapTo({ 2400.f, 224.f });
    camera.LockWithGlide(Aabb::FromMinMax({ 2470.f, 0.f }, { 3110.f, 448.f }), 1.f);
    EXPECT_TRUE(camera.HasPendingLock());
    EXPECT_FALSE(camera.IsLocked());

    for (int k = 1; k <= 59; ++k)
    {
        camera.Update();
        ASSERT_FALSE(camera.IsLocked()) << k;
        EXPECT_NEAR(camera.GetViewCenter().x, 2400.f + 390.f * static_cast<float>(k) / 60.f, 1e-2f) << k;
    }
    camera.Update();
    EXPECT_TRUE(camera.IsLocked());
    EXPECT_FALSE(camera.HasPendingLock());
    EXPECT_FLOAT_EQ(camera.GetViewCenter().x, 2790.f);

    camera.SetTarget({ 5000.f, 224.f });
    for (int i = 0; i < 120; ++i)
    {
        camera.Update();
        ASSERT_FLOAT_EQ(camera.GetPositionNoShake().x, 2470.f);
    }
}

TEST(CameraStageExt, UnlockDuringGlideCancelsThePendingLock)
{
    Camera camera = MakeCamera(Aabb::FromMinMax({ 0.f, 0.f }, { 4000.f, 448.f }));
    camera.SnapTo({ 1000.f, 224.f });
    camera.LockWithGlide(Aabb::FromMinMax({ 1500.f, 0.f }, { 2140.f, 448.f }), 1.f);
    for (int i = 0; i < 20; ++i)
    {
        camera.Update();
    }
    camera.Unlock();
    EXPECT_FALSE(camera.HasPendingLock());
    for (int i = 0; i < 60; ++i)
    {
        camera.Update();
    }
    EXPECT_FALSE(camera.IsLocked());
}

TEST(CameraStageExt, LockWithZeroGlideIsPlainLock)
{
    Camera camera = MakeCamera(Aabb::FromMinMax({ 0.f, 0.f }, { 4000.f, 448.f }));
    camera.LockWithGlide(Aabb::FromMinMax({ 100.f, 0.f }, { 740.f, 448.f }), 0.f);
    EXPECT_TRUE(camera.IsLocked());
    EXPECT_FALSE(camera.HasPendingLock());
}

TEST(StageRunnerCamera, SetBoundsDuringLockOnlyChangesTheRestoredBounds)
{
    RunnerFixture fx { OneSection(R"J([
        {"type":"lockCamera","rect":[320,0,960,448]},
        {"type":"setBounds","rect":[0,0,3000,448]},
        {"type":"banner","text":"x","seconds":1}])J") };
    fx.StepAt(5.f);
    EXPECT_EQ(fx.runner.GetLockDepth(), 1);
    EXPECT_EQ(fx.runner.GetBaseBounds(), Aabb::FromMinMax({ 0.f, 0.f }, { 3000.f, 448.f }));
    EXPECT_EQ(fx.runner.GetEffectiveBounds(), Aabb::FromMinMax({ 320.f, 0.f }, { 960.f, 448.f }));
    EXPECT_TRUE(fx.camera.IsLocked());
    EXPECT_EQ(fx.camera.GetLockBounds(), Aabb::FromMinMax({ 320.f, 0.f }, { 960.f, 448.f }));
    EXPECT_EQ(fx.camera.GetBounds(), Aabb::FromMinMax({ 0.f, 0.f }, { 3000.f, 448.f }));
}

TEST(StageRunnerCamera, UnlockRestoresAndNestedLocksPopInOrder)
{
    RunnerFixture fx { OneSection(R"J([
        {"type":"lockCamera","rect":[0,0,1000,448]},
        {"type":"lockCamera","rect":[200,0,840,448]},
        {"type":"unlockCamera","delay":1.0},
        {"type":"unlockCamera","delay":2.0},
        {"type":"unlockCamera","delay":3.0}])J") };
    fx.StepAt(5.f);
    EXPECT_EQ(fx.runner.GetLockDepth(), 2);
    EXPECT_EQ(fx.camera.GetLockBounds(), Aabb::FromMinMax({ 200.f, 0.f }, { 840.f, 448.f }));

    fx.StepAt(5.f, 61);   // tick 60
    EXPECT_EQ(fx.runner.GetLockDepth(), 1);
    EXPECT_TRUE(fx.camera.IsLocked());
    EXPECT_EQ(fx.camera.GetLockBounds(), Aabb::FromMinMax({ 0.f, 0.f }, { 1000.f, 448.f }));

    fx.StepAt(5.f, 60);   // tick 120
    EXPECT_EQ(fx.runner.GetLockDepth(), 0);
    EXPECT_FALSE(fx.camera.IsLocked());

    fx.StepAt(5.f, 60);   // 남은 unlock 은 무시
    EXPECT_EQ(fx.runner.GetLockDepth(), 0);
    EXPECT_EQ(fx.runner.GetEffectiveBounds(), fx.runner.GetBaseBounds());
}

TEST(StageRunnerCamera, CurrentRectLocksTheViewWhereItIs)
{
    RunnerFixture fx { OneSection(R"J([{"type":"lockCamera","rect":"current","delay":1.0}])J") };
    fx.camera.SnapTo({ 1500.f, 224.f });
    fx.StepAt(1500.f, 59);
    const Aabb viewBefore = Aabb::FromMinSize(fx.camera.GetPositionNoShake(), { 640.f, 448.f });
    fx.StepAt(1500.f);   // tick 59 → 락은 tick 60
    fx.StepAt(1500.f);
    EXPECT_EQ(fx.runner.GetLockDepth(), 1);
    EXPECT_EQ(fx.runner.GetEffectiveBounds(), viewBefore);
    for (int i = 0; i < 100; ++i)
    {
        fx.StepAt(2400.f);
        ASSERT_EQ(Aabb::FromMinSize(fx.camera.GetPositionNoShake(), { 640.f, 448.f }), viewBefore);
    }
}

TEST(StageRunnerCamera, GlideLockMovesThenLocks)
{
    RunnerFixture fx { OneSection(R"J([{"type":"lockCamera","rect":[2470,0,3110,448],"glide":1.0}])J") };
    fx.camera.SnapTo({ 2400.f, 224.f });
    fx.StepAt(2700.f, 1);   // tick 0: 락 요청, 이동 1틱
    EXPECT_TRUE(fx.camera.IsMoving());
    EXPECT_FALSE(fx.camera.IsLocked());
    EXPECT_TRUE(fx.runner.MakeScreenWalls().bHasRight);
    fx.StepAt(2700.f, 59);
    EXPECT_FALSE(fx.camera.IsMoving());
    EXPECT_TRUE(fx.camera.IsLocked());
    EXPECT_FLOAT_EQ(fx.camera.GetViewCenter().x, 2790.f);
    EXPECT_EQ(fx.camera.GetPositionNoShake().x, 2470.f);
}

TEST(StageRunnerCamera, PermanentLockCannotBeUnlockedAndIgnoresBoundsAndCurves)
{
    const std::string json = WrapStage(R"J([
        {"id":"a","triggerX":0,"actions":[]},
        {"id":"b","triggerX":100,"actions":[
            {"type":"lockCamera","rect":"arena","permanent":true},
            {"type":"unlockCamera"},
            {"type":"setBounds","rect":[0,0,1000,448]}]},
        {"id":"c","triggerX":500,"actions":[{"type":"setBounds","rect":[0,0,2000,448]},{"type":"unlockCamera"}]}])J",
        R"J({"initialBounds":[0,0,4000,448],"boundsCurves":[{"id":"cv","from":"a","until":"c","left":0,"right":4000,"height":448,"top":[[0,0],[4000,400]]}]})J");
    RunnerFixture fx { json };
    fx.StepAt(50.f, 2);   // 곡선 활성
    EXPECT_NE(fx.runner.GetBaseBounds().min.y, 0.f);
    fx.StepAt(150.f, 2);
    EXPECT_TRUE(fx.runner.HasPermanentLock());
    EXPECT_EQ(fx.runner.GetLockDepth(), 1);                 // unlockCamera 무시
    const Aabb baseAfter = fx.runner.GetBaseBounds();
    EXPECT_NE(baseAfter, Aabb::FromMinMax({ 0.f, 0.f }, { 1000.f, 448.f }));   // setBounds 무시
    fx.StepAt(300.f, 5);                                    // 곡선 무시: 기본 경계가 더 변하지 않는다
    EXPECT_EQ(fx.runner.GetBaseBounds(), baseAfter);
    fx.StepAt(600.f, 2);                                    // 뒤 섹션의 setBounds/unlock 도 무시
    EXPECT_EQ(fx.runner.GetLockDepth(), 1);
    EXPECT_EQ(fx.runner.GetBaseBounds(), baseAfter);
    EXPECT_EQ(fx.camera.GetLockBounds(), Aabb::FromMinMax({ 100.f, 0.f }, { 740.f, 448.f }));
}

TEST(StageRunnerCamera, BoundsCurveIsActiveBetweenFromAndUntil)
{
    const std::string json = WrapStage(R"J([
        {"id":"a","triggerX":0,"actions":[]},
        {"id":"b","triggerX":1000,"actions":[]},
        {"id":"c","triggerX":3000,"actions":[{"type":"setBounds","rect":[0,0,4000,448]}]}])J",
        R"J({"initialBounds":[0,0,4000,448],"boundsCurves":[{"id":"slope","source":"playerX","from":"b","until":"c","left":100,"right":4000,"height":448,"top":[[1000,100],[2000,500]],"round":"trunc"}]})J");
    RunnerFixture fx { json };
    fx.camera.SetBounds(Aabb::FromMinMax({ 0.f, 0.f }, { 4000.f, 1000.f }));

    fx.StepAt(500.f, 3);   // from 전: 비활성
    EXPECT_FALSE(fx.runner.IsCurveActive(0));
    EXPECT_FLOAT_EQ(fx.runner.GetBaseBounds().min.y, 0.f);

    fx.StepAt(1500.f);     // b 발동 → 곡선 활성, 같은 틱에 평가
    EXPECT_TRUE(fx.runner.IsCurveActive(0));
    EXPECT_EQ(fx.runner.GetBaseBounds(), Aabb::FromMinMax({ 100.f, 300.f }, { 4000.f, 748.f }));
    EXPECT_EQ(fx.camera.GetBounds(), fx.runner.GetBaseBounds());

    fx.StepAt(1750.f);     // 매 틱 재평가
    EXPECT_FLOAT_EQ(fx.runner.GetBaseBounds().min.y, 400.f);
    fx.StepAt(1200.f);     // 뒤로 가면 곡선도 되돌아간다
    EXPECT_FLOAT_EQ(fx.runner.GetBaseBounds().min.y, 180.f);
    fx.StepAt(2900.f);     // 오른쪽 끝에서 clamp
    EXPECT_FLOAT_EQ(fx.runner.GetBaseBounds().min.y, 500.f);

    fx.StepAt(3100.f);
    EXPECT_FALSE(fx.runner.IsCurveActive(0));
    EXPECT_EQ(fx.runner.GetBaseBounds(), Aabb::FromMinMax({ 0.f, 0.f }, { 4000.f, 448.f }));
    fx.StepAt(1500.f, 3);
    EXPECT_EQ(fx.runner.GetBaseBounds(), Aabb::FromMinMax({ 0.f, 0.f }, { 4000.f, 448.f }));
}

TEST(StageRunnerCamera, Stage0SlopeMatchesTheLegacyFormula)
{
    slug::Result<StageData> result = LoadStageFromFile(slug::FilePath { SLUG_ASSET_DIR } / "Stage" / "Stage0.json");
    ASSERT_TRUE(result);
    const StageData   data  = result.Take();
    const BoundsCurve& curve = data.camera.boundsCurves.at(0);

    int exact = 0;
    int total = 0;
    for (float x = 6500.f; x <= 8800.f; x += 0.5f)
    {
        const float legacy = std::trunc(std::min(850.f, 350.f + (x - 6650.f) * 0.255f));
        const float ours   = curve.EvaluateTop(x);
        ASSERT_LE(std::abs(ours - legacy), 1.f) << "x=" << x << " ours=" << ours << " legacy=" << legacy;
        exact += ours == legacy ? 1 : 0;
        ++total;
    }
    EXPECT_GT(exact, total * 9 / 10);   // 거의 모든 점이 정확히 일치 (float 경계에서만 1 차이)

    EXPECT_FLOAT_EQ(curve.EvaluateTop(6500.f), 311.f);     // trunc(311.75)
    EXPECT_FLOAT_EQ(curve.EvaluateTop(8610.7843137f), 850.f);
    EXPECT_FLOAT_EQ(curve.EvaluateTop(9000.f), 850.f);
    EXPECT_FLOAT_EQ(curve.EvaluateTop(6000.f), 311.f);     // 앞쪽 clamp
    const Aabb bounds = curve.EvaluateBounds(7000.f);
    EXPECT_FLOAT_EQ(bounds.min.x, 3180.f);
    EXPECT_FLOAT_EQ(bounds.max.x, 14094.f);
    EXPECT_FLOAT_EQ(bounds.GetHeight(), 448.f);
}

TEST(StageRunnerCamera, CameraMoveActionReachesTargetThenResumesTracking)
{
    RunnerFixture fx { OneSection(R"J([
        {"type":"cameraMove","to":[1000,300],"seconds":0.5},
        {"type":"setBounds","rect":[0,0,4000,1000]}])J") };
    fx.camera.SnapTo({ 320.f, 224.f });
    fx.StepAt(5.f, 1);
    EXPECT_TRUE(fx.camera.IsMoving());
    fx.StepAt(5.f, 29);
    EXPECT_FALSE(fx.camera.IsMoving());
    EXPECT_FLOAT_EQ(fx.camera.GetViewCenter().x, 1000.f);
    EXPECT_FLOAT_EQ(fx.camera.GetViewCenter().y, 300.f);
}

TEST(StageRunnerCamera, FollowsPlayerForwardOnlyAndNeverScrollsBack)
{
    RunnerFixture fx { OneSection("[]", kTall) };
    float maxX = fx.camera.GetPositionNoShake().x;
    for (int i = 0; i < 600; ++i)
    {
        const float x = i < 300 ? 100.f + static_cast<float>(i) * 3.f : 1000.f - static_cast<float>(i - 300) * 3.f;
        fx.StepAt(x);
        const float camX = fx.camera.GetPositionNoShake().x;
        ASSERT_GE(camX, maxX) << i;
        maxX = std::max(maxX, camX);
    }
    EXPECT_GT(maxX, 500.f);
}

TEST(StageRunnerCamera, RunnerHashChangesWithStateAndIsReproducible)
{
    auto HashAfter = [](int _ticks) {
        RunnerFixture fx { OneSection(R"J([{"type":"banner","text":"x","seconds":1,"delay":1.0}])J") };
        fx.StepAt(5.f, _ticks);
        return fx.runner.HashState();
    };
    EXPECT_EQ(HashAfter(10), HashAfter(10));
    EXPECT_NE(HashAfter(10), HashAfter(11));
}
