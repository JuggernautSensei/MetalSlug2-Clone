#pragma once
#include "Base/Random.h"
#include <memory>
#include "Base/Types.h"

#include "Audio/AudioSink.h"
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
    slug::RngMt19937*  pRng      = nullptr;
    slug::FilePath     assetRoot = {};
    IAudioSink*       pAudio    = nullptr;

    float fps           = 0.f;
    bool  bDebugDraw    = false;
    bool  bPaused       = false;
    int   slowMoDivisor = 1;       // F4: 2 이상이면 느린 재생
    int   debugWarp     = 0;
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
}
