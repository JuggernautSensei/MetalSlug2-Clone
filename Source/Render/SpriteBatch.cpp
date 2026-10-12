#include "Render/SpriteBatch.h"

#include <algorithm>
#include <cmath>
#include <cstdio>
#include <cstring>
#include <execution>
#include <fstream>
#include <SDL3/SDL.h>

#include "Base/Math.h"
#include "Base/RGBA.h"
#include "Base/SlugError.h"
#include "Base/Types.h"

#pragma warning(disable : 4505)
#define STB_IMAGE_STATIC
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#include "Base/Log.h"
#include "Base/PhaseTimer.h"

namespace
{
[[nodiscard]] SDL_FlipMode FlipFlags_(
    const SpriteDrawOptions& _options)
{
    const int flags = (_options.bFlipX ? static_cast<int>(SDL_FLIP_HORIZONTAL) : 0) | (_options.bFlipY ? static_cast<int>(SDL_FLIP_VERTICAL) : 0);
    return static_cast<SDL_FlipMode>(flags);
}
}   // namespace

IntRect ComputeDestRect(
    const AnimFrame& _frame,
    const Vec2       _worldOrigin,
    const Vec2       _cameraPos,
    const bool       _bFlipX,
    const bool       _bFlipY)
{
    const int originX = FloorToInt(_worldOrigin.x - _cameraPos.x);
    const int originY = FloorToInt(_worldOrigin.y - _cameraPos.y);

    IntRect rect;
    rect.w = _frame.w;
    rect.h = _frame.h;
    rect.y = _bFlipY ? originY + _frame.pivotY - _frame.h : originY - _frame.pivotY;
    rect.x = _bFlipX ? originX + _frame.pivotX - _frame.w : originX - _frame.pivotX;
    return rect;
}

void SortSpriteCommands(
    Vector<SpriteCommand>& _commands)
{
    std::sort(_commands.begin(), _commands.end(), [](const SpriteCommand& _a, const SpriteCommand& _b) {
        if (_a.depth != _b.depth)
        {
            return _a.depth < _b.depth;
        }
        if (_a.sortKey != _b.sortKey)
        {
            return _a.sortKey < _b.sortKey;
        }
        return _a.seq < _b.seq;
    });
}

TextureViewRects ComputeTextureView(
    const int  _texW,
    const int  _texH,
    const Vec2 _texWorldPos,
    const Vec2 _cameraPos,
    const int  _viewW,
    const int  _viewH)
{
    TextureViewRects result;
    if (_texW <= 0 || _texH <= 0 || _viewW <= 0 || _viewH <= 0)
    {
        return result;
    }

    const int texX = FloorToInt(_texWorldPos.x);
    const int texY = FloorToInt(_texWorldPos.y);
    const int camX = FloorToInt(_cameraPos.x);
    const int camY = FloorToInt(_cameraPos.y);

    const int x0 = std::max(texX, camX);
    const int y0 = std::max(texY, camY);
    const int x1 = std::min(texX + _texW, camX + _viewW);
    const int y1 = std::min(texY + _texH, camY + _viewH);
    if (x1 <= x0 || y1 <= y0)
    {
        return result;
    }

    result.bVisible = true;
    result.src      = { x0 - texX, y0 - texY, x1 - x0, y1 - y0 };
    result.dst      = { x0 - camX, y0 - camY, x1 - x0, y1 - y0 };
    return result;
}

