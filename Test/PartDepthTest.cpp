// 부품 depth: 플레이어/아이템 z, 헬기 부품 override, params.parts 엄격 파서, Depth 창 패치 왕복.
#include <algorithm>
#include <filesystem>

#include <gtest/gtest.h>

#include "Core/AssetPath.h"
#include "Editor/MotionEditor.h"
#include "Game/Props/ItemWorld.h"
#include "Render/SpriteBatch.h"
#include "VehicleTestUtil.h"

namespace
{
[[nodiscard]] String ReadText_(
    const FilePath& _path)
{
    const Result<String> text = ReadFileText(_path);
    return text ? text.GetValue() : String {};
}

// Defs/*.json 를 임시 폴더로 복사하고 _name 파일만 _text 로 바꿔 적재한다
[[nodiscard]] bool LoadPatched_(
    const char*      _name,
    const String&    _text,
    EnemyDefLibrary& _out,
    String&          _message)
{
    const FilePath dir = std::filesystem::temp_directory_path() / "SlugPartDepthTest";
    std::filesystem::remove_all(dir);
    std::filesystem::create_directories(dir);
    for (const std::filesystem::directory_entry& entry: std::filesystem::directory_iterator(VehicleDefsDir()))
    {
        if (entry.is_regular_file() && entry.path().extension() == ".json")
        {
            std::filesystem::copy_file(entry.path(), dir / entry.path().filename());
        }
    }
    (void)WriteFileBytes(dir / _name, _text);
    Result<EnemyDefLibrary> lib = EnemyDefLibrary::LoadDirectory(dir, &_message);
    std::filesystem::remove_all(dir);
    if (!lib)
    {
        return false;
    }
    _out = lib.Take();
    return true;
}

[[nodiscard]] String WithHeliParams_(
    const String& _insert)
{
    String       text = ReadText_(VehicleDefsDir() / "Helicopter.json");
    const size_t heli = text.find("\"heli\": {");
    const size_t at   = text.find("\"params\": {", heli);
    text.insert(at + 11, " " + _insert);
    return text;
}

[[nodiscard]] Vector<int> HeliDepths_(
    VehicleRig&             _rig,
    const AnimationLibrary& _anims)
{
    SpriteBatch sprites;
    _rig.World().Submit(sprites, _anims, _rig.Projectiles(), 0.f);
    Vector<int> depths;
    for (const SpriteCommand& command: sprites.GetPendingCommands())
    {
        depths.push_back(command.depth);
    }
    std::sort(depths.begin(), depths.end());
    return depths;
}
}   // namespace

TEST(PartDepth, PlayerZShiftsBulletDepthByTheSameDelta)
{
    Sim sim;
    sim.Player().SetZ(kPlayerDepth + 5);
    sim.Hold(eAction::Shot);
    sim.Step(8);
    const Span<const Bullet> bullets = sim.State().GetProjectiles().GetBullets();
    ASSERT_FALSE(bullets.empty());
    EXPECT_EQ(bullets.front().z, kPlayerBulletDepth + 5);

    Sim plain;
    plain.Hold(eAction::Shot);
    plain.Step(8);
    ASSERT_FALSE(plain.State().GetProjectiles().GetBullets().empty());
    EXPECT_EQ(plain.State().GetProjectiles().GetBullets().front().z, kDefaultZ);
    EXPECT_EQ(ResolveZ(kDefaultZ, kPlayerBulletDepth), kPlayerBulletDepth);
}

TEST(PartDepth, ItemAndPopupFollowInstanceZ)
{
    EXPECT_EQ(ZShift(kItemRenderDepth + 4, kItemRenderDepth, kPopupRenderDepth), kPopupRenderDepth + 4);
    EXPECT_EQ(ZShift(kDefaultZ, kItemRenderDepth, kPopupRenderDepth), kDefaultZ);
    EXPECT_EQ(ZDelta(kDefaultZ, kItemRenderDepth), 0);
}

