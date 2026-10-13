
#include <gtest/gtest.h>

#include "Base/Handle.h"
#include "Base/SlugError.h"
#include "Render/Animation.h"
#include "World/Constants.h"

namespace
{
[[nodiscard]] AnimClip MakeClip_(
    const char* _name,
    const int   _frameCount,
    const float _frameDuration,
    const bool  _bLoop,
    const int   _loopStart = 0)
{
    AnimClip clip;
    clip.name          = _name;
    clip.bLoop         = _bLoop;
    clip.loopStart     = _loopStart;
    clip.frameDuration = _frameDuration;
    for (int i = 0; i < _frameCount; ++i)
    {
        AnimFrame frame;
        frame.atlas  = 0;
        frame.x      = i * 10;
        frame.y      = 0;
        frame.w      = 10;
        frame.h      = 20;
        frame.pivotX = 5;
        frame.pivotY = 20;
        clip.frames.push_back(frame);
    }
    return clip;
}
}   // namespace

constexpr const char* kSampleJson = R"({
        "version": 1,
        "animations": {
            "Player_Stand_Idle_Lower_Right": {
                "loop": true, "loopStart": 1, "frameDuration": 0.06, "cellW": 85, "cellH": 58,
                "frames": [
                    { "atlas": 0, "x": 10, "y": 20, "w": 40, "h": 52, "pivotX": 21, "pivotY": 50 },
                    { "atlas": 1, "x": 60, "y": 20, "w": 41, "h": 52, "pivotX": 22, "pivotY": 50 },
                    { "atlas": 1, "x": 0,  "y": 0,  "w": 0,  "h": 0,  "pivotX": 0,  "pivotY": 0 }
                ]
            },
            "Player_Dead": {
                "frameDuration": 0.1,
                "frames": [ { "atlas": 0, "x": 0, "y": 0, "w": 8, "h": 8, "pivotX": 4, "pivotY": 8 } ]
            }
        }
    })";

namespace
{
[[nodiscard]] int ExpectedLoopFrame_(
    const int64_t _ticks,
    const int64_t _durationUnits,
    const int     _frameCount)
{
    const int64_t frame = (_ticks * (kAnimTimeUnitsPerSec / 60)) / _durationUnits;
    return static_cast<int>(frame % _frameCount);
}
}   // namespace

TEST(AnimationLibrary, ParsesJsonWithFramesAndPivots)
{
    const Result<AnimationLibrary> result = AnimationLibrary::LoadFromJson(kSampleJson);
    ASSERT_TRUE(result.HasValue());
    const AnimationLibrary& lib = result.GetValue();
    EXPECT_EQ(lib.GetClipCount(), 2u);

    const AnimId id = lib.Find("Player_Stand_Idle_Lower_Right");
    ASSERT_FALSE(id.IsNull());
    const AnimClip& clip = lib.Get(id);
    EXPECT_EQ(clip.name, "Player_Stand_Idle_Lower_Right");
    EXPECT_TRUE(clip.bLoop);
    EXPECT_EQ(clip.loopStart, 1);
    EXPECT_FLOAT_EQ(clip.frameDuration, 0.06f);
    EXPECT_EQ(clip.cellW, 85);
    EXPECT_EQ(clip.cellH, 58);
    ASSERT_EQ(clip.GetFrameCount(), 3);
    EXPECT_EQ(clip.frames[0].x, 10);
    EXPECT_EQ(clip.frames[0].w, 40);
    EXPECT_EQ(clip.frames[0].pivotX, 21);
    EXPECT_EQ(clip.frames[0].pivotY, 50);
    EXPECT_EQ(clip.frames[1].atlas, 1);
    EXPECT_TRUE(clip.frames[2].IsEmpty());
    EXPECT_FALSE(clip.frames[0].IsEmpty());

    const AnimClip* pDead = lib.FindClip("Player_Dead");
    ASSERT_NE(pDead, nullptr);
    EXPECT_FALSE(pDead->bLoop);
    EXPECT_EQ(pDead->loopStart, 0);
}

TEST(AnimationLibrary, FindIsStableAndNullForUnknownNames)
{
    const Result<AnimationLibrary> result = AnimationLibrary::LoadFromJson(kSampleJson);
    ASSERT_TRUE(result.HasValue());
    const AnimationLibrary& lib = result.GetValue();

    EXPECT_EQ(lib.Find("Player_Dead"), lib.Find("Player_Dead"));
    EXPECT_NE(lib.Find("Player_Dead"), lib.Find("Player_Stand_Idle_Lower_Right"));
    EXPECT_TRUE(lib.Find("Nope").IsNull());
    EXPECT_EQ(lib.FindClip("Nope"), nullptr);
    EXPECT_EQ(lib.GetOrNull(kNullHandle), nullptr);
    EXPECT_FALSE(lib.IsValid(AnimId { 99, 0 }));
}

