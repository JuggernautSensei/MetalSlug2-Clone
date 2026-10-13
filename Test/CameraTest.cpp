// 카메라의 추적, 데드존, 경계, 흔들림, 정수 출력과 실제 스테이지 봇 주행 시 끊김 없음을 검증한다.

#include <gtest/gtest.h>

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <cstdio>
#include <memory>

#include "Base/Math.h"
#include "Base/Types.h"
#include "Core/AssetPath.h"
#include "Core/Input.h"
#include "Game/Player/PlayerController.h"
#include "Game/SimRunner.h"
#include "Game/Stage/StageRunner.h"
#include "Game/Stage/StageState.h"
#include "Game/Stage/StageWorldHost.h"
#include "Render/Camera.h"
#include "StageTestUtil.h"

namespace
{
[[nodiscard]] bool IsIntegral_(
    const float _value)
{
    return _value == std::floor(_value);
}

[[nodiscard]] Camera MakeLevelCamera_()
{
    Camera camera;
    camera.SetBounds(Aabb::FromMinMax({ 0.f, 0.f }, { 4000.f, 448.f }));
    camera.SetTarget({ 320.f, 224.f });
    camera.SnapToTarget();
    return camera;
}

[[nodiscard]] Vector<Vec2> RunForwardOnly_()
{
    Camera camera = MakeLevelCamera_();
    camera.SetForwardOnly(true);
    Vector<Vec2> out;
    for (int i = 0; i < 600; ++i)
    {
        camera.SetTarget({ 320.f + static_cast<float>(i) * 2.7f, 224.f });
        camera.Update();
        out.push_back(camera.GetPosition());
    }
    return out;
}
}   // namespace

TEST(Camera, OutputIsAlwaysIntegralWhileFollowingFractionalTarget)
{
    Camera camera = MakeLevelCamera_();
    for (int i = 0; i < 600; ++i)
    {
        camera.SetTarget({ 320.f + static_cast<float>(i) * 3.37f, 224.f + std::sin(static_cast<float>(i) * 0.05f) * 3.1f });
        camera.Update();
        const Vec2 position = camera.GetPosition();
        ASSERT_TRUE(IsIntegral_(position.x)) << "tick " << i << " x=" << position.x;
        ASSERT_TRUE(IsIntegral_(position.y)) << "tick " << i << " y=" << position.y;
    }
}

TEST(Camera, FollowConvergesToTargetWithinDeadzone)
{
    Camera camera = MakeLevelCamera_();
    camera.SetTarget({ 1500.f, 224.f });
    for (int i = 0; i < 600; ++i)
    {
        camera.Update();
    }

    const CameraTuning& tuning = camera.GetTuning();
    EXPECT_LE(Abs(camera.GetViewCenter().x - 1500.f), tuning.deadzoneHalf.x + 0.01f);
    const Vec2 settled = camera.GetPosition();
    for (int i = 0; i < 120; ++i)
    {
        camera.Update();
    }
    EXPECT_EQ(camera.GetPosition(), settled);
}

TEST(Camera, HorizontalGlidesWithoutSnapAndVerticalIsSmooth)
{
    Camera camera = MakeLevelCamera_();
    camera.SetTarget({ 1000.f, 224.f });
    camera.Update();
    EXPECT_GT(camera.GetViewCenter().x, 320.f);
    EXPECT_LE(camera.GetViewCenter().x - 320.f, 12.f);

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
    Camera      camera = MakeLevelCamera_();
    const Vec2  start  = camera.GetPosition();
    const float dz     = camera.GetTuning().deadzoneHalf.x;

    for (int i = 0; i < 60; ++i)
    {
        camera.SetTarget({ 320.f + dz * std::sin(static_cast<float>(i) * 0.3f) * 0.9f, 224.f });
        camera.Update();
    }
    EXPECT_EQ(camera.GetPosition(), start);
}

