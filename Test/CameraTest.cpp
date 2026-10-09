#include <algorithm>
#include "Base/Types.h"
#include "Base/Math.h"
#include <cmath>

#include <gtest/gtest.h>

#include "Render/Camera.h"

using namespace slug;

namespace
{
bool IsIntegral(
    const float _value)
{
    return _value == std::floor(_value);
}

Camera MakeLevelCamera()
{
    Camera camera;
    camera.SetBounds(Aabb::FromMinMax({ 0.f, 0.f }, { 4000.f, 448.f }));
    camera.SetTarget({ 320.f, 224.f });
    camera.SnapToTarget();
    return camera;
}
}

TEST(Camera, OutputIsAlwaysIntegralWhileFollowingFractionalTarget)
{
    Camera camera = MakeLevelCamera();
    for (int i = 0; i < 600; ++i)
    {
        camera.SetTarget({ 320.f + static_cast<float>(i) * 3.37f, 224.f + std::sin(static_cast<float>(i) * 0.05f) * 3.1f });
        camera.Update();
        const Vec2 position = camera.GetPosition();
        ASSERT_TRUE(IsIntegral(position.x)) << "tick " << i << " x=" << position.x;
        ASSERT_TRUE(IsIntegral(position.y)) << "tick " << i << " y=" << position.y;
    }
}

TEST(Camera, FollowConvergesToTargetWithinDeadzone)
{
    Camera camera = MakeLevelCamera();
    camera.SetTarget({ 1500.f, 224.f });
    for (int i = 0; i < 600; ++i)
    {
        camera.Update();
    }

    const CameraTuning& tuning = camera.GetTuning();
    EXPECT_LE(std::abs(camera.GetViewCenter().x - 1500.f), tuning.deadzoneHalf.x + 0.01f);
    const Vec2 settled = camera.GetPosition();
    for (int i = 0; i < 120; ++i)
    {
        camera.Update();
    }
    EXPECT_EQ(camera.GetPosition(), settled);
}

TEST(Camera, HorizontalFollowIsInstantAndVerticalIsSmooth)
{
    Camera camera = MakeLevelCamera();
    camera.SetTarget({ 1000.f, 224.f });
    camera.Update();
    EXPECT_FLOAT_EQ(camera.GetViewCenter().x, 1000.f - camera.GetTuning().deadzoneHalf.x);

    camera.SetTarget({ 1000.f, 300.f });
    float previous = camera.GetViewCenter().y;
    camera.Update();
    EXPECT_LT(camera.GetViewCenter().y, 300.f - camera.GetTuning().deadzoneHalf.y);
    for (int i = 0; i < 400; ++i)
    {
        camera.Update();
        ASSERT_GE(camera.GetViewCenter().y, previous) << "tick " << i;
        ASSERT_LE(camera.GetViewCenter().y, 300.f - camera.GetTuning().deadzoneHalf.y + 0.01f);
        previous = camera.GetViewCenter().y;
    }
}

TEST(Camera, TargetInsideDeadzoneDoesNotMoveTheCamera)
{
    Camera             camera = MakeLevelCamera();
    const Vec2 start  = camera.GetPosition();
    const float        dz     = camera.GetTuning().deadzoneHalf.x;

    for (int i = 0; i < 60; ++i)
    {
        camera.SetTarget({ 320.f + dz * std::sin(static_cast<float>(i) * 0.3f) * 0.9f, 224.f });
        camera.Update();
    }
    EXPECT_EQ(camera.GetPosition(), start);
}

TEST(Camera, ClampsToBoundsOnBothEnds)
{
    Camera camera = MakeLevelCamera();

    camera.SetTarget({ 100000.f, 224.f });
    for (int i = 0; i < 2000; ++i)
    {
        camera.Update();
    }
    EXPECT_FLOAT_EQ(camera.GetPosition().x, 4000.f - 640.f);

    camera.SetTarget({ -100000.f, 224.f });
    for (int i = 0; i < 4000; ++i)
    {
        camera.Update();
    }
    EXPECT_FLOAT_EQ(camera.GetPosition().x, 0.f);
}

