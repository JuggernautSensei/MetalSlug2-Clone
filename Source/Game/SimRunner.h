#pragma once
#include "Base/Types.h"
#include "Game/GameState.h"

class SimRunner
{
public:
    explicit SimRunner(Scoped<IGameState> _pState);
    ~SimRunner();

    SimRunner(const SimRunner&)            = delete;
    SimRunner& operator=(const SimRunner&) = delete;

    void Step(InputFrame _input);
    void Run(Span<const InputFrame> _inputs);

    void Render(float _alpha);
    void SwitchState(Scoped<IGameState> _pNext);

    [[nodiscard]] IGameState*         GetState() const;
    [[nodiscard]] uint64_t            GetTickCount() const;
    [[nodiscard]] const InputHistory& GetHistory() const;
    [[nodiscard]] bool                IsFinished() const;

private:
    Scoped<IGameState> m_pState;
    InputHistory       m_history   = {};
    uint64_t           m_tickCount = 0;
};
