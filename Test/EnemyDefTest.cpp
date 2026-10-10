// 적 정의 파일 로드, 보병 종류별 값, 행동 이름 변환, 정의 오류 거부와 베이크된 애니메이션 일치를 검증한다.

#include <cmath>
#include <fstream>
#include <sstream>
#include <string>

#include "Base/Result.h"
#include "Base/Types.h"
#include "EnemyTestUtil.h"

using namespace slugtest;

namespace
{
[[nodiscard]] slug::String ReadDefsText_()
{
    std::ifstream     file(EnemyDefsPath().string());
    std::stringstream buffer;
    buffer << file.rdbuf();
    return buffer.str();
}

[[nodiscard]] slug::String ReplaceOnce_(
    slug::String       _text,
    const slug::String _from,
    const slug::String _to)
{
    const size_t pos = _text.find(_from);
    if (pos == slug::String::npos)
    {
        return {};
    }
    return _text.replace(pos, _from.size(), _to);
}

[[nodiscard]] slug::String ResolveSideText_(
    slug::String _name,
    const bool   _bLeft)
{
    const size_t pos = _name.find("{side}");
    if (pos != slug::String::npos)
    {
        _name.replace(pos, 6, _bLeft ? "Left" : "Right");
    }
    return _name;
}
}   // namespace

TEST(EnemyDefs, LoadsTheInfantryDefinitionsAndGlobals)
{
    EnemyDefLibrary lib;
    slug::String    message;
    ASSERT_TRUE(LoadEnemyLib(lib, &message)) << message.c_str();
    for (const char* name: kInfantryNames)
    {
        EXPECT_GE(lib.FindIndex(name), 0) << name;
    }
    EXPECT_FLOAT_EQ(lib.GetGlobals().dpsRef, 10.f);
    EXPECT_FLOAT_EQ(lib.GetGlobals().activationMarginPx, 150.f);
    EXPECT_FLOAT_EQ(lib.GetGlobals().despawnBehindCameraPx, 200.f);
}

TEST(EnemyDefs, AbstractBasesAreNotInTheLibrary)
{
    EnemyDefLibrary lib;
    ASSERT_TRUE(LoadEnemyLib(lib));
    EXPECT_EQ(lib.FindIndex("human_base"), -1);
    EXPECT_EQ(lib.FindIndex("soldier_base"), -1);
    EXPECT_EQ(lib.GetDefCount(), 8u);
}

TEST(EnemyDefs, BehaviorNamesRoundTrip)
{
    for (int i = 0; i < static_cast<int>(eEnemyBehavior::Count); ++i)
    {
        const eEnemyBehavior                behavior = static_cast<eEnemyBehavior>(i);
        const std::optional<eEnemyBehavior> found    = FindBehavior(GetBehaviorName(behavior));
        ASSERT_TRUE(found.has_value()) << GetBehaviorName(behavior);
        EXPECT_EQ(*found, behavior);
    }
    EXPECT_FALSE(FindBehavior("nope").has_value());
}

TEST(EnemyDefs, SecToTicksRoundsToNearestTick)
{
    EXPECT_EQ(SecToTicks(0.8f), 48);
    EXPECT_EQ(SecToTicks(0.5f), 30);
    EXPECT_EQ(SecToTicks(1.f / 60.f), 1);
}

TEST(EnemyDefs, EachSoldierVariantHasItsOwnAi)
{
    EnemyDefLibrary lib;
    ASSERT_TRUE(LoadEnemyLib(lib));
    const EnemyDef& bomb    = lib.Get(lib.FindIndex("soldier_bomb"));
    const EnemyDef& sword   = lib.Get(lib.FindIndex("soldier_sword"));
    const EnemyDef& missile = lib.Get(lib.FindIndex("soldier_missile"));
    EXPECT_EQ(bomb.behavior, eEnemyBehavior::Soldier);
    EXPECT_EQ(bomb.soldier.ai, eSoldierAi::Bomb);
    EXPECT_EQ(sword.soldier.ai, eSoldierAi::Sword);
    EXPECT_EQ(missile.soldier.ai, eSoldierAi::Missile);
    EXPECT_FLOAT_EQ(bomb.soldier.detect, 350.f);
    EXPECT_FLOAT_EQ(missile.soldier.detect, 400.f);
    EXPECT_FLOAT_EQ(sword.soldier.jumpAttack, 150.f);
}

