#include "Core/App.h"

#include <algorithm>
#include <cstdio>
#include <cstdlib>
#include <filesystem>
#include <format>
#include <thread>
#include <SDL3/SDL.h>

#include "Base/Log.h"
#include "Base/SlugError.h"
#include "Base/Types.h"
#include "Core/AssetPath.h"
#include "Game/StateFactory.h"

namespace
{
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

int App::Run(
    int    _argc,
    char** _argv)
{
    m_processTimer.Start();
    m_cmdArgs.reserve(static_cast<size_t>(_argc));
    for (int i = 0; i < _argc; ++i)
    {
        m_cmdArgs.emplace_back(_argv[i]);
    }

    if (!InitSystems_())
    {
        return 1;
    }

    m_bRunning   = true;
    m_returnCode = 0;
    Init_();

    Timer frameTimer;
    frameTimer.Start();
    while (m_bRunning)
    {
        PollEvents_();
        if (!m_bRunning)
        {
            break;
        }
        frameTimer.Lap();
        Update_(TimeCastF(frameTimer.GetElapsedCount(), eTimeUnit::Sec));
    }

    Shutdown_();
    SDL_Quit();
    return m_returnCode;
}

bool App::InitSystems_()
{
    if (!SDL_SetAppMetadata("MetalSlug2 (Clone)", "0.1.0", "com.mymetalslug2.game"))
    {
        SLUG_LOG_WARN("SDL_SetAppMetadata failed: {}", SDL_GetError());
    }
    if (!SDL_Init(SDL_INIT_VIDEO))
    {
        SLUG_LOG_FATAL("SDL_Init failed: {}", SDL_GetError());
        return false;
    }
    return true;
}

void App::PollEvents_()
{
    const bool bEditorKeys = m_bEditor && m_editor.OwnsKeyboard();
    if (bEditorKeys && !m_bEditorKeys)
    {
        m_input.ClearAll();
    }
    m_bEditorKeys = bEditorKeys;

    SDL_Event msg {};
    while (SDL_PollEvent(&msg))
    {
        if (m_bEditor)
        {
            m_editor.ProcessEvent(msg);
        }
        switch (msg.type)
        {
            case SDL_EVENT_QUIT: m_bRunning = false; break;
            case SDL_EVENT_KEY_DOWN:
                if (!bEditorKeys)
                {
                    OnKeyDown_(msg.key.scancode, msg.key.repeat);
                }
                break;
            case SDL_EVENT_KEY_UP: OnKeyUp_(msg.key.scancode); break;
            case SDL_EVENT_WINDOW_FOCUS_LOST: OnFocusLost_(); break;
            default: break;
        }
    }
}

void App::ParseArgs_()
{
    const Span<const String> args = m_cmdArgs;
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
        else if (args[i] == "--editor")
        {
            m_bEditor = true;
            if (g_debug.weapon == 0)
            {
                g_debug.weapon = 1;
            }
        }
        else if (args[i] == "--editor-anim" && bHasValue)
        {
            m_editorAnim = args[++i];
        }
        else if (args[i] == "--editor-tab" && bHasValue)
        {
            m_editorTab = args[++i];
        }
        else if (args[i] == "--editor-stage" && bHasValue)
        {
            m_editorStage = args[++i];
        }
        else if (args[i] == "--editor-god" && bHasValue)
        {
            m_editorGod = args[++i];
        }
        else if (args[i] == "--editor-lab-spawn" && bHasValue)
        {
            m_editorLabSpawn = args[++i];
        }
        else if (args[i] == "--no-audio")
        {
            m_bNoAudio = true;
        }
        else if (args[i] == "--no-async-load")
        {
            m_bNoAsyncLoad = true;
        }
        else if (args[i] == "--trace-load")
        {
            SetLoadTraceEnabled(true);
        }
        else if (args[i] == "--capture" && bHasValue)
        {
            m_capturePath = args[++i];
        }
        else if (args[i] == "--event-log" && bHasValue)
        {
            if (!OpenEventLog(args[++i]))
            {
                SLUG_LOG_WARN("이벤트 로그를 열 수 없습니다: {}", args[i]);
            }
        }
        else if (args[i] == "--capture-frame" && bHasValue)
        {
            m_captureFrame = std::atoi(args[++i].c_str());
        }
        else if (args[i] == "--weapon" && bHasValue)
        {
            g_debug.weapon = std::atoi(args[++i].c_str());
        }
        else if (args[i] == "--hold" && bHasValue)
        {
            const String name = args[++i];
            if (name == "J")
            {
                m_input.OnKeyDown(SDL_SCANCODE_J);
            }
            else if (name == "K")
            {
                m_input.OnKeyDown(SDL_SCANCODE_K);
            }
            else if (name == "Up")
            {
                m_input.OnKeyDown(SDL_SCANCODE_UP);
            }
            else if (name == "Down")
            {
                m_input.OnKeyDown(SDL_SCANCODE_DOWN);
            }
            else if (name == "Left")
            {
                m_input.OnKeyDown(SDL_SCANCODE_LEFT);
            }
            else if (name == "Right")
            {
                m_input.OnKeyDown(SDL_SCANCODE_RIGHT);
            }
        }
        else if (args[i] == "--warp" && bHasValue)
        {
            m_startWarpCount   = std::atoi(args[++i].c_str());
            g_debug.bDebugDraw = true;
        }
        else if (args[i] == "--warp-x" && bHasValue)
        {
            m_startWarpX = static_cast<float>(std::atof(args[++i].c_str()));
        }
        else if (args[i] == "--life" && bHasValue)
        {
            g_debug.lives = std::atoi(args[++i].c_str());
        }
        else if (args[i] == "--warp-boss")
        {
            g_debug.warp       = 2;
            g_debug.bDebugDraw = true;
        }
        else if (args[i] == "--capture-ticks" && bHasValue)
        {
            const String list  = args[++i];
            size_t       begin = 0;
            while (begin < list.size())
            {
                size_t end = list.find(',', begin);
                if (end == String::npos)
                {
                    end = list.size();
                }
                if (end > begin)
                {
                    m_captureTicks.push_back(std::strtoull(list.substr(begin, end - begin).c_str(), nullptr, 10));
                }
                begin = end + 1;
            }
            std::sort(m_captureTicks.begin(), m_captureTicks.end());
        }
    }
}

