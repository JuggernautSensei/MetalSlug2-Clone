#pragma once
#include <cmath>
#include <memory>

#include <gtest/gtest.h>

#include "Base/Math.h"
#include "Base/SlugError.h"
#include "Base/Types.h"
#include "Core/AssetPath.h"
#include "Core/Input.h"
#include "Game/SandboxState.h"
#include "Game/SimRunner.h"
#include "Render/Animation.h"

inline constexpr int kAimSettleTicks = 20;

[[nodiscard]] GroundMap MakeFlatGround(
    int _width  = 1600,
    int _height = 448,
    int _floorY = 378);

[[nodiscard]] SandboxConfig MakeConfig(
    GroundMap _ground,
    VECTOR2   _spawn = { 100.f, 378.f });

[[nodiscard]] SandboxConfig MakeFlatConfig();

class Sim
{
public:
    explicit Sim(
        SandboxConfig _config = MakeFlatConfig());

    [[nodiscard]] SandboxState&     State();
    [[nodiscard]] PlayerController& Player();
    [[nodiscard]] Tuning&           Tune();

    void Hold(eAction _action);
    void Release(eAction _action);
    void ReleaseAll();
    void Set(eAction _action, bool _bHeld);

    void Step(
        int _count = 1);

    void Tap(
        eAction _action);

    void StepWith(
        InputFrame _frame);

    [[nodiscard]] int StepUntilGrounded(
        int _maxTicks);

    [[nodiscard]] int StepUntilAirborne(
        int _maxTicks);

    [[nodiscard]] int StepUntilDead(
        int _maxTicks);

    [[nodiscard]] int StepUntilWeapon(
        int     _maxTicks,
        eWeapon _weapon);

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
