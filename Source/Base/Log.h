#pragma once
#include <format>
#include <string_view>
#include <utility>

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

// 이벤트 로그 (--event-log <파일>): 한 줄에 JSON 객체 하나 {"tick":N,"event":"이름",...필드}. 열려 있지 않으면 아무 일도 하지 않는다 (시뮬레이션에 영향 없음)
bool OpenEventLog(std::string_view _path);
bool IsEventLogOpen();
void SetEventLogTick(unsigned long long _tick);
void WriteEventLog(std::string_view _event, std::string_view _fields);

template<typename... Args>
void EventLogFormat(
    const std::string_view            _event,
    const std::format_string<Args...> _fields,
    Args&&... _args)
{
    if (IsEventLogOpen())
    {
        WriteEventLog(_event, std::format(_fields, std::forward<Args>(_args)...));
    }
}

#define SLUG_LOG_TRACE(...) LogFormat(eLogLevel::Trace, __VA_ARGS__)
#define SLUG_LOG_INFO(...)  LogFormat(eLogLevel::Info, __VA_ARGS__)
#define SLUG_LOG_WARN(...)  LogFormat(eLogLevel::Warn, __VA_ARGS__)
#define SLUG_LOG_ERROR(...) LogFormat(eLogLevel::Error, __VA_ARGS__)
#define SLUG_LOG_FATAL(...) LogFormat(eLogLevel::Fatal, __VA_ARGS__)
