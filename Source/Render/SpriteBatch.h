#pragma once
#include <cstdint>

#include "Base/SlugError.h"
#include "Base/Types.h"
#include "Render/Animation.h"
#include "Render/Camera.h"

struct SDL_Renderer;
struct SDL_Texture;
struct SDL_FRect;

struct IntRect
{
    int x = 0;
    int y = 0;
    int w = 0;
    int h = 0;

    [[nodiscard]] constexpr bool operator==(const IntRect&) const = default;
};

[[nodiscard]] IntRect ComputeDestRect(const AnimFrame& _frame, VECTOR2 _worldOrigin, VECTOR2 _cameraPos, bool _bFlipX, bool _bFlipY = false);

[[nodiscard]] RGBA ApplyHitFlashRamp(RGBA _color);

struct SpriteDrawOptions
{
    bool  bFlipX      = false;
    bool  bFlipY      = false;
    RGBA  tint        = { 255, 255, 255, 255 };
    float flashAmount = 0.f;   // 0.5 이상이면 플래시 변환본으로 그린다
    float rotationDeg = 0.f;   // 시계 방향 회전
    float scale       = 1.f;
};

inline constexpr int kBackdropDepth     = -1000;
inline constexpr int kHudDepth          = 1000;
inline constexpr int kPlayerDepth       = 20;
inline constexpr int kPlayerBulletDepth = 30;
inline constexpr int kPlayerBombDepth   = 31;
inline constexpr int kBulletHitDepth    = 55;
inline constexpr int kBombBlastDepth    = 60;
inline constexpr int kEnemyShotDepth    = 32;

struct SpriteCommand
{
    int               depth         = 0;
    int               sortKey       = 0;
    uint32_t          seq           = 0;   // 제출 순서
    AnimFrame         frame         = {};
    VECTOR2           origin        = { 0.f, 0.f };
    SpriteDrawOptions options       = {};
    int               standalone    = -1;
    IntRect           standaloneSrc = {};
};

void SortSpriteCommands(Vector<SpriteCommand>& _commands);

struct TextureViewRects
{
    bool    bVisible = false;
    IntRect src      = {};
    IntRect dst      = {};
};

// 아틀라스 페이지의 피격 플래시 변환본 RGBA
[[nodiscard]] Vector<uint8_t> BuildHitFlashRgba(Span<const uint8_t> _rgba, size_t _pixelCount);

struct DecodedImage
{
    int             width     = 0;
    int             height    = 0;
    Vector<uint8_t> rgba      = {};
    Vector<uint8_t> flashRgba = {};
};

[[nodiscard]] Vector<Result<DecodedImage>> DecodeImages(Span<const FilePath> _paths, bool _bBuildFlash, int _numWorkers);

void SetLoadTraceEnabled(bool _bEnabled);

[[nodiscard]] TextureViewRects ComputeTextureView(int _texW, int _texH, VECTOR2 _texWorldPos, VECTOR2 _cameraPos, int _viewW, int _viewH);

class SpriteBatch
{
public:
    SpriteBatch() = default;
    ~SpriteBatch();
    [[nodiscard]] static SpriteBatch& GetSingleton();
    SpriteBatch(const SpriteBatch&)                          = delete;
    SpriteBatch&               operator=(const SpriteBatch&) = delete;
    void                       Init(SDL_Renderer* _pRenderer);
    void                       Shutdown();
    [[nodiscard]] Result<void> AddAtlasPageFromRgba(int _width, int _height, Span<const uint8_t> _rgba);
    [[nodiscard]] Result<void> AddAtlasPageFromRgba(int _width, int _height, Span<const uint8_t> _rgba, Span<const uint8_t> _flashRgba);
    [[nodiscard]] Result<void> AddAtlasPage(const DecodedImage& _image);
    // 페이지를 _numWorkers 장씩 병렬 디코드해 순서대로 추가
    [[nodiscard]] bool        LoadAtlasPagesParallel(const AnimationLibrary& _library, int _numWorkers);
    [[nodiscard]] size_t      GetAtlasPageCount() const;
    [[nodiscard]] Result<int> LoadStandaloneTexture(const FilePath& _pngPath);
    [[nodiscard]] Result<int> AddStandaloneTextureFromRgba(int _width, int _height, Span<const uint8_t> _rgba);
    [[nodiscard]] Result<int> AddStandaloneTexture(const DecodedImage& _image);
    [[nodiscard]] Vector<int> LoadStandaloneTexturesParallel(Span<const FilePath> _paths, int _numWorkers);
    void                      DrawTexture(int _handle, VECTOR2 _texWorldPos, VECTOR2 _cameraPos, int _viewW, int _viewH, RGBA _tint = { 255, 255, 255, 255 }) const;
    void                      DrawTextureRepeatX(int _handle, VECTOR2 _texWorldPos, VECTOR2 _cameraPos, int _viewW, int _viewH, float _fromX, int _tileX, int _tileW) const;
    void                      SetAssetRoot(const FilePath& _root);
    [[nodiscard]] int         FindStandalone(const String& _relativeFile);
    void                      SubmitStandalone(int _depth, int _sortKey, int _handle, const IntRect& _src, VECTOR2 _worldCenter, const SpriteDrawOptions& _options = {});
    void                      DrawFrame(const AnimFrame& _frame, VECTOR2 _worldOrigin, VECTOR2 _cameraPos, const SpriteDrawOptions& _options = {}) const;
    void                      DrawFrame(const AnimFrame& _frame, VECTOR2 _worldOrigin, const Camera& _camera, const SpriteDrawOptions& _options = {}) const;
    void                      DrawFrameRect(const AnimFrame& _frame, const IntRect& _dest, const SpriteDrawOptions& _options = {}) const;
    void                      Submit(int _depth, int _sortKey, const AnimFrame& _frame, VECTOR2 _worldOrigin, const SpriteDrawOptions& _options = {});
    void                      Flush(VECTOR2 _cameraPos);
    void                      Flush(const Camera& _camera);

    // depth 가 _depth 보다
    void                                       FlushBelowDepth(VECTOR2 _cameraPos, int _depth);
    void                                       Clear();
    [[nodiscard]] const Vector<SpriteCommand>& GetPendingCommands() const;

private:
    struct Page
    {
        SDL_Texture* pTexture      = nullptr;
        int          width         = 0;
        int          height        = 0;
        SDL_Texture* pFlashTexture = nullptr;   // 피격 플래시 변환본
    };

    struct StandaloneTexture
    {
        int                  width  = 0;
        int                  height = 0;
        int                  stripW = 0;
        Vector<SDL_Texture*> strips = {};
    };

    void DrawStandaloneRect_(const StandaloneTexture& _texture, const SDL_FRect& _src, const SDL_FRect& _dst, RGBA _tint) const;
    void DrawCommand_(const SpriteCommand& _command, VECTOR2 _cameraPos) const;

    SDL_Renderer*             m_pRenderer        = nullptr;
    Vector<Page>              m_pages            = {};
    Vector<StandaloneTexture> m_textures         = {};
    Vector<SpriteCommand>     m_commands         = {};
    FilePath                  m_assetRoot        = {};
    HashMap<String, int>      m_namedStandalones = {};
    uint32_t                  m_nextSeq          = 0;
};