TEST(AnimationLibrary, IdsFollowFileOrderSoLoadsAreDeterministic)
{
    const Result<AnimationLibrary> a = AnimationLibrary::LoadFromJson(kSampleJson);
    const Result<AnimationLibrary> b = AnimationLibrary::LoadFromJson(kSampleJson);
    ASSERT_TRUE(a.HasValue() && b.HasValue());
    EXPECT_EQ(a->Find("Player_Stand_Idle_Lower_Right").GetIndex(), 0u);
    EXPECT_EQ(a->Find("Player_Dead").GetIndex(), 1u);
    EXPECT_EQ(a->Find("Player_Dead"), b->Find("Player_Dead"));
}

TEST(AnimationLibrary, RejectsMalformedData)
{
    EXPECT_TRUE(AnimationLibrary::LoadFromJson("{ not json").HasError());
    EXPECT_TRUE(AnimationLibrary::LoadFromJson("[]").HasError());
    EXPECT_TRUE(AnimationLibrary::LoadFromJson(R"({"version":1})").HasError());

    EXPECT_TRUE(AnimationLibrary::LoadFromJson(R"({"animations":{"A":{"frameDuration":0.1}}})").HasError());
    EXPECT_TRUE(AnimationLibrary::LoadFromJson(R"({"animations":{"A":{"frames":[{"atlas":0,"x":0,"y":0,"w":1,"h":1,"pivotX":0,"pivotY":0}]}}})").HasError());
    EXPECT_TRUE(AnimationLibrary::LoadFromJson(R"({"animations":{"A":{"frameDuration":0,"frames":[{"atlas":0,"x":0,"y":0,"w":1,"h":1,"pivotX":0,"pivotY":0}]}}})").HasError());
    EXPECT_TRUE(AnimationLibrary::LoadFromJson(R"({"animations":{"A":{"frameDuration":0.1,"frames":[{"atlas":0,"x":0}]}}})").HasError());
    EXPECT_TRUE(AnimationLibrary::LoadFromJson(R"({"animations":{"A":{"frameDuration":0.1,"frames":[]}}})").HasError());
}

TEST(AnimationLibrary, ClampsOutOfRangeLoopStart)
{
    const char*                    pJson = R"({"animations":{"A":{"loop":true,"loopStart":3,"frameDuration":0.1,"frames":[{"atlas":0,"x":0,"y":0,"w":1,"h":1,"pivotX":0,"pivotY":0}]}}})";
    const Result<AnimationLibrary> lib   = AnimationLibrary::LoadFromJson(pJson);
    ASSERT_TRUE(lib.HasValue());
    EXPECT_EQ(lib->FindClip("A")->loopStart, 0);
}

TEST(AnimationLibrary, AcceptsIntegerValuedFloatsAndFloatValuedIntegers)
{
    const char*                    pJson = R"({"animations":{"A":{"frameDuration":1,"frames":[{"atlas":0,"x":1.0,"y":2,"w":3,"h":4,"pivotX":5,"pivotY":6}]}}})";
    const Result<AnimationLibrary> lib   = AnimationLibrary::LoadFromJson(pJson);
    ASSERT_TRUE(lib.HasValue());
    EXPECT_FLOAT_EQ(lib->FindClip("A")->frameDuration, 1.f);
    EXPECT_EQ(lib->FindClip("A")->frames[0].x, 1);
}

TEST(AnimationLibrary, LoadFromDirectoryReportsMissingManifest)
{
    EXPECT_TRUE(AnimationLibrary::LoadFromDirectory("this/path/does/not/exist").HasError());
}

TEST(AnimationLibrary, AddClipReplacesSameNameKeepingId)
{
    AnimationLibrary lib;
    const AnimId     a = lib.AddClip(MakeClip_("Walk", 4, 0.1f, true));
    const AnimId     b = lib.AddClip(MakeClip_("Run", 6, 0.05f, true));
    EXPECT_NE(a, b);

    const AnimId again = lib.AddClip(MakeClip_("Walk", 2, 0.2f, false));
    EXPECT_EQ(again, a);
    EXPECT_EQ(lib.Get(a).GetFrameCount(), 2);
    EXPECT_EQ(lib.GetClipCount(), 2u);
}

TEST(AnimPlayer, LoopFrameIndexIsExactAtSixtyHertzForSixTickFrames)
{
    AnimationLibrary lib;
    const AnimId     id = lib.AddClip(MakeClip_("A", 4, 0.1f, true));

    AnimPlayer player;
    player.Play(id);
    for (int tick = 1; tick <= 3600; ++tick)
    {
        (void)player.Update(lib, kStepSec);
        ASSERT_EQ(player.GetFrame(), (tick / 6) % 4) << "tick " << tick;
    }
}

