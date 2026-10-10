#pragma once
#include <memory>

#include "Audio/AudioSink.h"
#include "Base/Random.h"
#include "Base/Types.h"
#include "Core/Input.h"
#include "Render/Animation.h"
#include "Render/Renderer.h"
#include "Render/SpriteBatch.h"

namespace slug
{

struct GameContext
{
    Renderer*         pRenderer = nullptr;
    SpriteBatch*      pSprites  = nullptr;
    AnimationLibrary* pAnims    = nullptr;
    RngMt19937*       pRng      = nullptr;
    FilePath          assetRoot = {};
    IAudioSink*       pAudio    = nullptr;

    float fps           = 0.f;
    bool  bDebugDraw    = false;
    bool  bPaused       = false;
    int   slowMoDivisor = 1;   // F4: 2 이상이면 느린 재생
    int   debugWarp     = 0;
    float debugWarpX    = 0.f;   // debugWarp == 3 일 때 목표 월드 x
    int   debugLives    = 0;   // --life N (0 이면 튜닝 값)
    int   debugSpawn    = 0;
    int   debugWeapon   = 0;

    [[nodiscard]] bool IsHeadless() const
    {
        return pRenderer == nullptr;
    }

    [[nodiscard]] IAudioSink& GetAudio() const
    {
        return pAudio ? *pAudio : NullAudio::Get();
    }
};

class IGameState
{
public:
    virtual ~IGameState() = default;

    virtual void OnEnter(GameContext&) {}
    virtual void OnExit(GameContext&) {}

    virtual void Tick(GameContext& _ctx, const InputHistory& _input) = 0;
    virtual void Render(GameContext& _ctx, float _alpha)             = 0;

    [[nodiscard]] virtual std::unique_ptr<IGameState> TakeNextState()
    {
        return nullptr;
    }

    [[nodiscard]] virtual bool IsFinished() const
    {
        return false;
    }
};

}   // namespace slug
