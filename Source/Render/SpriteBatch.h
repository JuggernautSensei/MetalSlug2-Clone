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
struct SDL_FRect;

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

[[nodiscard]] IntRect ComputeDestRect(const AnimFrame& _frame, Vec2 _worldOrigin, Vec2 _cameraPos, bool _bFlipX, bool _bFlipY = false);

// 피격 플래시: 원작은 스프라이트 전체를 밝기 기준 주황 램프로 바꿔 그린다 (Docs/Spec_HitFlash.md, 버스 실측).
// 아틀라스 페이지마다 램프 변환본 텍스처를 한 벌 더 만들어 두고, flashAmount 가 켜진 프레임은 변환본으로 그린다.
// 입력 RGB 의 평균 밝기 L 을 램프로 매핑한 색을 돌려준다 (알파는 그대로).
[[nodiscard]] RGBA ApplyHitFlashRamp(RGBA _color);

struct SpriteDrawOptions
{
    bool  bFlipX      = false;
    bool  bFlipY      = false;                    // 세로 반전 (SV-001 발칸탄처럼 한 사분면만 있는 스프라이트용)
    RGBA  tint        = { 255, 255, 255, 255 };   // 곱셈 색 + 알파
    float flashAmount = 0.f;                      // 0.5 이상이면 플래시 변환본으로 그린다 (0 = 끔)
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
        SDL_Texture* pTexture      = nullptr;
        int          width         = 0;
        int          height        = 0;
        SDL_Texture* pFlashTexture = nullptr;   // 피격 플래시 변환본 (아틀라스 페이지만, 없으면 nullptr)
    };

    // 단독 텍스처: GPU 최대 텍스처 크기(구형 GPU 는 8192) 를 넘는 가로 긴 이미지(Stage0_Main 14094px)를 세로 띠로 쪼개 들고 있다
    struct StandaloneTexture
    {
        int                  width  = 0;
        int                  height = 0;
        int                  stripW = 0;   // 띠 하나의 가로 (마지막 띠는 더 짧을 수 있다)
        Vector<SDL_Texture*> strips = {};
    };

    // src(텍스처 좌표) 를 dst 로 그린다. 띠 경계를 가로지르면 띠마다 나눠서 그린다 (가로 배율은 dst.w / src.w).
    void DrawStandaloneRect_(const StandaloneTexture& _texture, const SDL_FRect& _src, const SDL_FRect& _dst, RGBA _tint) const;

    SDL_Renderer*            m_pRenderer = nullptr;
    Vector<Page>             m_pages     = {};
    Vector<StandaloneTexture> m_textures = {};
    Vector<SpriteCommand> m_commands  = {};
    uint32_t              m_nextSeq   = 0;
};

}   // namespace slug
