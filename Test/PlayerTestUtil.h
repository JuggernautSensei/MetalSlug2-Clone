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

namespace slugtest
{

using namespace slug;

// 조준 키를 누른 뒤 조준각이 목표각에 닿을 때까지 기다리는 틱 (HMG 는 연속 조준각이라 시간이 걸린다)
inline constexpr int kAimSettleTicks = 20;

[[nodiscard]] inline GroundMap MakeFlatGround(
    const int _width  = 1600,
    const int _height = 448,
    const int _floorY = 378)
{
    GroundMap map = GroundMap::MakeEmpty(_width, _height);
    map.FillRect(0, _floorY, _width - 1, _height - 1, true);
    return map;
}

[[nodiscard]] inline SandboxConfig MakeConfig(
    GroundMap  _ground,
    const Vec2 _spawn = { 100.f, 378.f })
{
    SandboxConfig config;
    config.bLoadBakedGround = false;
    config.cameraBounds     = Aabb::FromMinMax({ 0.f, 0.f }, { static_cast<float>(_ground.GetWidth()), 448.f });
    config.ground           = std::move(_ground);
    config.spawnFeet        = _spawn;
    config.bSpawnTargets    = false;
    config.bLoadTuningFile  = false;
    config.bBackground      = false;
    return config;
}

[[nodiscard]] inline SandboxConfig MakeFlatConfig()
{
    return MakeConfig(MakeFlatGround());
}

class Sim
{
public:
    explicit Sim(
        SandboxConfig     _config = MakeFlatConfig(),
        AnimationLibrary* _pAnims = nullptr)
        : m_rng(uint64_t { 1 })
    {
        m_ctx.pRng   = &m_rng;
        m_ctx.pAnims = _pAnims;

        m_pState  = new SandboxState { std::move(_config) };
        m_pRunner = std::unique_ptr<SimRunner> {
            new SimRunner { m_ctx, std::unique_ptr<IGameState> { m_pState } }
        };
    }

    [[nodiscard]] SandboxState&     State() { return *m_pState; }
    [[nodiscard]] PlayerController& Player() { return m_pState->GetPlayer(); }
    [[nodiscard]] Tuning&           Tune() { return m_pState->GetTuning(); }
    [[nodiscard]] SimRunner&        Runner() { return *m_pRunner; }

    void Hold(const eAction _action) { m_frame.Set(_action, true); }
    void Release(const eAction _action) { m_frame.Set(_action, false); }
    void ReleaseAll() { m_frame = {}; }
    void Set(const eAction _action, const bool _bHeld) { m_frame.Set(_action, _bHeld); }

    void Step(
        const int _count = 1)
    {
        for (int i = 0; i < _count; ++i)
        {
            m_pRunner->Step(m_frame);
        }
    }

    void Tap(
        const eAction _action)
    {
        Hold(_action);
        Step();
        Release(_action);
    }

    void StepWith(
        const InputFrame _frame)
    {
        m_frame = _frame;
        m_pRunner->Step(_frame);
    }

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
        const int _maxTicks)
    {
        for (int i = 0; i < _maxTicks; ++i)
        {
            if (Player().IsGrounded())
            {
                return i;
            }
            Step();
        }
        return Player().IsGrounded() ? _maxTicks : -1;
    }

    [[nodiscard]] int StepUntilAirborne(
        const int _maxTicks)
    {
        for (int i = 0; i < _maxTicks; ++i)
        {
            if (!Player().IsGrounded())
            {
                return i;
            }
            Step();
        }
        return !Player().IsGrounded() ? _maxTicks : -1;
    }

    [[nodiscard]] int StepUntilDead(
        const int _maxTicks)
    {
        for (int i = 0; i < _maxTicks; ++i)
        {
            if (!Player().IsAlive())
            {
                return i;
            }
            Step();
        }
        return !Player().IsAlive() ? _maxTicks : -1;
    }

    [[nodiscard]] int StepUntilWeapon(
        const int     _maxTicks,
        const eWeapon _weapon)
    {
        for (int i = 0; i < _maxTicks; ++i)
        {
            if (Player().GetWeapon().weapon == _weapon)
            {
                return i;
            }
            Step();
        }
        return Player().GetWeapon().weapon == _weapon ? _maxTicks : -1;
    }

    [[nodiscard]] float X() { return Player().GetFeet().x; }
    [[nodiscard]] float Y() { return Player().GetFeet().y; }

private:
    slug::RngMt19937           m_rng     = {};
    GameContext                m_ctx     = {};
    SandboxState*              m_pState  = nullptr;
    std::unique_ptr<SimRunner> m_pRunner = nullptr;
    InputFrame                 m_frame   = {};
};

[[nodiscard]] inline bool LoadBakedAnims(
    AnimationLibrary& _out)
{
    slug::Result<AnimationLibrary> library = AnimationLibrary::LoadFromDirectory(slug::FindAssetRoot() / "Baked");
    if (!library)
    {
        return false;
    }
    _out = library.Take();
    return true;
}

#define SLUG_REQUIRE_BAKED(_anims)                                                           \
    AnimationLibrary _anims;                                                                 \
    if (!slugtest::LoadBakedAnims(_anims))                                                   \
    {                                                                                        \
        GTEST_SKIP() << "Assets/Baked 없음: Scripts/Bake.ps1 로 생성한 뒤 다시 실행하세요."; \
    }

}   // namespace slugtest
