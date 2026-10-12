#pragma once
#include "Game/GameState.h"
#include "Game/Ui/ScreenFlow.h"
#include "Game/Ui/UiSystem.h"

class UiDemoState final : public IGameState
{
public:
    void OnEnter(GameContext& _ctx) override;
    void Tick(GameContext& _ctx, const InputHistory& _input) override;
    void Render(GameContext& _ctx, float _alpha) override;

    [[nodiscard]] bool IsFinished() const override;

    [[nodiscard]] int             GetTickCount() const;

private:
    [[nodiscard]] HudModel MakeModel_() const;

    UiSystem   m_ui   = {};
    ScreenFlow m_flow = {};
    int        m_tick = 0;
};
