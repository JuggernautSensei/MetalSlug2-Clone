#pragma once
#include <cstdint>
#include <limits>
#include <random>
#include <type_traits>

#include "Base/Assert.h"

namespace slug
{

class RngMt19937
{
public:
    RngMt19937() = default;
    explicit RngMt19937(const uint64_t _seed)
        : m_mt(_seed)
    {
    }

    void                   Reseed(const uint64_t _seed) { m_mt.seed(_seed); }
    [[nodiscard]] uint64_t Generate() { return m_mt(); }

private:
    std::mt19937_64 m_mt;
};

template<typename R>
concept RngT = requires(R _rng) {
    { _rng.Generate() } -> std::same_as<uint64_t>;
};

// 실수는 [0, 1], 정수는 uint64 범위 그대로.
template<typename T, RngT R>
    requires std::is_arithmetic_v<T>
[[nodiscard]] T Random(R& _rng)
{
    if constexpr (std::is_floating_point_v<T>)
    {
        return static_cast<T>(_rng.Generate()) / static_cast<T>(std::numeric_limits<uint64_t>::max());
    }
    else
    {
        return static_cast<T>(_rng.Generate());
    }
}

// 실수는 [_min, _max], 정수는 [_min, _max] (양끝 포함).
template<typename T, RngT R>
    requires std::is_arithmetic_v<T>
[[nodiscard]] T Random(R& _rng, const T _min, const T _max)
{
    SLUG_ASSERT(_min <= _max, "Invalid range: min > max.");
    if constexpr (std::is_floating_point_v<T>)
    {
        const T range = _max - _min;
        return Random<T>(_rng) * range + _min;
    }
    else
    {
        const T range = _max - _min + 1;
        return static_cast<T>(_rng.Generate() % range) + _min;
    }
}

}   // namespace slug
