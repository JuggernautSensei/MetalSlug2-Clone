#include "Game/Ui/UiDemoState.h"

#include <algorithm>

namespace slug
{

void UiDemoState::OnEnter(
    GameContext& _ctx)
{
    m_ui.Init(_ctx);
    m_flow.Reset();
    m_tick = 0;
    m_ui.GetFade().SetBlack();
    m_ui.GetFade().FadeToClear(20);
}

HudModel UiDemoState::MakeModel_() const
{
    HudModel model;
    model.lives         = 2;
    model.score         = 1'234'560 + m_tick * 10;
    model.bInfiniteAmmo = m_tick < 300;   // 300틱 이후 HMG 탄 표시로 전환
    model.ammo          = std::max(0, 200 - (m_tick - 300) / 2);
    model.bombs         = 7;
    model.bShowTimer    = true;
    model.timerSeconds  = std::max(0, 60 - m_tick / 60);
    model.bShowBoss     = m_tick >= 260;
    model.bossHpRatio   = std::clamp(1.f - static_cast<float>(m_tick - 260) / 400.f, 0.f, 1.f);
    model.levelSprite   = "LEVEL_1";
    return model;
}

void UiDemoState::Tick(
    GameContext& _ctx,
    const InputHistory&)
{
    ++m_tick;
    m_ui.Tick();
    m_flow.Tick(m_ui, _ctx.GetAudio());

    if (m_tick == 10)
    {
        m_ui.ShowBanner("MISSION 4", 3.f);
    }
    else if (m_tick == 260)
    {
        BannerSpec spec = MakeBannerSpec("WARNING", 2.f);
        spec.color      = eFontColor::Red;
        m_ui.GetBanner().Show(std::move(spec));
    }
    else if (m_tick == 420)
    {
        m_flow.Start(MakeMissionCompleteFlowConfig(16), m_ui, _ctx.GetAudio());   // 구출 16명 결과
    }
    else if (m_tick == 1200)
    {
        m_flow.Start(MakeGameOverFlowConfig(), m_ui, _ctx.GetAudio());
    }
}

void UiDemoState::Render(
    GameContext& _ctx,
    const float)
{
    Renderer& renderer = *_ctx.pRenderer;
    renderer.BeginFrame({ 156, 190, 224, 255 });
    (void)m_ui.DrawStandalone(_ctx, "Stage0_Main", -900, -880);

    m_ui.DrawHud(_ctx, MakeModel_());
    m_ui.SetMissionResults(m_flow.GetResultsView());
    m_ui.DrawOverlays(_ctx);
}

int UiDemoState::GetTickCount() const
{
    return m_tick;
}

const UiSystem& UiDemoState::GetUi() const
{
    return m_ui;
}

}   // namespace slug
