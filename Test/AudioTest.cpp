#include "Core/AssetPath.h"
#include <filesystem>
#include "Base/Types.h"
#include "Base/Result.h"
#include "Base/Random.h"

#include <gtest/gtest.h>

#include "Audio/AudioMath.h"
#include "Audio/AudioSink.h"
#include "Audio/AudioSystem.h"
#include "Audio/SoundBank.h"
#include "Audio/VoiceAllocator.h"
#include "Game/GameState.h"
#include "Game/SandboxState.h"
#include "Game/SimRunner.h"
#include "PlayerTestUtil.h"
#include "TestAudio.h"

using namespace slug;

namespace
{
    constexpr const char* kBankJson = R"({
      "version": 1,
      "sounds": {
        "Stage1_BGM": { "file": "Audio/Stage1_BGM.wav", "bgm": true },
        "PlayerGunShot": { "file": "Audio/PlayerGunShot.wav", "bgm": false },
        "PlayerBomb": { "file": "Audio/PlayerBomb.wav" },
        "Coin": { "file": "Audio/Coin.wav", "bgm": false }
      }
    })";

    [[nodiscard]] SoundBank MakeBank()
    {
        slug::Result<SoundBank> bank = SoundBank::LoadFromJson(kBankJson, "root");
        EXPECT_TRUE(static_cast<bool>(bank));
        return bank.Take();
    }

    [[nodiscard]] AudioConfig MakeDryConfig()
    {
        AudioConfig config;
        config.bEnabled    = false;
        config.maxVoices   = 4;
        config.maxPerSound = 2;
        return config;
    }

    slug::FilePath BakedRoot()
    {
        return slug::FindAssetRoot() / "Baked";
    }
}

TEST(AudioBank, ParsesEntriesNamesFilesAndBgmFlag)
{
    const SoundBank bank = MakeBank();
    ASSERT_EQ(bank.GetCount(), 4u);

    const SoundEntry* pBgm = bank.Find("Stage1_BGM");
    ASSERT_NE(pBgm, nullptr);
    EXPECT_TRUE(pBgm->bBgm);
    EXPECT_EQ(pBgm->file, "Audio/Stage1_BGM.wav");

    const SoundEntry* pShot = bank.Find("PlayerGunShot");
    ASSERT_NE(pShot, nullptr);
    EXPECT_FALSE(pShot->bBgm);
}

TEST(AudioBank, MissingBgmKeyDefaultsToSfxAndPathJoinsRoot)
{
    const SoundBank   bank  = MakeBank();
    const SoundEntry* pBomb = bank.Find("PlayerBomb");
    ASSERT_NE(pBomb, nullptr);
    EXPECT_FALSE(pBomb->bBgm);
    EXPECT_EQ(pBomb->path, slug::FilePath { "root" } / "Audio/PlayerBomb.wav");
}

TEST(AudioBank, UnknownNameIsNullAndIndexMinusOne)
{
    const SoundBank bank = MakeBank();
    EXPECT_EQ(bank.Find("Nope"), nullptr);
    EXPECT_EQ(bank.FindIndex("Nope"), -1);
    EXPECT_GE(bank.FindIndex("Coin"), 0);
}

TEST(AudioBank, RejectsMalformedJsonAndMissingSections)
{
    EXPECT_FALSE(static_cast<bool>(SoundBank::LoadFromJson("{ not json")));
    EXPECT_FALSE(static_cast<bool>(SoundBank::LoadFromJson(R"({"version":1})")));
    EXPECT_FALSE(static_cast<bool>(SoundBank::LoadFromJson(R"({"sounds":{"A":{"bgm":true}}})")));
    EXPECT_FALSE(static_cast<bool>(SoundBank::LoadFromJson(R"({"sounds":{"A":5}})")));
}

TEST(AudioBank, DuplicateNameReplacesEntry)
{
    SoundBank bank;
    bank.Add({ "A", "Audio/a.wav", false, {} });
    bank.Add({ "A", "Audio/b.wav", true, {} });
    ASSERT_EQ(bank.GetCount(), 1u);
    EXPECT_EQ(bank.Find("A")->file, "Audio/b.wav");
    EXPECT_TRUE(bank.Find("A")->bBgm);
}

TEST(AudioBank, MissingDirectoryIsError)
{
    EXPECT_FALSE(static_cast<bool>(SoundBank::LoadFromDirectory("Z:/definitely/not/here")));
}

