#pragma once
#include "Base/Math.h"
#include "Base/RGBA.h"
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

    [[nodiscard]] Result<void> Init(const char* _title, int _windowScale);
    void                       Shutdown();

    void BeginFrame(RGBA _clearColor);
    void EndFrame();

    void FillRect(Vec2 _topLeft, Vec2 _size, RGBA _color);
    void DrawRect(Vec2 _topLeft, Vec2 _size, RGBA _color);
    void DrawLine(Vec2 _from, Vec2 _to, RGBA _color);

    static constexpr int kDebugCharSize = 8;
    void                 DrawDebugText(Vec2 _topLeft, StringView _text, RGBA _color);

    [[nodiscard]] SDL_Renderer* GetNative() const;
    [[nodiscard]] SDL_Window*   GetWindow() const;

    [[nodiscard]] Result<void> SaveScreenshotBmp(const FilePath& _path) const;

private:
    SDL_Window*   m_pWindow   = nullptr;
    SDL_Renderer* m_pRenderer = nullptr;
};

// 월드 좌표를 카메라 기준 화면 좌표(정수 칸)로 바꾼다
[[nodiscard]] Vec2 ToScreen(Vec2 _world, Vec2 _camera);

// 월드 AABB 를 화면에 외곽선으로 그린다 (디버그 표시, 최소 1px)
void DrawBox(Renderer& _renderer, const Aabb& _box, Vec2 _camera, RGBA _color);
