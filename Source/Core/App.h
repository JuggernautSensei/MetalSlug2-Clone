#pragma once
#include <SDL3/SDL.h>

#include "Audio/AudioSystem.h"
#include "Base/Types.h"
#include "Core/FixedStep.h"
#include "Core/Input.h"
#include "Editor/Editor.h"
#include "Game/GameState.h"
#include "Game/SimRunner.h"
#include "Render/Animation.h"
#include "Render/Renderer.h"
#include "Render/SpriteBatch.h"

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

    Timer m_processTimer = {};

    bool           m_bRunning   = false;
    int            m_returnCode = 0;
    Vector<String> m_cmdArgs    = {};

    InputSystem    m_input = {};
    FixedStepClock m_clock = {};

    Scoped<SimRunner> m_pRunner = nullptr;

    String   m_stateName      = {};
    uint64_t m_seed           = 1;
    bool     m_bNoAudio       = false;
    bool     m_bNoAsyncLoad   = false;
    bool     m_bEditor        = false;
    bool     m_bEditorKeys    = false;
    String   m_editorAnim     = {};
    String   m_editorTab      = {};
    String   m_editorStage    = {};
    String   m_editorLabSpawn = {};
    String   m_editorGod      = {};
    Editor   m_editor         = {};


    FilePath         m_capturePath      = {};
    int              m_captureFrame     = 30;
    int              m_renderFrameCount = 0;
    Vector<uint64_t> m_captureTicks     = {};
    size_t           m_captureTickHead  = 0;

    bool  m_bStepOnce      = false;
    bool  m_bSlowMo        = false;
    int   m_startWarpCount = 0;
    float m_startWarpX     = 0.f;
};
