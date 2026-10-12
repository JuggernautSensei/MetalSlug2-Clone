// Tuning/TextureView 테스트: 튜닝 JSON 로드와 기본값, 파일 감시, 텍스처 뷰 클리핑 검증
#include <fstream>

#include <gtest/gtest.h>

#include "Base/Result.h"
#include "Base/Types.h"
#include "Core/AssetPath.h"
#include "Core/Tuning.h"
#include "Render/SpriteBatch.h"


TEST(Tuning, EmptyObjectGivesAllDefaults)
{
    Result<Tuning> parsed = ParseTuning("{}");
    ASSERT_TRUE(parsed.HasValue());
    EXPECT_EQ(DumpTuning(parsed.GetValue()), DumpTuning(Tuning {}));
}

TEST(Tuning, PartialOverridesOnlyChangeTheNamedKeys)
{
    const char*          json   = R"({
        "move": { "runSpeed": 99.5, "coyoteTicks": 7 },
        "weapons": { "pistol": { "damage": 5, "pierce": true, "hitSize": [20, 9] } },
        "muzzle": { "stand": { "forward": [31, -33] } },
        "knife": { "range": 40 }
    })";
    Result<Tuning> parsed = ParseTuning(json);
    ASSERT_TRUE(parsed.HasValue());

    const Tuning& t = parsed.GetValue();
    const Tuning  d;
    EXPECT_FLOAT_EQ(t.move.runSpeed, 99.5f);
    EXPECT_EQ(t.move.coyoteTicks, 7);
    EXPECT_FLOAT_EQ(t.move.jumpSpeed, d.move.jumpSpeed);
    EXPECT_EQ(t.weapons.pistol.damage, 5);
    EXPECT_TRUE(t.weapons.pistol.bPierce);
    EXPECT_FLOAT_EQ(t.weapons.pistol.hitSize.x, 20.f);
    EXPECT_FLOAT_EQ(t.weapons.pistol.hitSize.y, 9.f);
    EXPECT_EQ(t.weapons.pistol.fireIntervalTicks, d.weapons.pistol.fireIntervalTicks);
    EXPECT_EQ(t.weapons.hmg.damage, d.weapons.hmg.damage);
    EXPECT_FLOAT_EQ(t.muzzle.stand.forward.x, 31.f);
    EXPECT_FLOAT_EQ(t.muzzle.stand.up.y, d.muzzle.stand.up.y);
    EXPECT_FLOAT_EQ(t.knife.range, 40.f);
}

TEST(Tuning, RejectsWrongTypes)
{
    EXPECT_FALSE(ParseTuning(R"({ "move": { "runSpeed": "fast" } })").HasValue());
    EXPECT_FALSE(ParseTuning(R"({ "move": 5 })").HasValue());
    EXPECT_FALSE(ParseTuning(R"({ "weapons": { "pistol": { "hitSize": [1] } } })").HasValue());
    EXPECT_FALSE(ParseTuning(R"({ "knife": { "damage": true } })").HasValue());
}

TEST(Tuning, RejectsBrokenJsonAndNonObjects)
{
    EXPECT_FALSE(ParseTuning("{ not json").HasValue());
    EXPECT_FALSE(ParseTuning("[1, 2]").HasValue());
}

TEST(Tuning, IgnoresUnknownKeys)
{
    Result<Tuning> parsed = ParseTuning(R"({ "futureSection": { "x": 1 }, "move": { "mystery": 3 } })");
    ASSERT_TRUE(parsed.HasValue());
    EXPECT_EQ(DumpTuning(parsed.GetValue()), DumpTuning(Tuning {}));
}

TEST(Tuning, AllowsCommentsInTheJsonFile)
{
    Result<Tuning> parsed = ParseTuning("{ // 주석\n \"move\": { \"runSpeed\": 110 } }");
    ASSERT_TRUE(parsed.HasValue());
    EXPECT_FLOAT_EQ(parsed.GetValue().move.runSpeed, 110.f);
}

TEST(Tuning, CommittedJsonFileMatchesCodeDefaults)
{
    const FilePath path   = GetDefaultTuningPath(FindAssetRoot());
    Result<Tuning> loaded = LoadTuning(path);
    ASSERT_TRUE(loaded.HasValue()) << path.string();
    EXPECT_EQ(DumpTuning(loaded.GetValue()), DumpTuning(Tuning {})) << "Assets/Tuning/Tuning.json 과 Tuning.h 기본값이 다릅니다";
}

TEST(Tuning, MissingFileIsAnError)
{
    EXPECT_FALSE(LoadTuning(FilePath { "no/such/Tuning.json" }).HasValue());
}