TEST(EnemyDefs, SoldierVariantsShareHpOneScore100AndBody)
{
    EnemyDefLibrary lib;
    ASSERT_TRUE(LoadEnemyLib(lib));
    for (const char* name: { "soldier_bomb", "soldier_sword", "soldier_missile", "soldier_bzk", "soldier_hang", "flying_bzk", "shielder" })
    {
        const EnemyDef& def = lib.Get(lib.FindIndex(name));
        EXPECT_EQ(def.hp, 1) << name;
        EXPECT_EQ(def.score, 100) << name;
        EXPECT_FLOAT_EQ(def.body.w, 40.f) << name;
        EXPECT_FLOAT_EQ(def.body.h, 75.f) << name;
        EXPECT_FLOAT_EQ(def.body.cy, -37.5f) << name;
    }
}

TEST(EnemyDefs, EveryBodyStandsOnTheFeetOrigin)
{
    EnemyDefLibrary lib;
    ASSERT_TRUE(LoadEnemyLib(lib));
    for (const char* name: kInfantryNames)
    {
        const EnemyDef& def = lib.Get(lib.FindIndex(name));
        EXPECT_NEAR(def.body.cy + def.body.h * 0.5f, 0.f, 0.01f) << name;   // 박스 하단 = 발
    }
}

TEST(EnemyDefs, BehaviorsMatchTheirKinds)
{
    EnemyDefLibrary lib;
    ASSERT_TRUE(LoadEnemyLib(lib));
    EXPECT_EQ(lib.Get(lib.FindIndex("soldier_bzk")).behavior, eEnemyBehavior::Bzk);
    EXPECT_EQ(lib.Get(lib.FindIndex("soldier_hang")).behavior, eEnemyBehavior::Hang);
    EXPECT_EQ(lib.Get(lib.FindIndex("flying_bzk")).behavior, eEnemyBehavior::FlyingBzk);
    EXPECT_EQ(lib.Get(lib.FindIndex("shielder")).behavior, eEnemyBehavior::Shielder);
    EXPECT_EQ(lib.Get(lib.FindIndex("pow")).behavior, eEnemyBehavior::Pow);
}

TEST(EnemyDefs, HangHasNoGravityAndFlyingBzkLandsIntoSoldierBzk)
{
    EnemyDefLibrary lib;
    ASSERT_TRUE(LoadEnemyLib(lib));
    EXPECT_FALSE(lib.Get(lib.FindIndex("soldier_hang")).physics.bGravity);
    EXPECT_FALSE(lib.Get(lib.FindIndex("flying_bzk")).physics.bGravity);
    EXPECT_TRUE(lib.Get(lib.FindIndex("soldier_bzk")).physics.bGravity);
    EXPECT_EQ(lib.Get(lib.FindIndex("flying_bzk")).flyingBzk.landedDef, "soldier_bzk");
    EXPECT_GE(lib.Get(lib.FindIndex("flying_bzk")).overlayClip, 0);   // 낙하산 오버레이
}

