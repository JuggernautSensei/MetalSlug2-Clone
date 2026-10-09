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
