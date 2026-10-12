#include "Editor/Editor.h"

#include <SDL3/SDL.h>
#include <imgui.h>
#include <imgui_impl_sdl3.h>
#include <imgui_internal.h>
#include <imgui_impl_sdlrenderer3.h>

#include "Editor/ImGuiSdlRendererViewports.h"
#include "Render/Renderer.h"
#include "Base/Log.h"

bool Editor::Init(
    SDL_Window*   _pWindow,
    SDL_Renderer* _pRenderer)
{
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.IniFilename = nullptr;
    io.Fonts->AddFontDefault();
    ImFontConfig koreanFont;
    koreanFont.MergeMode = true;
    (void)io.Fonts->AddFontFromFileTTF("C:/Windows/Fonts/malgun.ttf", 14.f, &koreanFont, io.Fonts->GetGlyphRangesKorean());
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable | ImGuiConfigFlags_ViewportsEnable;
    ImGui::GetStyle().WindowRounding = 0.f;
    if (!ImGui_ImplSDL3_InitForSDLRenderer(_pWindow, _pRenderer) || !ImGui_ImplSDLRenderer3_Init(_pRenderer))
    {
        ImGui::DestroyContext();
        return false;
    }
    ImGuiSdlRendererViewports_Init(_pRenderer);
    m_pRenderer = _pRenderer;
    m_animEditor.Init(_pRenderer);
    m_stageEditor.Init(_pRenderer);
    m_groundEditor.Init(_pRenderer);
    m_bInit     = true;
    return true;
}

void Editor::Shutdown()
{
    if (!m_bInit)
    {
        return;
    }
    m_groundEditor.Shutdown();
    m_stageEditor.Shutdown();
    m_animEditor.Shutdown();
    ImGuiSdlRendererViewports_Shutdown();
    ImGui_ImplSDLRenderer3_Shutdown();
    ImGui_ImplSDL3_Shutdown();
    ImGui::DestroyContext();
    m_pRenderer = nullptr;
    m_bInit     = false;
}

void Editor::ProcessEvent(
    const SDL_Event& _event)
{
    if (m_bInit)
    {
        ImGui_ImplSDL3_ProcessEvent(&_event);
    }
}

void Editor::SetAnimLibrary(
    AnimationLibrary* _pAnims)
{
    m_animEditor.SetLibrary(_pAnims);
    m_motionEditor.SetAnimLibrary(_pAnims);
}

void Editor::SelectAnimation(
    const char* _name)
{
    m_animEditor.SelectClip(_name);
}

void Editor::SelectTab(
    const char* _name)
{
    m_pendingTab = _name;
}

void Editor::SelectStage(
    const char* _name)
{
    m_stageEditor.SelectStage(_name);
}

void Editor::EnableFloatTest()
{
    m_bFloatTest = true;
}

void Editor::SetGodMode(
    const eGodMode _mode)
{
    m_godMode = _mode;
}

eGodMode Editor::GetGodMode() const
{
    return m_godMode;
}

EnemyLab& Editor::GetLab()
{
    return m_enemyLab;
}

void Editor::DrawLabOverlay(
    Renderer& _renderer)
{
    if (m_bInit)
    {
        m_enemyLab.DrawOverlay(_renderer);
        m_motionEditor.DrawOverlay(m_enemyLab, _renderer);
        m_hitboxEditor.DrawOverlay(m_enemyLab, _renderer);
    }
}

bool Editor::IsInitialized() const
{
    return m_bInit;
}

bool Editor::OwnsKeyboard() const
{
    return m_bInit && m_bOwnsKeyboard;
}

void Editor::Frame(
    const float    _fps,
    const uint64_t _tick)
{
    if (!m_bInit)
    {
        return;
    }

    // 논리 해상도(640x448) 프레젠테이션 중에는 ImGui 가 작게 그려지고 마우스 좌표도 어긋난다.
    // SDL3 마우스 이벤트는 창 픽셀 좌표이므로, ImGui 를 그리는 동안만 프레젠테이션을 끈다.
    int                             logicalW = 0;
    int                             logicalH = 0;
    SDL_RendererLogicalPresentation mode     = SDL_LOGICAL_PRESENTATION_DISABLED;
    SDL_GetRenderLogicalPresentation(m_pRenderer, &logicalW, &logicalH, &mode);
    SDL_SetRenderLogicalPresentation(m_pRenderer, 0, 0, SDL_LOGICAL_PRESENTATION_DISABLED);

    ImGui_ImplSDLRenderer3_NewFrame();
    ImGui_ImplSDL3_NewFrame();
    ImGui::NewFrame();

    ImGui::DockSpaceOverViewport(0, ImGui::GetMainViewport(), ImGuiDockNodeFlags_PassthruCentralNode);
    DrawMenuBar_();
    DrawEditorWindow_(_fps, _tick);
    m_animEditor.Poll();
    if (m_bShowDemo)
    {
        ImGui::ShowDemoWindow(&m_bShowDemo);
    }

    const ImGuiIO&     io      = ImGui::GetIO();
    const ImGuiWindow* pNav    = ImGui::GetCurrentContext()->NavWindow;
    const bool         bFocus  = pNav != nullptr && (pNav->Flags & ImGuiWindowFlags_DockNodeHost) == 0;
    m_bOwnsKeyboard            = io.WantCaptureKeyboard || io.WantTextInput || bFocus;

    ImGui::Render();
    ImGuiSdlRendererViewports_RenderMain(ImGui::GetDrawData());
    ImGui::UpdatePlatformWindows();
    ImGui::RenderPlatformWindowsDefault();

    ++m_frameCount;
    if (m_bFloatTest && m_frameCount == 30)
    {
        SLUG_LOG_INFO("viewports={}", ImGui::GetPlatformIO().Viewports.Size);
    }

    SDL_SetRenderLogicalPresentation(m_pRenderer, logicalW, logicalH, mode);

    if (m_bQuit)
    {
        SDL_Event quit {};
        quit.type = SDL_EVENT_QUIT;
        SDL_PushEvent(&quit);
        m_bQuit = false;
    }
}