TEST(EnemyDefs, ShielderHasFrontUpDownShieldsAndPowIsNeutral)
{
    EnemyDefLibrary lib;
    ASSERT_TRUE(LoadEnemyLib(lib));
    const EnemyDef& shielder = lib.Get(lib.FindIndex("shielder"));
    EXPECT_FALSE(shielder.shielder.shieldFront.IsEmpty());
    EXPECT_FALSE(shielder.shielder.shieldUp.IsEmpty());
    EXPECT_FALSE(shielder.shielder.shieldDown.IsEmpty());
    EXPECT_GT(shielder.shielder.shieldFront.cx, 0.f);   // 앞(바라보는 쪽) 에 있다
    EXPECT_EQ(shielder.team, eTeam::Enemy);
    EXPECT_EQ(lib.Get(lib.FindIndex("pow")).team, eTeam::Neutral);
}

TEST(EnemyDefs, ProjectilesMatchTheSpec)
{
    EnemyDefLibrary lib;
    ASSERT_TRUE(LoadEnemyLib(lib));
    const ProjectileDef* pBomb = lib.FindProjectile("soldier_bomb_proj");
    ASSERT_NE(pBomb, nullptr);
    EXPECT_FLOAT_EQ(pBomb->v0.x, 200.f);
    EXPECT_FLOAT_EQ(pBomb->v0.y, -500.f);
    EXPECT_FLOAT_EQ(pBomb->gravity, 750.f);
    EXPECT_FLOAT_EQ(pBomb->size.x, 20.f);

    const ProjectileDef* pMissile = lib.FindProjectile("soldier_missile_proj");
    ASSERT_NE(pMissile, nullptr);
    EXPECT_TRUE(pMissile->bRamp);
    EXPECT_FLOAT_EQ(pMissile->rampAccel, 400.f);
    EXPECT_FLOAT_EQ(pMissile->rampMax, 400.f);
    EXPECT_FLOAT_EQ(pMissile->size.x, 40.f);

    const ProjectileDef* pBazuka = lib.FindProjectile("bazuka_proj");
    ASSERT_NE(pBazuka, nullptr);
    EXPECT_FLOAT_EQ(pBazuka->speed, 200.f);
    EXPECT_FLOAT_EQ(pBazuka->gravity, 0.f);
}

TEST(EnemyDefs, SoldierWeaponsAndTheirHitboxesAreDeclared)
{
    EnemyDefLibrary lib;
    ASSERT_TRUE(LoadEnemyLib(lib));
    const EnemyDef& bomb   = lib.Get(lib.FindIndex("soldier_bomb"));
    const int       attack = bomb.FindWeapon("attack");
    ASSERT_GE(attack, 0);
    EXPECT_GE(bomb.weapons[static_cast<size_t>(attack)].projectile, 0);
    EXPECT_EQ(bomb.soldier.fireFrame, 10);

    const EnemyDef& sword = lib.Get(lib.FindIndex("soldier_sword"));
    const int       slash = sword.FindWeapon("slash");
    const int       jump  = sword.FindWeapon("jumpSlash");
    ASSERT_GE(slash, 0);
    ASSERT_GE(jump, 0);
    EXPECT_TRUE(sword.weapons[static_cast<size_t>(slash)].bHitbox);
    EXPECT_NEAR(sword.weapons[static_cast<size_t>(slash)].hitboxLifeSec, 0.4f, 1e-4f);
    EXPECT_TRUE(sword.weapons[static_cast<size_t>(jump)].bHitboxFollows);
    EXPECT_EQ(sword.soldier.slashFrame, 9);
}

TEST(EnemyDefs, DeathClipsAreResolvedForTheSoldierBase)
{
    EnemyDefLibrary lib;
    ASSERT_TRUE(LoadEnemyLib(lib));
    const EnemyDef& def = lib.Get(lib.FindIndex("soldier_bomb"));
    EXPECT_GE(def.death.clipBomb, 0);
    EXPECT_GE(def.death.clipSlash, 0);
    EXPECT_GE(def.death.clipStand1, 0);
    EXPECT_GE(def.death.clipStand2, 0);
    EXPECT_GE(def.death.clipFall, 0);
    EXPECT_GE(def.death.clipLand, 0);
    EXPECT_GE(def.death.clipJump, 0);
    EXPECT_EQ(def.death.fallAt1, 10);
    EXPECT_EQ(def.death.fallAt2, 8);
    EXPECT_FLOAT_EQ(def.death.blinkAtSec, 2.f);
    EXPECT_FLOAT_EQ(def.death.removeAtSec, 3.f);
}

