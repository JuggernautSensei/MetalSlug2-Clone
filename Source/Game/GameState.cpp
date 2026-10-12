#include "Game/GameState.h"

bool GameContext::IsHeadless() const
{
    return pRenderer == nullptr;
}

AudioSystem& GameContext::GetAudio() const
{
    static AudioSystem s_silent;
    return pAudio ? *pAudio : s_silent;
}

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
