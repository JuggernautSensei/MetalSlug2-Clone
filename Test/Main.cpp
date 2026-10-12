// 테스트 실행 진입점. 에셋 루트 폴백을 설정하고 gtest 를 실행한다.

#include <gtest/gtest.h>

#include "Base/Log.h"
#include "Core/AssetPath.h"

int main(
    int    _argc,
    char** _argv)
{
    InstallCoreLogger();
#ifdef SLUG_ASSET_DIR
    SetAssetRootFallback(SLUG_ASSET_DIR);
#endif
    ::testing::InitGoogleTest(&_argc, _argv);
    return RUN_ALL_TESTS();
}