TEST(Camera, NeverShowsOutsideBoundsAtAnyTick)
{
    Camera camera = MakeLevelCamera();
    for (int i = 0; i < 1500; ++i)
    {
        camera.SetTarget({ static_cast<float>((i * 37) % 5000) - 500.f, static_cast<float>((i * 11) % 700) - 100.f });
        camera.Update();
        const Aabb view = camera.GetViewRect();
        ASSERT_GE(view.min.x, 0.f);
        ASSERT_GE(view.min.y, 0.f);
        ASSERT_LE(view.max.x, 4000.f);
        ASSERT_LE(view.max.y, 448.f);
    }
}

TEST(Camera, BoundsSmallerThanViewAreCentered)
{
    Camera camera;
    camera.SetBounds(Aabb::FromMinMax({ 0.f, 0.f }, { 400.f, 300.f }));
    camera.SetTarget({ 200.f, 150.f });
    camera.SnapToTarget();
    EXPECT_FLOAT_EQ(camera.GetPosition().x, -120.f);
    EXPECT_FLOAT_EQ(camera.GetPosition().y, -74.f);
}

TEST(Camera, ForwardOnlyNeverMovesBackward)
{
    Camera camera = MakeLevelCamera();
    camera.SetForwardOnly(true);

    float maxX = camera.GetPositionNoShake().x;
    for (int i = 0; i < 900; ++i)
    {
        const float t = static_cast<float>(i);
        camera.SetTarget({ 320.f + (i < 400 ? t * 3.f : 1200.f - (t - 400.f) * 3.f), 224.f });
        camera.Update();

        const float x = camera.GetPositionNoShake().x;
        ASSERT_GE(x, maxX) << "tick " << i;
        maxX = std::max(maxX, x);
    }
    EXPECT_GT(maxX, 700.f);
}

TEST(Camera, WithoutForwardOnlyCameraFollowsBackward)
{
    Camera camera = MakeLevelCamera();
    camera.SetTarget({ 2000.f, 224.f });
    for (int i = 0; i < 600; ++i)
    {
        camera.Update();
    }
    const float forward = camera.GetPosition().x;

    camera.SetTarget({ 1000.f, 224.f });
    for (int i = 0; i < 600; ++i)
    {
        camera.Update();
    }
    EXPECT_LT(camera.GetPosition().x, forward - 500.f);
}

TEST(Camera, LockConfinesTheViewAndUnlockRestoresFollowing)
{
    Camera camera = MakeLevelCamera();
    camera.SetTarget({ 1000.f, 224.f });
    camera.SnapToTarget();

    const float lockLeft = camera.GetPosition().x;
    camera.Lock(Aabb::FromMinMax({ lockLeft, 0.f }, { lockLeft + 640.f, 448.f }));
    EXPECT_TRUE(camera.IsLocked());

    camera.SetTarget({ 3000.f, 224.f });
    for (int i = 0; i < 600; ++i)
    {
        camera.Update();
        ASSERT_FLOAT_EQ(camera.GetPosition().x, lockLeft);
    }

    camera.Unlock();
    for (int i = 0; i < 900; ++i)
    {
        camera.Update();
    }
    EXPECT_GT(camera.GetPosition().x, lockLeft + 1000.f);
}

TEST(Camera, LockWinsOverForwardOnly)
{
    Camera camera = MakeLevelCamera();
    camera.SetForwardOnly(true);
    camera.SetTarget({ 2000.f, 224.f });
    for (int i = 0; i < 600; ++i)
    {
        camera.Update();
    }
    ASSERT_GT(camera.GetPosition().x, 1000.f);

    camera.Lock(Aabb::FromMinMax({ 100.f, 0.f }, { 740.f, 448.f }));
    camera.Update();
    EXPECT_FLOAT_EQ(camera.GetPosition().x, 100.f);
}

TEST(Camera, SnapToTargetIsImmediateAndClamped)
{
    Camera camera = MakeLevelCamera();
    camera.SetTarget({ 2500.f, 224.f });
    camera.SnapToTarget();
    EXPECT_FLOAT_EQ(camera.GetPosition().x, 2500.f - 320.f);

    camera.SetTarget({ 50.f, 224.f });
    camera.SnapToTarget();
    EXPECT_FLOAT_EQ(camera.GetPosition().x, 0.f);
}