void App::Init_()
{
    ParseArgs_();

    if (m_stateName.empty())
    {
        const bool bHasAssets = std::filesystem::exists(FindAssetRoot() / "manifest.json");
        m_stateName           = bHasAssets ? String { GetDefaultStateName() } : String { "sandbox" };
    }

    const FilePath assetRoot  = FindAssetRoot();
    const int      numWorkers = m_bNoAsyncLoad ? 1 : Clamp(static_cast<int>(std::thread::hardware_concurrency()) - 1, 1, 8);
    SLUG_LOG_INFO("[load] workers {}", numWorkers);
    bool bAnimsLoaded = false;
    if (Result<AnimationLibrary> library = AnimationLibrary::LoadFromDirectory(assetRoot))
    {
        AnimationLibrary::GetSingleton() = library.Take();
        bAnimsLoaded                     = true;
    }
    else
    {
        SLUG_LOG_WARN("에셋 없음 ({}): manifest.json 확인", assetRoot.string());
    }

    if (const Result<void> result = Renderer::GetSingleton().Init("MetalSlug2 (Clone)", ChooseWindowScale_()); !result)
    {
        SLUG_LOG_FATAL("렌더러 초기화 실패: {}", result.GetError().MakeMessage());
        m_returnCode = 1;
        m_bRunning   = false;
        return;
    }

    if (m_bEditor && !m_editor.Init(Renderer::GetSingleton().GetWindow(), Renderer::GetSingleton().GetNative()))
    {
        SLUG_LOG_WARN("ImGui 초기화 실패: 에디터 없이 계속합니다");
        m_bEditor = false;
    }
    else if (m_bEditor && !m_editorAnim.empty())
    {
        m_editor.SelectAnimation(m_editorAnim.c_str());
    }
    if (m_bEditor && !m_editorStage.empty())
    {
        m_editor.SelectStage(m_editorStage.c_str());
    }
    if (m_bEditor && m_editorGod == "nodeath")
    {
        m_editor.SetGodMode(eGodMode::NoDeath);
    }
    else if (m_bEditor && m_editorGod == "invincible")
    {
        m_editor.SetGodMode(eGodMode::Invincible);
    }
    if (m_bEditor && !m_editorTab.empty())
    {
        m_editor.SelectTab(m_editorTab.c_str());
    }

    m_processTimer.Lap();
    const double rendererMs = TimeCast(m_processTimer.GetTotalCount(), eTimeUnit::Milli);
    SLUG_LOG_INFO("[load] renderer-init since-run-start {:.1f} ms", rendererMs);
    SpriteBatch::GetSingleton().Init(Renderer::GetSingleton().GetNative());
    SpriteBatch::GetSingleton().SetAssetRoot(assetRoot);
    if (bAnimsLoaded)
    {
        (void)SpriteBatch::GetSingleton().LoadAtlasPagesParallel(AnimationLibrary::GetSingleton(), numWorkers);
    }

    g_debug.numLoadWorkers = numWorkers;
    m_editor.SetAnimLibrary(m_bEditor ? &AnimationLibrary::GetSingleton() : nullptr);
    g_debug.bInfiniteAmmo = m_bEditor;

    {
        AudioConfig audioConfig;
        audioConfig.bEnabled = !m_bNoAudio && m_capturePath.empty();
        if (const Result<void> audio = AudioSystem::GetSingleton().InitFromDirectory(assetRoot, audioConfig); !audio)
        {
            SLUG_LOG_WARN("sounds.json 을 읽지 못해 소리 없이 계속합니다: {}", audio.GetError().MakeMessage());
        }
    }

    Scoped<IGameState> pState = CreateState(m_stateName);
    if (!pState)
    {
        SLUG_LOG_FATAL("알 수 없는 게임 상태: {}", m_stateName);
        m_returnCode = 1;
        m_bRunning   = false;
        return;
    }
    m_pRunner = Scoped<SimRunner> {
        new SimRunner { std::move(pState) }
    };
    if (m_bEditor)
    {
        m_editor.GetLab().Init(m_pRunner.GetPtr(), m_stateName, assetRoot);
        m_editor.GetLab().SetAutoSpawn(m_editorLabSpawn);
    }
}