TEST(AnimPlayer, FrameIndexMatchesExactRationalForNonIntegerTickDurations)
{
    const ARRAY<float, 6> durations = { 0.06f, 0.05f, 0.07f, 0.04f, 0.12f, 0.03f };
    for (const float duration: durations)
    {
        AnimationLibrary lib;
        const AnimId     id = lib.AddClip(MakeClip_("A", 5, duration, true));
        const int64_t    us = lib.Get(id).GetFrameDurationUnits();

        AnimPlayer player;
        player.Play(id);
        for (int tick = 1; tick <= 1800; ++tick)
        {
            (void)player.Update(lib, kStepSec);
            ASSERT_EQ(player.GetFrame(), ExpectedLoopFrame_(tick, us, 5)) << "duration " << duration << " tick " << tick;
        }
    }
}

TEST(AnimPlayer, NoDriftAfterAnHourOfFixedSteps)
{
    AnimationLibrary lib;
    const AnimId     id = lib.AddClip(MakeClip_("A", 4, 0.1f, true));
    AnimPlayer       player;
    player.Play(id);

    const int ticks = 60 * 60 * 60;
    for (int i = 0; i < ticks; ++i)
    {
        (void)player.Update(lib, kStepSec);
    }
    EXPECT_EQ(player.GetFrame(), (ticks / 6) % 4);
    EXPECT_LT(player.GetTimeUnits(), lib.Get(id).GetTotalDurationUnits());
}

TEST(AnimPlayer, UpdateReportsOnlyFrameChanges)
{
    AnimationLibrary lib;
    const AnimId     id = lib.AddClip(MakeClip_("A", 3, 0.1f, true));
    AnimPlayer       player;
    player.Play(id);

    int changes = 0;
    for (int tick = 0; tick < 18; ++tick)
    {
        changes += player.Update(lib, kStepSec) ? 1 : 0;
    }
    EXPECT_EQ(changes, 3);
}

TEST(AnimPlayer, LoopJumpsToLoopStartNotZero)
{
    AnimationLibrary lib;
    const AnimId     id = lib.AddClip(MakeClip_("Intro", 5, 0.1f, true, 2));
    AnimPlayer       player;
    player.Play(id);

    for (int i = 0; i < 30; ++i)
    {
        (void)player.Update(lib, kStepSec);
    }
    EXPECT_EQ(player.GetFrame(), 2);
    for (int frame = 3; frame <= 4; ++frame)
    {
        for (int i = 0; i < 6; ++i)
        {
            (void)player.Update(lib, kStepSec);
        }
        EXPECT_EQ(player.GetFrame(), frame);
    }
    for (int i = 0; i < 6; ++i)
    {
        (void)player.Update(lib, kStepSec);
    }
    EXPECT_EQ(player.GetFrame(), 2);
    EXPECT_FALSE(player.IsFinished());
}

TEST(AnimPlayer, NonLoopStopsOnLastFrameAndFinishes)
{
    AnimationLibrary lib;
    const AnimId     id = lib.AddClip(MakeClip_("Die", 4, 0.06f, false));
    AnimPlayer       player;
    player.Play(id);

    for (int tick = 1; tick <= 14; ++tick)
    {
        (void)player.Update(lib, kStepSec);
        ASSERT_FALSE(player.IsFinished()) << "tick " << tick;
    }
    EXPECT_EQ(player.GetFrame(), 3);

    (void)player.Update(lib, kStepSec);
    EXPECT_TRUE(player.IsFinished());
    EXPECT_EQ(player.GetFrame(), 3);

    for (int i = 0; i < 100; ++i)
    {
        EXPECT_FALSE(player.Update(lib, kStepSec));
    }
    EXPECT_EQ(player.GetFrame(), 3);
    EXPECT_TRUE(player.IsFinished());
}

TEST(AnimPlayer, PlayRestartsOnlyWhenAskedOrClipChanges)
{
    AnimationLibrary lib;
    const AnimId     a = lib.AddClip(MakeClip_("A", 4, 0.1f, true));
    const AnimId     b = lib.AddClip(MakeClip_("B", 4, 0.1f, true));

    AnimPlayer player;
    player.Play(a);
    for (int i = 0; i < 7; ++i)
    {
        (void)player.Update(lib, kStepSec);
    }
    ASSERT_EQ(player.GetFrame(), 1);

    player.Play(a);
    EXPECT_EQ(player.GetFrame(), 1);

    player.Play(a, true);
    EXPECT_EQ(player.GetFrame(), 0);
    EXPECT_EQ(player.GetTimeUnits(), 0);

    for (int i = 0; i < 7; ++i)
    {
        (void)player.Update(lib, kStepSec);
    }
    player.Play(b);
    EXPECT_EQ(player.GetClip(), b);
    EXPECT_EQ(player.GetFrame(), 0);
}

