#include "Game/StateFactory.h"

#include "Base/Types.h"
#include "Game/DemoState.h"
#include "Game/SandboxState.h"
#include "Game/Stage/StageState.h"
#include "Game/TitleState.h"
#include "Game/Ui/UiDemoState.h"

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
    if (_name == "buslab" || _name == "buslab_bot")
    {
        SandboxConfig config;
        config.cameraBounds = Aabb::FromMinMax({ 1800.f, 0.f }, { 3136.f, 448.f });   // 오른쪽 한계 = 카메라 락 (스테이지 bus 섹션 rect 와 같다)
        config.spawnFeet    = { 2300.f, 378.f };   // 이전 구간 끝에서 시작: 걸어가면 카메라 우측이 버스에 닿는다
        config.bSpawnTargets = false;
        config.bBusLab      = true;
        config.bBusBot      = _name == "buslab_bot";
        return std::unique_ptr<IGameState> { new SandboxState { config } };
    }
    if (_name == "bosslab" || _name == "bosslab_god" || _name == "bosslab_bot")
    {
        // 최종보스 전함 연습장: 실제 스테이지와 같은 배경/지면/카메라 스크롤 (Assets/Stage/BossLab.json = Stage0 의 openland 끝 + boss_arena).
        // _god 는 비교 캡처용 무적, _bot 은 봇 입력 (아레나까지 걷고 위로 사격) + HMG + 무적
        StageStateConfig config;
        config.stageFile = "BossLab.json";
        config.bGod      = _name != "bosslab";
        config.bBot      = _name == "bosslab_bot";
        config.bHmg      = _name == "bosslab_bot";
        config.bAutoDamage = _name == "bosslab_bot";
        return std::unique_ptr<IGameState> { new StageState { config } };
    }
    if (_name == "helilab" || _name == "helilab_god" || _name == "helilab_bot" || _name == "helilab_kill")
    {
        SandboxConfig config;
        config.spawnFeet     = { 1500.f, 262.f };   // 원작 망루 위 플레이어 높이 (V1 3:09 의 발 y 약 262)
        config.bSpawnTargets = false;
        config.bHeliLab      = true;
        config.platforms.push_back({ Aabb::FromMinMax({ 1380.f, 262.f }, { 1700.f, 282.f }), false });   // 망루 윗면 대용
        config.bGodMode      = _name != "helilab";
        config.bHeliBot      = _name == "helilab_bot" || _name == "helilab_kill";   // 봇 입력 (좌우 이동 + 위로 사격)
        config.heliHp        = _name == "helilab_kill" ? 40 : -1;                   // 격추 연출 캡처용: 봇 권총으로 곧 격추된다
        return std::unique_ptr<IGameState> { new SandboxState { config } };
    }
    if (_name == "sluglab")
    {
        // 평지에 주차된 SV-001 과 표적 (RemakePlan 3-4). 스펙: Docs/Spec_Slug.md
        SandboxConfig config;
        config.bLoadGround = false;
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
