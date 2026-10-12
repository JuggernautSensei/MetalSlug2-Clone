#include <fstream>
#include <iterator>

#include <gtest/gtest.h>

#include "Core/AssetPath.h"
#include "Editor/StageEditor.h"


[[nodiscard]] static String ReadAll_(
    const FilePath& _path)
{
    std::ifstream file { _path, std::ios::binary };
    return String { std::istreambuf_iterator<char>(file), std::istreambuf_iterator<char>() };
}

[[nodiscard]] static FilePath Stage0Path_()
{
    return FindAssetRoot() / "Stage" / "Stage0.json";
}

[[nodiscard]] static int FirstMovable_(
    const StageDocument& _doc)
{
    for (size_t i = 0; i < _doc.GetEntities().size(); ++i)
    {
        const StageEntity& e = _doc.GetEntities()[i];
        if (e.bHasX && e.bHasY && e.bCanCopy && e.section >= 0)
        {
            return static_cast<int>(i);
        }
    }
    return -1;
}

TEST(StageEditorTest, UneditedDocumentSavesByteIdentical)
{
    if (!std::filesystem::exists(Stage0Path_()))
    {
        GTEST_SKIP() << "Stage0.json 없음";
    }
    StageDocument doc;
    ASSERT_TRUE(doc.Load(Stage0Path_()));
    EXPECT_GT(doc.GetEntities().size(), 100u);
    EXPECT_GT(doc.GetSections().size(), 10u);
    EXPECT_EQ(doc.GetWorldWidth(), 14094);
    EXPECT_EQ(doc.GetBackground(), "Stage0_Main");
    EXPECT_FALSE(doc.IsDirty());

    const FilePath tmp = std::filesystem::temp_directory_path() / "slug_stageeditor_1.json";
    ASSERT_TRUE(doc.Save(tmp));
    EXPECT_EQ(ReadAll_(tmp), ReadAll_(Stage0Path_()));
    std::error_code ec;
    std::filesystem::remove(tmp, ec);
}

TEST(StageEditorTest, MoveOneEntityOnlyItsCoordinateDiffers)
{
    if (!std::filesystem::exists(Stage0Path_()))
    {
        GTEST_SKIP() << "Stage0.json 없음";
    }
    StageDocument doc;
    ASSERT_TRUE(doc.Load(Stage0Path_()));
    const String original = doc.GetText();
    const int    index    = FirstMovable_(doc);
    ASSERT_GE(index, 0);
    const StageEntity before = doc.GetEntities()[static_cast<size_t>(index)];

    ASSERT_TRUE(doc.SetPosition(index, before.x + 7.0, before.y - 3.0));
    EXPECT_TRUE(doc.IsDirty());

    const FilePath tmp = std::filesystem::temp_directory_path() / "slug_stageeditor_2.json";
    ASSERT_TRUE(doc.Save(tmp));
    StageDocument reloaded;
    ASSERT_TRUE(reloaded.Load(tmp));
    ASSERT_EQ(reloaded.GetEntities().size(), doc.GetEntities().size());
    for (size_t i = 0; i < reloaded.GetEntities().size(); ++i)
    {
        const StageEntity& a = reloaded.GetEntities()[i];
        const StageEntity& b = doc.GetEntities()[i];
        EXPECT_EQ(a.def, b.def);
        EXPECT_EQ(a.x, b.x);
        EXPECT_EQ(a.y, b.y);
        if (i == static_cast<size_t>(index))
        {
            EXPECT_EQ(a.x, before.x + 7.0);
            EXPECT_EQ(a.y, before.y - 3.0);
        }
    }

    ASSERT_TRUE(reloaded.SetPosition(index, before.x, before.y));
    EXPECT_EQ(reloaded.GetText(), original);

    doc.Undo();
    EXPECT_EQ(doc.GetText(), original);
    std::error_code ec;
    std::filesystem::remove(tmp, ec);
}

