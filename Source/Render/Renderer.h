#pragma once
#include "Base/SlugError.h"
#include "Base/Types.h"
#include "World/Geometry.h"

struct SDL_Window;
struct SDL_Renderer;

class Renderer
{
public:
    static constexpr int kLogicalWidth  = 640;
    static constexpr int kLogicalHeight = 448;

    ~Renderer();
    [[nodiscard]] static Renderer& GetSingleton();
    [[nodiscard]] Result<void>     Init(const char* _title, int _windowScale);
    void                           Shutdown();
    void                           BeginFrame(RGBA _clearColor);
    void                           EndFrame();
    void                           FillRect(VECTOR2 _topLeft, VECTOR2 _size, RGBA _color);
    void                           DrawRect(VECTOR2 _topLeft, VECTOR2 _size, RGBA _color);
    void                           DrawLine(VECTOR2 _from, VECTOR2 _to, RGBA _color);

    static constexpr int        kDebugCharSize = 8;
    void                        DrawDebugText(VECTOR2 _topLeft, StringView _text, RGBA _color);
    [[nodiscard]] SDL_Renderer* GetNative() const;
    [[nodiscard]] SDL_Window*   GetWindow() const;
    [[nodiscard]] Result<void>  SaveScreenshotBmp(const FilePath& _path) const;

private:
    SDL_Window*   m_pWindow   = nullptr;
    SDL_Renderer* m_pRenderer = nullptr;
};

[[nodiscard]] VECTOR2 ToScreen(VECTOR2 _world, VECTOR2 _camera);

void DrawBox(Renderer& _renderer, const Aabb& _box, VECTOR2 _camera, RGBA _color);
