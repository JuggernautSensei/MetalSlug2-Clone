#pragma once
#include "Game/GameState.h"

namespace slug
{

// ===========================================
//  SimRunner
//   입력 프레임을 받아 현재 상태를 한 틱씩 진행한다.
//   App(라이브) 과 헤드리스 도구/테스트가 동일하게 사용한다.
// ===========================================

class SimRunner
{
public:
    SimRunner(GameContext& _ctx, jug::Scoped<IGameState> _pState);
    ~SimRunner();

    SimRunner(const SimRunner&)            = delete;
    SimRunner& operator=(const SimRunner&) = delete;

    void Step(InputFrame _input);
    void Run(jug::Span<const InputFrame> _inputs);

    void Render(float _alpha);

    [[nodiscard]] IGameState*        GetState() const;
    [[nodiscard]] uint64_t           GetTickCount() const;
    [[nodiscard]] const InputHistory& GetHistory() const;
    [[nodiscard]] bool               IsFinished() const;

private:
    void SwitchState_(jug::Scoped<IGameState> _pNext);

    GameContext&              m_ctx;
    jug::Scoped<IGameState>   m_pState;
    InputHistory              m_history   = {};
    uint64_t                  m_tickCount = 0;
};

}   // namespace slug
