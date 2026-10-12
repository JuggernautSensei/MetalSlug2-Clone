#pragma once
#include <cstdint>
#include <optional>

#include "Base/Math.h"
#include "Base/SlugError.h"
#include "Base/Types.h"

struct GroundBoundsPolicy
{
    bool bSolidLeft   = false;
    bool bSolidRight  = false;
    bool bSolidTop    = false;
    bool bSolidBottom = true;
};

class GroundMap
{
public:
    static constexpr uint8_t kSolidMask = 1;

    [[nodiscard]] static GroundMap MakeEmpty(int _width, int _height);

    [[nodiscard]] static GroundMap MakeFromRows(Span<const StringView> _rows, char _solid = '#');

    GroundMap() = default;

    void SetSolid(int _x, int _y, bool _bSolid);
    void FillRect(int _x0, int _y0, int _x1, int _y1, bool _bSolid);

    [[nodiscard]] int      GetWidth() const;
    [[nodiscard]] int      GetHeight() const;
    [[nodiscard]] bool     IsEmpty() const;
    [[nodiscard]] uint64_t CountSolidPixels() const;

    void SetBoundsPolicy(const GroundBoundsPolicy& _policy);

    // 무한 꼬리: x >= m_tailFrom 은 [m_tailX, m_tailX + m_tailW) 구간을 주기적으로 반복한다 (보스전 엔드리스 지형).
    void               SetPeriodicTail(int _from, int _tailX, int _tailW);
    [[nodiscard]] bool HasPeriodicTail() const;
    void MaterializeTail(int _periods);

    [[nodiscard]] bool IsSolid(
        const int _x,
        const int _y) const;

    [[nodiscard]] bool IsSolidAt(Vec2 _point) const;

    [[nodiscard]] bool IsRectSolid(int _x0, int _y0, int _x1, int _y1) const;

    [[nodiscard]] std::optional<int> FindGroundY(int _x, int _fromY, int _maxDist) const;

    [[nodiscard]] float EstimateSlope(int _x, int _y, int _halfWidth, int _searchRange) const;

    [[nodiscard]] Vec2 EstimateNormal(int _x, int _y, int _halfWidth, int _searchRange) const;

private:
    [[nodiscard]] bool IsOutOfBoundsSolid_(int _x, int _y) const;

    int                m_width    = 0;
    int                m_height   = 0;
    size_t             m_stride   = 0;
    Vector<uint8_t>    m_bits     = {};
    GroundBoundsPolicy m_policy   = {};
    int                m_tailFrom = 0;
    int                m_tailX    = 0;
    int                m_tailW    = 0;
};
