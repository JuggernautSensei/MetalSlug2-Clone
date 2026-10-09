#pragma once
#include <cstdint>

namespace slug
{

struct RGBA
{
    constexpr RGBA() = default;
    constexpr RGBA(
        const uint8_t _r,
        const uint8_t _g,
        const uint8_t _b,
        const uint8_t _a = 255)
        : r(_r)
        , g(_g)
        , b(_b)
        , a(_a)
    {
    }

    [[nodiscard]] constexpr bool operator==(const RGBA& _other) const = default;

    static const RGBA kZero;
    static const RGBA kBlack;
    static const RGBA kWhite;
    static const RGBA kRed;
    static const RGBA kGreen;
    static const RGBA kBlue;
    static const RGBA kYellow;
    static const RGBA kCyan;
    static const RGBA kMagenta;

    uint8_t r = 0;
    uint8_t g = 0;
    uint8_t b = 0;
    uint8_t a = 255;
};

inline constexpr RGBA RGBA::kZero    = RGBA { 0, 0, 0, 0 };
inline constexpr RGBA RGBA::kBlack   = RGBA { 0, 0, 0, 255 };
inline constexpr RGBA RGBA::kWhite   = RGBA { 255, 255, 255, 255 };
inline constexpr RGBA RGBA::kRed     = RGBA { 255, 0, 0, 255 };
inline constexpr RGBA RGBA::kGreen   = RGBA { 0, 255, 0, 255 };
inline constexpr RGBA RGBA::kBlue    = RGBA { 0, 0, 255, 255 };
inline constexpr RGBA RGBA::kYellow  = RGBA { 255, 255, 0, 255 };
inline constexpr RGBA RGBA::kCyan    = RGBA { 0, 255, 255, 255 };
inline constexpr RGBA RGBA::kMagenta = RGBA { 255, 0, 255, 255 };
}
