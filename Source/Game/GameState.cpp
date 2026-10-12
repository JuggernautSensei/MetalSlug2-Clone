#include "Game/GameState.h"

bool GameContext::IsHeadless() const
{
    return pRenderer == nullptr;
}

IAudioSink& GameContext::GetAudio() const
{
    return pAudio ? *pAudio : NullAudio::Get();
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