TEST(Tuning, JumpFeelNumbersMatchTheDesignTargets)
{
    const Tuning t;

    float y     = 0.f;
    float vy    = -t.move.jumpSpeed;
    float apex  = 0.f;
    int   ticks = 0;
    do
    {
        vy += t.move.gravity / 60.f;
        y += vy / 60.f;
        apex = std::min(apex, y);
        ++ticks;
    } while (y < 0.f && ticks < 1000);

    EXPECT_GE(-apex, 120.f);
    EXPECT_LE(-apex, 135.f);
    EXPECT_GE(static_cast<float>(ticks) / 60.f, 0.8f);
    EXPECT_LE(static_cast<float>(ticks) / 60.f, 0.92f);
    EXPECT_LE(t.move.terminalVelocity, 1100.f);
}

TEST(Tuning, WeaponTablesAreInternallyConsistent)
{
    const Tuning t;
    for (const eWeapon weapon: { eWeapon::Pistol, eWeapon::Hmg, eWeapon::Shotgun })
    {
        const WeaponTuning& w = t.weapons.Get(weapon);
        EXPECT_GT(w.damage, 0);
        EXPECT_GE(w.fireIntervalTicks, 1);
        EXPECT_TRUE(w.holdRepeatTicks == 0 || w.holdRepeatTicks >= w.fireIntervalTicks);
        EXPECT_GT(w.projectileSpeed, 0.f);
        EXPECT_GT(w.range, 0.f);
        EXPECT_GE(w.shotOverlayTicks, 1);
    }
    EXPECT_EQ(t.weapons.pistol.ammo, 0);
    EXPECT_GE(t.weapons.pistol.damage, 3);
    EXPECT_GE(t.weapons.hmg.burstShotsMin, 3);   // HMG 는 한 번 입력에 3~5발 버스트 (발수는 난수)
    EXPECT_LE(t.weapons.hmg.burstShotsMax, 5);
    EXPECT_LE(t.weapons.hmg.burstShotsMin, t.weapons.hmg.burstShotsMax);
    EXPECT_GE(t.weapons.hmg.spreadDeg, 0.f);   // 흩뿌림/위치 흔들림은 0 이상 (0 = 끔)
    EXPECT_GE(t.weapons.hmg.muzzleJitter.x, 0.f);
    EXPECT_GE(t.weapons.hmg.muzzleJitter.y, 0.f);
    EXPECT_EQ(t.weapons.pistol.holdRepeatTicks, 0);   // 권총/화염/HMG 는 홀드 자동 연사 없음
    EXPECT_EQ(t.weapons.flame.holdRepeatTicks, 0);
    EXPECT_EQ(t.weapons.hmg.holdRepeatTicks, 0);
    EXPECT_GT(t.bomb.damage, t.weapons.shotgun.damage);
    EXPECT_GT(t.bomb.initial, 0);
}

TEST(Tuning, KnifeKillsAGruntInOneSlash)
{
    const Tuning t;
    EXPECT_GE(t.knife.damage, t.sandbox.gruntHp);
}

static void WriteFile_(
    const FilePath& _path,
    const String&   _text)
{
    std::ofstream out { _path, std::ios::binary | std::ios::trunc };
    out << _text;
}

static void Touch_(
    const FilePath& _path,
    const int             _seconds)
{
    std::error_code ec;
    std::filesystem::last_write_time(_path, std::filesystem::last_write_time(_path, ec) + std::chrono::seconds(_seconds), ec);
}

TEST(TuningWatcher, ReloadsWhenTheFileChangesAndKeepsOldValuesOnBadJson)
{
    const FilePath dir = std::filesystem::temp_directory_path() / "slug_tuning_watcher_test";
    std::filesystem::create_directories(dir);
    const FilePath path = dir / "Tuning.json";
    WriteFile_(path, R"({ "move": { "runSpeed": 111 } })");

    Tuning        tuning;
    TuningWatcher watcher;
    watcher.Init(path, 5);

    for (int i = 0; i < 20; ++i)
    {
        EXPECT_FALSE(watcher.Poll(tuning));
    }
    EXPECT_EQ(watcher.GetReloadCount(), 0);

    WriteFile_(path, R"({ "move": { "runSpeed": 77 } })");
    Touch_(path, 10);
    bool bReloaded = false;
    for (int i = 0; i < 5 && !bReloaded; ++i)
    {
        bReloaded = watcher.Poll(tuning);
    }
    EXPECT_TRUE(bReloaded);
    EXPECT_FLOAT_EQ(tuning.move.runSpeed, 77.f);
    EXPECT_EQ(watcher.GetReloadCount(), 1);

    WriteFile_(path, "{ broken");
    Touch_(path, 20);
    for (int i = 0; i < 10; ++i)
    {
        (void)watcher.Poll(tuning);
    }
    EXPECT_FLOAT_EQ(tuning.move.runSpeed, 77.f);
    EXPECT_EQ(watcher.GetReloadCount(), 1);

    WriteFile_(path, R"({ "move": { "runSpeed": 55 } })");
    Touch_(path, 30);
    for (int i = 0; i < 10; ++i)
    {
        (void)watcher.Poll(tuning);
    }
    EXPECT_FLOAT_EQ(tuning.move.runSpeed, 55.f);

    std::error_code ec;
    std::filesystem::remove_all(dir, ec);
}

