#pragma once
#include <memory>

#include "Base/Types.h"
#include "Core/Input.h"
#include "Game/Player/PlayerTypes.h"

struct DebugSwitches
{
    float    fps            = 0.f;
    bool     bDebugDraw     = false;
    bool     bPaused        = false;
    int      warp           = 0;
    float    warpX          = 0.f;
    int      lives          = 0;
    int      spawn          = 0;
    int      weapon         = 0;
    eGodMode godMode        = eGodMode::Off;
    bool     bInfiniteAmmo  = false;
    int      numLoadWorkers = 1;
};

extern DebugSwitches g_debug;

class IGameState
{
public:
    virtual ~IGameState() = default;

    virtual void OnEnter();
    virtual void OnExit();

    virtual void Tick(const InputHistory& _input) = 0;
    virtual void Render(float _alpha)             = 0;

    [[nodiscard]] virtual Scoped<IGameState> TakeNextState();

    [[nodiscard]] virtual bool IsFinished() const;

    // 회귀 기준선용 상태 해시
    [[nodiscard]] virtual uint64_t DebugHash() const;
};
