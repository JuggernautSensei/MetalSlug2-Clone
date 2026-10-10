#pragma once
#include <cstdint>

#include "Base/Math.h"
#include "Base/RGBA.h"
#include "Base/Result.h"
#include "Base/Types.h"
#include "Render/Animation.h"
#include "Render/Camera.h"

struct SDL_Renderer;
struct SDL_Texture;

namespace slug
{

struct IntRect
{
    int x = 0;
    int y = 0;
    int w = 0;
    int h = 0;

    [[nodiscard]] constexpr bool operator==(const IntRect&) const = default;
};

[[nodiscard]] IntRect ComputeDestRect(const AnimFrame& _frame, Vec2 _worldOrigin, Vec2 _cameraPos, bool _bFlipX);

// 피격 플래시: 같은 프레임을 ADD 블렌드로 한 번 더 그린다 (주황 틴트, 스프라이트 알파로 마스크).
inline constexpr uint8_t kHitFlashR        = 255;
inline constexpr uint8_t kHitFlashG        = 110;
inline constexpr uint8_t kHitFlashB        = 40;
inline constexpr float   kHitFlashMaxAlpha = 0.5f;   // flashAmount 1 일 때 ADD 알파

struct SpriteDrawOptions
{
    bool  bFlipX      = false;
    RGBA  tint        = { 255, 255, 255, 255 };   // 곱셈 색 + 알파
    float flashAmount = 0.f;                      // 0 = 끔, 1 = 최대 플래시 (0..1 로 클램프)
};

struct SpriteCommand
{
    int               layer   = 0;
    int               sortKey = 0;   // 같은 레이어 안의 명시적 정렬 키 (예: 발 y 좌표)
    uint32_t          seq     = 0;   // 제출 순서 (안정 정렬용)
    AnimFrame         frame   = {};
    Vec2              origin  = { 0.f, 0.f };
    SpriteDrawOptions options = {};
};

void SortSpriteCommands(Vector<SpriteCommand>& _commands);

struct TextureViewRects
{
    bool    bVisible = false;
    IntRect src      = {};
    IntRect dst      = {};
};

[[nodiscard]] TextureViewRects ComputeTextureView(int _texW, int _texH, Vec2 _texWorldPos, Vec2 _cameraPos, int _viewW, int _viewH);

class SpriteBatch
{
public:
    SpriteBatch() = default;
    ~SpriteBatch();

    SpriteBatch(const SpriteBatch&)            = delete;
    SpriteBatch& operator=(const SpriteBatch&) = delete;

    void Init(SDL_Renderer* _pRenderer);
    void Shutdown();

    [[nodiscard]] Result<void> LoadAtlasPage(const FilePath& _pngPath);
    [[nodiscard]] Result<void> LoadAtlasPages(const AnimationLibrary& _library);
    [[nodiscard]] Result<void> AddAtlasPageFromRgba(int _width, int _height, Span<const uint8_t> _rgba);
    [[nodiscard]] size_t       GetAtlasPageCount() const { return m_pages.size(); }

    [[nodiscard]] Result<int> LoadStandaloneTexture(const FilePath& _pngPath);
    [[nodiscard]] Result<int> AddStandaloneTextureFromRgba(int _width, int _height, Span<const uint8_t> _rgba);

    void DrawTexture(int _handle, Vec2 _texWorldPos, Vec2 _cameraPos, int _viewW, int _viewH, RGBA _tint = { 255, 255, 255, 255 }) const;
    // 텍스처 x >= _fromX 구간을 [_tileX, _tileX + _tileW) 열의 주기 반복으로 그린다 (보스전 엔드리스 배경). _texWorldPos 는 원점 (0,0) 권장.
    void DrawTextureRepeatX(int _handle, Vec2 _texWorldPos, Vec2 _cameraPos, int _viewW, int _viewH, float _fromX, int _tileX, int _tileW) const;

    void DrawFrame(const AnimFrame& _frame, Vec2 _worldOrigin, Vec2 _cameraPos, const SpriteDrawOptions& _options = {}) const;
    void DrawFrame(const AnimFrame& _frame, Vec2 _worldOrigin, const Camera& _camera, const SpriteDrawOptions& _options = {}) const;

    void DrawFrameRect(const AnimFrame& _frame, const IntRect& _dest, const SpriteDrawOptions& _options = {}) const;

    void Submit(int _layer, int _sortKey, const AnimFrame& _frame, Vec2 _worldOrigin, const SpriteDrawOptions& _options = {});
    void Flush(Vec2 _cameraPos);
    void Flush(const Camera& _camera);
    void Clear() { m_commands.clear(); }

    [[nodiscard]] const Vector<SpriteCommand>& GetPendingCommands() const { return m_commands; }

private:
    struct Page
    {
        SDL_Texture* pTexture = nullptr;
        int          width    = 0;
        int          height   = 0;
    };

    SDL_Renderer*         m_pRenderer = nullptr;
    Vector<Page>          m_pages     = {};
    Vector<Page>          m_textures  = {};
    Vector<SpriteCommand> m_commands  = {};
    uint32_t              m_nextSeq   = 0;
};

}   // namespace slug
