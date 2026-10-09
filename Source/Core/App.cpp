#include "Core/App.h"

#include <SDL3/SDL.h>

#include <JugX/ConsoleLogger.h>
#include <JugX/CoreLogger.h>
#include <JugX/EventDispatcher.h>

#include "Game/StateFactory.h"

namespace slug
{

namespace
{
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

void App::ParseArgs_()
{
    const jug::Span<const jug::String> args = GetCommandLineArgs();
    for (size_t i = 1; i < args.size(); ++i)
    {
        const bool bHasValue = i + 1 < args.size();
        if (args[i] == "--state" && bHasValue)
        {
            m_stateName = args[++i];
        }
        else if (args[i] == "--seed" && bHasValue)
        {
            m_seed = std::strtoull(args[++i].c_str(), nullptr, 10);
        }
        else if (args[i] == "--record" && bHasValue)
        {
            m_recordPath = args[++i];
        }
        else if (args[i] == "--replay" && bHasValue)
        {
            m_replayPath = args[++i];
        }
        else if (args[i] == "--replay-hold")
        {
            m_bReplayHold = true;
        }
        else if (args[i] == "--capture" && bHasValue)
        {
            m_capturePath = args[++i];
        }
        else if (args[i] == "--capture-frame" && bHasValue)
        {
            m_captureFrame = std::atoi(args[++i].c_str());
        }
    }
}

void App::Init()
{
    static jug::ConsoleLogger s_logger;
    jug::SetCoreLogger(&s_logger);

    ParseArgs_();

    if (!m_replayPath.empty())
    {
        jug::Result<ReplayData> loaded = LoadReplay(m_replayPath);
        if (!loaded)
        {
            JUG_CORE_LOG_FATAL("리플레이 로드 실패: {}", loaded.GetError().MakeMessage());
            SetReturnCode(1);
            Quit();
            return;
        }
        m_playback = loaded.Take();
        m_bPlayback = true;
        m_seed      = m_playback.seed;
        if (m_stateName.empty())
        {
            m_stateName = m_playback.stateName;
        }
    }
    if (m_stateName.empty())
    {
        m_stateName = jug::String { GetDefaultStateName() };
    }

    if (const jug::Result<void> result = m_renderer.Init("Metal Slug Remake", ChooseWindowScale_()); !result)
    {
        JUG_CORE_LOG_FATAL("렌더러 초기화 실패: {}", result.GetError().MakeMessage());
        SetReturnCode(1);
        Quit();
        return;
    }

    // 에셋 로드 (없으면 경고만 하고 계속: 스프라이트 없이도 데모 상태는 동작)
    const jug::FilePath bakedRoot = jug::FilePath { SLUG_ASSET_DIR } / "Baked";
    m_sprites.Init(m_renderer.GetNative());
    if (jug::Result<AnimationLibrary> library = AnimationLibrary::LoadFromDirectory(bakedRoot))
    {
        m_anims = library.Take();
        if (const jug::Result<void> atlas = m_sprites.LoadAtlasPages(m_anims); !atlas)
        {
            JUG_CORE_LOG_WARN("아틀라스 로드 실패: {}", atlas.GetError().MakeMessage());
        }
    }
    else
    {
        JUG_CORE_LOG_WARN("Baked 에셋 없음 ({}): Scripts/Bake.ps1 을 실행하세요", bakedRoot.string());
    }

    m_rng = jug::RngMt19937 { m_seed };
    m_recording.seed      = m_seed;
    m_recording.stateName = m_stateName;

    m_ctx.pRenderer = &m_renderer;
    m_ctx.pSprites  = &m_sprites;
    m_ctx.pAnims    = &m_anims;
    m_ctx.pRng      = &m_rng;
    m_ctx.assetRoot = jug::FilePath { SLUG_ASSET_DIR };

    jug::Scoped<IGameState> pState = CreateState(m_stateName);
    if (!pState)
    {
        JUG_CORE_LOG_FATAL("알 수 없는 게임 상태: {}", m_stateName);
        SetReturnCode(1);
        Quit();
        return;
    }
    m_pRunner = jug::Scoped<SimRunner> { new SimRunner { m_ctx, std::move(pState) } };
}

void App::Shutdown()
{
    SaveRecording_();
    m_pRunner.Reset();
    m_sprites.Shutdown();
    m_renderer.Shutdown();
}

void App::SaveRecording_() const
{
    if (m_recordPath.empty())
    {
        return;
    }
    if (const jug::Result<void> result = SaveReplay(m_recordPath, m_recording); !result)
    {
        JUG_CORE_LOG_ERROR("리플레이 저장 실패: {}", result.GetError().MakeMessage());
        return;
    }
    JUG_CORE_LOG_INFO("리플레이 저장: {} ({} 틱)", m_recordPath.string(), m_recording.frames.size());
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
    if (!m_pRunner)
    {
        return;
    }

    const int steps = m_clock.Advance(_deltaTimeSec);
    for (int i = 0; i < steps; ++i)
    {
        Tick_();
    }

    Render_(m_clock.GetAlpha());
}

void App::Tick_()
{
    InputFrame frame;
    if (m_bPlayback)
    {
        if (m_playbackHead < m_playback.frames.size())
        {
            frame = m_playback.frames[m_playbackHead++];
        }
        else if (!m_bReplayHold)
        {
            Quit();
            return;
        }
    }
    else
    {
        frame = m_input.Sample();
    }

    if (!m_recordPath.empty())
    {
        m_recording.frames.push_back(frame);
    }

    m_pRunner->Step(frame);

    // 디버그 종료 키: Esc (Pause)
    if (!m_bPlayback && m_pRunner->GetHistory().WasPressed(eAction::Pause))
    {
        Quit();
    }
}

void App::Render_(
    const float _alpha)
{
    // 상태가 BeginFrame 을 부르지 않았을 때를 대비해 기본 클리어는 상태 Render 가 책임진다.
    m_pRunner->Render(_alpha);

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