TEST(AudioBank, BakedSoundsJsonListsExistingFilesAndExtras)
{
    slug::Result<SoundBank> bank = SoundBank::LoadFromDirectory(BakedRoot());
    if (!bank)
    {
        GTEST_SKIP() << "Assets/Baked 없음";
    }
    for (const SoundEntry& entry: bank->GetEntries())
    {
        EXPECT_TRUE(std::filesystem::exists(entry.path)) << entry.name << " -> " << entry.file;
    }
    EXPECT_TRUE(bank->Find(sfx::kBgmStage) != nullptr && bank->Find(sfx::kBgmStage)->bBgm);
    EXPECT_NE(bank->Find(sfx::kPistolShot), nullptr);
    EXPECT_NE(bank->Find(sfx::kBombExplode), nullptr);
    EXPECT_NE(bank->Find(sfx::kPlayerDeath), nullptr);
    EXPECT_NE(bank->Find(sfx::kKnife1), nullptr);
    EXPECT_NE(bank->Find(sfx::kPickupCoin), nullptr);
}

TEST(AudioVoices, FillsEmptySlotsWithoutStealing)
{
    VoiceAllocator voices { 3, 3 };
    for (int i = 0; i < 3; ++i)
    {
        const VoiceAllocator::Acquired got = voices.Acquire(i);
        EXPECT_FALSE(got.bStolen);
        EXPECT_EQ(got.slot, i);
    }
    EXPECT_EQ(voices.GetActiveCount(), 3);
}

TEST(AudioVoices, WhenFullStealsTheOldestVoice)
{
    VoiceAllocator voices { 3, 3 };
    (void)voices.Acquire(0);   // slot 0 (가장 오래됨)
    (void)voices.Acquire(1);
    (void)voices.Acquire(2);

    const VoiceAllocator::Acquired got = voices.Acquire(3);
    EXPECT_TRUE(got.bStolen);
    EXPECT_EQ(got.slot, 0);
    EXPECT_EQ(voices.GetActiveCount(), 3);   // 상한을 넘지 않는다
    EXPECT_EQ(voices.GetSoundId(0), 3);

    EXPECT_EQ(voices.Acquire(4).slot, 1);   // 그다음으로 오래된 것
}

TEST(AudioVoices, PerSoundLimitStealsOldestOfThatSoundOnly)
{
    VoiceAllocator voices { 8, 2 };
    const int      first  = voices.Acquire(7).slot;
    const int      other  = voices.Acquire(1).slot;
    const int      second = voices.Acquire(7).slot;
    ASSERT_EQ(voices.CountSound(7), 2);

    const VoiceAllocator::Acquired third = voices.Acquire(7);
    EXPECT_TRUE(third.bStolen);
    EXPECT_EQ(third.slot, first);
    EXPECT_EQ(voices.CountSound(7), 2);
    EXPECT_TRUE(voices.IsActive(other));
    EXPECT_TRUE(voices.IsActive(second));
    EXPECT_EQ(voices.GetActiveCount(), 3);
}

TEST(AudioVoices, ReleaseFreesSlotForReuse)
{
    VoiceAllocator voices { 2, 2 };
    const int      a = voices.Acquire(0).slot;
    (void)voices.Acquire(1);
    voices.Release(a);
    EXPECT_EQ(voices.GetActiveCount(), 1);

    const VoiceAllocator::Acquired got = voices.Acquire(2);
    EXPECT_FALSE(got.bStolen);
    EXPECT_EQ(got.slot, a);

    voices.ReleaseAll();
    EXPECT_EQ(voices.GetActiveCount(), 0);
}

TEST(AudioVoices, NeverExceedsCapacityUnderSpam)
{
    VoiceAllocator voices { 5, 3 };
    for (int i = 0; i < 200; ++i)
    {
        (void)voices.Acquire(i % 7);
        EXPECT_LE(voices.GetActiveCount(), 5);
        for (int s = 0; s < 7; ++s)
        {
            EXPECT_LE(voices.CountSound(s), 3);
        }
    }
}

TEST(AudioMath, PitchStaysInsideVariationRange)
{
    EXPECT_FLOAT_EQ(ComputePitch(0.f, 0.3f), 1.f);
    EXPECT_FLOAT_EQ(ComputePitch(0.1f, 0.5f), 1.f);
    EXPECT_NEAR(ComputePitch(0.1f, 0.f), 0.9f, 1e-6f);
    EXPECT_NEAR(ComputePitch(0.1f, 1.f), 1.1f, 1e-6f);

    AudioRng rng;
    for (int i = 0; i < 1000; ++i)
    {
        const float pitch = ComputePitch(0.2f, rng.NextUnit());
        EXPECT_GE(pitch, 0.8f);
        EXPECT_LE(pitch, 1.2f);
    }
}

