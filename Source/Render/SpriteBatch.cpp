#include "Render/SpriteBatch.h"

#include <algorithm>
#include <cmath>
#include <fstream>

#include <SDL3/SDL.h>

// 다른 번역 단위(Baker 등)가 stb_image 구현을 따로 포함해도 링크 충돌이 없도록 static 으로 만든다.
#define STB_IMAGE_STATIC
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#include <JugX/CoreLogger.h>

#include "Core/Error.h"

namespace slug
{

// ===========================================
//  순수 계산부
// ===========================================

IntRect ComputeDestRect(
    const AnimFrame&   _frame,
    const jug::VECTOR2 _worldOrigin,
    const jug::VECTOR2 _cameraPos,
    const bool         _bFlipX)
{
    // 원점을 화면 좌표로 옮긴 뒤 정수로 내림한다. (카메라가 정수이면 floor(origin) - camera 와 같다)
    const int originX = FloorToInt(_worldOrigin.x - _cameraPos.x);
    const int originY = FloorToInt(_worldOrigin.y - _cameraPos.y);

    IntRect rect;
    rect.w = _frame.w;
    rect.h = _frame.h;
    rect.y = originY - _frame.pivotY;
    // 반전: [ox - px, ox - px + w] 를 ox 기준으로 접으면 [ox + px - w, ox + px]
    rect.x = _bFlipX ? originX + _frame.pivotX - _frame.w : originX - _frame.pivotX;
    return rect;
}

void SortSpriteCommands(
    jug::Vector<SpriteCommand>& _commands)
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
    const int          _texW,
    const int          _texH,
    const jug::VECTOR2 _texWorldPos,
    const jug::VECTOR2 _cameraPos,
    const int          _viewW,
    const int          _viewH)
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

    // 월드 좌표에서 텍스처 사각형과 화면 사각형의 교집합
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
    // 파일을 읽어 RGBA8 로 디코드한다. 한글/유니코드 경로를 안전하게 열기 위해 파일을 직접 읽어 메모리 디코드.
    struct DecodedImage
    {
        int                  width  = 0;
        int                  height = 0;
        jug::Vector<uint8_t> rgba   = {};
    };

    [[nodiscard]] jug::Result<DecodedImage> DecodePngFile_(
        const jug::FilePath& _pngPath)
    {
        std::ifstream file { _pngPath, std::ios::binary | std::ios::ate };
        if (!file)
        {
            return jug::Error { eError::FileOpen };
        }

        const std::streamsize size = file.tellg();
        if (size <= 0)
        {
            return jug::Error { eError::FileRead };
        }

        jug::Vector<uint8_t> encoded(static_cast<size_t>(size));
        file.seekg(0, std::ios::beg);
        if (!file.read(reinterpret_cast<char*>(encoded.data()), size))
        {
            return jug::Error { eError::FileRead };
        }

        int      width    = 0;
        int      height   = 0;
        int      channels = 0;
        stbi_uc* pPixels  = stbi_load_from_memory(encoded.data(), static_cast<int>(encoded.size()), &width, &height, &channels, 4);
        if (!pPixels)
        {
            JUG_CORE_LOG_ERROR("PNG 디코드 실패: {}", stbi_failure_reason());
            return jug::Error { eError::InvalidData };
        }

        DecodedImage image;
        image.width  = width;
        image.height = height;
        image.rgba.assign(pPixels, pPixels + static_cast<size_t>(width) * static_cast<size_t>(height) * 4);
        stbi_image_free(pPixels);
        return image;
    }
}   // namespace

// ===========================================
//  SpriteBatch
// ===========================================

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

jug::Result<void> SpriteBatch::AddAtlasPageFromRgba(
    const int                      _width,
    const int                      _height,
    const jug::Span<const uint8_t> _rgba)
{
    if (!m_pRenderer || _width <= 0 || _height <= 0 || _rgba.size() < static_cast<size_t>(_width) * static_cast<size_t>(_height) * 4)
    {
        return jug::Error { eError::InvalidData };
    }

    SDL_Texture* pTexture = SDL_CreateTexture(m_pRenderer, SDL_PIXELFORMAT_RGBA32, SDL_TEXTUREACCESS_STATIC, _width, _height);
    if (!pTexture)
    {
        JUG_CORE_LOG_ERROR("SDL_CreateTexture 실패: {}", SDL_GetError());
        return jug::Error { eError::TextureCreate };
    }

    if (!SDL_UpdateTexture(pTexture, nullptr, _rgba.data(), _width * 4))
    {
        JUG_CORE_LOG_ERROR("SDL_UpdateTexture 실패: {}", SDL_GetError());
        SDL_DestroyTexture(pTexture);
        return jug::Error { eError::TextureCreate };
    }

    SDL_SetTextureBlendMode(pTexture, SDL_BLENDMODE_BLEND);
    SDL_SetTextureScaleMode(pTexture, SDL_SCALEMODE_NEAREST);

    m_pages.push_back({ pTexture, _width, _height });
    return jug::Error { jug::kOK };
}

