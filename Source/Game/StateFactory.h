#pragma once
#include "Game/GameState.h"

namespace slug
{

// 이름으로 게임 상태를 만든다. 알 수 없는 이름이면 nullptr.
// 등록된 이름: "demo", "sandbox"  (이후 마일스톤에서 "title", "stage0" 추가)
[[nodiscard]] jug::Scoped<IGameState> CreateState(jug::StringView _name);

// --state 를 지정하지 않았을 때의 기본 상태 이름
[[nodiscard]] jug::StringView GetDefaultStateName();

}   // namespace slug
