#pragma once
#include <variant>

#include "Base/Assert.h"
#include "Base/Error.h"

namespace slug
{

template<typename T>
class Result
{
public:
    static_assert(!std::is_same_v<T, Error>, "Result<T> cannot hold Error. Use Result<void> instead.");

    Result(T _value)
        : m_storage(std::in_place_index<0>, std::move(_value))
    {
    }
    Result(const Error _error)
        : m_storage(std::in_place_index<1>, _error)
    {
    }
    Result(const eError _error)
        : m_storage(std::in_place_index<1>, Error { _error })
    {
    }

    [[nodiscard]] bool HasValue() const { return m_storage.index() == 0; }
    [[nodiscard]] bool HasError() const { return m_storage.index() == 1; }
    explicit operator bool() const { return HasValue(); }

    [[nodiscard]] T& GetValue()
    {
        SLUG_ASSERT(HasValue(), "Result does not contain a value");
        return std::get<0>(m_storage);
    }
    [[nodiscard]] const T& GetValue() const
    {
        SLUG_ASSERT(HasValue(), "Result does not contain a value");
        return std::get<0>(m_storage);
    }
    [[nodiscard]] T Take()
    {
        SLUG_ASSERT(HasValue(), "Result does not contain a value");
        return std::move(std::get<0>(m_storage));
    }
    [[nodiscard]] Error GetError() const
    {
        SLUG_ASSERT(HasError(), "Result does not contain an error");
        return std::get<1>(m_storage);
    }

    [[nodiscard]] T&       operator*()       { return GetValue(); }
    [[nodiscard]] const T& operator*() const { return GetValue(); }
    [[nodiscard]] T*       operator->()       { return &GetValue(); }
    [[nodiscard]] const T* operator->() const { return &GetValue(); }

private:
    std::variant<T, Error> m_storage;
};

template<>
class Result<void>
{
public:
    Result() = default;
    Result(const Error _error)
        : m_error(_error)
    {
    }
    Result(const eError _error)
        : m_error(_error)
    {
    }

    [[nodiscard]] bool IsOK() const { return m_error.IsOK(); }
    [[nodiscard]] bool HasError() const { return m_error.IsError(); }
    [[nodiscard]] Error GetError() const { return m_error; }
    explicit operator bool() const { return IsOK(); }

private:
    Error m_error = {};
};

}   // namespace slug
