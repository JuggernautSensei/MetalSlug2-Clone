#include "Game/StateFactory.h"

#include "Base/Types.h"
#include "Game/SandboxState.h"
#include "Game/Stage/StageState.h"
#include "Game/TitleState.h"

Scoped<IGameState> CreateState(
    const StringView _name)
{
    if (_name == "sandbox")
    {
        return MakeScoped<SandboxState>();
    }
    if (_name == "enemylab")
    {
        SandboxConfig config;
        config.bEnemyLab = true;
        return MakeScoped<SandboxState>(config);
    }
    if (_name == "stage0")
    {
        return MakeScoped<StageState>();
    }
    if (_name == "title")
    {
        return MakeScoped<TitleState>();
    }
    return nullptr;
}

StringView GetDefaultStateName()
{
    return "title";
}
