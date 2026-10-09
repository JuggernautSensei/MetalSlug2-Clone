#pragma once
#include "Base/Result.h"
#include "Base/RGBA.h"
#include "Base/Types.h"
#include "Base/Math.h"

struct SDL_Window;
struct SDL_Renderer;

namespace slug
{
class Renderer
{
public:
    static constexpr int kLogicalWidth  = 640;
    static constexpr int kLogicalHeight = 448;

    ~Renderer();

    [[nodiscard]] slug::Result<void> Init(const char* _title, int _windowScale);
    void                            Shutdown();

    void BeginFrame(slug::RGBA _clearColor);
    void EndFrame();

    void FillRect(Vec2 _topLeft, Vec2 _size, slug::RGBA _color);
    void DrawRect(Vec2 _topLeft, Vec2 _size, slug::RGBA _color);
    void DrawLine(Vec2 _from, Vec2 _to, slug::RGBA _color);

    static constexpr int kDebugCharSize = 8;
    void                 DrawDebugText(Vec2 _topLeft, slug::StringView _text, slug::RGBA _color);

    [[nodiscard]] SDL_Window*   GetWindow() const;
    [[nodiscard]] SDL_Renderer* GetNative() const;
    [[nodiscard]] bool          IsInitialized() const;

    [[nodiscard]] slug::Result<void> SaveScreenshotBmp(const slug::FilePath& _path) const;

private:
    SDL_Window*   m_pWindow   = nullptr;
    SDL_Renderer* m_pRenderer = nullptr;
};
}
