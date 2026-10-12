// 리플레이/SimRunner 테스트: 리플레이 직렬화와 SimRunner 의 고정 틱 진행, 헤드리스 컨텍스트 검증
#include <gtest/gtest.h>

#include "Base/Math.h"
#include "Base/Result.h"
#include "Base/Types.h"
#include "Core/Replay.h"
#include "Game/DemoState.h"
#include "Game/SimRunner.h"
#include "Game/StateFactory.h"


[[nodiscard]] static Vector<InputFrame> MakeScript_()
{
    Vector<InputFrame> frames;
    for (int i = 0; i < 240; ++i)
    {
        InputFrame frame;
        frame.Set(eAction::Right, i < 120);
        frame.Set(eAction::Down, i >= 60 && i < 180);
        frame.Set(eAction::Left, i >= 180);
        frames.push_back(frame);
    }
    return frames;
}

[[nodiscard]] static Vec2 RunDemo_(
    const Vector<InputFrame>& _frames)
{
    RngMt19937 rng { uint64_t { 7 } };
    GameContext      ctx;
    ctx.pRng = &rng;

    DemoState* pDemo = new DemoState;
    SimRunner  runner { ctx, std::unique_ptr<IGameState> { pDemo } };
    runner.Run(_frames);
    return pDemo->GetPosition();
}

TEST(Replay, SerializeRoundTrip)
{
    ReplayData replay;
    replay.seed      = 123456789;
    replay.stateName = "stage0";
    replay.frames    = MakeScript_();

    const Vector<uint8_t> bytes  = SerializeReplay(replay);
    Result<ReplayData>    loaded = DeserializeReplay(bytes);
    ASSERT_TRUE(loaded.HasValue());

    const ReplayData& back = loaded.GetValue();
    EXPECT_EQ(back.seed, replay.seed);
    EXPECT_EQ(back.stateName, replay.stateName);
    ASSERT_EQ(back.frames.size(), replay.frames.size());
    for (size_t i = 0; i < back.frames.size(); ++i)
    {
        EXPECT_TRUE(back.frames[i] == replay.frames[i]) << "frame " << i;
    }
}

TEST(Replay, RejectsBadMagicAndTruncation)
{
    ReplayData replay;
    replay.frames               = MakeScript_();
    Vector<uint8_t> bytes = SerializeReplay(replay);

    Vector<uint8_t> badMagic = bytes;
    badMagic[0]                    = 'X';
    EXPECT_FALSE(DeserializeReplay(badMagic).HasValue());

    Vector<uint8_t> truncated(bytes.begin(), bytes.begin() + static_cast<std::ptrdiff_t>(bytes.size() / 2));
    EXPECT_FALSE(DeserializeReplay(truncated).HasValue());

    EXPECT_FALSE(DeserializeReplay({}).HasValue());
}

TEST(SimRunner, SameInputsGiveIdenticalResult)
{
    const Vector<InputFrame> script = MakeScript_();
    const Vec2                     a      = RunDemo_(script);
    const Vec2                     b      = RunDemo_(script);
    EXPECT_FLOAT_EQ(a.x, b.x);
    EXPECT_FLOAT_EQ(a.y, b.y);
    EXPECT_FLOAT_EQ(a.x, 320.f + 120.f);
    EXPECT_FLOAT_EQ(a.y, 224.f + 240.f);
}

TEST(SimRunner, StateFactoryKnowsDemoOnly)
{
    EXPECT_TRUE(static_cast<bool>(CreateState("demo")));
    EXPECT_FALSE(static_cast<bool>(CreateState("does-not-exist")));
}

TEST(SimRunner, HeadlessContextHasNoRenderer)
{
    GameContext ctx;
    EXPECT_TRUE(ctx.IsHeadless());
    SimRunner runner { ctx, CreateState("demo") };
    runner.Step({});
    runner.Render(0.f);   // 헤드리스에서는 렌더가 무시되어야 한다 (크래시 없음)
    EXPECT_EQ(runner.GetTickCount(), 1u);
    EXPECT_FALSE(runner.IsFinished());

    InputFrame bomb;
    bomb.Set(eAction::Bomb, true);
    runner.Step(bomb);
    EXPECT_TRUE(runner.IsFinished());
}
