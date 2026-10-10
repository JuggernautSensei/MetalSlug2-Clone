#pragma once
#include <SDL3/SDL.h>

#include "Audio/AudioSystem.h"
#include "Base/Random.h"
#include "Base/Types.h"
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

class App final
{
public:
    int Run(int _argc, char** _argv);

private:
    [[nodiscard]] bool InitSystems_();
    void               Init_();
    void               Shutdown_();
    void               PollEvents_();
    void               OnKeyDown_(SDL_Scancode _key, bool _bRepeat);
    void               OnKeyUp_(SDL_Scancode _key);
    void               OnFocusLost_();
    void               Update_(float _deltaTimeSec);

    void ParseArgs_();
    void Tick_();
    void Render_(float _alpha);
    void SaveRecording_() const;

    bool           m_bRunning   = false;
    int            m_returnCode = 0;
    Vector<String> m_cmdArgs    = {};

    Renderer         m_renderer = {};
    SpriteBatch      m_sprites  = {};
    AnimationLibrary m_anims    = {};
    RngMt19937       m_rng      = RngMt19937 { uint64_t { 1 } };
    GameContext      m_ctx      = {};
    AudioSystem      m_audio    = {};
    InputSystem      m_input    = {};
    FixedStepClock   m_clock    = {};

    std::unique_ptr<SimRunner> m_pRunner = nullptr;

    String   m_stateName   = {};
    uint64_t m_seed        = 1;
    FilePath m_recordPath  = {};
    FilePath m_replayPath  = {};
    bool     m_bReplayHold = false;
    bool     m_bNoAudio    = false;

    ReplayData m_recording    = {};
    ReplayData m_playback     = {};
    size_t     m_playbackHead = 0;
    bool       m_bPlayback    = false;

    FilePath         m_capturePath      = {};
    int              m_captureFrame     = 30;
    int              m_renderFrameCount = 0;
    Vector<uint64_t> m_captureTicks     = {};
    size_t           m_captureTickHead  = 0;

    bool  m_bPaused        = false;
    bool  m_bStepOnce      = false;
    bool  m_bSlowMo        = false;
    bool  m_bDebugDraw     = false;
    int   m_pendingWarp    = 0;
    int   m_debugLives     = 0;
    int   m_startWarpCount = 0;
    int   m_pendingSpawn   = 0;
    int   m_pendingWeapon  = 0;
    float m_fps            = 0.f;
};

}   // namespace slug