TEST(StageEditorTest, AddDuplicateDeleteRoundTrip)
{
    if (!std::filesystem::exists(Stage0Path_()))
    {
        GTEST_SKIP() << "Stage0.json 없음";
    }
    StageDocument doc;
    ASSERT_TRUE(doc.Load(Stage0Path_()));
    const String original = doc.GetText();
    const size_t count    = doc.GetEntities().size();


    const int added = doc.Add("soldier_bomb", "enemy", 1, 1234.0, 56.5);
    ASSERT_GE(added, 0);
    ASSERT_EQ(doc.GetEntities().size(), count + 1);
    const StageEntity e = doc.GetEntities()[static_cast<size_t>(added)];
    EXPECT_EQ(e.def, "soldier_bomb");
    EXPECT_EQ(e.kind, "enemy");
    EXPECT_EQ(e.section, 1);
    EXPECT_EQ(e.x, 1234.0);
    EXPECT_EQ(e.y, 56.5);


    const FilePath tmp = std::filesystem::temp_directory_path() / "slug_stageeditor_3.json";
    ASSERT_TRUE(doc.Save(tmp));
    StageDocument reloaded;
    ASSERT_TRUE(reloaded.Load(tmp));
    ASSERT_EQ(reloaded.GetEntities().size(), count + 1);
    EXPECT_EQ(reloaded.GetEntities()[static_cast<size_t>(added)].x, 1234.0);
    std::error_code ec;
    std::filesystem::remove(tmp, ec);


    ASSERT_TRUE(doc.Delete(added));
    EXPECT_EQ(doc.GetText(), original);


    int withId = -1;
    for (size_t i = 0; i < doc.GetEntities().size(); ++i)
    {
        if (!doc.GetEntities()[i].id.empty() && doc.GetEntities()[i].bHasX)
        {
            withId = static_cast<int>(i);
            break;
        }
    }
    ASSERT_GE(withId, 0);
    const StageEntity src = doc.GetEntities()[static_cast<size_t>(withId)];
    const int         dup = doc.Duplicate(withId, 16.0, 0.0);
    ASSERT_EQ(dup, withId + 1);
    const StageEntity copy = doc.GetEntities()[static_cast<size_t>(dup)];
    EXPECT_EQ(copy.def, src.def);
    EXPECT_EQ(copy.x, src.x + 16.0);
    EXPECT_NE(copy.id, src.id);
    ASSERT_TRUE(doc.Delete(dup));
    EXPECT_EQ(doc.GetText(), original);


    ASSERT_TRUE(doc.Delete(FirstMovable_(doc)));
    EXPECT_NE(doc.GetText(), original);
    doc.Undo();
    EXPECT_EQ(doc.GetText(), original);
}

TEST(StageEditorTest, UnknownFieldsAndLayoutSurvive)
{
    const String text =
        "{\n"
        "  \"version\": 1,\n"
        "  \"mystery\": {\"a\": [1, 2, {\"b\": null}], \"c\": \"x\\\"y\"},\n"
        "  \"world\": {\"size\": [1000, 500], \"background\": \"bg\"},\n"
        "  \"sections\": [\n"
        "    {\n"
        "      \"id\": \"s0\", \"triggerX\": 0, \"extra\": [true],\n"
        "      \"actions\": [\n"
        "        {\"type\": \"spawn\", \"kind\": \"prop\", \"def\": \"truck\", \"x\": 10, \"y\": 20, \"note\": \"hi\", \"future\": {\"k\": 1}},\n"
        "        {\"type\": \"spawnWave\", \"entries\": [{\"delay\": 1, \"kind\": \"enemy\", \"def\": \"soldier_bomb\", \"camera\": {\"ax\": \"left\"}}]},\n"
        "        {\"type\": \"spawn\", \"kind\": \"enemy\", \"def\": \"soldier_sword\", \"x\": 30, \"groundSnap\": true, \"id\": \"e1\"}\n"
        "      ]\n"
        "    }\n"
        "  ]\n"
        "}\n";
    StageDocument doc;
    ASSERT_TRUE(doc.LoadFromText(text));
    ASSERT_EQ(doc.GetEntities().size(), 3u);
    EXPECT_TRUE(doc.GetEntities()[0].IsPlaceable());
    EXPECT_FALSE(doc.GetEntities()[1].IsPlaceable());
    EXPECT_TRUE(doc.GetEntities()[2].IsPlaceable());
    EXPECT_EQ(doc.GetText(), text);

    ASSERT_TRUE(doc.SetPosition(0, 11.5, 20.0));
    EXPECT_NE(doc.GetText().find("\"x\": 11.5, \"y\": 20, \"note\": \"hi\", \"future\": {\"k\": 1}"), String::npos);
    EXPECT_NE(doc.GetText().find("\"mystery\": {\"a\": [1, 2, {\"b\": null}], \"c\": \"x\\\"y\"},"), String::npos);


    ASSERT_TRUE(doc.Delete(2));
    EXPECT_EQ(doc.GetEntities().size(), 2u);
    EXPECT_NE(doc.GetText().find("]}\n      ]\n"), String::npos);
    doc.Undo();
    doc.Undo();
    EXPECT_EQ(doc.GetText(), text);


    EXPECT_FALSE(doc.LoadFromText("{\"sections\": [}"));
    EXPECT_EQ(doc.GetText(), text);
}

[[nodiscard]] static int FindSectionWithRect_(
    const StageDocument& _doc)
{
    for (size_t i = 0; i < _doc.GetSections().size(); ++i)
    {
        const StageEditSection& s = _doc.GetSections()[i];
        bool                    bSpawn = false;
        for (const StageEntity& e: _doc.GetEntities())
        {
            bSpawn = bSpawn || (e.section == static_cast<int>(i) && e.bHasX);
        }
        if (!s.rects.empty() && bSpawn && s.triggerBegin >= 0)
        {
            return static_cast<int>(i);
        }
    }
    return -1;
}

