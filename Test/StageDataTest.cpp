#include "Core/AssetPath.h"
#include <cmath>
#include "Base/Types.h"
#include "Base/Result.h"

#include <gtest/gtest.h>

#include "Base/Error.h"
#include "StageTestUtil.h"

using namespace slug;
using namespace slugtest;

namespace
{
std::string OneSection(
    const std::string& _actions)
{
    return WrapStage(R"J([{"id":"s0","triggerX":0,"actions":)J" + _actions + "}]");
}

const StageAction& FirstAction(
    const StageData& _data)
{
    return _data.sections.at(0).actions.at(0);
}
}

TEST(StageData, LoadsMinimalStageWithDefaults)
{
    const StageData data = MustLoad(OneSection("[]"));
    EXPECT_EQ(data.name, "T");
    EXPECT_FLOAT_EQ(data.viewSize.x, 640.f);
    EXPECT_EQ(data.groundInitial, "G0");
    EXPECT_EQ(data.backdropInitial, "a");
    ASSERT_EQ(data.sections.size(), 1u);
    EXPECT_EQ(data.sections[0].id, "s0");
    EXPECT_TRUE(data.camera.bForward);
    EXPECT_FLOAT_EQ(data.camera.playerMargin, 14.f);
}

TEST(StageData, NoteKeyIsAllowedEverywhere)
{
    const std::string json = WrapStage(R"J([{"id":"s0","triggerX":0,"note":"hi","actions":[
        {"type":"banner","text":"x","seconds":1,"note":"a"},
        {"type":"spawnWave","note":"b","entries":[{"delay":1,"kind":"enemy","def":"e","x":1,"y":2,"note":"c"}]}]}])J");
    const StageData data = MustLoad(json);
    EXPECT_EQ(data.sections[0].actions.size(), 2u);
}

TEST(StageData, SpawnDefaultsDependOnKind)
{
    const StageData data = MustLoad(OneSection(R"J([
        {"type":"spawn","kind":"enemy","def":"soldier_bomb","x":10,"groundSnap":true},
        {"type":"spawn","kind":"prop","def":"truck","x":10,"y":20},
        {"type":"spawn","kind":"npc","def":"pow","x":10,"y":20,"settle":true},
        {"type":"spawn","kind":"item","def":"random_fruit","x":10,"y":20},
        {"type":"spawn","kind":"collider","def":"box_collider","x":10,"y":20,"size":[5,6],"collisionLayer":"wall"},
        {"type":"spawn","kind":"decor","def":"anim_decor","x":10,"y":20,"anim":"Parker","layer":"background"}])J"));
    const auto& a = data.sections[0].actions;
    EXPECT_EQ(a[0].spawn.origin, eOrigin::Feet);
    EXPECT_EQ(a[0].when, eWhen::Normal);
    EXPECT_EQ(a[1].spawn.origin, eOrigin::LegacyCenter);
    EXPECT_EQ(a[1].when, eWhen::Always);
    EXPECT_EQ(a[2].spawn.origin, eOrigin::Feet);
    EXPECT_EQ(a[2].when, eWhen::Always);   // npc
    EXPECT_EQ(a[3].when, eWhen::Always);   // item
    EXPECT_EQ(a[4].when, eWhen::Always);   // collider
    EXPECT_EQ(a[4].spawn.collisionLayer, eCollisionLayer::Wall);
    EXPECT_FLOAT_EQ(a[4].spawn.size.x, 5.f);
    EXPECT_EQ(a[5].when, eWhen::Always);   // decor
    EXPECT_TRUE(a[5].spawn.bHasLayer);
    EXPECT_EQ(a[5].spawn.layer, eStageLayer::Background);
    EXPECT_FLOAT_EQ(a[5].spawn.cullMargin, kDefaultCullMargin);
}

