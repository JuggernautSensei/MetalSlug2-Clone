#pragma once
#include <algorithm>
#include <cmath>
#include <cstdint>
#include <cstring>
#include <vector>

namespace slug::baker
{
struct IRect
{
    int x = 0;
    int y = 0;
    int w = 0;
    int h = 0;

    [[nodiscard]] bool IsEmpty() const { return w <= 0 || h <= 0; }
    [[nodiscard]] bool operator==(const IRect&) const = default;
};

struct Image
{
    int                  width  = 0;
    int                  height = 0;
    std::vector<uint8_t> pixels;
    bool                 bSourceHasAlpha = false;

    [[nodiscard]] static Image Make(
        const int _width,
        const int _height)
    {
        Image img;
        img.width  = _width;
        img.height = _height;
        img.pixels.assign(static_cast<size_t>(_width) * static_cast<size_t>(_height) * 4u, 0u);
        return img;
    }

    [[nodiscard]] bool IsEmpty() const { return width <= 0 || height <= 0; }

    [[nodiscard]] const uint8_t* At(
        const int _x,
        const int _y) const
    {
        return pixels.data() + (static_cast<size_t>(_y) * static_cast<size_t>(width) + static_cast<size_t>(_x)) * 4u;
    }

    [[nodiscard]] uint8_t* At(
        const int _x,
        const int _y)
    {
        return pixels.data() + (static_cast<size_t>(_y) * static_cast<size_t>(width) + static_cast<size_t>(_x)) * 4u;
    }
};

[[nodiscard]] inline IRect FindOpaqueBounds(
    const Image& _image,
    const IRect& _region,
    const int    _alphaThreshold = 0)
{
    const int x0 = std::max(_region.x, 0);
    const int y0 = std::max(_region.y, 0);
    const int x1 = std::min(_region.x + _region.w, _image.width);
    const int y1 = std::min(_region.y + _region.h, _image.height);
    if (x0 >= x1 || y0 >= y1)
    {
        return {};
    }

    int minX = x1;
    int minY = y1;
    int maxX = -1;
    int maxY = -1;
    for (int y = y0; y < y1; ++y)
    {
        const uint8_t* pRow = _image.At(x0, y);
        for (int x = x0; x < x1; ++x, pRow += 4)
        {
            if (pRow[3] > _alphaThreshold)
            {
                minX = std::min(minX, x);
                maxX = std::max(maxX, x);
                minY = std::min(minY, y);
                maxY = std::max(maxY, y);
            }
        }
    }

    if (maxX < 0)
    {
        return {};
    }
    return { minX, minY, maxX - minX + 1, maxY - minY + 1 };
}

[[nodiscard]] inline Image CropImage(
    const Image& _src,
    const IRect& _rect)
{
    if (_rect.IsEmpty())
    {
        return {};
    }

    Image out = Image::Make(_rect.w, _rect.h);
    for (int y = 0; y < _rect.h; ++y)
    {
        const int sy = _rect.y + y;
        if (sy < 0 || sy >= _src.height)
        {
            continue;
        }

        const int sx0 = std::max(_rect.x, 0);
        const int sx1 = std::min(_rect.x + _rect.w, _src.width);
        if (sx0 >= sx1)
        {
            continue;
        }

        std::memcpy(out.At(sx0 - _rect.x, y), _src.At(sx0, sy), static_cast<size_t>(sx1 - sx0) * 4u);
    }
    return out;
}

inline void CopyInto(
    Image&       _dst,
    const Image& _src,
    const int    _dx,
    const int    _dy)
{
    for (int y = 0; y < _src.height; ++y)
    {
        const int dy = _dy + y;
        if (dy < 0 || dy >= _dst.height)
        {
            continue;
        }

        const int x0 = std::max(_dx, 0);
        const int x1 = std::min(_dx + _src.width, _dst.width);
        if (x0 >= x1)
        {
            continue;
        }

        std::memcpy(_dst.At(x0, dy), _src.At(x0 - _dx, y), static_cast<size_t>(x1 - x0) * 4u);
    }
}

inline void ApplyColorKey(
    Image&        _image,
    const uint8_t _r,
    const uint8_t _g,
    const uint8_t _b)
{
    uint8_t* p = _image.pixels.data();
    for (size_t i = 0, n = _image.pixels.size(); i < n; i += 4)
    {
        if (p[i] == _r && p[i + 1] == _g && p[i + 2] == _b)
        {
            p[i] = p[i + 1] = p[i + 2] = p[i + 3] = 0;
        }
        else
        {
            p[i + 3] = 255;
        }
    }
}

inline void ApplyMagentaKeyKeepAlpha(Image& _image)
{
    uint8_t* p = _image.pixels.data();
    for (size_t i = 0, n = _image.pixels.size(); i < n; i += 4)
    {
        if (p[i] == 255 && p[i + 1] == 0 && p[i + 2] == 255 && p[i + 3] == 255)
        {
            p[i] = p[i + 1] = p[i + 2] = p[i + 3] = 0;
        }
    }
}

inline void ForceOpaque(Image& _image)
{
    uint8_t* p = _image.pixels.data();
    for (size_t i = 3, n = _image.pixels.size(); i < n; i += 4)
    {
        p[i] = 255;
    }
}

inline void ClearTransparentColor(Image& _image)
{
    uint8_t* p = _image.pixels.data();
    for (size_t i = 0, n = _image.pixels.size(); i < n; i += 4)
    {
        if (p[i + 3] == 0)
        {
            p[i] = p[i + 1] = p[i + 2] = 0;
        }
    }
}

[[nodiscard]] inline uint64_t CountOpaqueMagenta(const Image& _image)
{
    uint64_t       count = 0;
    const uint8_t* p     = _image.pixels.data();
    for (size_t i = 0, n = _image.pixels.size(); i < n; i += 4)
    {
        if (p[i] == 255 && p[i + 1] == 0 && p[i + 2] == 255 && p[i + 3] == 255)
        {
            ++count;
        }
    }
    return count;
}

// 불투명 픽셀이 전부 같은 RGB 이고 면적이 2500px 이상이며 어둡거나(최대 채널 160 이하) 채도가 높은(최대-최소 96 이상) 프레임은
// 레거시 폭발/화염의 단색 실루엣으로 보고 소프트 플래시로 바꾼다. 중심(불투명 영역 중심)에서 가장 먼 픽셀까지의 거리 기준으로
// 0~75% 는 밝은 노랑-흰색(255,248,200), 바깥 25% 에서 주황(255,170,60)으로 보간한다. 알파(모양)는 그대로 둔다. 바꾸면 true.
[[nodiscard]] inline bool ConvertSolidSilhouetteToFlash(Image& _image)
{
    constexpr uint64_t kMinArea    = 2500;
    constexpr int      kDarkMax    = 160;
    constexpr int      kSatRange   = 96;
    constexpr double   kFadeStart  = 0.75;
    constexpr double   kCore[3]    = { 255.0, 248.0, 200.0 };
    constexpr double   kOrange[3]  = { 255.0, 170.0, 60.0 };

    uint64_t area = 0;
    uint8_t  first[3] = { 0, 0, 0 };
    uint8_t* p        = _image.pixels.data();
    for (size_t i = 0, n = _image.pixels.size(); i < n; i += 4)
    {
        if (p[i + 3] == 0)
        {
            continue;
        }
        if (area == 0)
        {
            first[0] = p[i];
            first[1] = p[i + 1];
            first[2] = p[i + 2];
        }
        else if (p[i] != first[0] || p[i + 1] != first[1] || p[i + 2] != first[2])
        {
            return false;
        }
        ++area;
    }
    if (area < kMinArea)
    {
        return false;
    }

    const int maxC = std::max({ static_cast<int>(first[0]), static_cast<int>(first[1]), static_cast<int>(first[2]) });
    const int minC = std::min({ static_cast<int>(first[0]), static_cast<int>(first[1]), static_cast<int>(first[2]) });
    if (maxC > kDarkMax && maxC - minC < kSatRange)
    {
        return false;
    }

    const IRect bounds = FindOpaqueBounds(_image, { 0, 0, _image.width, _image.height });
    const double cx    = bounds.x + (bounds.w - 1) * 0.5;
    const double cy    = bounds.y + (bounds.h - 1) * 0.5;
    double       maxD  = 0.0;
    for (int y = 0; y < _image.height; ++y)
    {
        for (int x = 0; x < _image.width; ++x)
        {
            if (_image.At(x, y)[3] != 0)
            {
                const double dx = x - cx;
                const double dy = y - cy;
                maxD            = std::max(maxD, std::sqrt(dx * dx + dy * dy));
            }
        }
    }
    if (maxD <= 0.0)
    {
        return false;
    }

    for (int y = 0; y < _image.height; ++y)
    {
        for (int x = 0; x < _image.width; ++x)
        {
            uint8_t* px = _image.At(x, y);
            if (px[3] == 0)
            {
                continue;
            }
            const double dx = x - cx;
            const double dy = y - cy;
            const double d  = std::sqrt(dx * dx + dy * dy) / maxD;
            const double t  = (d <= kFadeStart) ? 0.0 : std::min(1.0, (d - kFadeStart) / (1.0 - kFadeStart));
            for (int c = 0; c < 3; ++c)
            {
                px[c] = static_cast<uint8_t>(std::lround(kCore[c] + (kOrange[c] - kCore[c]) * t));
            }
        }
    }
    return true;
}

// 불투명 픽셀의 90% 이상이 순수 검정(RGB 0)인 프레임(레거시 BMP 의 검은 실루엣)을 흰색 플래시로 바꾼다. 바꾸면 true.
[[nodiscard]] inline bool ConvertBlackSilhouetteToWhite(Image& _image)
{
    constexpr uint64_t kMinOpaque = 200;
    uint64_t           opaque     = 0;
    uint64_t           black      = 0;
    uint8_t*           p          = _image.pixels.data();
    for (size_t i = 0, n = _image.pixels.size(); i < n; i += 4)
    {
        if (p[i + 3] == 0)
        {
            continue;
        }
        ++opaque;
        if ((p[i] | p[i + 1] | p[i + 2]) == 0)
        {
            ++black;
        }
    }
    if (opaque < kMinOpaque || black * 10 < opaque * 9)
    {
        return false;
    }
    for (size_t i = 0, n = _image.pixels.size(); i < n; i += 4)
    {
        if (p[i + 3] != 0 && (p[i] | p[i + 1] | p[i + 2]) == 0)
        {
            p[i] = p[i + 1] = p[i + 2] = 255;
        }
    }
    return true;
}

[[nodiscard]] inline uint64_t HashImage(const Image& _image)
{
    uint64_t hash = 14695981039346656037ull;
    auto     mix  = [&hash](const uint64_t _v)
    {
        hash ^= _v;
        hash *= 1099511628211ull;
    };
    mix(static_cast<uint64_t>(_image.width));
    mix(static_cast<uint64_t>(_image.height));
    for (const uint8_t byte: _image.pixels)
    {
        mix(byte);
    }
    return hash;
}

[[nodiscard]] inline bool SameImage(
    const Image& _a,
    const Image& _b)
{
    return _a.width == _b.width && _a.height == _b.height && _a.pixels == _b.pixels;
}

inline void FillRect(
    Image&        _dst,
    const IRect&  _rect,
    const uint8_t _r,
    const uint8_t _g,
    const uint8_t _b,
    const uint8_t _a = 255)
{
    const int x0 = std::max(_rect.x, 0);
    const int y0 = std::max(_rect.y, 0);
    const int x1 = std::min(_rect.x + _rect.w, _dst.width);
    const int y1 = std::min(_rect.y + _rect.h, _dst.height);
    for (int y = y0; y < y1; ++y)
    {
        for (int x = x0; x < x1; ++x)
        {
            uint8_t* p = _dst.At(x, y);
            p[0]       = _r;
            p[1]       = _g;
            p[2]       = _b;
            p[3]       = _a;
        }
    }
}

inline void PutPixel(
    Image&        _dst,
    const int     _x,
    const int     _y,
    const uint8_t _r,
    const uint8_t _g,
    const uint8_t _b)
{
    if (_x < 0 || _y < 0 || _x >= _dst.width || _y >= _dst.height)
    {
        return;
    }
    uint8_t* p = _dst.At(_x, _y);
    p[0]       = _r;
    p[1]       = _g;
    p[2]       = _b;
    p[3]       = 255;
}

inline void BlendOver(
    Image&       _dst,
    const Image& _src,
    const int    _dx,
    const int    _dy)
{
    for (int y = 0; y < _src.height; ++y)
    {
        const int dy = _dy + y;
        if (dy < 0 || dy >= _dst.height)
        {
            continue;
        }
        for (int x = 0; x < _src.width; ++x)
        {
            const int dx = _dx + x;
            if (dx < 0 || dx >= _dst.width)
            {
                continue;
            }

            const uint8_t* s = _src.At(x, y);
            if (s[3] == 0)
            {
                continue;
            }

            uint8_t*  d = _dst.At(dx, dy);
            const int a = s[3];
            d[0]        = static_cast<uint8_t>((s[0] * a + d[0] * (255 - a) + 127) / 255);
            d[1]        = static_cast<uint8_t>((s[1] * a + d[1] * (255 - a) + 127) / 255);
            d[2]        = static_cast<uint8_t>((s[2] * a + d[2] * (255 - a) + 127) / 255);
            d[3]        = 255;
        }
    }
}
}
