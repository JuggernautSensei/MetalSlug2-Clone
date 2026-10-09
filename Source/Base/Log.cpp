#include "Base/Log.h"

#include <cstdio>

namespace slug
{

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

}   // namespace slug
