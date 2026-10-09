#pragma once
#include "Base/Types.h"
#include "Game/GameState.h"

namespace slug
{
class SimRunner
{
public:
    SimRunner(GameContext& _ctx, std::unique_ptr<IGameState> _pState);
    ~SimRunner();

    SimRunner(const SimRunner&)            = delete;
    SimRunner& operator=(const SimRunner&) = delete;

    void Step(InputFrame _input);
    void Run(slug::Span<const InputFrame> _inputs);

    void Render(float _alpha);

    [[nodiscard]] IGameState*        GetState() const;
    [[nodiscard]] uint64_t           GetTickCount() const;
    [[nodiscard]] const InputHistory& GetHistory() const;
    [[nodiscard]] bool               IsFinished() const;

private:
    void SwitchState_(std::unique_ptr<IGameState> _pNext);

    GameContext&              m_ctx;
    std::unique_ptr<IGameState>   m_pState;
    InputHistory              m_history   = {};
    uint64_t                  m_tickCount = 0;
};
}
