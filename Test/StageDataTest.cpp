#include <cmath>
#include <filesystem>

#include <gtest/gtest.h>

#include "Base/SlugError.h"
#include "Base/Types.h"
#include "Core/AssetPath.h"
#include "Game/Stage/StageMapping.h"
#include "StageTestUtil.h"
#include "World/GroundMap.h"

namespace
{
[[nodiscard]] String OneSection_(
    const String& _actions)
{
    return WrapStage(R"J([{"id":"s0","triggerX":0,"actions":)J" + _actions + "}]");
}

[[nodiscard]] const StageAction& FirstAction_(
    const StageData& _data)
{
    return _data.sections.at(0).actions.at(0);
}
}   // namespace

TEST(StageData, LoadsMinimalStageWithDefaults)
{
    const StageData data = MustLoad(OneSection_("[]"));
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
    const String    json = WrapStage(R"J([{"id":"s0","triggerX":0,"note":"hi","actions":[
        {"type":"banner","text":"x","seconds":1,"note":"a"},
        {"type":"spawnWave","note":"b","entries":[{"delay":1,"kind":"enemy","def":"e","x":1,"y":2,"note":"c"}]}]}])J");
    const StageData data = MustLoad(json);
    EXPECT_EQ(data.sections[0].actions.size(), 2u);
}

TEST(StageData, SpawnDefaultsDependOnKind)
{
    const StageData            data = MustLoad(OneSection_(R"J([
        {"type":"spawn","kind":"enemy","def":"soldier_bomb","x":10,"groundSnap":true},
        {"type":"spawn","kind":"prop","def":"truck","x":10,"y":20},
        {"type":"spawn","kind":"npc","def":"pow","x":10,"y":20,"settle":true},
        {"type":"spawn","kind":"item","def":"random_fruit","x":10,"y":20},
        {"type":"spawn","kind":"collider","def":"box_collider","x":10,"y":20,"size":[5,6],"collisionLayer":"wall"},
        {"type":"spawn","kind":"decor","def":"anim_decor","x":10,"y":20,"anim":"Parker","layer":"background"}])J"));
    const Vector<StageAction>& a    = data.sections[0].actions;
    EXPECT_EQ(a[0].spawn.origin, eOrigin::Feet);
    EXPECT_EQ(a[0].when, eWhen::Normal);
    EXPECT_EQ(a[1].spawn.origin, eOrigin::LegacyCenter);
    EXPECT_EQ(a[1].when, eWhen::Always);
    EXPECT_EQ(a[2].spawn.origin, eOrigin::Feet);
    EXPECT_EQ(a[2].when, eWhen::Always);
    EXPECT_EQ(a[3].when, eWhen::Always);
    EXPECT_EQ(a[4].when, eWhen::Always);
    EXPECT_EQ(a[4].spawn.collisionLayer, eCollisionLayer::Wall);
    EXPECT_FLOAT_EQ(a[4].spawn.size.x, 5.f);
    EXPECT_EQ(a[5].when, eWhen::Always);
    EXPECT_TRUE(a[5].spawn.bHasLayer);
    EXPECT_EQ(a[5].spawn.layer, eStageLayer::Background);
    EXPECT_FLOAT_EQ(a[5].spawn.cullMargin, kDefaultCullMargin);
}

TEST(StageData, NonSpawnActionDefaultsToWhenRule)
{
    const StageData            data = MustLoad(OneSection_(R"J([
        {"type":"setBounds","rect":[0,0,1000,448]},
        {"type":"setBackdrop","mode":"b"},
        {"type":"setCameraOffset","offset":[1,2]},
        {"type":"unlockCamera"},
        {"type":"setGround","name":"G1"},
        {"type":"lockCamera","rect":"arena"},
        {"type":"banner","text":"x","seconds":1},
        {"type":"setGround","name":"G1","when":"silent"}])J"));
    const Vector<StageAction>& a    = data.sections[0].actions;
    EXPECT_EQ(a[0].when, eWhen::Always);
    EXPECT_EQ(a[1].when, eWhen::Always);
    EXPECT_EQ(a[2].when, eWhen::Always);
    EXPECT_EQ(a[3].when, eWhen::Always);
    EXPECT_EQ(a[4].when, eWhen::Normal);
    EXPECT_EQ(a[5].when, eWhen::Normal);
    EXPECT_EQ(a[6].when, eWhen::Normal);
    EXPECT_EQ(a[7].when, eWhen::Silent);
}

