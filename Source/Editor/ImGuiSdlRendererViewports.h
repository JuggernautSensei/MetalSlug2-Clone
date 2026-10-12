#pragma once

struct SDL_Renderer;

struct ImDrawData;

// imgui_impl_sdlrenderer3 는 multi-viewport 를 지원하지 않는다.
// 보조 뷰포트를 메인 SDL_Renderer 의 렌더 타깃에 그린 뒤 창 서피스로 복사한다.
// ImGui_ImplSDLRenderer3_Init 직후에 호출한다.
void ImGuiSdlRendererViewports_Init(SDL_Renderer* _pRenderer);
void ImGuiSdlRendererViewports_RenderMain(ImDrawData* _pDraw);   // 메인 뷰포트용: 창 원점 보정 후 그린다
void ImGuiSdlRendererViewports_Shutdown();
