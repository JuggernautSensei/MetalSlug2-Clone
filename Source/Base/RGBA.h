#pragma once
#include <cstdint>

struct RGBA
{
    constexpr RGBA() = default;
    __forceinline constexpr RGBA(uint8_t _r, uint8_t _g, uint8_t _b, uint8_t _a = 255)
        : r(_r)
        , g(_g)
        , b(_b)
        , a(_a)
    {
    }

    [[nodiscard]] constexpr bool operator==(const RGBA& _other) const = default;

    uint8_t r = 0;
    uint8_t g = 0;
    uint8_t b = 0;
    uint8_t a = 255;
};
