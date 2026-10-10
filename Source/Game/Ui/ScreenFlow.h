#pragma once
#include <cstdint>

#include "Base/Types.h"

#include "Audio/AudioSink.h"
#include "Game/Ui/BitmapFont.h"
#include "Game/Ui/UiSystem.h"

namespace slug
{
enum class eFlowPhase : uint8_t
{
    Idle,
    Delay,
    Results,      // 구출 결과 상자 (RECAPTURED PRISONER 집계)
    Banner,
    Art,          // 전면 아트 (게임 오버 샌드 아트): 페이드 인 후 홀드
    FadeOut,
    Hold,
    Done
};

struct ScreenFlowConfig
{
    slug::String bannerText      = {};
    eFontColor  bannerColor     = eFontColor::Yellow;
    slug::String sfx             = {};
    float       bgmFadeSeconds  = 1.f;    // Start() 시 BGM 페이드 아웃
    int         delayTicks      = 0;
    int         resultsTicks    = 0;      // 0 이면 결과 상자 단계를 건너뛴다
    int         rescuedCount    = 0;      // 구출한 POW 수 (결과 상자 집계용)
    int         bannerTicks     = 120;
    int         fadeTicks       = 40;
    int         holdTicks       = 20;
    slug::String artSprite      = {};     // 비어 있지 않으면 배너 대신 전면 아트를 bannerTicks 동안 보인다 (앞 artFadeInTicks 는 페이드 인)
    int         artFadeInTicks  = 0;

    [[nodiscard]] int GetTotalTicks() const { return delayTicks + resultsTicks + bannerTicks + fadeTicks + holdTicks; }
};

// 구출 수에 따른 등급 문구와 보너스 (영상 기준: 16명 -> 380000, GREAT!)
[[nodiscard]] slug::StringView MissionRankText(int _rescued);
[[nodiscard]] int              MissionRankBonus(int _rescued);
[[nodiscard]] int              MissionBonusScore(int _rescued);   // 구출 x 10000 + 등급 보너스
[[nodiscard]] int              MissionResultsTicks(int _rescued);
[[nodiscard]] MissionResultsView MakeMissionResultsView(int _rescued, int _localTicks);

[[nodiscard]] ScreenFlowConfig MakeGameOverFlowConfig();
[[nodiscard]] ScreenFlowConfig MakeMissionCompleteFlowConfig(int _rescued = 0);

class ScreenFlow
{
public:
    void Start(const ScreenFlowConfig& _config, UiSystem& _ui, IAudioSink& _audio);
    void Tick(UiSystem& _ui, IAudioSink& _audio);
    void Reset();

    [[nodiscard]] eFlowPhase GetPhase() const { return m_phase; }
    [[nodiscard]] bool       IsRunning() const { return m_phase != eFlowPhase::Idle && m_phase != eFlowPhase::Done; }
    [[nodiscard]] bool       IsDone() const { return m_phase == eFlowPhase::Done; }
    [[nodiscard]] int        GetElapsedTicks() const { return m_elapsed; }
    [[nodiscard]] MissionResultsView GetResultsView() const;
    [[nodiscard]] GameOverArtView    GetArtView() const;

private:
    void BeginBanner_(UiSystem& _ui, IAudioSink& _audio);
    void BeginFade_(UiSystem& _ui);
    void Advance_(UiSystem& _ui, IAudioSink& _audio);

    ScreenFlowConfig m_config  = {};
    eFlowPhase       m_phase   = eFlowPhase::Idle;
    int              m_elapsed = 0;
};
}
