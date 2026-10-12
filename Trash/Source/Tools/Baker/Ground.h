#pragma once
#include <cstdint>
#include <cstring>
#include <vector>

#include "Tools/Baker/Image.h"

namespace slug::baker
{

struct GroundThreshold
{
    int blueMin  = 192;   // B >= blueMin
    int redMax   = 96;    // R <= redMax
    int greenMax = 96;    // G <= greenMax
    int alphaMin = 128;   // A >= alphaMin
};

struct GroundStats
{
    uint64_t solidCount        = 0;
    uint64_t unclassifiedCount = 0;
    uint64_t totalCount        = 0;
    IRect    solidBounds       = {};
};

[[nodiscard]] inline bool IsSolidPixel(
    const uint8_t          _r,
    const uint8_t          _g,
    const uint8_t          _b,
    const uint8_t          _a,
    const GroundThreshold& _th)
{
    return _b >= _th.blueMin && _r <= _th.redMax && _g <= _th.greenMax && _a >= _th.alphaMin;
}

[[nodiscard]] inline bool IsAirPixel(
    const uint8_t          _r,
    const uint8_t          _g,
    const uint8_t          _b,
    const uint8_t          _a,
    const GroundThreshold& _th)
{
    return _a < _th.alphaMin || (_r >= 192 && _g >= 192 && _b >= 192);
}

[[nodiscard]] inline Vector<uint8_t> BuildGroundBits(
    const Image&           _image,
    const GroundThreshold& _th,
    GroundStats*           _pStats = nullptr)
{
    const int   rowBytes = (_image.width + 7) / 8;
    GroundStats stats;
    stats.totalCount = static_cast<uint64_t>(_image.width) * static_cast<uint64_t>(_image.height);

    Vector<uint8_t> bits(static_cast<size_t>(rowBytes) * static_cast<size_t>(_image.height), 0u);
    int             minX = _image.width;
    int             minY = _image.height;
    int             maxX = -1;
    int             maxY = -1;

    for (int y = 0; y < _image.height; ++y)
    {
        uint8_t*       pRowBits = bits.data() + static_cast<size_t>(y) * static_cast<size_t>(rowBytes);
        const uint8_t* p        = _image.At(0, y);
        for (int x = 0; x < _image.width; ++x, p += 4)
        {
            if (IsSolidPixel(p[0], p[1], p[2], p[3], _th))
            {
                pRowBits[x >> 3] = static_cast<uint8_t>(pRowBits[x >> 3] | (1u << (x & 7)));
                ++stats.solidCount;
                minX = std::min(minX, x);
                maxX = std::max(maxX, x);
                minY = std::min(minY, y);
                maxY = std::max(maxY, y);
            }
            else if (!IsAirPixel(p[0], p[1], p[2], p[3], _th))
            {
                ++stats.unclassifiedCount;
            }
        }
    }

    if (maxX >= 0)
    {
        stats.solidBounds = { minX, minY, maxX - minX + 1, maxY - minY + 1 };
    }
    if (_pStats != nullptr)
    {
        *_pStats = stats;
    }
    return bits;
}

[[nodiscard]] inline Vector<uint8_t> BuildGroundFile(
    const int              _width,
    const int              _height,
    const Vector<uint8_t>& _bits)
{
    Vector<uint8_t> file(12 + _bits.size());
    std::memcpy(file.data(), "GRND", 4);
    for (int i = 0; i < 4; ++i)
    {
        file[4 + static_cast<size_t>(i)] = static_cast<uint8_t>((static_cast<uint32_t>(_width) >> (8 * i)) & 0xFFu);
        file[8 + static_cast<size_t>(i)] = static_cast<uint8_t>((static_cast<uint32_t>(_height) >> (8 * i)) & 0xFFu);
    }
    if (!_bits.empty())
    {
        std::memcpy(file.data() + 12, _bits.data(), _bits.size());
    }
    return file;
}

}   // namespace slug::baker
