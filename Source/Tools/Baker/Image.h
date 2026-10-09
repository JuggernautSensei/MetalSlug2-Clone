#pragma once
#include <algorithm>
#include <cstdint>
#include <cstring>
#include <vector>

namespace slug::baker
{

// ===========================================
//  기본 도형
// ===========================================

struct IRect
{
    int x = 0;
    int y = 0;
    int w = 0;
    int h = 0;

    [[nodiscard]] bool IsEmpty() const { return w <= 0 || h <= 0; }
    [[nodiscard]] bool operator==(const IRect&) const = default;
};

// ===========================================
//  RGBA8 이미지 (straight alpha, 프리멀티플라이 하지 않음)
// ===========================================

struct Image
{
    int                  width  = 0;
    int                  height = 0;
    std::vector<uint8_t> pixels;                          // width * height * 4
    bool                 bSourceHasAlpha = false;         // 원본 BMP 가 알파 채널을 가졌는지 (리포트용)

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

// ===========================================
//  영역 처리
// ===========================================

// _region 중 이미지 밖은 투명으로 취급한다. 알파가 _alphaThreshold 보다 큰 픽셀의 타이트한 바운딩 박스를
// 이미지 좌표계로 반환한다. 불투명 픽셀이 없으면 w = h = 0.
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

// _rect 영역을 복사한다. 이미지 밖은 투명(0).
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

// _src 를 _dst 의 (_dx, _dy) 에 덮어쓴다 (알파 합성 없음, 범위 밖은 잘라냄).
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

// ===========================================
//  알파 처리
// ===========================================

// 컬러키 색(RGB 일치)을 완전 투명으로 만든다. 나머지는 알파 255 로 고정한다 (레거시 TransparentBlt 동작).
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

// 불투명한 순수 마젠타 (255, 0, 255, 255) 만 완전 투명으로 만든다. 다른 픽셀의 알파는 건드리지 않는다.
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

// 알파를 전부 255 로 만든다 (레거시 BitBlt 동작).
inline void ForceOpaque(Image& _image)
{
    uint8_t* p = _image.pixels.data();
    for (size_t i = 3, n = _image.pixels.size(); i < n; i += 4)
    {
        p[i] = 255;
    }
}

// 알파 0 픽셀의 RGB 를 0 으로 만든다 (선형 필터링 시 마젠타 등이 번지는 것을 방지, 중복 제거에도 유리).
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

// 불투명한 순수 마젠타 (255, 0, 255, 255) 픽셀 수 (컬러키 누수 검출용)
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

// ===========================================
//  해시 / 비교 (프레임 중복 제거용)
// ===========================================

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

// ===========================================
//  그리기 (컨택트 시트용)
// ===========================================

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

// straight alpha 소스를 불투명 대상 위에 합성한다.
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

}   // namespace slug::baker
