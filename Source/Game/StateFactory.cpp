#include "Game/StateFactory.h"

#include "Base/Types.h"
#include "Game/DemoState.h"
#include "Game/SandboxState.h"
#include "Game/Stage/StageState.h"
#include "Game/TitleState.h"
#include "Game/Ui/UiDemoState.h"

Scoped<IGameState> CreateState(
    const StringView _name)
{
    if (_name == "demo")
    {
        return MakeScoped<DemoState>();
    }
    if (_name == "sandbox")
    {
        return MakeScoped<SandboxState>();
    }
    if (_name == "proplab")
    {
        SandboxConfig config;
        config.spawnFeet     = { 1500.f, 378.f };
        config.bSpawnTargets = false;
        config.bPropLab      = true;
        return MakeScoped<SandboxState>(config);
    }
    if (_name == "enemylab")
    {
        SandboxConfig config;
        config.bEnemyLab = true;
        return MakeScoped<SandboxState>(config);
    }
    if (_name == "vehiclelab")
    {
        SandboxConfig config;
        config.bVehicleLab = true;
        return MakeScoped<SandboxState>(config);
    }
    if (_name == "buslab" || _name == "buslab_bot")
    {
        SandboxConfig config;
        config.cameraBounds  = Aabb::FromMinMax({ 1800.f, 0.f }, { 3136.f, 448.f });
        config.spawnFeet     = { 2300.f, 378.f };
        config.bSpawnTargets = false;
        config.bBusLab       = true;
        config.bBusBot       = _name == "buslab_bot";
        return MakeScoped<SandboxState>(config);
    }
    if (_name == "bosslab" || _name == "bosslab_god" || _name == "bosslab_bot")
    {
        StageStateConfig config;
        config.stageFile   = "BossLab.json";
        config.bGod        = _name != "bosslab";
        config.bBot        = _name == "bosslab_bot";
        config.bHmg        = _name == "bosslab_bot";
        config.bAutoDamage = _name == "bosslab_bot";
        return MakeScoped<StageState>(config);
    }
    if (_name == "helilab" || _name == "helilab_god" || _name == "helilab_bot" || _name == "helilab_kill")
    {
        SandboxConfig config;
        config.spawnFeet     = { 1500.f, 262.f };   // 원작 망루 위 플레이어 높이
        config.bSpawnTargets = false;
        config.bHeliLab      = true;
        config.platforms.push_back({ Aabb::FromMinMax({ 1380.f, 262.f }, { 1700.f, 282.f }), false });   // 망루 윗면 대용
        config.bGodMode = _name != "helilab";
        config.bHeliBot = _name == "helilab_bot" || _name == "helilab_kill";
        config.heliHp   = _name == "helilab_kill" ? 40 : -1;
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
    if (_name == "uidemo")
    {
        return MakeScoped<UiDemoState>();
    }
    return nullptr;
}

StringView GetDefaultStateName()
{
    return "title";
}
