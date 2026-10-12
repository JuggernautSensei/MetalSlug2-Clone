#include "Base/Log.h"

#include <cstdio>

void LogMessage(
    const eLogLevel        _level,
    const std::string_view _message)
{
    const char* pPrefix = "";
    switch (_level)
    {
        case eLogLevel::Trace: pPrefix = "[trace] "; break;
        case eLogLevel::Info: pPrefix = "[info] "; break;
        case eLogLevel::Warn: pPrefix = "[warn] "; break;
        case eLogLevel::Error: pPrefix = "[error] "; break;
        case eLogLevel::Fatal: pPrefix = "[fatal] "; break;
    }
    std::fprintf(stderr, "%s%.*s\n", pPrefix, static_cast<int>(_message.size()), _message.data());
}

static std::FILE*         g_pEventLog   = nullptr;
static unsigned long long g_EventTick   = 0;

bool OpenEventLog(
    const std::string_view _path)
{
    const std::string path(_path);
    g_pEventLog = nullptr;
    (void)fopen_s(&g_pEventLog, path.c_str(), "w");
    return g_pEventLog != nullptr;
}

bool IsEventLogOpen()
{
    return g_pEventLog != nullptr;
}

void SetEventLogTick(
    const unsigned long long _tick)
{
    g_EventTick = _tick;
}

void WriteEventLog(
    const std::string_view _event,
    const std::string_view _fields)
{
    if (!g_pEventLog)
    {
        return;
    }
    std::fprintf(g_pEventLog, "{\"tick\":%llu,\"event\":\"%.*s\"", g_EventTick, static_cast<int>(_event.size()), _event.data());
    if (!_fields.empty())
    {
        std::fprintf(g_pEventLog, ",%.*s", static_cast<int>(_fields.size()), _fields.data());
    }
    std::fputs("}\n", g_pEventLog);
    std::fflush(g_pEventLog);
}
