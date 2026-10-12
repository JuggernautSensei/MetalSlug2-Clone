#pragma once
#include "Base/Types.h"
#include "Game/GameState.h"

[[nodiscard]] std::unique_ptr<IGameState> CreateState(StringView _name);

[[nodiscard]] StringView GetDefaultStateName();