TEST(AudioMath, AudioRngIsDeterministicPerSeedAndInUnitRange)
{
    AudioRng a { 123 };
    AudioRng b { 123 };
    AudioRng c { 124 };
    bool     bDiffer = false;
    for (int i = 0; i < 100; ++i)
    {
        const float va = a.NextUnit();
        EXPECT_FLOAT_EQ(va, b.NextUnit());
        EXPECT_GE(va, 0.f);
        EXPECT_LT(va, 1.f);
        bDiffer = bDiffer || va != c.NextUnit();
    }
    EXPECT_TRUE(bDiffer);
}

TEST(AudioMath, BgmFaderReachesZeroInExactDuration)
{
    BgmFader fader;
    fader.BeginFadeOut(1.f);
    int ticks = 0;
    while (!fader.Update(1.f / 60.f) && ticks < 1000)
    {
        ++ticks;
    }
    EXPECT_NEAR(ticks + 1, 60, 1);
    EXPECT_FLOAT_EQ(fader.gain, 0.f);
    EXPECT_TRUE(fader.IsDone());
}

TEST(AudioMath, BgmFaderZeroSecondsIsImmediate)
{
    BgmFader fader;
    fader.BeginFadeOut(0.f);
    EXPECT_FLOAT_EQ(fader.gain, 0.f);
    EXPECT_TRUE(fader.Update(0.f));
}

TEST(AudioSinks, NullAudioAcceptsEverythingSilently)
{
    NullAudio& null = NullAudio::Get();
    null.Play("anything", 1.f, 0.5f);
    null.PlayBgm("bgm", true);
    null.FadeOutBgm(2.f);
    null.StopBgm();
    null.AllStop();
    SUCCEED();
}

TEST(AudioSinks, TestAudioRecordsEventsInOrder)
{
    TestAudio audio;
    audio.Play("A", 0.5f, 0.1f);
    audio.PlayBgm("Song", true);
    audio.Play("B");
    audio.FadeOutBgm(1.5f);
    audio.StopBgm();

    ASSERT_EQ(audio.GetEvents().size(), 5u);
    EXPECT_EQ(audio.GetEvents()[0].kind, eAudioEvent::Play);
    EXPECT_EQ(audio.GetEvents()[0].name, "A");
    EXPECT_FLOAT_EQ(audio.GetEvents()[0].volume, 0.5f);
    EXPECT_FLOAT_EQ(audio.GetEvents()[0].pitchVariation, 0.1f);
    EXPECT_EQ(audio.GetEvents()[1].kind, eAudioEvent::PlayBgm);
    EXPECT_TRUE(audio.GetEvents()[1].bLoop);
    EXPECT_EQ(audio.GetEvents()[3].kind, eAudioEvent::FadeOutBgm);
    EXPECT_FLOAT_EQ(audio.GetEvents()[3].seconds, 1.5f);
    EXPECT_EQ(audio.GetPlayCount(), 2);
    EXPECT_EQ(audio.CountPlays("A"), 1);
    EXPECT_TRUE(audio.GetCurrentBgm().empty());
}

TEST(AudioSinks, GameContextFallsBackToNullAudio)
{
    GameContext ctx;
    EXPECT_EQ(&ctx.GetAudio(), &NullAudio::Get());

    TestAudio audio;
    ctx.pAudio = &audio;
    ctx.GetAudio().Play("X");
    EXPECT_EQ(audio.CountPlays("X"), 1);
}

TEST(AudioSystemDry, UninitializedSystemIsAnInertNoOp)
{
    AudioSystem audio;
    EXPECT_FALSE(audio.IsInitialized());
    audio.Play("PlayerGunShot");
    audio.PlayBgm("Stage1_BGM", true);
    audio.FadeOutBgm(1.f);
    audio.StopBgm();
    audio.AllStop();
    audio.Update(0.016f);
    EXPECT_FALSE(audio.IsInitialized());
}

TEST(AudioSystemDry, DisabledConfigNeverOpensADevice)
{
    AudioSystem audio;
    ASSERT_TRUE(static_cast<bool>(audio.Init(MakeBank(), MakeDryConfig())));
    EXPECT_TRUE(audio.IsInitialized());
}

TEST(AudioSystemDry, InitFromBakedDirectoryOrError)
{
    AudioSystem audio;
    const slug::Result<void> result = audio.InitFromDirectory(BakedRoot(), MakeDryConfig());
    if (!result)
    {
        GTEST_SKIP() << "Assets/Baked 없음";
    }
    EXPECT_TRUE(audio.IsInitialized());
    audio.Play(sfx::kPistolShot);
}

TEST(AudioSystemDry, MissingDirectoryReturnsErrorButSystemStaysInert)
{
    AudioSystem audio;
    EXPECT_FALSE(static_cast<bool>(audio.InitFromDirectory("Z:/no/such/dir", MakeDryConfig())));
    EXPECT_FALSE(audio.IsInitialized());
    audio.Play("Coin");
}

