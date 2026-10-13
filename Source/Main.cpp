#include <cstdio>
#include <cstdlib>
#include <cstring>

#include "Base/Log.h"
#include "Base/SlugError.h"
#include "Core/App.h"
#include "Core/Tuning.h"

int main(
    const int _argc,
    char**    _argv)
{
    InstallCoreLogger();
    if (_argc > 1 && std::strcmp(_argv[1], "--dump-tuning") == 0)
    {
        std::fputs(DumpTuning(Tuning {}).c_str(), stdout);
        return 0;
    }
    App app;
    return app.Run(_argc, _argv);
}
