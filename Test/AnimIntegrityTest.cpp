// 애니메이션 데이터 무결성 테스트 (Docs/AnimAudit.md): 클립 오매핑/좌우 피벗 튐/아틀라스 겹침/스테이지 NPC 클립 혼입을 잡는다.
// Assets 가 없으면 건너뛴다.
#include <algorithm>
#include <cmath>
#include <map>
#include <set>
#include <vector>

#include <gtest/gtest.h>

#include "Base/Json.h"
#include "Base/Types.h"
#include "Core/AssetPath.h"
#include "EnemyTestUtil.h"
#include "Game/Enemies/EnemyCommon.h"
#include "Game/Enemies/EnemyDef.h"
#include "Render/Animation.h"

namespace
{
[[nodiscard]] bool Load_(
    AnimationLibrary& _out)
{
    Result<AnimationLibrary> lib = AnimationLibrary::LoadFromDirectory(FindAssetRoot());
    if (!lib)
    {
        return false;
    }
    _out = lib.Take();
    return true;
}

// 클립 이름의 첫 토큰 (엔티티 계열): "MissleTank_Up_Idle_Left" -> "MissleTank"
[[nodiscard]] String FamilyOf_(
    const String& _name)
{
    const size_t pos = _name.find('_');
    return pos == String::npos ? _name : _name.substr(0, pos);
}

[[nodiscard]] bool BottomAnchor_(
    const AnimClip& _clip,
    int&            _out)
{
    bool bAny = false;
    int  low  = 0;
    for (const AnimFrame& f: _clip.frames)
    {
        if (f.IsEmpty())
        {
            continue;
        }
        const int bottom = f.h - f.pivotY;
        low              = bAny ? Max(low, bottom) : bottom;
        bAny             = true;
    }
    _out = low;
    return bAny;
}
}   // namespace

TEST(AnimIntegrity, AtlasRectanglesNeverOverlapAndStayInsidePages)
{
    AnimationLibrary lib;
    if (!Load_(lib))
    {
        GTEST_SKIP() << "Assets 없음";
    }

    RbMap<int, RbSet<ARRAY<int, 4>>> byPage;
    for (const AnimClip& clip: lib.GetClips())
    {
        for (const AnimFrame& f: clip.frames)
        {
            if (!f.IsEmpty())
            {
                EXPECT_LE(f.x + f.w, 2048) << clip.name.c_str();
                EXPECT_LE(f.y + f.h, 2048) << clip.name.c_str();
                byPage[f.atlas].insert(ARRAY<int, 4> { f.x, f.y, f.w, f.h });
            }
        }
    }
    for (const std::pair<const int, RbSet<ARRAY<int, 4>>>& page: byPage)
    {
        const Vector<ARRAY<int, 4>> rects(page.second.begin(), page.second.end());
        for (size_t i = 0; i < rects.size(); ++i)
        {
            const ARRAY<int, 4>& a = rects[i];
            for (size_t j = i + 1; j < rects.size(); ++j)
            {
                const ARRAY<int, 4>& b        = rects[j];
                const bool           bOverlap = a[0] < b[0] + b[2] && b[0] < a[0] + a[2] && a[1] < b[1] + b[3] && b[1] < a[1] + a[3];
                ASSERT_FALSE(bOverlap) << "page " << page.first << " rect " << a[0] << "," << a[1] << " vs " << b[0] << "," << b[1];
            }
        }
    }
}

TEST(AnimIntegrity, LeftRightTwinsKeepTheSameFootAnchor)
{
    AnimationLibrary lib;
    if (!Load_(lib))
    {
        GTEST_SKIP() << "Assets 없음";
    }
    int checked = 0;
    for (const AnimClip& clip: lib.GetClips())
    {
        const String suffix = "_Left";
        if (clip.name.size() <= suffix.size() || clip.name.compare(clip.name.size() - suffix.size(), suffix.size(), suffix) != 0)
        {
            continue;
        }
        const String    twinName = clip.name.substr(0, clip.name.size() - suffix.size()) + "_Right";
        const AnimClip* pTwin    = lib.FindClip(twinName);
        if (!pTwin)
        {
            continue;
        }
        int left  = 0;
        int right = 0;
        if (!BottomAnchor_(clip, left) || !BottomAnchor_(*pTwin, right))
        {
            continue;
        }
        ++checked;
        // 오타(OffsetY -430 등)는 수백 px 튄다. 정상 차이는 두 자릿수 이하다 (전함 이펙트 34px 가 최대).
        EXPECT_LE(Abs(left - right), 64) << clip.name.c_str() << " vs " << twinName.c_str();
        EXPECT_EQ(clip.bLoop, pTwin->bLoop) << clip.name.c_str();
    }
    EXPECT_GT(checked, 100);
}

