// 테스트 실행 진입점. 에셋 루트 폴백을 설정하고, 오프스크린 SDL + 소프트웨어 렌더러 서비스를 한 번 만든 뒤 gtest 를 실행한다.

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
class ServicesEnvironment: public ::testing::Environment
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

// 테스트가 서로의 서비스 상태를 물려받지 않게 매 테스트 뒤에 비운다.
class ResetServicesListener: public ::testing::EmptyTestEventListener
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
