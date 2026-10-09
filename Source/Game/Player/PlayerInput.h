#pragma once
#include <JugX/Typedef.h>

#include "Core/Input.h"
#include "Core/Tuning.h"

namespace slug
{

// ===========================================
//  PlayerCommand
//   이번 틱의 입력을 "플레이어가 하고 싶은 것"으로 해석한 값. 상태 갱신 전에 만들어지므로 지연이 없다.
// ===========================================

struct PlayerCommand
{
    int  moveX         = 0;   // -1, 0, +1 (좌우를 동시에 누르면 0)
    bool bUpHeld       = false;
    bool bDownHeld     = false;
    bool bJumpHeld     = false;
    bool bShotHeld     = false;
    bool bJumpBuffered = false;   // 버퍼 창 안의 눌림 (소비 전)
    bool bShotBuffered = false;
    bool bBombBuffered = false;
};

// ===========================================
//  PlayerInputBuffer
//   액션별 "마지막 눌림으로부터 지난 틱 수" 를 센다. 버퍼 창 안이면 눌림이 살아 있고, Consume 하면 지워진다.
//   InputHistory 는 const 로만 전달되므로 소비 상태는 이쪽에서 관리한다.
// ===========================================

class PlayerInputBuffer
{
public:
    void Reset();

    // 매 틱 가장 먼저 호출 (이번 틱의 눌림 엣지를 반영)
    void Update(const InputHistory& _history);

    [[nodiscard]] bool IsBuffered(eAction _action, int _windowTicks) const;
    void               Consume(eAction _action);

    // 이번 틱 입력 해석. 버퍼 창은 Tuning.move 의 값을 쓴다.
    [[nodiscard]] PlayerCommand Read(const InputHistory& _history, const MoveTuning& _move) const;

private:
    static constexpr int kNever = 1'000'000;

    jug::ARRAY<int, kActionCount> m_ticksSincePress = {};
};

}   // namespace slug
