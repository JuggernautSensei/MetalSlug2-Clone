#pragma once
#include <cmath>
#include <compare>
#include <concepts>
#include <cstddef>
#include <cstdint>
#include <type_traits>

#include <JugX/Vector.h>

using jug::kDeg2Rad;
using jug::kRad2Deg;

using jug::Abs;
using jug::Clamp;
using jug::Lerp;
using jug::Max;
using jug::Min;
using jug::Sqrt;
using jug::VECTOR2;

// _rng.Generate() 의 64비트 값을 [_lo, _hi] 로 편다
template<typename RngT>
[[nodiscard]] float RandRange(RngT& _rng, const float _lo, const float _hi)
{
    const double unit = static_cast<double>(_rng.Generate()) / 18'446'744'073'709'551'615.0;
    return _lo + static_cast<float>(unit) * (_hi - _lo);
}