namespace
{
[[nodiscard]] Result<DecodedImage> DecodePngFile_(
    const FilePath& _pngPath)
{
    const ScopedPhaseTimer timer { "png-decode", _pngPath.filename().string() };

    std::ifstream file { _pngPath, std::ios::binary | std::ios::ate };
    if (!file)
    {
        return Error { eError::FileOpen };
    }

    const std::streamsize size = file.tellg();
    if (size <= 0)
    {
        return Error { eError::FileRead };
    }

    Vector<uint8_t> encoded(static_cast<size_t>(size));
    file.seekg(0, std::ios::beg);
    if (!file.read(reinterpret_cast<char*>(encoded.data()), size))
    {
        return Error { eError::FileRead };
    }

    int      width    = 0;
    int      height   = 0;
    int      channels = 0;
    stbi_uc* pPixels  = stbi_load_from_memory(encoded.data(), static_cast<int>(encoded.size()), &width, &height, &channels, 4);
    if (!pPixels)
    {
        SLUG_LOG_ERROR("PNG 디코드 실패: {}", stbi_failure_reason());
        return Error { eError::InvalidData };
    }

    DecodedImage image;
    image.width  = width;
    image.height = height;
    image.rgba.assign(pPixels, pPixels + static_cast<size_t>(width) * static_cast<size_t>(height) * 4);
    stbi_image_free(pPixels);
    return image;
}

bool g_bTraceLoad = false;

[[nodiscard]] uint64_t HashRgba_(
    const Span<const uint8_t> _rgba)
{
    uint64_t hash = 14695981039346656037ull;
    for (const uint8_t byte: _rgba)
    {
        hash ^= byte;
        hash *= 1099511628211ull;
    }
    return hash;
}

void TraceAtlasPage_(
    const size_t              _page,
    const FilePath&           _path,
    const int                 _width,
    const int                 _height,
    const Span<const uint8_t> _rgba)
{
    if (g_bTraceLoad)
    {
        std::fprintf(stderr, "[tex] atlas page=%zu file=%s %dx%d rgba=%016llx\n", _page, _path.filename().string().c_str(), _width, _height, static_cast<unsigned long long>(HashRgba_(_rgba)));
    }
}

void TraceStandalone_(
    const int                 _handle,
    const FilePath&           _path,
    const int                 _width,
    const int                 _height,
    const Span<const uint8_t> _rgba)
{
    if (g_bTraceLoad)
    {
        std::fprintf(stderr, "[tex] standalone handle=%d file=%s %dx%d rgba=%016llx\n", _handle, _path.filename().string().c_str(), _width, _height, static_cast<unsigned long long>(HashRgba_(_rgba)));
    }
}
}   // namespace

void SetLoadTraceEnabled(
    const bool _bEnabled)
{
    g_bTraceLoad = _bEnabled;
}

// 피격 플래시 램프 노드 (원작 버스 피격 프레임 실측: 입력 평균 밝기 L -> 출력 RGB)
struct FlashRampNode
{
    int l = 0;
    int r = 0;
    int g = 0;
    int b = 0;
};

namespace
{
const ARRAY<FlashRampNode, 17> kFlashRamp = {
    {
     { 0, 145, 52, 0 },
     { 8, 151, 56, 0 },
     { 24, 159, 61, 0 },
     { 40, 186, 80, 3 },
     { 56, 190, 90, 9 },
     { 72, 200, 106, 21 },
     { 88, 216, 126, 36 },
     { 104, 228, 140, 47 },
     { 120, 243, 154, 54 },
     { 136, 249, 163, 65 },
     { 152, 249, 182, 98 },
     { 168, 253, 198, 115 },
     { 184, 254, 210, 130 },
     { 200, 255, 221, 145 },
     { 216, 255, 234, 167 },
     { 232, 255, 248, 207 },
     { 255, 255, 255, 235 },
     }
};

uint8_t LerpChannel_(
    const int _from,
    const int _to,
    const int _t,
    const int _span)
{
    return static_cast<uint8_t>(_from + (_to - _from) * _t / _span);
}
}   // namespace

Vector<uint8_t> BuildHitFlashRgba(
    const Span<const uint8_t> _rgba,
    const size_t              _pixelCount)
{
    Vector<uint8_t> out(_pixelCount * 4);
    for (size_t i = 0; i < _pixelCount; ++i)
    {
        const size_t o = i * 4;
        const RGBA   c = ApplyHitFlashRamp({ _rgba[o], _rgba[o + 1], _rgba[o + 2], _rgba[o + 3] });
        out[o]         = c.r;
        out[o + 1]     = c.g;
        out[o + 2]     = c.b;
        out[o + 3]     = c.a;
    }
    return out;
}

namespace
{
[[nodiscard]] Result<DecodedImage> DecodeImage_(
    const FilePath& _path,
    const bool      _bBuildFlash)
{
    Result<DecodedImage> decoded = DecodePngFile_(_path);
    if (!decoded || !_bBuildFlash)
    {
        return decoded;
    }

    DecodedImage image = decoded.Take();
    {
        const ScopedPhaseTimer timer { "flash-build", _path.filename().string() };
        image.flashRgba = BuildHitFlashRgba(image.rgba, static_cast<size_t>(image.width) * static_cast<size_t>(image.height));
    }
    return image;
}
}   // namespace