TEST(StageData, BannerParsesOptionalSfx)
{
    const StageData data = MustLoad(OneSection_(R"J([
        {"type":"banner","text":"A","seconds":1},
        {"type":"banner","text":"B","seconds":1,"sfx":"Sfx_Mission4Start"}])J"));
    EXPECT_TRUE(data.sections[0].actions[0].name.empty());
    EXPECT_EQ(data.sections[0].actions[1].name, "Sfx_Mission4Start");
    EXPECT_NE(LoadError(OneSection_(R"J([{"type":"banner","text":"A","seconds":1,"sfx":3}])J")).find("sfx"), String::npos);
}

TEST(StageData, PlayBgmParsesFadeOutSecAndRejectsBadValues)
{
    const StageData data = MustLoad(OneSection_(R"J([
        {"type":"playBgm","name":"A"},
        {"type":"playBgm","name":"B","fadeOutSec":2.0}])J"));
    EXPECT_FLOAT_EQ(data.sections[0].actions[0].fadeOutSec, 0.f);
    EXPECT_FLOAT_EQ(data.sections[0].actions[1].fadeOutSec, 2.f);

    EXPECT_NE(LoadError(OneSection_(R"J([{"type":"playBgm","name":"A","fadeOutSec":-1}])J")).find("'fadeOutSec' must be >= 0"), String::npos);
    EXPECT_NE(LoadError(OneSection_(R"J([{"type":"playBgm","name":"A","fadeOutSec":"2"}])J")).find("fadeOutSec: 숫자가 아닙니다"), String::npos);
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
    const StageData          data = MustLoad(OneSection_(R"J([
        {"type":"spawnWave","entries":[
            {"delay":1,"kind":"enemy","def":"a","camera":{"ax":"right","dx":[-149,-50],"ay":"top","dy":47.5}},
            {"delay":2,"kind":"enemy","def":"b","camera":{"ax":"left","dx":250,"ay":"bottom","dy":-3}},
            {"delay":3,"kind":"enemy","def":"c","camera":{"ax":"center","ay":"center"}}]}])J"));
    const Vector<WaveEntry>& e    = FirstAction_(data).waveEntries;
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
    const StageData   data   = MustLoad(OneSection_(R"J([
        {"type":"spawn","kind":"prop","def":"shop_5","x":1,"y":2,"params":{"hp":15,"sprites":["A","B"],"hitbox":{"size":[150,75],"offset":[0,-200]},"explodeOnDeath":true,"arena":"arena"}}])J"));
    const ParamValue& params = FirstAction_(data).spawn.params;
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
    const StageData            data = MustLoad(OneSection_(R"J([
        {"type":"lockCamera","rect":"arena","permanent":true},
        {"type":"lockCamera","rect":"current","glide":0.5},
        {"type":"setBounds","rect":[10,20,30,40]}])J"));
    const Vector<StageAction>& a    = data.sections[0].actions;
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
    const StageData    data = MustLoad(OneSection_(R"J([
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
    EXPECT_EQ(data.CountSpawns(), 4);
    EXPECT_EQ(data.CountGates(), 2);
}

TEST(StageData, BoundsCurveResolvesSectionIndicesAndEvaluates)
{
    const String    json = WrapStage(R"J([
        {"id":"a","triggerX":0,"actions":[]},
        {"id":"b","triggerX":100,"actions":[]},
        {"id":"c","triggerX":200,"actions":[]}])J",
                                     R"J({"initialBounds":[0,0,4000,448],"boundsCurves":[{"id":"slope","source":"playerX","from":"b","until":"c","left":100,"right":3000,"height":448,"top":[[1000,100],[2000,600]],"round":"trunc"}]})J");
    const StageData data = MustLoad(json);
    ASSERT_EQ(data.camera.boundsCurves.size(), 1u);
    const BoundsCurve& curve = data.camera.boundsCurves[0];
    EXPECT_EQ(curve.fromIndex, 1);
    EXPECT_EQ(curve.untilIndex, 2);
    EXPECT_FLOAT_EQ(curve.EvaluateTop(0.f), 100.f);
    EXPECT_FLOAT_EQ(curve.EvaluateTop(1500.f), 350.f);
    EXPECT_FLOAT_EQ(curve.EvaluateTop(1000.5f), 100.f);
    EXPECT_FLOAT_EQ(curve.EvaluateTop(9999.f), 600.f);
    const Aabb bounds = curve.EvaluateBounds(1500.f);
    EXPECT_EQ(bounds, Aabb::FromMinMax({ 100.f, 350.f }, { 3000.f, 798.f }));
}

