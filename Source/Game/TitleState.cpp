#include "Game/TitleState.h"
#include "Base/Types.h"
#include "Base/RGBA.h"

#include "Game/StateFactory.h"
#include "Game/Ui/Hud.h"

namespace slug
{

namespace
{

    constexpr int kGradientBandH = 8;

    void DrawBackground_(
        Renderer& _renderer)
    {
        const slug::RGBA top    = { 14, 18, 44, 255 };
        const slug::RGBA bottom = { 58, 22, 14, 255 };
        for (int y = 0; y < Renderer::kLogicalHeight; y += kGradientBandH)
        {
            const float t     = static_cast<float>(y) / static_cast<float>(Renderer::kLogicalHeight);
            const auto  mix   = [t](const uint8_t _a, const uint8_t _b) { return static_cast<uint8_t>(_a + static_cast<int>((_b - _a) * t)); };
            const slug::RGBA color = { mix(top.r, bottom.r), mix(top.g, bottom.g), mix(top.b, bottom.b), 255 };
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

void TitleState::OnEnter(
    GameContext& _ctx)
{
    m_ui.Init(_ctx);
    m_tickCount = 0;
    m_bStarting = false;
    m_selected.clear();
    m_pNext = nullptr;

    // 검정에서 밝아지며 시작
    m_ui.GetFade().SetBlack();
    m_ui.GetFade().FadeToClear(m_config.fadeInTicks);

    _ctx.GetAudio().StopBgm();
    if (m_config.bPlayVoice)
    {
        _ctx.GetAudio().Play(sfx::kTitleVoice);
    }
}

void TitleState::OnExit(
    GameContext& _ctx)
{
    _ctx.GetAudio().AllStop();
}

std::unique_ptr<IGameState> TitleState::CreateNext_() const
{
    if (!m_config.nextState.empty())
    {
        return CreateState(m_config.nextState);
    }
    if (std::unique_ptr<IGameState> pStage = CreateState("stage0"))
    {
        return pStage;
    }
    return CreateState("sandbox");
}

void TitleState::Tick(
    GameContext&        _ctx,
    const InputHistory& _input)
{
    ++m_tickCount;
    m_ui.Tick();

    if (m_bStarting)
    {
        // 암전이 끝나면 다음 상태를 만든다 (SimRunner 가 Tick 직후 TakeNextState 로 가져간다)
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
        m_selected  = m_config.nextState.empty() ? slug::String { CreateState("stage0") ? "stage0" : "sandbox" } : m_config.nextState;
        _ctx.GetAudio().Play(sfx::kOk);
        m_ui.GetFade().FadeToBlack(m_config.startFadeTicks);
    }
}

std::unique_ptr<IGameState> TitleState::TakeNextState()
{
    return std::move(m_pNext);
}

void TitleState::Render(
    GameContext& _ctx,
    const float  /*_alpha*/)
{
    Renderer&       renderer = *_ctx.pRenderer;
    const UiLayout& layout   = m_ui.GetLayout();

    renderer.BeginFrame({ 0, 0, 0, 255 });
    DrawBackground_(renderer);

    constexpr const char* kTitleName = "MY METAL SLUG 2";
    TextStyle             title;
    title.color = eFontColor::YellowWhite;
    title.align = eTextAlign::Center;
    title.scale = FitTextScale(kTitleName, 600, 3);
    m_ui.DrawText(_ctx, layout.screenW / 2, layout.titleNameY, kTitleName, title);

    // PRESS START: 시작 후에는 빠르게 깜빡인다
    const int  period = m_bStarting ? 4 : layout.titleBlinkTicks;
    if (IsBlinkOn(m_ui.GetTick(), period))
    {
        TextStyle start;
        start.color = eFontColor::White;
        start.align = eTextAlign::Center;
        m_ui.DrawText(_ctx, layout.screenW / 2, layout.titlePressStartY, "PRESS START", start);
    }

    TextStyle hint;
    hint.color = eFontColor::Yellow;
    hint.bBold = false;
    hint.align = eTextAlign::Center;
    m_ui.DrawText(_ctx, layout.screenW / 2, layout.titleHintY, "PRESS ENTER OR SPACE", hint);

    m_ui.DrawOverlays(_ctx);
}

}   // namespace slug