Vector<Result<DecodedImage>> DecodeImages(
    const Span<const FilePath> _paths,
    const bool                 _bBuildFlash,
    const int                  _numWorkers)
{
    Vector<Result<DecodedImage>> results(_paths.size(), Result<DecodedImage> { Error { eError::InvalidData } });
    if (_numWorkers <= 1)
    {
        for (size_t i = 0; i < _paths.size(); ++i)
        {
            results[i] = DecodeImage_(_paths[i], _bBuildFlash);
        }
        return results;
    }

    std::for_each(std::execution::par, results.begin(), results.end(), [&](Result<DecodedImage>& _result) {
        _result = DecodeImage_(_paths[static_cast<size_t>(&_result - results.data())], _bBuildFlash);
    });
    return results;
}

namespace
{
// 텍스처 선택: 플래시가 켜졌고 변환본이 있으면 변환본.
SDL_Texture* PickTexture_(
    SDL_Texture* const _pNormal,
    SDL_Texture* const _pFlash,
    const float        _flashAmount)
{
    return (_flashAmount >= 0.5f && _pFlash) ? _pFlash : _pNormal;
}
}   // namespace

RGBA ApplyHitFlashRamp(
    const RGBA _color)
{
    const int l = (static_cast<int>(_color.r) + static_cast<int>(_color.g) + static_cast<int>(_color.b)) / 3;
    for (size_t i = 1; i < kFlashRamp.size(); ++i)
    {
        const FlashRampNode& hi = kFlashRamp[i];
        if (l <= hi.l)
        {
            const FlashRampNode& lo   = kFlashRamp[i - 1];
            const int            span = hi.l - lo.l;
            const int            t    = l - lo.l;
            return { LerpChannel_(lo.r, hi.r, t, span), LerpChannel_(lo.g, hi.g, t, span), LerpChannel_(lo.b, hi.b, t, span), _color.a };
        }
    }
    return { 255, 255, 235, _color.a };
}

SpriteBatch::~SpriteBatch()
{
    Shutdown();
}

void SpriteBatch::Init(
    SDL_Renderer* _pRenderer)
{
    Shutdown();
    m_pRenderer = _pRenderer;
}

void SpriteBatch::Shutdown()
{
    for (Page& page: m_pages)
    {
        if (page.pTexture)
        {
            SDL_DestroyTexture(page.pTexture);
        }
        if (page.pFlashTexture)
        {
            SDL_DestroyTexture(page.pFlashTexture);
        }
    }
    m_pages.clear();
    for (StandaloneTexture& texture: m_textures)
    {
        for (SDL_Texture* pStrip: texture.strips)
        {
            SDL_DestroyTexture(pStrip);
        }
    }
    m_textures.clear();
    m_namedStandalones.clear();
    m_commands.clear();
    m_nextSeq   = 0;
    m_pRenderer = nullptr;
}

Result<void> SpriteBatch::AddAtlasPageFromRgba(
    const int                 _width,
    const int                 _height,
    const Span<const uint8_t> _rgba)
{
    if (_width <= 0 || _height <= 0 || _rgba.size() < static_cast<size_t>(_width) * static_cast<size_t>(_height) * 4)
    {
        return Error { eError::InvalidData };
    }
    const Vector<uint8_t> flashRgba = BuildHitFlashRgba(_rgba, static_cast<size_t>(_width) * static_cast<size_t>(_height));
    return AddAtlasPageFromRgba(_width, _height, _rgba, flashRgba);
}

