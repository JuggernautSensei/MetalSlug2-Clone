#pragma once
#include <algorithm>
#include <cmath>
#include <cstdint>
#include <span>

namespace slug::baker
{
struct Pivot
{
    int x = 0;
    int y = 0;
};

[[nodiscard]] inline int ResolveCellWidth(
    const int _textureWidth,
    const int _animWidth,
    const int _end)
{
    if (_textureWidth == _animWidth && _end >= 0)
    {
        return _animWidth / (_end + 1);
    }
    return _animWidth;
}

[[nodiscard]] inline Pivot ComputePivot(
    const uint32_t _cellW,
    const uint32_t _cellH,
    const int      _offsetX,
    const int      _offsetY,
    const int      _trimX,
    const int      _trimY)
{
    return {
        static_cast<int>(_cellW / 2u) - _offsetX - _trimX,
        static_cast<int>(_cellH / 2u) - _offsetY - _trimY
    };
}

struct FrameBox
{
    int  w       = 0;
    int  h       = 0;
    int  pivotX  = 0;
    int  pivotY  = 0;
    bool bEmpty  = false;
};

struct DriftResult
{
    double x   = 0.0;   // 프레임 간 앵커 X 변화 최대
    double y   = 0.0;   // 프레임 간 앵커 Y 변화 최대
    double max = 0.0;   // max(x, y)
};

[[nodiscard]] inline DriftResult ComputeAnchorDrift(
    const std::span<const FrameBox> _frames,
    const bool                      _bLoop,
    const int                       _loopStart)
{
    DriftResult result;

    auto anchorX = [](const FrameBox& _f) { return -static_cast<double>(_f.pivotX) + static_cast<double>(_f.w) * 0.5; };
    auto anchorY = [](const FrameBox& _f) { return -static_cast<double>(_f.pivotY) + static_cast<double>(_f.h); };
    auto accum   = [&](const FrameBox& _a, const FrameBox& _b)
    {
        result.x = std::max(result.x, std::abs(anchorX(_b) - anchorX(_a)));
        result.y = std::max(result.y, std::abs(anchorY(_b) - anchorY(_a)));
    };

    const FrameBox* pPrev = nullptr;
    for (const FrameBox& frame: _frames)
    {
        if (frame.bEmpty)
        {
            continue;
        }
        if (pPrev != nullptr)
        {
            accum(*pPrev, frame);
        }
        pPrev = &frame;
    }

    if (_bLoop && pPrev != nullptr && _loopStart >= 0 && _loopStart < static_cast<int>(_frames.size()))
    {
        const FrameBox& first = _frames[static_cast<size_t>(_loopStart)];
        if (!first.bEmpty && &first != pPrev)
        {
            accum(*pPrev, first);
        }
    }

    result.max = std::max(result.x, result.y);
    return result;
}
}