namespace
{
    struct AudioSim
    {
        explicit AudioSim(IAudioSink* _pAudio)
            : rng(uint64_t { 1 })
        {
            ctx.pRng   = &rng;
            ctx.pAudio = _pAudio;
            pState     = new SandboxState { slugtest::MakeFlatConfig() };
            runner     = std::unique_ptr<SimRunner> { new SimRunner { ctx, std::unique_ptr<IGameState> { pState } } };
        }

        void RunScript(const int _ticks)
        {
            for (int t = 0; t < _ticks; ++t)
            {
                InputFrame frame;
                frame.Set(eAction::Right, (t / 40) % 2 == 0);
                frame.Set(eAction::Shot, t % 9 < 4);
                frame.Set(eAction::Jump, t % 70 == 10);
                frame.Set(eAction::Bomb, t % 120 == 50);
                runner->Step(frame);
            }
        }

        slug::RngMt19937        rng;
        GameContext            ctx;
        SandboxState*          pState = nullptr;
        std::unique_ptr<SimRunner> runner = nullptr;
    };
}

TEST(AudioDeterminism, SameStateHashWithNullTestAndDryAudio)
{
    AudioSim withNull { nullptr };
    withNull.RunScript(600);

    TestAudio testAudio;
    AudioSim  withTest { &testAudio };
    withTest.RunScript(600);

    AudioSystem dry;
    ASSERT_TRUE(static_cast<bool>(dry.Init(MakeBank(), MakeDryConfig())));
    AudioSim withDry { &dry };
    withDry.RunScript(600);

    EXPECT_EQ(withNull.pState->HashState(), withTest.pState->HashState());
    EXPECT_EQ(withNull.pState->HashState(), withDry.pState->HashState());
    EXPECT_GT(testAudio.GetPlayCount(), 0);   // 소리 요청은 실제로 나갔다
}

TEST(AudioDeterminism, SimulationDoesNotConsumeSimRngForAudio)
{
    AudioSim a { nullptr };
    TestAudio testAudio;
    AudioSim b { &testAudio };
    a.RunScript(300);
    b.RunScript(300);
    EXPECT_EQ(a.rng.Generate(), b.rng.Generate());
}

TEST(AudioSandbox, PistolShotRequestsShotSound)
{
    TestAudio audio;
    AudioSim  sim { &audio };

    InputFrame shot;
    shot.Set(eAction::Shot, true);
    sim.runner->Step(shot);
    sim.runner->Step({});
    for (int i = 0; i < 20; ++i)
    {
        sim.runner->Step({});
    }
    EXPECT_GE(audio.CountPlays(sfx::kPistolShot), 1);
}

TEST(AudioSandbox, BombThrowRequestsBombThrowName)
{
    TestAudio audio;
    AudioSim  sim { &audio };

    InputFrame bomb;
    bomb.Set(eAction::Bomb, true);
    sim.runner->Step(bomb);
    for (int i = 0; i < 30; ++i)
    {
        sim.runner->Step({});
    }
    EXPECT_EQ(audio.CountPlays(sfx::kBombThrow), 1);
}

TEST(AudioSandbox, EntersWithStageBgmAndStopsOnExit)
{
    TestAudio audio;
    {
        AudioSim sim { &audio };
        EXPECT_EQ(audio.GetCurrentBgm(), sfx::kBgmStage);
    }
    EXPECT_GE(audio.CountKind(eAudioEvent::AllStop), 1);
    EXPECT_TRUE(audio.GetCurrentBgm().empty());
}

TEST(AudioDevice, RealEnginePlaysEveryBakedSoundWithoutFailing)
{
    AudioConfig config;
    config.bEnabled     = true;
    config.masterVolume = 0.001f;   // 테스트 중 소리가 거의 나지 않게

    AudioSystem audio;
    if (!audio.InitFromDirectory(BakedRoot(), config))
    {
        GTEST_SKIP() << "Assets/Baked 없음";
    }

    slug::Result<SoundBank> bank = SoundBank::LoadFromDirectory(BakedRoot());
    ASSERT_TRUE(static_cast<bool>(bank));
    for (const SoundEntry& entry: bank->GetEntries())
    {
        if (!entry.bBgm)
        {
            audio.Play(entry.name, 1.f, 0.05f);
        }
    }

    audio.PlayBgm(sfx::kBgmStage, true);
    for (int i = 0; i < 10; ++i)
    {
        audio.Update(0.016f);
    }
    audio.FadeOutBgm(0.05f);
    for (int i = 0; i < 10; ++i)
    {
        audio.Update(0.016f);
    }
    audio.AllStop();
    EXPECT_TRUE(audio.IsInitialized());
}
