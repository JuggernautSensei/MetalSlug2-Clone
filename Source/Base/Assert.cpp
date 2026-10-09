#include "Base/Assert.h"

#include <cstdio>

#include "Base/Log.h"

namespace slug
{

void ReportAssertFailure(
    const std::string_view      _expr,
    const std::string_view      _message,
    const std::source_location& _loc)
{
    LogFormat(eLogLevel::Fatal, "Assert failed: ({}) {} [{}:{}]", _expr, _message, _loc.file_name(), _loc.line());
#ifdef _MSC_VER
    __debugbreak();
#else
    std::abort();
#endif
}

}   // namespace slug
