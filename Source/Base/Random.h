#pragma once
#include <cstdint>
#include <limits>
#include <random>
#include <type_traits>

#include "Base/Assert.h"

class RngMt19937
{
public:
    RngMt19937() = default;
    explicit RngMt19937(const uint64_t _seed);

    void                   Reseed(const uint64_t _seed);
    [[nodiscard]] uint64_t Generate();

private:
    std::mt19937_64 m_mt;
};

// [_min, _max] 범위의 난수. 정수는 양 끝을 포함하고, 실수는 [_min, _max) 구간이다.
template<typename T>
    requires std::is_arithmetic_v<T>
[[nodiscard]] T Random(RngMt19937& _rng, const T _min, const T _max)
{
    SLUG_ASSERT(_min <= _max, "Invalid range: min > max.");
    if constexpr (std::is_floating_point_v<T>)
    {
        const T unit = static_cast<T>(_rng.Generate()) / static_cast<T>(std::numeric_limits<uint64_t>::max());
        return unit * (_max - _min) + _min;
    }
    else
    {
        const T range = _max - _min + 1;
        return static_cast<T>(_rng.Generate() % range) + _min;
    }
}
