#pragma once
#include "Base/Types.h"
#include "Core/Input.h"
#include "Core/Tuning.h"

struct PlayerCommand
{
    int  moveX         = 0;
    bool bUpHeld       = false;
    bool bDownHeld     = false;
    bool bShotHeld     = false;
    bool bJumpBuffered = false;
    bool bShotBuffered = false;
    bool bBombBuffered = false;
};

class PlayerInputBuffer
{
public:
    void Reset();

    void Update(const InputHistory& _history);

    [[nodiscard]] bool IsBuffered(eAction _action, int _windowTicks) const;
    void               Consume(eAction _action);

    [[nodiscard]] PlayerCommand Read(const InputHistory& _history, const MoveTuning& _move) const;

private:
    static constexpr int kNever = 1'000'000;

    DIRECT_ENUM_ARRAY<eAction, int> m_ticksSincePress = {};
};
