#pragma once
#include <JugX/Result.h>
#include <JugX/RGBA.h>
#include <JugX/Vector2.h>

struct SDL_Window;
struct SDL_Renderer;

namespace slug
{

// ===========================================
//  Renderer
//   SDL3 GPU 렌더러 래퍼. 논리 해상도 640x448 을 정수배로 확대한다 (nearest, vsync).
// ===========================================

class Renderer
{
public:
    static constexpr int kLogicalWidth  = 640;
    static constexpr int kLogicalHeight = 448;

    ~Renderer();

    [[nodiscard]] jug::Result<void> Init(const char* _title, int _windowScale);
    void                            Shutdown();

    void BeginFrame(jug::RGBA _clearColor);
    void EndFrame();

    // 논리 좌표(픽셀) 기준. 좌표는 정수로 스냅하여 그린다.
    void FillRect(jug::VECTOR2 _topLeft, jug::VECTOR2 _size, jug::RGBA _color);
    void DrawRect(jug::VECTOR2 _topLeft, jug::VECTOR2 _size, jug::RGBA _color);

    [[nodiscard]] SDL_Window*   GetWindow() const;
    [[nodiscard]] SDL_Renderer* GetNative() const;
    [[nodiscard]] bool          IsInitialized() const;

    // 화면(논리 좌표)을 BMP 로 저장한다. 자동 캡처/QA 용.
    [[nodiscard]] jug::Result<void> SaveScreenshotBmp(const jug::FilePath& _path) const;

private:
    SDL_Window*   m_pWindow   = nullptr;
    SDL_Renderer* m_pRenderer = nullptr;
};

}   // namespace slug
