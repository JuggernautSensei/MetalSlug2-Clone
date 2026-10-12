// 장식 월드의 스폰 파라미터 해석, 시퀀스 이동/반전/루프와 레이어 마스크 정리를 검증한다.

#include <gtest/gtest.h>

#include "Base/SlugError.h"
#include "Game/Props/DecorWorld.h"
#include "Game/Stage/StageData.h"
#include "Render/Animation.h"
#include "World/Constants.h"

// 프레임 2장짜리 클립 A/B 와 1장짜리 T
constexpr const char* kDecorJson = R"({
    "version": 1,
    "animations": {
        "A": { "loop": true, "loopStart": 0, "frameDuration": 0.1, "cellW": 10, "cellH": 10,
               "frames": [ { "atlas": 0, "x": 0, "y": 0, "w": 10, "h": 10, "pivotX": 5, "pivotY": 10 },
                           { "atlas": 0, "x": 10, "y": 0, "w": 10, "h": 10, "pivotX": 5, "pivotY": 10 } ] },
        "B": { "loop": true, "loopStart": 0, "frameDuration": 0.1, "cellW": 10, "cellH": 10,
               "frames": [ { "atlas": 0, "x": 20, "y": 0, "w": 10, "h": 10, "pivotX": 5, "pivotY": 10 },
                           { "atlas": 0, "x": 30, "y": 0, "w": 10, "h": 10, "pivotX": 5, "pivotY": 10 } ] },
        "T": { "loop": true, "loopStart": 0, "frameDuration": 0.1, "cellW": 10, "cellH": 10,
               "frames": [ { "atlas": 0, "x": 40, "y": 0, "w": 10, "h": 10, "pivotX": 5, "pivotY": 10 } ] }
    }
})";

namespace
{
[[nodiscard]] ParamValue Step_(
    const char*  _clip,
    const double _seconds,
    const double _vx,
    const bool   _bFlip = false)
{
    ParamValue step = ParamValue::MakeObject();
    step.Set("clip", ParamValue::MakeString(_clip));
    step.Set("seconds", ParamValue::MakeNumber(_seconds));
    ParamValue velocity = ParamValue::MakeArray();
    velocity.PushBack(ParamValue::MakeNumber(_vx));
    velocity.PushBack(ParamValue::MakeNumber(0.0));
    step.Set("velocity", velocity);
    if (_bFlip)
    {
        step.Set("flip", ParamValue::MakeBool(true));
    }
    return step;
}

// A(0.5s, +60px/s) -> B(0.25s, 정지) -> A(0.5s, -60px/s, 좌우 반전)
[[nodiscard]] ParamValue MakeWalkParams_(
    const bool _bLoop)
{
    ParamValue sequence = ParamValue::MakeArray();
    sequence.PushBack(Step_("A", 0.5, 60.0));
    sequence.PushBack(Step_("B", 0.25, 0.0));
    sequence.PushBack(Step_("A", 0.5, -60.0, true));

    ParamValue params = ParamValue::MakeObject();
    params.Set("sequence", sequence);
    params.Set("loop", ParamValue::MakeBool(_bLoop));
    return params;
}

void RunTicks_(
    DecorWorld&             _world,
    const AnimationLibrary& _lib,
    const int               _ticks)
{
    for (int i = 0; i < _ticks; ++i)
    {
        _world.Tick(_lib, kStepSec);
    }
}
}   // namespace

TEST(DecorSpawnParams, ParsesVelocitySequenceAndFlags)
{
    ParamValue params = MakeWalkParams_(true);
    params.Set("velocity", ParamValue::MakeArray());

    const DecorSpawn spawn = MakeDecorSpawn("A", eStageLayer::Background, { 100.f, 50.f }, &params);
    EXPECT_EQ(spawn.clip, "A");
    EXPECT_EQ(spawn.layer, eStageLayer::Background);
    EXPECT_FLOAT_EQ(spawn.pos.x, 100.f);
    EXPECT_TRUE(spawn.bLoop);
    ASSERT_EQ(spawn.sequence.size(), 3u);
    EXPECT_EQ(spawn.sequence[1].clip, "B");
    EXPECT_FLOAT_EQ(spawn.sequence[0].seconds, 0.5f);
    EXPECT_FLOAT_EQ(spawn.sequence[0].velocity.x, 60.f);
    EXPECT_FALSE(spawn.sequence[0].bFlipX);
    EXPECT_TRUE(spawn.sequence[2].bFlipX);
}

TEST(DecorSpawnParams, ScaleDefaultsToOneAndReadsTheParamWithoutChangingOthers)
{
    EXPECT_FLOAT_EQ(MakeDecorSpawn("A", eStageLayer::FarBackground, { 0.f, 0.f }, nullptr).scale, 1.f);

    ParamValue params = ParamValue::MakeObject();
    params.Set("scale", ParamValue::MakeNumber(0.5));
    const DecorSpawn half = MakeDecorSpawn("A", eStageLayer::FarBackground, { 1.f, 2.f }, &params);
    EXPECT_FLOAT_EQ(half.scale, 0.5f);
    EXPECT_FLOAT_EQ(half.pos.x, 1.f);
    EXPECT_FALSE(half.bLoop);

    ParamValue bad = ParamValue::MakeObject();
    bad.Set("scale", ParamValue::MakeNumber(0.0));
    EXPECT_FLOAT_EQ(MakeDecorSpawn("A", eStageLayer::FarBackground, { 0.f, 0.f }, &bad).scale, 1.f);
}

