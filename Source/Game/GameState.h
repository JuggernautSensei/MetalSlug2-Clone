#pragma once
#include <JugX/Random.h>
#include <JugX/Scoped.h>
#include <JugX/Typedef.h>

#include "Core/Input.h"
#include "Render/Animation.h"
#include "Render/Renderer.h"
#include "Render/SpriteBatch.h"

namespace slug
{

// ===========================================
//  GameContext
//   게임 상태들이 공유하는 서비스 묶음 (비소유 포인터).
//   헤드리스 실행(테스트/리플레이 검증)에서는 pRenderer / pSprites 가 nullptr 이다.
// ===========================================

struct GameContext
{
    Renderer*         pRenderer = nullptr;
    SpriteBatch*      pSprites  = nullptr;
    AnimationLibrary* pAnims    = nullptr;
    jug::RngMt19937*  pRng      = nullptr;
    jug::FilePath     assetRoot = {};   // 보통 <repo>/Assets

    // 디버그 정보 (App 이 채운다. 시뮬레이션 결과에는 영향이 없다)
    float fps           = 0.f;
    bool  bDebugDraw    = false;   // F1: 판정 상자/지면 탐침 표시
    bool  bPaused       = false;   // F2
    int   slowMoDivisor = 1;       // F4: 2 이상이면 느린 재생

    [[nodiscard]] bool IsHeadless() const
    {
        return pRenderer == nullptr;
    }
};

// ===========================================
//  IGameState
//   Tick 은 항상 고정 60Hz 로 호출된다. 라이브 실행과 헤드리스 리플레이가 같은 코드 경로를 탄다.
// ===========================================

class IGameState
{
public:
    virtual ~IGameState() = default;

    virtual void OnEnter(GameContext& /*_ctx*/) {}
    virtual void OnExit(GameContext& /*_ctx*/) {}

    virtual void Tick(GameContext& _ctx, const InputHistory& _input) = 0;
    virtual void Render(GameContext& _ctx, float _alpha)             = 0;

    // Tick 중 상태 전환을 요청했다면 다음 상태를 반환한다 (소유권 이전). 없으면 nullptr.
    [[nodiscard]] virtual jug::Scoped<IGameState> TakeNextState()
    {
        return nullptr;
    }

    // 헤드리스 검증용: 목표 달성(미션 클리어 등) 여부
    [[nodiscard]] virtual bool IsFinished() const
    {
        return false;
    }
};

}   // namespace slug
