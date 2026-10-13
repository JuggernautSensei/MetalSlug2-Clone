#pragma once
#include <cmath>

[[nodiscard]] inline int FloorToInt(float _value)
{
    return static_cast<int>(Floor(_value));
}

[[nodiscard]] inline int CeilToInt(float _value)
{
    return static_cast<int>(Ceil(_value));
}

struct Aabb
{
    VECTOR2 min = { 0.f, 0.f };
    VECTOR2 max = { 0.f, 0.f };

    [[nodiscard]] static Aabb FromMinMax(VECTOR2 _min, VECTOR2 _max);
    [[nodiscard]] static Aabb FromMinSize(VECTOR2 _min, VECTOR2 _size);
    [[nodiscard]] static Aabb FromCenterSize(VECTOR2 _center, VECTOR2 _size);
    [[nodiscard]] static Aabb FromBottomCenter(VECTOR2 _bottomCenter, VECTOR2 _size);
    [[nodiscard]] float       GetWidth() const;
    [[nodiscard]] float       GetHeight() const;
    [[nodiscard]] VECTOR2     GetSize() const;
    [[nodiscard]] VECTOR2     GetCenter() const;
    [[nodiscard]] bool        IsEmpty() const;

    [[nodiscard]] constexpr bool Overlaps(const Aabb& _other) const
    {
        return min.x < _other.max.x && max.x > _other.min.x && min.y < _other.max.y && max.y > _other.min.y;
    }

    [[nodiscard]] constexpr bool Contains(VECTOR2 _point) const
    {
        return _point.x >= min.x && _point.x < max.x && _point.y >= min.y && _point.y < max.y;
    }

    [[nodiscard]] constexpr bool Contains(const Aabb& _other) const
    {
        return _other.min.x >= min.x && _other.max.x <= max.x && _other.min.y >= min.y && _other.max.y <= max.y;
    }

    [[nodiscard]] bool operator==(const Aabb& _other) const;
};
