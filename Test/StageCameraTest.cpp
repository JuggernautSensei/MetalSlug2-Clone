
#include <algorithm>
#include <cmath>

#include <gtest/gtest.h>

#include "Base/SlugError.h"
#include "Base/Types.h"
#include "Core/AssetPath.h"
#include "StageTestUtil.h"

constexpr const char* kTall = R"J({"initialBounds":[0,0,4000,1000]})J";

namespace
{
[[nodiscard]] String OneSection_(
    const String& _actions,
    const String& _camera = R"J({"initialBounds":[0,0,4000,448]})J")
{
    return WrapStage(R"J([{"id":"s0","triggerX":0,"actions":)J" + _actions + "}]", _camera);
}

[[nodiscard]] Camera MakeCamera_(
    const Aabb& _bounds = Aabb::FromMinMax({ 0.f, 0.f }, { 4000.f, 1000.f }))
{
    Camera camera;
    camera.SetBounds(_bounds);
    return camera;
}

[[nodiscard]] uint64_t HashAfterTicks_(
    const int _ticks)
{
    RunnerFixture fx { OneSection_(R"J([{"type":"banner","text":"x","seconds":1,"delay":1.0}])J") };
    fx.StepAt(5.f, _ticks);
    return fx.runner.HashState();
}
}   // namespace

TEST(CameraStageExt, ForwardOnlyYNeverMovesUp)
{
    Camera camera = MakeCamera_();
    camera.SetForwardOnlyY(true);
    camera.SetTarget({ 320.f, 700.f });
    camera.SnapToTarget();
    const float lowest = camera.GetViewCenter().y;
    camera.SetTarget({ 320.f, 100.f });
    for (int i = 0; i < 300; ++i)
    {
        camera.Update();
        ASSERT_GE(camera.GetViewCenter().y, lowest) << "tick " << i;
    }
}

TEST(CameraStageExt, ForwardOnlyYIsOffByDefaultAndBoundsStillWin)
{
    Camera camera = MakeCamera_();
    EXPECT_FALSE(camera.IsForwardOnlyY());
    camera.SetTarget({ 320.f, 700.f });
    camera.SnapToTarget();
    camera.SetTarget({ 320.f, 100.f });
    for (int i = 0; i < 600; ++i)
    {
        camera.Update();
    }
    EXPECT_LT(camera.GetViewCenter().y, 400.f);

    camera.SetForwardOnlyY(true);
    camera.SetBounds(Aabb::FromMinMax({ 0.f, 0.f }, { 4000.f, 448.f }));
    camera.Update();
    EXPECT_FLOAT_EQ(camera.GetViewCenter().y, 224.f);
}