TEST(StageDataErrors, InvalidJsonSyntax)
{
    String                  diag;
    const Result<StageData> result = LoadStageFromJson("{ not json", &diag);
    ASSERT_FALSE(result);
    EXPECT_TRUE(HasErrorCode(result.GetError(), eError::Parse));
    EXPECT_NE(diag.find("syntax"), String::npos);
}

TEST(StageDataErrors, UnknownTopLevelKey)
{
    String json = OneSection_("[]");
    json.insert(1, R"J("bogus":1,)J");
    const String error = LoadError(json);
    EXPECT_NE(error.find("알 수 없는 키 'bogus'"), String::npos) << error;
    EXPECT_NE(error.find("root"), String::npos) << error;
}

TEST(StageDataErrors, UnknownActionKeyReportsSectionAndActionIndex)
{
    const String error = LoadError(OneSection_(R"J([{"type":"banner","text":"x","seconds":1},{"type":"banner","text":"y","seconds":1,"colour":"red"}])J"));
    EXPECT_NE(error.find("sections[0] 's0'"), String::npos) << error;
    EXPECT_NE(error.find("action #1"), String::npos) << error;
    EXPECT_NE(error.find("알 수 없는 키 'colour'"), String::npos) << error;
}

TEST(StageDataErrors, UnknownActionTypeAndEnumValues)
{
    EXPECT_NE(LoadError(OneSection_(R"J([{"type":"explode"}])J")).find("unknown value 'explode'"), String::npos);
    EXPECT_NE(LoadError(OneSection_(R"J([{"type":"spawn","kind":"monster","def":"a","x":1,"y":1}])J")).find("unknown value 'monster'"), String::npos);
    EXPECT_NE(LoadError(OneSection_(R"J([{"type":"fade","to":"red","seconds":1}])J")).find("unknown value 'red'"), String::npos);
    EXPECT_NE(LoadError(OneSection_(R"J([{"type":"banner","text":"x","seconds":1,"when":"sometimes"}])J")).find("unknown value 'sometimes'"), String::npos);
}

TEST(StageDataErrors, MissingRequiredAndWrongTypes)
{
    EXPECT_NE(LoadError(OneSection_(R"J([{"type":"banner","seconds":1}])J")).find("text: 필수 값이 없습니다"), String::npos);
    EXPECT_NE(LoadError(OneSection_(R"J([{"type":"banner","text":5,"seconds":1}])J")).find("text: 문자열이 아닙니다"), String::npos);
    EXPECT_NE(LoadError(OneSection_(R"J([{"type":"banner","text":"x","seconds":"1"}])J")).find("seconds: 숫자가 아닙니다"), String::npos);
    EXPECT_NE(LoadError(OneSection_(R"J([{"type":"spawn","kind":"enemy","def":"a","x":1,"y":1,"repeat":{"count":2.5}}])J")).find("count: 정수가 아닙니다"), String::npos);
    EXPECT_NE(LoadError(OneSection_(R"J([{"type":"banner","text":"x","seconds":1,"delay":-1}])J")).find("'delay' must be >= 0"), String::npos);
    EXPECT_NE(LoadError(OneSection_(R"J([{"type":"cameraMove","to":[1],"seconds":1}])J")).find("[x, y] 숫자 배열이 아닙니다"), String::npos);
}

TEST(StageDataErrors, SpawnPositionRules)
{
    EXPECT_NE(LoadError(OneSection_(R"J([{"type":"spawn","kind":"enemy","def":"a"}])J")).find("needs a position"), String::npos);
    EXPECT_NE(LoadError(OneSection_(R"J([{"type":"spawn","kind":"enemy","def":"a","x":1}])J")).find("needs 'y' unless groundSnap"), String::npos);
    EXPECT_NE(LoadError(OneSection_(R"J([{"type":"spawn","kind":"enemy","def":"a","x":1,"y":2,"groundSnap":true}])J")).find("'y' must be omitted"), String::npos);
    EXPECT_NE(LoadError(OneSection_(R"J([{"type":"spawn","kind":"enemy","def":"a","x":1,"camera":{"ax":"left","ay":"top"}}])J")).find("cannot be combined"), String::npos);
    EXPECT_TRUE(LoadError(OneSection_(R"J([{"type":"spawn","kind":"enemy","def":"a","x":1,"y":2,"fromY":3}])J")).empty()) << "에디터 Y 잠금 해제가 남기는 groundSnap:false + fromY + y 조합은 허용 (fromY 는 무시)";
    EXPECT_NE(LoadError(OneSection_(R"J([{"type":"spawn","kind":"prop","def":"a","x":1,"groundSnap":true}])J")).find("groundSnap requires origin 'feet'"), String::npos);
}

