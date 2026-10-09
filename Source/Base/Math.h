#pragma once
#include <cmath>
#include <compare>
#include <concepts>
#include <cstddef>
#include <cstdint>
#include <numbers>
#include <type_traits>

namespace slug
{

inline constexpr float kPi        = std::numbers::pi_v<float>;
inline constexpr float kDeg2Rad   = kPi / 180.f;
inline constexpr float kRad2Deg   = 180.f / kPi;
inline constexpr float kEpsilon   = 1e-5f;

template<typename T, typename U>
[[nodiscard]] constexpr std::common_type_t<T, U> Min(const T _x, const U _y)
{
    return _x < _y ? _x : _y;
}

template<typename T, typename U>
[[nodiscard]] constexpr std::common_type_t<T, U> Max(const T _x, const U _y)
{
    return _x > _y ? _x : _y;
}

template<typename T>
[[nodiscard]] constexpr T Clamp(const T _x, const T _min, const T _max)
{
    return _x < _min ? _min : (_x > _max ? _max : _x);
}

template<typename T>
[[nodiscard]] constexpr T Abs(const T _x)
{
    return _x < T { 0 } ? -_x : _x;
}

template<typename T>
[[nodiscard]] constexpr T Lerp(const T _x, const T _y, const float _t)
{
    return static_cast<T>(_x + (_y - _x) * _t);
}

struct Vec2
{
    constexpr Vec2() = default;
    constexpr Vec2(const float _x, const float _y)
        : x(_x)
        , y(_y)
    {
    }
    explicit constexpr Vec2(const float _value)
        : x(_value)
        , y(_value)
    {
    }

    [[nodiscard]] constexpr Vec2 operator-() const { return Vec2 { -x, -y }; }
    [[nodiscard]] constexpr Vec2 operator+(const Vec2 _other) const { return Vec2 { x + _other.x, y + _other.y }; }
    [[nodiscard]] constexpr Vec2 operator-(const Vec2 _other) const { return Vec2 { x - _other.x, y - _other.y }; }
    [[nodiscard]] constexpr Vec2 operator*(const float _scalar) const { return Vec2 { x * _scalar, y * _scalar }; }
    [[nodiscard]] constexpr Vec2 operator/(const float _scalar) const { return Vec2 { x / _scalar, y / _scalar }; }
    [[nodiscard]] constexpr Vec2 operator*(const Vec2 _other) const { return Vec2 { x * _other.x, y * _other.y }; }
    [[nodiscard]] constexpr Vec2 operator/(const Vec2 _other) const { return Vec2 { x / _other.x, y / _other.y }; }

    constexpr Vec2& operator+=(const Vec2 _other)
    {
        x += _other.x;
        y += _other.y;
        return *this;
    }
    constexpr Vec2& operator-=(const Vec2 _other)
    {
        x -= _other.x;
        y -= _other.y;
        return *this;
    }
    constexpr Vec2& operator*=(const float _scalar)
    {
        x *= _scalar;
        y *= _scalar;
        return *this;
    }
    constexpr Vec2& operator/=(const float _scalar)
    {
        x /= _scalar;
        y /= _scalar;
        return *this;
    }
    constexpr Vec2& operator*=(const Vec2 _other)
    {
        x *= _other.x;
        y *= _other.y;
        return *this;
    }
    constexpr Vec2& operator/=(const Vec2 _other)
    {
        x /= _other.x;
        y /= _other.y;
        return *this;
    }

    [[nodiscard]] constexpr bool operator==(const Vec2& _other) const = default;

    [[nodiscard]] constexpr float& operator[](const size_t _index) { return _index == 0 ? x : y; }
    [[nodiscard]] constexpr const float& operator[](const size_t _index) const { return _index == 0 ? x : y; }

    [[nodiscard]] constexpr float*       GetPtr()       { return &x; }
    [[nodiscard]] constexpr const float* GetPtr() const { return &x; }

    static const Vec2 kZero;
    static const Vec2 kOne;
    static const Vec2 kRight;
    static const Vec2 kUp;
    static const Vec2 kUnitX;
    static const Vec2 kUnitY;

    float x = 0.f;
    float y = 0.f;
};

inline constexpr Vec2 Vec2::kZero  = { 0.f, 0.f };
inline constexpr Vec2 Vec2::kOne   = { 1.f, 1.f };
inline constexpr Vec2 Vec2::kRight = { 1.f, 0.f };
inline constexpr Vec2 Vec2::kUp    = { 0.f, 1.f };
inline constexpr Vec2 Vec2::kUnitX = { 1.f, 0.f };
inline constexpr Vec2 Vec2::kUnitY = { 0.f, 1.f };

}   // namespace slug
