#pragma once
#include "Base/Types.h"
#include "Game/GameState.h"

[[nodiscard]] Scoped<IGameState> CreateState(StringView _name);

[[nodiscard]] StringView GetDefaultStateName();