TEST(StageDataErrors, OriginAndHalfHeightRules)
{
    EXPECT_NE(LoadError(OneSection_(R"J([{"type":"spawn","kind":"enemy","def":"a","x":1,"y":2,"origin":"legacyCenter"}])J")).find("needs 'halfHeight'"), String::npos);
    EXPECT_NE(LoadError(OneSection_(R"J([{"type":"spawn","kind":"enemy","def":"a","x":1,"y":2,"halfHeight":5}])J")).find("only valid with origin 'legacyCenter'"), String::npos);
    EXPECT_NE(LoadError(OneSection_(R"J([{"type":"spawn","kind":"enemy","def":"a","x":1,"y":2,"origin":"corner"}])J")).find("unknown value 'corner'"), String::npos);
}

TEST(StageDataErrors, KindSpecificFields)
{
    EXPECT_NE(LoadError(OneSection_(R"J([{"type":"spawn","kind":"collider","def":"box","x":1,"y":2}])J")).find("size: 필수 값이 없습니다"), String::npos);
    EXPECT_NE(LoadError(OneSection_(R"J([{"type":"spawn","kind":"enemy","def":"a","x":1,"y":2,"size":[1,1]}])J")).find("only valid for kind 'collider'"), String::npos);
    EXPECT_NE(LoadError(OneSection_(R"J([{"type":"spawn","kind":"decor","def":"anim_decor","x":1,"y":2}])J")).find("needs 'anim'"), String::npos);
    EXPECT_NE(LoadError(OneSection_(R"J([{"type":"spawn","kind":"prop","def":"a","x":1,"y":2,"anim":"Parker"}])J")).find("only valid for kind 'decor'"), String::npos);
    EXPECT_NE(LoadError(OneSection_(R"J([{"type":"spawn","kind":"enemy","def":"a","x":1,"y":2,"params":[1,2]}])J")).find("params: 객체가 아닙니다"), String::npos);
}

TEST(StageDataErrors, WaveEntryRules)
{
    EXPECT_NE(LoadError(OneSection_(R"J([{"type":"spawnWave","entries":[{"kind":"enemy","def":"a","x":1,"y":2}]}])J")).find("delay: 필수 값이 없습니다"), String::npos);
    EXPECT_NE(LoadError(OneSection_(R"J([{"type":"spawnWave","entries":[]}])J")).find("must not be empty"), String::npos);
    EXPECT_NE(LoadError(OneSection_(R"J([{"type":"spawnWave","entries":[{"delay":1,"type":"spawn","kind":"enemy","def":"a","x":1,"y":2}]}])J")).find("알 수 없는 키 'type'"), String::npos);
}

TEST(StageDataErrors, WaveTimeRepeatRules)
{
    EXPECT_NE(LoadError(OneSection_(R"J([{"type":"spawnWave","entries":[{"delay":1,"every":2,"times":0,"kind":"enemy","def":"a","x":1,"y":2}]}])J")).find("'times' must be >= 1"), String::npos);
    EXPECT_NE(LoadError(OneSection_(R"J([{"type":"spawnWave","entries":[{"delay":1,"times":3,"kind":"enemy","def":"a","x":1,"y":2}]}])J")).find("'every' must be > 0"), String::npos);
}

TEST(StageData, WaveEntryParsesEveryAndTimesWithOneTimeDefault)
{
    const StageData          data = MustLoad(OneSection_(R"J([
        {"type":"spawnWave","entries":[
            {"delay":12,"every":36,"times":20,"kind":"npc","def":"pow","x":1,"y":2},
            {"delay":1,"kind":"enemy","def":"b","x":1,"y":2}]}])J"));
    const Vector<WaveEntry>& e    = FirstAction_(data).waveEntries;
    ASSERT_EQ(e.size(), 2u);
    EXPECT_FLOAT_EQ(e[0].every, 36.f);
    EXPECT_EQ(e[0].times, 20);
    EXPECT_EQ(e[1].times, 1);
}

