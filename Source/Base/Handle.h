#pragma once
#include <compare>
#include <cstdint>
#include <functional>

namespace slug
{

struct NullHandleType
{
};

inline constexpr NullHandleType kNullHandle {};

template<typename T>
class Handle
{
public:
    constexpr Handle() = default;
    constexpr Handle(const NullHandleType)
    {
    }
    constexpr Handle(const uint32_t _index, const uint8_t _token)
        : m_value((static_cast<uint32_t>(_token) << 24) | (_index & 0xFFFFFFu))
    {
    }
    constexpr explicit Handle(const uint32_t _value)
        : m_value(_value)
    {
    }

    [[nodiscard]] constexpr bool operator==(const Handle _other) const { return m_value == _other.m_value; }
    [[nodiscard]] constexpr auto operator<=>(const Handle _other) const { return m_value <=> _other.m_value; }

    [[nodiscard]] constexpr bool     IsNull() const { return m_value == kNullValue; }
    constexpr explicit               operator bool() const { return !IsNull(); }
    [[nodiscard]] constexpr uint32_t GetIndex() const { return m_value & 0xFFFFFFu; }
    [[nodiscard]] constexpr uint32_t GetValue() const { return m_value; }

private:
    static constexpr uint32_t kNullValue = 0xFFFFFFFFu;

    uint32_t m_value = kNullValue;
};
}

template<typename T>
struct std::hash<slug::Handle<T>>
{
    size_t operator()(const slug::Handle<T> _handle) const
    {
        return std::hash<uint32_t>()(_handle.GetValue());
    }
};
