// 플레이어 리플레이 테스트: 녹화된 입력을 재생했을 때 결과가 비트 단위로 같은지 검증
#include <filesystem>

#include "Base/Random.h"
#include "Base/Result.h"
#include "Base/Types.h"
#include "Core/AssetPath.h"
#include "Core/Replay.h"
#include "Game/StateFactory.h"
#include "PlayerTestUtil.h"


class Script
{
public:
    explicit Script(
        const int _ticks)
        : m_frames(static_cast<size_t>(_ticks))
    {
    }

    Script& Hold(
        const eAction _action,
        const int     _from,
        const int     _to)
    {
        for (int t = _from; t < _to && t < static_cast<int>(m_frames.size()); ++t)
        {
            m_frames[static_cast<size_t>(t)].Set(_action, true);
        }
        return *this;
    }

    Script& Tap(
        const eAction _action,
        const int     _tick)
    {
        return Hold(_action, _tick, _tick + 1);
    }

    [[nodiscard]] ReplayData Build() const
    {
        ReplayData replay;
        replay.seed      = 1;
        replay.stateName = "sandbox";
        replay.frames    = m_frames;
        return replay;
    }

private:
    Vector<InputFrame> m_frames = {};
};

[[nodiscard]] static FilePath RepoRoot_()
{
    return FindAssetRoot().parent_path();
}

static void Write_(
    const FilePath& _path,
    const Script&         _script)
{
    std::filesystem::create_directories(_path.parent_path());
    ASSERT_TRUE(static_cast<bool>(SaveReplay(_path, _script.Build()))) << _path.string();
}

[[nodiscard]] static Script MakeRegressionScript_()
{
    Script s { 900 };
    s.Hold(eAction::Right, 0, 50);    // 보병(330) 앞까지
    s.Hold(eAction::Shot, 50, 64);    // 칼 자동 발동
    s.Hold(eAction::Right, 64, 88);   // 두 번째 보병(400)
    s.Tap(eAction::Shot, 90);
    s.Tap(eAction::Shot, 97);
    s.Hold(eAction::Right, 100, 165);   // 지프(640) 쪽으로
    s.Hold(eAction::Shot, 150, 210);    // 지프를 총으로
    s.Tap(eAction::Jump, 220);
    s.Hold(eAction::Jump, 250, 265);
    s.Tap(eAction::Bomb, 280);
    s.Hold(eAction::Down, 300, 325);
    s.Hold(eAction::Right, 350, 380);
    s.Tap(eAction::Bomb, 385);
    return s;
}

TEST(PlayerReplay, WriteRegressionAndVisualScripts)
{
    Write_(RepoRoot_() / "Test" / "Replays" / "sandbox_regression.rep", MakeRegressionScript_());

    const FilePath visual = RepoRoot_() / "Captures" / "Visual";

    Write_(visual / "run.rep", Script { 200 }.Hold(eAction::Right, 10, 200));
    Write_(visual / "runleft.rep", Script { 200 }.Hold(eAction::Right, 10, 60).Hold(eAction::Left, 60, 200));
    Write_(visual / "jump.rep", Script { 120 }.Hold(eAction::Right, 10, 200).Hold(eAction::Jump, 20, 60));
    Write_(visual / "jumpup.rep", Script { 120 }.Hold(eAction::Jump, 20, 70));
    Write_(visual / "crouch.rep", Script { 120 }.Hold(eAction::Down, 10, 90).Hold(eAction::Right, 50, 90));
    Write_(visual / "shoot.rep", Script { 200 }.Tap(eAction::Shot, 20).Hold(eAction::Up, 60, 100).Tap(eAction::Shot, 70).Hold(eAction::Up, 120, 160).Hold(eAction::Right, 120, 160).Tap(eAction::Shot, 130));
    Write_(visual / "shootcrouch.rep", Script { 120 }.Hold(eAction::Down, 10, 80).Tap(eAction::Shot, 30));
    Write_(visual / "shootair.rep", Script { 140 }.Hold(eAction::Jump, 10, 50).Hold(eAction::Down, 30, 60).Tap(eAction::Shot, 35).Tap(eAction::Shot, 18));
    Write_(visual / "knife.rep", Script { 200 }.Hold(eAction::Right, 5, 100).Hold(eAction::Shot, 100, 140));
    Write_(visual / "bomb.rep", Script { 160 }.Tap(eAction::Bomb, 20));
    Write_(visual / "death.rep", Script { 700 }.Hold(eAction::Right, 5, 700));
    Write_(visual / "hold.rep", Script { 160 }.Hold(eAction::Shot, 20, 140));
}

TEST(PlayerReplay, RegressionReplayPlaysTheSameWayTwiceAndHitsTheMilestones)
{
    const FilePath     path   = RepoRoot_() / "Test" / "Replays" / "sandbox_regression.rep";
    Result<ReplayData> loaded = LoadReplay(path);
    ASSERT_TRUE(loaded.HasValue()) << "먼저 PlayerReplay.WriteRegressionAndVisualScripts 가 실행되어야 합니다: " << path.string();

    if (!std::filesystem::exists(FindAssetRoot() / "Ground" / "Stage0_0.png"))
    {
        GTEST_SKIP() << "Assets 없음";
    }

    const ReplayData replay = loaded.Take();
    EXPECT_EQ(replay.stateName, "sandbox");

    ARRAY<uint64_t, 2> hashes = {};
    for (int run = 0; run < 2; ++run)
    {
        RngMt19937 rng { replay.seed };
        GameContext      ctx;
        ctx.pRng      = &rng;
        ctx.assetRoot = FindAssetRoot();

        SandboxState* pState = new SandboxState;
        SimRunner     runner { ctx, std::unique_ptr<IGameState> { pState } };
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
    RngMt19937 rng { uint64_t { 1 } };
    GameContext      ctx;
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
