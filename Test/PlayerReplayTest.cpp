// 플레이어 리플레이 테스트: 녹화된 입력을 재생했을 때 결과가 비트 단위로 같은지 검증
#include <filesystem>

#include "Base/SlugError.h"
#include "Base/Types.h"
#include "Core/AssetPath.h"
#include "Core/Replay.h"
#include "Game/StateFactory.h"
#include "PlayerTestUtil.h"

namespace
{
[[nodiscard]] FilePath RepoRoot_()
{
    return FindAssetRoot().parent_path();
}
}   // namespace

TEST(PlayerReplay, RegressionReplayPlaysTheSameWayTwiceAndHitsTheMilestones)
{
    const FilePath     path   = RepoRoot_() / "Test" / "Replays" / "sandbox_regression.rep";
    Result<ReplayData> loaded = LoadReplay(path);
    ASSERT_TRUE(loaded.HasValue()) << "회귀 리플레이를 읽지 못했습니다: " << path.string();

    if (!std::filesystem::exists(FindAssetRoot() / "Ground" / "Stage0_0.png"))
    {
        GTEST_SKIP() << "Assets 없음";
    }

    const ReplayData replay = loaded.Take();
    EXPECT_EQ(replay.stateName, "sandbox");

    ARRAY<uint64_t, 2> hashes = {};
    for (int run = 0; run < 2; ++run)
    {
        RngMt19937  rng { replay.seed };
        GameContext ctx;
        ctx.pRng      = &rng;
        ctx.assetRoot = FindAssetRoot();

        SandboxState* pState = new SandboxState;
        SimRunner     runner { ctx, Scoped<IGameState> { pState } };
        runner.Run(replay.frames);

        hashes[run]              = pState->HashState();
        const PlayerStats& stats = pState->GetPlayer().GetStats();
        EXPECT_GE(stats.knifeSlashes, 1);   // 근접 보병을 칼로
        EXPECT_GE(stats.shotsFired, 1);
        EXPECT_GE(stats.jumps, 1);
        EXPECT_GE(stats.bombsThrown, 1);
        EXPECT_GE(pState->GetTargets().GetKillCount(), 1);
        EXPECT_GT(pState->GetPlayer().GetScore(), 0);
        EXPECT_EQ(runner.GetTickCount(), replay.frames.size());
    }
    EXPECT_EQ(hashes[0], hashes[1]);
}

TEST(PlayerReplay, HeadlessReplayThroughTheStateFactoryUsesTheSamePath)
{
    RngMt19937  rng { uint64_t { 1 } };
    GameContext ctx;
    ctx.pRng = &rng;
    SimRunner  runner { ctx, CreateState("sandbox") };
    InputFrame right;
    right.Set(eAction::Right, true);
    right.Set(eAction::Shot, true);
    for (int i = 0; i < 600; ++i)
    {
        runner.Step(right);
    }
    EXPECT_EQ(runner.GetTickCount(), 600u);
}
