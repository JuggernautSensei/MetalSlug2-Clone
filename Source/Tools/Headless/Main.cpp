#include <cstdio>
#include "Base/Types.h"
#include "Base/Result.h"
#include <cstdlib>

#include "Base/Log.h"
#include "Base/Random.h"

#include "Core/Replay.h"
#include "Core/Tuning.h"
#include "Game/SimRunner.h"
#include "Game/StateFactory.h"
#include "Render/Animation.h"

int main(
    int    _argc,
    char** _argv)
{
    slug::FilePath replayPath;
    slug::String   stateName;
    uint64_t      maxTicks         = 0;
    bool          bExpectFinished = false;

    for (int i = 1; i < _argc; ++i)
    {
        const slug::String arg      = _argv[i];
        const bool        bHasNext = i + 1 < _argc;
        if (arg == "--replay" && bHasNext)
        {
            replayPath = _argv[++i];
        }
        else if (arg == "--state" && bHasNext)
        {
            stateName = _argv[++i];
        }
        else if (arg == "--ticks" && bHasNext)
        {
            maxTicks = std::strtoull(_argv[++i], nullptr, 10);
        }
        else if (arg == "--dump-tuning")
        {
            std::fputs(slug::DumpTuning(slug::Tuning {}).c_str(), stdout);
            return 0;
        }
        else if (arg == "--expect-finished")
        {
            bExpectFinished = true;
        }
    }

    if (replayPath.empty())
    {
        std::fprintf(stderr, "usage: SlugHeadless --replay <file> [--state <name>] [--ticks <n>] [--expect-finished]\n");
        return 1;
    }

    slug::Result<slug::ReplayData> loaded = slug::LoadReplay(replayPath);
    if (!loaded)
    {
        SLUG_LOG_ERROR("리플레이 로드 실패: {}", loaded.GetError().MakeMessage());
        return 1;
    }
    const slug::ReplayData replay = loaded.Take();
    if (stateName.empty())
    {
        stateName = replay.stateName;
    }

    slug::AnimationLibrary anims;
    if (slug::Result<slug::AnimationLibrary> library = slug::AnimationLibrary::LoadFromDirectory(slug::FilePath { SLUG_ASSET_DIR } / "Baked"))
    {
        anims = library.Take();
    }
    else
    {
        SLUG_LOG_WARN("Baked 에셋 없음: 애니메이션 없이 진행");
    }

    slug::RngMt19937   rng { replay.seed };
    slug::GameContext ctx;
    ctx.pAnims    = &anims;
    ctx.pRng      = &rng;
    ctx.assetRoot = slug::FilePath { SLUG_ASSET_DIR };

    std::unique_ptr<slug::IGameState> pState = slug::CreateState(stateName);
    if (!pState)
    {
        SLUG_LOG_ERROR("알 수 없는 게임 상태: {}", stateName);
        return 1;
    }

    slug::SimRunner runner { ctx, std::move(pState) };
    const uint64_t  limit = maxTicks > 0 ? maxTicks : replay.frames.size();
    for (uint64_t tick = 0; tick < limit && tick < replay.frames.size(); ++tick)
    {
        runner.Step(replay.frames[tick]);
        if (runner.IsFinished())
        {
            break;
        }
    }

    std::printf("ticks=%llu finished=%d\n", static_cast<unsigned long long>(runner.GetTickCount()), runner.IsFinished() ? 1 : 0);
    if (bExpectFinished && !runner.IsFinished())
    {
        return 2;
    }
    return 0;
}
