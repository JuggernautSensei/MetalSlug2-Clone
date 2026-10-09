#include "Game/StateFactory.h"

#include "Game/DemoState.h"
#include "Game/SandboxState.h"

namespace slug
{

jug::Scoped<IGameState> CreateState(
    const jug::StringView _name)
{
    if (_name == "demo")
    {
        return jug::Scoped<IGameState> { new DemoState };
    }
    if (_name == "sandbox")
    {
        return jug::Scoped<IGameState> { new SandboxState };
    }
    return nullptr;
}

jug::StringView GetDefaultStateName()
{
    return "sandbox";
}

}   // namespace slug
