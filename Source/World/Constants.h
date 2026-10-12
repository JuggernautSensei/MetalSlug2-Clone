#pragma once

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

// 이동 튜닝 (MoveTuning, SlugMoveTuning) 에서 물리 튜닝만 뽑는다
template<typename MoveT>
[[nodiscard]] PhysicsTuning MakePhysicsTuning(const MoveT& _move)
{
    PhysicsTuning tuning;
    tuning.gravity          = _move.gravity;
    tuning.terminalVelocity = _move.terminalVelocity;
    tuning.runSpeed         = _move.runSpeed;
    tuning.jumpSpeed        = _move.jumpSpeed;
    tuning.maxStepUp        = _move.maxStepUp;
    tuning.maxSnapDown      = _move.maxSnapDown;
    return tuning;
}
