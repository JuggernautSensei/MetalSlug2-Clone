#pragma once
#include <cmath>
#include <optional>

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
    Vec2 min = { 0.f, 0.f };
    Vec2 max = { 0.f, 0.f };

    [[nodiscard]] static Aabb FromMinMax(
        const Vec2 _min,
        const Vec2 _max);

    [[nodiscard]] static Aabb FromMinSize(
        const Vec2 _min,
        const Vec2 _size);

    [[nodiscard]] static Aabb FromCenterSize(
        const Vec2 _center,
        const Vec2 _size);

    [[nodiscard]] static Aabb FromBottomCenter(
        const Vec2 _bottomCenter,
        const Vec2 _size);

    [[nodiscard]] float GetWidth() const;

    [[nodiscard]] float GetHeight() const;

    [[nodiscard]] Vec2 GetSize() const;

    [[nodiscard]] Vec2 GetCenter() const;

    [[nodiscard]] Vec2 GetBottomCenter() const;

    [[nodiscard]] bool IsEmpty() const;

    [[nodiscard]] __forceinline constexpr bool Overlaps(
        const Aabb& _other) const
    {
        return min.x < _other.max.x && max.x > _other.min.x && min.y < _other.max.y && max.y > _other.min.y;
    }

    [[nodiscard]] __forceinline constexpr bool Contains(
        const Vec2 _point) const
    {
        return _point.x >= min.x && _point.x < max.x && _point.y >= min.y && _point.y < max.y;
    }

    [[nodiscard]] __forceinline constexpr bool Contains(
        const Aabb& _other) const
    {
        return _other.min.x >= min.x && _other.max.x <= max.x && _other.min.y >= min.y && _other.max.y <= max.y;
    }

    [[nodiscard]] std::optional<Aabb> Intersection(
        const Aabb& _other) const;

    [[nodiscard]] Aabb Expanded(
        const float _amount) const;

    [[nodiscard]] Aabb Expanded(
        const Vec2 _amount) const;

    [[nodiscard]] Aabb Translated(
        const Vec2 _offset) const;

    [[nodiscard]] Aabb Union(
        const Aabb& _other) const;

    [[nodiscard]] Aabb MirroredX(
        const float _axisX) const;

    [[nodiscard]] bool operator==(
        const Aabb& _other) const;
};
