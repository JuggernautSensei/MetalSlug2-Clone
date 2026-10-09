#pragma once
#include <JugX/Application.h>
#include <JugX/Random.h>

#include "Core/FixedStep.h"
#include "Core/Input.h"
#include "Core/Replay.h"
#include "Game/GameState.h"
#include "Game/SimRunner.h"
#include "Render/Animation.h"
#include "Render/Renderer.h"
#include "Render/SpriteBatch.h"

namespace slug
{

// ===========================================
//  App
//   JugX Application 파생. 가변 프레임 → 고정 60Hz 시뮬레이션(SimRunner) + 보간 렌더.
//
//   명령행:
//     --state <name>            시작 게임 상태 (기본: StateFactory 의 기본값)
//     --seed <n>                RNG 시드
//     --record <path>           플레이한 입력을 리플레이로 저장 (종료 시)
//     --replay <path>           리플레이 입력으로 시뮬레이션 (끝나면 종료, --replay-hold 면 유지)
//     --capture <path> [--capture-frame <n>]   n 번째 렌더 프레임을 BMP 로 저장하고 종료
// ===========================================

class App final : public jug::Application
{
public:
    App();
    ~App() override;

protected:
    void Init() override;
    void Shutdown() override;
    void OnEvent(jug::Event& _event) override;
    void Update(float _deltaTimeSec) override;

private:
    void OnKeyDown_(const jug::KeyDownEvent& _event);
    void OnKeyUp_(const jug::KeyUpEvent& _event);
    void OnFocusLost_(const jug::WindowFocusLostEvent& _event);

    void ParseArgs_();
    void Tick_();
    void Render_(float _alpha);
    void SaveRecording_() const;

    Renderer          m_renderer  = {};
    SpriteBatch       m_sprites   = {};
    AnimationLibrary  m_anims     = {};
    jug::RngMt19937   m_rng       = jug::RngMt19937 { uint64_t { 1 } };
    GameContext       m_ctx       = {};
    InputSystem       m_input     = {};
    FixedStepClock    m_clock     = {};

    jug::Scoped<SimRunner> m_pRunner = nullptr;

    // 명령행 옵션
    jug::String   m_stateName  = {};
    uint64_t      m_seed       = 1;
    jug::FilePath m_recordPath = {};
    jug::FilePath m_replayPath = {};
    bool          m_bReplayHold = false;

    // 리플레이
    ReplayData m_recording    = {};
    ReplayData m_playback     = {};
    size_t     m_playbackHead = 0;
    bool       m_bPlayback    = false;

    // QA 캡처
    jug::FilePath m_capturePath      = {};
    int           m_captureFrame     = 30;
    int           m_renderFrameCount = 0;
};

}   // namespace slug
