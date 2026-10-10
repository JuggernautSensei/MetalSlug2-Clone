#include "Game/Ui/ScreenFlow.h"

#include <algorithm>

namespace slug
{

ScreenFlowConfig MakeGameOverFlowConfig()
{
    ScreenFlowConfig config;
    config.bannerText     = "GAME OVER";
    config.bannerColor    = eFontColor::Red;
    config.sfx            = sfx::kGameOver;
    config.bgmFadeSeconds = 0.5f;
    config.delayTicks     = 0;
    config.artSprite      = "UI_GameOver_Helmet";
    config.artFadeInTicks = 30;                           // 0.5 s 페이드 인
    config.bannerTicks    = config.artFadeInTicks + 180;   // 페이드 인 + 3 s 홀드
    config.fadeTicks      = 40;
    config.holdTicks      = 20;
    return config;
}

namespace
{
    constexpr int kResultsFadeTicks  = 12;    // 상자 페이드 인
    constexpr int kTicksPerPow       = 6;     // 구출 수 한 명 집계 간격 (0.1초)
    constexpr int kRankDelayTicks    = 18;    // 집계 끝 -> 보너스/등급 표시
    constexpr int kRankHoldTicks     = 150;   // 등급 표시 후 대기 (2.5초)
    constexpr int kPowBonusScore     = 10000;
}

slug::StringView MissionRankText(
    const int _rescued)
{
    if (_rescued >= 15)
    {
        return "GREAT!";
    }
    if (_rescued >= 8)
    {
        return "GOOD!";
    }
    return {};
}

int MissionRankBonus(
    const int _rescued)
{
    if (_rescued >= 15)
    {
        return 220000;
    }
    if (_rescued >= 8)
    {
        return 100000;
    }
    return 0;
}

int MissionBonusScore(
    const int _rescued)
{
    const int rescued = std::max(0, _rescued);
    return rescued * kPowBonusScore + MissionRankBonus(rescued);
}

int MissionResultsTicks(
    const int _rescued)
{
    return kResultsFadeTicks + std::max(0, _rescued) * kTicksPerPow + kRankDelayTicks + kRankHoldTicks;
}

MissionResultsView MakeMissionResultsView(
    const int _rescued,
    const int _localTicks)
{
    const int rescued = std::max(0, _rescued);
    MissionResultsView view;
    if (_localTicks < 0 || _localTicks >= MissionResultsTicks(rescued))
    {
        return view;
    }

    view.bBoxVisible = true;
    view.boxAlpha    = static_cast<uint8_t>(std::min(255, _localTicks * 255 / kResultsFadeTicks));

    const int countEnd = kResultsFadeTicks + rescued * kTicksPerPow;
    const int rankAt   = countEnd + kRankDelayTicks;
    if (_localTicks >= kResultsFadeTicks)
    {
        view.rescued = std::min(rescued, (_localTicks - kResultsFadeTicks) / kTicksPerPow + 1);
    }
    view.bCounting  = _localTicks >= kResultsFadeTicks && _localTicks < countEnd;
    view.bonusScore = _localTicks < rankAt ? view.rescued * kPowBonusScore : MissionBonusScore(rescued);
    if (_localTicks >= rankAt && !MissionRankText(rescued).empty())
    {
        view.bRankVisible = true;
        view.rankText     = slug::String { MissionRankText(rescued) };
    }
    return view;
}

ScreenFlowConfig MakeMissionCompleteFlowConfig(
    const int _rescued)
{
    ScreenFlowConfig config;
    config.bannerText     = "MISSION 4\nCOMPLETE!";
    config.bannerColor    = eFontColor::Yellow;
    config.sfx            = sfx::kMissionComplete;
    config.bgmFadeSeconds = 0.f;
    config.delayTicks     = 60;
    config.rescuedCount   = std::max(0, _rescued);
    config.resultsTicks   = MissionResultsTicks(config.rescuedCount);
    config.bannerTicks    = 210;
    config.fadeTicks      = 90;
    config.holdTicks      = 30;
    return config;
}

void ScreenFlow::Reset()
{
    m_config  = {};
    m_phase   = eFlowPhase::Idle;
    m_elapsed = 0;
}

void ScreenFlow::Start(
    const ScreenFlowConfig& _config,
    UiSystem&               _ui,
    IAudioSink&             _audio)
{
    m_config  = _config;
    m_elapsed = 0;
    m_phase   = eFlowPhase::Delay;

    if (m_config.bgmFadeSeconds > 0.f)
    {
        _audio.FadeOutBgm(m_config.bgmFadeSeconds);
    }
    else
    {
        _audio.StopBgm();
    }

    Advance_(_ui, _audio);
}

void ScreenFlow::Tick(
    UiSystem&   _ui,
    IAudioSink& _audio)
{
    if (!IsRunning())
    {
        return;
    }
    ++m_elapsed;
    Advance_(_ui, _audio);
}

MissionResultsView ScreenFlow::GetResultsView() const
{
    if (m_phase != eFlowPhase::Results)
    {
        return {};
    }
    return MakeMissionResultsView(m_config.rescuedCount, m_elapsed - m_config.delayTicks);
}

GameOverArtView ScreenFlow::GetArtView() const
{
    GameOverArtView view;
    if (m_config.artSprite.empty() || m_phase == eFlowPhase::Idle || m_phase == eFlowPhase::Done || m_phase == eFlowPhase::Delay || m_phase == eFlowPhase::Results)
    {
        return view;
    }
    view.bVisible = true;
    view.sprite   = m_config.artSprite;
    view.alpha    = 255;
    if (m_phase == eFlowPhase::Art)
    {
        const int localTicks = m_elapsed - (m_config.delayTicks + m_config.resultsTicks);
        if (m_config.artFadeInTicks > 0)
        {
            view.alpha = static_cast<uint8_t>(std::clamp(localTicks * 255 / m_config.artFadeInTicks, 0, 255));
        }
    }
    return view;
}

void ScreenFlow::BeginBanner_(
    UiSystem&   _ui,
    IAudioSink& _audio)
{
    if (!m_config.artSprite.empty())
    {
        m_phase = eFlowPhase::Art;
        if (!m_config.sfx.empty())
        {
            _audio.Play(m_config.sfx);
        }
        return;
    }
    m_phase = eFlowPhase::Banner;

    BannerSpec spec  = MakeBannerSpec(m_config.bannerText, static_cast<float>(m_config.bannerTicks) / 60.f);
    spec.fadeInTicks  = std::min(18, m_config.bannerTicks / 4);
    spec.fadeOutTicks = spec.fadeInTicks;
    spec.holdTicks    = std::max(0, m_config.bannerTicks - 2 * spec.fadeInTicks);
    spec.color        = m_config.bannerColor;
    _ui.GetBanner().Show(std::move(spec));

    if (!m_config.sfx.empty())
    {
        _audio.Play(m_config.sfx);
    }
}

void ScreenFlow::BeginFade_(
    UiSystem& _ui)
{
    m_phase = eFlowPhase::FadeOut;
    _ui.GetFade().FadeToBlack(m_config.fadeTicks);
}

void ScreenFlow::Advance_(
    UiSystem&   _ui,
    IAudioSink& _audio)
{
    const int resultsStart = m_config.delayTicks;
    const int bannerStart  = resultsStart + m_config.resultsTicks;
    const int fadeStart    = bannerStart + m_config.bannerTicks;
    const int holdStart    = fadeStart + m_config.fadeTicks;
    const int doneAt       = holdStart + m_config.holdTicks;

    if (m_phase == eFlowPhase::Delay && m_elapsed >= resultsStart)
    {
        if (m_config.resultsTicks > 0)
        {
            m_phase = eFlowPhase::Results;
        }
        else
        {
            BeginBanner_(_ui, _audio);
        }
    }
    if (m_phase == eFlowPhase::Results && m_elapsed >= bannerStart)
    {
        BeginBanner_(_ui, _audio);
    }
    if ((m_phase == eFlowPhase::Banner || m_phase == eFlowPhase::Art) && m_elapsed >= fadeStart)
    {
        BeginFade_(_ui);
    }
    if (m_phase == eFlowPhase::FadeOut && m_elapsed >= holdStart)
    {
        m_phase = eFlowPhase::Hold;
    }
    if (m_phase == eFlowPhase::Hold && m_elapsed >= doneAt)
    {
        m_phase = eFlowPhase::Done;
    }
}
}
