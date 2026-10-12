#pragma once
#include <cstdint>

#include "Base/Types.h"
#include "Game/Player/PlayerTypes.h"
#include "Editor/AnimEditor.h"
#include "Editor/DepthEditor.h"
#include "Editor/EnemyLab.h"
#include "Editor/GroundEditor.h"
#include "Editor/HitboxEditor.h"
#include "Editor/MotionEditor.h"
#include "Editor/StageEditor.h"

union SDL_Event;
struct SDL_Window;
struct SDL_Renderer;

class Renderer;

// --editor 모드 전용 ImGui 루트 패널. 일반 모드에서는 Init 자체를 호출하지 않는다.
class Editor final
{
public:
    [[nodiscard]] bool Init(SDL_Window* _pWindow, SDL_Renderer* _pRenderer);
    void               Shutdown();
    void               ProcessEvent(const SDL_Event& _event);
    void               SelectAnimation(const char* _name);   // --editor-anim
    void               SelectTab(const char* _name);
    void               SetAnimLibrary(AnimationLibrary* _pAnims);   // Animation 탭 편집을 실행 중 라이브러리에 반영
    void               SelectStage(const char* _name);
    void               EnableFloatTest();   // --editor-float-test
    void               SetGodMode(eGodMode _mode);   // --editor-god
    [[nodiscard]] eGodMode GetGodMode() const;
    [[nodiscard]] EnemyLab& GetLab();
    void               DrawLabOverlay(Renderer& _renderer);

    // 게임 프레임을 다 그린 뒤, 스크린샷/Present 전에 호출한다.
    void Frame(float _fps, uint64_t _tick);

    [[nodiscard]] bool IsInitialized() const;
    [[nodiscard]] bool OwnsKeyboard() const;   // ImGui 창이 키보드를 쓰는 중 (게임 입력 차단)

private:
    void DrawMenuBar_();
    void DrawEditorWindow_(float _fps, uint64_t _tick);

    SDL_Renderer* m_pRenderer = nullptr;
    AnimEditor    m_animEditor = {};
    StageEditor   m_stageEditor = {};
    GroundEditor  m_groundEditor = {};
    EnemyLab      m_enemyLab = {};
    HitboxEditor  m_hitboxEditor = {};
    MotionEditor  m_motionEditor = {};
    DepthEditor   m_depthEditor = {};
    eGodMode      m_godMode  = eGodMode::Off;
    String        m_pendingTab = {};
    bool          m_bInit     = false;
    bool          m_bShowDemo = false;
    bool          m_bQuit     = false;
    bool          m_bOwnsKeyboard = false;
    bool          m_bFloatTest = false;
    int           m_frameCount = 0;
};