TEST(Camera, ClampsToBoundsOnBothEnds)
{
    Camera camera = MakeLevelCamera_();

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
    Camera camera = MakeLevelCamera_();
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
    Camera camera = MakeLevelCamera_();
    camera.SetForwardOnly(true);

    float maxX = camera.GetPosition().x;
    for (int i = 0; i < 900; ++i)
    {
        const float t = static_cast<float>(i);
        camera.SetTarget({ 320.f + (i < 400 ? t * 3.f : 1200.f - (t - 400.f) * 3.f), 224.f });
        camera.Update();

        const float x = camera.GetPosition().x;
        ASSERT_GE(x, maxX) << "tick " << i;
        maxX = Max(x, maxX);
    }
    EXPECT_GT(maxX, 700.f);
}

TEST(Camera, WithoutForwardOnlyCameraFollowsBackward)
{
    Camera camera = MakeLevelCamera_();
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
    Camera camera = MakeLevelCamera_();
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
    Camera camera = MakeLevelCamera_();
    camera.SetForwardOnly(true);
    camera.SetTarget({ 2000.f, 224.f });
    for (int i = 0; i < 600; ++i)
    {
        camera.Update();
    }
    ASSERT_GT(camera.GetPosition().x, 1000.f);

    camera.Lock(Aabb::FromMinMax({ 100.f, 0.f }, { 740.f, 448.f }));
    for (int i = 0; i < 300; ++i)
    {
        camera.Update();
    }
    EXPECT_FLOAT_EQ(camera.GetPosition().x, 100.f);
}

