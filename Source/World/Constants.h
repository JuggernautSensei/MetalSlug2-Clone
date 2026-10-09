#pragma once

namespace slug
{
inline constexpr float kStepSec = 1.f / 60.f;   // 고정 스텝 (FixedStepClock 과 동일한 60Hz)

inline constexpr float kGravity          = 1400.f;
inline constexpr float kTerminalVelocity = 3000.f;
inline constexpr float kRunSpeed         = 250.f;
inline constexpr float kJumpSpeed        = 600.f;

struct PhysicsTuning
{
    float gravity          = kGravity;
    float terminalVelocity = kTerminalVelocity;
    float runSpeed         = kRunSpeed;
    float jumpSpeed        = kJumpSpeed;

    float maxStepUp = 4.f;

    float maxSnapDown = 4.f;

    int dropThroughSteps = 8;
};
}
