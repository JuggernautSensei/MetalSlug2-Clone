#include "Editor/ImGuiSdlRendererViewports.h"

#include <SDL3/SDL.h>
#include <imgui.h>
#include <imgui_impl_sdlrenderer3.h>

struct ViewportTarget
{
    SDL_Texture* pTexture = nullptr;
    int          width    = 0;
    int          height   = 0;
};

static SDL_Renderer* g_pRenderer = nullptr;

static void MoveToWindowOrigin(
    ImDrawData* _pDraw)
{
    const ImVec2 origin = _pDraw->DisplayPos;
    for (int n = 0; n < _pDraw->CmdListsCount; ++n)
    {
        ImDrawList* pList = _pDraw->CmdLists[n];
        for (ImDrawVert& vertex: pList->VtxBuffer)
        {
            vertex.pos.x -= origin.x;
            vertex.pos.y -= origin.y;
        }
        for (ImDrawCmd& cmd: pList->CmdBuffer)
        {
            cmd.ClipRect.x -= origin.x;
            cmd.ClipRect.y -= origin.y;
            cmd.ClipRect.z -= origin.x;
            cmd.ClipRect.w -= origin.y;
        }
    }
    _pDraw->DisplayPos = ImVec2 { 0.f, 0.f };
}

static void CreateTarget(
    ImGuiViewport* _pViewport)
{
    _pViewport->RendererUserData = IM_NEW(ViewportTarget)();
}

static void DestroyTarget(
    ImGuiViewport* _pViewport)
{
    ViewportTarget* pTarget = static_cast<ViewportTarget*>(_pViewport->RendererUserData);
    if (pTarget != nullptr)
    {
        SDL_DestroyTexture(pTarget->pTexture);
        IM_DELETE(pTarget);
    }
    _pViewport->RendererUserData = nullptr;
}

static void RenderTarget(
    ImGuiViewport* _pViewport,
    void*)
{
    ViewportTarget* pTarget = static_cast<ViewportTarget*>(_pViewport->RendererUserData);
    ImDrawData*     pDraw   = _pViewport->DrawData;
    SDL_Window*     pWindow = SDL_GetWindowFromID(static_cast<SDL_WindowID>(reinterpret_cast<intptr_t>(_pViewport->PlatformHandle)));
    const int       width   = static_cast<int>(pDraw->DisplaySize.x * pDraw->FramebufferScale.x);
    const int       height  = static_cast<int>(pDraw->DisplaySize.y * pDraw->FramebufferScale.y);
    if (pTarget == nullptr || pWindow == nullptr || width <= 0 || height <= 0)
    {
        return;
    }
    if (pTarget->pTexture == nullptr || pTarget->width != width || pTarget->height != height)
    {
        SDL_DestroyTexture(pTarget->pTexture);
        pTarget->pTexture = SDL_CreateTexture(g_pRenderer, SDL_PIXELFORMAT_RGBA32, SDL_TEXTUREACCESS_TARGET, width, height);
        pTarget->width    = width;
        pTarget->height   = height;
        if (pTarget->pTexture == nullptr)
        {
            return;
        }
    }

    MoveToWindowOrigin(pDraw);

    SDL_SetRenderTarget(g_pRenderer, pTarget->pTexture);
    SDL_SetRenderDrawColor(g_pRenderer, 0, 0, 0, 255);
    SDL_RenderClear(g_pRenderer);
    ImGui_ImplSDLRenderer3_RenderDrawData(pDraw, g_pRenderer);
    SDL_Surface* pPixels = SDL_RenderReadPixels(g_pRenderer, nullptr);
    SDL_SetRenderTarget(g_pRenderer, nullptr);
    if (pPixels == nullptr)
    {
        return;
    }
    SDL_Surface* pWindowSurface = SDL_GetWindowSurface(pWindow);
    if (pWindowSurface != nullptr)
    {
        SDL_BlitSurface(pPixels, nullptr, pWindowSurface, nullptr);
        SDL_UpdateWindowSurface(pWindow);
    }
    SDL_DestroySurface(pPixels);
}

void ImGuiSdlRendererViewports_Init(
    SDL_Renderer* _pRenderer)
{
    g_pRenderer = _pRenderer;
    ImGui::GetIO().BackendFlags |= ImGuiBackendFlags_RendererHasViewports;
    ImGuiPlatformIO& platformIo       = ImGui::GetPlatformIO();
    platformIo.Renderer_CreateWindow  = CreateTarget;
    platformIo.Renderer_DestroyWindow = DestroyTarget;
    platformIo.Renderer_RenderWindow  = RenderTarget;
}

void ImGuiSdlRendererViewports_RenderMain(
    ImDrawData* _pDraw)
{
    MoveToWindowOrigin(_pDraw);
    ImGui_ImplSDLRenderer3_RenderDrawData(_pDraw, g_pRenderer);
}

void ImGuiSdlRendererViewports_Shutdown()
{
    ImGui::DestroyPlatformWindows();
    ImGui::GetIO().BackendFlags &= ~ImGuiBackendFlags_RendererHasViewports;
    g_pRenderer = nullptr;
}
