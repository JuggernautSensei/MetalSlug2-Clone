#pragma once
#include "Base/Types.h"

#include "Game/GameState.h"
#include "Game/Ui/UiSystem.h"

namespace slug
{
struct TitleConfig
{
    slug::String nextState      = {};
    int         fadeInTicks    = 24;    // 처음 화면이 밝아지는 시간
    int         startFadeTicks = 24;    // 시작 입력 후 암전 시간
    int         inputDelayTicks = 12;
    bool        bPlayVoice     = true;
};

class TitleState final : public IGameState
{
public:
    TitleState();
    explicit TitleState(TitleConfig _config);

    void OnEnter(GameContext& _ctx) override;
    void OnExit(GameContext& _ctx) override;
    void Tick(GameContext& _ctx, const InputHistory& _input) override;
    void Render(GameContext& _ctx, float _alpha) override;

    [[nodiscard]] std::unique_ptr<IGameState> TakeNextState() override;

    [[nodiscard]] bool                     IsStarting() const { return m_bStarting; }
    [[nodiscard]] int                      GetTickCount() const { return m_tickCount; }
    [[nodiscard]] const slug::String&       GetSelectedStateName() const { return m_selected; }
    [[nodiscard]] const UiSystem&          GetUi() const { return m_ui; }

private:
    [[nodiscard]] std::unique_ptr<IGameState> CreateNext_() const;

    TitleConfig m_config = {};
    UiSystem    m_ui     = {};

    int         m_tickCount = 0;
    bool        m_bStarting = false;
    slug::String m_selected  = {};
    std::unique_ptr<IGameState> m_pNext = nullptr;
};
}
