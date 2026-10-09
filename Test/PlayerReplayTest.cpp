#include <filesystem>
#include "Base/Types.h"
#include "Base/Result.h"
#include "Base/Random.h"

#include "PlayerTestUtil.h"

#include "Core/Replay.h"
#include "Game/StateFactory.h"

using namespace slugtest;

// ===========================================
//  리플레이 스크립트
//   사람이 손으로 녹화하는 대신 코드로 입력 타임라인을 만든다. 같은 스크립트가
//    - Test/Replays/*.rep  (회귀 테스트)
//    - Captures/Visual/*.rep  (Slug.exe --replay ... --capture-ticks 로 눈으로 확인)
//   로 저장된다.
// ===========================================

namespace
{
class Script
{
public:
    explicit Script(const int _ticks)
        : m_frames(static_cast<size_t>(_ticks))
    {
    }

    // [from, to) 틱 동안 눌림
    Script& Hold(const eAction _action, const int _from, const int _to)
    {
        for (int t = _from; t < _to && t < static_cast<int>(m_frames.size()); ++t)
        {
            m_frames[static_cast<size_t>(t)].Set(_action, true);
        }
        return *this;
    }

    // 한 틱만 누름
    Script& Tap(const eAction _action, const int _tick)
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
    slug::Vector<InputFrame> m_frames;
};

slug::FilePath RepoRoot()
{
    return slug::FilePath { SLUG_ASSET_DIR }.parent_path();
}

void Write(
    const slug::FilePath& _path,
    const Script&        _script)
{
    std::filesystem::create_directories(_path.parent_path());
    ASSERT_TRUE(static_cast<bool>(SaveReplay(_path, _script.Build()))) << _path.string();
}

// 회귀용: 달리기 → 칼로 보병 처치 → 사격 → 점프 → 폭탄 → 앉기
Script MakeRegressionScript()
{
    Script s { 900 };
    s.Hold(eAction::Right, 0, 50);      // 보병(330) 앞까지
    s.Hold(eAction::Shot, 50, 64);      // 칼 자동 발동
    s.Hold(eAction::Right, 64, 88);     // 두 번째 보병(400)
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
}   // namespace

TEST(PlayerReplay, WriteRegressionAndVisualScripts)
{
    Write(RepoRoot() / "Test" / "Replays" / "sandbox_regression.rep", MakeRegressionScript());

    const slug::FilePath visual = RepoRoot() / "Captures" / "Visual";

    // 달리기: 연속 프레임으로 떨림 확인
    Write(visual / "run.rep", Script { 200 }.Hold(eAction::Right, 10, 200));
    Write(visual / "runleft.rep", Script { 200 }.Hold(eAction::Right, 10, 60).Hold(eAction::Left, 60, 200));
    // 점프
    Write(visual / "jump.rep", Script { 120 }.Hold(eAction::Right, 10, 200).Hold(eAction::Jump, 20, 60));
    Write(visual / "jumpup.rep", Script { 120 }.Hold(eAction::Jump, 20, 70));
    // 앉기 / 앉아 이동
    Write(visual / "crouch.rep", Script { 120 }.Hold(eAction::Down, 10, 90).Hold(eAction::Right, 50, 90));
    // 사격: 앞 / 위 / 대각 / 앉아서 / 공중 아래
    Write(visual / "shoot.rep", Script { 200 }.Tap(eAction::Shot, 20).Hold(eAction::Up, 60, 100).Tap(eAction::Shot, 70).Hold(eAction::Up, 120, 160).Hold(eAction::Right, 120, 160).Tap(eAction::Shot, 130));
    Write(visual / "shootcrouch.rep", Script { 120 }.Hold(eAction::Down, 10, 80).Tap(eAction::Shot, 30));
    Write(visual / "shootair.rep", Script { 140 }.Hold(eAction::Jump, 10, 50).Hold(eAction::Down, 30, 60).Tap(eAction::Shot, 35).Tap(eAction::Shot, 18));
    // 칼: 첫 보병(330) 앞까지 달려가 휘두른다
    Write(visual / "knife.rep", Script { 200 }.Hold(eAction::Right, 5, 100).Hold(eAction::Shot, 100, 140));
    // 폭탄
    Write(visual / "bomb.rep", Script { 160 }.Tap(eAction::Bomb, 20));
    // 사망: 사수(1300) 가까이 달린다
    Write(visual / "death.rep", Script { 700 }.Hold(eAction::Right, 5, 700));
    // HMG/기본 연사
    Write(visual / "hold.rep", Script { 160 }.Hold(eAction::Shot, 20, 140));
}

TEST(PlayerReplay, RegressionReplayPlaysTheSameWayTwiceAndHitsTheMilestones)
{
    const slug::FilePath     path   = RepoRoot() / "Test" / "Replays" / "sandbox_regression.rep";
    slug::Result<ReplayData> loaded = LoadReplay(path);
    ASSERT_TRUE(loaded.HasValue()) << "먼저 PlayerReplay.WriteRegressionAndVisualScripts 가 실행되어야 합니다: " << path.string();

    if (!std::filesystem::exists(slug::FilePath { SLUG_ASSET_DIR } / "Baked" / "Ground" / "Stage0_0.bin"))
    {
        GTEST_SKIP() << "Assets/Baked 없음: Scripts/Bake.ps1 로 생성하세요";
    }

    const ReplayData replay = loaded.Take();
    EXPECT_EQ(replay.stateName, "sandbox");

    uint64_t hashes[2] = {};
    for (int run = 0; run < 2; ++run)
    {
        slug::RngMt19937 rng { replay.seed };
        GameContext     ctx;
        ctx.pRng      = &rng;
        ctx.assetRoot = slug::FilePath { SLUG_ASSET_DIR };

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
    // 상태 이름으로 만든 샌드박스(기본 구성)를 SimRunner 로 돌려도 안전하게 끝까지 간다 (애셋 유무와 무관)
    slug::RngMt19937 rng { uint64_t { 1 } };
    GameContext     ctx;
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
