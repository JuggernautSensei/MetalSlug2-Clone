#pragma once
#include <cstdint>
#include <optional>

#include "Base/Result.h"
#include "Base/Types.h"
#include "Base/Math.h"

namespace slug
{
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

    [[nodiscard]] static slug::Result<GroundMap> Load(const slug::FilePath& _path);
    [[nodiscard]] static slug::Result<GroundMap> LoadFromMemory(slug::Span<const uint8_t> _fileData);

    [[nodiscard]] static GroundMap MakeFromBits(int _width, int _height, slug::Span<const uint8_t> _packedBits);

    [[nodiscard]] static GroundMap MakeEmpty(int _width, int _height);

    [[nodiscard]] static GroundMap MakeFromRows(slug::Span<const slug::StringView> _rows, char _solid = '#');

    GroundMap() = default;

    void SetSolid(int _x, int _y, bool _bSolid);
    void FillRect(int _x0, int _y0, int _x1, int _y1, bool _bSolid);

    [[nodiscard]] int      GetWidth() const { return m_width; }
    [[nodiscard]] int      GetHeight() const { return m_height; }
    [[nodiscard]] bool     IsEmpty() const { return m_width == 0 || m_height == 0; }
    [[nodiscard]] uint64_t CountSolidPixels() const;

    void                                    SetBoundsPolicy(const GroundBoundsPolicy& _policy) { m_policy = _policy; }

    // 무한 꼬리: x >= m_tailFrom 은 [m_tailX, m_tailX + m_tailW) 구간을 주기적으로 반복한다 (보스전 엔드리스 지형).
    void SetPeriodicTail(int _from, int _tailX, int _tailW);
    [[nodiscard]] bool HasPeriodicTail() const { return m_tailW > 0; }

    [[nodiscard]] bool IsSolid(
        const int _x,
        const int _y) const
    {
        int x = _x;
        if (m_tailW > 0 && x >= m_tailFrom)
        {
            x = m_tailX + (x - m_tailFrom) % m_tailW;
        }
        if (static_cast<unsigned>(x) >= static_cast<unsigned>(m_width) || static_cast<unsigned>(_y) >= static_cast<unsigned>(m_height))
        {
            return IsOutOfBoundsSolid_(_x, _y);
        }
        return ((m_bits[static_cast<size_t>(_y) * m_stride + static_cast<size_t>(x >> 3)] >> (x & 7)) & kSolidMask) != 0;
    }

    [[nodiscard]] bool IsSolidAt(Vec2 _point) const;

    [[nodiscard]] bool IsRectSolid(int _x0, int _y0, int _x1, int _y1) const;

    [[nodiscard]] std::optional<int> FindGroundY(int _x, int _fromY, int _maxDist) const;

    [[nodiscard]] std::optional<int> FindCeilingY(int _x, int _fromY, int _maxDist) const;

    [[nodiscard]] std::optional<int> FindSurfaceUp(int _x, int _y, int _maxStep) const;

    [[nodiscard]] float EstimateSlope(int _x, int _y, int _halfWidth, int _searchRange) const;

    [[nodiscard]] Vec2 EstimateNormal(int _x, int _y, int _halfWidth, int _searchRange) const;

private:
    [[nodiscard]] bool IsOutOfBoundsSolid_(int _x, int _y) const;

    int                  m_width  = 0;
    int                  m_height = 0;
    size_t               m_stride = 0;   // 행당 바이트 수
    slug::Vector<uint8_t> m_bits   = {};
    GroundBoundsPolicy   m_policy = {};
    int                  m_tailFrom = 0;
    int                  m_tailX    = 0;
    int                  m_tailW    = 0;
};
}
