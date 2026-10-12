#pragma once
#include <format>
#include <string_view>
#include <utility>

#include <JugX/Logger.h>

using jug::eLogLevel;

void LogMessage(eLogLevel _level, StringView _message);

// JugX 코어 로거(JUG_CORE_LOG_*, JUG_ASSERT 실패 출력)를 LogMessage 로 보낸다. 로그 첫 사용 전에 호출한다.
void InstallCoreLogger();

template<typename... Args>
void LogFormat(
    const eLogLevel                   _level,
    const std::format_string<Args...> _format,
    Args&&... _args)
{
    LogMessage(_level, std::format(_format, std::forward<Args>(_args)...));
}

// --event-log <파일>: 한 줄에 JSON 객체 하나. 열려 있지 않으면 아무 일도 하지 않는다 (시뮬레이션 무영향)
bool OpenEventLog(StringView _path);
bool IsEventLogOpen();
void SetEventLogTick(unsigned long long _tick);
void WriteEventLog(StringView _event, StringView _fields);

template<typename... Args>
void EventLogFormat(
    const StringView                  _event,
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
