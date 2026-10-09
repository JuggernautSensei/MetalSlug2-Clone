#pragma once
#include <algorithm>
#include <cmath>
#include <cstdint>
#include <span>

namespace slug::baker
{

// ===========================================
//  레거시 애니메이션 셀/피벗 계산
// ===========================================

struct Pivot
{
    int x = 0;
    int y = 0;
};

// 레거시 ResourceManager::LoadAnimationFile 규칙:
// .anime 의 Width 가 텍스처 전체 너비와 같으면 스트립 전체 너비로 보고 (End + 1) 로 나눈다 (정수 나눗셈).
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

// 레거시 그리기: left = pos.x - (cellW / 2) + offsetX  (cellW 는 uint32, 정수 나눗셈)
//   -> 셀 좌상단 기준 엔티티 원점 = (cellW / 2 - offsetX, cellH / 2 - offsetY)
//   -> 트림 후 pivot = 원점 - (trimX, trimY)
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

// ===========================================
//  바닥 중앙 앵커 드리프트
// ===========================================

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

// 엔티티 원점 기준 트림 프레임의 바닥 중앙: (-pivotX + w / 2, -pivotY + h).
// 연속한 프레임 사이의 변화량 최대치를 구한다. 루프면 마지막 -> _loopStart 도 포함한다.
// 빈 프레임은 건너뛴다.
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

}   // namespace slug::baker
