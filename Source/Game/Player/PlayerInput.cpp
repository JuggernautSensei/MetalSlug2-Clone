#include "Game/Player/PlayerInput.h"

void PlayerInputBuffer::Reset()
{
    m_ticksSincePress.Fill(kNever);
}

void PlayerInputBuffer::Update(
    const InputHistory& _history)
{
    for (size_t i = 0; i < kActionCount; ++i)
    {
        const eAction action = static_cast<eAction>(i);
        if (_history.WasPressed(action))
        {
            m_ticksSincePress[action] = 0;
        }
        else if (m_ticksSincePress[action] < kNever)
        {
            ++m_ticksSincePress[action];
        }
    }
}

bool PlayerInputBuffer::IsBuffered(
    eAction _action,
    int     _windowTicks) const
{
    return m_ticksSincePress[_action] < _windowTicks;
}

void PlayerInputBuffer::Consume(
    eAction _action)
{
    m_ticksSincePress[_action] = kNever;
}

PlayerCommand PlayerInputBuffer::Read(
    const InputHistory& _history,
    const MoveTuning&   _move) const
{
    PlayerCommand command;

    const bool bLeft  = _history.IsHeld(eAction::Left);
    const bool bRight = _history.IsHeld(eAction::Right);
    command.moveX     = (bRight ? 1 : 0) - (bLeft ? 1 : 0);

    command.bUpHeld   = _history.IsHeld(eAction::Up);
    command.bDownHeld = _history.IsHeld(eAction::Down);
    command.bShotHeld = _history.IsHeld(eAction::Shot);

    command.bJumpBuffered = IsBuffered(eAction::Jump, _move.jumpBufferTicks);
    command.bShotBuffered = IsBuffered(eAction::Shot, _move.shotBufferTicks);
    command.bBombBuffered = IsBuffered(eAction::Bomb, _move.bombBufferTicks);
    return command;
}
