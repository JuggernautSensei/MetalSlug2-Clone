#pragma once
#include "Game/GameState.h"
#include "Base/Types.h"

namespace slug
{
[[nodiscard]] std::unique_ptr<IGameState> CreateState(slug::StringView _name);

[[nodiscard]] slug::StringView GetDefaultStateName();
}
