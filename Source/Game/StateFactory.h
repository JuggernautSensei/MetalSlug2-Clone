#pragma once
#include "Base/Types.h"
#include "Game/GameState.h"

namespace slug
{

[[nodiscard]] std::unique_ptr<IGameState> CreateState(StringView _name);

[[nodiscard]] StringView GetDefaultStateName();

}   // namespace slug
