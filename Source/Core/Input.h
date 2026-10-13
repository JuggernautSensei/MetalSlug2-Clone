#pragma once
#include <cstddef>
#include <cstdint>
#include <SDL3/SDL_scancode.h>

#include "Base/Types.h"

enum class eAction : uint16_t
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
    IndexedFlags<eAction> held = {};

    [[nodiscard]] bool IsHeld(eAction _action) const;

    void Set(eAction _action, bool _bHeld);

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

    ARRAY<bool, kKeyCapacity> m_keyDown = {};
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

    [[nodiscard]] bool WasPressedWithin(eAction _action, int _frames) const;

private:
    ARRAY<InputFrame, kCapacity> m_frames = {};
    size_t                       m_head   = 0;
    size_t                       m_count  = 0;
};
