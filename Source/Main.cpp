#include "Base/Log.h"
#include "Base/SlugError.h"
#include "Core/App.h"

int main(
    const int _argc,
    char**    _argv)
{
    InstallCoreLogger();
    App app;
    return app.Run(_argc, _argv);
}
