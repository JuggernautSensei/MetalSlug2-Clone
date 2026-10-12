#pragma once
#include <cstdint>

#include "Base/Math.h"
#include "Base/RGBA.h"
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

[[nodiscard]] IntRect ComputeDestRect(const AnimFrame& _frame, Vec2 _worldOrigin, Vec2 _cameraPos, bool _bFlipX, bool _bFlipY = false);

// 피격 플래시: 원작은 스프라이트 전체를 밝기 기준 주황 램프로 바꿔 그린다 (Docs/Spec_HitFlash.md, 버스 실측).
[[nodiscard]] RGBA ApplyHitFlashRamp(RGBA _color);

struct SpriteDrawOptions
{
    bool  bFlipX      = false;
    bool  bFlipY      = false;
    RGBA  tint        = { 255, 255, 255, 255 };
    float flashAmount = 0.f;   // 0.5 이상이면 플래시 변환본으로 그린다 (0 = 끔)
    float rotationDeg = 0.f;   // 시계 방향 회전 (도, 0 = 없음). 피벗(frame.pivotX/Y) 점을 중심으로 돈다 (로켓 진행 방향)
    float scale       = 1.f;   // 배율 (1 = 원본). 피벗 점을 고정한 채 확대/축소한다 (원작 비행기 등 원근 축소용)
};

// 깊이 축: 작을수록 뒤. 모든 그리기는 하나의 depth 값을 가진다 (Docs/DepthSpec.md)
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
    uint32_t          seq           = 0;   // 제출 순서 (동점 처리 2순위)
    AnimFrame         frame         = {};
    Vec2              origin        = { 0.f, 0.f };
    SpriteDrawOptions options       = {};
    int               standalone    = -1;   // >= 0 이면 frame 대신 이 단독 텍스처의 standaloneSrc 를 origin 가운데로 그린다
    IntRect           standaloneSrc = {};
};

void SortSpriteCommands(Vector<SpriteCommand>& _commands);

struct TextureViewRects
{
    bool    bVisible = false;
    IntRect src      = {};
    IntRect dst      = {};
};

// 아틀라스 페이지의 피격 플래시 변환본 RGBA. 순수 CPU 작업이라 워커 스레드에서 불러도 된다.
[[nodiscard]] Vector<uint8_t> BuildHitFlashRgba(Span<const uint8_t> _rgba, size_t _pixelCount);

struct DecodedImage
{
    int             width     = 0;
    int             height    = 0;
    Vector<uint8_t> rgba      = {};
    Vector<uint8_t> flashRgba = {};
};

// PNG 파일 읽기 + 디코드 (+ 플래시 변환본) 를 병렬로 한다. 결과는 _paths 순서 그대로. 메모리 상한은 호출부가 한 번에 넘기는 개수로 정한다.
[[nodiscard]] Vector<Result<DecodedImage>> DecodeImages(Span<const FilePath> _paths, bool _bBuildFlash, int _numWorkers);

void SetLoadTraceEnabled(bool _bEnabled);

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

    [[nodiscard]] Result<void> AddAtlasPageFromRgba(int _width, int _height, Span<const uint8_t> _rgba);
    [[nodiscard]] Result<void> AddAtlasPageFromRgba(int _width, int _height, Span<const uint8_t> _rgba, Span<const uint8_t> _flashRgba);
    [[nodiscard]] Result<void> AddAtlasPage(const DecodedImage& _image);
    // 페이지를 _numWorkers 장씩 병렬 디코드해 순서대로 추가한다. 실패한 페이지에서 멈춘다 (페이지 번호가 어긋나지 않게).
    [[nodiscard]] bool   LoadAtlasPagesParallel(const AnimationLibrary& _library, int _numWorkers);
    [[nodiscard]] size_t GetAtlasPageCount() const;

    [[nodiscard]] Result<int> LoadStandaloneTexture(const FilePath& _pngPath);
    [[nodiscard]] Result<int> AddStandaloneTextureFromRgba(int _width, int _height, Span<const uint8_t> _rgba);
    [[nodiscard]] Result<int> AddStandaloneTexture(const DecodedImage& _image);
    [[nodiscard]] Vector<int> LoadStandaloneTexturesParallel(Span<const FilePath> _paths, int _numWorkers);

    void DrawTexture(int _handle, Vec2 _texWorldPos, Vec2 _cameraPos, int _viewW, int _viewH, RGBA _tint = { 255, 255, 255, 255 }) const;
    // 텍스처 x >= _fromX 구간을 [_tileX, _tileX + _tileW) 열의 주기 반복으로 그린다 (보스전 엔드리스 배경). _texWorldPos 는 원점 (0,0) 권장.
    void DrawTextureRepeatX(int _handle, Vec2 _texWorldPos, Vec2 _cameraPos, int _viewW, int _viewH, float _fromX, int _tileX, int _tileW) const;

    void              SetAssetRoot(const FilePath& _root);
    [[nodiscard]] int FindStandalone(const String& _relativeFile);
    void              SubmitStandalone(int _depth, int _sortKey, int _handle, const IntRect& _src, Vec2 _worldCenter, const SpriteDrawOptions& _options = {});

    void DrawFrame(const AnimFrame& _frame, Vec2 _worldOrigin, Vec2 _cameraPos, const SpriteDrawOptions& _options = {}) const;
    void DrawFrame(const AnimFrame& _frame, Vec2 _worldOrigin, const Camera& _camera, const SpriteDrawOptions& _options = {}) const;

    void DrawFrameRect(const AnimFrame& _frame, const IntRect& _dest, const SpriteDrawOptions& _options = {}) const;

    void Submit(int _depth, int _sortKey, const AnimFrame& _frame, Vec2 _worldOrigin, const SpriteDrawOptions& _options = {});
    void Flush(Vec2 _cameraPos);
    void Flush(const Camera& _camera);

    // depth 가 _depth 보다 작은 명령만 정렬해 먼저 그리고 지운다 (나머지는 그대로 남는다). 지면 텍스처보다 뒤에 놓일 원경 물체 (보스 선체) 용
    void FlushBelowDepth(Vec2 _cameraPos, int _depth);
    void Clear();

    [[nodiscard]] const Vector<SpriteCommand>& GetPendingCommands() const;

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
        int                  stripW = 0;
        Vector<SDL_Texture*> strips = {};
    };

    void DrawStandaloneRect_(const StandaloneTexture& _texture, const SDL_FRect& _src, const SDL_FRect& _dst, RGBA _tint) const;
    void DrawCommand_(const SpriteCommand& _command, Vec2 _cameraPos) const;

    SDL_Renderer*             m_pRenderer        = nullptr;
    Vector<Page>              m_pages            = {};
    Vector<StandaloneTexture> m_textures         = {};
    Vector<SpriteCommand>     m_commands         = {};
    FilePath                  m_assetRoot        = {};
    HashMap<String, int>      m_namedStandalones = {};
    uint32_t                  m_nextSeq          = 0;
};
