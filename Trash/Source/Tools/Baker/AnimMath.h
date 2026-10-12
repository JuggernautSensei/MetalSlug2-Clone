#pragma once
#include <algorithm>
#include <cmath>
#include <cstdint>

#include "Base/Types.h"

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
    int  w      = 0;
    int  h      = 0;
    int  pivotX = 0;
    int  pivotY = 0;
    bool bEmpty = false;
};

struct DriftResult
{
    double x   = 0.0;   // 프레임 간 앵커 X 변화 최대
    double y   = 0.0;   // 프레임 간 앵커 Y 변화 최대
    double max = 0.0;   // max(x, y)
};

namespace anim_math_detail
{
    // 프레임의 바닥 중앙 앵커 X (원점 기준)
    [[nodiscard]] inline double AnchorX_(
        const FrameBox& _f)
    {
        return -static_cast<double>(_f.pivotX) + static_cast<double>(_f.w) * 0.5;
    }

    // 프레임의 바닥 앵커 Y (원점 기준)
    [[nodiscard]] inline double AnchorY_(
        const FrameBox& _f)
    {
        return -static_cast<double>(_f.pivotY) + static_cast<double>(_f.h);
    }

    // 두 프레임 사이의 앵커 변화를 누적 최대로 기록한다.
    inline void Accumulate_(
        DriftResult&    _result,
        const FrameBox& _a,
        const FrameBox& _b)
    {
        _result.x = std::max(_result.x, std::abs(AnchorX_(_b) - AnchorX_(_a)));
        _result.y = std::max(_result.y, std::abs(AnchorY_(_b) - AnchorY_(_a)));
    }
}   // namespace anim_math_detail

[[nodiscard]] inline DriftResult ComputeAnchorDrift(
    const Span<const FrameBox> _frames,
    const bool                 _bLoop,
    const int                  _loopStart)
{
    DriftResult result;

    const FrameBox* pPrev = nullptr;
    for (const FrameBox& frame: _frames)
    {
        if (frame.bEmpty)
        {
            continue;
        }
        if (pPrev != nullptr)
        {
            anim_math_detail::Accumulate_(result, *pPrev, frame);
        }
        pPrev = &frame;
    }

    if (_bLoop && pPrev != nullptr && _loopStart >= 0 && _loopStart < static_cast<int>(_frames.size()))
    {
        const FrameBox& first = _frames[static_cast<size_t>(_loopStart)];
        if (!first.bEmpty && &first != pPrev)
        {
            anim_math_detail::Accumulate_(result, *pPrev, first);
        }
    }

    result.max = std::max(result.x, result.y);
    return result;
}

}   // namespace slug::baker
