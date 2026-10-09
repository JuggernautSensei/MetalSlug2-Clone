#include <gtest/gtest.h>

#include "Core/FixedStep.h"
#include "Core/Input.h"

using namespace slug;

// ===========================================
//  FixedStepClock
// ===========================================

TEST(FixedStepClock, OneStepPerSixtiethOfSecond)
{
    FixedStepClock clock;
    EXPECT_EQ(clock.Advance(FixedStepClock::kStepSec), 1);
    EXPECT_EQ(clock.Advance(FixedStepClock::kStepSec * 2.0), 2);
}

TEST(FixedStepClock, SubStepDeltasAccumulate)
{
    FixedStepClock clock;
    EXPECT_EQ(clock.Advance(FixedStepClock::kStepSec * 0.5), 0);
    EXPECT_EQ(clock.Advance(FixedStepClock::kStepSec * 0.6), 1);
}

TEST(FixedStepClock, SameStepCountAtDifferentFrameRates)
{
    // 1초를 60fps / 144fps / 30fps 로 쪼개도 총 스텝 수는 60 (오차 1 이내)
    const double frameRates[] = { 60.0, 144.0, 30.0, 240.0 };
    for (const double fps: frameRates)
    {
        FixedStepClock clock;
        int            total  = 0;
        const int      frames = static_cast<int>(fps);
        for (int i = 0; i < frames; ++i)
        {
            total += clock.Advance(1.0 / fps);
        }
        EXPECT_NEAR(total, 60, 1) << "fps=" << fps;
    }
}

TEST(FixedStepClock, LongStallIsCapped)
{
    FixedStepClock clock;
    EXPECT_EQ(clock.Advance(10.0), FixedStepClock::kMaxStepsPerFrame);
    EXPECT_LT(clock.GetAlpha(), 1.f);
}

TEST(FixedStepClock, AlphaIsFractionOfStep)
{
    FixedStepClock clock;
    EXPECT_EQ(clock.Advance(FixedStepClock::kStepSec * 1.25), 1);
    EXPECT_NEAR(clock.GetAlpha(), 0.25f, 1e-3f);
}

// ===========================================
//  InputHistory
// ===========================================

namespace
{
    InputFrame Held(
        const eAction _action)
    {
        InputFrame frame;
        frame.Set(_action, true);
        return frame;
    }
}   // namespace

TEST(InputHistory, PressEdgeIsDetectedOnFirstFrame)
{
    InputHistory history;
    history.Push({});
    EXPECT_FALSE(history.WasPressed(eAction::Jump));

    history.Push(Held(eAction::Jump));
    EXPECT_TRUE(history.WasPressed(eAction::Jump));
    EXPECT_TRUE(history.IsHeld(eAction::Jump));

    history.Push(Held(eAction::Jump));
    EXPECT_FALSE(history.WasPressed(eAction::Jump));
    EXPECT_TRUE(history.IsHeld(eAction::Jump));
}

TEST(InputHistory, ReleaseEdge)
{
    InputHistory history;
    history.Push(Held(eAction::Shot));
    history.Push({});
    EXPECT_TRUE(history.WasReleased(eAction::Shot));
    history.Push({});
    EXPECT_FALSE(history.WasReleased(eAction::Shot));
}

TEST(InputHistory, PressBufferWindow)
{
    InputHistory history;
    history.Push({});
    history.Push(Held(eAction::Jump));
    history.Push({});
    history.Push({});
    EXPECT_TRUE(history.WasPressedWithin(eAction::Jump, 4));
    EXPECT_FALSE(history.WasPressedWithin(eAction::Jump, 2));
}

TEST(InputHistory, ConsumedPressIsNotReported)
{
    InputHistory history;
    history.Push({});
    history.Push(Held(eAction::Jump));
    ASSERT_TRUE(history.WasPressedWithin(eAction::Jump, 4));
    history.ConsumePress(eAction::Jump);
    EXPECT_FALSE(history.WasPressedWithin(eAction::Jump, 4));

    // 새 눌림은 다시 감지된다.
    history.Push({});
    history.Push(Held(eAction::Jump));
    EXPECT_TRUE(history.WasPressedWithin(eAction::Jump, 2));
}

TEST(InputSystem, MultipleKeysMapToOneAction)
{
    InputSystem input;
    input.OnKeyDown(SDL_SCANCODE_A);
    EXPECT_TRUE(input.Sample().IsHeld(eAction::Left));
    input.OnKeyDown(SDL_SCANCODE_LEFT);
    input.OnKeyUp(SDL_SCANCODE_A);
    EXPECT_TRUE(input.Sample().IsHeld(eAction::Left));
    input.OnKeyUp(SDL_SCANCODE_LEFT);
    EXPECT_FALSE(input.Sample().IsHeld(eAction::Left));
}