TEST(TuningWatcher, MissingFileNeverReloads)
{
    TuningWatcher watcher;
    watcher.Init("no/such/file.json", 1);
    Tuning tuning;
    for (int i = 0; i < 10; ++i)
    {
        EXPECT_FALSE(watcher.Poll(tuning));
    }
}

TEST(TextureView, FullyInsideShowsTheViewSizedRegion)
{
    const TextureViewRects view = ComputeTextureView(14094, 1328, { 0.f, 0.f }, { 1000.f, 0.f }, 640, 448);
    ASSERT_TRUE(view.bVisible);
    EXPECT_EQ(view.src, (IntRect { 1000, 0, 640, 448 }));
    EXPECT_EQ(view.dst, (IntRect { 0, 0, 640, 448 }));
}

TEST(TextureView, ClipsAtTheRightEdgeOfTheTexture)
{
    const TextureViewRects view = ComputeTextureView(1000, 448, { 0.f, 0.f }, { 700.f, 0.f }, 640, 448);
    ASSERT_TRUE(view.bVisible);
    EXPECT_EQ(view.src, (IntRect { 700, 0, 300, 448 }));
    EXPECT_EQ(view.dst, (IntRect { 0, 0, 300, 448 }));
}

TEST(TextureView, ClipsAtTheLeftEdgeWhenTheCameraIsBeforeTheTexture)
{
    const TextureViewRects view = ComputeTextureView(1000, 448, { 0.f, 0.f }, { -100.f, 0.f }, 640, 448);
    ASSERT_TRUE(view.bVisible);
    EXPECT_EQ(view.src, (IntRect { 0, 0, 540, 448 }));
    EXPECT_EQ(view.dst, (IntRect { 100, 0, 540, 448 }));
}

TEST(TextureView, TextureOffsetInWorldShiftsTheSource)
{
    const TextureViewRects view = ComputeTextureView(500, 300, { 400.f, 100.f }, { 300.f, 50.f }, 640, 448);
    ASSERT_TRUE(view.bVisible);
    EXPECT_EQ(view.src, (IntRect { 0, 0, 500, 300 }));
    EXPECT_EQ(view.dst, (IntRect { 100, 50, 500, 300 }));
}

TEST(TextureView, OffscreenIsInvisible)
{
    EXPECT_FALSE(ComputeTextureView(100, 100, { 0.f, 0.f }, { 200.f, 0.f }, 640, 448).bVisible);
    EXPECT_FALSE(ComputeTextureView(100, 100, { 2000.f, 0.f }, { 0.f, 0.f }, 640, 448).bVisible);
    EXPECT_FALSE(ComputeTextureView(100, 100, { 0.f, 600.f }, { 0.f, 0.f }, 640, 448).bVisible);
}

TEST(TextureView, SubpixelCameraAndOriginAreFlooredSoEdgesDoNotShimmer)
{
    const TextureViewRects a = ComputeTextureView(2000, 448, { 0.f, 0.f }, { 100.25f, 0.f }, 640, 448);
    const TextureViewRects b = ComputeTextureView(2000, 448, { 0.f, 0.f }, { 100.99f, 0.f }, 640, 448);
    EXPECT_EQ(a.src, b.src);
    EXPECT_EQ(a.src.x, 100);

    const TextureViewRects c = ComputeTextureView(2000, 448, { 0.f, 0.f }, { 101.f, 0.f }, 640, 448);
    EXPECT_EQ(c.src.x, 101);
}

TEST(TextureView, DegenerateInputsAreInvisible)
{
    EXPECT_FALSE(ComputeTextureView(0, 100, { 0.f, 0.f }, { 0.f, 0.f }, 640, 448).bVisible);
    EXPECT_FALSE(ComputeTextureView(100, 100, { 0.f, 0.f }, { 0.f, 0.f }, 0, 448).bVisible);
}
