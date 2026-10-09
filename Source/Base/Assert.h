#pragma once
#include <cstdlib>
#include <format>
#include <source_location>
#include <string_view>

namespace slug
{

void ReportAssertFailure(std::string_view _expr, std::string_view _message, const std::source_location& _loc);

template<typename... Args>
void AssertFail(
    const std::string_view            _expr,
    const std::source_location&       _loc,
    const std::format_string<Args...> _format,
    Args&&... _args)
{
    ReportAssertFailure(_expr, std::format(_format, std::forward<Args>(_args)...), _loc);
}
}

#ifdef NDEBUG
#    define SLUG_ASSERT(_cond, ...) ((void)0)
#else
#    define SLUG_ASSERT(_cond, ...)                                                      \
        do                                                                               \
        {                                                                                \
            if (!std::is_constant_evaluated() && !(_cond))                               \
            {                                                                            \
                slug::AssertFail(#_cond, std::source_location::current(), __VA_ARGS__); \
            }                                                                            \
        } while (false)
#endif