TEST(StageEditorTest, SectionMoveChangesOnlyItsNumbers)
{
    if (!std::filesystem::exists(Stage0Path_()))
    {
        GTEST_SKIP() << "Stage0.json 없음";
    }
    StageDocument doc;
    ASSERT_TRUE(doc.Load(Stage0Path_()));
    const String original = doc.GetText();
    const int    sec      = FindSectionWithRect_(doc);
    ASSERT_GE(sec, 0);
    const StageEditSection s0       = doc.GetSections()[static_cast<size_t>(sec)];
    const Vector<StageEntity> ents0 = Vector<StageEntity>(doc.GetEntities().begin(), doc.GetEntities().end());
    Vector<double>            triggers;
    for (const StageEditSection& t: doc.GetSections())
    {
        triggers.push_back(t.triggerX);
    }

    ASSERT_TRUE(doc.MoveSection(sec, 13.0, true, true, true));
    ASSERT_EQ(doc.GetEntities().size(), ents0.size());
    const StageEditSection& s1 = doc.GetSections()[static_cast<size_t>(sec)];
    EXPECT_EQ(s1.triggerX, s0.triggerX + 13.0);
    for (size_t r = 0; r < s0.rects.size(); ++r)
    {
        EXPECT_EQ(s1.rects[r].v[0], s0.rects[r].v[0] + 13.0);
        EXPECT_EQ(s1.rects[r].v[2], s0.rects[r].v[2] + 13.0);
        EXPECT_EQ(s1.rects[r].v[1], s0.rects[r].v[1]);
        EXPECT_EQ(s1.rects[r].v[3], s0.rects[r].v[3]);
        EXPECT_EQ(s1.rects[r].glide, s0.rects[r].glide);
    }
    for (size_t i = 0; i < ents0.size(); ++i)
    {
        const StageEntity& a = doc.GetEntities()[i];
        const bool         bMoved = ents0[i].section == sec && ents0[i].bHasX;
        EXPECT_EQ(a.x, bMoved ? ents0[i].x + 13.0 : ents0[i].x);
        EXPECT_EQ(a.y, ents0[i].y);
    }
    for (size_t i = 0; i < doc.GetSections().size(); ++i)
    {
        if (static_cast<int>(i) != sec)
        {
            EXPECT_EQ(doc.GetSections()[i].triggerX, triggers[i]);
        }
    }
    doc.Undo();
    EXPECT_EQ(doc.GetText(), original);

    ASSERT_TRUE(doc.MoveSection(sec, -5.0, true, true, false));
    for (size_t i = 0; i < ents0.size(); ++i)
    {
        EXPECT_EQ(doc.GetEntities()[i].x, ents0[i].x);
    }
    EXPECT_EQ(doc.GetSections()[static_cast<size_t>(sec)].triggerX, s0.triggerX - 5.0);
    ASSERT_TRUE(doc.MoveSection(sec, 5.0, true, true, false));
    EXPECT_EQ(doc.GetText(), original);

    ASSERT_TRUE(doc.MoveSection(sec, 7.0, false, false, true));
    EXPECT_EQ(doc.GetSections()[static_cast<size_t>(sec)].triggerX, s0.triggerX);
    EXPECT_EQ(doc.GetSections()[static_cast<size_t>(sec)].rects[0].v[0], s0.rects[0].v[0]);
    doc.Undo();
    EXPECT_EQ(doc.GetText(), original);
}

TEST(StageEditorTest, SectionRectResizeChangesOnlyRectNumbers)
{
    if (!std::filesystem::exists(Stage0Path_()))
    {
        GTEST_SKIP() << "Stage0.json 없음";
    }
    StageDocument doc;
    ASSERT_TRUE(doc.Load(Stage0Path_()));
    const String original = doc.GetText();
    const int    sec      = FindSectionWithRect_(doc);
    ASSERT_GE(sec, 0);
    const StageEditRect r0 = doc.GetSections()[static_cast<size_t>(sec)].rects[0];
    ARRAY<double, 4>    v  = r0.v;
    v[2] += 40.0;
    v[3] -= 8.0;
    ASSERT_TRUE(doc.SetRect(sec, 0, v));
    const StageEditRect& r1 = doc.GetSections()[static_cast<size_t>(sec)].rects[0];
    EXPECT_EQ(r1.v[0], r0.v[0]);
    EXPECT_EQ(r1.v[1], r0.v[1]);
    EXPECT_EQ(r1.v[2], v[2]);
    EXPECT_EQ(r1.v[3], v[3]);
    if (r0.glideBegin >= 0)
    {
        ASSERT_TRUE(doc.SetGlide(sec, 0, r0.glide + 0.5));
        EXPECT_EQ(doc.GetSections()[static_cast<size_t>(sec)].rects[0].glide, r0.glide + 0.5);
    }
    doc.Undo();
    doc.Undo();
    EXPECT_EQ(doc.GetText(), original);
}
