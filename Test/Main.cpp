
#include <SDL3/SDL.h>
#include <gtest/gtest.h>

#include "Audio/AudioSystem.h"
#include "Base/Log.h"
#include "Core/AssetPath.h"
#include "Render/Animation.h"
#include "Render/Renderer.h"
#include "Render/SpriteBatch.h"

namespace
{
class ServicesEnvironment : public ::testing::Environment
{
public:
    void SetUp() override
    {
        SDL_SetHint(SDL_HINT_VIDEO_DRIVER, "offscreen");
        SDL_SetHint(SDL_HINT_RENDER_DRIVER, "software");
        ASSERT_TRUE(SDL_Init(SDL_INIT_VIDEO)) << SDL_GetError();
        ASSERT_TRUE(static_cast<bool>(Renderer::GetSingleton().Init("SlugTest", 1)));
        SpriteBatch::GetSingleton().Init(Renderer::GetSingleton().GetNative());
        SpriteBatch::GetSingleton().SetAssetRoot(FindAssetRoot());
    }

    void TearDown() override
    {
        AudioSystem::GetSingleton().Shutdown();
        SpriteBatch::GetSingleton().Shutdown();
        Renderer::GetSingleton().Shutdown();
        SDL_Quit();
    }
};

class ResetServicesListener : public ::testing::EmptyTestEventListener
{
public:
    void OnTestEnd(const ::testing::TestInfo&) override
    {
        AudioSystem::GetSingleton().Shutdown();
        AudioSystem::GetSingleton().bRecord = false;
        AudioSystem::GetSingleton().events.clear();
        AnimationLibrary::GetSingleton() = {};
        SpriteBatch::GetSingleton().Init(Renderer::GetSingleton().GetNative());
    }
};
}   // namespace

int main(
    int    _argc,
    char** _argv)
{
    InstallCoreLogger();
#ifdef SLUG_ASSET_DIR
    SetAssetRootFallback(SLUG_ASSET_DIR);
#endif
    ::testing::InitGoogleTest(&_argc, _argv);
    ::testing::AddGlobalTestEnvironment(new ServicesEnvironment);
    ::testing::UnitTest::GetInstance()->listeners().Append(new ResetServicesListener);
    return RUN_ALL_TESTS();
}
