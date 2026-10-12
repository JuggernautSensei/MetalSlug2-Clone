#include "World/Geometry.h"

std::optional<Aabb> Aabb::Intersection(
    const Aabb& _other) const
{
    if (!Overlaps(_other))
    {
        return std::nullopt;
    }
    return Aabb {
        { Max(min.x, _other.min.x), Max(min.y, _other.min.y) },
        { Min(max.x, _other.max.x), Min(max.y, _other.max.y) }
    };
}

Aabb Aabb::FromMinMax(
    const Vec2 _min,
    const Vec2 _max)
{
    return Aabb { _min, _max };
}

Aabb Aabb::FromMinSize(
    const Vec2 _min,
    const Vec2 _size)
{
    return Aabb {
        _min, { _min.x + _size.x, _min.y + _size.y }
    };
}

Aabb Aabb::FromCenterSize(
    const Vec2 _center,
    const Vec2 _size)
{
    const Vec2 half = { _size.x * 0.5f, _size.y * 0.5f };
    return Aabb {
        { _center.x - half.x, _center.y - half.y },
        { _center.x + half.x, _center.y + half.y }
    };
}

Aabb Aabb::FromBottomCenter(
    const Vec2 _bottomCenter,
    const Vec2 _size)
{
    return Aabb {
        { _bottomCenter.x - _size.x * 0.5f, _bottomCenter.y - _size.y },
        { _bottomCenter.x + _size.x * 0.5f,           _bottomCenter.y }
    };
}

float Aabb::GetWidth() const
{
    return max.x - min.x;
}

float Aabb::GetHeight() const
{
    return max.y - min.y;
}

Vec2 Aabb::GetSize() const
{
    return { max.x - min.x, max.y - min.y };
}

Vec2 Aabb::GetCenter() const
{
    return { (min.x + max.x) * 0.5f, (min.y + max.y) * 0.5f };
}

Vec2 Aabb::GetBottomCenter() const
{
    return { (min.x + max.x) * 0.5f, max.y };
}

bool Aabb::IsEmpty() const
{
    return max.x <= min.x || max.y <= min.y;
}

Aabb Aabb::Expanded(
    const float _amount) const
{
    return Aabb {
        { min.x - _amount, min.y - _amount },
        { max.x + _amount, max.y + _amount }
    };
}

Aabb Aabb::Expanded(
    const Vec2 _amount) const
{
    return Aabb {
        { min.x - _amount.x, min.y - _amount.y },
        { max.x + _amount.x, max.y + _amount.y }
    };
}

Aabb Aabb::Translated(
    const Vec2 _offset) const
{
    return Aabb {
        { min.x + _offset.x, min.y + _offset.y },
        { max.x + _offset.x, max.y + _offset.y }
    };
}

Aabb Aabb::Union(
    const Aabb& _other) const
{
    return Aabb {
        { Min(min.x, _other.min.x), Min(min.y, _other.min.y) },
        { Max(max.x, _other.max.x), Max(max.y, _other.max.y) }
    };
}

Aabb Aabb::MirroredX(
    const float _axisX) const
{
    return Aabb {
        { 2.f * _axisX - max.x, min.y },
        { 2.f * _axisX - min.x, max.y }
    };
}

bool Aabb::operator==(
    const Aabb& _other) const
{
    return min == _other.min && max == _other.max;
}
