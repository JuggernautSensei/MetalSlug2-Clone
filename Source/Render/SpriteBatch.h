#pragma once
#include <cstdint>

#include <JugX/Result.h>
#include <JugX/RGBA.h>
#include <JugX/Typedef.h>
#include <JugX/Vector2.h>

#include "Render/Animation.h"
#include "Render/Camera.h"

struct SDL_Renderer;
struct SDL_Texture;

namespace slug
{

// ===========================================
//  순수 계산부 (SDL 불필요, 단위 테스트 대상)
// ===========================================

struct IntRect
{
    int x = 0;
    int y = 0;
    int w = 0;
    int h = 0;

    [[nodiscard]] constexpr bool operator==(const IntRect&) const = default;
};

// 프레임을 월드 위치 _worldOrigin 에 그릴 때의 화면 목적지 사각형.
// 원점을 먼저 정수로 스냅(floor)한 뒤 피벗을 빼므로 서브픽셀 위치와 무관하게 프레임 크기가 일정하다.
// _bFlipX: 원점을 축으로 좌우 반전 (피벗이 같은 원점에 놓인다).
[[nodiscard]] IntRect ComputeDestRect(const AnimFrame& _frame, jug::VECTOR2 _worldOrigin, jug::VECTOR2 _cameraPos, bool _bFlipX);

struct SpriteDrawOptions
{
    bool      bFlipX = false;
    jug::RGBA tint   = { 255, 255, 255, 255 };   // 곱셈 색 + 알파
};

// 정렬 키: layer → sortKey → 제출 순서(seq). 모두 같으면 먼저 제출한 것이 아래.
struct SpriteCommand
{
    int               layer   = 0;
    int               sortKey = 0;   // 같은 레이어 안의 명시적 정렬 키 (예: 발 y 좌표)
    uint32_t          seq     = 0;   // 제출 순서 (안정 정렬용)
    AnimFrame         frame   = {};
    jug::VECTOR2      origin  = { 0.f, 0.f };
    SpriteDrawOptions options = {};
};

void SortSpriteCommands(jug::Vector<SpriteCommand>& _commands);

// 독립 텍스처(큰 배경 등)를 월드의 _texWorldPos 에 놓았을 때, 화면(_viewW x _viewH, 월드 좌상단 _cameraPos)에 보이는 부분.
// src 는 텍스처 안의 사각형, dst 는 화면 좌표 사각형이다 (둘은 같은 크기). 보이는 부분이 없으면 bVisible = false.
// 위치는 floor 로 정수 스냅하므로 서브픽셀 카메라/원점에서도 가장자리가 흔들리지 않는다.
struct TextureViewRects
{
    bool    bVisible = false;
    IntRect src      = {};
    IntRect dst      = {};
};

[[nodiscard]] TextureViewRects ComputeTextureView(int _texW, int _texH, jug::VECTOR2 _texWorldPos, jug::VECTOR2 _cameraPos, int _viewW, int _viewH);

// ===========================================
//  SpriteBatch
//   아틀라스 PNG 페이지를 SDL_Texture(nearest) 로 올려 두고, 제출된 스프라이트를 정렬해서 그린다.
//   그리기 전용 상태만 가지며 게임 로직을 모른다.
// ===========================================

class SpriteBatch
{
public:
    SpriteBatch() = default;
    ~SpriteBatch();

    SpriteBatch(const SpriteBatch&)            = delete;
    SpriteBatch& operator=(const SpriteBatch&) = delete;

    void Init(SDL_Renderer* _pRenderer);   // 비소유. 렌더러보다 먼저 Shutdown 해야 한다
    void Shutdown();

    // --- 아틀라스 ---
    [[nodiscard]] jug::Result<void> LoadAtlasPage(const jug::FilePath& _pngPath);                                    // 페이지 인덱스 = 로드 순서
    [[nodiscard]] jug::Result<void> LoadAtlasPages(const AnimationLibrary& _library);                                // library.GetRoot()/pages[i]
    [[nodiscard]] jug::Result<void> AddAtlasPageFromRgba(int _width, int _height, jug::Span<const uint8_t> _rgba);   // 테스트/절차 생성용
    [[nodiscard]] size_t            GetAtlasPageCount() const { return m_pages.size(); }

    // --- 독립 텍스처 (아틀라스에 넣지 않는 큰 배경 등). 반환값은 핸들(로드 순서 인덱스) ---
    [[nodiscard]] jug::Result<int> LoadStandaloneTexture(const jug::FilePath& _pngPath);
    [[nodiscard]] jug::Result<int> AddStandaloneTextureFromRgba(int _width, int _height, jug::Span<const uint8_t> _rgba);
    [[nodiscard]] size_t           GetStandaloneTextureCount() const { return m_textures.size(); }

    // 텍스처의 좌상단을 월드 _texWorldPos 에 놓고 보이는 부분만 그린다.
    void DrawTexture(int _handle, jug::VECTOR2 _texWorldPos, jug::VECTOR2 _cameraPos, int _viewW, int _viewH, jug::RGBA _tint = { 255, 255, 255, 255 }) const;

    // --- 즉시 그리기 ---
    // 프레임 하나를 바로 그린다. 투명 프레임/없는 페이지는 무시한다.
    void DrawFrame(const AnimFrame& _frame, jug::VECTOR2 _worldOrigin, jug::VECTOR2 _cameraPos, const SpriteDrawOptions& _options = {}) const;
    void DrawFrame(const AnimFrame& _frame, jug::VECTOR2 _worldOrigin, const Camera& _camera, const SpriteDrawOptions& _options = {}) const;

    // UI 용: 프레임(트림된 사각형) 전체를 화면 논리 좌표 _dest 에 늘려 그린다 (피벗/카메라 무시, 정수 배율 확대 텍스트 등).
    void DrawFrameRect(const AnimFrame& _frame, const IntRect& _dest, const SpriteDrawOptions& _options = {}) const;

    // --- 지연 그리기 (정렬 후 Flush) ---
    void Submit(int _layer, int _sortKey, const AnimFrame& _frame, jug::VECTOR2 _worldOrigin, const SpriteDrawOptions& _options = {});
    void Flush(jug::VECTOR2 _cameraPos);   // 정렬 → 그리기 → 큐 비움
    void Flush(const Camera& _camera);
    void Clear() { m_commands.clear(); }

    [[nodiscard]] const jug::Vector<SpriteCommand>& GetPendingCommands() const { return m_commands; }

private:
    struct Page
    {
        SDL_Texture* pTexture = nullptr;
        int          width    = 0;
        int          height   = 0;
    };

    SDL_Renderer*              m_pRenderer = nullptr;
    jug::Vector<Page>          m_pages     = {};
    jug::Vector<Page>          m_textures  = {};   // 독립 텍스처
    jug::Vector<SpriteCommand> m_commands  = {};
    uint32_t                   m_nextSeq   = 0;
};

}   // namespace slug
