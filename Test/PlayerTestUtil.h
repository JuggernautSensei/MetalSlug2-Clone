#pragma once
#include <cmath>

#include <gtest/gtest.h>

#include "Core/Input.h"
#include "Game/SandboxState.h"
#include "Game/SimRunner.h"
#include "Render/Animation.h"

// ===========================================
//  플레이어 테스트 도우미
//   SimRunner + SandboxState 를 헤드리스로 돌리고, 입력을 InputFrame 으로 한 틱씩 넣는다 (라이브와 같은 Tick 경로).
// ===========================================

namespace slugtest
{

using namespace slug;

inline GroundMap MakeFlatGround(
    const int _width  = 1600,
    const int _height = 448,
    const int _floorY = 378)
{
    GroundMap map = GroundMap::MakeEmpty(_width, _height);
    map.FillRect(0, _floorY, _width - 1, _height - 1, true);
    return map;
}

inline SandboxConfig MakeConfig(
    GroundMap          _ground,
    const jug::VECTOR2 _spawn = { 100.f, 378.f })
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

// 평지 + 표적 없음 + 기본 Tuning
inline SandboxConfig MakeFlatConfig()
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
        m_pRunner = jug::Scoped<SimRunner> {
            new SimRunner { m_ctx, jug::Scoped<IGameState> { m_pState } }
        };
    }

    [[nodiscard]] SandboxState&     State() { return *m_pState; }
    [[nodiscard]] PlayerController& Player() { return m_pState->GetPlayer(); }
    [[nodiscard]] Tuning&           Tune() { return m_pState->GetTuning(); }
    [[nodiscard]] SimRunner&        Runner() { return *m_pRunner; }

    // --- 입력 ---
    void Hold(const eAction _action) { m_frame.Set(_action, true); }
    void Release(const eAction _action) { m_frame.Set(_action, false); }
    void ReleaseAll() { m_frame = {}; }
    void Set(const eAction _action, const bool _bHeld) { m_frame.Set(_action, _bHeld); }

    // 현재 눌린 키로 _count 틱 진행
    void Step(const int _count = 1)
    {
        for (int i = 0; i < _count; ++i)
        {
            m_pRunner->Step(m_frame);
        }
    }

    // _action 을 한 틱만 눌렀다가 뗀다 (눌린 틱이 곧 Step 한 번)
    void Tap(const eAction _action)
    {
        Hold(_action);
        Step();
        Release(_action);
    }

    // 지정한 입력 프레임을 그대로 한 틱
    void StepWith(const InputFrame _frame)
    {
        m_frame = _frame;
        m_pRunner->Step(_frame);
    }

    // 조건이 참이 될 때까지 최대 _maxTicks 틱 진행한다. 참이 된 시점의 진행 틱 수를 돌려주고, 못 이루면 -1.
    template<typename Pred>
    int StepUntil(const int _maxTicks, Pred&& _pred)
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

    [[nodiscard]] float X() { return Player().GetFeet().x; }
    [[nodiscard]] float Y() { return Player().GetFeet().y; }

private:
    jug::RngMt19937        m_rng;
    GameContext            m_ctx     = {};
    SandboxState*          m_pState  = nullptr;
    jug::Scoped<SimRunner> m_pRunner = nullptr;
    InputFrame             m_frame   = {};
};

// Baked 애셋이 있으면 애니메이션 라이브러리를 읽는다. 없으면 false (테스트는 skip 해야 한다).
inline bool LoadBakedAnims(
    AnimationLibrary& _out)
{
    jug::Result<AnimationLibrary> library = AnimationLibrary::LoadFromDirectory(jug::FilePath { SLUG_ASSET_DIR } / "Baked");
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
