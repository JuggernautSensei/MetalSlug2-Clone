#include "Core/AssetPath.h"
#include "Core/App.h"
#include "Base/Types.h"
#include "Base/Result.h"
#include "Base/Random.h"

#include <SDL3/SDL.h>

#include "Base/Log.h"

#include <algorithm>
#include <cstdlib>
#include <filesystem>
#include <format>

#include "Game/StateFactory.h"

namespace slug
{

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
    const int _argc,
    char**    _argv)
{
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

    auto lastTime = SteadyClock::now();
    while (m_bRunning)
    {
        PollEvents_();
        if (!m_bRunning)
        {
            break;
        }
        const auto now = SteadyClock::now();
        Update_(std::chrono::duration<float>(now - lastTime).count());
        lastTime = now;
    }

    Shutdown_();
    SDL_Quit();
    return m_returnCode;
}

bool App::InitSystems_()
{
    if (!SDL_SetAppMetadata("MyMetalSlug2", "0.1.0", "com.mymetalslug2.game"))
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
    SDL_Event msg {};
    while (SDL_PollEvent(&msg))
    {
        switch (msg.type)
        {
            case SDL_EVENT_QUIT: m_bRunning = false; break;
            case SDL_EVENT_KEY_DOWN: OnKeyDown_(msg.key.scancode, msg.key.repeat); break;
            case SDL_EVENT_KEY_UP: OnKeyUp_(msg.key.scancode); break;
            case SDL_EVENT_WINDOW_FOCUS_LOST: OnFocusLost_(); break;
            default: break;
        }
    }
}

void App::ParseArgs_()
{
    const slug::Span<const slug::String> args = m_cmdArgs;
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
        else if (args[i] == "--no-audio")
        {
            m_bNoAudio = true;
        }
        else if (args[i] == "--capture" && bHasValue)
        {
            m_capturePath = args[++i];
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
            const slug::String name = args[++i];
            if (name == "J")
                m_input.OnKeyDown(SDL_SCANCODE_J);
            else if (name == "K")
                m_input.OnKeyDown(SDL_SCANCODE_K);
            else if (name == "Up")
                m_input.OnKeyDown(SDL_SCANCODE_UP);
            else if (name == "Down")
                m_input.OnKeyDown(SDL_SCANCODE_DOWN);
            else if (name == "Left")
                m_input.OnKeyDown(SDL_SCANCODE_LEFT);
            else if (name == "Right")
                m_input.OnKeyDown(SDL_SCANCODE_RIGHT);
        }
        else if (args[i] == "--warp" && bHasValue)
        {
            m_startWarpCount = std::atoi(args[++i].c_str());
            m_bDebugDraw     = true;
        }
        else if (args[i] == "--warp-boss")
        {
            m_pendingWarp = 2;   // F6 와 같다: 스테이지 보스 아레나
            m_bDebugDraw  = true;
        }
        else if (args[i] == "--capture-ticks" && bHasValue)
        {
            const slug::String list  = args[++i];
            size_t             begin = 0;
            while (begin < list.size())
            {
                size_t end = list.find(',', begin);
                if (end == slug::String::npos)
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
        slug::Result<ReplayData> loaded = LoadReplay(m_replayPath);
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
        const bool bHasBaked = std::filesystem::exists(slug::FindAssetRoot() / "Baked");
        m_stateName          = bHasBaked ? slug::String { GetDefaultStateName() } : slug::String { "sandbox" };
    }

    if (const slug::Result<void> result = m_renderer.Init("MyMetalSlug2", ChooseWindowScale_()); !result)
    {
        SLUG_LOG_FATAL("렌더러 초기화 실패: {}", result.GetError().MakeMessage());
        m_returnCode = 1;
        m_bRunning   = false;
        return;
    }

    const slug::FilePath bakedRoot = slug::FindAssetRoot() / "Baked";
    m_sprites.Init(m_renderer.GetNative());
    if (slug::Result<AnimationLibrary> library = AnimationLibrary::LoadFromDirectory(bakedRoot))
    {
        m_anims = library.Take();
        if (const slug::Result<void> atlas = m_sprites.LoadAtlasPages(m_anims); !atlas)
        {
            SLUG_LOG_WARN("아틀라스 로드 실패: {}", atlas.GetError().MakeMessage());
        }
    }
    else
    {
        SLUG_LOG_WARN("Baked 에셋 없음 ({}): Scripts/Bake.ps1 을 실행하세요", bakedRoot.string());
    }

    m_rng                 = slug::RngMt19937 { m_seed };
    m_recording.seed      = m_seed;
    m_recording.stateName = m_stateName;

    m_ctx.pRenderer = &m_renderer;
    m_ctx.pSprites  = &m_sprites;
    m_ctx.pAnims    = &m_anims;
    m_ctx.pRng      = &m_rng;
    m_ctx.assetRoot = slug::FindAssetRoot();

    {
        AudioConfig audioConfig;
        audioConfig.bEnabled = !m_bNoAudio && m_capturePath.empty();
        if (const slug::Result<void> audio = m_audio.InitFromDirectory(bakedRoot, audioConfig); audio)
        {
            m_ctx.pAudio = &m_audio;
        }
        else
        {
            SLUG_LOG_WARN("sounds.json 을 읽지 못해 소리 없이 계속합니다: {}", audio.GetError().MakeMessage());
        }
    }

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
}

void App::Shutdown_()
{
    SaveRecording_();
    m_pRunner.reset();
    m_ctx.pAudio = nullptr;
    m_audio.Shutdown();
    m_sprites.Shutdown();
    m_renderer.Shutdown();
}

void App::SaveRecording_() const
{
    if (m_recordPath.empty())
    {
        return;
    }
    if (const slug::Result<void> result = SaveReplay(m_recordPath, m_recording); !result)
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
            if (m_bDebugDraw) { m_pendingWarp = 1; }
            return;   // 스테이지: 다음 섹션
        case SDL_SCANCODE_F6:
            if (m_bDebugDraw) { m_pendingWarp = 2; }
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
    m_ctx.debugWarp   = m_pendingWarp;
    m_pendingWarp     = 0;
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
        steps = m_clock.Advance(static_cast<double>(_deltaTimeSec) / static_cast<double>(m_ctx.slowMoDivisor));
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

    ++m_renderFrameCount;
    if (!m_capturePath.empty() && !m_captureTicks.empty())
    {
        while (m_captureTickHead < m_captureTicks.size() && m_pRunner->GetTickCount() >= m_captureTicks[m_captureTickHead])
        {
            const uint64_t       tick = m_captureTicks[m_captureTickHead++];
            const slug::FilePath path = slug::FilePath { std::format("{}_{:05}.bmp", m_capturePath.string(), tick) };
            if (const slug::Result<void> result = m_renderer.SaveScreenshotBmp(path); !result)
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
        if (const slug::Result<void> result = m_renderer.SaveScreenshotBmp(m_capturePath); !result)
        {
            SLUG_LOG_ERROR("스크린샷 저장 실패: {}", result.GetError().MakeMessage());
            m_returnCode = 2;
        }
        m_bRunning = false;
    }

    m_renderer.EndFrame();
}
}   // namespace slug