TEST(StageData, NonSpawnActionDefaultsToWhenRule)
{
    const StageData data = MustLoad(OneSection(R"J([
        {"type":"setBounds","rect":[0,0,1000,448]},
        {"type":"setBackdrop","mode":"b"},
        {"type":"setCameraOffset","offset":[1,2]},
        {"type":"unlockCamera"},
        {"type":"setGround","name":"G1"},
        {"type":"lockCamera","rect":"arena"},
        {"type":"banner","text":"x","seconds":1},
        {"type":"setGround","name":"G1","when":"silent"}])J"));
    const auto& a = data.sections[0].actions;
    EXPECT_EQ(a[0].when, eWhen::Always);
    EXPECT_EQ(a[1].when, eWhen::Always);
    EXPECT_EQ(a[2].when, eWhen::Always);
    EXPECT_EQ(a[3].when, eWhen::Always);
    EXPECT_EQ(a[4].when, eWhen::Normal);
    EXPECT_EQ(a[5].when, eWhen::Normal);
    EXPECT_EQ(a[6].when, eWhen::Normal);
    EXPECT_EQ(a[7].when, eWhen::Silent);
}

TEST(StageData, ShouldRunActionTruthTable)
{
    EXPECT_TRUE(ShouldRunAction(eWhen::Normal, false));
    EXPECT_FALSE(ShouldRunAction(eWhen::Normal, true));
    EXPECT_FALSE(ShouldRunAction(eWhen::Silent, false));
    EXPECT_TRUE(ShouldRunAction(eWhen::Silent, true));
    EXPECT_TRUE(ShouldRunAction(eWhen::Always, false));
    EXPECT_TRUE(ShouldRunAction(eWhen::Always, true));
}

TEST(StageData, CameraAnchorParsesFixedAndRangeDx)
{
    const StageData data = MustLoad(OneSection(R"J([
        {"type":"spawnWave","entries":[
            {"delay":1,"kind":"enemy","def":"a","camera":{"ax":"right","dx":[-149,-50],"ay":"top","dy":47.5}},
            {"delay":2,"kind":"enemy","def":"b","camera":{"ax":"left","dx":250,"ay":"bottom","dy":-3}},
            {"delay":3,"kind":"enemy","def":"c","camera":{"ax":"center","ay":"center"}}]}])J"));
    const auto& e = FirstAction(data).waveEntries;
    ASSERT_EQ(e.size(), 3u);
    EXPECT_TRUE(e[0].spawn.camera.IsRandomX());
    EXPECT_EQ(e[0].spawn.camera.dxMin, -149);
    EXPECT_EQ(e[0].spawn.camera.dxMax, -50);
    EXPECT_EQ(e[0].spawn.camera.ax, eAnchorX::Right);
    EXPECT_FLOAT_EQ(e[0].spawn.camera.dy, 47.5f);
    EXPECT_FALSE(e[1].spawn.camera.IsRandomX());
    EXPECT_EQ(e[1].spawn.camera.dxMin, 250);
    EXPECT_EQ(e[1].spawn.camera.ay, eAnchorY::Bottom);
    EXPECT_EQ(e[2].spawn.camera.ax, eAnchorX::Center);
    EXPECT_FLOAT_EQ(e[2].delay, 3.f);
}

TEST(StageData, ParamsKeepNestedStructure)
{
    const StageData data = MustLoad(OneSection(R"J([
        {"type":"spawn","kind":"prop","def":"shop_5","x":1,"y":2,"params":{"hp":15,"sprites":["A","B"],"hitbox":{"size":[150,75],"offset":[0,-200]},"explodeOnDeath":true,"arena":"arena"}}])J"));
    const ParamValue& params = FirstAction(data).spawn.params;
    EXPECT_EQ(params.GetInt("hp"), 15);
    EXPECT_TRUE(params.GetBool("explodeOnDeath"));
    ASSERT_NE(params.Find("sprites"), nullptr);
    EXPECT_EQ(params.Find("sprites")->GetSize(), 2u);
    EXPECT_EQ(params.Find("sprites")->At(1).AsString(), "B");
    const ParamValue* pHitbox = params.Find("hitbox");
    ASSERT_NE(pHitbox, nullptr);
    EXPECT_FLOAT_EQ(pHitbox->GetVec2("size").x, 150.f);
    EXPECT_FLOAT_EQ(pHitbox->GetVec2("offset").y, -200.f);
    EXPECT_EQ(params.GetString("arena"), "arena");
    EXPECT_FALSE(params.Has("missing"));
    EXPECT_EQ(params.GetInt("missing", 7), 7);
}

