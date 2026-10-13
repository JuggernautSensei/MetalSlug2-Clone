#include "World/Geometry.h"

Aabb Aabb::FromMinMax(
    const VECTOR2 _min,
    const VECTOR2 _max)
{
    return Aabb { _min, _max };
}

Aabb Aabb::FromMinSize(
    const VECTOR2 _min,
    const VECTOR2 _size)
{
    return Aabb {
        _min, { _min.x + _size.x, _min.y + _size.y }
    };
}

Aabb Aabb::FromCenterSize(
    const VECTOR2 _center,
    const VECTOR2 _size)
{
    const VECTOR2 half = { _size.x * 0.5f, _size.y * 0.5f };
    return Aabb {
        { _center.x - half.x, _center.y - half.y },
        { _center.x + half.x, _center.y + half.y }
    };
}

Aabb Aabb::FromBottomCenter(
    const VECTOR2 _bottomCenter,
    const VECTOR2 _size)
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

VECTOR2 Aabb::GetSize() const
{
    return { max.x - min.x, max.y - min.y };
}

VECTOR2 Aabb::GetCenter() const
{
    return { (min.x + max.x) * 0.5f, (min.y + max.y) * 0.5f };
}

bool Aabb::IsEmpty() const
{
    return max.x <= min.x || max.y <= min.y;
}

bool Aabb::operator==(
    const Aabb& _other) const
{
    return min == _other.min && max == _other.max;
}