Result<void> SpriteBatch::AddAtlasPageFromRgba(
    const int                 _width,
    const int                 _height,
    const Span<const uint8_t> _rgba,
    const Span<const uint8_t> _flashRgba)
{
    if (!m_pRenderer || _width <= 0 || _height <= 0 || _rgba.size() < static_cast<size_t>(_width) * static_cast<size_t>(_height) * 4)
    {
        return Error { eError::InvalidData };
    }

    const ScopedPhaseTimer timer { "gpu-upload", "atlas page" };
    SDL_Texture*           pTexture = SDL_CreateTexture(m_pRenderer, SDL_PIXELFORMAT_RGBA32, SDL_TEXTUREACCESS_STATIC, _width, _height);
    if (!pTexture)
    {
        SLUG_LOG_ERROR("SDL_CreateTexture 실패: {}", SDL_GetError());
        return Error { eError::TextureCreate };
    }

    if (!SDL_UpdateTexture(pTexture, nullptr, _rgba.data(), _width * 4))
    {
        SLUG_LOG_ERROR("SDL_UpdateTexture 실패: {}", SDL_GetError());
        SDL_DestroyTexture(pTexture);
        return Error { eError::TextureCreate };
    }

    SDL_SetTextureBlendMode(pTexture, SDL_BLENDMODE_BLEND);
    SDL_SetTextureScaleMode(pTexture, SDL_SCALEMODE_NEAREST);

    // 피격 플래시 변환본 (실패해도 일반 페이지는 쓸 수 있다: 그 경우 플래시만 생략된다)
    SDL_Texture* pFlash = SDL_CreateTexture(m_pRenderer, SDL_PIXELFORMAT_RGBA32, SDL_TEXTUREACCESS_STATIC, _width, _height);
    if (pFlash && _flashRgba.size() >= static_cast<size_t>(_width) * static_cast<size_t>(_height) * 4 && SDL_UpdateTexture(pFlash, nullptr, _flashRgba.data(), _width * 4))
    {
        SDL_SetTextureBlendMode(pFlash, SDL_BLENDMODE_BLEND);
        SDL_SetTextureScaleMode(pFlash, SDL_SCALEMODE_NEAREST);
    }
    else
    {
        if (pFlash)
        {
            SDL_DestroyTexture(pFlash);
        }
        pFlash = nullptr;
        SLUG_LOG_ERROR("플래시 변환 텍스처 생성 실패: {}", SDL_GetError());
    }

    m_pages.push_back({ pTexture, _width, _height, pFlash });
    return Error { kOK };
}

Result<void> SpriteBatch::AddAtlasPage(
    const DecodedImage& _image)
{
    if (_image.flashRgba.empty())
    {
        return AddAtlasPageFromRgba(_image.width, _image.height, _image.rgba);
    }
    return AddAtlasPageFromRgba(_image.width, _image.height, _image.rgba, _image.flashRgba);
}

Result<int> SpriteBatch::AddStandaloneTexture(
    const DecodedImage& _image)
{
    return AddStandaloneTextureFromRgba(_image.width, _image.height, _image.rgba);
}

bool SpriteBatch::LoadAtlasPagesParallel(
    const AnimationLibrary& _library,
    const int               _numWorkers)
{
    const ScopedPhaseTimer timer { "atlas-pages", "all" };
    Vector<FilePath>       paths;
    for (const String& page: _library.GetAtlasPages())
    {
        paths.push_back(_library.GetRoot() / page);
    }

    const size_t chunk = static_cast<size_t>(std::max(1, _numWorkers));
    for (size_t begin = 0; begin < paths.size(); begin += chunk)
    {
        const size_t                       count  = std::min(chunk, paths.size() - begin);
        const Vector<Result<DecodedImage>> images = DecodeImages(Span<const FilePath> { paths.data() + begin, count }, true, _numWorkers);
        for (size_t i = 0; i < count; ++i)
        {
            const FilePath& path  = paths[begin + i];
            const bool      bAdded = images[i].HasValue() && AddAtlasPage(images[i].GetValue()).IsOK();
            if (!bAdded)
            {
                SLUG_LOG_ERROR("아틀라스 페이지 로드 실패: {}", path.filename().string());
                return false;
            }
            TraceAtlasPage_(m_pages.size() - 1, path, images[i]->width, images[i]->height, images[i]->rgba);
        }
    }
    return true;
}

