#pragma once
#include <format>
#include <string_view>
#include <utility>

namespace slug
{

enum class eLogLevel
{
    Trace,
    Info,
    Warn,
    Error,
    Fatal
};

void LogMessage(eLogLevel _level, std::string_view _message);

template<typename... Args>
void LogFormat(
    const eLogLevel                   _level,
    const std::format_string<Args...> _format,
    Args&&... _args)
{
    LogMessage(_level, std::format(_format, std::forward<Args>(_args)...));
}
}

#define SLUG_LOG_TRACE(...) slug::LogFormat(slug::eLogLevel::Trace, __VA_ARGS__)
#define SLUG_LOG_INFO(...)  slug::LogFormat(slug::eLogLevel::Info, __VA_ARGS__)
#define SLUG_LOG_WARN(...)  slug::LogFormat(slug::eLogLevel::Warn, __VA_ARGS__)
#define SLUG_LOG_ERROR(...) slug::LogFormat(slug::eLogLevel::Error, __VA_ARGS__)
#define SLUG_LOG_FATAL(...) slug::LogFormat(slug::eLogLevel::Fatal, __VA_ARGS__)
