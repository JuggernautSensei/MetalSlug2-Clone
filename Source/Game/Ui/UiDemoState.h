#pragma once
#include "Game/GameState.h"
#include "Game/Ui/ScreenFlow.h"
#include "Game/Ui/UiSystem.h"

namespace slug
{

// ===========================================
//  UiDemoState ("uidemo")
//   HUD / 배너 / 페이드 / GAME OVER 흐름을 정해진 틱 스크립트로 보여 주는 QA 상태 (플레이 불가).
//   결정적 캡처:  Slug.exe --state uidemo --capture <prefix> --capture-ticks 5,19,100,...
//
//   타임라인 (틱):
//      0   검정에서 20틱 페이드 인
//     10   배너 "MISSION 4 START" (3초)
//    260   배너 "WARNING" (붉은색, 2초) + 보스 HP 바 등장
//    420   배너 "MISSION 4\nCOMPLETE" 두 줄
//    560   GAME OVER 흐름 시작 (배너 2초 → 암전 → 완료)
//    740   이후 IsFinished() == true
// ===========================================

class UiDemoState final : public IGameState
{
public:
    void OnEnter(GameContext& _ctx) override;
    void Tick(GameContext& _ctx, const InputHistory& _input) override;
    void Render(GameContext& _ctx, float _alpha) override;

    [[nodiscard]] bool IsFinished() const override { return m_flow.IsDone(); }

    [[nodiscard]] int                GetTickCount() const { return m_tick; }
    [[nodiscard]] const ScreenFlow&  GetFlow() const { return m_flow; }
    [[nodiscard]] const UiSystem&    GetUi() const { return m_ui; }

private:
    [[nodiscard]] HudModel MakeModel_() const;

    UiSystem   m_ui   = {};
    ScreenFlow m_flow = {};
    int        m_tick = 0;
};

}   // namespace slug