TEST(Camera, SnapToTargetIsImmediateAndClamped)
{
    Camera camera = MakeLevelCamera_();
    camera.SetTarget({ 2500.f, 224.f });
    camera.SnapToTarget();
    EXPECT_FLOAT_EQ(camera.GetPosition().x, 2500.f - 320.f);

    camera.SetTarget({ 50.f, 224.f });
    camera.SnapToTarget();
    EXPECT_FLOAT_EQ(camera.GetPosition().x, 0.f);
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

TEST(Camera, DeterministicAcrossIdenticalRuns)
{
    EXPECT_EQ(RunForwardOnly_(), RunForwardOnly_());
}

TEST(Camera, ScrollRampsUpWhilePlayerRunsAndStopsInstantlyWhenPlayerStops)
{
    Camera      camera = MakeLevelCamera_();
    const float half   = camera.GetTuning().deadzoneHalf.x;
    float       x      = 320.f + half + 1.f;
    camera.SetTarget({ x, 224.f });
    camera.Update();

    float previousCenter = camera.GetViewCenter().x;
    float firstStep      = 0.f;
    float lastStep       = 0.f;
    for (int i = 0; i < 40; ++i)
    {
        x += 4.17f;
        camera.SetTarget({ x, 224.f });
        camera.Update();
        const float step = camera.GetViewCenter().x - previousCenter;
        previousCenter   = camera.GetViewCenter().x;
        if (i == 0)
        {
            firstStep = step;
        }
        lastStep = step;
    }
    EXPECT_LT(firstStep, 2.f);
    EXPECT_NEAR(lastStep, 4.17f, 0.05f);

    // 플레이어가 멈추면 감속하며 멈춘다 (한 프레임에 멈추지 않고, 플레이어보다 앞서 나가지 않는다)
    camera.Update();
    const float stopStep = camera.GetViewCenter().x - previousCenter;
    EXPECT_GT(stopStep, 0.f);
    EXPECT_LT(stopStep, lastStep);
    for (int i = 0; i < 200; ++i)
    {
        camera.Update();
    }
    EXPECT_LE(camera.GetViewCenter().x, x - half + 0.01f);
}

// StageIntegrationTest.cpp 의 IntegrationSim::BotStep 과 같은 봇 (실제 스테이지 구동용)
class CameraBotSim
{
public:
    CameraBotSim()
    {
        Scoped<StageState> pOwned = MakeScoped<StageState>();
        m_pState                  = pOwned.GetPtr();
        m_runner                  = MakeScoped<SimRunner>(Scoped<IGameState> { pOwned.Release() });
    }

    [[nodiscard]] StageState&       State() { return *m_pState; }
    [[nodiscard]] const IGameState* Current() const { return m_runner->GetState(); }
    [[nodiscard]] const IGameState* StagePtr() const { return m_pState; }

    void BotStep(
        const bool _bRight,
        const bool _bShoot)
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
        m_frame = {};
        m_frame.Set(eAction::Right, bRight);
        m_frame.Set(eAction::Left, m_backTicks > 0);
        m_frame.Set(eAction::Shot, _bShoot && (m_botTick % 24) < 3);
        m_frame.Set(eAction::Jump, bJump);
        m_runner->Step(m_frame);
        ++m_botTick;
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

    InputFrame        m_frame      = {};
    StageState*       m_pState     = nullptr;
    uint64_t          m_botTick    = 0;
    float             m_lastX      = -1.f;
    int               m_stuckTicks = 0;
    int               m_jumpTicks  = 0;
    int               m_backTicks  = 0;
    Scoped<SimRunner> m_runner     = nullptr;
};

TEST(CameraRegression, RealStageBotRunHasNoJerkOverFirst5000Ticks)
{
    CameraBotSim  sim;
    const Camera& cam         = sim.State().GetCamera();
    Vec2          prevF       = cam.GetViewCenter();
    Vec2          prevI       = cam.GetPosition();
    float         prevDx      = 0.f;
    float         prevDy      = 0.f;
    float         prevDxI     = 0.f;
    float         maxStep     = 0.f;
    float         maxAccel    = 0.f;
    float         maxAccelY   = 0.f;
    float         maxIntStep  = 0.f;
    float         maxIntAccel = 0.f;
    int           ticks       = 0;
    for (int i = 0; i < 5000 && sim.Current() == sim.StagePtr(); ++i)
    {
        sim.BotStep(true, true);
        if (sim.Current() != sim.StagePtr())
        {
            break;
        }
        const Vec2  f   = cam.GetViewCenter();
        const Vec2  pix = cam.GetPosition();
        const float dx  = f.x - prevF.x;
        const float dy  = f.y - prevF.y;
        const float dxI = pix.x - prevI.x;
        const float dyI = pix.y - prevI.y;
        if (i >= 120)
        {
            maxStep     = Max(maxStep, Max(Abs(dx), Abs(dy)));
            maxAccel    = Max(maxAccel, Abs(dx - prevDx));
            maxAccelY   = Max(maxAccelY, Abs(dy - prevDy));
            maxIntStep  = Max(maxIntStep, Max(Abs(dxI), Abs(dyI)));
            maxIntAccel = Max(maxIntAccel, Abs(dxI - prevDxI));
            EXPECT_LE(Abs(dx), 12.f) << "tick " << i;
            EXPECT_LE(Abs(dy), 12.f) << "tick " << i;
            EXPECT_LE(Abs(dx - prevDx), 1.5f) << "tick " << i;
            EXPECT_LE(Abs(dy - prevDy), 1.5f) << "tick " << i;
            EXPECT_LE(Abs(dxI), 12.f) << "tick " << i;
        }
        prevDx  = dx;
        prevDy  = dy;
        prevDxI = dxI;
        prevF   = f;
        prevI   = pix;
        ++ticks;
    }
    std::printf("[cam-regression] ticks=%d float: maxStep=%.2f maxAccelX=%.2f maxAccelY=%.2f | int: maxStep=%.0f maxAccelX=%.0f\n", ticks, maxStep, maxAccel, maxAccelY, maxIntStep, maxIntAccel);
    EXPECT_GE(ticks, 1000);
}

TEST(Camera, FollowStepsNeverJumpAndForwardOnlyNeverTwitchesOnReverse)
{
    Camera camera = MakeLevelCamera_();
    camera.SetForwardOnly(true);
    camera.SetTarget({ 320.f, 224.f });
    camera.SnapToTarget();

    float focus      = 320.f;
    float prevCenter = camera.GetViewCenter().x;
    float prevStep   = 0.f;
    for (int i = 0; i < 300; ++i)
    {
        if (i < 100)
        {
            focus += 4.17f;
        }
        else if (i < 130)
        {
            focus += 0.f;
        }
        else if (i < 170)
        {
            focus -= 4.f;
        }
        else
        {
            focus += 2.5f;
        }
        camera.SetTarget({ focus, 224.f });
        camera.Update();

        const float center = camera.GetViewCenter().x;
        const float step   = center - prevCenter;
        ASSERT_GE(step, -1e-4f) << "tick " << i;
        ASSERT_LE(step, Max(prevStep * 1.5f, 1.f) + 1e-4f) << "tick " << i;
        prevCenter = center;
        prevStep   = step;
    }
}

TEST(Camera, StopDeceleratesMonotonicallyAndNeverOvershoots)
{
    Camera camera = MakeLevelCamera_();
    camera.SetForwardOnly(true);
    const float half = camera.GetTuning().deadzoneHalf.x;
    camera.SetTarget({ 320.f, 224.f });
    camera.SnapToTarget();

    float focus = 320.f;
    for (int i = 0; i < 120; ++i)
    {
        focus += 4.17f;
        camera.SetTarget({ focus, 224.f });
        camera.Update();
    }

    float prevCenter  = camera.GetViewCenter().x;
    float prevStep    = 1e9f;
    int   movingTicks = 0;
    for (int i = 0; i < 200; ++i)
    {
        camera.SetTarget({ focus, 224.f });
        camera.Update();
        const float center = camera.GetViewCenter().x;
        const float step   = center - prevCenter;
        prevCenter         = center;
        ASSERT_GE(step, -1e-4f) << "tick " << i;
        ASSERT_LE(step, prevStep + 1e-4f) << "tick " << i;
        ASSERT_LE(center, focus - half + 1e-3f) << "tick " << i;
        if (step > 1e-4f)
        {
            ++movingTicks;
        }
        prevStep = step;
    }
    EXPECT_FLOAT_EQ(prevStep, 0.f);
    EXPECT_GE(movingTicks, 2);
}

TEST(Camera, UnlockAfterLockGlidesWithCappedStepAndSettlesInTime)
{
    Camera camera = MakeLevelCamera_();
    camera.SetTarget({ 1000.f, 224.f });
    camera.SnapToTarget();
    camera.Lock(Aabb::FromMinMax({ 680.f, 0.f }, { 1320.f, 448.f }));
    camera.SetTarget({ 1400.f, 224.f });
    for (int i = 0; i < 60; ++i)
    {
        camera.Update();
        ASSERT_FLOAT_EQ(camera.GetViewCenter().x, 1000.f) << "tick " << i;
    }

    camera.Unlock();
    const float goal    = 1400.f - camera.GetTuning().deadzoneHalf.x;
    float       prev    = 1000.f;
    float       maxStep = 0.f;
    int         ticks   = -1;
    for (int i = 0; i < 300; ++i)
    {
        camera.Update();
        const float center = camera.GetViewCenter().x;
        maxStep            = Max(maxStep, Abs(center - prev));
        prev               = center;
        if (ticks < 0 && Abs(goal - center) < 1.f)
        {
            ticks = i + 1;
        }
    }
    EXPECT_LE(maxStep, 12.f);
    EXPECT_GE(ticks, 30);
    EXPECT_LE(ticks, 60);
    EXPECT_NEAR(camera.GetViewCenter().x, goal, 0.5f);
}

TEST(Camera, SetBoundsChangeMovesCameraAtMost2_5PerTick)
{
    Camera camera = MakeLevelCamera_();
    camera.SetTarget({ 1000.f, 224.f });
    camera.SnapToTarget();
    camera.SetBounds(Aabb::FromMinMax({ 0.f, 100.f }, { 4000.f, 548.f }));

    Vec2  prev    = camera.GetViewCenter();
    float maxStep = 0.f;
    for (int i = 0; i < 150; ++i)
    {
        camera.Update();
        const Vec2 center = camera.GetViewCenter();
        maxStep           = Max(maxStep, Max(Abs(center.x - prev.x), Abs(center.y - prev.y)));
        prev              = center;
    }
    EXPECT_LE(maxStep, 2.5f);
    EXPECT_NEAR(camera.GetViewCenter().y, 324.f, 0.01f);
}

TEST(Camera, LockWhileCameraOutsideGlidesInInsteadOfJumping)
{
    Camera camera = MakeLevelCamera_();
    camera.SetTarget({ 2000.f, 224.f });
    camera.SnapToTarget();
    camera.Lock(Aabb::FromMinMax({ 100.f, 0.f }, { 740.f, 448.f }));

    float prev    = camera.GetViewCenter().x;
    float maxStep = 0.f;
    for (int i = 0; i < 300; ++i)
    {
        camera.Update();
        maxStep = Max(maxStep, Abs(camera.GetViewCenter().x - prev));
        prev    = camera.GetViewCenter().x;
    }
    EXPECT_LE(maxStep, 12.f);
    EXPECT_FLOAT_EQ(camera.GetViewCenter().x, 420.f);
}

TEST(Camera, ForwardOnlyNeverMovesBackwardWhenBoundsShrink)
{
    Camera camera = MakeLevelCamera_();
    camera.SetForwardOnly(true);
    camera.SetTarget({ 2000.f, 224.f });
    camera.SnapToTarget();

    camera.SetBounds(Aabb::FromMinMax({ 0.f, 0.f }, { 1900.f, 448.f }));
    float prev = camera.GetViewCenter().x;
    for (int i = 0; i < 200; ++i)
    {
        camera.Update();
        ASSERT_GE(camera.GetViewCenter().x, prev) << "tick " << i;
        prev = camera.GetViewCenter().x;
    }
}

// 자동 전진 (보스 스크롤): 하한 (AddScrollFloor) 은 카메라를 오른쪽으로만 민다. 정지 target 이어도 하한까지 간다.
TEST(Camera, ScrollFloorDrivesTheViewRightEvenWithoutAPlayerPush)
{
    Camera camera = MakeLevelCamera_();
    camera.SetTarget({ 1000.f, 224.f });
    camera.SnapToTarget();
    const float x0 = camera.GetViewCenter().x;
    for (int i = 0; i < 60; ++i)
    {
        camera.AddScrollFloor(0.3f);   // 18 px/s
        camera.Update(kStepSec);
    }
    EXPECT_TRUE(camera.HasScrollFloor());
    EXPECT_NEAR(camera.GetScrollFloorX(), x0 + 18.f + 0.f, 0.01f) << "하한은 처음 중심 + 누적 전진량";
    EXPECT_GE(camera.GetViewCenter().x, camera.GetScrollFloorX() - 0.01f);
    EXPECT_NEAR(camera.GetViewCenter().x - x0, 18.f, 1.5f) << "1 s 동안 18 px 전진";
}

TEST(Camera, ScrollFloorNeverMovesTheViewPastTheLockRight)
{
    Camera camera = MakeLevelCamera_();
    camera.Lock(Aabb::FromMinMax({ 1000.f, 0.f }, { 1700.f, 448.f }));
    camera.SetTarget({ 1000.f, 224.f });
    camera.SnapToTarget();
    for (int i = 0; i < 60 * 60; ++i)
    {
        camera.AddScrollFloor(0.3f);
        camera.Update(kStepSec);
    }
    EXPECT_LE(camera.GetViewCenter().x, 1700.f - 320.f + 0.01f) << "잠금 오른쪽 끝을 넘지 않는다";
    EXPECT_GE(camera.GetViewCenter().x, 1700.f - 320.f - 0.5f);
}

TEST(Camera, ClearingTheScrollFloorStopsTheForcedMotion)
{
    Camera camera = MakeLevelCamera_();
    camera.SetTarget({ 300.f, 224.f });
    camera.SnapToTarget();
    camera.AddScrollFloor(40.f);
    camera.Update(kStepSec);
    const float moved = camera.GetViewCenter().x;
    camera.ClearScrollFloor();
    EXPECT_FALSE(camera.HasScrollFloor());
    for (int i = 0; i < 30; ++i)
    {
        camera.Update(kStepSec);
    }
    EXPECT_LE(camera.GetViewCenter().x, moved + 0.5f) << "하한을 지우면 목표 (플레이어) 쪽으로 돌아간다 (더 가지 않는다)";
}
