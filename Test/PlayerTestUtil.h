// 플레이어/샌드박스 테스트 공용 도우미: 평지 설정과 Sim(SandboxState 를 SimRunner 로 한 틱씩 돌리는 하네스).
#pragma once
#include <cmath>
#include <memory>

#include <gtest/gtest.h>

#include "Base/Math.h"
#include "Base/Random.h"
#include "Base/Result.h"
#include "Base/Types.h"
#include "Core/AssetPath.h"
#include "Core/Input.h"
#include "Game/SandboxState.h"
#include "Game/SimRunner.h"
#include "Render/Animation.h"


// 조준 키를 누른 뒤 조준각이 목표각에 닿을 때까지 기다리는 틱 (HMG 는 연속 조준각이라 시간이 걸린다)
inline constexpr int kAimSettleTicks = 20;

[[nodiscard]] GroundMap MakeFlatGround(
    const int _width  = 1600,
    const int _height = 448,
    const int _floorY = 378);

[[nodiscard]] SandboxConfig MakeConfig(
    GroundMap  _ground,
    const Vec2 _spawn = { 100.f, 378.f });

[[nodiscard]] SandboxConfig MakeFlatConfig();

class Sim
{
public:
    explicit Sim(
        SandboxConfig     _config = MakeFlatConfig(),
        AnimationLibrary* _pAnims = nullptr);

    [[nodiscard]] SandboxState&     State();
    [[nodiscard]] PlayerController& Player();
    [[nodiscard]] Tuning&           Tune();
    [[nodiscard]] SimRunner&        Runner();

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

    // 조건이 참이 될 때까지 최대 _maxTicks 만큼 진행한다. 참이 된 틱 수를, 끝내 안 되면 -1 을 돌려준다.
    template<typename Pred>
    int StepUntil(
        const int _maxTicks,
        Pred&&    _pred)
    {
        for (int i = 0; i < _maxTicks; ++i)
        {
            if (_pred())
            {
                return i;
            }
            Step();
        }
        return _pred() ? _maxTicks : -1;
    }

    // 자주 쓰는 종료 조건 (StepUntil 의 술어 대신 쓴다)
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
    RngMt19937           m_rng     = {};
    GameContext                m_ctx     = {};
    SandboxState*              m_pState  = nullptr;
    std::unique_ptr<SimRunner> m_pRunner = nullptr;
    InputFrame                 m_frame   = {};
};

[[nodiscard]] bool LoadAnims(
    AnimationLibrary& _out);

#define SLUG_REQUIRE_BAKED(_anims)                                                           \
    AnimationLibrary _anims;                                                                 \
    if (!LoadAnims(_anims))                                                   \
    {                                                                                        \
        GTEST_SKIP() << "Assets 없음"; \
    }
