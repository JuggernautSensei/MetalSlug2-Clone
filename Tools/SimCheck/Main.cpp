#include "Audio/AudioSystem.h"
#include "Base/Log.h"
#include "Core/AssetPath.h"
#include "Game/SimRunner.h"
#include "Game/StateFactory.h"
#include "Render/Animation.h"
#include "Render/Renderer.h"
#include "Render/SpriteBatch.h"

int main()
{
    InstallCoreLogger();
    const FilePath root = FindAssetRoot();
    Result<AnimationLibrary> anims = AnimationLibrary::LoadFromDirectory(root);
    if (!anims) return 1;
    AnimationLibrary::GetSingleton() = anims.Take();
    if (!Renderer::GetSingleton().Init("SimCheck", 1)) return 1;
    AudioConfig audio;
    audio.bEnabled = false;
    if (!AudioSystem::GetSingleton().InitFromDirectory(root, audio)) return 1;

    constexpr const char* states[] = { "stage0", "sandbox", "enemylab", "vehiclelab", "proplab", "bosslab", "bosslab_bot", "buslab", "buslab_bot", "helilab", "helilab_kill", "title", "demo" };
    for (const char* name : states)
    {
        for (const int flip : { 0, 120 })
        {
            g_debug = {};
            SpriteBatch::GetSingleton().Init(Renderer::GetSingleton().GetNative());
            SpriteBatch::GetSingleton().SetAssetRoot(root);
            if (!SpriteBatch::GetSingleton().LoadAtlasPagesParallel(AnimationLibrary::GetSingleton(), 1)) return 1;
            {
                SimRunner sim { CreateState(name) };
                if (!sim.GetState()) return 1;
                for (int tick = 0; tick < 1800; ++tick)
                {
                    InputFrame frame;
                    const bool left = flip > 0 && (tick / flip) % 2 != 0;
                    frame.Set(left ? eAction::Left : eAction::Right, true);
                    frame.Set(eAction::Shot, tick % 7 < 4);
                    frame.Set(eAction::Jump, tick % 90 < 2);
                    frame.Set(eAction::Bomb, tick % 240 < 2);
                    frame.Set(eAction::Up, tick % 180 >= 150);
                    frame.Set(eAction::Start, tick == 60);
                    sim.Step(frame);
                    if ((tick + 1) % 60 == 0)
                    {
                        std::printf("SIM %s flip=%d tick=%llu finished=%d hash=%016llx\n", name, flip, static_cast<unsigned long long>(sim.GetTickCount()), sim.IsFinished(), static_cast<unsigned long long>(sim.GetState()->DebugHash()));
                    }
                }
            }
            SpriteBatch::GetSingleton().Shutdown();
        }
    }
    AudioSystem::GetSingleton().Shutdown();
    Renderer::GetSingleton().Shutdown();
    return 0;
}