TEST(AnimPlayer, FinishedFlagClearsWhenRestarted)
{
    AnimationLibrary lib;
    const AnimId     id = lib.AddClip(MakeClip_("Once", 2, 0.05f, false));
    AnimPlayer       player;
    player.Play(id);
    for (int i = 0; i < 20; ++i)
    {
        (void)player.Update(lib, kStepSec);
    }
    ASSERT_TRUE(player.IsFinished());
    player.Play(id, true);
    EXPECT_FALSE(player.IsFinished());
    EXPECT_EQ(player.GetFrame(), 0);
}

TEST(AnimPlayer, SpeedScaleStretchesTime)
{
    AnimationLibrary lib;
    const AnimId     id = lib.AddClip(MakeClip_("A", 8, 0.1f, true));
    AnimPlayer       fast;
    fast.Play(id);
    for (int i = 0; i < 6; ++i)
    {
        (void)fast.Update(lib, kStepSec, 2.f);
    }
    EXPECT_EQ(fast.GetFrame(), 2);
}

TEST(AnimPlayer, UpdateWithoutClipIsHarmless)
{
    AnimationLibrary lib;
    AnimPlayer       player;
    EXPECT_FALSE(player.Update(lib, kStepSec));
    EXPECT_EQ(player.GetFrame(), 0);
    EXPECT_EQ(player.GetFrameData(lib), nullptr);
}

TEST(AnimPlayer, SetNormalizedTimeMapsRatioToFrame)
{
    AnimationLibrary lib;
    const AnimId     id   = lib.AddClip(MakeClip_("Run", 8, 0.1f, true));
    const AnimClip&  clip = lib.Get(id);

    AnimPlayer player;
    player.Play(id);
    player.SetNormalizedTime(clip, 0.5f);
    EXPECT_EQ(player.GetFrame(), 4);
    EXPECT_NEAR(player.GetNormalizedTime(clip), 0.5f, 1e-5f);

    player.SetNormalizedTime(clip, 1.25f);
    EXPECT_EQ(player.GetFrame(), 2);

    player.SetNormalizedTime(clip, 0.f);
    EXPECT_EQ(player.GetFrame(), 0);
}

TEST(AnimPlayer, SyncPhaseKeepsUpperAndLowerBodyInStep)
{
    AnimationLibrary lib;
    const AnimId     lower     = lib.AddClip(MakeClip_("Lower", 6, 0.1f, true));
    const AnimId     upper     = lib.AddClip(MakeClip_("Upper", 12, 0.05f, true));
    const AnimClip&  lowerClip = lib.Get(lower);
    const AnimClip&  upperClip = lib.Get(upper);

    AnimPlayer lowerPlayer;
    AnimPlayer upperPlayer;
    lowerPlayer.Play(lower);
    upperPlayer.Play(upper);

    for (int tick = 1; tick <= 500; ++tick)
    {
        (void)lowerPlayer.Update(lib, kStepSec);
        upperPlayer.SyncPhase(upperClip, lowerPlayer, lowerClip);
        ASSERT_EQ(upperPlayer.GetFrame() / 2, lowerPlayer.GetFrame()) << "tick " << tick;
        ASSERT_NEAR(upperPlayer.GetNormalizedTime(upperClip), lowerPlayer.GetNormalizedTime(lowerClip), 1e-5f);
    }
}

TEST(AnimPlayer, SyncPhaseWithSameFrameCountGivesIdenticalFrames)
{
    AnimationLibrary lib;
    const AnimId     a = lib.AddClip(MakeClip_("A", 6, 0.1f, true));
    const AnimId     b = lib.AddClip(MakeClip_("B", 6, 0.1f, true));

    AnimPlayer pa;
    AnimPlayer pb;
    pa.Play(a);
    pb.Play(b);
    for (int tick = 0; tick < 1000; ++tick)
    {
        (void)pa.Update(lib, kStepSec);
        pb.SyncPhase(lib.Get(b), pa, lib.Get(a));
        ASSERT_EQ(pa.GetFrame(), pb.GetFrame());
    }
}

TEST(AnimPlayer, GetFrameDataReturnsCurrentFrame)
{
    AnimationLibrary lib;
    const AnimId     id = lib.AddClip(MakeClip_("A", 3, 0.1f, true));
    AnimPlayer       player;
    player.Play(id);
    for (int i = 0; i < 6; ++i)
    {
        (void)player.Update(lib, kStepSec);
    }
    const AnimFrame* pFrame = player.GetFrameData(lib);
    ASSERT_NE(pFrame, nullptr);
    EXPECT_EQ(pFrame->x, 10);
}