TEST(StageData, RectsResolveNamesAndCurrent)
{
    const StageData data = MustLoad(OneSection(R"J([
        {"type":"lockCamera","rect":"arena","permanent":true},
        {"type":"lockCamera","rect":"current","glide":0.5},
        {"type":"setBounds","rect":[10,20,30,40]}])J"));
    const auto& a = data.sections[0].actions;
    EXPECT_FALSE(a[0].rect.bCurrent);
    EXPECT_EQ(a[0].rect.rect, Aabb::FromMinMax({ 100.f, 0.f }, { 740.f, 448.f }));
    EXPECT_EQ(a[0].rect.name, "arena");
    EXPECT_TRUE(a[0].bPermanent);
    EXPECT_TRUE(a[1].rect.bCurrent);
    EXPECT_FLOAT_EQ(a[1].glide, 0.5f);
    EXPECT_EQ(a[2].rect.rect, Aabb::FromMinMax({ 10.f, 20.f }, { 30.f, 40.f }));
}

TEST(StageData, GateParsesNestedActionsAndCounts)
{
    const StageData data = MustLoad(OneSection(R"J([
        {"type":"spawn","kind":"enemy","def":"x","id":"a","x":1,"groundSnap":true,"repeat":{"count":3,"stepX":10}},
        {"type":"gate","id":"g","waitFor":["a"],"until":"destroyFinished","abortOnGameOver":true,"actions":[
            {"type":"banner","text":"B","seconds":1,"delay":1.5},
            {"type":"spawn","kind":"prop","def":"p","x":1,"y":1},
            {"type":"gate","id":"g2","waitFor":["a"],"actions":[]}]}])J"));
    const StageAction& gate = data.sections[0].actions[1];
    EXPECT_EQ(gate.until, eGateUntil::DestroyFinished);
    EXPECT_TRUE(gate.bAbortOnGameOver);
    ASSERT_EQ(gate.actions.size(), 3u);
    EXPECT_FLOAT_EQ(gate.actions[0].delay, 1.5f);
    EXPECT_EQ(data.CountSpawns(), 4);   // repeat 3 + 게이트 안 spawn 1
    EXPECT_EQ(data.CountGates(), 2);
}

TEST(StageData, BoundsCurveResolvesSectionIndicesAndEvaluates)
{
    const std::string json = WrapStage(R"J([
        {"id":"a","triggerX":0,"actions":[]},
        {"id":"b","triggerX":100,"actions":[]},
        {"id":"c","triggerX":200,"actions":[]}])J",
        R"J({"initialBounds":[0,0,4000,448],"boundsCurves":[{"id":"slope","source":"playerX","from":"b","until":"c","left":100,"right":3000,"height":448,"top":[[1000,100],[2000,600]],"round":"trunc"}]})J");
    const StageData data = MustLoad(json);
    ASSERT_EQ(data.camera.boundsCurves.size(), 1u);
    const BoundsCurve& curve = data.camera.boundsCurves[0];
    EXPECT_EQ(curve.fromIndex, 1);
    EXPECT_EQ(curve.untilIndex, 2);
    EXPECT_FLOAT_EQ(curve.EvaluateTop(0.f), 100.f);       // 앞쪽 clamp
    EXPECT_FLOAT_EQ(curve.EvaluateTop(1500.f), 350.f);    // 보간
    EXPECT_FLOAT_EQ(curve.EvaluateTop(1000.5f), 100.f);   // trunc(100.25)
    EXPECT_FLOAT_EQ(curve.EvaluateTop(9999.f), 600.f);    // 뒤쪽 clamp
    const Aabb bounds = curve.EvaluateBounds(1500.f);
    EXPECT_EQ(bounds, Aabb::FromMinMax({ 100.f, 350.f }, { 3000.f, 798.f }));
}

TEST(StageDataErrors, InvalidJsonSyntax)
{
    StageLoadDiagnostics diag;
    const slug::Result<StageData> result = LoadStageFromJson("{ not json", &diag);
    ASSERT_FALSE(result);
    EXPECT_EQ(result.GetError().GetCode(), eError::Parse);
    EXPECT_NE(diag.message.find("syntax"), std::string::npos);
}

TEST(StageDataErrors, UnknownTopLevelKey)
{
    std::string json = OneSection("[]");
    json.insert(1, R"J("bogus":1,)J");
    const std::string error = LoadError(json);
    EXPECT_NE(error.find("unknown key 'bogus'"), std::string::npos) << error;
    EXPECT_NE(error.find("root"), std::string::npos) << error;
}

