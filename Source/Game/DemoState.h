#pragma once
#include "Base/Math.h"
#include "Game/GameState.h"

class DemoState final : public IGameState
{
public:
    void               Tick(GameContext& _ctx, const InputHistory& _input) override;
    void               Render(GameContext& _ctx, float _alpha) override;
    [[nodiscard]] bool IsFinished() const override;

    [[nodiscard]] Vec2 GetPosition() const;
    [[nodiscard]] uint64_t DebugHash() const override;

private:
    static constexpr float kSpeedPerTick = 2.f;   // 120 px/s @ 60Hz

    Vec2 m_prevPos   = { 320.f, 224.f };
    Vec2 m_pos       = { 320.f, 224.f };
    bool m_bFinished = false;
};
