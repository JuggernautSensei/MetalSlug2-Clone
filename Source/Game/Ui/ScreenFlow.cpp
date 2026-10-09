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
    config.bannerTicks    = 120;
    config.fadeTicks      = 40;
    config.holdTicks      = 20;
    return config;
}

ScreenFlowConfig MakeMissionCompleteFlowConfig()
{
    ScreenFlowConfig config;
    config.bannerText     = "MISSION COMPLETE";
    config.bannerColor    = eFontColor::Yellow;
    config.sfx            = sfx::kMissionComplete;
    config.bgmFadeSeconds = 0.f;
    config.delayTicks     = 90;
    config.bannerTicks    = 240;
    config.fadeTicks      = 60;
    config.holdTicks      = 15;
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

void ScreenFlow::BeginBanner_(
    UiSystem&   _ui,
    IAudioSink& _audio)
{
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
    const int bannerStart = m_config.delayTicks;
    const int fadeStart   = bannerStart + m_config.bannerTicks;
    const int holdStart   = fadeStart + m_config.fadeTicks;
    const int doneAt      = holdStart + m_config.holdTicks;

    if (m_phase == eFlowPhase::Delay && m_elapsed >= bannerStart)
    {
        BeginBanner_(_ui, _audio);
    }
    if (m_phase == eFlowPhase::Banner && m_elapsed >= fadeStart)
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
