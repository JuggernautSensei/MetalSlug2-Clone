#include "Game/StateFactory.h"

#include "Game/DemoState.h"
#include "Game/SandboxState.h"
#include "Game/TitleState.h"
#include "Game/Stage/StageState.h"
#include "Game/Ui/UiDemoState.h"

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
    if (_name == "proplab")
    {
        // 소품 검증: Stage0 상점 거리 (플레이어 1500,378), 표적 없음 (상태 이름 proplab)
        SandboxConfig config;
        config.spawnFeet     = { 1500.f, 378.f };
        config.bSpawnTargets = false;
        config.bPropLab      = true;
        return jug::Scoped<IGameState> { new SandboxState { config } };
    }
    if (_name == "enemylab")
    {
        SandboxConfig config;
        config.bEnemyLab = true;   // 보병 8종을 플레이어 앞에 배치 (적 시각 검증)
        return jug::Scoped<IGameState> { new SandboxState { config } };
    }
    if (_name == "vehiclelab")
    {
        SandboxConfig config;
        config.bVehicleLab = true;   // 차량 5종을 화면 안에 배치 (차량 시각 검증)
        return jug::Scoped<IGameState> { new SandboxState { config } };
    }
    if (_name == "bosslab")
    {
        // 최종보스 검증: 640x448 평지 아레나 (바닥 y 378), 플레이어는 오른쪽에서 시작, 보스는 왼쪽 밖에서 레일로 등장
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
        return jug::Scoped<IGameState> { new SandboxState { config } };
    }
    if (_name == "stage0")
    {
        return jug::Scoped<IGameState> { new StageState };
    }
    if (_name == "title")
    {
        return jug::Scoped<IGameState> { new TitleState };
    }
    if (_name == "uidemo")
    {
        return jug::Scoped<IGameState> { new UiDemoState };
    }
    return nullptr;
}

jug::StringView GetDefaultStateName()
{
    // "stage0" 가 등록되고 Assets/Baked 가 있으면 "title" 로 바꾼다 (타이틀 → stage0). 바꿀 때 Test/PlayerMoveTest.cpp 의 기본 상태 검사도 갱신.
    return "sandbox";
}

}   // namespace slug
