// 고정 스텝 클록과 입력 이력/입력 시스템의 기본 동작을 검증한다.

#include <gtest/gtest.h>

#include "Core/FixedStep.h"
#include "Core/Input.h"

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
    const ARRAY<double, 4> frameRates = { 60.0, 144.0, 30.0, 240.0 };
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

[[nodiscard]] static InputFrame Held_(
    const eAction _action)
{
    InputFrame frame;
    frame.Set(_action, true);
    return frame;
}

TEST(InputHistory, PressEdgeIsDetectedOnFirstFrame)
{
    InputHistory history;
    history.Push({});
    EXPECT_FALSE(history.WasPressed(eAction::Jump));

    history.Push(Held_(eAction::Jump));
    EXPECT_TRUE(history.WasPressed(eAction::Jump));
    EXPECT_TRUE(history.IsHeld(eAction::Jump));

    history.Push(Held_(eAction::Jump));
    EXPECT_FALSE(history.WasPressed(eAction::Jump));
    EXPECT_TRUE(history.IsHeld(eAction::Jump));
}

TEST(InputHistory, PressBufferWindow)
{
    InputHistory history;
    history.Push({});
    history.Push(Held_(eAction::Jump));
    history.Push({});
    history.Push({});
    EXPECT_TRUE(history.WasPressedWithin(eAction::Jump, 4));
    EXPECT_FALSE(history.WasPressedWithin(eAction::Jump, 2));
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
