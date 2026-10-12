#pragma once
#include <memory>

#include "Audio/AudioSystem.h"
#include "Base/Types.h"
#include "Core/Input.h"
#include "Game/Player/PlayerTypes.h"
#include "Render/Animation.h"
#include "Render/Renderer.h"
#include "Render/SpriteBatch.h"

struct GameContext
{
    Renderer*         pRenderer   = nullptr;
    SpriteBatch*      pSprites    = nullptr;
    AnimationLibrary* pAnims      = nullptr;
    RngMt19937*       pRng        = nullptr;
    FilePath          assetRoot   = {};
    AudioSystem*      pAudio      = nullptr;
    int               loadWorkers = 1;

    float    fps           = 0.f;
    bool     bDebugDraw    = false;
    bool     bPaused       = false;
    int      slowMoDivisor = 1;   // F4: 2 이상이면 느린 재생
    int      debugWarp     = 0;
    float    debugWarpX    = 0.f;
    int      debugLives    = 0;   // --life N (0 이면 튜닝 값)
    int      debugSpawn    = 0;
    int      debugWeapon   = 0;
    eGodMode godMode       = eGodMode::Off;
    bool     bInfiniteAmmo = false;

    [[nodiscard]] bool IsHeadless() const;

    [[nodiscard]] AudioSystem& GetAudio() const;
};

class IGameState
{
public:
    virtual ~IGameState() = default;

    virtual void OnEnter(GameContext&);
    virtual void OnExit(GameContext&);

    virtual void Tick(GameContext& _ctx, const InputHistory& _input) = 0;
    virtual void Render(GameContext& _ctx, float _alpha)             = 0;

    [[nodiscard]] virtual Scoped<IGameState> TakeNextState();

    [[nodiscard]] virtual bool IsFinished() const;

    // 회귀 기준선용 상태 해시 (Headless 가 끝에 출력). 구현하지 않은 상태는 0.
    [[nodiscard]] virtual uint64_t DebugHash() const;
};