TEST(EnemyDefs, ClipTimingMatchesTheSpecTicks)
{
    EnemyDefLibrary lib;
    ASSERT_TRUE(LoadEnemyLib(lib));
    const EnemyDef& def       = lib.Get(lib.FindIndex("soldier_bomb"));
    const ClipDef&  throwClip = def.GetClip(def.FindClip("throw"));
    EXPECT_EQ(throwClip.frames, 14);
    EXPECT_EQ(throwClip.TickOfFrame(10), 48);      // f10 = 0.80 s
    EXPECT_EQ(throwClip.GetDurationTicks(), 68);   // 14 * 0.08 s = 67.2 틱 → 올림 68
    EXPECT_TRUE(def.GetClip(def.FindClip("idle")).bLoop);
}

TEST(EnemyDefsErrors, RealFileParsesWithoutMessage)
{
    slug::String                        message;
    const slug::Result<EnemyDefLibrary> lib = EnemyDefLibrary::Parse(ReadDefsText_(), &message);
    EXPECT_TRUE(static_cast<bool>(lib)) << message.c_str();
}

TEST(EnemyDefsErrors, UnknownKeyInADefinitionIsRejected)
{
    const slug::String text = ReplaceOnce_(ReadDefsText_(), "\"behavior\": \"pow\",", "\"behavior\": \"pow\", \"bogus\": 1,");
    ASSERT_FALSE(text.empty());
    slug::String message;
    EXPECT_FALSE(static_cast<bool>(EnemyDefLibrary::Parse(text, &message)));
    EXPECT_NE(message.find("pow"), slug::String::npos) << message.c_str();
}

TEST(EnemyDefsErrors, MissingRequiredClipIsRejected)
{
    const slug::String text = ReplaceOnce_(ReadDefsText_(), "\"throw\": { \"name\": \"SD_Throw_Bomb_{side}\"", "\"throwX\": { \"name\": \"SD_Throw_Bomb_{side}\"");
    ASSERT_FALSE(text.empty());
    slug::String message;
    EXPECT_FALSE(static_cast<bool>(EnemyDefLibrary::Parse(text, &message)));
    EXPECT_NE(message.find("soldier_bomb"), slug::String::npos) << message.c_str();
}

TEST(EnemyDefsErrors, UnknownBehaviorIsRejected)
{
    const slug::String text = ReplaceOnce_(ReadDefsText_(), "\"behavior\": \"pow\"", "\"behavior\": \"nope\"");
    ASSERT_FALSE(text.empty());
    EXPECT_FALSE(static_cast<bool>(EnemyDefLibrary::Parse(text)));
}

TEST(EnemyDefsErrors, UnusedBehaviorParamIsRejected)
{
    const slug::String text = ReplaceOnce_(ReadDefsText_(), "\"detect\": 350, \"closeStop\"", "\"detect\": 350, \"zzz\": 1, \"closeStop\"");
    ASSERT_FALSE(text.empty());
    slug::String message;
    EXPECT_FALSE(static_cast<bool>(EnemyDefLibrary::Parse(text, &message)));
    EXPECT_NE(message.find("zzz"), slug::String::npos) << message.c_str();
}

TEST(EnemyDefsErrors, UnknownWeaponProjectileIsRejected)
{
    const slug::String text = ReplaceOnce_(ReadDefsText_(), "\"projectile\": \"soldier_bomb_proj\"", "\"projectile\": \"nope\"");
    ASSERT_FALSE(text.empty());
    EXPECT_FALSE(static_cast<bool>(EnemyDefLibrary::Parse(text)));
}

