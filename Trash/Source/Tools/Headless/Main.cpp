#include <cstdio>
#include <cstdlib>
#include <memory>

#include "Base/Log.h"
#include "Base/Random.h"
#include "Base/Result.h"
#include "Base/Types.h"
#include "Core/AssetPath.h"
#include "Core/Replay.h"
#include "Core/Tuning.h"
#include "Game/SimRunner.h"
#include "Game/StateFactory.h"
#include "Render/Animation.h"

namespace
{

// FNV-1a 64: 회귀 기준선 해시 (tick 수, 종료 여부, 상태의 DebugHash 를 섞는다)
uint64_t FnvMix(
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
slug::InputFrame MakeScriptedFrame(
    const uint64_t _tick,
    const uint64_t _flipPeriod)
{
    slug::InputFrame frame;
    const bool       bLeft = _flipPeriod > 0 && ((_tick / _flipPeriod) % 2) == 1;
    frame.Set(bLeft ? slug::eAction::Left : slug::eAction::Right, true);
    frame.Set(slug::eAction::Jump, (_tick % 150) < 12 && _tick > 60);
    frame.Set(slug::eAction::Shot, (_tick % 10) < 3);
    frame.Set(slug::eAction::Up, (_tick % 400) >= 380);
    frame.Set(slug::eAction::Bomb, (_tick % 420) == 300);
    return frame;
}

}   // namespace

int main(
    int    _argc,
    char** _argv)
{
#ifdef SLUG_ASSET_DIR
    slug::SetAssetRootFallback(SLUG_ASSET_DIR);
#endif
    slug::FilePath replayPath;
    slug::String   stateName;
    uint64_t       maxTicks        = 0;
    bool           bExpectFinished = false;

    for (int i = 1; i < _argc; ++i)
    {
        const slug::String arg      = _argv[i];
        const bool         bHasNext = i + 1 < _argc;
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
        else if (arg == "--gen-replay" && _argc > i + 4)
        {
            // --gen-replay <out> <state> <ticks> <flipPeriod>: 합성 입력 리플레이를 만든다 (회귀 기준선 생성용)
            slug::ReplayData data;
            data.seed      = 12345;
            data.stateName = _argv[i + 2];
            const uint64_t ticks      = std::strtoull(_argv[i + 3], nullptr, 10);
            const uint64_t flipPeriod = std::strtoull(_argv[i + 4], nullptr, 10);
            for (uint64_t tick = 0; tick < ticks; ++tick)
            {
                data.frames.push_back(MakeScriptedFrame(tick, flipPeriod));
            }
            if (const slug::Result<void> saved = slug::SaveReplay(_argv[i + 1], data); !saved)
            {
                SLUG_LOG_ERROR("리플레이 저장 실패: {}", saved.GetError().MakeMessage());
                return 1;
            }
            std::printf("wrote %s frames=%llu\n", _argv[i + 1], static_cast<unsigned long long>(ticks));
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
    if (slug::Result<slug::AnimationLibrary> library = slug::AnimationLibrary::LoadFromDirectory(slug::FindAssetRoot()))
    {
        anims = library.Take();
    }
    else
    {
        SLUG_LOG_WARN("에셋 없음: 애니메이션 없이 진행");
    }

    slug::RngMt19937  rng { replay.seed };
    slug::GameContext ctx;
    ctx.pAnims    = &anims;
    ctx.pRng      = &rng;
    ctx.assetRoot = slug::FindAssetRoot();

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

    uint64_t hash = 14695981039346656037ull;
    hash          = FnvMix(hash, runner.GetTickCount());
    hash          = FnvMix(hash, runner.IsFinished() ? 1ull : 0ull);
    hash          = FnvMix(hash, runner.GetState() != nullptr ? runner.GetState()->DebugHash() : 0ull);
    std::printf("ticks=%llu finished=%d hash=%016llx\n", static_cast<unsigned long long>(runner.GetTickCount()), runner.IsFinished() ? 1 : 0, static_cast<unsigned long long>(hash));
    if (bExpectFinished && !runner.IsFinished())
    {
        return 2;
    }
    return 0;
}
