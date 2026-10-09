#include "Game/DemoState.h"
#include "Base/Math.h"

namespace slug
{

void DemoState::Tick(
    GameContext&,
    const InputHistory& _input)
{
    m_prevPos = m_pos;

    Vec2 move = { 0.f, 0.f };
    if (_input.IsHeld(eAction::Left))
    {
        move.x -= kSpeedPerTick;
    }
    if (_input.IsHeld(eAction::Right))
    {
        move.x += kSpeedPerTick;
    }
    if (_input.IsHeld(eAction::Up))
    {
        move.y -= kSpeedPerTick;
    }
    if (_input.IsHeld(eAction::Down))
    {
        move.y += kSpeedPerTick;
    }
    m_pos += move;

    if (_input.WasPressed(eAction::Bomb))
    {
        m_bFinished = true;
    }
}

void DemoState::Render(
    GameContext&      _ctx,
    const float       _alpha)
{
    Renderer& renderer = *_ctx.pRenderer;
    renderer.BeginFrame({ 24, 28, 40, 255 });

    const Vec2 drawPos = m_prevPos + (m_pos - m_prevPos) * _alpha;
    renderer.FillRect({ drawPos.x - 12.f, drawPos.y - 12.f }, { 24.f, 24.f }, { 240, 200, 60, 255 });
    renderer.DrawRect({ 1.f, 1.f }, { Renderer::kLogicalWidth - 2.f, Renderer::kLogicalHeight - 2.f }, { 90, 100, 130, 255 });
}

bool DemoState::IsFinished() const
{
    return m_bFinished;
}
}