TEST(AnimIntegrity, EveryDefClipExistsWithMatchingFrameCountAndSingleFamily)
{
    AnimationLibrary lib;
    if (!Load_(lib))
    {
        GTEST_SKIP() << "Assets 없음";
    }

    String                  message;
    Result<EnemyDefLibrary> loaded = EnemyDefLibrary::LoadDirectory(FindAssetRoot() / "Defs", &message);
    ASSERT_TRUE(loaded) << message.c_str();
    const EnemyDefLibrary defs = loaded.Take();

    // 정의가 여러 계열의 클립을 쓰는 것이 의도인 경우만 허용한다
    const RbSet<String> mixedOk = { "pow" };

    for (const EnemyDef& def: defs.GetDefs())
    {
        RbSet<String> families;
        for (const NamedClip& named: def.clips)
        {
            const ClipDef& clip = named.clip;
            for (int side = 0; side < (clip.bSided ? 2 : 1); ++side)
            {
                const String    name = clip.bSided ? ResolveSide(clip.name, side == 0 ? -1 : 1) : clip.name;
                const AnimClip* p    = lib.FindClip(name);
                ASSERT_NE(p, nullptr) << def.name.c_str() << ": " << name.c_str();
                EXPECT_EQ(p->GetFrameCount(), clip.frames) << def.name.c_str() << ": " << name.c_str();
            }
            families.insert(FamilyOf_(clip.name));
        }
        // 한 정의의 몸 클립은 한 계열이어야 한다
        if (def.clips.empty() || mixedOk.count(def.name) > 0)
        {
            continue;
        }
        // 인간형의 공통 사망 클립(SD_Die_*, human_base 상속)은 몸 계열과 따로 센다.
        if (families.size() > 1)
        {
            families.erase("SD");
        }
        EXPECT_EQ(families.size(), 1u) << def.name.c_str() << " 의 클립 계열이 섞였다";
    }
}

TEST(AnimIntegrity, StageDecorSequencesStayWithinOneActor)
{
    AnimationLibrary lib;
    if (!Load_(lib))
    {
        GTEST_SKIP() << "Assets 없음";
    }
    Result<JsonDeserializer> doc = LoadJsonFile(FindAssetRoot() / "Stage" / "Stage0.json");
    ASSERT_TRUE(doc);

    const RbMap<String, RbSet<String>> actors = {
        { "Parker",                                                                { "Parker", "Parker_Run", "Parker_Run_2", "Parker_Stopping" } },
        {    "Kid", { "Order_Boy_Run_Left", "Order_Boy_Run_Right", "Order_Idle", "Order_Turn", "Order_Run_2", "Order_Stopping", "Order_Recoil" } },
    };

    int                      decorCount = 0;
    const Result<JsonReader> sections   = doc->GetReader().FindField("sections");
    ASSERT_TRUE(sections);
    for (const JsonReader section: sections->GetArray())
    {
        const Result<JsonReader> actions = TryField(section, "actions");
        if (!actions)
        {
            continue;
        }
        for (const JsonReader action: actions->GetArray())
        {
            const Result<String> kind = TryRead<String>(action, "kind");
            const Result<String> anim = TryRead<String>(action, "anim");
            if (!kind || kind.GetValue() != "decor" || !anim)
            {
                continue;
            }
            ++decorCount;
            const AnimClip* pStart = lib.FindClip(anim.GetValue());
            ASSERT_NE(pStart, nullptr) << anim.GetValue().c_str();

            const String             actor  = anim.GetValue().rfind("Parker", 0) == 0 ? "Parker" : (anim.GetValue().rfind("Order_", 0) == 0 ? "Kid" : "");
            const Result<JsonReader> params = TryField(action, "params");
            if (!params)
            {
                continue;
            }
            const Result<JsonReader> sequence = TryField(*params, "sequence");
            if (!sequence)
            {
                continue;
            }
            for (const JsonReader step: sequence->GetArray())
            {
                const Result<String> clip = step.ReadField<String>("clip");
                ASSERT_TRUE(clip);
                EXPECT_NE(lib.FindClip(clip.GetValue()), nullptr) << "시퀀스 클립 없음: " << clip.GetValue().c_str();
                if (!actor.empty())
                {
                    EXPECT_GT(actors.at(actor).count(clip.GetValue()), 0u) << actor.c_str() << " 시퀀스에 다른 배우의 클립: " << clip.GetValue().c_str();
                }
            }
        }
    }
    EXPECT_GE(decorCount, 3);
}