Vector<int> SpriteBatch::LoadStandaloneTexturesParallel(
    const Span<const FilePath> _paths,
    const int                  _numWorkers)
{
    Vector<int>  handles(_paths.size(), -1);
    const size_t chunk = static_cast<size_t>(std::max(1, _numWorkers));
    for (size_t begin = 0; begin < _paths.size(); begin += chunk)
    {
        const size_t                       count  = std::min(chunk, _paths.size() - begin);
        const Vector<Result<DecodedImage>> images = DecodeImages(Span<const FilePath> { _paths.data() + begin, count }, false, _numWorkers);
        for (size_t i = 0; i < count; ++i)
        {
            if (!images[i])
            {
                continue;
            }
            if (const Result<int> added = AddStandaloneTexture(images[i].GetValue()))
            {
                handles[begin + i] = added.GetValue();
                TraceStandalone_(added.GetValue(), _paths[begin + i], images[i]->width, images[i]->height, images[i]->rgba);
            }
        }
    }
    return handles;
}

Result<int> SpriteBatch::AddStandaloneTextureFromRgba(
    const int                 _width,
    const int                 _height,
    const Span<const uint8_t> _rgba)
{
    if (!m_pRenderer || _width <= 0 || _height <= 0 || _rgba.size() < static_cast<size_t>(_width) * static_cast<size_t>(_height) * 4)
    {
        return Error { eError::InvalidData };
    }

    const ScopedPhaseTimer timer { "gpu-upload", "standalone" };

    // 가로가 kMaxStripW 를 넘으면 띠로 쪼갠다 (구형 GPU/드라이버의 8192 제한에서 최후반부가 깨지는 문제 방지)
    constexpr int     kMaxStripW = 2048;
    const int         stripW     = std::min(_width, kMaxStripW);
    StandaloneTexture texture    = {};
    texture.width                = _width;
    texture.height               = _height;
    texture.stripW               = stripW;

    Vector<uint8_t> strip;
    for (int x = 0; x < _width; x += stripW)
    {
        const int w = std::min(stripW, _width - x);
        strip.resize(static_cast<size_t>(w) * static_cast<size_t>(_height) * 4);
        for (int y = 0; y < _height; ++y)
        {
            std::memcpy(strip.data() + static_cast<size_t>(y) * static_cast<size_t>(w) * 4, _rgba.data() + (static_cast<size_t>(y) * static_cast<size_t>(_width) + static_cast<size_t>(x)) * 4, static_cast<size_t>(w) * 4);
        }

        SDL_Texture* pTexture = SDL_CreateTexture(m_pRenderer, SDL_PIXELFORMAT_RGBA32, SDL_TEXTUREACCESS_STATIC, w, _height);
        if (!pTexture || !SDL_UpdateTexture(pTexture, nullptr, strip.data(), w * 4))
        {
            SLUG_LOG_ERROR("단독 텍스처 띠 생성 실패 ({}x{} 중 x={}): {}", _width, _height, x, SDL_GetError());
            if (pTexture)
            {
                SDL_DestroyTexture(pTexture);
            }
            for (SDL_Texture* pDone: texture.strips)
            {
                SDL_DestroyTexture(pDone);
            }
            return Error { eError::TextureCreate };
        }
        SDL_SetTextureBlendMode(pTexture, SDL_BLENDMODE_BLEND);
        SDL_SetTextureScaleMode(pTexture, SDL_SCALEMODE_NEAREST);
        texture.strips.push_back(pTexture);
    }
    m_textures.push_back(std::move(texture));
    return static_cast<int>(m_textures.size()) - 1;
}

void SpriteBatch::DrawStandaloneRect_(
    const StandaloneTexture& _texture,
    const SDL_FRect&         _src,
    const SDL_FRect&         _dst,
    const RGBA               _tint) const
{
    if (_src.w <= 0.f || _src.h <= 0.f)
    {
        return;
    }
    const float scaleX = _dst.w / _src.w;
    const int   first  = std::max(0, static_cast<int>(_src.x) / _texture.stripW);
    const int   last   = std::min(static_cast<int>(_texture.strips.size()) - 1, static_cast<int>(_src.x + _src.w - 0.001f) / _texture.stripW);
    for (int k = first; k <= last; ++k)
    {
        const float stripL = static_cast<float>(k * _texture.stripW);
        const float l      = std::max(_src.x, stripL);
        const float r      = std::min(_src.x + _src.w, stripL + static_cast<float>(_texture.stripW));
        if (r <= l)
        {
            continue;
        }
        const SDL_FRect src    = { l - stripL, _src.y, r - l, _src.h };
        const SDL_FRect dst    = { _dst.x + (l - _src.x) * scaleX, _dst.y, (r - l) * scaleX, _dst.h };
        SDL_Texture*    pStrip = _texture.strips[static_cast<size_t>(k)];
        SDL_SetTextureColorMod(pStrip, _tint.r, _tint.g, _tint.b);
        SDL_SetTextureAlphaMod(pStrip, _tint.a);
        SDL_RenderTexture(m_pRenderer, pStrip, &src, &dst);
    }
}

