#include "Game/SimRunner.h"

#include "Base/Types.h"

SimRunner::SimRunner(
    GameContext&       _ctx,
    Scoped<IGameState> _pState)
    : m_ctx(_ctx)
    , m_pState(std::move(_pState))
{
    if (m_pState)
    {
        m_pState->OnEnter(m_ctx);
    }
}

SimRunner::~SimRunner()
{
    if (m_pState)
    {
        m_pState->OnExit(m_ctx);
    }
}

void SimRunner::Step(
    const InputFrame _input)
{
    m_history.Push(_input);
    ++m_tickCount;

    if (!m_pState)
    {
        return;
    }

    m_pState->Tick(m_ctx, m_history);

    if (Scoped<IGameState> pNext = m_pState->TakeNextState(); pNext)
    {
        SwitchState(std::move(pNext));
    }
}

void SimRunner::Run(
    const Span<const InputFrame> _inputs)
{
    for (const InputFrame& input: _inputs)
    {
        Step(input);
    }
}

void SimRunner::Render(
    const float _alpha)
{
    if (m_pState)
    {
        m_pState->Render(m_ctx, _alpha);
    }
}

IGameState* SimRunner::GetState() const
{
    return m_pState.GetPtr();
}

uint64_t SimRunner::GetTickCount() const
{
    return m_tickCount;
}

const InputHistory& SimRunner::GetHistory() const
{
    return m_history;
}

bool SimRunner::IsFinished() const
{
    return m_pState && m_pState->IsFinished();
}

void SimRunner::SwitchState(
    Scoped<IGameState> _pNext)
{
    if (m_pState)
    {
        m_pState->OnExit(m_ctx);
    }
    m_pState = std::move(_pNext);
    m_history.Clear();
    if (m_pState)
    {
        m_pState->OnEnter(m_ctx);
    }
}
