#include "Render/Renderer.h"

#include <SDL3/SDL.h>

#include "Base/SlugError.h"
#include "Base/Log.h"
#include "Base/Math.h"
#include "Base/RGBA.h"
#include "Base/Types.h"

Renderer::~Renderer()
{
    Shutdown();
}

Result<void> Renderer::Init(
    const char* _title,
    const int   _windowScale)
{
    const int scale = _windowScale < 1 ? 1 : _windowScale;

    m_pWindow = SDL_CreateWindow(_title, kLogicalWidth * scale, kLogicalHeight * scale, SDL_WINDOW_RESIZABLE);
    if (!m_pWindow)
    {
        SLUG_LOG_ERROR("SDL_CreateWindow 실패: {}", SDL_GetError());
        return Error { eError::WindowCreate };
    }

    m_pRenderer = SDL_CreateRenderer(m_pWindow, nullptr);
    if (!m_pRenderer)
    {
        SLUG_LOG_ERROR("SDL_CreateRenderer 실패: {}", SDL_GetError());
        SDL_DestroyWindow(m_pWindow);
        m_pWindow = nullptr;
        return Error { eError::RendererCreate };
    }

    SDL_SetRenderVSync(m_pRenderer, 1);
    SDL_SetRenderLogicalPresentation(m_pRenderer, kLogicalWidth, kLogicalHeight, SDL_LOGICAL_PRESENTATION_INTEGER_SCALE);
    SDL_SetRenderDrawBlendMode(m_pRenderer, SDL_BLENDMODE_BLEND);

    SLUG_LOG_INFO("렌더러: {} / 창 {}x{}", SDL_GetRendererName(m_pRenderer), kLogicalWidth * scale, kLogicalHeight * scale);
    return Error { kOK };
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
    const RGBA _clearColor)
{
    SDL_SetRenderDrawColor(m_pRenderer, _clearColor.r, _clearColor.g, _clearColor.b, _clearColor.a);
    SDL_RenderClear(m_pRenderer);
}

void Renderer::EndFrame()
{
    SDL_RenderPresent(m_pRenderer);
}

void Renderer::FillRect(
    const Vec2 _topLeft,
    const Vec2 _size,
    const RGBA _color)
{
    const SDL_FRect rect = { std::floor(_topLeft.x), std::floor(_topLeft.y), std::floor(_size.x), std::floor(_size.y) };
    SDL_SetRenderDrawColor(m_pRenderer, _color.r, _color.g, _color.b, _color.a);
    SDL_RenderFillRect(m_pRenderer, &rect);
}

void Renderer::DrawRect(
    const Vec2 _topLeft,
    const Vec2 _size,
    const RGBA _color)
{
    const SDL_FRect rect = { std::floor(_topLeft.x), std::floor(_topLeft.y), std::floor(_size.x), std::floor(_size.y) };
    SDL_SetRenderDrawColor(m_pRenderer, _color.r, _color.g, _color.b, _color.a);
    SDL_RenderRect(m_pRenderer, &rect);
}

void Renderer::DrawLine(
    const Vec2 _from,
    const Vec2 _to,
    const RGBA _color)
{
    SDL_SetRenderDrawColor(m_pRenderer, _color.r, _color.g, _color.b, _color.a);
    SDL_RenderLine(m_pRenderer, std::floor(_from.x) + 0.5f, std::floor(_from.y) + 0.5f, std::floor(_to.x) + 0.5f, std::floor(_to.y) + 0.5f);
}

void Renderer::DrawDebugText(
    const Vec2       _topLeft,
    const StringView _text,
    const RGBA       _color)
{
    SDL_SetRenderDrawColor(m_pRenderer, _color.r, _color.g, _color.b, _color.a);

    const float x = std::floor(_topLeft.x);
    float       y = std::floor(_topLeft.y);

    String line;
    for (const char c: _text)
    {
        if (c == '\n')
        {
            SDL_RenderDebugText(m_pRenderer, x, y, line.c_str());
            line.clear();
            y += static_cast<float>(kDebugCharSize + 2);
        }
        else
        {
            line += c;
        }
    }
    if (!line.empty())
    {
        SDL_RenderDebugText(m_pRenderer, x, y, line.c_str());
    }
}

SDL_Renderer* Renderer::GetNative() const
{
    return m_pRenderer;
}

SDL_Window* Renderer::GetWindow() const
{
    return m_pWindow;
}

Result<void> Renderer::SaveScreenshotBmp(
    const FilePath& _path) const
{
    SDL_Surface* pSurface = SDL_RenderReadPixels(m_pRenderer, nullptr);
    if (!pSurface)
    {
        SLUG_LOG_ERROR("SDL_RenderReadPixels 실패: {}", SDL_GetError());
        return Error { eError::FileRead };
    }

    const String pathUtf8 = _path.string();
    const bool   bSaved   = SDL_SaveBMP(pSurface, pathUtf8.c_str());
    SDL_DestroySurface(pSurface);
    if (!bSaved)
    {
        SLUG_LOG_ERROR("SDL_SaveBMP 실패: {}", SDL_GetError());
        return Error { eError::FileOpen };
    }
    return Error { kOK };
}
