// Prop 정의 테스트: Props.json 파싱 규칙과 오류 보고 검증
#include <string>

#include "Base/Json.h"
#include "Base/Types.h"
#include "PropTestUtil.h"

namespace
{
[[nodiscard]] String Wrap_(
    const String& _defs)
{
    return String { "{ \"version\": 1, \"defs\": { " } + _defs + " } }";
}

[[nodiscard]] Result<PropDefLibrary> ParseText_(
    const String& _text,
    String*       _pMessage = nullptr)
{
    return PropDefLibrary::Parse(StringView { _text.c_str(), _text.size() }, _pMessage);
}
}   // namespace

TEST(PropDef, StageDefNamesAllPresent)
{
    const PropDefLibrary&        lib   = GetPropLib();
    const ARRAY<const char*, 25> names = {
        "shop_0", "shop_1", "shop_2", "shop_3", "shop_4", "shop_5", "truck", "fence", "mob_boat", "bridge_deck", "bridge_house", "bridge_house2house", "bridge_block", "house_upper", "house_open_window", "house_cloth", "house_no_cloth", "house_yellow", "ground_mini_house", "ground_house", "big_house", "tower", "guard_post", "sign_fall", "crate"
    };
    for (const char* name: names)
    {
        EXPECT_NE(lib.Find(name), nullptr) << name;
    }
    EXPECT_EQ(lib.GetDefs().size(), names.size());
}

TEST(PropDef, ItemIdsAndGroupsMatchStageFormat)
{
    const PropDefLibrary& lib = GetPropLib();
    for (const char* id: { "gunitem_mg", "gunitem_sg", "bomb_supple", "bullet_supple", "item_letter" })
    {
        EXPECT_NE(lib.FindItem(id), nullptr) << id;
    }
    ASSERT_NE(lib.FindGroup("random_fruit"), nullptr);
    ASSERT_NE(lib.FindGroup("random_anime"), nullptr);
    ASSERT_NE(lib.FindGroup("random_score"), nullptr);
    EXPECT_EQ(lib.FindGroup("random_fruit")->names.size(), 7u);
    EXPECT_EQ(lib.FindGroup("random_anime")->names.size(), 6u);
    EXPECT_EQ(lib.FindGroup("random_score")->names.size(), 13u);
}

TEST(PropDef, ItemScoresFollowSpec)
{
    const PropDefLibrary& lib = GetPropLib();
    EXPECT_EQ(lib.FindItem("item_apple")->score, 500);
    EXPECT_EQ(lib.FindItem("item_banana")->score, 300);
    EXPECT_EQ(lib.FindItem("item_banana_bundle")->score, 1000);
    EXPECT_EQ(lib.FindItem("item_mando")->score, 3000);
    EXPECT_EQ(lib.FindItem("item_letter")->score, 1100);
    EXPECT_EQ(lib.FindItem("item_fruits")->score, 2500);
    EXPECT_TRUE(lib.FindItem("item_apple")->bRoll);
    EXPECT_FALSE(lib.FindItem("item_letter")->bRoll);
    EXPECT_EQ(lib.FindItem("bomb_supple")->amount, 10);
    EXPECT_EQ(lib.FindItem("gunitem_mg")->weapon, eWeapon::Hmg);
    EXPECT_EQ(lib.FindItem("gunitem_sg")->weapon, eWeapon::Shotgun);
}

TEST(PropDef, ShopDefHasStandBoxReplacedOnlyOnShop5)
{
    const PropDefLibrary& lib = GetPropLib();
    const PropDef*        s5  = lib.Find("shop_5");
    ASSERT_NE(s5, nullptr);
    EXPECT_EQ(s5->hp, 12);   // 권총 4발
    ASSERT_EQ(s5->parts.size(), 3u);
    EXPECT_EQ(s5->parts[2].when, ePropWhen::Destroyed);
    EXPECT_EQ(s5->parts[2].size.x, 220.f);
    EXPECT_EQ(s5->parts[2].offset.y, 130.f);
    ASSERT_EQ(s5->spawns.size(), 1u);
    EXPECT_EQ(s5->spawns[0].def.c_str(), String { "sign_fall" });
    ASSERT_EQ(s5->drops.size(), 1u);
    EXPECT_TRUE(s5->drops[0].bBounce);

    const PropDef* s0 = lib.Find("shop_0");
    EXPECT_TRUE(s0->spawns.empty());
    EXPECT_EQ(s0->parts[1].when, ePropWhen::Intact);
}