Result<int> SpriteBatch::LoadStandaloneTexture(
    const FilePath& _pngPath)
{
    Result<DecodedImage> image = DecodePngFile_(_pngPath);
    if (!image)
    {
        return image.GetError();
    }
    const Result<int> result = AddStandaloneTexture(image.GetValue());
    if (result)
    {
        TraceStandalone_(result.GetValue(), _pngPath, image->width, image->height, image->rgba);
    }
    return result;
}

void SpriteBatch::DrawTexture(
    const int  _handle,
    const Vec2 _texWorldPos,
    const Vec2 _cameraPos,
    const int  _viewW,
    const int  _viewH,
    const RGBA _tint) const
{
    if (!m_pRenderer || _handle < 0 || static_cast<size_t>(_handle) >= m_textures.size())
    {
        return;
    }
    const StandaloneTexture& texture = m_textures[static_cast<size_t>(_handle)];

    const TextureViewRects view = ComputeTextureView(texture.width, texture.height, _texWorldPos, _cameraPos, _viewW, _viewH);
    if (!view.bVisible)
    {
        return;
    }

    const SDL_FRect src = { static_cast<float>(view.src.x), static_cast<float>(view.src.y), static_cast<float>(view.src.w), static_cast<float>(view.src.h) };
    const SDL_FRect dst = { static_cast<float>(view.dst.x), static_cast<float>(view.dst.y), static_cast<float>(view.dst.w), static_cast<float>(view.dst.h) };
    DrawStandaloneRect_(texture, src, dst, _tint);
}

void SpriteBatch::DrawTextureRepeatX(
    const int   _handle,
    const Vec2  _texWorldPos,
    const Vec2  _cameraPos,
    const int   _viewW,
    const int   _viewH,
    const float _fromX,
    const int   _tileX,
    const int   _tileW) const
{
    if (!m_pRenderer || _handle < 0 || static_cast<size_t>(_handle) >= m_textures.size() || _tileW <= 0)
    {
        return;
    }
    const StandaloneTexture& texture = m_textures[static_cast<size_t>(_handle)];

    const int texX = FloorToInt(_texWorldPos.x);
    const int texY = FloorToInt(_texWorldPos.y);
    const int camX = FloorToInt(_cameraPos.x);
    const int camY = FloorToInt(_cameraPos.y);
    const int from = FloorToInt(_fromX);

    // Y 는 전체 높이를 그대로 쓴다 (카메라 세로 범위로 자른다)
    const int y0 = std::max(texY, camY);
    const int y1 = std::min(texY + texture.height, camY + _viewH);
    if (y1 <= y0)
    {
        return;
    }

    // 엔드리스 구간: 세그먼트 k 는 월드 [from + k*tileW, from + (k+1)*tileW) 를 텍스처 열 [_tileX, _tileX+tileW) 로 그린다
    const int viewLeft  = camX;
    const int viewRight = camX + _viewW;
    const int firstK    = viewLeft > from ? (viewLeft - from) / _tileW : 0;
    for (int k = firstK;; ++k)
    {
        const int segStart = from + k * _tileW;
        if (segStart >= viewRight)
        {
            break;
        }
        const int x0 = std::max(segStart, viewLeft);
        const int x1 = std::min(segStart + _tileW, viewRight);
        if (x1 <= x0)
        {
            continue;
        }
        const SDL_FRect src = { static_cast<float>(_tileX + (x0 - segStart)), static_cast<float>(y0 - texY), static_cast<float>(x1 - x0), static_cast<float>(y1 - y0) };
        const SDL_FRect dst = { static_cast<float>(x0 - camX), static_cast<float>(y0 - camY), static_cast<float>(x1 - x0), static_cast<float>(y1 - y0) };
        DrawStandaloneRect_(texture, src, dst, { 255, 255, 255, 255 });
    }
    (void)texX;
}