TEST(StageDataErrors, UnknownActionKeyReportsSectionAndActionIndex)
{
    const std::string error = LoadError(OneSection(R"J([{"type":"banner","text":"x","seconds":1},{"type":"banner","text":"y","seconds":1,"colour":"red"}])J"));
    EXPECT_NE(error.find("sections[0] 's0'"), std::string::npos) << error;
    EXPECT_NE(error.find("action #1"), std::string::npos) << error;
    EXPECT_NE(error.find("unknown key 'colour'"), std::string::npos) << error;
}

TEST(StageDataErrors, UnknownActionTypeAndEnumValues)
{
    EXPECT_NE(LoadError(OneSection(R"J([{"type":"explode"}])J")).find("unknown value 'explode'"), std::string::npos);
    EXPECT_NE(LoadError(OneSection(R"J([{"type":"spawn","kind":"monster","def":"a","x":1,"y":1}])J")).find("unknown value 'monster'"), std::string::npos);
    EXPECT_NE(LoadError(OneSection(R"J([{"type":"fade","to":"red","seconds":1}])J")).find("unknown value 'red'"), std::string::npos);
    EXPECT_NE(LoadError(OneSection(R"J([{"type":"banner","text":"x","seconds":1,"when":"sometimes"}])J")).find("unknown value 'sometimes'"), std::string::npos);
}

TEST(StageDataErrors, MissingRequiredAndWrongTypes)
{
    EXPECT_NE(LoadError(OneSection(R"J([{"type":"banner","seconds":1}])J")).find("missing required field 'text'"), std::string::npos);
    EXPECT_NE(LoadError(OneSection(R"J([{"type":"banner","text":5,"seconds":1}])J")).find("'text' must be a string"), std::string::npos);
    EXPECT_NE(LoadError(OneSection(R"J([{"type":"banner","text":"x","seconds":"1"}])J")).find("'seconds' must be a number"), std::string::npos);
    EXPECT_NE(LoadError(OneSection(R"J([{"type":"spawn","kind":"enemy","def":"a","x":1,"y":1,"repeat":{"count":2.5}}])J")).find("'count' must be an integer"), std::string::npos);
    EXPECT_NE(LoadError(OneSection(R"J([{"type":"banner","text":"x","seconds":1,"delay":-1}])J")).find("'delay' must be >= 0"), std::string::npos);
    EXPECT_NE(LoadError(OneSection(R"J([{"type":"cameraMove","to":[1],"seconds":1}])J")).find("array of 2 numbers"), std::string::npos);
}

TEST(StageDataErrors, SpawnPositionRules)
{
    EXPECT_NE(LoadError(OneSection(R"J([{"type":"spawn","kind":"enemy","def":"a"}])J")).find("needs a position"), std::string::npos);
    EXPECT_NE(LoadError(OneSection(R"J([{"type":"spawn","kind":"enemy","def":"a","x":1}])J")).find("needs 'y' unless groundSnap"), std::string::npos);
    EXPECT_NE(LoadError(OneSection(R"J([{"type":"spawn","kind":"enemy","def":"a","x":1,"y":2,"groundSnap":true}])J")).find("'y' must be omitted"), std::string::npos);
    EXPECT_NE(LoadError(OneSection(R"J([{"type":"spawn","kind":"enemy","def":"a","x":1,"camera":{"ax":"left","ay":"top"}}])J")).find("cannot be combined"), std::string::npos);
    EXPECT_NE(LoadError(OneSection(R"J([{"type":"spawn","kind":"enemy","def":"a","x":1,"y":2,"fromY":3}])J")).find("'fromY' requires groundSnap"), std::string::npos);
    EXPECT_NE(LoadError(OneSection(R"J([{"type":"spawn","kind":"prop","def":"a","x":1,"groundSnap":true}])J")).find("groundSnap requires origin 'feet'"), std::string::npos);
}

TEST(StageDataErrors, OriginAndHalfHeightRules)
{
    EXPECT_NE(LoadError(OneSection(R"J([{"type":"spawn","kind":"enemy","def":"a","x":1,"y":2,"origin":"legacyCenter"}])J")).find("needs 'halfHeight'"), std::string::npos);
    EXPECT_NE(LoadError(OneSection(R"J([{"type":"spawn","kind":"enemy","def":"a","x":1,"y":2,"halfHeight":5}])J")).find("only valid with origin 'legacyCenter'"), std::string::npos);
    EXPECT_NE(LoadError(OneSection(R"J([{"type":"spawn","kind":"enemy","def":"a","x":1,"y":2,"origin":"corner"}])J")).find("unknown value 'corner'"), std::string::npos);
}