TEST(PartDepth, HeliPartOverrideChangesOnlyThatPart)
{
    SLUG_REQUIRE_BAKED(anims);
    VehicleRig rig({ 300.f, 378.f });
    ASSERT_TRUE(rig.IsLoaded()) << rig.GetMessage();
    SpawnParams params;
    params.facing     = -1;
    params.bActivated = true;
    (void)rig.Spawn("heli", { 507.f, 100.f }, params);
    rig.Step(10);
    const Vector<int> before = HeliDepths_(rig, anims);
    ASSERT_FALSE(before.empty());
    ASSERT_NE(std::find(before.begin(), before.end(), 16), before.end()) << "rotor/pod depth 16";

    rig.Lib().FindMutable("heli")->heli.partRotor.depth = 18;
    Vector<int> expected                                = before;
    *std::find(expected.begin(), expected.end(), 16)    = 18;
    std::sort(expected.begin(), expected.end());
    EXPECT_EQ(HeliDepths_(rig, anims), expected);
}

TEST(PartDepth, PartsParserIsStrict)
{
    EnemyDefLibrary lib;
    String          message;
    ASSERT_TRUE(LoadPatched_("Helicopter.json", WithHeliParams_("\"parts\": { \"rotor\": { \"depth\": 18, \"sort\": 2 } },"), lib, message)) << message;
    EXPECT_EQ(lib.Get(lib.FindIndex("heli")).heli.partRotor.depth, 18);
    EXPECT_EQ(lib.Get(lib.FindIndex("heli")).heli.partRotor.sort, 2);
    EXPECT_EQ(lib.Get(lib.FindIndex("heli")).heli.partPod.depth, 16);

    EnemyDefLibrary bad;
    EXPECT_FALSE(LoadPatched_("Helicopter.json", WithHeliParams_("\"parts\": { \"bogus\": { \"depth\": 1 } },"), bad, message));
    EXPECT_FALSE(LoadPatched_("Helicopter.json", WithHeliParams_("\"parts\": { \"rotor\": { \"depthh\": 1 } },"), bad, message));
    EXPECT_FALSE(LoadPatched_("Helicopter.json", WithHeliParams_("\"parts\": { \"rotor\": { \"depth\": 1.5 } },"), bad, message));
}

TEST(PartDepth, DepthEditorPatchRoundTripOnTempCopy)
{
    const String original = ReadText_(VehicleDefsDir() / "Helicopter.json");
    if (original.empty())
    {
        GTEST_SKIP() << "Helicopter.json 없음";
    }
    MotionDocument doc;
    ASSERT_TRUE(doc.LoadFromText(original));
    ASSERT_TRUE(doc.SetPartDepth("heli", "rotor", 16, 0, 16, 0));
    EXPECT_EQ(doc.GetText(), original) << "무편집은 바이트 동일";

    ASSERT_TRUE(doc.SetPartDepth("heli", "rotor", 18, 3, 16, 0));
    EXPECT_NE(doc.GetText().find("\"parts\": { \"rotor\": { \"depth\": 18, \"sort\": 3 } }"), String::npos);
    EnemyDefLibrary lib;
    String          message;
    ASSERT_TRUE(LoadPatched_("Helicopter.json", doc.GetText(), lib, message)) << message;
    const HeliCfg& cfg = lib.Get(lib.FindIndex("heli")).heli;
    EXPECT_EQ(cfg.partRotor.depth, 18);
    EXPECT_EQ(cfg.partRotor.sort, 3);
    EXPECT_EQ(cfg.partPod.depth, 16);

    ASSERT_TRUE(doc.SetPartDepth("heli", "rotor", 19, 3, 16, 0));
    EXPECT_NE(doc.GetText().find("\"depth\": 19"), String::npos);
    ASSERT_TRUE(doc.SetPartDepth("heli", "pod", 17, 0, 16, 0));
    ASSERT_TRUE(LoadPatched_("Helicopter.json", doc.GetText(), lib, message)) << message;
    EXPECT_EQ(lib.Get(lib.FindIndex("heli")).heli.partPod.depth, 17);
    EXPECT_EQ(lib.Get(lib.FindIndex("heli")).heli.partRotor.depth, 19);
}
