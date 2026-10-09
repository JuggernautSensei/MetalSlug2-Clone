#pragma once
#include "Base/Types.h"

#include "Game/GameState.h"
#include "Game/Ui/UiSystem.h"

namespace slug
{

// ===========================================
//  TitleState ("title")
//   TitleBackground(1280x720 → 640x360 로 구운 이미지) + 점멸 PRESS START.
//   Enter/Space(eAction::Start) 또는 Shot 으로 시작: 효과음 → 암전 → 다음 상태.
//   다음 상태 = config.nextState, 비어 있으면 "stage0" 이 StateFactory 에 있을 때 "stage0", 없으면 "sandbox".
// ===========================================

struct TitleConfig
{
    slug::String nextState      = {};    // 비면 자동 선택 (stage0 → sandbox)
    int         fadeInTicks    = 24;    // 처음 화면이 밝아지는 시간
    int         startFadeTicks = 24;    // 시작 입력 후 암전 시간
    int         inputDelayTicks = 12;   // 진입 직후 이 틱 동안은 시작 입력을 무시 (이전 장면의 눌림 방지)
    bool        bPlayVoice     = true;  // 진입 시 타이틀 음성
};

class TitleState final : public IGameState
{
public:
    TitleState();
    explicit TitleState(TitleConfig _config);

    void OnEnter(GameContext& _ctx) override;
    void OnExit(GameContext& _ctx) override;
    void Tick(GameContext& _ctx, const InputHistory& _input) override;
    void Render(GameContext& _ctx, float _alpha) override;

    [[nodiscard]] std::unique_ptr<IGameState> TakeNextState() override;

    // --- 테스트/도구용 ---
    [[nodiscard]] bool                     IsStarting() const { return m_bStarting; }
    [[nodiscard]] int                      GetTickCount() const { return m_tickCount; }
    [[nodiscard]] const slug::String&       GetSelectedStateName() const { return m_selected; }
    [[nodiscard]] const UiSystem&          GetUi() const { return m_ui; }

private:
    [[nodiscard]] std::unique_ptr<IGameState> CreateNext_() const;

    TitleConfig m_config = {};
    UiSystem    m_ui     = {};

    int         m_tickCount = 0;
    bool        m_bStarting = false;
    slug::String m_selected  = {};
    std::unique_ptr<IGameState> m_pNext = nullptr;
};

}   // namespace slug
