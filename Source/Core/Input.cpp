#include "Core/Input.h"

struct KeyBinding
{
    eAction      action;
    SDL_Scancode key;
};

// clang-format off
constexpr ARRAY<KeyBinding, 18> kBindings = { {
    {  eAction::Left,        SDL_SCANCODE_A },
    {  eAction::Left,     SDL_SCANCODE_LEFT },
    { eAction::Right,        SDL_SCANCODE_D },
    { eAction::Right,    SDL_SCANCODE_RIGHT },
    {    eAction::Up,        SDL_SCANCODE_W },
    {    eAction::Up,       SDL_SCANCODE_UP },
    {  eAction::Down,        SDL_SCANCODE_S },
    {  eAction::Down,     SDL_SCANCODE_DOWN },
    {  eAction::Shot,        SDL_SCANCODE_J },
    {  eAction::Shot,        SDL_SCANCODE_Z },
    {  eAction::Jump,        SDL_SCANCODE_K },
    {  eAction::Jump,        SDL_SCANCODE_X },
    {  eAction::Bomb,        SDL_SCANCODE_L },
    {  eAction::Bomb,        SDL_SCANCODE_C },
    { eAction::Pause,   SDL_SCANCODE_ESCAPE },
    { eAction::Start,   SDL_SCANCODE_RETURN },
    { eAction::Start, SDL_SCANCODE_KP_ENTER },
    { eAction::Start,    SDL_SCANCODE_SPACE },
} };
// clang-format on

void InputSystem::OnKeyDown(
    const SDL_Scancode _key)
{
    const size_t index = static_cast<size_t>(_key);
    if (index < kKeyCapacity)
    {
        m_keyDown[index] = true;
    }
}

void InputSystem::OnKeyUp(
    const SDL_Scancode _key)
{
    const size_t index = static_cast<size_t>(_key);
    if (index < kKeyCapacity)
    {
        m_keyDown[index] = false;
    }
}

void InputSystem::ClearAll()
{
    m_keyDown.fill(false);
}

bool InputSystem::IsKeyDown_(
    const SDL_Scancode _key) const
{
    const size_t index = static_cast<size_t>(_key);
    return index < kKeyCapacity && m_keyDown[index];
}

InputFrame InputSystem::Sample() const
{
    InputFrame frame;
    for (const KeyBinding& binding: kBindings)
    {
        if (IsKeyDown_(binding.key))
        {
            frame.Set(binding.action, true);
        }
    }
    return frame;
}

void InputHistory::Push(
    const InputFrame _frame)
{
    m_head           = (m_head + 1) % kCapacity;
    m_frames[m_head] = _frame;
    m_count          = m_count < kCapacity ? m_count + 1 : kCapacity;
}

void InputHistory::Clear()
{
    m_frames.fill({});
    m_head  = 0;
    m_count = 0;
}

const InputFrame& InputHistory::GetCurrent() const
{
    return m_frames[m_head];
}

const InputFrame& InputHistory::GetAgo_(
    const size_t _ago) const
{
    return m_frames[(m_head + kCapacity - (_ago % kCapacity)) % kCapacity];
}

bool InputHistory::IsHeld(
    const eAction _action) const
{
    return GetCurrent().IsHeld(_action);
}

bool InputHistory::WasPressed(
    const eAction _action) const
{
    return WasPressedWithin(_action, 1);
}

bool InputHistory::WasPressedWithin(
    const eAction _action,
    const int     _frames) const
{
    const size_t   window = static_cast<size_t>(_frames < 1 ? 1 : (_frames > static_cast<int>(kCapacity) - 1 ? static_cast<int>(kCapacity) - 1 : _frames));
    const uint16_t mask   = static_cast<uint16_t>(1u << static_cast<unsigned>(_action));

    for (size_t ago = 0; ago < window && ago < m_count; ++ago)
    {
        const size_t index   = (m_head + kCapacity - ago) % kCapacity;
        const bool   bNow    = (m_frames[index].held & mask) != 0;
        const bool   bBefore = (ago + 1 < m_count) ? ((m_frames[(index + kCapacity - 1) % kCapacity].held & mask) != 0) : false;
        if (bNow && !bBefore)
        {
            return true;
        }
    }
    return false;
}

bool InputFrame::IsHeld(
    const eAction _action) const
{
    return ((held >> static_cast<unsigned>(_action)) & 1u) != 0;
}

void InputFrame::Set(
    const eAction _action,
    const bool    _bHeld)
{
    const uint16_t mask = static_cast<uint16_t>(1u << static_cast<unsigned>(_action));
    held                = _bHeld ? static_cast<uint16_t>(held | mask) : static_cast<uint16_t>(held & ~mask);
}
