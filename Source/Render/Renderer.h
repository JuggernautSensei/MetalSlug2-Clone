#pragma once
#include "Base/Result.h"
#include "Base/RGBA.h"
#include "Base/Types.h"
#include "Base/Math.h"

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

    [[nodiscard]] slug::Result<void> Init(const char* _title, int _windowScale);
    void                            Shutdown();

    void BeginFrame(slug::RGBA _clearColor);
    void EndFrame();

    // 논리 좌표(픽셀) 기준. 좌표는 정수로 스냅하여 그린다.
    void FillRect(Vec2 _topLeft, Vec2 _size, slug::RGBA _color);
    void DrawRect(Vec2 _topLeft, Vec2 _size, slug::RGBA _color);
    void DrawLine(Vec2 _from, Vec2 _to, slug::RGBA _color);

    // SDL 내장 8x8 디버그 폰트로 문자열을 그린다 (ASCII). 개행 문자는 줄바꿈.
    static constexpr int kDebugCharSize = 8;
    void                 DrawDebugText(Vec2 _topLeft, slug::StringView _text, slug::RGBA _color);

    [[nodiscard]] SDL_Window*   GetWindow() const;
    [[nodiscard]] SDL_Renderer* GetNative() const;
    [[nodiscard]] bool          IsInitialized() const;

    // 화면(논리 좌표)을 BMP 로 저장한다. 자동 캡처/QA 용.
    [[nodiscard]] slug::Result<void> SaveScreenshotBmp(const slug::FilePath& _path) const;

private:
    SDL_Window*   m_pWindow   = nullptr;
    SDL_Renderer* m_pRenderer = nullptr;
};

}   // namespace slug