TEST(StageDataErrors, ReferenceChecks)
{
    EXPECT_NE(LoadError(OneSection_(R"J([{"type":"lockCamera","rect":"nowhere"}])J")).find("unknown rect 'nowhere'"), String::npos);
    EXPECT_NE(LoadError(OneSection_(R"J([{"type":"setBounds","rect":"current"}])J")).find("'current' is not allowed"), String::npos);
    EXPECT_NE(LoadError(OneSection_(R"J([{"type":"setBackdrop","mode":"zzz"}])J")).find("unknown backdrop mode 'zzz'"), String::npos);
    EXPECT_NE(LoadError(OneSection_(R"J([{"type":"setGround","name":"G9"}])J")).find("unknown ground map 'G9'"), String::npos);
    EXPECT_NE(LoadError(OneSection_(R"J([{"type":"gate","id":"g","waitFor":["ghost"],"actions":[]}])J")).find("waitFor unknown spawn id 'ghost'"), String::npos);
    EXPECT_NE(LoadError(OneSection_(R"J([{"type":"command","target":"ghost","command":"go"}])J")).find("command target unknown spawn id 'ghost'"), String::npos);
    EXPECT_NE(LoadError(OneSection_(R"J([{"type":"spawn","kind":"enemy","def":"a","x":1,"y":2,"params":{"arena":"missing"}}])J")).find("unknown rect 'missing'"), String::npos);
}

TEST(StageDataErrors, DuplicateIds)
{
    EXPECT_NE(LoadError(OneSection_(R"J([{"type":"spawn","kind":"enemy","def":"a","id":"x","x":1,"y":2},{"type":"spawn","kind":"enemy","def":"a","id":"x","x":1,"y":2}])J")).find("duplicate spawn id 'x'"), String::npos);
    EXPECT_NE(LoadError(OneSection_(R"J([{"type":"spawn","kind":"enemy","def":"a","id":"x","x":1,"y":2},{"type":"gate","id":"g","waitFor":["x"],"actions":[]},{"type":"gate","id":"g","waitFor":["x"],"actions":[]}])J")).find("duplicate gate id 'g'"), String::npos);
    const String twoSections = WrapStage(R"J([{"id":"s0","triggerX":0,"actions":[]},{"id":"s0","triggerX":10,"actions":[]}])J");
    EXPECT_NE(LoadError(twoSections).find("duplicate section id"), String::npos);
}

TEST(StageDataErrors, SectionOrderingAndCurveReferences)
{
    const String unordered = WrapStage(R"J([{"id":"a","triggerX":100,"actions":[]},{"id":"b","triggerX":50,"actions":[]}])J");
    const String error     = LoadError(unordered);
    EXPECT_NE(error.find("sections[1] 'b'"), String::npos) << error;
    EXPECT_NE(error.find("triggerX"), String::npos) << error;

    const String badCurve = WrapStage(R"J([{"id":"a","triggerX":0,"actions":[]}])J",
                                      R"J({"boundsCurves":[{"id":"c","from":"a","until":"zzz","left":0,"right":100,"height":448,"top":[[0,0],[1,1]]}]})J");
    EXPECT_NE(LoadError(badCurve).find("'until' references unknown section 'zzz'"), String::npos);

    const String reversed = WrapStage(R"J([{"id":"a","triggerX":0,"actions":[]},{"id":"b","triggerX":5,"actions":[]}])J",
                                      R"J({"boundsCurves":[{"id":"c","from":"b","until":"a","left":0,"right":100,"height":448,"top":[[0,0],[1,1]]}]})J");
    EXPECT_NE(LoadError(reversed).find("must come before"), String::npos);
}

TEST(StageDataErrors, GroundAndBackdropInitialMustExist)
{
    String       json = OneSection_("[]");
    const size_t pos  = json.find(R"J("initial":"G0")J");
    ASSERT_NE(pos, String::npos);
    json.replace(pos, 14, R"J("initial":"Gx")J");
    EXPECT_NE(LoadError(json).find("is not in maps"), String::npos);
}

TEST(StageDataErrors, MissingFileIsReported)
{
    String                  diag;
    const Result<StageData> result = LoadStageFromFile("does/not/exist/Stage.json", &diag);
    ASSERT_FALSE(result);
    EXPECT_TRUE(HasErrorCode(result.GetError(), eError::FileOpen));
}