TEST(CameraStageExt, BeginMoveIsSmoothstepAndTickExactThenTrackingResumes)
{
    Camera camera = MakeCamera_();
    camera.SnapTo({ 320.f, 224.f });
    camera.SetTarget({ 9999.f, 224.f });
    camera.BeginMove({ 820.f, 424.f }, 0.5f);
    EXPECT_TRUE(camera.IsMoving());

    constexpr int kTicks = 63;
    for (int k = 1; k <= kTicks; ++k)
    {
        camera.Update();
        if (k < kTicks)
        {
            const float t = static_cast<float>(k) / static_cast<float>(kTicks);
            const float s = t * t * (3.f - 2.f * t);
            ASSERT_TRUE(camera.IsMoving()) << k;
            EXPECT_NEAR(camera.GetViewCenter().x, 320.f + 500.f * s, 1e-3f) << k;
            EXPECT_NEAR(camera.GetViewCenter().y, 224.f + 200.f * s, 1e-3f) << k;
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
    EXPECT_GT(camera.GetViewCenter().x, 1500.f);
}

TEST(CameraStageExt, BeginMoveWithZeroSecondsSnapsAndMoveRespectsBounds)
{
    Camera camera = MakeCamera_();
    camera.BeginMove({ 1500.f, 300.f }, 0.f);
    EXPECT_FALSE(camera.IsMoving());
    EXPECT_FLOAT_EQ(camera.GetViewCenter().x, 1500.f);

    camera.BeginMove({ 9000.f, 300.f }, 1.f);
    for (int i = 0; i < 300 && camera.IsMoving(); ++i)
    {
        camera.Update();
        ASSERT_LE(camera.GetViewCenter().x, 3680.f + 1e-3f);
    }
    EXPECT_FALSE(camera.IsMoving());
    EXPECT_FLOAT_EQ(camera.GetViewCenter().x, 3680.f);
}

TEST(CameraStageExt, LockWithGlideMovesFirstThenLocksOnArrival)
{
    Camera camera = MakeCamera_(Aabb::FromMinMax({ 0.f, 0.f }, { 4000.f, 448.f }));
    camera.SnapTo({ 2400.f, 224.f });
    camera.LockWithGlide(Aabb::FromMinMax({ 2470.f, 0.f }, { 3110.f, 448.f }), 1.f);
    EXPECT_TRUE(camera.HasPendingLock());
    EXPECT_FALSE(camera.IsLocked());

    for (int k = 1; k <= 59; ++k)
    {
        camera.Update();
        ASSERT_FALSE(camera.IsLocked()) << k;
        const float t = static_cast<float>(k) / 60.f;
        EXPECT_NEAR(camera.GetViewCenter().x, 2400.f + 390.f * t * t * (3.f - 2.f * t), 1e-2f) << k;
    }
    camera.Update();
    EXPECT_TRUE(camera.IsLocked());
    EXPECT_FALSE(camera.HasPendingLock());
    EXPECT_FLOAT_EQ(camera.GetViewCenter().x, 2790.f);

    camera.SetTarget({ 5000.f, 224.f });
    for (int i = 0; i < 120; ++i)
    {
        camera.Update();
        ASSERT_FLOAT_EQ(camera.GetPosition().x, 2470.f);
    }
}

TEST(CameraStageExt, UnlockDuringGlideCancelsThePendingLock)
{
    Camera camera = MakeCamera_(Aabb::FromMinMax({ 0.f, 0.f }, { 4000.f, 448.f }));
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
    Camera camera = MakeCamera_(Aabb::FromMinMax({ 0.f, 0.f }, { 4000.f, 448.f }));
    camera.LockWithGlide(Aabb::FromMinMax({ 100.f, 0.f }, { 740.f, 448.f }), 0.f);
    EXPECT_TRUE(camera.IsLocked());
    EXPECT_FALSE(camera.HasPendingLock());
}

TEST(StageRunnerCamera, SetBoundsDuringLockOnlyChangesTheRestoredBounds)
{
    RunnerFixture fx { OneSection_(R"J([
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
    RunnerFixture fx { OneSection_(R"J([
        {"type":"lockCamera","rect":[0,0,1000,448]},
        {"type":"lockCamera","rect":[200,0,840,448]},
        {"type":"unlockCamera","delay":1.0},
        {"type":"unlockCamera","delay":2.0},
        {"type":"unlockCamera","delay":3.0}])J") };
    fx.StepAt(5.f);
    EXPECT_EQ(fx.runner.GetLockDepth(), 2);
    EXPECT_EQ(fx.camera.GetLockBounds(), Aabb::FromMinMax({ 200.f, 0.f }, { 840.f, 448.f }));

    fx.StepAt(5.f, 61);
    EXPECT_EQ(fx.runner.GetLockDepth(), 1);
    EXPECT_TRUE(fx.camera.IsLocked());
    EXPECT_EQ(fx.camera.GetLockBounds(), Aabb::FromMinMax({ 0.f, 0.f }, { 1000.f, 448.f }));

    fx.StepAt(5.f, 60);
    EXPECT_EQ(fx.runner.GetLockDepth(), 0);
    EXPECT_FALSE(fx.camera.IsLocked());

    fx.StepAt(5.f, 60);
    EXPECT_EQ(fx.runner.GetLockDepth(), 0);
    EXPECT_EQ(fx.runner.GetEffectiveBounds(), fx.runner.GetBaseBounds());
}

TEST(StageRunnerCamera, CurrentRectLocksTheViewWhereItIs)
{
    RunnerFixture fx { OneSection_(R"J([{"type":"lockCamera","rect":"current","delay":1.0}])J") };
    fx.camera.SnapTo({ 1500.f, 224.f });
    fx.StepAt(1500.f, 59);
    const Aabb viewBefore = Aabb::FromMinSize(fx.camera.GetPosition(), { 640.f, 448.f });
    fx.StepAt(1500.f);
    fx.StepAt(1500.f);
    EXPECT_EQ(fx.runner.GetLockDepth(), 1);
    EXPECT_EQ(fx.runner.GetEffectiveBounds(), viewBefore);
    for (int i = 0; i < 100; ++i)
    {
        fx.StepAt(2400.f);
        ASSERT_EQ(Aabb::FromMinSize(fx.camera.GetPosition(), { 640.f, 448.f }), viewBefore);
    }
}

TEST(StageRunnerCamera, GlideLockMovesThenLocks)
{
    RunnerFixture fx { OneSection_(R"J([{"type":"lockCamera","rect":[2470,0,3110,448],"glide":1.0}])J") };
    fx.camera.SnapTo({ 2400.f, 224.f });
    fx.StepAt(2700.f, 1);
    EXPECT_TRUE(fx.camera.IsMoving());
    EXPECT_FALSE(fx.camera.IsLocked());
    EXPECT_TRUE(fx.runner.MakeScreenWalls().bHasRight);
    fx.StepAt(2700.f, 59);
    EXPECT_FALSE(fx.camera.IsMoving());
    EXPECT_TRUE(fx.camera.IsLocked());
    EXPECT_FLOAT_EQ(fx.camera.GetViewCenter().x, 2790.f);
    EXPECT_EQ(fx.camera.GetPosition().x, 2470.f);
}

TEST(StageRunnerCamera, PermanentLockCannotBeUnlockedAndIgnoresBoundsAndCurves)
{
    const String  json = WrapStage(R"J([
        {"id":"a","triggerX":0,"actions":[]},
        {"id":"b","triggerX":100,"actions":[
            {"type":"lockCamera","rect":"arena","permanent":true},
            {"type":"unlockCamera"},
            {"type":"setBounds","rect":[0,0,1000,448]}]},
        {"id":"c","triggerX":500,"actions":[{"type":"setBounds","rect":[0,0,2000,448]},{"type":"unlockCamera"}]}])J",
                                   R"J({"initialBounds":[0,0,4000,448],"boundsCurves":[{"id":"cv","from":"a","until":"c","left":0,"right":4000,"height":448,"top":[[0,0],[4000,400]]}]})J");
    RunnerFixture fx { json };
    fx.StepAt(50.f, 2);
    EXPECT_NE(fx.runner.GetBaseBounds().min.y, 0.f);
    fx.StepAt(150.f, 2);
    EXPECT_TRUE(fx.runner.HasPermanentLock());
    EXPECT_EQ(fx.runner.GetLockDepth(), 1);
    const Aabb baseAfter = fx.runner.GetBaseBounds();
    EXPECT_NE(baseAfter, Aabb::FromMinMax({ 0.f, 0.f }, { 1000.f, 448.f }));
    fx.StepAt(300.f, 5);
    EXPECT_EQ(fx.runner.GetBaseBounds(), baseAfter);
    fx.StepAt(600.f, 2);
    EXPECT_EQ(fx.runner.GetLockDepth(), 1);
    EXPECT_EQ(fx.runner.GetBaseBounds(), baseAfter);
    EXPECT_EQ(fx.camera.GetLockBounds(), Aabb::FromMinMax({ 100.f, 0.f }, { 740.f, 448.f }));
}

TEST(StageRunnerCamera, BoundsCurveIsActiveBetweenFromAndUntil)
{
    const String  json = WrapStage(R"J([
        {"id":"a","triggerX":0,"actions":[]},
        {"id":"b","triggerX":1000,"actions":[]},
        {"id":"c","triggerX":3000,"actions":[{"type":"setBounds","rect":[0,0,4000,448]}]}])J",
                                   R"J({"initialBounds":[0,0,4000,448],"boundsCurves":[{"id":"slope","source":"playerX","from":"b","until":"c","left":100,"right":4000,"height":448,"top":[[1000,100],[2000,500]],"round":"trunc"}]})J");
    RunnerFixture fx { json };
    fx.camera.SetBounds(Aabb::FromMinMax({ 0.f, 0.f }, { 4000.f, 1000.f }));

    fx.StepAt(500.f, 3);
    EXPECT_FALSE(fx.runner.IsCurveActive(0));
    EXPECT_FLOAT_EQ(fx.runner.GetBaseBounds().min.y, 0.f);

    fx.StepAt(1500.f);
    EXPECT_TRUE(fx.runner.IsCurveActive(0));
    EXPECT_EQ(fx.runner.GetBaseBounds(), Aabb::FromMinMax({ 100.f, 300.f }, { 4000.f, 748.f }));
    EXPECT_EQ(fx.camera.GetBounds(), fx.runner.GetBaseBounds());

    fx.StepAt(1750.f);
    EXPECT_FLOAT_EQ(fx.runner.GetBaseBounds().min.y, 400.f);
    fx.StepAt(1200.f);
    EXPECT_FLOAT_EQ(fx.runner.GetBaseBounds().min.y, 180.f);
    fx.StepAt(2900.f);
    EXPECT_FLOAT_EQ(fx.runner.GetBaseBounds().min.y, 500.f);

    fx.StepAt(3100.f);
    EXPECT_FALSE(fx.runner.IsCurveActive(0));
    EXPECT_EQ(fx.runner.GetBaseBounds(), Aabb::FromMinMax({ 0.f, 0.f }, { 4000.f, 448.f }));
    fx.StepAt(1500.f, 3);
    EXPECT_EQ(fx.runner.GetBaseBounds(), Aabb::FromMinMax({ 0.f, 0.f }, { 4000.f, 448.f }));
}

