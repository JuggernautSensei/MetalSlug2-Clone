#include "Game/StateFactory.h"
#include "Base/Types.h"

#include "Game/DemoState.h"
#include "Game/SandboxState.h"
#include "Game/TitleState.h"
#include "Game/Stage/StageState.h"
#include "Game/Ui/UiDemoState.h"

namespace slug
{

std::unique_ptr<IGameState> CreateState(
    const slug::StringView _name)
{
    if (_name == "demo")
    {
        return std::unique_ptr<IGameState> { new DemoState };
    }
    if (_name == "sandbox")
    {
        return std::unique_ptr<IGameState> { new SandboxState };
    }
    if (_name == "proplab")
    {
        SandboxConfig config;
        config.spawnFeet     = { 1500.f, 378.f };
        config.bSpawnTargets = false;
        config.bPropLab      = true;
        return std::unique_ptr<IGameState> { new SandboxState { config } };
    }
    if (_name == "enemylab")
    {
        SandboxConfig config;
        config.bEnemyLab = true;
        return std::unique_ptr<IGameState> { new SandboxState { config } };
    }
    if (_name == "vehiclelab")
    {
        SandboxConfig config;
        config.bVehicleLab = true;
        return std::unique_ptr<IGameState> { new SandboxState { config } };
    }
    if (_name == "bosslab")
    {
        SandboxConfig config;
        config.bLoadBakedGround = false;
        config.ground           = GroundMap::MakeEmpty(640, 448);
        config.ground.FillRect(0, 378, 639, 447, true);
        config.cameraBounds     = Aabb::FromMinMax({ 0.f, 0.f }, { 640.f, 448.f });
        config.spawnFeet        = { 540.f, 378.f };
        config.bSpawnTargets    = false;
        config.bBackground      = false;
        config.bBossLab         = true;
        config.bossArena        = Aabb::FromMinMax({ 0.f, 0.f }, { 640.f, 448.f });
        config.bossGroundY      = 378.f;
        return std::unique_ptr<IGameState> { new SandboxState { config } };
    }
    if (_name == "stage0")
    {
        return std::unique_ptr<IGameState> { new StageState };
    }
    if (_name == "title")
    {
        return std::unique_ptr<IGameState> { new TitleState };
    }
    if (_name == "uidemo")
    {
        return std::unique_ptr<IGameState> { new UiDemoState };
    }
    return nullptr;
}

slug::StringView GetDefaultStateName()
{
    return "title";
}
}
