// 생성 상자(hitboxes) 로더/문서/등록 + params 제자리 교체 왕복.

#include <fstream>
#include <sstream>

#include <gtest/gtest.h>

#include "Editor/HitboxEditor.h"
#include "Editor/MotionEditor.h"
#include "EnemyTestUtil.h"

[[nodiscard]] static String DefsText_()
{
    std::ifstream     file(EnemyDefsPath().string());
    std::stringstream buffer;
    buffer << file.rdbuf();
    return buffer.str();
}

[[nodiscard]] static String WithBoxes_(
    const String& _boxes)
{
    String       text = DefsText_();
    const String key  = "\"soldier_bomb\": {";
    const size_t at   = text.find(key);
    if (at == String::npos)
    {
        return {};
    }
    text.insert(at + key.size(), " \"hitboxes\": " + _boxes + ",");
    return text;
}

TEST(HitboxCreate, LoaderAcceptsAndRejectsKeys)
{
    String                        message;
    const Result<EnemyDefLibrary> ok = EnemyDefLibrary::Parse(WithBoxes_("[{\"id\": \"head\", \"kind\": \"hurt\", \"w\": 10, \"h\": 12, \"offset\": [1, -20]}, {\"kind\": \"attack\", \"w\": 8, \"h\": 8, \"frames\": [2, 3], \"damage\": 2}]"), &message);
    ASSERT_TRUE(static_cast<bool>(ok)) << message.c_str();
    const EnemyDef* pDef = ok->Find("soldier_bomb");
    ASSERT_NE(pDef, nullptr);
    ASSERT_EQ(pDef->hitboxes.size(), 2u);
    EXPECT_EQ(pDef->hitboxes[0].kind, eExtraBoxKind::Hurt);
    EXPECT_FLOAT_EQ(pDef->hitboxes[0].box.cy, -20.f);
    EXPECT_EQ(pDef->hitboxes[1].frameTo, 3);
    EXPECT_EQ(pDef->hitboxes[1].damage, 2);
    EXPECT_FALSE(static_cast<bool>(EnemyDefLibrary::Parse(WithBoxes_("[{\"kind\": \"hurt\", \"w\": 10, \"h\": 12, \"bogus\": 1}]"))));
    EXPECT_FALSE(static_cast<bool>(EnemyDefLibrary::Parse(WithBoxes_("[{\"kind\": \"body\", \"w\": 10, \"h\": 12}]"))));
    EXPECT_FALSE(static_cast<bool>(EnemyDefLibrary::Parse(WithBoxes_("[{\"kind\": \"hurt\", \"h\": 12}]"))));
}

TEST(HitboxCreate, DocumentInsertsReplacesRemovesAndStaysIdenticalWhenUnedited)
{
    const String   text = DefsText_();
    HitboxDocument doc;
    ASSERT_TRUE(doc.LoadFromText(text, "(file)"));
    Vector<ExtraBoxDef> boxes;
    EXPECT_TRUE(doc.ApplyExtraBoxes("soldier_bomb", boxes));
    EXPECT_EQ(doc.GetText(), text);

    ExtraBoxDef box;
    box.id  = "head";
    box.box = { 30.f, 30.f, 0.f, -20.f };
    boxes.push_back(box);
    ASSERT_TRUE(doc.ApplyExtraBoxes("soldier_bomb", boxes));
    String                        message;
    const Result<EnemyDefLibrary> lib = EnemyDefLibrary::Parse(doc.GetText(), &message);
    ASSERT_TRUE(static_cast<bool>(lib)) << message.c_str();
    ASSERT_NE(lib->Find("soldier_bomb"), nullptr);
    EXPECT_EQ(lib->Find("soldier_bomb")->hitboxes.size(), 1u);

    boxes[0].box.cx = 5.f;
    ASSERT_TRUE(doc.ApplyExtraBoxes("soldier_bomb", boxes));
    EXPECT_NE(doc.GetText().find("\"offset\": [5, -20]"), String::npos);

    boxes.clear();
    ASSERT_TRUE(doc.ApplyExtraBoxes("soldier_bomb", boxes));
    EXPECT_EQ(doc.GetText(), text);
    EXPECT_FALSE(doc.ApplyExtraBoxes("no_such_def", boxes) && !boxes.empty());
}

constexpr float kGroundY = 378.f;

TEST(HitboxCreate, HurtBoxRegistersAnExtraHurtbox)
{
    EnemyRig rig;
    ASSERT_TRUE(rig.IsLoaded());
    (void)rig.Spawn("soldier_bomb", { 300.f, kGroundY });
    rig.Step(2);
    const size_t before = rig.Combat().GetHurtboxes().size();
    EnemyDef*    pDef   = rig.Lib().FindMutable("soldier_bomb");
    ASSERT_NE(pDef, nullptr);
    ExtraBoxDef box;
    box.box = { 20.f, 20.f, 0.f, -30.f };
    pDef->hitboxes.push_back(box);
    rig.Step(1);
    EXPECT_EQ(rig.Combat().GetHurtboxes().size(), before + 1);
}

TEST(HitboxCreate, ParamPatchChangesOnlyTheEditedNumber)
{
    const char* const json = "{\"defs\": {\"a\": {\"params\": {\"readySec\": 0.5, \"maxShots\": 3, \"bAuto\": true, \"muzzle\": [1, 2]}}}}";
    MotionDocument    doc;
    ASSERT_TRUE(doc.LoadFromText(json));
    Vector<MotionParam> params = doc.ListParams("a");
    ASSERT_EQ(params.size(), 4u);
    EXPECT_TRUE(params[1].bInt);
    params[1].v[0] = 5.0;
    ASSERT_TRUE(doc.SetParam("a", params[1]));
    String expected = json;
    expected.replace(expected.find("3,"), 1, "5");
    EXPECT_EQ(doc.GetText(), expected);
    params[2].v[0] = 0.0;
    ASSERT_TRUE(doc.SetParam("a", params[2]));
    EXPECT_NE(doc.GetText().find("\"bAuto\": false"), String::npos);
}
