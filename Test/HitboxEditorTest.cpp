
#include <gtest/gtest.h>

#include "Core/AssetPath.h"
#include "Editor/HitboxEditor.h"

namespace
{
[[nodiscard]] String ReadAllText_(
    const FilePath& _path)
{
    const Result<String> text = ReadFileText(_path);
    return text ? text.GetValue() : String {};
}

[[nodiscard]] int FindEntry_(
    const HitboxDocument& _doc,
    StringView            _owner,
    StringView            _label)
{
    for (size_t i = 0; i < _doc.GetEntries().size(); ++i)
    {
        const HitboxEntry& e = _doc.GetEntries()[i];
        if (e.owner == _owner && e.label == _label)
        {
            return static_cast<int>(i);
        }
    }
    return -1;
}
}   // namespace

TEST(HitboxEditorTest, UneditedTuningAndDefsSaveByteIdentical)
{
    const FilePath           root  = FindAssetRoot();
    const ARRAY<FilePath, 3> paths = { root / "Tuning" / "Tuning.json", root / "Defs" / "Enemies.json", root / "Defs" / "BigShiee.json" };
    for (const FilePath& path: paths)
    {
        if (!std::filesystem::exists(path))
        {
            GTEST_SKIP() << path.string();
        }
        HitboxDocument doc;
        ASSERT_TRUE(doc.Load(path, "x"));
        EXPECT_GT(doc.GetEntries().size(), 0u);
        EXPECT_FALSE(doc.IsDirty());
        const FilePath tmp = std::filesystem::temp_directory_path() / "slug_hitbox_1.json";
        ASSERT_TRUE(doc.Save(tmp));
        EXPECT_EQ(ReadAllText_(tmp), ReadAllText_(path));
        std::error_code ec;
        std::filesystem::remove(tmp, ec);
    }
}

TEST(HitboxEditorTest, TuningHurtStandChangeTouchesOnlyThatNumber)
{
    const FilePath path = FindAssetRoot() / "Tuning" / "Tuning.json";
    if (!std::filesystem::exists(path))
    {
        GTEST_SKIP() << "Tuning.json 없음";
    }
    HitboxDocument doc;
    ASSERT_TRUE(doc.Load(path, "Player"));
    const String original = doc.GetText();
    const int    index    = FindEntry_(doc, "Player", "body.hurtStand");
    ASSERT_GE(index, 0);
    EXPECT_DOUBLE_EQ(doc.GetEntries()[static_cast<size_t>(index)].value[0], 16.0);

    ASSERT_TRUE(doc.SetValue(index, eHitboxField::W, 21.0));
    EXPECT_TRUE(doc.IsDirty());

    const FilePath tmp = std::filesystem::temp_directory_path() / "slug_hitbox_2.json";
    ASSERT_TRUE(doc.Save(tmp));
    HitboxDocument reloaded;
    ASSERT_TRUE(reloaded.Load(tmp, "Player"));
    const String    saved = ReadAllText_(tmp);
    std::error_code ec;
    std::filesystem::remove(tmp, ec);

    const size_t pos = original.find("\"hurtStand\": [16, 54]");
    ASSERT_NE(pos, String::npos);
    String expected = original;
    expected.replace(pos, String { "\"hurtStand\": [16, 54]" }.size(), "\"hurtStand\": [21, 54]");
    EXPECT_EQ(saved, expected);

    const int again = FindEntry_(reloaded, "Player", "body.hurtStand");
    ASSERT_GE(again, 0);
    EXPECT_DOUBLE_EQ(reloaded.GetEntries()[static_cast<size_t>(again)].value[0], 21.0);
    EXPECT_DOUBLE_EQ(reloaded.GetEntries()[static_cast<size_t>(again)].value[1], 54.0);
    EXPECT_EQ(reloaded.GetEntries().size(), doc.GetEntries().size());
}

TEST(HitboxEditorTest, EnemyDefBodyEditKeepsFormattingAndOtherBoxes)
{
    const FilePath path = FindAssetRoot() / "Defs" / "Enemies.json";
    if (!std::filesystem::exists(path))
    {
        GTEST_SKIP() << "Enemies.json 없음";
    }
    HitboxDocument doc;
    ASSERT_TRUE(doc.Load(path, "(file)"));
    const int index = FindEntry_(doc, "human_base", "defs.human_base.body");
    ASSERT_GE(index, 0);
    const HitboxEntry before = doc.GetEntries()[static_cast<size_t>(index)];
    EXPECT_TRUE(before.HasField(eHitboxField::Cx));
    const String original = doc.GetText();

    ASSERT_TRUE(doc.SetValue(index, eHitboxField::H, 80.0));
    ASSERT_TRUE(doc.SetValue(index, eHitboxField::Cy, -40.0));

    const String from = "\"body\": { \"w\": 40, \"h\": 75, \"cx\": 0, \"cy\": -37.5 }";
    const size_t at   = original.find(from);
    ASSERT_NE(at, String::npos);
    String expected = original;
    expected.replace(at, from.size(), "\"body\": { \"w\": 40, \"h\": 80, \"cx\": 0, \"cy\": -40 }");
    EXPECT_EQ(doc.GetText(), expected);
}

TEST(HitboxEditorTest, FromTextHandlesArraysAndObjects)
{
    HitboxDocument doc;
    ASSERT_TRUE(doc.LoadFromText("{ \"body\": { \"hurtStand\": [16, 54] }, \"defs\": { \"a\": { \"hurtbox\": { \"w\": 1, \"h\": 2 } } } }", "Player"));
    ASSERT_EQ(doc.GetEntries().size(), 2u);
    EXPECT_EQ(doc.GetEntries()[0].owner, "Player");
    EXPECT_FALSE(doc.GetEntries()[0].HasField(eHitboxField::Cx));
    EXPECT_EQ(doc.GetEntries()[1].owner, "a");
    EXPECT_FALSE(doc.GetEntries()[1].HasField(eHitboxField::Cx));
    EXPECT_FALSE(doc.SetValue(1, eHitboxField::Cx, 3.0));
}
