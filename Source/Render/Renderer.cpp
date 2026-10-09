#include "Render/Renderer.h"

#include <SDL3/SDL.h>

#include <JugX/CoreLogger.h>
#include <JugX/Math.h>

#include "Core/Error.h"

namespace slug
{

Renderer::~Renderer()
{
    Shutdown();
}

jug::Result<void> Renderer::Init(
    const char* _title,
    const int   _windowScale)
{
    const int scale = _windowScale < 1 ? 1 : _windowScale;

    m_pWindow = SDL_CreateWindow(_title, kLogicalWidth * scale, kLogicalHeight * scale, SDL_WINDOW_RESIZABLE);
    if (!m_pWindow)
    {
        JUG_CORE_LOG_ERROR("SDL_CreateWindow 실패: {}", SDL_GetError());
        return jug::Error { eError::WindowCreate };
    }

    m_pRenderer = SDL_CreateRenderer(m_pWindow, nullptr);
    if (!m_pRenderer)
    {
        JUG_CORE_LOG_ERROR("SDL_CreateRenderer 실패: {}", SDL_GetError());
        SDL_DestroyWindow(m_pWindow);
        m_pWindow = nullptr;
        return jug::Error { eError::RendererCreate };
    }

    SDL_SetRenderVSync(m_pRenderer, 1);
    SDL_SetRenderLogicalPresentation(m_pRenderer, kLogicalWidth, kLogicalHeight, SDL_LOGICAL_PRESENTATION_INTEGER_SCALE);
    SDL_SetRenderDrawBlendMode(m_pRenderer, SDL_BLENDMODE_BLEND);

    JUG_CORE_LOG_INFO("렌더러: {} / 창 {}x{}", SDL_GetRendererName(m_pRenderer), kLogicalWidth * scale, kLogicalHeight * scale);
    return jug::Error { jug::kOK };
}

void Renderer::Shutdown()
{
    if (m_pRenderer)
    {
        SDL_DestroyRenderer(m_pRenderer);
        m_pRenderer = nullptr;
    }
    if (m_pWindow)
    {
        SDL_DestroyWindow(m_pWindow);
        m_pWindow = nullptr;
    }
}

void Renderer::BeginFrame(
    const jug::RGBA _clearColor)
{
    SDL_SetRenderDrawColor(m_pRenderer, _clearColor.r, _clearColor.g, _clearColor.b, _clearColor.a);
    SDL_RenderClear(m_pRenderer);
}

void Renderer::EndFrame()
{
    SDL_RenderPresent(m_pRenderer);
}

void Renderer::FillRect(
    const jug::VECTOR2 _topLeft,
    const jug::VECTOR2 _size,
    const jug::RGBA    _color)
{
    // 정수 스냅: 서브픽셀 위치로 인한 떨림을 막는다.
    const SDL_FRect rect = { std::floor(_topLeft.x), std::floor(_topLeft.y), std::floor(_size.x), std::floor(_size.y) };
    SDL_SetRenderDrawColor(m_pRenderer, _color.r, _color.g, _color.b, _color.a);
    SDL_RenderFillRect(m_pRenderer, &rect);
}

void Renderer::DrawRect(
    const jug::VECTOR2 _topLeft,
    const jug::VECTOR2 _size,
    const jug::RGBA    _color)
{
    const SDL_FRect rect = { std::floor(_topLeft.x), std::floor(_topLeft.y), std::floor(_size.x), std::floor(_size.y) };
    SDL_SetRenderDrawColor(m_pRenderer, _color.r, _color.g, _color.b, _color.a);
    SDL_RenderRect(m_pRenderer, &rect);
}

SDL_Window* Renderer::GetWindow() const
{
    return m_pWindow;
}

SDL_Renderer* Renderer::GetNative() const
{
    return m_pRenderer;
}

bool Renderer::IsInitialized() const
{
    return m_pRenderer != nullptr;
}

jug::Result<void> Renderer::SaveScreenshotBmp(
    const jug::FilePath& _path) const
{
    SDL_Surface* pSurface = SDL_RenderReadPixels(m_pRenderer, nullptr);
    if (!pSurface)
    {
        JUG_CORE_LOG_ERROR("SDL_RenderReadPixels 실패: {}", SDL_GetError());
        return jug::Error { eError::FileRead };
    }

    const jug::String pathUtf8 = _path.string();
    const bool        bSaved   = SDL_SaveBMP(pSurface, pathUtf8.c_str());
    SDL_DestroySurface(pSurface);
    if (!bSaved)
    {
        JUG_CORE_LOG_ERROR("SDL_SaveBMP 실패: {}", SDL_GetError());
        return jug::Error { eError::FileOpen };
    }
    return jug::Error { jug::kOK };
}

}   // namespace slug
