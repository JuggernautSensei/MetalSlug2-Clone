// 인스턴스 z: 스폰 z 가 제출 depth 를 옮기고 부품 상대 오프셋은 유지된다. 스테이지 "z" 파서/에디터 패치.
#include <algorithm>

#include <gtest/gtest.h>

#include "Core/AssetPath.h"
#include "Editor/StageEditor.h"
#include "Render/SpriteBatch.h"
#include "StageTestUtil.h"
#include "VehicleTestUtil.h"

[[nodiscard]] static Vector<int> SubmittedDepths_(
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

TEST(InstanceZ, SoldierSubmitsAtInstanceZ)
{
    SLUG_REQUIRE_BAKED(anims);
    VehicleRig rig({ 100.f, 378.f }, 0.f);
    ASSERT_TRUE(rig.IsLoaded());
    SpawnParams params;
    params.z = GetEnemyDefaultZ(eEnemyBehavior::Soldier) + 10;
    (void)rig.Spawn("soldier_sword", { 200.f, 378.f }, params);
    rig.Step(3);
    const Vector<int> depths = SubmittedDepths_(rig, anims);
    ASSERT_FALSE(depths.empty());
    EXPECT_EQ(depths.front(), kEnemyDepth + 10);
    EXPECT_EQ(depths.back(), kEnemyDepth + 10);
}

TEST(InstanceZ, BusPartsKeepRelativeOffset)
{
    SLUG_REQUIRE_BAKED(anims);
    VehicleRig base({ 2700.f, 378.f }, 2496.f);
    VehicleRig lifted({ 2700.f, 378.f }, 2496.f);
    ASSERT_TRUE(base.IsLoaded());
    ASSERT_TRUE(lifted.IsLoaded());
    SpawnParams busParams;
    busParams.facing = -1;
    (void)base.Spawn("bus", { 2985.f, 378.f }, busParams);
    busParams.z = GetEnemyDefaultZ(eEnemyBehavior::Bus) + 10;
    (void)lifted.Spawn("bus", { 2985.f, 378.f }, busParams);
    base.Step(30);
    lifted.Step(30);
    Vector<int>       a = SubmittedDepths_(base, anims);
    const Vector<int> b = SubmittedDepths_(lifted, anims);
    ASSERT_FALSE(a.empty());
    ASSERT_EQ(a.size(), b.size());
    for (size_t i = 0; i < a.size(); ++i)
    {
        EXPECT_EQ(a[i] + 10, b[i]);
    }
}

TEST(InstanceZ, StageParserAcceptsIntegerZ)
{
    const std::string spawn = R"J([{"type":"spawn","kind":"enemy","def":"soldier_bomb","x":10,"y":20,"z":25}])J";
    const StageData   data  = MustLoad(WrapStage(R"J([{"id":"s0","triggerX":0,"actions":)J" + spawn + "}]"));
    const SpawnSpec&  s     = data.sections[0].actions[0].spawn;
    EXPECT_TRUE(s.bHasZ);
    EXPECT_EQ(s.z, 25);
    const std::string bad = R"J([{"type":"spawn","kind":"enemy","def":"soldier_bomb","x":10,"y":20,"z":1.5}])J";
    EXPECT_FALSE(LoadError(WrapStage(R"J([{"id":"s0","triggerX":0,"actions":)J" + bad + "}]")).empty());
}

TEST(InstanceZ, StageDocumentZPatchRoundTrip)
{
    const FilePath path = FindAssetRoot() / "Stage" / "Stage0.json";
    if (!std::filesystem::exists(path))
    {
        GTEST_SKIP() << "Stage0.json 없음";
    }
    StageDocument doc;
    ASSERT_TRUE(doc.Load(path));
    const String original = doc.GetText();
    int          index    = -1;
    for (size_t i = 0; i < doc.GetEntities().size(); ++i)
    {
        const StageEntity& e = doc.GetEntities()[i];
        if (e.kind == "enemy" && e.bCanCopy && e.section >= 0 && !e.bHasZ)
        {
            index = static_cast<int>(i);
            break;
        }
    }
    ASSERT_GE(index, 0);
    ASSERT_TRUE(doc.SetZ(index, 40, 15));
    EXPECT_TRUE(doc.GetEntities()[static_cast<size_t>(index)].bHasZ);
    EXPECT_EQ(doc.GetEntities()[static_cast<size_t>(index)].z, 40);
    ASSERT_TRUE(doc.SetZ(index, 41, 15));
    EXPECT_EQ(doc.GetEntities()[static_cast<size_t>(index)].z, 41);
    ASSERT_TRUE(doc.SetZ(index, 15, 15));
    EXPECT_FALSE(doc.GetEntities()[static_cast<size_t>(index)].bHasZ);
    EXPECT_EQ(doc.GetText(), original);
}

TEST(StageEditorUnlock, GroundSnapUnlockInsertsYAndUndoRestoresText)
{
    const FilePath path = FindAssetRoot() / "Stage" / "Stage0.json";
    if (!std::filesystem::exists(path))
    {
        GTEST_SKIP() << "Stage0.json 없음";
    }
    StageDocument doc;
    ASSERT_TRUE(doc.Load(path));
    const String original = doc.GetText();
    int          index    = -1;
    for (size_t i = 0; i < doc.GetEntities().size(); ++i)
    {
        const StageEntity& e = doc.GetEntities()[i];
        if (e.bGroundSnap && !e.bHasY && e.snapBegin >= 0)
        {
            index = static_cast<int>(i);
            break;
        }
    }
    ASSERT_GE(index, 0);
    ASSERT_TRUE(doc.UnlockGround(index, 123.0));
    const StageEntity& unlocked = doc.GetEntities()[static_cast<size_t>(index)];
    EXPECT_FALSE(unlocked.bGroundSnap);
    EXPECT_TRUE(unlocked.bHasY);
    EXPECT_EQ(unlocked.y, 123.0);
    doc.Undo();
    EXPECT_EQ(doc.GetText(), original);
}
