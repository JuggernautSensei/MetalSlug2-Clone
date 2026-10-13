#include "Base/Log.h"

#include <cstdio>
#include <format>

#include <JugX/CoreLogger.h>

void LogMessage(
    eLogLevel  _level,
    StringView _message)
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

namespace
{

class SlugCoreLogger final : public jug::Logger
{
public:
    SlugCoreLogger()
    {
        SetPattern(jug::Flags<jug::eLogPattern> {});
    }

    void Flush() override
    {
        std::fflush(stderr);
    }

private:
    void LogImpl(
        eLogLevel       _level,
        jug::StringView _msg,
        bool            _bEndOfLog) override
    {
        if (_bEndOfLog)
        {
            LogMessage(_level, _msg);
        }
    }

    void VLogImpl(
        eLogLevel        _level,
        jug::StringView  _msg,
        std::format_args _args,
        bool             _bEndOfLog) override
    {
        if (_bEndOfLog)
        {
            LogMessage(_level, std::vformat(_msg, _args));
        }
    }
};

SlugCoreLogger g_coreLogger;

}   // namespace

void InstallCoreLogger()
{
    jug::SetCoreLogger(&g_coreLogger);
}

namespace
{
std::FILE*         g_pEventLog = nullptr;
unsigned long long g_EventTick = 0;
}   // namespace

bool OpenEventLog(
    StringView _path)
{
    const String path(_path);
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
    StringView _event,
    StringView _fields)
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