TEST(StageDataReal, OpenlandPowsAreAboutEvery700PxAndCoverTheWeaponMix)
{
    String            diag;
    Result<StageData> result = LoadStageFromFile(FindAssetRoot() / "Stage" / "Stage0.json", &diag);
    ASSERT_TRUE(result) << diag;
    const StageData data = result.Take();

    Vector<float>  xs;
    Vector<String> drops;
    for (const StageSection& section: data.sections)
    {
        if (section.id.find("openland_") != 0 || section.id == "openland_4")
        {
            continue;
        }
        for (const StageAction& action: section.actions)
        {
            if (action.type == eActionType::Spawn && action.spawn.kind == eEntityKind::Npc && action.spawn.def == "pow")
            {
                xs.push_back(action.spawn.x);
                drops.push_back(action.spawn.drop);
            }
        }
    }
    ASSERT_EQ(xs.size(), 5u);
    EXPECT_GE(xs.front(), 8980.f);
    EXPECT_LE(xs.back(), 12350.f);
    for (size_t i = 1; i < xs.size(); ++i)
    {
        EXPECT_GE(xs[i] - xs[i - 1], 500.f);
        EXPECT_LE(xs[i] - xs[i - 1], 1000.f);
    }
    const Vector<String> expected = { "gunitem_mg", "gunitem_rl", "bomb_supple", "gunitem_sg", "gunitem_fs" };
    EXPECT_EQ(drops, Vector<String>(expected.begin(), expected.end()));
}

TEST(StageDataReal, OpenlandHousesAndBlocksStayAlignedWithTheGroundMaskBumps)
{
    String            diag;
    Result<StageData> result = LoadStageFromFile(FindAssetRoot() / "Stage" / "Stage0.json", &diag);
    ASSERT_TRUE(result) << diag;
    const StageData data = result.Take();

    std::optional<GroundMap> mask = LoadGroundMask(FindAssetRoot() / "Ground" / "Stage0_0.png");
    ASSERT_TRUE(mask.has_value());
    mask->SetPeriodicTail(14094, 13070, 1024);
    mask->MaterializeTail(3);

    int houses = 0;
    for (const StageSection& section: data.sections)
    {
        for (const StageAction& action: section.actions)
        {
            if (action.type != eActionType::Spawn || action.spawn.kind != eEntityKind::Prop || action.spawn.x < 9000.f)
            {
                continue;
            }
            const int x = static_cast<int>(action.spawn.x);
            if (action.spawn.def == "big_house")
            {
                ++houses;
                EXPECT_TRUE(mask->IsSolid(x - 140, 1215) && mask->IsSolid(x + 150, 1215)) << "big_house x=" << x << " 가 마스크 둔덕과 어긋났다";
                EXPECT_FALSE(mask->IsSolid(x - 230, 1215)) << "big_house x=" << x;
                EXPECT_FALSE(mask->IsSolid(x + 230, 1215)) << "big_house x=" << x;
            }
            else if (action.spawn.def == "bridge_block" && x < 16400)
            {
                const bool bumpHere = mask->IsSolid(x + 21, 1235);
                if (x != 14194)
                {
                    EXPECT_TRUE(bumpHere) << "bridge_block x=" << x << " 가 마스크 둔덕과 어긋났다";
                }
            }
        }
    }
    EXPECT_GE(houses, 7);
}

TEST(StageDataReal, BossArenaStreetHasItsHousesAndBlocksAheadOfTheCamera)
{
    String            diag;
    Result<StageData> result = LoadStageFromFile(FindAssetRoot() / "Stage" / "Stage0.json", &diag);
    ASSERT_TRUE(result) << diag;
    const StageData data = result.Take();

    const StageSection& openland4 = data.sections[static_cast<size_t>(data.FindSection("openland_4"))];
    int                 hmgCount  = 0;
    int                 houses    = 0;
    int                 blocks    = 0;
    for (const StageAction& action: openland4.actions)
    {
        if (action.type != eActionType::Spawn)
        {
            continue;
        }
        if (action.spawn.kind == eEntityKind::Item && action.spawn.def == "gunitem_mg")
        {
            ++hmgCount;
            EXPECT_FLOAT_EQ(action.spawn.x, 12950.f);
        }
        if (action.spawn.kind == eEntityKind::Prop && action.spawn.def == "big_house")
        {
            ++houses;
            EXPECT_GE(action.spawn.x, 13500.f) << "아레나 집은 카메라 밖 (openland_4 시작 시 화면 오른쪽 밖) 에 미리 배치";
        }
        if (action.spawn.kind == eEntityKind::Prop && action.spawn.def == "bridge_block")
        {
            ++blocks;
        }
    }
    EXPECT_EQ(hmgCount, 1);
    EXPECT_GE(houses, 2) << "13542 와 엔드리스 꼬리 (주기 1024) 의 집";
    EXPECT_GE(blocks, 2);
    const StageSection& stageEnd = data.sections.back();
    for (const StageAction& action: stageEnd.actions)
    {
        if (action.type == eActionType::Spawn)
        {
            EXPECT_NE(action.spawn.kind, eEntityKind::Item) << "마지막 섹션은 화면에 보이는 픽업을 만들지 않는다";
        }
    }
}