TEST(StageDataErrors, KindSpecificFields)
{
    EXPECT_NE(LoadError(OneSection(R"J([{"type":"spawn","kind":"collider","def":"box","x":1,"y":2}])J")).find("missing required field 'size'"), std::string::npos);
    EXPECT_NE(LoadError(OneSection(R"J([{"type":"spawn","kind":"enemy","def":"a","x":1,"y":2,"size":[1,1]}])J")).find("only valid for kind 'collider'"), std::string::npos);
    EXPECT_NE(LoadError(OneSection(R"J([{"type":"spawn","kind":"decor","def":"anim_decor","x":1,"y":2}])J")).find("needs 'anim'"), std::string::npos);
    EXPECT_NE(LoadError(OneSection(R"J([{"type":"spawn","kind":"prop","def":"a","x":1,"y":2,"anim":"Parker"}])J")).find("only valid for kind 'decor'"), std::string::npos);
    EXPECT_NE(LoadError(OneSection(R"J([{"type":"spawn","kind":"enemy","def":"a","x":1,"y":2,"params":[1,2]}])J")).find("'params' must be an object"), std::string::npos);
}

TEST(StageDataErrors, WaveEntryRules)
{
    EXPECT_NE(LoadError(OneSection(R"J([{"type":"spawnWave","entries":[{"kind":"enemy","def":"a","x":1,"y":2}]}])J")).find("missing required field 'delay'"), std::string::npos);
    EXPECT_NE(LoadError(OneSection(R"J([{"type":"spawnWave","entries":[]}])J")).find("must not be empty"), std::string::npos);
    EXPECT_NE(LoadError(OneSection(R"J([{"type":"spawnWave","entries":[{"delay":1,"type":"spawn","kind":"enemy","def":"a","x":1,"y":2}]}])J")).find("unknown key 'type'"), std::string::npos);
}

TEST(StageDataErrors, ReferenceChecks)
{
    EXPECT_NE(LoadError(OneSection(R"J([{"type":"lockCamera","rect":"nowhere"}])J")).find("unknown rect 'nowhere'"), std::string::npos);
    EXPECT_NE(LoadError(OneSection(R"J([{"type":"setBounds","rect":"current"}])J")).find("'current' is not allowed"), std::string::npos);
    EXPECT_NE(LoadError(OneSection(R"J([{"type":"setBackdrop","mode":"zzz"}])J")).find("unknown backdrop mode 'zzz'"), std::string::npos);
    EXPECT_NE(LoadError(OneSection(R"J([{"type":"setGround","name":"G9"}])J")).find("unknown ground map 'G9'"), std::string::npos);
    EXPECT_NE(LoadError(OneSection(R"J([{"type":"gate","id":"g","waitFor":["ghost"],"actions":[]}])J")).find("waitFor unknown spawn id 'ghost'"), std::string::npos);
    EXPECT_NE(LoadError(OneSection(R"J([{"type":"command","target":"ghost","command":"go"}])J")).find("command target unknown spawn id 'ghost'"), std::string::npos);
    EXPECT_NE(LoadError(OneSection(R"J([{"type":"spawn","kind":"enemy","def":"a","x":1,"y":2,"params":{"arena":"missing"}}])J")).find("unknown rect 'missing'"), std::string::npos);
}

TEST(StageDataErrors, DuplicateIds)
{
    EXPECT_NE(LoadError(OneSection(R"J([{"type":"spawn","kind":"enemy","def":"a","id":"x","x":1,"y":2},{"type":"spawn","kind":"enemy","def":"a","id":"x","x":1,"y":2}])J")).find("duplicate spawn id 'x'"), std::string::npos);
    EXPECT_NE(LoadError(OneSection(R"J([{"type":"spawn","kind":"enemy","def":"a","id":"x","x":1,"y":2},{"type":"gate","id":"g","waitFor":["x"],"actions":[]},{"type":"gate","id":"g","waitFor":["x"],"actions":[]}])J")).find("duplicate gate id 'g'"), std::string::npos);
    const std::string twoSections = WrapStage(R"J([{"id":"s0","triggerX":0,"actions":[]},{"id":"s0","triggerX":10,"actions":[]}])J");
    EXPECT_NE(LoadError(twoSections).find("duplicate section id"), std::string::npos);
}