TEST(Camera, WorldScreenRoundTripAndVisibility)
{
    Camera camera = MakeLevelCamera();
    camera.SetTarget({ 1000.f, 224.f });
    camera.SnapToTarget();

    const Vec2 world  = { 1234.5f, 100.25f };
    const Vec2 screen = camera.WorldToScreen(world);
    EXPECT_EQ(camera.ScreenToWorld(screen), world);
    EXPECT_FLOAT_EQ(screen.x, 1234.5f - camera.GetPosition().x);

    EXPECT_TRUE(camera.IsVisible(Aabb::FromMinMax({ 1000.f, 100.f }, { 1010.f, 110.f })));
    EXPECT_FALSE(camera.IsVisible(Aabb::FromMinMax({ 3000.f, 100.f }, { 3010.f, 110.f })));
    const Aabb view        = camera.GetViewRect();
    const Aabb justOutside = Aabb::FromMinMax({ view.max.x + 5.f, 100.f }, { view.max.x + 15.f, 110.f });
    EXPECT_FALSE(camera.IsVisible(justOutside));
    EXPECT_TRUE(camera.IsVisible(justOutside, 8.f));
}

TEST(Camera, TargetOffsetLooksAhead)
{
    CameraTuning tuning;
    tuning.targetOffset = { 80.f, 0.f };
    Camera camera { tuning };
    camera.SetBounds(Aabb::FromMinMax({ 0.f, 0.f }, { 4000.f, 448.f }));
    camera.SetTarget({ 1000.f, 224.f });
    camera.SnapToTarget();
    EXPECT_FLOAT_EQ(camera.GetViewCenter().x, 1080.f);
}

TEST(Camera, ZeroSmoothTimeFollowsInstantly)
{
    CameraTuning tuning;
    tuning.smoothTime   = 0.f;
    tuning.deadzoneHalf = { 0.f, 0.f };
    Camera camera { tuning };
    camera.SetBounds(Aabb::FromMinMax({ 0.f, 0.f }, { 4000.f, 448.f }));
    camera.SetTarget({ 1500.f, 224.f });
    camera.Update();
    EXPECT_FLOAT_EQ(camera.GetViewCenter().x, 1500.f);
}

TEST(Camera, ShakeIsDeterministicDecaysAndStaysInBounds)
{
    auto Run = []() {
        Camera camera = MakeLevelCamera();
        camera.SetTarget({ 2000.f, 224.f });
        camera.SnapToTarget();
        camera.Shake(6.f, 0.5f);

        slug::Vector<Vec2> positions;
        for (int i = 0; i < 60; ++i)
        {
            camera.Update();
            positions.push_back(camera.GetPosition());
        }
        return positions;
    };

    const slug::Vector<Vec2> a = Run();
    const slug::Vector<Vec2> b = Run();
    ASSERT_EQ(a.size(), b.size());
    bool bMoved = false;
    for (size_t i = 0; i < a.size(); ++i)
    {
        ASSERT_EQ(a[i], b[i]) << i;
        ASSERT_TRUE(IsIntegral(a[i].x) && IsIntegral(a[i].y));
        ASSERT_GE(a[i].y, 0.f);   // 위쪽 경계 밖이 보이지 않는다
        bMoved = bMoved || a[i].x != 1680.f;
    }
    EXPECT_TRUE(bMoved);
    EXPECT_FLOAT_EQ(a.back().x, 1680.f);
    EXPECT_FLOAT_EQ(a.back().y, 0.f);
}

TEST(Camera, StopShakeRestoresImmediately)
{
    Camera camera = MakeLevelCamera();
    camera.SetTarget({ 2000.f, 224.f });
    camera.SnapToTarget();
    camera.Shake(10.f, 5.f);
    camera.Update();
    camera.StopShake();
    EXPECT_EQ(camera.GetPosition(), camera.GetPositionNoShake());
}

TEST(Camera, DeterministicAcrossIdenticalRuns)
{
    auto Run = []() {
        Camera camera = MakeLevelCamera();
        camera.SetForwardOnly(true);
        slug::Vector<Vec2> out;
        for (int i = 0; i < 600; ++i)
        {
            camera.SetTarget({ 320.f + static_cast<float>(i) * 2.7f, 224.f });
            camera.Update();
            out.push_back(camera.GetPosition());
        }
        return out;
    };
    EXPECT_EQ(Run(), Run());
}
