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
    const bool       _bFlipX,
    const float      _scale)
{
    const int originX = FloorToInt(_worldOrigin.x - _cameraPos.x);
    const int originY = FloorToInt(_worldOrigin.y - _cameraPos.y);

    // 피벗(원점) 기준으로 확대한다 (1.0 이면 원본 크기)
    const int w      = _scale == 1.f ? _frame.w : static_cast<int>(std::lround(static_cast<float>(_frame.w) * _scale));
    const int h      = _scale == 1.f ? _frame.h : static_cast<int>(std::lround(static_cast<float>(_frame.h) * _scale));
    const int pivotX = _scale == 1.f ? _frame.pivotX : static_cast<int>(std::lround(static_cast<float>(_frame.pivotX) * _scale));
    const int pivotY = _scale == 1.f ? _frame.pivotY : static_cast<int>(std::lround(static_cast<float>(_frame.pivotY) * _scale));

    IntRect rect;
    rect.w = w;
    rect.h = h;
    rect.y = originY - pivotY;
    rect.x = _bFlipX ? originX + pivotX - w : originX - pivotX;
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

    // 같은 소스 사각형을 ADD 블렌드로 한 번 더 그린다. 알파가 0 인 픽셀은 기여하지 않는다.
    void DrawHitFlashPass_(
        SDL_Renderer*    _pRenderer,
        SDL_Texture*     _pTexture,
        const SDL_FRect& _src,
        const SDL_FRect& _dst,
        const bool       _bFlipX,
        const float      _flashAmount)
    {
        const float   amount = std::clamp(_flashAmount, 0.f, 1.f);
        const uint8_t alpha  = static_cast<uint8_t>(std::lround(amount * kHitFlashMaxAlpha * 255.f));
        if (alpha == 0)
        {
            return;
        }
        SDL_SetTextureBlendMode(_pTexture, SDL_BLENDMODE_ADD);
        SDL_SetTextureColorMod(_pTexture, kHitFlashR, kHitFlashG, kHitFlashB);
        SDL_SetTextureAlphaMod(_pTexture, alpha);
        SDL_RenderTextureRotated(_pRenderer, _pTexture, &_src, &_dst, 0.0, nullptr, _bFlipX ? SDL_FLIP_HORIZONTAL : SDL_FLIP_NONE);
        SDL_SetTextureBlendMode(_pTexture, SDL_BLENDMODE_BLEND);
    }

}   // namespace

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

    m_pages.push_back({ pTexture, _width, _height });
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

    const IntRect dest = ComputeDestRect(_frame, _worldOrigin, _cameraPos, _options.bFlipX, _options.scale);

    const SDL_FRect src = { static_cast<float>(_frame.x), static_cast<float>(_frame.y), static_cast<float>(_frame.w), static_cast<float>(_frame.h) };
    const SDL_FRect dst = { static_cast<float>(dest.x), static_cast<float>(dest.y), static_cast<float>(dest.w), static_cast<float>(dest.h) };

    SDL_SetTextureColorMod(page.pTexture, _options.tint.r, _options.tint.g, _options.tint.b);
    SDL_SetTextureAlphaMod(page.pTexture, _options.tint.a);
    SDL_RenderTextureRotated(m_pRenderer, page.pTexture, &src, &dst, 0.0, nullptr, _options.bFlipX ? SDL_FLIP_HORIZONTAL : SDL_FLIP_NONE);
    DrawHitFlashPass_(m_pRenderer, page.pTexture, src, dst, _options.bFlipX, _options.flashAmount);
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

    SDL_SetTextureColorMod(page.pTexture, _options.tint.r, _options.tint.g, _options.tint.b);
    SDL_SetTextureAlphaMod(page.pTexture, _options.tint.a);
    SDL_RenderTextureRotated(m_pRenderer, page.pTexture, &src, &dst, 0.0, nullptr, _options.bFlipX ? SDL_FLIP_HORIZONTAL : SDL_FLIP_NONE);
    DrawHitFlashPass_(m_pRenderer, page.pTexture, src, dst, _options.bFlipX, _options.flashAmount);
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