TEST(StageDataErrors, SectionOrderingAndCurveReferences)
{
    const std::string unordered = WrapStage(R"J([{"id":"a","triggerX":100,"actions":[]},{"id":"b","triggerX":50,"actions":[]}])J");
    const std::string error     = LoadError(unordered);
    EXPECT_NE(error.find("sections[1] 'b'"), std::string::npos) << error;
    EXPECT_NE(error.find("triggerX"), std::string::npos) << error;

    const std::string badCurve = WrapStage(R"J([{"id":"a","triggerX":0,"actions":[]}])J",
        R"J({"boundsCurves":[{"id":"c","from":"a","until":"zzz","left":0,"right":100,"height":448,"top":[[0,0],[1,1]]}]})J");
    EXPECT_NE(LoadError(badCurve).find("'until' references unknown section 'zzz'"), std::string::npos);

    const std::string reversed = WrapStage(R"J([{"id":"a","triggerX":0,"actions":[]},{"id":"b","triggerX":5,"actions":[]}])J",
        R"J({"boundsCurves":[{"id":"c","from":"b","until":"a","left":0,"right":100,"height":448,"top":[[0,0],[1,1]]}]})J");
    EXPECT_NE(LoadError(reversed).find("must come before"), std::string::npos);
}

TEST(StageDataErrors, GroundAndBackdropInitialMustExist)
{
    std::string json = OneSection("[]");
    const size_t pos = json.find(R"J("initial":"G0")J");
    ASSERT_NE(pos, std::string::npos);
    json.replace(pos, 14, R"J("initial":"Gx")J");
    EXPECT_NE(LoadError(json).find("is not in maps"), std::string::npos);
}

TEST(StageDataErrors, MissingFileIsReported)
{
    StageLoadDiagnostics diag;
    const slug::Result<StageData> result = LoadStageFromFile("does/not/exist/Stage.json", &diag);
    ASSERT_FALSE(result);
    EXPECT_EQ(result.GetError().GetCode(), eError::FileOpen);
}

TEST(StageDataReal, BossArenaHasHmgPickupAndNoHouseInsideIt)
{
    StageLoadDiagnostics diag;
    slug::Result<StageData> result = LoadStageFromFile(slug::FindAssetRoot() / "Stage" / "Stage0.json", &diag);
    ASSERT_TRUE(result) << diag.message;
    const StageData data = result.Take();

    // 보스 섹션: 아레나 왼쪽 +120 바닥에 HMG 픽업 (선체가 들어오기 전에 주울 수 있는 위치)
    const StageSection& boss = data.sections.back();
    int hmgCount = 0;
    for (const StageAction& action: boss.actions)
    {
        if (action.type == eActionType::Spawn && action.spawn.kind == eEntityKind::Item && action.spawn.def == "gunitem_mg")
        {
            ++hmgCount;
            EXPECT_FLOAT_EQ(action.spawn.x, 13454.f + 120.f);
        }
    }
    EXPECT_EQ(hmgCount, 1);

    // openland_4: 아레나(13454~14094) 와 겹치던 big_house(13542) 는 제거 (파괴 불가 소품이 선체 뒤에서 탄을 막았다)
    const StageSection& openland4 = data.sections[static_cast<size_t>(data.FindSection("openland_4"))];
    for (const StageAction& action: openland4.actions)
    {
        if (action.type == eActionType::Spawn)
        {
            EXPECT_NE(action.spawn.def, "big_house");
        }
    }
}