TEST(DecorSpawnParams, SkipsStepsWithoutClipAndReadsVelocityWithoutSequence)
{
    ParamValue params   = ParamValue::MakeObject();
    ParamValue velocity = ParamValue::MakeArray();
    velocity.PushBack(ParamValue::MakeNumber(70.0));
    velocity.PushBack(ParamValue::MakeNumber(-50.0));
    params.Set("velocity", velocity);
    ParamValue sequence = ParamValue::MakeArray();
    ParamValue noClip   = ParamValue::MakeObject();
    noClip.Set("seconds", ParamValue::MakeNumber(1.0));
    sequence.PushBack(noClip);
    params.Set("sequence", sequence);

    const DecorSpawn spawn = MakeDecorSpawn("T", eStageLayer::FarBackground, { 0.f, 0.f }, &params);
    EXPECT_TRUE(spawn.sequence.empty());
    EXPECT_FLOAT_EQ(spawn.velocity.x, 70.f);
    EXPECT_FLOAT_EQ(spawn.velocity.y, -50.f);
}

TEST(DecorWorld, SequenceAdvancesStepsAndFinishesWithoutLoop)
{
    const Result<AnimationLibrary> lib = AnimationLibrary::LoadFromJson(kDecorJson);
    ASSERT_TRUE(lib.HasValue());

    ParamValue     params = MakeWalkParams_(false);
    DecorWorld     world;
    const uint32_t id = world.Spawn(MakeDecorSpawn("A", eStageLayer::Background, { 100.f, 50.f }, &params));

    Vec2       pos      = {};
    Vec2       velocity = {};
    StringView clip;
    bool       bFinished = false;

    RunTicks_(world, *lib, 20);   // 0.33 s: 첫 구간 (A, +60)
    ASSERT_TRUE(world.Get(id, pos, velocity, clip, bFinished));
    EXPECT_EQ(clip, "A");
    EXPECT_NEAR(pos.x, 120.f, 0.5f);
    EXPECT_FALSE(bFinished);

    RunTicks_(world, *lib, 20);   // 0.67 s: 둘째 구간 (B, 정지)
    ASSERT_TRUE(world.Get(id, pos, velocity, clip, bFinished));
    EXPECT_EQ(clip, "B");
    EXPECT_NEAR(pos.x, 129.f, 1.f);   // 구간 전환 틱은 새 속도(0)로 움직인다
    EXPECT_FLOAT_EQ(velocity.x, 0.f);

    RunTicks_(world, *lib, 20);   // 1.0 s: 셋째 구간 (A, -60, 반전) 진행 중
    ASSERT_TRUE(world.Get(id, pos, velocity, clip, bFinished));
    EXPECT_EQ(clip, "A");
    EXPECT_FLOAT_EQ(velocity.x, -60.f);
    EXPECT_FALSE(bFinished);

    RunTicks_(world, *lib, 30);   // 1.5 s: 시퀀스 끝, 제자리 정지
    ASSERT_TRUE(world.Get(id, pos, velocity, clip, bFinished));
    EXPECT_TRUE(bFinished);
    EXPECT_FLOAT_EQ(velocity.x, 0.f);
    EXPECT_NEAR(pos.x, 100.f, 1.5f);
}

TEST(DecorWorld, SequenceLoopsBackToFirstStep)
{
    const Result<AnimationLibrary> lib = AnimationLibrary::LoadFromJson(kDecorJson);
    ASSERT_TRUE(lib.HasValue());

    ParamValue     params = MakeWalkParams_(true);
    DecorWorld     world;
    const uint32_t id = world.Spawn(MakeDecorSpawn("A", eStageLayer::Background, { 100.f, 50.f }, &params));

    RunTicks_(world, *lib, 90);   // 1.5 s: 한 바퀴 돌고 첫 구간 (A, +60) 으로 복귀
    Vec2       pos      = {};
    Vec2       velocity = {};
    StringView clip;
    bool       bFinished = true;
    ASSERT_TRUE(world.Get(id, pos, velocity, clip, bFinished));
    EXPECT_FALSE(bFinished);
    EXPECT_EQ(clip, "A");
    EXPECT_FLOAT_EQ(velocity.x, 60.f);
}

TEST(DecorWorld, SingleClipMovesAtConstantVelocityAndNeverFinishes)
{
    const Result<AnimationLibrary> lib = AnimationLibrary::LoadFromJson(kDecorJson);
    ASSERT_TRUE(lib.HasValue());

    ParamValue params   = ParamValue::MakeObject();
    ParamValue velocity = ParamValue::MakeArray();
    velocity.PushBack(ParamValue::MakeNumber(70.0));
    velocity.PushBack(ParamValue::MakeNumber(-50.0));
    params.Set("velocity", velocity);

    DecorWorld     world;
    const uint32_t id = world.Spawn(MakeDecorSpawn("A", eStageLayer::FarBackground, { 100.f, 50.f }, &params));
    RunTicks_(world, *lib, 60);   // 1 s

    Vec2       pos = {};
    Vec2       vel = {};
    StringView clip;
    bool       bFinished = true;
    ASSERT_TRUE(world.Get(id, pos, vel, clip, bFinished));
    EXPECT_FALSE(bFinished);
    EXPECT_NEAR(pos.x, 170.f, 0.5f);
    EXPECT_NEAR(pos.y, 0.f, 0.5f);
    EXPECT_EQ(clip, "A");
}

TEST(DecorWorld, ClearByLayerMaskRemovesOnlyMatchingLayers)
{
    DecorWorld world;
    world.Spawn(MakeDecorSpawn("A", eStageLayer::FarBackground, { 0.f, 0.f }, nullptr));
    world.Spawn(MakeDecorSpawn("A", eStageLayer::Background, { 0.f, 0.f }, nullptr));
    ASSERT_EQ(world.GetCount(), 2u);

    world.ClearByLayerMask(LayerBit(eStageLayer::FarBackground));
    EXPECT_EQ(world.GetCount(), 1u);
}