TEST(StageRunnerCamera, Stage0SlopeMatchesTheLegacyFormula)
{
    Result<StageData> result = LoadStageFromFile(FindAssetRoot() / "Stage" / "Stage0.json");
    ASSERT_TRUE(result);
    const StageData    data  = result.Take();
    const BoundsCurve& curve = data.camera.boundsCurves.at(0);

    int exact = 0;
    int total = 0;
    for (float x = 6500.f; x <= 8800.f; x += 0.5f)
    {
        const float legacy = std::trunc(Min(350.f + (x - 6650.f) * 0.255f, 850.f));
        const float ours   = curve.EvaluateTop(x);
        ASSERT_LE(Abs(ours - legacy), 1.f) << "x=" << x << " ours=" << ours << " legacy=" << legacy;
        exact += ours == legacy ? 1 : 0;
        ++total;
    }
    EXPECT_GT(exact, total * 9 / 10);

    EXPECT_FLOAT_EQ(curve.EvaluateTop(6500.f), 311.f);
    EXPECT_FLOAT_EQ(curve.EvaluateTop(8610.7843137f), 850.f);
    EXPECT_FLOAT_EQ(curve.EvaluateTop(9000.f), 850.f);
    EXPECT_FLOAT_EQ(curve.EvaluateTop(6000.f), 311.f);
    const Aabb bounds = curve.EvaluateBounds(7000.f);
    EXPECT_FLOAT_EQ(bounds.min.x, 3180.f);
    EXPECT_FLOAT_EQ(bounds.max.x, 14094.f);
    EXPECT_FLOAT_EQ(bounds.GetHeight(), 448.f);
}