TEST(StageDataReal, Stage0AmbientWalkersUseSequenceParams)
{
    String            diag;
    Result<StageData> result = LoadStageFromFile(FindAssetRoot() / "Stage" / "Stage0.json", &diag);
    ASSERT_TRUE(result) << diag;
    const StageData data = result.Take();

    const int intro = data.FindSection("intro");
    ASSERT_GE(intro, 0);
    const SpawnSpec* pOld    = nullptr;
    int              walkers = 0;
    for (const StageAction& action: data.sections[static_cast<size_t>(intro)].actions)
    {
        if (action.type == eActionType::Spawn && action.spawn.def == "anim_decor" && action.spawn.params.Has("sequence"))
        {
            ++walkers;
            if (action.spawn.anim == "Parker")
            {
                pOld = &action.spawn;
            }
        }
    }
    EXPECT_EQ(walkers, 1);
    ASSERT_NE(pOld, nullptr);
    const ParamValue* pSequence = pOld->params.Find("sequence");
    ASSERT_NE(pSequence, nullptr);
    EXPECT_EQ(pSequence->GetSize(), 2u);
    EXPECT_EQ(pSequence->At(0).GetString("clip"), "Parker");
    EXPECT_EQ(pSequence->At(1).GetString("clip"), "Parker_Run_2");
    EXPECT_FLOAT_EQ(pSequence->At(1).GetVec2("velocity").x, -45.f);
    EXPECT_FALSE(pSequence->At(1).GetBool("flip"));

    const int street = data.FindSection("street");
    ASSERT_GE(street, 0);
    int streetWalkers = 0;
    for (const StageAction& action: data.sections[static_cast<size_t>(street)].actions)
    {
        if (action.type == eActionType::Spawn && action.spawn.def == "anim_decor")
        {
            ++streetWalkers;
        }
    }
    EXPECT_EQ(streetWalkers, 1);
}

TEST(StageDataReal, BossArenaIsBehindTheSarubiaLockAndClearsOnlyAfterTheBossIsDestroyed)
{
    String            diag;
    Result<StageData> result = LoadStageFromFile(FindAssetRoot() / "Stage" / "Stage0.json", &diag);
    ASSERT_TRUE(result) << diag;
    const StageData data = result.Take();

    const StageSection& arena = data.sections.back();
    EXPECT_EQ(arena.id, "boss_arena");
    EXPECT_GT(arena.triggerX, 13066.f);
    EXPECT_EQ(data.FindSection("boss_arena"), data.FindSection("openland_4") + 1);

    ASSERT_EQ(arena.actions.size(), 4u);
    EXPECT_EQ(arena.actions[0].type, eActionType::PlayBgm);
    EXPECT_EQ(arena.actions[0].name, "Boss_BGM");
    EXPECT_FLOAT_EQ(arena.actions[0].fadeOutSec, 2.f);
    EXPECT_EQ(arena.actions[1].type, eActionType::LockCamera);
    EXPECT_EQ(arena.actions[2].type, eActionType::Spawn);
    EXPECT_EQ(arena.actions[2].spawn.def, "bigshiee");
    EXPECT_EQ(arena.actions[2].spawn.kind, eEntityKind::Enemy);
    EXPECT_EQ(arena.actions[2].spawn.id, "bigshiee");
    const StageAction& gate = arena.actions[3];
    EXPECT_EQ(gate.type, eActionType::Gate);
    EXPECT_EQ(gate.until, eGateUntil::DestroyFinished);
    ASSERT_EQ(gate.waitFor.size(), 1u);
    EXPECT_EQ(gate.waitFor[0], "bigshiee");
    ASSERT_EQ(gate.actions.size(), 4u);
    EXPECT_EQ(gate.actions[0].type, eActionType::UnlockCamera);
    EXPECT_EQ(gate.actions[3].type, eActionType::EndStage);
}