void App::Shutdown_()
{
    m_pRunner.Reset();
    AudioSystem::GetSingleton().Shutdown();
    SpriteBatch::GetSingleton().Shutdown();
    m_editor.Shutdown();
    Renderer::GetSingleton().Shutdown();
}

void App::OnKeyDown_(
    SDL_Scancode _key,
    bool         _bRepeat)
{
    if (_bRepeat)
    {
        return;
    }

    switch (_key)
    {
        case SDL_SCANCODE_F1: g_debug.bDebugDraw = !g_debug.bDebugDraw; return;
        case SDL_SCANCODE_F2: g_debug.bPaused = !g_debug.bPaused; return;
        case SDL_SCANCODE_F3: m_bStepOnce = true; return;
        case SDL_SCANCODE_F4: m_bSlowMo = !m_bSlowMo; return;
        case SDL_SCANCODE_F5:
            if (g_debug.bDebugDraw)
            {
                g_debug.warp = 1;
            }
            return;
        case SDL_SCANCODE_F6:
            if (g_debug.bDebugDraw)
            {
                g_debug.warp = 2;
            }
            return;
        case SDL_SCANCODE_1: g_debug.spawn = 1; return;
        case SDL_SCANCODE_2: g_debug.spawn = 2; return;
        case SDL_SCANCODE_3: g_debug.spawn = 3; return;
        case SDL_SCANCODE_4: g_debug.spawn = 4; return;
        case SDL_SCANCODE_5: g_debug.spawn = 5; return;
        case SDL_SCANCODE_6: g_debug.spawn = 6; return;
        case SDL_SCANCODE_7: g_debug.spawn = 7; return;
        case SDL_SCANCODE_8: g_debug.spawn = 8; return;
        case SDL_SCANCODE_9: g_debug.spawn = 9; return;
        case SDL_SCANCODE_F7: g_debug.weapon = 1; return;
        case SDL_SCANCODE_F8: g_debug.weapon = 2; return;
        case SDL_SCANCODE_F9: g_debug.weapon = 3; return;
        case SDL_SCANCODE_F10: g_debug.weapon = 4; return;
        default: break;
    }

    m_input.OnKeyDown(_key);
}