TEST(EnemyDefsErrors, FlyingBzkWithoutItsLandedDefIsRejected)
{
    const slug::String text = ReplaceOnce_(ReadDefsText_(), "\"landedDef\": \"soldier_bzk\"", "\"landedDef\": \"nope\"");
    ASSERT_FALSE(text.empty());
    EXPECT_FALSE(static_cast<bool>(EnemyDefLibrary::Parse(text)));
}

TEST(EnemyDefsErrors, DuplicateDefinitionNameIsRejected)
{
    slug::String       text   = ReadDefsText_();
    const slug::String anchor = "\"pow\": {";
    const slug::String extra  = "\"pow\": { \"kind\": \"npc\", \"behavior\": \"pow\", \"hp\": 1, \"body\": { \"w\": 50, \"h\": 80, \"cx\": 0, \"cy\": -40 } }, ";
    text                      = ReplaceOnce_(text, anchor, extra + anchor);
    ASSERT_FALSE(text.empty());
    EXPECT_FALSE(static_cast<bool>(EnemyDefLibrary::Parse(text)));
}

TEST(EnemyDefsBaked, EveryDefinedClipExistsInTheAnimationLibrary)
{
    SLUG_REQUIRE_BAKED(anims);
    EnemyDefLibrary lib;
    ASSERT_TRUE(LoadEnemyLib(lib));
    for (const EnemyDef& def: lib.GetDefs())
    {
        for (const NamedClip& named: def.clips)
        {
            const bool bSided = named.clip.bSided;
            for (const bool bLeft: { true, false })
            {
                const slug::String name = bSided ? ResolveSideText_(named.clip.name.c_str(), bLeft) : named.clip.name.c_str();
                EXPECT_TRUE(anims.IsValid(anims.Find(name))) << def.name.c_str() << ": " << name;
                if (!bSided)
                {
                    break;
                }
            }
        }
    }
    for (const ProjectileDef& proj: lib.GetProjectiles())
    {
        for (const bool bLeft: { true, false })
        {
            const slug::String fly = ResolveSideText_(proj.fly.name.c_str(), bLeft);
            EXPECT_TRUE(anims.IsValid(anims.Find(fly))) << proj.name.c_str() << ": " << fly;
        }
    }
    for (const EnemyDef& def: lib.GetDefs())
    {
        for (const FxDef& fx: def.death.fx)
        {
            EXPECT_TRUE(anims.IsValid(anims.Find(ResolveSideText_(fx.clip.c_str(), true)))) << def.name.c_str();
        }
    }
}

TEST(EnemyDefsBaked, ClipFrameCountAndTimingMatchTheBakedAnimation)
{
    SLUG_REQUIRE_BAKED(anims);
    EnemyDefLibrary lib;
    ASSERT_TRUE(LoadEnemyLib(lib));
    for (const EnemyDef& def: lib.GetDefs())
    {
        for (const NamedClip& named: def.clips)
        {
            if (named.clip.name.find("{side}") == slug::String::npos)
            {
                continue;
            }
            const AnimClip* pLeft  = anims.FindClip(ResolveSideText_(named.clip.name.c_str(), true));
            const AnimClip* pRight = anims.FindClip(ResolveSideText_(named.clip.name.c_str(), false));
            ASSERT_NE(pLeft, nullptr) << named.clip.name.c_str();
            const bool bLeftOk  = pLeft->GetFrameCount() == named.clip.frames && std::abs(pLeft->frameDuration - named.clip.frameSec) < 0.002f;
            const bool bRightOk = pRight && pRight->GetFrameCount() == named.clip.frames && std::abs(pRight->frameDuration - named.clip.frameSec) < 0.002f;
            EXPECT_TRUE(bLeftOk || bRightOk) << def.name.c_str() << "." << named.name.c_str() << " frames " << named.clip.frames << " sec " << named.clip.frameSec;
        }
    }
}
