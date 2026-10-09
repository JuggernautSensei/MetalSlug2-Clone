#include "Game/TitleState.h"

#include "Game/StateFactory.h"
#include "Game/Ui/Hud.h"

namespace slug
{

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

jug::Scoped<IGameState> TitleState::CreateNext_() const
{
    if (!m_config.nextState.empty())
    {
        return CreateState(m_config.nextState);
    }
    if (jug::Scoped<IGameState> pStage = CreateState("stage0"))
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
        m_selected  = m_config.nextState.empty() ? jug::String { CreateState("stage0") ? "stage0" : "sandbox" } : m_config.nextState;
        _ctx.GetAudio().Play(sfx::kOk);
        m_ui.GetFade().FadeToBlack(m_config.startFadeTicks);
    }
}

jug::Scoped<IGameState> TitleState::TakeNextState()
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
    (void)m_ui.DrawStandalone(_ctx, "TitleBackground", 0, layout.titleBackgroundY);

    // PRESS START: 시작 후에는 빠르게 깜빡인다
    const int  period = m_bStarting ? 4 : layout.titleBlinkTicks;
    const bool bOn    = IsBlinkOn(m_ui.GetTick(), period);
    if (bOn)
    {
        if (!m_ui.DrawSprite(_ctx, "PressStartUI", layout.screenW / 2, layout.titlePressStartY))
        {
            // 에셋이 없을 때 (헤드리스 외 비정상 환경) 디버그 폰트로 대체
            renderer.DrawDebugText({ 252.f, static_cast<float>(layout.titlePressStartY) - 4.f }, "PRESS START", { 255, 255, 255, 255 });
        }
    }

    TextStyle hint;
    hint.color = eFontColor::Yellow;
    hint.bBold = false;
    hint.align = eTextAlign::Center;
    m_ui.DrawText(_ctx, layout.screenW / 2, layout.titleHintY, "PRESS ENTER OR SPACE", hint);

    m_ui.DrawOverlays(_ctx);
}

}   // namespace slug
