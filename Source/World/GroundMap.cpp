#include "World/GroundMap.h"

#include <cmath>

#include "Base/SlugError.h"
#include "Base/Math.h"
#include "Base/Types.h"

namespace
{
[[nodiscard]] size_t StrideOf_(
    const int _width)
{
    return (static_cast<size_t>(_width) + 7) / 8;
}
}   // namespace

GroundMap GroundMap::MakeEmpty(
    const int _width,
    const int _height)
{
    JUG_ASSERT(_width >= 0 && _height >= 0, "맵 크기는 음수일 수 없다.\n");

    GroundMap map;
    map.m_width  = _width;
    map.m_height = _height;
    map.m_stride = StrideOf_(_width);
    map.m_bits.assign(map.m_stride * static_cast<size_t>(_height), 0);
    return map;
}

GroundMap GroundMap::MakeFromRows(
    const Span<const StringView> _rows,
    const char                   _solid)
{
    size_t width = 0;
    for (const StringView row: _rows)
    {
        width = Max(width, row.size());
    }

    GroundMap map = MakeEmpty(static_cast<int>(width), static_cast<int>(_rows.size()));
    for (size_t y = 0; y < _rows.size(); ++y)
    {
        for (size_t x = 0; x < _rows[y].size(); ++x)
        {
            if (_rows[y][x] == _solid)
            {
                map.SetSolid(static_cast<int>(x), static_cast<int>(y), true);
            }
        }
    }
    return map;
}

void GroundMap::SetPeriodicTail(
    const int _from,
    const int _tailX,
    const int _tailW)
{
    JUG_ASSERT(_tailW > 0 && _tailX >= 0 && _tailX + _tailW <= m_width, "주기 꼬리는 비트맵 안의 구간이어야 한다.\n");
    m_tailFrom = _from;
    m_tailX    = _tailX;
    m_tailW    = _tailW;
}

void GroundMap::MaterializeTail(
    const int _periods)
{
    if (m_tailW <= 0 || _periods <= 0)
    {
        return;
    }
    const int    newWidth  = m_tailFrom + m_tailW * _periods;
    const size_t newStride = StrideOf_(newWidth);
    if (newWidth <= m_width)
    {
        return;
    }
    Vector<uint8_t> bits(newStride * static_cast<size_t>(m_height), 0);
    for (int y = 0; y < m_height; ++y)
    {
        for (int x = 0; x < newWidth; ++x)
        {
            if (IsSolid(x, y))
            {
                bits[static_cast<size_t>(y) * newStride + static_cast<size_t>(x >> 3)] |= static_cast<uint8_t>(1u << (x & 7));
            }
        }
    }
    m_bits     = std::move(bits);
    m_width    = newWidth;
    m_stride   = newStride;
    m_tailFrom = newWidth;
}

void GroundMap::SetSolid(
    const int  _x,
    const int  _y,
    const bool _bSolid)
{
    if (static_cast<unsigned>(_x) >= static_cast<unsigned>(m_width) || static_cast<unsigned>(_y) >= static_cast<unsigned>(m_height))
    {
        return;
    }

    uint8_t&      byte = m_bits[static_cast<size_t>(_y) * m_stride + static_cast<size_t>(_x >> 3)];
    const uint8_t mask = static_cast<uint8_t>(1u << (_x & 7));
    byte               = _bSolid ? static_cast<uint8_t>(byte | mask) : static_cast<uint8_t>(byte & ~mask);
}

void GroundMap::FillRect(
    const int  _x0,
    const int  _y0,
    const int  _x1,
    const int  _y1,
    const bool _bSolid)
{
    const int x0 = Max(0, Min(_x0, _x1));
    const int x1 = Min(m_width - 1, Max(_x0, _x1));
    const int y0 = Max(0, Min(_y0, _y1));
    const int y1 = Min(m_height - 1, Max(_y0, _y1));
    for (int y = y0; y <= y1; ++y)
    {
        for (int x = x0; x <= x1; ++x)
        {
            SetSolid(x, y, _bSolid);
        }
    }
}

uint64_t GroundMap::CountSolidPixels() const
{
    uint64_t count = 0;
    for (int y = 0; y < m_height; ++y)
    {
        for (int x = 0; x < m_width; ++x)
        {
            count += IsSolid(x, y) ? 1u : 0u;
        }
    }
    return count;
}

