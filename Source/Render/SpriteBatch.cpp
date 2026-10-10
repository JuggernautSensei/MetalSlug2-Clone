#include "Render/SpriteBatch.h"

#include <algorithm>
#include <cmath>
#include <fstream>
#include <SDL3/SDL.h>

#include "Base/Math.h"
#include "Base/RGBA.h"
#include "Base/Result.h"
#include "Base/Types.h"

#define STB_IMAGE_STATIC
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#include "Base/Log.h"

#include "Base/Error.h"

namespace slug
{

IntRect ComputeDestRect(
    const AnimFrame& _frame,
    const Vec2       _worldOrigin,
    const Vec2       _cameraPos,
    const bool       _bFlipX)
{
    const int originX = FloorToInt(_worldOrigin.x - _cameraPos.x);
    const int originY = FloorToInt(_worldOrigin.y - _cameraPos.y);

    IntRect rect;
    rect.w = _frame.w;
    rect.h = _frame.h;
    rect.y = originY - _frame.pivotY;
    rect.x = _bFlipX ? originX + _frame.pivotX - _frame.w : originX - _frame.pivotX;
    return rect;
}

void SortSpriteCommands(
    Vector<SpriteCommand>& _commands)
{
    std::sort(_commands.begin(), _commands.end(), [](const SpriteCommand& _a, const SpriteCommand& _b) {
        if (_a.layer != _b.layer)
        {
            return _a.layer < _b.layer;
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
    struct DecodedImage
    {
        int             width  = 0;
        int             height = 0;
        Vector<uint8_t> rgba   = {};
    };

    [[nodiscard]] Result<DecodedImage> DecodePngFile_(
        const FilePath& _pngPath)
    {
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

    // 피격 플래시 램프 노드 (원작 버스 피격 프레임 실측: 입력 평균 밝기 L -> 출력 RGB)
    struct FlashRampNode
    {
        int l = 0;
        int r = 0;
        int g = 0;
        int b = 0;
    };

    const ARRAY<FlashRampNode, 17> kFlashRamp = { {
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
    } };

    uint8_t LerpChannel_(
        const int _from,
        const int _to,
        const int _t,
        const int _span)
    {
        return static_cast<uint8_t>(_from + (_to - _from) * _t / _span);
    }

    // 플래시 변환본 RGBA 를 만든다 (알파 0 픽셀은 그대로 둔다).
    Vector<uint8_t> BuildFlashRgba_(
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
    for (Page& texture: m_textures)
    {
        if (texture.pTexture)
        {
            SDL_DestroyTexture(texture.pTexture);
        }
    }
    m_textures.clear();
    m_commands.clear();
    m_nextSeq   = 0;
    m_pRenderer = nullptr;
}

Result<void> SpriteBatch::AddAtlasPageFromRgba(
    const int                 _width,
    const int                 _height,
    const Span<const uint8_t> _rgba)
{
    if (!m_pRenderer || _width <= 0 || _height <= 0 || _rgba.size() < static_cast<size_t>(_width) * static_cast<size_t>(_height) * 4)
    {
        return Error { eError::InvalidData };
    }

    SDL_Texture* pTexture = SDL_CreateTexture(m_pRenderer, SDL_PIXELFORMAT_RGBA32, SDL_TEXTUREACCESS_STATIC, _width, _height);
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
    SDL_Texture*          pFlash = SDL_CreateTexture(m_pRenderer, SDL_PIXELFORMAT_RGBA32, SDL_TEXTUREACCESS_STATIC, _width, _height);
    const Vector<uint8_t> flashRgba = BuildFlashRgba_(_rgba, static_cast<size_t>(_width) * static_cast<size_t>(_height));
    if (pFlash && SDL_UpdateTexture(pFlash, nullptr, flashRgba.data(), _width * 4))
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

Result<void> SpriteBatch::LoadAtlasPage(
    const FilePath& _pngPath)
{
    Result<DecodedImage> image = DecodePngFile_(_pngPath);
    if (!image)
    {
        return image.GetError();
    }
    return AddAtlasPageFromRgba(image->width, image->height, image->rgba);
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

    SDL_Texture* pTexture = SDL_CreateTexture(m_pRenderer, SDL_PIXELFORMAT_RGBA32, SDL_TEXTUREACCESS_STATIC, _width, _height);
    if (!pTexture)
    {
        SLUG_LOG_ERROR("SDL_CreateTexture 실패 ({}x{}): {}", _width, _height, SDL_GetError());
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
    m_textures.push_back({ pTexture, _width, _height });
    return static_cast<int>(m_textures.size()) - 1;
}

Result<int> SpriteBatch::LoadStandaloneTexture(
    const FilePath& _pngPath)
{
    Result<DecodedImage> image = DecodePngFile_(_pngPath);
    if (!image)
    {
        return image.GetError();
    }
    return AddStandaloneTextureFromRgba(image->width, image->height, image->rgba);
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
    const Page& texture = m_textures[static_cast<size_t>(_handle)];
    if (!texture.pTexture)
    {
        return;
    }

    const TextureViewRects view = ComputeTextureView(texture.width, texture.height, _texWorldPos, _cameraPos, _viewW, _viewH);
    if (!view.bVisible)
    {
        return;
    }

    const SDL_FRect src = { static_cast<float>(view.src.x), static_cast<float>(view.src.y), static_cast<float>(view.src.w), static_cast<float>(view.src.h) };
    const SDL_FRect dst = { static_cast<float>(view.dst.x), static_cast<float>(view.dst.y), static_cast<float>(view.dst.w), static_cast<float>(view.dst.h) };
    SDL_SetTextureColorMod(texture.pTexture, _tint.r, _tint.g, _tint.b);
    SDL_SetTextureAlphaMod(texture.pTexture, _tint.a);
    SDL_RenderTexture(m_pRenderer, texture.pTexture, &src, &dst);
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
    const Page& texture = m_textures[static_cast<size_t>(_handle)];
    if (!texture.pTexture)
    {
        return;
    }

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
        SDL_SetTextureColorMod(texture.pTexture, 255, 255, 255);
        SDL_SetTextureAlphaMod(texture.pTexture, 255);
        SDL_RenderTexture(m_pRenderer, texture.pTexture, &src, &dst);
    }
    (void)texX;
}

Result<void> SpriteBatch::LoadAtlasPages(
    const AnimationLibrary& _library)
{
    for (const String& page: _library.GetAtlasPages())
    {
        const Result<void> result = LoadAtlasPage(_library.GetRoot() / page);
        if (!result)
        {
            SLUG_LOG_ERROR("아틀라스 페이지 로드 실패: {}", page);
            return result.GetError();
        }
    }
    return Error { kOK };
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

    const IntRect dest = ComputeDestRect(_frame, _worldOrigin, _cameraPos, _options.bFlipX);

    const SDL_FRect src = { static_cast<float>(_frame.x), static_cast<float>(_frame.y), static_cast<float>(_frame.w), static_cast<float>(_frame.h) };
    const SDL_FRect dst = { static_cast<float>(dest.x), static_cast<float>(dest.y), static_cast<float>(dest.w), static_cast<float>(dest.h) };

    SDL_Texture* const pDraw = PickTexture_(page.pTexture, page.pFlashTexture, _options.flashAmount);
    SDL_SetTextureColorMod(pDraw, _options.tint.r, _options.tint.g, _options.tint.b);
    SDL_SetTextureAlphaMod(pDraw, _options.tint.a);
    SDL_RenderTextureRotated(m_pRenderer, pDraw, &src, &dst, 0.0, nullptr, _options.bFlipX ? SDL_FLIP_HORIZONTAL : SDL_FLIP_NONE);
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
    SDL_RenderTextureRotated(m_pRenderer, pDraw, &src, &dst, 0.0, nullptr, _options.bFlipX ? SDL_FLIP_HORIZONTAL : SDL_FLIP_NONE);
}

void SpriteBatch::Submit(
    const int                _layer,
    const int                _sortKey,
    const AnimFrame&         _frame,
    const Vec2               _worldOrigin,
    const SpriteDrawOptions& _options)
{
    SpriteCommand command;
    command.layer   = _layer;
    command.sortKey = _sortKey;
    command.seq     = m_nextSeq++;
    command.frame   = _frame;
    command.origin  = _worldOrigin;
    command.options = _options;
    m_commands.push_back(command);
}

void SpriteBatch::Flush(
    const Vec2 _cameraPos)
{
    SortSpriteCommands(m_commands);
    for (const SpriteCommand& command: m_commands)
    {
        DrawFrame(command.frame, command.origin, _cameraPos, command.options);
    }
    m_commands.clear();
    m_nextSeq = 0;
}

void SpriteBatch::Flush(
    const Camera& _camera)
{
    Flush(_camera.GetPosition());
}

}   // namespace slug
