#pragma once
#include <cmath>
#include <memory>

#include <gtest/gtest.h>

#include "Base/SlugError.h"
#include "Base/Types.h"
#include "Core/AssetPath.h"
#include "Core/Input.h"
#include "Game/SandboxState.h"
#include "Game/SimRunner.h"
#include "Render/Animation.h"

inline constexpr int kAimSettleTicks = 20;

[[nodiscard]] GroundMap MakeFlatGround(
    const int _width  = 1600,
    const int _height = 448,
    const int _floorY = 378);

[[nodiscard]] SandboxConfig MakeConfig(
    GroundMap     _ground,
    const VECTOR2 _spawn = { 100.f, 378.f });

[[nodiscard]] SandboxConfig MakeFlatConfig();

class Sim
{
public:
    explicit Sim(
        SandboxConfig _config = MakeFlatConfig());

    [[nodiscard]] SandboxState&     State();
    [[nodiscard]] PlayerController& Player();
    [[nodiscard]] Tuning&           Tune();

    void Hold(const eAction _action);
    void Release(const eAction _action);
    void ReleaseAll();
    void Set(const eAction _action, const bool _bHeld);

    void Step(
        const int _count = 1);

    void Tap(
        const eAction _action);

    void StepWith(
        const InputFrame _frame);

    [[nodiscard]] int StepUntilGrounded(
        const int _maxTicks);

    [[nodiscard]] int StepUntilAirborne(
        const int _maxTicks);

    [[nodiscard]] int StepUntilDead(
        const int _maxTicks);

    [[nodiscard]] int StepUntilWeapon(
        const int     _maxTicks,
        const eWeapon _weapon);

    [[nodiscard]] float X();
    [[nodiscard]] float Y();

private:
    SandboxState*     m_pState  = nullptr;
    Scoped<SimRunner> m_pRunner = nullptr;
    InputFrame        m_frame   = {};
};

[[nodiscard]] bool LoadAnims(
    AnimationLibrary& _out);

#define SLUG_REQUIRE_BAKED(_anims)                               \
    AnimationLibrary& _anims = AnimationLibrary::GetSingleton(); \
    if (!LoadAnims(_anims))                                      \
    {                                                            \
        GTEST_SKIP() << "Assets 없음";                           \
    }