TEST(StageRunnerCamera, CameraMoveActionReachesTargetThenResumesTracking)
{
    RunnerFixture fx { OneSection_(R"J([
        {"type":"cameraMove","to":[1000,300],"seconds":0.5},
        {"type":"setBounds","rect":[0,0,4000,1000]}])J") };
    fx.camera.SnapTo({ 320.f, 224.f });
    fx.StepAt(5.f, 1);
    EXPECT_TRUE(fx.camera.IsMoving());
    for (int i = 0; i < 200 && fx.camera.IsMoving(); ++i)
    {
        fx.StepAt(5.f, 1);
    }
    EXPECT_FALSE(fx.camera.IsMoving());
    EXPECT_FLOAT_EQ(fx.camera.GetViewCenter().x, 1000.f);
    EXPECT_FLOAT_EQ(fx.camera.GetViewCenter().y, 300.f);
}

TEST(StageRunnerCamera, FollowsPlayerForwardOnlyAndNeverScrollsBack)
{
    RunnerFixture fx { OneSection_("[]", kTall) };
    float         maxX = fx.camera.GetPosition().x;
    for (int i = 0; i < 600; ++i)
    {
        const float x = i < 300 ? 100.f + static_cast<float>(i) * 3.f : 1000.f - static_cast<float>(i - 300) * 3.f;
        fx.StepAt(x);
        const float camX = fx.camera.GetPosition().x;
        ASSERT_GE(camX, maxX) << i;
        maxX = Max(camX, maxX);
    }
    EXPECT_GT(maxX, 500.f);
}

TEST(StageRunnerCamera, RunnerHashChangesWithStateAndIsReproducible)
{
    EXPECT_EQ(HashAfterTicks_(10), HashAfterTicks_(10));
    EXPECT_NE(HashAfterTicks_(10), HashAfterTicks_(11));
}
