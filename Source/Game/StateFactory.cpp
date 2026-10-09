#include "Game/StateFactory.h"

#include "Game/DemoState.h"

namespace slug
{

jug::Scoped<IGameState> CreateState(
    const jug::StringView _name)
{
    if (_name == "demo")
    {
        return jug::Scoped<IGameState> { new DemoState };
    }
    return nullptr;
}

jug::StringView GetDefaultStateName()
{
    return "demo";
}

}   // namespace slug