jug::Result<void> SpriteBatch::LoadAtlasPage(
    const jug::FilePath& _pngPath)
{
    jug::Result<DecodedImage> image = DecodePngFile_(_pngPath);
    if (!image)
    {
        return image.GetError();
    }
    return AddAtlasPageFromRgba(image->width, image->height, image->rgba);
}

jug::Result<int> SpriteBatch::AddStandaloneTextureFromRgba(
    const int                      _width,
    const int                      _height,
    const jug::Span<const uint8_t> _rgba)
{
    if (!m_pRenderer || _width <= 0 || _height <= 0 || _rgba.size() < static_cast<size_t>(_width) * static_cast<size_t>(_height) * 4)
    {
        return jug::Error { eError::InvalidData };
    }

    SDL_Texture* pTexture = SDL_CreateTexture(m_pRenderer, SDL_PIXELFORMAT_RGBA32, SDL_TEXTUREACCESS_STATIC, _width, _height);
    if (!pTexture)
    {
        JUG_CORE_LOG_ERROR("SDL_CreateTexture 실패 ({}x{}): {}", _width, _height, SDL_GetError());
        return jug::Error { eError::TextureCreate };
    }
    if (!SDL_UpdateTexture(pTexture, nullptr, _rgba.data(), _width * 4))
    {
        JUG_CORE_LOG_ERROR("SDL_UpdateTexture 실패: {}", SDL_GetError());
        SDL_DestroyTexture(pTexture);
        return jug::Error { eError::TextureCreate };
    }

    SDL_SetTextureBlendMode(pTexture, SDL_BLENDMODE_BLEND);
    SDL_SetTextureScaleMode(pTexture, SDL_SCALEMODE_NEAREST);
    m_textures.push_back({ pTexture, _width, _height });
    return static_cast<int>(m_textures.size()) - 1;
}

jug::Result<int> SpriteBatch::LoadStandaloneTexture(
    const jug::FilePath& _pngPath)
{
    jug::Result<DecodedImage> image = DecodePngFile_(_pngPath);
    if (!image)
    {
        return image.GetError();
    }
    return AddStandaloneTextureFromRgba(image->width, image->height, image->rgba);
}

void SpriteBatch::DrawTexture(
    const int          _handle,
    const jug::VECTOR2 _texWorldPos,
    const jug::VECTOR2 _cameraPos,
    const int          _viewW,
    const int          _viewH,
    const jug::RGBA    _tint) const
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

jug::Result<void> SpriteBatch::LoadAtlasPages(
    const AnimationLibrary& _library)
{
    for (const jug::String& page: _library.GetAtlasPages())
    {
        const jug::Result<void> result = LoadAtlasPage(_library.GetRoot() / page);
        if (!result)
        {
            JUG_CORE_LOG_ERROR("아틀라스 페이지 로드 실패: {}", page);
            return result.GetError();
        }
    }
    return jug::Error { jug::kOK };
}

void SpriteBatch::DrawFrame(
    const AnimFrame&         _frame,
    const jug::VECTOR2       _worldOrigin,
    const jug::VECTOR2       _cameraPos,
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

    SDL_SetTextureColorMod(page.pTexture, _options.tint.r, _options.tint.g, _options.tint.b);
    SDL_SetTextureAlphaMod(page.pTexture, _options.tint.a);
    SDL_RenderTextureRotated(m_pRenderer, page.pTexture, &src, &dst, 0.0, nullptr, _options.bFlipX ? SDL_FLIP_HORIZONTAL : SDL_FLIP_NONE);
}

void SpriteBatch::DrawFrame(
    const AnimFrame&         _frame,
    const jug::VECTOR2       _worldOrigin,
    const Camera&            _camera,
    const SpriteDrawOptions& _options) const
{
    DrawFrame(_frame, _worldOrigin, _camera.GetPosition(), _options);
}

void SpriteBatch::Submit(
    const int                _layer,
    const int                _sortKey,
    const AnimFrame&         _frame,
    const jug::VECTOR2       _worldOrigin,
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
    const jug::VECTOR2 _cameraPos)
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
