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
class AsyncLoader;

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
    float rotationDeg = 0.f;                      // 시계 방향 회전 (도, 0 = 없음). 피벗(frame.pivotX/Y) 점을 중심으로 돈다 (로켓 진행 방향)
    float scale       = 1.f;                      // 배율 (1 = 원본). 피벗 점을 고정한 채 확대/축소한다 (원작 비행기 등 원근 축소용)
};

// 깊이 축: 작을수록 뒤. 모든 그리기는 하나의 depth 값을 가진다 (Docs/DepthSpec.md)
inline constexpr int kBackdropDepth     = -1000;   // 배경 (지형 텍스처보다 뒤)
inline constexpr int kHudDepth          = 1000;    // HUD/UI (스프라이트 위)
inline constexpr int kPlayerDepth       = 20;      // 플레이어 몸
inline constexpr int kPlayerBulletDepth = 30;      // 플레이어/슬러그 탄
inline constexpr int kPlayerBombDepth   = 31;      // 슬러그 포탄
inline constexpr int kBulletHitDepth    = 55;      // 탄 소멸 이펙트
inline constexpr int kBombBlastDepth    = 60;      // 폭탄 폭발 이펙트
inline constexpr int kEnemyShotDepth    = 32;      // 적 투사체 (탄 30/폭탄 31 위)

struct SpriteCommand
{
    int               depth         = 0;
    int               sortKey       = 0;   // 같은 depth 안의 동점 처리 1순위 (예: 발 y 좌표), 2순위는 seq
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

// --trace-load: 텍스처 로드마다 "[tex] ..." 한 줄을 stderr 로 남긴다 (꺼져 있으면 아무 일도 하지 않는다)
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

    [[nodiscard]] Result<void> LoadAtlasPage(const FilePath& _pngPath);
    [[nodiscard]] Result<void> LoadAtlasPages(const AnimationLibrary& _library);
    [[nodiscard]] Result<void> AddAtlasPageFromRgba(int _width, int _height, Span<const uint8_t> _rgba);
    // _flashRgba 는 BuildHitFlashRgba 로 만든 변환본 (워커에서 미리 만들어 둘 때)
    [[nodiscard]] Result<void> AddAtlasPageFromRgba(int _width, int _height, Span<const uint8_t> _rgba, Span<const uint8_t> _flashRgba);
    [[nodiscard]] size_t       GetAtlasPageCount() const;

    [[nodiscard]] Result<int> LoadStandaloneTexture(const FilePath& _pngPath);
    [[nodiscard]] Result<int> AddStandaloneTextureFromRgba(int _width, int _height, Span<const uint8_t> _rgba);

    // 비동기 로드: PNG 디코드(+플래시 변환본) 는 워커에서, 텍스처 생성/등록은 메인 스레드에서 제출 순서대로 한다.
    // 결과는 _loader.WaitAll() 이후 확정된다 (페이지 번호/핸들은 동기 로드와 같은 순서). 워커가 0 이면 그 자리에서 끝난다.
    void LoadAtlasPagesAsync(const AnimationLibrary& _library, AsyncLoader& _loader);
    // 로드 실패 시 *_pOutHandle 은 -1. 완료 전까지 _pOutHandle 이 가리키는 메모리는 그대로 있어야 한다.
    void LoadStandaloneTextureAsync(const FilePath& _pngPath, AsyncLoader& _loader, int* _pOutHandle);

    void DrawTexture(int _handle, Vec2 _texWorldPos, Vec2 _cameraPos, int _viewW, int _viewH, RGBA _tint = { 255, 255, 255, 255 }) const;
    // 텍스처 x >= _fromX 구간을 [_tileX, _tileX + _tileW) 열의 주기 반복으로 그린다 (보스전 엔드리스 배경). _texWorldPos 는 원점 (0,0) 권장.
    void DrawTextureRepeatX(int _handle, Vec2 _texWorldPos, Vec2 _cameraPos, int _viewW, int _viewH, float _fromX, int _tileX, int _tileW) const;

    // 단독 텍스처 (sprites.json 의 file 항목) 를 에셋 루트 기준 상대 경로로 한 번만 읽는다. 실패하면 -1
    void              SetAssetRoot(const FilePath& _root);
    [[nodiscard]] int FindStandalone(const String& _relativeFile);
    // 단독 텍스처의 _src 구간을 _worldCenter 가 구간 가운데에 오도록 제출한다
    void SubmitStandalone(int _depth, int _sortKey, int _handle, const IntRect& _src, Vec2 _worldCenter, const SpriteDrawOptions& _options = {});

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
        int                  stripW = 0;   // 띠 하나의 가로 (마지막 띠는 더 짧을 수 있다)
        Vector<SDL_Texture*> strips = {};
    };

    // src(텍스처 좌표) 를 dst 로 그린다. 띠 경계를 가로지르면 띠마다 나눠서 그린다 (가로 배율은 dst.w / src.w).
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
