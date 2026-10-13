#pragma once
#include <cmath>
#include <compare>
#include <concepts>
#include <cstddef>
#include <cstdint>
#include <type_traits>

#include <JugX/Math.h>

using jug::kDeg2Rad;
using jug::kRad2Deg;

using jug::Abs;
using jug::Clamp;
using jug::Lerp;
using jug::Max;
using jug::Min;
using jug::Sqrt;

// _rng.Generate() 의 64비트 값을 [_lo, _hi] 로 편다
template<typename RngT>
[[nodiscard]] float RandRange(RngT& _rng, const float _lo, const float _hi)
{
    const double unit = static_cast<double>(_rng.Generate()) / 18'446'744'073'709'551'615.0;
    return _lo + static_cast<float>(unit) * (_hi - _lo);
}

struct Vec2
{
    constexpr Vec2() = default;
    __forceinline constexpr Vec2(const float _x, const float _y)
        : x(_x)
        , y(_y)
    {
    }
    explicit __forceinline constexpr Vec2(const float _value)
        : x(_value)
        , y(_value)
    {
    }

    [[nodiscard]] __forceinline constexpr Vec2 operator-() const { return Vec2 { -x, -y }; }
    [[nodiscard]] __forceinline constexpr Vec2 operator+(const Vec2 _other) const { return Vec2 { x + _other.x, y + _other.y }; }
    [[nodiscard]] __forceinline constexpr Vec2 operator-(const Vec2 _other) const { return Vec2 { x - _other.x, y - _other.y }; }
    [[nodiscard]] __forceinline constexpr Vec2 operator*(const float _scalar) const { return Vec2 { x * _scalar, y * _scalar }; }
    [[nodiscard]] __forceinline constexpr Vec2 operator/(const float _scalar) const { return Vec2 { x / _scalar, y / _scalar }; }
    [[nodiscard]] __forceinline constexpr Vec2 operator*(const Vec2 _other) const { return Vec2 { x * _other.x, y * _other.y }; }
    [[nodiscard]] __forceinline constexpr Vec2 operator/(const Vec2 _other) const { return Vec2 { x / _other.x, y / _other.y }; }

    __forceinline constexpr Vec2& operator+=(const Vec2 _other)
    {
        x += _other.x;
        y += _other.y;
        return *this;
    }
    __forceinline constexpr Vec2& operator-=(const Vec2 _other)
    {
        x -= _other.x;
        y -= _other.y;
        return *this;
    }
    __forceinline constexpr Vec2& operator*=(const float _scalar)
    {
        x *= _scalar;
        y *= _scalar;
        return *this;
    }
    __forceinline constexpr Vec2& operator/=(const float _scalar)
    {
        x /= _scalar;
        y /= _scalar;
        return *this;
    }
    __forceinline constexpr Vec2& operator*=(const Vec2 _other)
    {
        x *= _other.x;
        y *= _other.y;
        return *this;
    }
    __forceinline constexpr Vec2& operator/=(const Vec2 _other)
    {
        x /= _other.x;
        y /= _other.y;
        return *this;
    }

    [[nodiscard]] constexpr bool operator==(const Vec2& _other) const = default;

    [[nodiscard]] __forceinline constexpr float&       operator[](const size_t _index) { return _index == 0 ? x : y; }
    [[nodiscard]] __forceinline constexpr const float& operator[](const size_t _index) const { return _index == 0 ? x : y; }

    [[nodiscard]] __forceinline constexpr float*       GetPtr() { return &x; }
    [[nodiscard]] __forceinline constexpr const float* GetPtr() const { return &x; }

    float x = 0.f;
    float y = 0.f;
};

[[nodiscard]] constexpr Vec2 Lerp(const Vec2 _x, const Vec2 _y, const float _t)
{
    return _x + (_y - _x) * _t;
}
