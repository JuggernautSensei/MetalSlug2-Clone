#pragma once
#include <cmath>
#include <optional>

#include "Base/Math.h"

namespace slug
{

// ===========================================
//  정수 변환 헬퍼
// ===========================================

[[nodiscard]] inline int FloorToInt(
    const float _value)
{
    return static_cast<int>(std::floor(_value));
}

[[nodiscard]] inline int CeilToInt(
    const float _value)
{
    return static_cast<int>(std::ceil(_value));
}

// ===========================================
//  Aabb
//   2D 축 정렬 사각형. 월드 픽셀 좌표(+y 아래). min 포함, max 미포함 개념의 연속 좌표이다.
//   Overlaps 는 "면적이 겹칠 때만" true 이다 (변이 닿기만 하면 false).
// ===========================================

struct Aabb
{
    Vec2 min = { 0.f, 0.f };
    Vec2 max = { 0.f, 0.f };

    [[nodiscard]] static constexpr Aabb FromMinMax(
        const Vec2 _min,
        const Vec2 _max)
    {
        return Aabb { _min, _max };
    }

    [[nodiscard]] static constexpr Aabb FromMinSize(
        const Vec2 _min,
        const Vec2 _size)
    {
        return Aabb {
            _min, { _min.x + _size.x, _min.y + _size.y }
        };
    }

    [[nodiscard]] static constexpr Aabb FromCenterSize(
        const Vec2 _center,
        const Vec2 _size)
    {
        const Vec2 half = { _size.x * 0.5f, _size.y * 0.5f };
        return Aabb {
            { _center.x - half.x, _center.y - half.y },
            { _center.x + half.x, _center.y + half.y }
        };
    }

    // 발(하단 중앙) 기준. 캐릭터 원점 규약과 동일하다.
    [[nodiscard]] static constexpr Aabb FromBottomCenter(
        const Vec2 _bottomCenter,
        const Vec2 _size)
    {
        return Aabb {
            { _bottomCenter.x - _size.x * 0.5f, _bottomCenter.y - _size.y },
            { _bottomCenter.x + _size.x * 0.5f,           _bottomCenter.y }
        };
    }

    [[nodiscard]] constexpr float GetWidth() const
    {
        return max.x - min.x;
    }

    [[nodiscard]] constexpr float GetHeight() const
    {
        return max.y - min.y;
    }

    [[nodiscard]] constexpr Vec2 GetSize() const
    {
        return { max.x - min.x, max.y - min.y };
    }

    [[nodiscard]] constexpr Vec2 GetCenter() const
    {
        return { (min.x + max.x) * 0.5f, (min.y + max.y) * 0.5f };
    }

    [[nodiscard]] constexpr Vec2 GetBottomCenter() const
    {
        return { (min.x + max.x) * 0.5f, max.y };
    }

    // 크기가 0 이하이면 비어 있다고 본다.
    [[nodiscard]] constexpr bool IsEmpty() const
    {
        return max.x <= min.x || max.y <= min.y;
    }

    [[nodiscard]] constexpr bool Overlaps(
        const Aabb& _other) const
    {
        return min.x < _other.max.x && max.x > _other.min.x && min.y < _other.max.y && max.y > _other.min.y;
    }

    // 점 포함 (min 포함, max 미포함)
    [[nodiscard]] constexpr bool Contains(
        const Vec2 _point) const
    {
        return _point.x >= min.x && _point.x < max.x && _point.y >= min.y && _point.y < max.y;
    }

    [[nodiscard]] constexpr bool Contains(
        const Aabb& _other) const
    {
        return _other.min.x >= min.x && _other.max.x <= max.x && _other.min.y >= min.y && _other.max.y <= max.y;
    }

    // 겹치는 영역. 겹치지 않으면 nullopt.
    [[nodiscard]] std::optional<Aabb> Intersection(
        const Aabb& _other) const
    {
        if (!Overlaps(_other))
        {
            return std::nullopt;
        }
        return Aabb {
            { slug::Max(min.x, _other.min.x), slug::Max(min.y, _other.min.y) },
            { slug::Min(max.x, _other.max.x), slug::Min(max.y, _other.max.y) }
        };
    }

    // 모든 변을 _amount 만큼 바깥으로(음수면 안쪽으로) 확장한 사각형.
    [[nodiscard]] constexpr Aabb Expanded(
        const float _amount) const
    {
        return Aabb {
            { min.x - _amount, min.y - _amount },
            { max.x + _amount, max.y + _amount }
        };
    }

    [[nodiscard]] constexpr Aabb Expanded(
        const Vec2 _amount) const
    {
        return Aabb {
            { min.x - _amount.x, min.y - _amount.y },
            { max.x + _amount.x, max.y + _amount.y }
        };
    }

    [[nodiscard]] constexpr Aabb Translated(
        const Vec2 _offset) const
    {
        return Aabb {
            { min.x + _offset.x, min.y + _offset.y },
            { max.x + _offset.x, max.y + _offset.y }
        };
    }

    // 두 사각형을 모두 포함하는 최소 사각형
    [[nodiscard]] constexpr Aabb Union(
        const Aabb& _other) const
    {
        return Aabb {
            { slug::Min(min.x, _other.min.x), slug::Min(min.y, _other.min.y) },
            { slug::Max(max.x, _other.max.x), slug::Max(max.y, _other.max.y) }
        };
    }

    // _axisX 를 기준으로 좌우 반전한 사각형 (바라보는 방향이 바뀔 때 로컬 히트박스를 뒤집는 용도)
    [[nodiscard]] constexpr Aabb MirroredX(
        const float _axisX) const
    {
        return Aabb {
            { 2.f * _axisX - max.x, min.y },
            { 2.f * _axisX - min.x, max.y }
        };
    }

    [[nodiscard]] constexpr bool operator==(
        const Aabb& _other) const
    {
        return min == _other.min && max == _other.max;
    }
};

// ===========================================
//  Circle
// ===========================================

struct Circle
{
    Vec2 center = { 0.f, 0.f };
    float        radius = 0.f;

    [[nodiscard]] bool Overlaps(
        const Circle& _other) const
    {
        const float dx  = _other.center.x - center.x;
        const float dy  = _other.center.y - center.y;
        const float sum = radius + _other.radius;
        return dx * dx + dy * dy < sum * sum;
    }

    [[nodiscard]] bool Overlaps(
        const Aabb& _box) const
    {
        const float cx = slug::Clamp(center.x, _box.min.x, _box.max.x);
        const float cy = slug::Clamp(center.y, _box.min.y, _box.max.y);
        const float dx = center.x - cx;
        const float dy = center.y - cy;
        return dx * dx + dy * dy < radius * radius;
    }

    [[nodiscard]] Aabb GetBounds() const
    {
        return Aabb {
            { center.x - radius, center.y - radius },
            { center.x + radius, center.y + radius }
        };
    }
};

}   // namespace slug
