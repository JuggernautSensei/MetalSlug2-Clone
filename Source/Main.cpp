#include <cstring>

#include "Base/Log.h"
#include "Core/App.h"
#include "Core/Headless.h"

int main(
    int    _argc,
    char** _argv)
{
    InstallCoreLogger();
    if (_argc > 1 && std::strcmp(_argv[1], "--headless") == 0)
    {
        return RunHeadless(_argc, _argv);
    }
    App app;
    return app.Run(_argc, _argv);
}
