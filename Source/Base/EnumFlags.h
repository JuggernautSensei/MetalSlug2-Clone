#pragma once
#include <concepts>
#include <cstdint>
#include <type_traits>

namespace slug
{

struct AllFlagTag
{
};

inline constexpr AllFlagTag kAllFlag {};

template<typename E>
    requires std::is_enum_v<E>
class IndexedFlags
{
public:
    constexpr IndexedFlags() = default;
    constexpr IndexedFlags(AllFlagTag)
        : m_bits(~0u)
    {
    }
    template<typename... Es>
        requires(sizeof...(Es) > 0 && (std::same_as<Es, E> && ...))
    constexpr IndexedFlags(const Es... _values)
        : m_bits((0u | ... | Bit_(_values)))
    {
    }

    [[nodiscard]] constexpr bool Has(const E _value) const { return (m_bits & Bit_(_value)) != 0u; }
    constexpr void               Add(const E _value) { m_bits |= Bit_(_value); }
    constexpr void               Remove(const E _value) { m_bits &= ~Bit_(_value); }

private:
    static constexpr uint32_t Bit_(const E _value)
    {
        return 1u << static_cast<uint32_t>(_value);
    }

    uint32_t m_bits = 0u;
};
}
