#pragma once
#include <cstdint>

#include "Audio/AudioSystem.h"
#include "Base/Types.h"
#include "Game/Ui/BitmapFont.h"
#include "Game/Ui/UiSystem.h"

enum class eFlowPhase : uint8_t
{
    Idle,
    Delay,
    Results,
    Banner,
    Art,
    FadeOut,
    Hold,
    Done
};

struct ScreenFlowConfig
{
    String     bannerText     = {};
    eFontColor bannerColor    = eFontColor::Yellow;
    String     sfx            = {};
    float      bgmFadeSeconds = 1.f;
    int        delayTicks     = 0;
    int        resultsTicks   = 0;   // 0 이면 결과 상자 단계를 건너뛴다
    int        rescuedCount   = 0;
    int        bannerTicks    = 120;
    int        fadeTicks      = 40;
    int        holdTicks      = 20;
    String     artSprite      = {};   // 비어 있지 않으면 배너 대신 전면 아트를 bannerTicks 동안 보인다
    int        artFadeInTicks = 0;

    [[nodiscard]] int GetTotalTicks() const;
};

[[nodiscard]] StringView         MissionRankText(int _rescued);
[[nodiscard]] int                MissionRankBonus(int _rescued);
[[nodiscard]] int                MissionBonusScore(int _rescued);
[[nodiscard]] int                MissionResultsTicks(int _rescued);
[[nodiscard]] MissionResultsView MakeMissionResultsView(int _rescued, int _localTicks);

[[nodiscard]] ScreenFlowConfig MakeGameOverFlowConfig();
[[nodiscard]] ScreenFlowConfig MakeMissionCompleteFlowConfig(int _rescued = 0);

class ScreenFlow
{
public:
    void Start(const ScreenFlowConfig& _config, UiSystem& _ui, AudioSystem& _audio);
    void Tick(UiSystem& _ui, AudioSystem& _audio);
    void Reset();

    [[nodiscard]] eFlowPhase         GetPhase() const;
    [[nodiscard]] bool               IsRunning() const;
    [[nodiscard]] bool               IsDone() const;
    [[nodiscard]] int                GetElapsedTicks() const;
    [[nodiscard]] MissionResultsView GetResultsView() const;
    [[nodiscard]] GameOverArtView    GetArtView() const;

private:
    void BeginBanner_(UiSystem& _ui, AudioSystem& _audio);
    void BeginFade_(UiSystem& _ui);
    void Advance_(UiSystem& _ui, AudioSystem& _audio);

    ScreenFlowConfig m_config  = {};
    eFlowPhase       m_phase   = eFlowPhase::Idle;
    int              m_elapsed = 0;
};
