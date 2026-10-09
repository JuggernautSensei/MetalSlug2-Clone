#pragma once
#include "Base/Math.h"

#include "Game/GameState.h"

namespace slug
{

// ===========================================
//  DemoState
//   입력으로 사각형을 움직이는 최소 상태. 고정 스텝/보간/리플레이 결정성 검증용.
// ===========================================

class DemoState final : public IGameState
{
public:
    void Tick(GameContext& _ctx, const InputHistory& _input) override;
    void Render(GameContext& _ctx, float _alpha) override;
    [[nodiscard]] bool IsFinished() const override;

    [[nodiscard]] Vec2 GetPosition() const { return m_pos; }

private:
    static constexpr float kSpeedPerTick = 2.f;   // 120 px/s @ 60Hz

    Vec2 m_prevPos   = { 320.f, 224.f };
    Vec2 m_pos       = { 320.f, 224.f };
    bool         m_bFinished = false;
};

}   // namespace slug
