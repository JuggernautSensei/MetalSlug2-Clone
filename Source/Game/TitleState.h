#pragma once
#include "Base/Types.h"
#include "Game/GameState.h"
#include "Game/Ui/UiSystem.h"

struct TitleConfig
{
    String nextState       = {};
    int    fadeInTicks     = 24;
    int    startFadeTicks  = 24;
    int    inputDelayTicks = 12;
    bool   bPlayVoice      = true;
};

class TitleState final : public IGameState
{
public:
    TitleState();
    explicit TitleState(TitleConfig _config);

    void OnEnter() override;
    void OnExit() override;
    void Tick(const InputHistory& _input) override;
    void Render(float _alpha) override;

    [[nodiscard]] Scoped<IGameState> TakeNextState() override;

    [[nodiscard]] bool            IsStarting() const;
    [[nodiscard]] int             GetTickCount() const;
    [[nodiscard]] const String&   GetSelectedStateName() const;
    [[nodiscard]] const UiSystem& GetUi() const;

private:
    [[nodiscard]] Scoped<IGameState> CreateNext_() const;

    TitleConfig m_config = {};
    UiSystem    m_ui     = {};

    int                m_tickCount = 0;
    bool               m_bStarting = false;
    String             m_selected  = {};
    Scoped<IGameState> m_pNext     = nullptr;
};