void SpriteBatch::DrawFrame(
    const AnimFrame&         _frame,
    const Vec2               _worldOrigin,
    const Vec2               _cameraPos,
    const SpriteDrawOptions& _options) const
{
    if (!m_pRenderer || _frame.IsEmpty() || _frame.atlas < 0 || static_cast<size_t>(_frame.atlas) >= m_pages.size())
    {
        return;
    }

    const Page& page = m_pages[static_cast<size_t>(_frame.atlas)];
    if (!page.pTexture)
    {
        return;
    }

    const IntRect dest = ComputeDestRect(_frame, _worldOrigin, _cameraPos, _options.bFlipX, _options.bFlipY);

    const SDL_FRect src = { static_cast<float>(_frame.x), static_cast<float>(_frame.y), static_cast<float>(_frame.w), static_cast<float>(_frame.h) };
    SDL_FRect       dst = { static_cast<float>(dest.x), static_cast<float>(dest.y), static_cast<float>(dest.w), static_cast<float>(dest.h) };

    // 피벗 점(프레임 기준점, 플립 반영)의 화면 위치는 배율과 무관하게 고정한다
    const float cx      = _options.bFlipX ? static_cast<float>(_frame.w - _frame.pivotX) : static_cast<float>(_frame.pivotX);
    const float cy      = _options.bFlipY ? static_cast<float>(_frame.h - _frame.pivotY) : static_cast<float>(_frame.pivotY);
    const bool  bScaled = _options.scale > 0.f && _options.scale != 1.f;
    if (bScaled)
    {
        dst.x += cx * (1.f - _options.scale);
        dst.y += cy * (1.f - _options.scale);
        dst.w *= _options.scale;
        dst.h *= _options.scale;
    }

    SDL_Texture* const pDraw = PickTexture_(page.pTexture, page.pFlashTexture, _options.flashAmount);
    SDL_SetTextureColorMod(pDraw, _options.tint.r, _options.tint.g, _options.tint.b);
    SDL_SetTextureAlphaMod(pDraw, _options.tint.a);
    if (_options.rotationDeg != 0.f)
    {
        // 피벗 점(프레임 기준점)이 월드 원점에 놓인 채 돈다. dst 좌표계에서 피벗 위치를 중심으로 준다 (배율 적용된 dst 기준)
        const float      s      = bScaled ? _options.scale : 1.f;
        const SDL_FPoint center = { cx * s, cy * s };
        SDL_RenderTextureRotated(m_pRenderer, pDraw, &src, &dst, static_cast<double>(_options.rotationDeg), &center, FlipFlags_(_options));
        return;
    }
    SDL_RenderTextureRotated(m_pRenderer, pDraw, &src, &dst, 0.0, nullptr, FlipFlags_(_options));
}

void SpriteBatch::DrawFrame(
    const AnimFrame&         _frame,
    const Vec2               _worldOrigin,
    const Camera&            _camera,
    const SpriteDrawOptions& _options) const
{
    DrawFrame(_frame, _worldOrigin, _camera.GetPosition(), _options);
}

void SpriteBatch::DrawFrameRect(
    const AnimFrame&         _frame,
    const IntRect&           _dest,
    const SpriteDrawOptions& _options) const
{
    if (!m_pRenderer || _frame.IsEmpty() || _dest.w <= 0 || _dest.h <= 0 || _frame.atlas < 0 || static_cast<size_t>(_frame.atlas) >= m_pages.size())
    {
        return;
    }

    const Page& page = m_pages[static_cast<size_t>(_frame.atlas)];
    if (!page.pTexture)
    {
        return;
    }

    const SDL_FRect src = { static_cast<float>(_frame.x), static_cast<float>(_frame.y), static_cast<float>(_frame.w), static_cast<float>(_frame.h) };
    const SDL_FRect dst = { static_cast<float>(_dest.x), static_cast<float>(_dest.y), static_cast<float>(_dest.w), static_cast<float>(_dest.h) };

    SDL_Texture* const pDraw = PickTexture_(page.pTexture, page.pFlashTexture, _options.flashAmount);
    SDL_SetTextureColorMod(pDraw, _options.tint.r, _options.tint.g, _options.tint.b);
    SDL_SetTextureAlphaMod(pDraw, _options.tint.a);
    SDL_RenderTextureRotated(m_pRenderer, pDraw, &src, &dst, 0.0, nullptr, FlipFlags_(_options));
}

