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
    Banner,
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
    int         bannerTicks     = 120;
    int         fadeTicks       = 40;
    int         holdTicks       = 20;

    [[nodiscard]] int GetTotalTicks() const { return delayTicks + bannerTicks + fadeTicks + holdTicks; }
};

[[nodiscard]] ScreenFlowConfig MakeGameOverFlowConfig();
[[nodiscard]] ScreenFlowConfig MakeMissionCompleteFlowConfig();

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

private:
    void BeginBanner_(UiSystem& _ui, IAudioSink& _audio);
    void BeginFade_(UiSystem& _ui);
    void Advance_(UiSystem& _ui, IAudioSink& _audio);

    ScreenFlowConfig m_config  = {};
    eFlowPhase       m_phase   = eFlowPhase::Idle;
    int              m_elapsed = 0;
};
}
