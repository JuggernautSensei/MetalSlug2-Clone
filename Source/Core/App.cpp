#include "Core/App.h"

#include <SDL3/SDL.h>

#include <JugX/ConsoleLogger.h>
#include <JugX/CoreLogger.h>
#include <JugX/EventDispatcher.h>

namespace slug
{

namespace
{
    constexpr float kDemoSpeedPerStep = 2.f;   // 120 px/s @ 60Hz

    // 모니터에 들어가는 최대 정수 배율 (최대 3배)
    [[nodiscard]] int ChooseWindowScale_()
    {
        const SDL_DisplayID display = SDL_GetPrimaryDisplay();
        SDL_Rect            usable  = {};
        if (display == 0 || !SDL_GetDisplayUsableBounds(display, &usable))
        {
            return 2;
        }

        int scale = 3;
        while (scale > 1 && (Renderer::kLogicalWidth * scale > usable.w - 40 || Renderer::kLogicalHeight * scale > usable.h - 80))
        {
            --scale;
        }
        return scale;
    }
}   // namespace

App::App()
    : Application({ .appName = "Metal Slug Remake", .appVersion = "0.1.0", .appIdentifier = "com.msremake.game", .bInitGamepad = false })
{
}

App::~App() = default;

void App::Init()
{
    static jug::ConsoleLogger s_logger;
    jug::SetCoreLogger(&s_logger);

    const jug::Span<const jug::String> args = GetCommandLineArgs();
    for (size_t i = 1; i < args.size(); ++i)
    {
        if (args[i] == "--capture" && i + 1 < args.size())
        {
            m_capturePath = args[++i];
        }
        else if (args[i] == "--capture-frame" && i + 1 < args.size())
        {
            m_captureFrame = std::atoi(args[++i].c_str());
        }
    }

    if (const jug::Result<void> result = m_renderer.Init("Metal Slug Remake", ChooseWindowScale_()); !result)
    {
        JUG_CORE_LOG_FATAL("렌더러 초기화 실패: {}", result.GetError().MakeMessage());
        SetReturnCode(1);
        Quit();
    }
}

void App::Shutdown()
{
    m_renderer.Shutdown();
}

void App::OnEvent(
    jug::Event& _event)
{
    jug::EventDispatcher dispatcher { _event };
    dispatcher.Dispatch<jug::KeyDownEvent>(this, &App::OnKeyDown_);
    dispatcher.Dispatch<jug::KeyUpEvent>(this, &App::OnKeyUp_);
    dispatcher.Dispatch<jug::WindowFocusLostEvent>(this, &App::OnFocusLost_);
}

void App::OnKeyDown_(
    const jug::KeyDownEvent& _event)
{
    if (_event.IsRepeat())
    {
        return;
    }

    m_input.OnKeyDown(_event.GetKey());
}

void App::OnKeyUp_(
    const jug::KeyUpEvent& _event)
{
    m_input.OnKeyUp(_event.GetKey());
}

void App::OnFocusLost_(
    const jug::WindowFocusLostEvent&)
{
    // 포커스를 잃으면 눌림 상태가 영원히 남지 않도록 비운다.
    m_input.ClearAll();
}

void App::Update(
    const float _deltaTimeSec)
{
    const int steps = m_clock.Advance(_deltaTimeSec);
    for (int i = 0; i < steps; ++i)
    {
        Tick_();
    }

    Render_(m_clock.GetAlpha());
}

void App::Tick_()
{
    m_history.Push(m_input.Sample());
    ++m_tickCount;

    m_prevPos = m_pos;

    jug::VECTOR2 move = { 0.f, 0.f };
    if (m_history.IsHeld(eAction::Left))
    {
        move.x -= kDemoSpeedPerStep;
    }
    if (m_history.IsHeld(eAction::Right))
    {
        move.x += kDemoSpeedPerStep;
    }
    if (m_history.IsHeld(eAction::Up))
    {
        move.y -= kDemoSpeedPerStep;
    }
    if (m_history.IsHeld(eAction::Down))
    {
        move.y += kDemoSpeedPerStep;
    }
    m_pos += move;

    if (m_history.WasPressed(eAction::Pause))
    {
        Quit();
    }
}

void App::Render_(
    const float _alpha)
{
    m_renderer.BeginFrame({ 24, 28, 40, 255 });

    // 보간된 위치를 정수로 스냅하여 그린다.
    const jug::VECTOR2 drawPos = m_prevPos + (m_pos - m_prevPos) * _alpha;
    m_renderer.FillRect({ drawPos.x - 12.f, drawPos.y - 12.f }, { 24.f, 24.f }, { 240, 200, 60, 255 });
    m_renderer.DrawRect({ 1.f, 1.f }, { Renderer::kLogicalWidth - 2.f, Renderer::kLogicalHeight - 2.f }, { 90, 100, 130, 255 });

    ++m_renderFrameCount;
    if (!m_capturePath.empty() && m_renderFrameCount == m_captureFrame)
    {
        if (const jug::Result<void> result = m_renderer.SaveScreenshotBmp(m_capturePath); !result)
        {
            JUG_CORE_LOG_ERROR("스크린샷 저장 실패: {}", result.GetError().MakeMessage());
            SetReturnCode(2);
        }
        Quit();
    }

    m_renderer.EndFrame();
}

}   // namespace slug