TEST(StageDataReal, Stage0LoadsWithExpectedCounts)
{
    StageLoadDiagnostics diag;
    slug::Result<StageData> result = LoadStageFromFile(slug::FindAssetRoot() / "Stage" / "Stage0.json", &diag);
    ASSERT_TRUE(result) << diag.message;
    const StageData data = result.Take();

    EXPECT_EQ(data.sections.size(), 21u);
    EXPECT_EQ(data.CountSpawns(), 175);
    EXPECT_EQ(data.CountWaveEntries(), 32);
    EXPECT_EQ(data.CountGates(), 4);

    EXPECT_EQ(data.name, "Stage0");
    EXPECT_EQ(data.background, "Stage0_Main");
    EXPECT_FLOAT_EQ(data.worldSize.x, 14094.f);
    EXPECT_EQ(data.groundInitial, "Stage0_0");
    EXPECT_EQ(data.groundMaps.size(), 2u);
    EXPECT_EQ(data.backdrops.size(), 4u);
    EXPECT_EQ(data.camera.initialBounds, Aabb::FromMinMax({ 0.f, 0.f }, { 3520.f, 448.f }));
    EXPECT_FLOAT_EQ(data.camera.playerMargin, 14.f);

    EXPECT_EQ(data.sections.front().id, "intro");
    EXPECT_EQ(data.sections.back().id, "boss");
    EXPECT_FLOAT_EQ(data.sections.back().triggerX, 13500.f);
    EXPECT_EQ(data.FindSection("bus_start"), 3);

    const NamedRect* pArena = data.FindRect("bossArena");
    ASSERT_NE(pArena, nullptr);
    EXPECT_EQ(pArena->rect, Aabb::FromMinMax({ 13454.f, 864.f }, { 14094.f, 1312.f }));

    ASSERT_EQ(data.camera.boundsCurves.size(), 1u);
    EXPECT_EQ(data.camera.boundsCurves[0].fromIndex, data.FindSection("downtown_start"));
    EXPECT_EQ(data.camera.boundsCurves[0].untilIndex, data.FindSection("openland_start"));
    EXPECT_TRUE(data.camera.boundsCurves[0].bTrunc);

    EXPECT_TRUE(data.player.bGroundSnap);
    EXPECT_FLOAT_EQ(data.player.x, 100.f);
}

TEST(StageDataReal, Stage0KeySemantics)
{
    slug::Result<StageData> result = LoadStageFromFile(slug::FindAssetRoot() / "Stage" / "Stage0.json");
    ASSERT_TRUE(result);
    const StageData data = result.Take();

    const StageSection& busCreate = data.sections[static_cast<size_t>(data.FindSection("bus_create"))];
    EXPECT_EQ(busCreate.actions[0].type, eActionType::SetGround);
    EXPECT_EQ(busCreate.actions[0].when, eWhen::Silent);
    EXPECT_EQ(busCreate.actions[1].spawn.def, "mb_truck");
    EXPECT_EQ(busCreate.actions[1].spawn.origin, eOrigin::LegacyCenter);
    EXPECT_FLOAT_EQ(busCreate.actions[1].spawn.halfHeight, 130.f);

    const StageSection& busStart = data.sections[static_cast<size_t>(data.FindSection("bus_start"))];
    EXPECT_EQ(busStart.actions[0].type, eActionType::LockCamera);
    EXPECT_FLOAT_EQ(busStart.actions[0].glide, 1.f);
    EXPECT_EQ(busStart.actions[2].type, eActionType::Gate);
    EXPECT_EQ(busStart.actions[2].waitFor.size(), 1u);

    const StageSection& boss = data.sections.back();
    bool bPermanent = false;
    const StageAction* pGate = nullptr;
    for (const StageAction& action: boss.actions)
    {
        bPermanent = bPermanent || (action.type == eActionType::LockCamera && action.bPermanent);
        if (action.type == eActionType::Gate)
        {
            pGate = &action;
        }
    }
    EXPECT_TRUE(bPermanent);
    ASSERT_NE(pGate, nullptr);
    EXPECT_EQ(pGate->until, eGateUntil::DestroyFinished);
    EXPECT_TRUE(pGate->bAbortOnGameOver);
    EXPECT_EQ(pGate->actions.back().type, eActionType::EndStage);
    EXPECT_FLOAT_EQ(pGate->actions.back().delay, 0.f);

    const StageSection& busEnd = data.sections[static_cast<size_t>(data.FindSection("bus_end"))];
    bool bFence = false;
    for (const StageAction& action: busEnd.actions)
    {
        if (action.type == eActionType::Spawn && action.spawn.def == "fence")
        {
            bFence = true;
            EXPECT_EQ(action.spawn.GetCount(), 8);
            EXPECT_FLOAT_EQ(action.spawn.repeat.stepX, 64.f);
        }
    }
    EXPECT_TRUE(bFence);
}