void SpriteBatch::Submit(
    const int                _depth,
    const int                _sortKey,
    const AnimFrame&         _frame,
    const Vec2               _worldOrigin,
    const SpriteDrawOptions& _options)
{
    SpriteCommand command;
    command.depth   = _depth;
    command.sortKey = _sortKey;
    command.seq     = m_nextSeq++;
    command.frame   = _frame;
    command.origin  = _worldOrigin;
    command.options = _options;
    m_commands.push_back(command);
}

void SpriteBatch::SetAssetRoot(
    const FilePath& _root)
{
    m_assetRoot = _root;
}

int SpriteBatch::FindStandalone(
    const String& _relativeFile)
{
    if (const HashMap<String, int>::const_iterator it = m_namedStandalones.find(_relativeFile); it != m_namedStandalones.end())
    {
        return it->second;
    }
    int handle = -1;
    if (m_pRenderer && !m_assetRoot.empty())
    {
        if (const Result<int> loaded = LoadStandaloneTexture(m_assetRoot / FilePath { _relativeFile }))
        {
            handle = loaded.GetValue();
        }
    }
    m_namedStandalones.emplace(_relativeFile, handle);
    return handle;
}

void SpriteBatch::SubmitStandalone(
    const int                _depth,
    const int                _sortKey,
    const int                _handle,
    const IntRect&           _src,
    const Vec2               _worldCenter,
    const SpriteDrawOptions& _options)
{
    SpriteCommand command;
    command.depth         = _depth;
    command.sortKey       = _sortKey;
    command.seq           = m_nextSeq++;
    command.origin        = _worldCenter;
    command.options       = _options;
    command.standalone    = _handle;
    command.standaloneSrc = _src;
    m_commands.push_back(command);
}

void SpriteBatch::DrawCommand_(
    const SpriteCommand& _command,
    const Vec2           _cameraPos) const
{
    if (_command.standalone < 0)
    {
        DrawFrame(_command.frame, _command.origin, _cameraPos, _command.options);
        return;
    }
    if (!m_pRenderer || static_cast<size_t>(_command.standalone) >= m_textures.size())
    {
        return;
    }
    const IntRect&  r   = _command.standaloneSrc;
    const SDL_FRect src = { static_cast<float>(r.x), static_cast<float>(r.y), static_cast<float>(r.w), static_cast<float>(r.h) };
    const float     w   = static_cast<float>(r.w) * _command.options.scale;
    const float     h   = static_cast<float>(r.h) * _command.options.scale;
    const SDL_FRect dst = { static_cast<float>(FloorToInt(_command.origin.x - _cameraPos.x)) - w * 0.5f, static_cast<float>(FloorToInt(_command.origin.y - _cameraPos.y)) - h * 0.5f, w, h };
    DrawStandaloneRect_(m_textures[static_cast<size_t>(_command.standalone)], src, dst, _command.options.tint);
}

void SpriteBatch::Flush(
    const Vec2 _cameraPos)
{
    SortSpriteCommands(m_commands);
    for (const SpriteCommand& command: m_commands)
    {
        DrawCommand_(command, _cameraPos);
    }
    m_commands.clear();
    m_nextSeq = 0;
}

void SpriteBatch::FlushBelowDepth(
    const Vec2 _cameraPos,
    const int  _depth)
{
    SortSpriteCommands(m_commands);
    size_t count = 0;
    while (count < m_commands.size() && m_commands[count].depth < _depth)
    {
        DrawCommand_(m_commands[count], _cameraPos);
        ++count;
    }
    m_commands.erase(m_commands.begin(), m_commands.begin() + static_cast<std::ptrdiff_t>(count));
}

void SpriteBatch::Flush(
    const Camera& _camera)
{
    Flush(_camera.GetPosition());
}

size_t SpriteBatch::GetAtlasPageCount() const
{
    return m_pages.size();
}

void SpriteBatch::Clear()
{
    m_commands.clear();
}

const Vector<SpriteCommand>& SpriteBatch::GetPendingCommands() const
{
    return m_commands;
}