void Editor::DrawMenuBar_()
{
    if (!ImGui::BeginMainMenuBar())
    {
        return;
    }
    if (ImGui::BeginMenu("File"))
    {
        if (ImGui::MenuItem("Quit"))
        {
            m_bQuit = true;
        }
        ImGui::EndMenu();
    }
    if (ImGui::BeginMenu("View"))
    {
        ImGui::MenuItem("ImGui Demo", nullptr, &m_bShowDemo);
        ImGui::EndMenu();
    }
    ImGui::EndMainMenuBar();
}

void Editor::DrawEditorWindow_(
    const float    _fps,
    const uint64_t _tick)
{
    const ImGuiViewport* pHost = ImGui::GetMainViewport();
    ImGui::SetNextWindowPos(ImVec2 { pHost->Pos.x + 16.f, pHost->Pos.y + 40.f }, ImGuiCond_FirstUseEver);
    if (m_bFloatTest && m_frameCount == 5)
    {
        const ImGuiViewport* pMain = ImGui::GetMainViewport();
        ImGui::SetNextWindowPos(ImVec2 { pMain->Pos.x + pMain->Size.x + 40.f, pMain->Pos.y + 40.f }, ImGuiCond_Always);
    }
    ImGui::SetNextWindowSize(ImVec2 { 380.f, 520.f }, ImGuiCond_FirstUseEver);
    if (ImGui::Begin("Editor"))
    {
        ImGui::Text("FPS %.1f   Tick %llu", static_cast<double>(_fps), static_cast<unsigned long long>(_tick));
        ImGui::Separator();
        ImGui::TextUnformatted("Player");
        for (int i = 0; i < static_cast<int>(eGodMode::Count); ++i)
        {
            const eGodMode mode = static_cast<eGodMode>(i);
            ImGui::SameLine();
            if (ImGui::RadioButton(GetGodModeName(mode), m_godMode == mode))
            {
                m_godMode = mode;
            }
        }
        ImGui::Separator();
        if (ImGui::BeginTabBar("EditorTabs"))
        {
            const ImGuiTabItemFlags animFlags = (m_pendingTab == "animation") ? ImGuiTabItemFlags_SetSelected : ImGuiTabItemFlags_None;
            const ImGuiTabItemFlags mapFlags  = (m_pendingTab == "map") ? ImGuiTabItemFlags_SetSelected : ImGuiTabItemFlags_None;
            const ImGuiTabItemFlags collisionFlags = (m_pendingTab == "collision") ? ImGuiTabItemFlags_SetSelected : ImGuiTabItemFlags_None;
            const ImGuiTabItemFlags aiFlags = (m_pendingTab == "ai") ? ImGuiTabItemFlags_SetSelected : ImGuiTabItemFlags_None;
            m_pendingTab.clear();
            if (ImGui::BeginTabItem("Animation", nullptr, animFlags))
            {
                m_animEditor.Draw();
                ImGui::EndTabItem();
            }
            if (ImGui::BeginTabItem("Map", nullptr, mapFlags))
            {
                m_stageEditor.Draw();
                ImGui::EndTabItem();
            }
            if (ImGui::BeginTabItem("Collision", nullptr, collisionFlags))
            {
                m_groundEditor.Draw();
                ImGui::EndTabItem();
            }
            if (ImGui::BeginTabItem("AI", nullptr, aiFlags))
            {
                m_enemyLab.Draw();
                m_hitboxEditor.Draw(m_enemyLab);
                m_motionEditor.Draw(m_enemyLab);
                m_depthEditor.Draw(m_enemyLab);
                ImGui::EndTabItem();
            }
            ImGui::EndTabBar();
        }
    }
    ImGui::End();
}