TEST(PropDef, TruckVariantsHaveLegacyYCorrection)
{
    const PropDef* truck = GetPropLib().Find("truck");
    ASSERT_NE(truck, nullptr);
    ASSERT_EQ(truck->variants.size(), 3u);
    EXPECT_EQ(truck->variants[0].dy, 0.f);
    EXPECT_EQ(truck->variants[1].dy, -10.f);
    EXPECT_EQ(truck->variants[2].dy, -17.f);
    EXPECT_EQ(truck->variants[0].parts.size(), 4u);
    EXPECT_TRUE(truck->bRemoveOnDestroy);
}

TEST(PropDef, HouseRoofIsEnemyOnlyPlatform)
{
    const PropDef* house = GetPropLib().Find("house_upper");
    ASSERT_NE(house, nullptr);
    bool bHasEnemyPlatform = false;
    for (const PropPart& part: house->parts)
    {
        bHasEnemyPlatform = bHasEnemyPlatform || part.role == ePropRole::EnemyPlatform;
    }
    EXPECT_TRUE(bHasEnemyPlatform);
    EXPECT_EQ(house->hp, 18);   // 권총 6발
}

TEST(PropDef, DamagedThresholdsAreHalfOfHp)
{
    const PropDefLibrary& lib = GetPropLib();
    EXPECT_EQ(lib.Find("ground_house")->hp, 24);
    EXPECT_EQ(lib.Find("ground_house")->damagedAtHp, 12);
    EXPECT_EQ(lib.Find("big_house")->hp, 24);
    EXPECT_EQ(lib.Find("big_house")->damagedAtHp, 12);
    EXPECT_TRUE(lib.Find("ground_house")->bHasGroundPatch);
    EXPECT_EQ(lib.Find("ground_house")->groundPatch.sprite.c_str(), String { "Stage0_House_4_PixelMap" });
}

TEST(PropDef, BoatHasSecondaryAndTwoDropGroups)
{
    const PropDef* boat = GetPropLib().Find("mob_boat");
    ASSERT_NE(boat, nullptr);
    EXPECT_EQ(boat->hp, 12);
    EXPECT_TRUE(boat->bHasSecondary);
    ASSERT_EQ(boat->drops.size(), 2u);
    EXPECT_EQ(boat->drops[0].count, 3);
    EXPECT_EQ(boat->drops[1].count, 3);
}

TEST(PropDef, DebrisClipNamesFollowSets)
{
    EXPECT_EQ(GetDebrisClipCount(eDebrisSet::Metal), 12);
    EXPECT_EQ(GetDebrisClipName(eDebrisSet::Metal, 11).c_str(), String { "Metal12" });
    EXPECT_EQ(GetDebrisClipName(eDebrisSet::Rock, 0).c_str(), String { "Rock1" });
    EXPECT_EQ(GetDebrisClipName(eDebrisSet::Wood, 5).c_str(), String { "Wood6" });
}

TEST(PropDef, ClipAndSpriteNamesExistInAssets)
{
    AnimationLibrary anims;
    SpriteLibrary    sprites;
    if (!LoadLibs(anims, sprites))
    {
        GTEST_SKIP() << "Assets 없음";
    }
    const String missing = GetPropLib().FindMissingAsset(anims, sprites);
    EXPECT_TRUE(missing.empty()) << String { missing.c_str() };
}

TEST(PropDef, RejectsUnknownKeyWithPath)
{
    String                 msg;
    Result<PropDefLibrary> r = ParseText_(Wrap_(R"("bad": { "hp": 1, "bogus": 3 })"), &msg);
    EXPECT_FALSE(static_cast<bool>(r));
    EXPECT_NE(String { msg.c_str() }.find("bogus"), String::npos);
    EXPECT_NE(String { msg.c_str() }.find("bad"), String::npos);
}

TEST(PropDef, RejectsBadRoleAndWhen)
{
    EXPECT_FALSE(static_cast<bool>(ParseText_(Wrap_(R"("a": { "hp": 1, "parts": [ { "role": "solid", "size": [1,1] } ] })"))));
    EXPECT_FALSE(static_cast<bool>(ParseText_(Wrap_(R"("a": { "hp": 1, "parts": [ { "role": "hurt", "when": "later", "size": [1,1] } ] })"))));
}

