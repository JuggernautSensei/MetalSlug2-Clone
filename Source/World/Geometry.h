#pragma once
#include <cmath>

#include "Base/Math.h"

[[nodiscard]] __forceinline int FloorToInt(
    const float _value)
{
    return static_cast<int>(std::floor(_value));
}

[[nodiscard]] __forceinline int CeilToInt(
    const float _value)
{
    return static_cast<int>(std::ceil(_value));
}

struct Aabb
{
    VECTOR2 min = { 0.f, 0.f };
    VECTOR2 max = { 0.f, 0.f };

    [[nodiscard]] static Aabb FromMinMax(const VECTOR2 _min, const VECTOR2 _max);

    [[nodiscard]] static Aabb FromMinSize(const VECTOR2 _min, const VECTOR2 _size);

    [[nodiscard]] static Aabb FromCenterSize(const VECTOR2 _center, const VECTOR2 _size);

    [[nodiscard]] static Aabb FromBottomCenter(const VECTOR2 _bottomCenter, const VECTOR2 _size);

    [[nodiscard]] float GetWidth() const;

    [[nodiscard]] float GetHeight() const;

    [[nodiscard]] VECTOR2 GetSize() const;

    [[nodiscard]] VECTOR2 GetCenter() const;

    [[nodiscard]] bool IsEmpty() const;

    [[nodiscard]] __forceinline constexpr bool Overlaps(
        const Aabb& _other) const
    {
        return min.x < _other.max.x && max.x > _other.min.x && min.y < _other.max.y && max.y > _other.min.y;
    }

    [[nodiscard]] __forceinline constexpr bool Contains(
        const VECTOR2 _point) const
    {
        return _point.x >= min.x && _point.x < max.x && _point.y >= min.y && _point.y < max.y;
    }

    [[nodiscard]] __forceinline constexpr bool Contains(
        const Aabb& _other) const
    {
        return _other.min.x >= min.x && _other.max.x <= max.x && _other.min.y >= min.y && _other.max.y <= max.y;
    }

    [[nodiscard]] bool operator==(const Aabb& _other) const;
};
