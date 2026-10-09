#pragma once
#include <JugX/Application.h>
#include <JugX/Random.h>

#include "Audio/AudioSystem.h"
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
//     --no-audio                오디오 장치를 열지 않는다 (소리 요청은 조용히 무시)
//     --capture <path> [--capture-frame <n>]   n 번째 렌더 프레임을 BMP 로 저장하고 종료
//     --capture <prefix> --capture-ticks a,b,c  시뮬레이션 틱 a,b,c 를 마친 화면을 <prefix>_<tick>.bmp 로 저장하고 종료
//                                               (이 모드는 렌더 프레임마다 정확히 1틱만 진행해 결정적으로 캡처한다)
//   디버그 키: F1 판정 상자 표시, F2 일시정지, F3 한 틱 진행(일시정지 중), F4 슬로모션(1/4), F5/F6 스테이지 워프(F1 켜진 상태: 다음 섹션 / 보스 아레나)
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
    AudioSystem       m_audio     = {};
    InputSystem       m_input     = {};
    FixedStepClock    m_clock     = {};

    jug::Scoped<SimRunner> m_pRunner = nullptr;

    // 명령행 옵션
    jug::String   m_stateName  = {};
    uint64_t      m_seed       = 1;
    jug::FilePath m_recordPath = {};
    jug::FilePath m_replayPath = {};
    bool          m_bReplayHold = false;
    bool          m_bNoAudio    = false;

    // 리플레이
    ReplayData m_recording    = {};
    ReplayData m_playback     = {};
    size_t     m_playbackHead = 0;
    bool       m_bPlayback    = false;

    // QA 캡처
    jug::FilePath m_capturePath      = {};
    int           m_captureFrame     = 30;
    int           m_renderFrameCount = 0;
    jug::Vector<uint64_t> m_captureTicks = {};
    size_t        m_captureTickHead  = 0;

    // 디버그 제어
    bool  m_bPaused     = false;
    bool  m_bStepOnce   = false;
    bool  m_bSlowMo     = false;
    bool  m_bDebugDraw  = false;
    int   m_pendingWarp = 0;   // F5/F6 (F1 켜진 상태): 다음 Update 에서 GameContext::debugWarp 로 전달
    int   m_pendingSpawn = 0;  // 숫자키 1..8 (적 디버그 스폰): 다음 Update 에서 GameContext::debugSpawn 으로 전달
    float m_fps         = 0.f;
};

}   // namespace slug
