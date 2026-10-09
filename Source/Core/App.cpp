#include "Core/App.h"

#include <SDL3/SDL.h>

#include <JugX/ConsoleLogger.h>
#include <JugX/CoreLogger.h>
#include <JugX/EventDispatcher.h>

#include <algorithm>
#include <cstdlib>
#include <filesystem>
#include <format>

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
        else if (args[i] == "--warp" && bHasValue)
        {
            // 스테이지 디버그: 시작하자마자 다음 섹션으로 N 번 워프 (F1 디버그 켜짐)
            m_startWarpCount = std::atoi(args[++i].c_str());
            m_bDebugDraw     = true;
        }
        else if (args[i] == "--capture-ticks" && bHasValue)
        {
            // 쉼표로 구분된 틱 목록 (오름차순으로 정렬해 사용)
            const jug::String list = args[++i];
            size_t            begin = 0;
            while (begin < list.size())
            {
                size_t end = list.find(',', begin);
                if (end == jug::String::npos)
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
        // 기본 상태: Baked 에셋이 있으면 타이틀 (타이틀 → stage0), 없으면 샌드박스 (에셋 없이도 동작)
        const bool bHasBaked = std::filesystem::exists(jug::FilePath { SLUG_ASSET_DIR } / "Baked");
        m_stateName          = bHasBaked ? jug::String { GetDefaultStateName() } : jug::String { "sandbox" };
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

    // 오디오: 실패/--no-audio/캡처 모드에서는 장치 없이(또는 NullAudio 로) 계속한다. 소리는 시뮬레이션에 영향을 주지 않는다.
    {
        AudioConfig audioConfig;
        audioConfig.bEnabled = !m_bNoAudio && m_capturePath.empty();
        if (const jug::Result<void> audio = m_audio.InitFromDirectory(bakedRoot, audioConfig); audio)
        {
            m_ctx.pAudio = &m_audio;
        }
        else
        {
            JUG_CORE_LOG_WARN("sounds.json 을 읽지 못해 소리 없이 계속합니다: {}", audio.GetError().MakeMessage());
        }
    }

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

    // 디버그 키는 시뮬레이션 입력이 아니라 앱 제어이다 (리플레이에 기록되지 않는다).
    switch (_event.GetKey())
    {
        case jug::eKey::F1: m_bDebugDraw = !m_bDebugDraw; return;
        case jug::eKey::F2: m_bPaused = !m_bPaused; return;
        case jug::eKey::F3: m_bStepOnce = true; return;
        case jug::eKey::F4: m_bSlowMo = !m_bSlowMo; return;
        case jug::eKey::F5: if (m_bDebugDraw) { m_pendingWarp = 1; } return;   // 스테이지: 다음 섹션
        case jug::eKey::F6: if (m_bDebugDraw) { m_pendingWarp = 2; } return;   // 스테이지: 보스 아레나
        // 숫자키 1..8: 샌드박스 적 디버그 스폰 (보병 정의 8종, 플레이어 앞)
        case jug::eKey::Num1: m_pendingSpawn = 1; return;
        case jug::eKey::Num2: m_pendingSpawn = 2; return;
        case jug::eKey::Num3: m_pendingSpawn = 3; return;
        case jug::eKey::Num4: m_pendingSpawn = 4; return;
        case jug::eKey::Num5: m_pendingSpawn = 5; return;
        case jug::eKey::Num6: m_pendingSpawn = 6; return;
        case jug::eKey::Num7: m_pendingSpawn = 7; return;
        case jug::eKey::Num8: m_pendingSpawn = 8; return;
        case jug::eKey::Num9: m_pendingSpawn = 9; return;   // 차량 5종 순환 (SandboxState::SpawnDebugEnemy_ 의 슬롯 8)
        default: break;
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
    m_audio.Update(_deltaTimeSec);   // 끝난 소리 회수 + BGM 페이드 (렌더 프레임 단위)

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
        m_pendingWarp = 1;   // --warp N: F5 를 N 번 누른 것과 같다 (다음 섹션으로 N 번 워프)
        --m_startWarpCount;
    }
    m_ctx.debugWarp     = m_pendingWarp;   // 상태가 소비한다 (스테이지 F5/F6)
    m_pendingWarp       = 0;
    if (m_pendingSpawn > 0)
    {
        m_ctx.debugSpawn = m_pendingSpawn;   // 샌드박스가 소비한다 (적 디버그 스폰)
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
    if (!m_capturePath.empty() && !m_captureTicks.empty())
    {
        while (m_captureTickHead < m_captureTicks.size() && m_pRunner->GetTickCount() >= m_captureTicks[m_captureTickHead])
        {
            const uint64_t     tick = m_captureTicks[m_captureTickHead++];
            const jug::FilePath path = jug::FilePath { std::format("{}_{:05}.bmp", m_capturePath.string(), tick) };
            if (const jug::Result<void> result = m_renderer.SaveScreenshotBmp(path); !result)
            {
                JUG_CORE_LOG_ERROR("스크린샷 저장 실패: {}", result.GetError().MakeMessage());
                SetReturnCode(2);
            }
        }
        if (m_captureTickHead >= m_captureTicks.size())
        {
            Quit();
        }
    }
    else if (!m_capturePath.empty() && m_renderFrameCount == m_captureFrame)
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
