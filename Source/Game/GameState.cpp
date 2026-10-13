#include "Game/GameState.h"

DebugSwitches g_debug;

void IGameState::OnEnter(GameContext&)
{
}

void IGameState::OnExit(GameContext&)
{
}

Scoped<IGameState> IGameState::TakeNextState()
{
    return nullptr;
}

bool IGameState::IsFinished() const
{
    return false;
}

uint64_t IGameState::DebugHash() const
{
    return 0;
}
