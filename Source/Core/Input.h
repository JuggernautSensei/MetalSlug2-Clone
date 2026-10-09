#pragma once
#include <JugX/KeyCode.h>

namespace slug
{

// ===========================================
//  게임 액션
// ===========================================

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
    Count
};

inline constexpr size_t kActionCount = static_cast<size_t>(eAction::Count);

// ===========================================
//  InputFrame
//   고정 스텝 1틱 동안의 입력 스냅샷. 리플레이는 이 값만 기록하면 된다.
// ===========================================

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

// ===========================================
//  InputSystem
//   SDL 키 이벤트를 모아 두었다가 스텝마다 Sample() 로 액션 상태를 만든다.
//   엣지(눌림/뗌) 판정은 InputHistory 가 연속된 InputFrame 으로 계산한다.
// ===========================================

class InputSystem
{
public:
    void OnKeyDown(jug::eKey _key);
    void OnKeyUp(jug::eKey _key);
    void ClearAll();

    [[nodiscard]] InputFrame Sample() const;

private:
    static constexpr size_t kKeyCapacity = 512;

    [[nodiscard]] bool IsKeyDown_(jug::eKey _key) const;

    jug::ARRAY<bool, kKeyCapacity> m_keyDown = {};
};

// ===========================================
//  InputHistory
//   최근 N 스텝의 입력을 보관하여 엣지/버퍼링 질의를 제공한다.
// ===========================================

class InputHistory
{
public:
    static constexpr size_t kCapacity = 16;

    void Push(InputFrame _frame);
    void Clear();

    [[nodiscard]] const InputFrame& GetCurrent() const;

    [[nodiscard]] bool IsHeld(eAction _action) const;
    [[nodiscard]] bool WasPressed(eAction _action) const;    // 이번 스텝에 눌림
    [[nodiscard]] bool WasReleased(eAction _action) const;   // 이번 스텝에 뗌

    // 최근 _frames 스텝 이내에 눌림이 있었는가 (입력 버퍼)
    [[nodiscard]] bool WasPressedWithin(eAction _action, int _frames) const;

    // 눌림 버퍼 소비 (같은 눌림이 두 번 처리되지 않도록 현재까지의 눌림 기록을 지운다)
    void ConsumePress(eAction _action);

private:
    [[nodiscard]] const InputFrame& GetAgo_(size_t _ago) const;

    jug::ARRAY<InputFrame, kCapacity> m_frames          = {};
    jug::ARRAY<uint16_t, kCapacity>   m_consumedPresses = {};   // 스텝별 소비된 눌림 마스크
    size_t                            m_head            = 0;
    size_t                            m_count           = 0;
};

}   // namespace slug
