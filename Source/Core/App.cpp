#include "Core/App.h"

#include <algorithm>
#include <cstdlib>
#include <filesystem>
#include <format>
#include <SDL3/SDL.h>

#include "Base/Log.h"
#include "Base/SlugError.h"
#include "Base/Types.h"
#include "Core/AssetPath.h"
#include "Game/StateFactory.h"

[[nodiscard]] static int ChooseWindowScale_()
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

int App::Run(
    const int _argc,
    char**    _argv)
{
    m_processStart = SteadyClock::now();
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

    SteadyClock::time_point lastTime = SteadyClock::now();
    while (m_bRunning)
    {
        PollEvents_();
        if (!m_bRunning)
        {
            break;
        }
        const SteadyClock::time_point now = SteadyClock::now();
        Update_(std::chrono::duration<float>(now - lastTime).count());
        lastTime = now;
    }

    Shutdown_();
    SDL_Quit();
    return m_returnCode;
}

void App::SubmitToMainThread(
    std::unique_ptr<IMainThreadTask> _pTask)
{
    m_mainQueue.Submit(std::move(_pTask));
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
        else if (args[i] == "--editor")
        {
            m_bEditor = true;
            if (m_pendingWeapon == 0)
            {
                m_pendingWeapon = 1;
            }
        }
        else if (args[i] == "--editor-float-test")
        {
            m_bEditorFloatTest = true;
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
            m_pendingWeapon = std::atoi(args[++i].c_str());
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
            m_startWarpCount = std::atoi(args[++i].c_str());
            m_bDebugDraw     = true;
        }
        else if (args[i] == "--warp-x" && bHasValue)
        {
            // 디버그: 월드 x 로 직접 워프 (그 앞 구간은 조용히 적용). 맵 검증용
            m_startWarpX = static_cast<float>(std::atof(args[++i].c_str()));
        }
        else if (args[i] == "--life" && bHasValue)
        {
            m_debugLives = std::atoi(args[++i].c_str());
        }
        else if (args[i] == "--warp-boss")
        {
            m_pendingWarp = 2;   // F6 와 같다: 스테이지 마지막 구간 (종료 지점)
            m_bDebugDraw  = true;
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

    if (!m_replayPath.empty())
    {
        Result<ReplayData> loaded = LoadReplay(m_replayPath);
        if (!loaded)
        {
            SLUG_LOG_FATAL("리플레이 로드 실패: {}", loaded.GetError().MakeMessage());
            m_returnCode = 1;
            m_bRunning   = false;
            return;
        }
        m_playback  = loaded.Take();
        m_bPlayback = true;
        m_seed      = m_playback.seed;
        if (m_stateName.empty())
        {
            m_stateName = m_playback.stateName;
        }
    }
    if (m_stateName.empty())
    {
        const bool bHasAssets = std::filesystem::exists(FindAssetRoot() / "manifest.json");
        m_stateName         = bHasAssets ? String { GetDefaultStateName() } : String { "sandbox" };
    }

    // 아틀라스 디코드는 렌더러가 필요 없으므로 창/디바이스 생성보다 먼저 워커에 맡긴다 (텍스처 생성은 메인 스레드에서 나중에).
    // 워커가 없는 동기 폴백은 제출 즉시 텍스처까지 만들므로 렌더러 초기화 뒤로 미룬다.
    const FilePath assetRoot = FindAssetRoot();
    m_loader.Start(m_bNoAsyncLoad ? 0 : AsyncLoader::ChooseWorkerCount());
    SLUG_LOG_INFO("[load] workers {}", m_loader.GetWorkerCount());
    bool bAnimsLoaded    = false;
    bool bAtlasSubmitted = false;
    if (Result<AnimationLibrary> library = AnimationLibrary::LoadFromDirectory(assetRoot))
    {
        m_anims      = library.Take();
        bAnimsLoaded = true;
        if (m_loader.GetWorkerCount() > 0)
        {
            m_sprites.LoadAtlasPagesAsync(m_anims, m_loader);
            bAtlasSubmitted = true;
        }
    }
    else
    {
        SLUG_LOG_WARN("에셋 없음 ({}): manifest.json 확인", assetRoot.string());
    }

    if (const Result<void> result = m_renderer.Init("MetalSlug2 (Clone)", ChooseWindowScale_()); !result)
    {
        SLUG_LOG_FATAL("렌더러 초기화 실패: {}", result.GetError().MakeMessage());
        m_returnCode = 1;
        m_bRunning   = false;
        return;
    }

    if (m_bEditor && !m_editor.Init(m_renderer.GetWindow(), m_renderer.GetNative()))
    {
        SLUG_LOG_WARN("ImGui 초기화 실패: 에디터 없이 계속합니다");
        m_bEditor = false;
    }
    else if (m_bEditor && !m_editorAnim.empty())
    {
        m_editor.SelectAnimation(m_editorAnim.c_str());
    }
    if (m_bEditor && m_bEditorFloatTest)
    {
        m_editor.EnableFloatTest();
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

    const double rendererMs = std::chrono::duration<double, std::milli>(SteadyClock::now() - m_processStart).count();
    SLUG_LOG_INFO("[load] renderer-init since-run-start {:.1f} ms", rendererMs);
    m_sprites.Init(m_renderer.GetNative());
    m_sprites.SetAssetRoot(assetRoot);
    if (bAnimsLoaded && !bAtlasSubmitted)
    {
        m_sprites.LoadAtlasPagesAsync(m_anims, m_loader);
    }

    m_rng                 = RngMt19937 { m_seed };
    m_recording.seed      = m_seed;
    m_recording.stateName = m_stateName;

    m_ctx.pRenderer  = &m_renderer;
    m_ctx.pSprites   = &m_sprites;
    m_ctx.pAnims     = &m_anims;
    m_ctx.pLoader    = &m_loader;
    m_editor.SetAnimLibrary(m_bEditor ? &m_anims : nullptr);
    m_ctx.pRng       = &m_rng;
    m_ctx.assetRoot  = FindAssetRoot();
    m_ctx.debugLives = m_debugLives;

    {
        AudioConfig audioConfig;
        audioConfig.bEnabled = !m_bNoAudio && m_capturePath.empty();
        audioConfig.pLoader  = &m_loader;
        if (const Result<void> audio = m_audio.InitFromDirectory(assetRoot, audioConfig); audio)
        {
            m_ctx.pAudio = &m_audio;
        }
        else
        {
            SLUG_LOG_WARN("sounds.json 을 읽지 못해 소리 없이 계속합니다: {}", audio.GetError().MakeMessage());
        }
    }

    m_loader.WaitAll();

    std::unique_ptr<IGameState> pState = CreateState(m_stateName);
    if (!pState)
    {
        SLUG_LOG_FATAL("알 수 없는 게임 상태: {}", m_stateName);
        m_returnCode = 1;
        m_bRunning   = false;
        return;
    }
    m_pRunner = std::unique_ptr<SimRunner> {
        new SimRunner { m_ctx, std::move(pState) }
    };
    if (m_bEditor)
    {
        m_editor.GetLab().Init(m_pRunner.get(), m_stateName, m_ctx.assetRoot);
        m_editor.GetLab().SetAutoSpawn(m_editorLabSpawn);
    }
}

void App::Shutdown_()
{
    SaveRecording_();
    m_loader.Shutdown();
    m_pRunner.reset();
    m_ctx.pAudio = nullptr;
    m_audio.Shutdown();
    m_sprites.Shutdown();
    m_editor.Shutdown();
    m_renderer.Shutdown();
}

void App::SaveRecording_() const
{
    if (m_recordPath.empty())
    {
        return;
    }
    if (const Result<void> result = SaveReplay(m_recordPath, m_recording); !result)
    {
        SLUG_LOG_ERROR("리플레이 저장 실패: {}", result.GetError().MakeMessage());
        return;
    }
    SLUG_LOG_INFO("리플레이 저장: {} ({} 틱)", m_recordPath.string(), m_recording.frames.size());
}

void App::OnKeyDown_(
    const SDL_Scancode _key,
    const bool         _bRepeat)
{
    if (_bRepeat)
    {
        return;
    }

    switch (_key)
    {
        case SDL_SCANCODE_F1: m_bDebugDraw = !m_bDebugDraw; return;
        case SDL_SCANCODE_F2: m_bPaused = !m_bPaused; return;
        case SDL_SCANCODE_F3: m_bStepOnce = true; return;
        case SDL_SCANCODE_F4: m_bSlowMo = !m_bSlowMo; return;
        case SDL_SCANCODE_F5:
            if (m_bDebugDraw)
            {
                m_pendingWarp = 1;
            }
            return;   // 스테이지: 다음 섹션
        case SDL_SCANCODE_F6:
            if (m_bDebugDraw)
            {
                m_pendingWarp = 2;
            }
            return;   // 스테이지: 보스 아레나
        case SDL_SCANCODE_1: m_pendingSpawn = 1; return;
        case SDL_SCANCODE_2: m_pendingSpawn = 2; return;
        case SDL_SCANCODE_3: m_pendingSpawn = 3; return;
        case SDL_SCANCODE_4: m_pendingSpawn = 4; return;
        case SDL_SCANCODE_5: m_pendingSpawn = 5; return;
        case SDL_SCANCODE_6: m_pendingSpawn = 6; return;
        case SDL_SCANCODE_7: m_pendingSpawn = 7; return;
        case SDL_SCANCODE_8: m_pendingSpawn = 8; return;
        case SDL_SCANCODE_9: m_pendingSpawn = 9; return;
        case SDL_SCANCODE_F7: m_pendingWeapon = 1; return;
        case SDL_SCANCODE_F8: m_pendingWeapon = 2; return;
        case SDL_SCANCODE_F9: m_pendingWeapon = 3; return;
        case SDL_SCANCODE_F10: m_pendingWeapon = 4; return;
        default: break;
    }

    m_input.OnKeyDown(_key);
}

void App::OnKeyUp_(
    const SDL_Scancode _key)
{
    m_input.OnKeyUp(_key);
}

void App::OnFocusLost_()
{
    m_input.ClearAll();
}

void App::Update_(
    const float _deltaTimeSec)
{
    m_mainQueue.DrainAll();
    m_audio.Update(_deltaTimeSec);

    if (!m_pRunner)
    {
        return;
    }

    if (_deltaTimeSec > 0.f)
    {
        m_fps = m_fps <= 0.f ? 1.f / _deltaTimeSec : m_fps * 0.95f + (1.f / _deltaTimeSec) * 0.05f;
    }
    m_ctx.fps           = m_fps;
    m_ctx.bDebugDraw    = m_bDebugDraw;
    m_ctx.bPaused       = m_bPaused;
    m_ctx.slowMoDivisor = m_bSlowMo ? 4 : 1;
    if (m_startWarpCount > 0)
    {
        m_pendingWarp = 1;
        --m_startWarpCount;
    }
    m_ctx.debugWarpX  = 0.f;
    if (m_startWarpX > 0.f)
    {
        m_pendingWarp = 3;
        m_ctx.debugWarpX = m_startWarpX;
        m_startWarpX     = 0.f;
    }
    m_ctx.debugWarp   = m_pendingWarp;
    m_ctx.debugLives  = m_debugLives;
    m_pendingWarp     = 0;
    m_ctx.godMode     = m_bEditor ? m_editor.GetGodMode() : eGodMode::Off;
    m_ctx.bInfiniteAmmo = m_bEditor;
    m_ctx.debugWeapon = m_pendingWeapon;
    m_pendingWeapon   = 0;
    if (m_pendingSpawn > 0)
    {
        m_ctx.debugSpawn = m_pendingSpawn;
        m_pendingSpawn   = 0;
    }

    int steps = 0;
    if (!m_captureTicks.empty())
    {
        steps = 1;   // 결정적 캡처: 렌더 프레임당 정확히 1틱
    }
    else if (m_bPaused)
    {
        steps       = m_bStepOnce ? 1 : 0;
        m_bStepOnce = false;
    }
    else
    {
        double deltaSec = static_cast<double>(_deltaTimeSec) / static_cast<double>(m_ctx.slowMoDivisor);
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
            m_bRunning = false;
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

    SetEventLogTick(m_pRunner->GetTickCount() + 1);
    m_pRunner->Step(frame);

    if (!m_bPlayback && m_pRunner->GetHistory().WasPressed(eAction::Pause))
    {
        m_bRunning = false;
    }
}

void App::Render_(
    const float _alpha)
{
    m_pRunner->Render(_alpha);
    if (m_bEditor)
    {
        m_editor.DrawLabOverlay(m_renderer);
        m_editor.Frame(m_fps, m_pRunner->GetTickCount());
    }

    ++m_renderFrameCount;
    if (m_renderFrameCount == 1)
    {
        const double ms = std::chrono::duration<double, std::milli>(SteadyClock::now() - m_processStart).count();
        SLUG_LOG_INFO("[load] first-frame since-run-start {:.1f} ms", ms);
    }
    if (!m_capturePath.empty() && !m_captureTicks.empty())
    {
        while (m_captureTickHead < m_captureTicks.size() && m_pRunner->GetTickCount() >= m_captureTicks[m_captureTickHead])
        {
            const uint64_t tick = m_captureTicks[m_captureTickHead++];
            const FilePath path = FilePath { std::format("{}_{:05}.bmp", m_capturePath.string(), tick) };
            if (const Result<void> result = m_renderer.SaveScreenshotBmp(path); !result)
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
        if (const Result<void> result = m_renderer.SaveScreenshotBmp(m_capturePath); !result)
        {
            SLUG_LOG_ERROR("스크린샷 저장 실패: {}", result.GetError().MakeMessage());
            m_returnCode = 2;
        }
        m_bRunning = false;
    }

    m_renderer.EndFrame();
}
