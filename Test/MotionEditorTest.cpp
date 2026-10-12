// Motions 창의 문서 모델: 바뀐 숫자만 제자리 교체, 무편집은 바이트 동일, effects 왕복.

#include <gtest/gtest.h>

#include "Core/AssetPath.h"
#include "Editor/MotionEditor.h"
#include "Game/Enemies/EnemyDef.h"

namespace
{
const char* const kMotionJson =
    "{\"defs\": {\"a\": {\"clips\": {\"throw\": {\"name\": \"X\", \"frames\": 4, \"sec\": 0.08}},\n"
    "  \"params\": {\"parts\": {\"cover\": {\"depth\": 16, \"sort\": 3}}},\n"
    "  \"weapons\": {\"attack\": {\"projectile\": \"p\", \"muzzle\": [0, -37], \"aim\": \"facing\","
    " \"muzzleFx\": {\"clip\": \"F\", \"offset\": [1, 2]}}}}}}";

[[nodiscard]] MotionWeaponValues BaseValues_()
{
    MotionWeaponValues v;
    v.muzzle   = { 0.f, -37.f };
    v.bHasFx   = true;
    v.fxOffset = { 1.f, 2.f };
    return v;
}
}   // namespace

TEST(MotionEditorTest, UneditedApplyIsByteIdentical)
{
    MotionDocument doc;
    ASSERT_TRUE(doc.LoadFromText(kMotionJson));
    EXPECT_TRUE(doc.ApplyWeapon("a", "attack", BaseValues_()));
    EXPECT_TRUE(doc.ApplyClipSec("a", "throw", 0.08));
    EXPECT_TRUE(doc.ApplyPartDepth("a", "cover", 16, 3));
    EXPECT_EQ(doc.GetText(), String { kMotionJson });
    EXPECT_FALSE(doc.ApplyWeapon("a", "missing", BaseValues_()));
}

TEST(MotionEditorTest, MuzzleChangeTouchesOnlyThatNumber)
{
    MotionDocument doc;
    ASSERT_TRUE(doc.LoadFromText(kMotionJson));
    MotionWeaponValues v = BaseValues_();
    v.muzzle.y           = -40.f;
    ASSERT_TRUE(doc.ApplyWeapon("a", "attack", v));
    String       expected = kMotionJson;
    const size_t at       = expected.find("[0, -37]");
    expected.replace(at, 8, "[0, -40]");
    EXPECT_EQ(doc.GetText(), expected);
}

TEST(MotionEditorTest, EffectsInsertEditRemoveRoundTrip)
{
    MotionDocument doc;
    ASSERT_TRUE(doc.LoadFromText(kMotionJson));
    MotionWeaponValues v = BaseValues_();
    v.motionClip         = "throw";
    v.effects.push_back({
        "E", { 3.f, 4.f },
         2, 60
    });
    ASSERT_TRUE(doc.ApplyWeapon("a", "attack", v));
    EXPECT_NE(doc.GetText().find("\"motionClip\": \"throw\""), String::npos);
    EXPECT_NE(doc.GetText().find("\"effects\": [{ \"clip\": \"E\", \"offset\": [3, 4], \"frame\": 2 }]"), String::npos);

    const String once = doc.GetText();
    ASSERT_TRUE(doc.ApplyWeapon("a", "attack", v));
    EXPECT_EQ(doc.GetText(), once);

    v.effects[0].offset = { 5.f, 4.f };
    ASSERT_TRUE(doc.ApplyWeapon("a", "attack", v));
    EXPECT_NE(doc.GetText().find("\"offset\": [5, 4]"), String::npos);

    v.effects.clear();
    ASSERT_TRUE(doc.ApplyWeapon("a", "attack", v));
    EXPECT_NE(doc.GetText().find("\"effects\": []"), String::npos);
}

TEST(MotionEditorTest, EffectsLoadFromRealDefFile)
{
    const FilePath path = FindAssetRoot() / "Defs" / "Enemies.json";
    if (!std::filesystem::exists(path))
    {
        GTEST_SKIP() << path.string();
    }
    MotionDocument doc;
    ASSERT_TRUE(doc.Load(path));
    MotionWeaponValues v;
    v.muzzle     = { 0.f, -63.f };
    v.bHasFx     = true;
    v.fxOffset   = { 0.f, 0.f };
    v.motionClip = "shot";
    v.effects.push_back({
        "Bazuka_Shot_Effect", { 7.f, -9.f },
         1, 60
    });
    ASSERT_TRUE(doc.ApplyWeapon("soldier_bzk", "rocket", v));

    String                        message;
    const Result<EnemyDefLibrary> lib = EnemyDefLibrary::Parse(doc.GetText(), &message);
    ASSERT_TRUE(lib) << message;
    const EnemyDef* pDef = lib->Find("soldier_bzk");
    ASSERT_NE(pDef, nullptr);
    const int w = pDef->FindWeapon("rocket");
    ASSERT_GE(w, 0);
    ASSERT_EQ(pDef->weapons[static_cast<size_t>(w)].effects.size(), 1u);
    EXPECT_FLOAT_EQ(pDef->weapons[static_cast<size_t>(w)].effects[0].offset.x, 7.f);
    EXPECT_EQ(pDef->weapons[static_cast<size_t>(w)].motionClip, "shot");
}
