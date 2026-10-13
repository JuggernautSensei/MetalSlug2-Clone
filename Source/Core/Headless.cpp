#include "Core/Headless.h"

#include <cstdio>
#include <cstdlib>
#include <memory>

#include "Base/Log.h"
#include "Base/SlugError.h"
#include "Base/Types.h"
#include "Core/AssetPath.h"
#include "Core/Replay.h"
#include "Core/Tuning.h"
#include "Game/SimRunner.h"
#include "Game/StateFactory.h"
#include "Render/Animation.h"

namespace
{
uint64_t FnvMix_(
    uint64_t       _hash,
    const uint64_t _value)
{
    for (int i = 0; i < 8; ++i)
    {
        _hash ^= (_value >> (i * 8)) & 0xFFull;
        _hash *= 1099511628211ull;
    }
    return _hash;
}

// 결정적 합성 입력: 오른쪽 이동 + 주기적 점프/사격/폭탄. _flipPeriod > 0 이면 그 주기로 좌우를 번갈아 걷는다.
InputFrame MakeScriptedFrame_(
    const uint64_t _tick,
    const uint64_t _flipPeriod)
{
    InputFrame frame;
    const bool bLeft = _flipPeriod > 0 && ((_tick / _flipPeriod) % 2) == 1;
    frame.Set(bLeft ? eAction::Left : eAction::Right, true);
    frame.Set(eAction::Jump, (_tick % 150) < 12 && _tick > 60);
    frame.Set(eAction::Shot, (_tick % 10) < 3);
    frame.Set(eAction::Up, (_tick % 400) >= 380);
    frame.Set(eAction::Bomb, (_tick % 420) == 300);
    return frame;
}
}   // namespace

int RunHeadless(
    const int _argc,
    char**    _argv)
{
#ifdef SLUG_ASSET_DIR
    SetAssetRootFallback(SLUG_ASSET_DIR);
#endif
    FilePath replayPath;
    String   stateName;
    uint64_t maxTicks = 0;

    for (int i = 1; i < _argc; ++i)
    {
        const String arg      = _argv[i];
        const bool   bHasNext = i + 1 < _argc;
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
            std::fputs(DumpTuning(Tuning {}).c_str(), stdout);
            return 0;
        }
        else if (arg == "--gen-replay" && _argc > i + 4)
        {
            // --gen-replay <out> <state> <ticks> <flipPeriod>: 합성 입력 리플레이를 만든다 (회귀 기준선 생성용)
            ReplayData data;
            data.seed                 = 12345;
            data.stateName            = _argv[i + 2];
            const uint64_t ticks      = std::strtoull(_argv[i + 3], nullptr, 10);
            const uint64_t flipPeriod = std::strtoull(_argv[i + 4], nullptr, 10);
            for (uint64_t tick = 0; tick < ticks; ++tick)
            {
                data.frames.push_back(MakeScriptedFrame_(tick, flipPeriod));
            }
            if (const Result<void> saved = SaveReplay(_argv[i + 1], data); !saved)
            {
                SLUG_LOG_ERROR("리플레이 저장 실패: {}", saved.GetError().MakeMessage());
                return 1;
            }
            std::printf("wrote %s frames=%llu\n", _argv[i + 1], static_cast<unsigned long long>(ticks));
            return 0;
        }
    }

    if (replayPath.empty())
    {
        std::fprintf(stderr, "usage: MyMetalSlug2 --headless --replay <file> [--state <name>] [--ticks <n>]\n");
        return 1;
    }

    Result<ReplayData> loaded = LoadReplay(replayPath);
    if (!loaded)
    {
        SLUG_LOG_ERROR("리플레이 로드 실패: {}", loaded.GetError().MakeMessage());
        return 1;
    }
    const ReplayData replay = loaded.Take();
    if (stateName.empty())
    {
        stateName = replay.stateName;
    }

    AnimationLibrary anims;
    if (Result<AnimationLibrary> library = AnimationLibrary::LoadFromDirectory(FindAssetRoot()))
    {
        anims = library.Take();
    }
    else
    {
        SLUG_LOG_WARN("에셋 없음: 애니메이션 없이 진행");
    }

    GameContext ctx;
    ctx.pAnims    = &anims;
    ctx.assetRoot = FindAssetRoot();

    Scoped<IGameState> pState = CreateState(stateName);
    if (!pState)
    {
        SLUG_LOG_ERROR("알 수 없는 게임 상태: {}", stateName);
        return 1;
    }

    SimRunner      runner { ctx, std::move(pState) };
    const uint64_t limit = maxTicks > 0 ? maxTicks : replay.frames.size();
    for (uint64_t tick = 0; tick < limit && tick < replay.frames.size(); ++tick)
    {
        runner.Step(replay.frames[tick]);
        if (runner.IsFinished())
        {
            break;
        }
    }

    uint64_t hash = 14695981039346656037ull;
    hash          = FnvMix_(hash, runner.GetTickCount());
    hash          = FnvMix_(hash, runner.IsFinished() ? 1ull : 0ull);
    hash          = FnvMix_(hash, runner.GetState() != nullptr ? runner.GetState()->DebugHash() : 0ull);
    std::printf("ticks=%llu finished=%d hash=%016llx\n", static_cast<unsigned long long>(runner.GetTickCount()), runner.IsFinished() ? 1 : 0, static_cast<unsigned long long>(hash));
    return 0;
}