TEST(PropDef, RejectsMissingHpNegativeHpAndBadVersion)
{
    EXPECT_FALSE(static_cast<bool>(ParseText_(Wrap_(R"("a": { "idle": { "sprite": "X" } })"))));
    EXPECT_FALSE(static_cast<bool>(ParseText_(Wrap_(R"("a": { "hp": -3 })"))));
    EXPECT_FALSE(static_cast<bool>(ParseText_(R"({ "version": 2, "defs": {} })")));
}

TEST(PropDef, RejectsVisualWithBothAnimAndSprite)
{
    String msg;
    EXPECT_FALSE(static_cast<bool>(ParseText_(Wrap_(R"("a": { "hp": 1, "idle": { "anim": "A", "sprite": "B" } })"), &msg)));
    EXPECT_NE(String { msg.c_str() }.find("하나"), String::npos);
}

TEST(PropDef, RejectsUnknownDropAndSpawnReferences)
{
    EXPECT_FALSE(static_cast<bool>(ParseText_(Wrap_(R"("a": { "hp": 1, "drops": [ { "item": "nope" } ] })"))));
    EXPECT_FALSE(static_cast<bool>(ParseText_(Wrap_(R"("a": { "hp": 1, "spawns": [ { "def": "nope" } ] })"))));
}

TEST(PropDef, RoleArrayExpandsIntoOnePartPerRole)
{
    Result<PropDefLibrary> r = ParseText_(Wrap_(R"("a": { "hp": 5, "parts": [ { "role": ["hurt", "wall"], "size": [10, 20], "offset": [1, 2] } ] })"));
    ASSERT_TRUE(static_cast<bool>(r));
    const PropDef* def = r->Find("a");
    ASSERT_NE(def, nullptr);
    ASSERT_EQ(def->parts.size(), 2u);
    EXPECT_EQ(def->parts[0].role, ePropRole::Hurt);
    EXPECT_EQ(def->parts[1].role, ePropRole::Wall);
    EXPECT_EQ(def->parts[1].offset.y, 2.f);
}

TEST(PropDef, RejectsGunItemWithoutWeaponAndUnknownKind)
{
    EXPECT_FALSE(static_cast<bool>(PropDefLibrary::Parse(R"({ "version": 1, "items": { "g": { "kind": "gun", "visual": { "sprite": "S" } } } })")));
    EXPECT_FALSE(static_cast<bool>(PropDefLibrary::Parse(R"({ "version": 1, "items": { "g": { "kind": "rock", "visual": { "sprite": "S" } } } })")));
}

TEST(PropDef, SpawnParamsParseStageOverrides)
{
    const PropDefLibrary&    lib = GetPropLib();
    Result<JsonDeserializer> doc = JsonDeserializer::Load(R"({ "hp": 30, "sprites": ["A", "B"], "hitbox": { "size": [150, 75], "offset": [0, -200] }, "standBox": { "size": [230, 20], "offset": [35, 55] } })");
    ASSERT_TRUE(static_cast<bool>(doc));
    PropSpawnParams params;
    String          err;
    ASSERT_TRUE(lib.ParseSpawnParams("shop_5", doc->GetReader(), params, err)) << String { err.c_str() };
    EXPECT_EQ(params.hp, 30);
    EXPECT_EQ(params.idleSprite.c_str(), String { "A" });
    EXPECT_EQ(params.destroySprite.c_str(), String { "B" });
    ASSERT_TRUE(params.hitbox.has_value());
    EXPECT_EQ(params.hitbox->offset.y, -200.f);
    ASSERT_TRUE(params.standBox.has_value());
    EXPECT_EQ(params.standBox->size.x, 230.f);
}

TEST(PropDef, SpawnParamsRejectUnknownKeysAndBadVariant)
{
    const PropDefLibrary&    lib = GetPropLib();
    Result<JsonDeserializer> doc = JsonDeserializer::Load(R"({ "playerWaterTrigger": { "size": [1050, 40] } })");
    ASSERT_TRUE(static_cast<bool>(doc));
    PropSpawnParams params;
    String          err;
    EXPECT_FALSE(lib.ParseSpawnParams("shop_0", doc->GetReader(), params, err));
    EXPECT_NE(String { err.c_str() }.find("playerWaterTrigger"), String::npos);

    Result<JsonDeserializer> doc2 = JsonDeserializer::Load(R"({ "variant": 1 })");
    ASSERT_TRUE(static_cast<bool>(doc2));
    EXPECT_FALSE(lib.ParseSpawnParams("shop_0", doc2->GetReader(), params, err));
    EXPECT_FALSE(lib.ParseSpawnParams("nope", doc2->GetReader(), params, err));
}
