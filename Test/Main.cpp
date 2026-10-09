#include <gtest/gtest.h>

#include "Core/AssetPath.h"

int main(
    int    _argc,
    char** _argv)
{
#ifdef SLUG_ASSET_DIR
    slug::SetAssetRootFallback(SLUG_ASSET_DIR);
#endif
    ::testing::InitGoogleTest(&_argc, _argv);
    return RUN_ALL_TESTS();
}
