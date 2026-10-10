#include "Game/StateFactory.h"

#include "Base/Types.h"
#include "Game/DemoState.h"
#include "Game/SandboxState.h"
#include "Game/Stage/StageState.h"
#include "Game/TitleState.h"
#include "Game/Ui/UiDemoState.h"

namespace slug
{

std::unique_ptr<IGameState> CreateState(
    const StringView _name)
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
    if (_name == "buslab")
    {
        SandboxConfig config;
        config.cameraBounds = Aabb::FromMinMax({ 2000.f, 0.f }, { 3136.f, 448.f });
        config.spawnFeet    = { 2620.f, 378.f };
        config.bSpawnTargets = false;
        config.bBusLab      = true;
        return std::unique_ptr<IGameState> { new SandboxState { config } };
    }
    if (_name == "helilab" || _name == "helilab_god")
    {
        SandboxConfig config;
        config.spawnFeet     = { 1500.f, 262.f };   // 원작 망루 위 플레이어 높이 (V1 3:09 의 발 y 약 262)
        config.bSpawnTargets = false;
        config.bHeliLab      = true;
        config.platforms.push_back({ Aabb::FromMinMax({ 1380.f, 262.f }, { 1700.f, 282.f }), false });   // 망루 윗면 대용
        config.bGodMode      = _name == "helilab_god";
        return std::unique_ptr<IGameState> { new SandboxState { config } };
    }
    if (_name == "sluglab")
    {
        // 평지에 주차된 SV-001 과 표적 (RemakePlan 3-4). 스펙: Docs/Spec_Slug.md
        SandboxConfig config;
        config.bLoadBakedGround = false;
        config.ground           = GroundMap::MakeEmpty(3520, 448);
        config.ground.FillRect(0, 378, 3519, 447, true);
        config.spawnFeet = { 200.f, 378.f };
        config.bSlugLab  = true;
        config.bSpawnTargets = false;
        config.slugFeet.push_back({ 430.f, 378.f });
        config.slugFeet.push_back({ 2300.f, 378.f });
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

StringView GetDefaultStateName()
{
    return "title";
}

}   // namespace slug