TEST(StageDataReal, RiverJeepSpawnsSixtyPixelsLeftOfTheHeliLock)
{
    String            diag;
    Result<StageData> result = LoadStageFromFile(FindAssetRoot() / "Stage" / "Stage0.json", &diag);
    ASSERT_TRUE(result) << diag;
    const StageData data   = result.Take();
    bool            bFound = false;
    for (const StageSection& section: data.sections)
    {
        for (const StageAction& action: section.actions)
        {
            if (action.type == eActionType::Spawn && action.spawn.id == "river_jeep")
            {
                bFound = true;
                EXPECT_TRUE(action.spawn.bHasX);
                EXPECT_FLOAT_EQ(action.spawn.x, 5100.f);
            }
        }
    }
    EXPECT_TRUE(bFound);
}

TEST(StageDataReal, Stage0LoadsWithExpectedCounts)
{
    String            diag;
    Result<StageData> result = LoadStageFromFile(FindAssetRoot() / "Stage" / "Stage0.json", &diag);
    ASSERT_TRUE(result) << diag;
    const StageData data = result.Take();

    EXPECT_EQ(data.sections.size(), 22u);
    EXPECT_EQ(data.CountSpawns(), 195);
    EXPECT_EQ(data.CountWaveEntries(), 33);
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
    EXPECT_EQ(data.sections.back().id, "boss_arena");
    EXPECT_FLOAT_EQ(data.sections.back().triggerX, 13700.f);
    EXPECT_EQ(data.FindSection("boardwalk"), 4);

    ASSERT_EQ(data.camera.boundsCurves.size(), 1u);
    EXPECT_EQ(data.camera.boundsCurves[0].fromIndex, data.FindSection("downtown_start"));
    EXPECT_EQ(data.camera.boundsCurves[0].untilIndex, data.FindSection("openland_start"));
    EXPECT_TRUE(data.camera.boundsCurves[0].bTrunc);

    EXPECT_TRUE(data.player.bGroundSnap);
    EXPECT_FLOAT_EQ(data.player.x, 100.f);
}

TEST(StageDataReal, Stage0KeySemantics)
{
    Result<StageData> result = LoadStageFromFile(FindAssetRoot() / "Stage" / "Stage0.json");
    ASSERT_TRUE(result);
    const StageData data = result.Take();

    const StageSection& groundSwap = data.sections[static_cast<size_t>(data.FindSection("ground_swap"))];
    ASSERT_EQ(groundSwap.actions.size(), 2u);
    EXPECT_EQ(groundSwap.actions[0].type, eActionType::SetGround);
    EXPECT_EQ(groundSwap.actions[0].when, eWhen::Silent);
    EXPECT_EQ(groundSwap.actions[1].type, eActionType::Spawn);
    EXPECT_EQ(groundSwap.actions[1].spawn.def, "bus");
    EXPECT_EQ(groundSwap.actions[1].spawn.id, "bus");

    const StageSection& arena = data.sections.back();
    ASSERT_FALSE(arena.actions.empty());
    ASSERT_EQ(arena.actions.back().type, eActionType::Gate);
    ASSERT_FALSE(arena.actions.back().actions.empty());
    EXPECT_EQ(arena.actions.back().actions.back().type, eActionType::EndStage);
    EXPECT_FLOAT_EQ(arena.actions.back().actions.back().delay, 0.f);

    const StageSection& boardwalk = data.sections[static_cast<size_t>(data.FindSection("boardwalk"))];
    bool                bFence    = false;
    for (const StageAction& action: boardwalk.actions)
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

TEST(StageDataReal, BossArenaKeepsTheUnevenTerrainWithoutFlattening)
{
    const std::filesystem::path root   = FindAssetRoot();
    std::optional<GroundMap>    loaded = LoadGroundMask(root / "Ground" / "Stage0_1.png");
    ASSERT_TRUE(loaded.has_value());
    const GroundMap ground = std::move(*loaded);
    int             bumpy  = 0;
    for (int x = 12760; x < 14090; x += 7)
    {
        EXPECT_TRUE(ground.IsSolid(x, 1290)) << "바닥 아래는 고체: x=" << x;
        bumpy += ground.IsSolid(x, 1240) ? 1 : 0;
    }
    EXPECT_GT(bumpy, 50) << "아레나 지형이 평탄화되면 안 된다 (언덕/잔해 지형 유지)";
}
