#pragma once
#include "Base/Math.h"
#include "Game/GameState.h"

class DemoState final : public IGameState
{
public:
    void               Tick(const InputHistory& _input) override;
    void               Render(float _alpha) override;
    [[nodiscard]] bool IsFinished() const override;

    [[nodiscard]] VECTOR2  GetPosition() const;
    [[nodiscard]] uint64_t DebugHash() const override;

private:
    static constexpr float kSpeedPerTick = 2.f;

    VECTOR2 m_prevPos   = { 320.f, 224.f };
    VECTOR2 m_pos       = { 320.f, 224.f };
    bool    m_bFinished = false;
};