bool GroundMap::IsOutOfBoundsSolid_(
    const int _x,
    const int _y) const
{
    return (_x < 0 && m_policy.bSolidLeft) || (_x >= m_width && m_policy.bSolidRight) || (_y < 0 && m_policy.bSolidTop) || (_y >= m_height && m_policy.bSolidBottom);
}

bool GroundMap::IsSolidAt(
    const Vec2 _point) const
{
    return IsSolid(static_cast<int>(std::floor(_point.x)), static_cast<int>(std::floor(_point.y)));
}

bool GroundMap::IsRectSolid(
    const int _x0,
    const int _y0,
    const int _x1,
    const int _y1) const
{
    const int x0 = Min(_x0, _x1);
    const int x1 = Max(_x0, _x1);
    const int y0 = Min(_y0, _y1);
    const int y1 = Max(_y0, _y1);

    if ((x0 < 0 && m_policy.bSolidLeft) || (x1 >= m_width && m_policy.bSolidRight) || (y0 < 0 && m_policy.bSolidTop) || (y1 >= m_height && m_policy.bSolidBottom))
    {
        return true;
    }

    const int cx0 = Max(x0, 0);
    const int cx1 = Min(x1, m_width - 1);
    const int cy0 = Max(y0, 0);
    const int cy1 = Min(y1, m_height - 1);
    if (cx0 > cx1 || cy0 > cy1)
    {
        return false;
    }

    const size_t  b0        = static_cast<size_t>(cx0 >> 3);
    const size_t  b1        = static_cast<size_t>(cx1 >> 3);
    const uint8_t firstMask = static_cast<uint8_t>(0xFFu << (cx0 & 7));
    const uint8_t lastMask  = static_cast<uint8_t>(0xFFu >> (7 - (cx1 & 7)));

    for (int y = cy0; y <= cy1; ++y)
    {
        const uint8_t* pRow = m_bits.data() + static_cast<size_t>(y) * m_stride;
        if (b0 == b1)
        {
            if ((pRow[b0] & firstMask & lastMask) != 0)
            {
                return true;
            }
            continue;
        }

        if ((pRow[b0] & firstMask) != 0 || (pRow[b1] & lastMask) != 0)
        {
            return true;
        }
        for (size_t b = b0 + 1; b < b1; ++b)
        {
            if (pRow[b] != 0)
            {
                return true;
            }
        }
    }
    return false;
}

std::optional<int> GroundMap::FindGroundY(
    const int _x,
    const int _fromY,
    const int _maxDist) const
{
    if (_maxDist < 0)
    {
        return std::nullopt;
    }

    const int endY = _fromY + _maxDist;
    for (int y = _fromY; y <= endY; ++y)
    {
        if (IsSolid(_x, y))
        {
            return y;
        }
    }
    return std::nullopt;
}

float GroundMap::EstimateSlope(
    const int _x,
    const int _y,
    const int _halfWidth,
    const int _searchRange) const
{
    if (_halfWidth <= 0)
    {
        return 0.f;
    }

    const std::optional<int> left  = FindGroundY(_x - _halfWidth, _y - _searchRange, _searchRange * 2);
    const std::optional<int> right = FindGroundY(_x + _halfWidth, _y - _searchRange, _searchRange * 2);
    if (!left || !right)
    {
        return 0.f;
    }
    return static_cast<float>(*right - *left) / static_cast<float>(_halfWidth * 2);
}

Vec2 GroundMap::EstimateNormal(
    const int _x,
    const int _y,
    const int _halfWidth,
    const int _searchRange) const
{
    const float slope  = EstimateSlope(_x, _y, _halfWidth, _searchRange);
    const float invLen = 1.f / Sqrt(slope * slope + 1.f);
    return { slope * invLen, -invLen };
}

int GroundMap::GetWidth() const
{
    return m_width;
}

int GroundMap::GetHeight() const
{
    return m_height;
}

bool GroundMap::IsEmpty() const
{
    return m_width == 0 || m_height == 0;
}

void GroundMap::SetBoundsPolicy(
    const GroundBoundsPolicy& _policy)
{
    m_policy = _policy;
}

bool GroundMap::HasPeriodicTail() const
{
    return m_tailW > 0;
}

bool GroundMap::IsSolid(
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
