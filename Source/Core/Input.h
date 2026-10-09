#pragma once
#include <cstddef>
#include <cstdint>

#include <SDL3/SDL_scancode.h>

#include "Base/Types.h"

namespace slug
{
enum class eAction : uint8_t
{
    Left,
    Right,
    Up,
    Down,
    Shot,
    Jump,
    Bomb,
    Pause,
    Start,
    Count
};

inline constexpr size_t kActionCount = static_cast<size_t>(eAction::Count);

struct InputFrame
{
    uint16_t held = 0;

    [[nodiscard]] constexpr bool IsHeld(
        const eAction _action) const
    {
        return ((held >> static_cast<unsigned>(_action)) & 1u) != 0;
    }

    constexpr void Set(
        const eAction _action,
        const bool    _bHeld)
    {
        const uint16_t mask = static_cast<uint16_t>(1u << static_cast<unsigned>(_action));
        held                = _bHeld ? static_cast<uint16_t>(held | mask) : static_cast<uint16_t>(held & ~mask);
    }

    [[nodiscard]] constexpr bool operator==(const InputFrame&) const = default;
};

class InputSystem
{
public:
    void OnKeyDown(SDL_Scancode _key);
    void OnKeyUp(SDL_Scancode _key);
    void ClearAll();

    [[nodiscard]] InputFrame Sample() const;

private:
    static constexpr size_t kKeyCapacity = 512;

    [[nodiscard]] bool IsKeyDown_(SDL_Scancode _key) const;

    slug::ARRAY<bool, kKeyCapacity> m_keyDown = {};
};

class InputHistory
{
public:
    static constexpr size_t kCapacity = 16;

    void Push(InputFrame _frame);
    void Clear();

    [[nodiscard]] const InputFrame& GetCurrent() const;

    [[nodiscard]] bool IsHeld(eAction _action) const;
    [[nodiscard]] bool WasPressed(eAction _action) const;
    [[nodiscard]] bool WasReleased(eAction _action) const;

    [[nodiscard]] bool WasPressedWithin(eAction _action, int _frames) const;

    void ConsumePress(eAction _action);

private:
    [[nodiscard]] const InputFrame& GetAgo_(size_t _ago) const;

    slug::ARRAY<InputFrame, kCapacity> m_frames          = {};
    slug::ARRAY<uint16_t, kCapacity>   m_consumedPresses = {};
    size_t                            m_head            = 0;
    size_t                            m_count           = 0;
};
}