void App::OnKeyUp_(
    SDL_Scancode _key)
{
    m_input.OnKeyUp(_key);
}

void App::OnFocusLost_()
{
    m_input.ClearAll();
}

void App::Update_(
    float _deltaTimeSec)
{
    AudioSystem::GetSingleton().Update(_deltaTimeSec);

    if (!m_pRunner)
    {
        return;
    }

    if (_deltaTimeSec > 0.f)
    {
        g_debug.fps = g_debug.fps <= 0.f ? 1.f / _deltaTimeSec : g_debug.fps * 0.95f + (1.f / _deltaTimeSec) * 0.05f;
    }
    if (m_startWarpCount > 0)
    {
        g_debug.warp = 1;
        --m_startWarpCount;
    }
    if (m_startWarpX > 0.f)
    {
        g_debug.warp  = 3;
        g_debug.warpX = m_startWarpX;
        m_startWarpX  = 0.f;
    }
    g_debug.godMode = m_bEditor ? m_editor.GetGodMode() : eGodMode::Off;

    int steps = 0;
    if (!m_captureTicks.empty())
    {
        steps = 1;
    }
    else if (g_debug.bPaused)
    {
        steps       = m_bStepOnce ? 1 : 0;
        m_bStepOnce = false;
    }
    else
    {
        double deltaSec = static_cast<double>(_deltaTimeSec) / (m_bSlowMo ? 4.0 : 1.0);
        if (m_bEditor)
        {
            deltaSec *= static_cast<double>(m_editor.GetLab().GetTimeScale());
        }
        steps = m_clock.Advance(deltaSec);
        if (m_bEditor)
        {
            steps = m_editor.GetLab().TakeSteps(steps);
        }
    }
    for (int i = 0; i < steps; ++i)
    {
        Tick_();
    }
    g_debug.warp   = 0;   // 한 프레임 안에 소비되지 않은 요청은 버린다
    g_debug.weapon = 0;

    Render_(m_clock.GetAlpha());
}

void App::Tick_()
{
    const InputFrame frame = m_input.Sample();
    SetEventLogTick(m_pRunner->GetTickCount() + 1);
    m_pRunner->Step(frame);
    if (m_pRunner->GetHistory().WasPressed(eAction::Pause))
    {
        m_bRunning = false;
    }
}

void App::Render_(
    float _alpha)
{
    m_pRunner->Render(_alpha);
    if (m_bEditor)
    {
        m_editor.DrawLabOverlay(Renderer::GetSingleton());
        m_editor.Frame(g_debug.fps, m_pRunner->GetTickCount());
    }

    ++m_renderFrameCount;
    if (m_renderFrameCount == 1)
    {
        m_processTimer.Lap();
        const double ms = TimeCast(m_processTimer.GetTotalCount(), eTimeUnit::Milli);
        SLUG_LOG_INFO("[load] first-frame since-run-start {:.1f} ms", ms);
    }
    if (!m_capturePath.empty() && !m_captureTicks.empty())
    {
        while (m_captureTickHead < m_captureTicks.size() && m_pRunner->GetTickCount() >= m_captureTicks[m_captureTickHead])
        {
            const uint64_t tick = m_captureTicks[m_captureTickHead++];
            const FilePath path = FilePath { std::format("{}_{:05}.bmp", m_capturePath.string(), tick) };
            if (const Result<void> result = Renderer::GetSingleton().SaveScreenshotBmp(path); !result)
            {
                SLUG_LOG_ERROR("스크린샷 저장 실패: {}", result.GetError().MakeMessage());
                m_returnCode = 2;
            }
        }
        if (m_captureTickHead >= m_captureTicks.size())
        {
            m_bRunning = false;
        }
    }
    else if (!m_capturePath.empty() && m_renderFrameCount == m_captureFrame)
    {
        if (const Result<void> result = Renderer::GetSingleton().SaveScreenshotBmp(m_capturePath); !result)
        {
            SLUG_LOG_ERROR("스크린샷 저장 실패: {}", result.GetError().MakeMessage());
            m_returnCode = 2;
        }
        m_bRunning = false;
    }

    Renderer::GetSingleton().EndFrame();
}
