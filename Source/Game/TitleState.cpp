#include "Game/TitleState.h"

#include "Audio/AudioSystem.h"
#include "Base/RGBA.h"
#include "Base/Types.h"
#include "Game/StateFactory.h"
#include "Game/Ui/Hud.h"
#include "Render/Renderer.h"

constexpr int kGradientBandH = 8;

namespace
{
[[nodiscard]] uint8_t MixChannel_(
    uint8_t _a,
    uint8_t _b,
    float   _t)
{
    return static_cast<uint8_t>(_a + static_cast<int>((_b - _a) * _t));
}

void DrawBackground_(
    Renderer& _renderer)
{
    const RGBA top    = { 14, 18, 44, 255 };
    const RGBA bottom = { 58, 22, 14, 255 };
    for (int y = 0; y < Renderer::kLogicalHeight; y += kGradientBandH)
    {
        const float t     = static_cast<float>(y) / static_cast<float>(Renderer::kLogicalHeight);
        const RGBA  color = { MixChannel_(top.r, bottom.r, t), MixChannel_(top.g, bottom.g, t), MixChannel_(top.b, bottom.b, t), 255 };
        _renderer.FillRect({ 0.f, static_cast<float>(y) }, { static_cast<float>(Renderer::kLogicalWidth), static_cast<float>(kGradientBandH) }, color);
    }
}
}   // namespace

TitleState::TitleState() = default;

TitleState::TitleState(
    TitleConfig _config)
    : m_config(std::move(_config))
{
}

void TitleState::OnEnter()
{
    m_ui.Init();
    m_tickCount = 0;
    m_bStarting = false;
    m_selected.clear();
    m_pNext = nullptr;

    m_ui.GetFade().SetBlack();
    m_ui.GetFade().FadeToClear(m_config.fadeInTicks);

    AudioSystem::GetSingleton().StopBgm();
    if (m_config.bPlayVoice)
    {
        AudioSystem::GetSingleton().Play(kTitleVoice);
    }
}

void TitleState::OnExit()
{
    AudioSystem::GetSingleton().AllStop();
}

Scoped<IGameState> TitleState::CreateNext_() const
{
    if (!m_config.nextState.empty())
    {
        return CreateState(m_config.nextState);
    }
    if (Scoped<IGameState> pStage = CreateState("stage0"))
    {
        return pStage;
    }
    return CreateState("sandbox");
}

void TitleState::Tick(
    const InputHistory& _input)
{
    ++m_tickCount;
    m_ui.Tick();

    if (m_bStarting)
    {
        if (!m_ui.GetFade().IsBusy() && !m_pNext)
        {
            m_pNext = CreateNext_();
        }
        return;
    }

    const bool bStart = _input.WasPressed(eAction::Start) || _input.WasPressed(eAction::Shot);
    if (bStart && m_tickCount > m_config.inputDelayTicks)
    {
        m_bStarting = true;
        m_selected  = m_config.nextState.empty() ? String { CreateState("stage0") ? "stage0" : "sandbox" } : m_config.nextState;
        AudioSystem::GetSingleton().Play(kOk);
        m_ui.GetFade().FadeToBlack(m_config.startFadeTicks);
    }
}

Scoped<IGameState> TitleState::TakeNextState()
{
    return std::move(m_pNext);
}

void TitleState::Render(
    const float)
{
    Renderer&       renderer = Renderer::GetSingleton();
    const UiLayout& layout   = m_ui.GetLayout();

    renderer.BeginFrame({ 0, 0, 0, 255 });
    if (!m_ui.DrawStandalone("TitleBackground", 0, 0))
    {
        DrawBackground_(renderer);
    }

    const int period = m_bStarting ? 4 : layout.titleBlinkTicks;
    if (IsBlinkOn(m_ui.GetTick(), period))
    {
        TextStyle start;
        start.color = eFontColor::White;
        start.align = eTextAlign::Center;
        m_ui.DrawText(layout.screenW / 2, layout.titlePressStartY, "PRESS START", start);
    }

    TextStyle hint;
    hint.color = eFontColor::Yellow;
    hint.bBold = false;
    hint.align = eTextAlign::Center;
    m_ui.DrawText(layout.screenW / 2, layout.titleHintY, "PRESS ENTER OR SPACE", hint);

    m_ui.DrawOverlays();
}

bool TitleState::IsStarting() const
{
    return m_bStarting;
}

int TitleState::GetTickCount() const
{
    return m_tickCount;
}

const String& TitleState::GetSelectedStateName() const
{
    return m_selected;
}

const UiSystem& TitleState::GetUi() const
{
    return m_ui;
}
