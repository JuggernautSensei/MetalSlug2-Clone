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
        return Scoped<IGameState> { new DemoState };
    }
    if (_name == "sandbox")
    {
        return Scoped<IGameState> { new SandboxState };
    }
    if (_name == "proplab")
    {
        SandboxConfig config;
        config.spawnFeet     = { 1500.f, 378.f };
        config.bSpawnTargets = false;
        config.bPropLab      = true;
        return Scoped<IGameState> { new SandboxState { config } };
    }
    if (_name == "enemylab")
    {
        SandboxConfig config;
        config.bEnemyLab = true;
        return Scoped<IGameState> { new SandboxState { config } };
    }
    if (_name == "vehiclelab")
    {
        SandboxConfig config;
        config.bVehicleLab = true;
        return Scoped<IGameState> { new SandboxState { config } };
    }
    if (_name == "buslab" || _name == "buslab_bot")
    {
        SandboxConfig config;
        config.cameraBounds  = Aabb::FromMinMax({ 1800.f, 0.f }, { 3136.f, 448.f });
        config.spawnFeet     = { 2300.f, 378.f };
        config.bSpawnTargets = false;
        config.bBusLab       = true;
        config.bBusBot       = _name == "buslab_bot";
        return Scoped<IGameState> { new SandboxState { config } };
    }
    if (_name == "bosslab" || _name == "bosslab_god" || _name == "bosslab_bot")
    {
        // 최종보스 전함 연습장: 실제 스테이지와 같은 배경/지면/카메라 스크롤 (Assets/Stage/BossLab.json = Stage0 의 openland 끝 + boss_arena).
        StageStateConfig config;
        config.stageFile   = "BossLab.json";
        config.bGod        = _name != "bosslab";
        config.bBot        = _name == "bosslab_bot";
        config.bHmg        = _name == "bosslab_bot";
        config.bAutoDamage = _name == "bosslab_bot";
        return Scoped<IGameState> { new StageState { config } };
    }
    if (_name == "helilab" || _name == "helilab_god" || _name == "helilab_bot" || _name == "helilab_kill")
    {
        SandboxConfig config;
        config.spawnFeet     = { 1500.f, 262.f };   // 원작 망루 위 플레이어 높이 (V1 3:09 의 발 y 약 262)
        config.bSpawnTargets = false;
        config.bHeliLab      = true;
        config.platforms.push_back({ Aabb::FromMinMax({ 1380.f, 262.f }, { 1700.f, 282.f }), false });   // 망루 윗면 대용
        config.bGodMode = _name != "helilab";
        config.bHeliBot = _name == "helilab_bot" || _name == "helilab_kill";
        config.heliHp   = _name == "helilab_kill" ? 40 : -1;
        return Scoped<IGameState> { new SandboxState { config } };
    }
    if (_name == "sluglab")
    {
        // 평지에 주차된 SV-001 과 표적. 스펙: Docs/Spec_Slug.md
        SandboxConfig config;
        config.bLoadGround = false;
        config.ground      = GroundMap::MakeEmpty(3520, 448);
        config.ground.FillRect(0, 378, 3519, 447, true);
        config.spawnFeet     = { 200.f, 378.f };
        config.bSlugLab      = true;
        config.bSpawnTargets = false;
        config.slugFeet.push_back({ 430.f, 378.f });
        config.slugFeet.push_back({ 2300.f, 378.f });
        return Scoped<IGameState> { new SandboxState { config } };
    }
    if (_name == "stage0")
    {
        return Scoped<IGameState> { new StageState };
    }
    if (_name == "title")
    {
        return Scoped<IGameState> { new TitleState };
    }
    if (_name == "uidemo")
    {
        return Scoped<IGameState> { new UiDemoState };
    }
    return nullptr;
}

StringView GetDefaultStateName()
{
    return "title";
}
