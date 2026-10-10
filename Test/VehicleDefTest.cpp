// 탈것 정의 테스트: 적 정의 JSON 로드와 탈것별 필드 검증
#include <cmath>

#include "Base/Result.h"
#include "Base/Types.h"
#include "VehicleTestUtil.h"

using namespace slugtest;

namespace
{
[[nodiscard]] const EnemyDef& Need_(
    const EnemyDefLibrary& _lib,
    const char*            _name)
{
    static const EnemyDef kEmpty {};
    const EnemyDef*       pDef = _lib.Find(_name);
    EXPECT_NE(pDef, nullptr) << _name;
    return pDef ? *pDef : kEmpty;
}

// 지연(delaySec > 0) 이펙트가 몇 개인지 센다
[[nodiscard]] int CountDelayed_(
    const EnemyDef& _def)
{
    int count = 0;
    for (const FxDef& fx: _def.death.fx)
    {
        count += fx.delaySec > 0.f ? 1 : 0;
    }
    return count;
}

void ExpectClipExists_(
    const AnimationLibrary& _anims,
    const slug::String&     _name,
    const char*             _owner)
{
    if (_name.empty())
    {
        return;
    }
    const bool bSided = _name.find("{side}") != slug::String::npos;
    for (int side = 0; side < (bSided ? 2 : 1); ++side)
    {
        const slug::String name = bSided ? ResolveSide(_name, side == 0 ? -1 : 1) : _name;
        EXPECT_TRUE(_anims.IsValid(_anims.Find(name))) << _owner << ": " << name.c_str();
    }
}
}   // namespace

TEST(VehicleDefs, LoadsTheDefsDirectoryWithAllFourVehiclesAndTheirProjectiles)
{
    EnemyDefLibrary lib;
    slug::String    message;
    ASSERT_TRUE(LoadVehicleLib(lib, &message)) << message.c_str();
    for (const char* name: kVehicleNames)
    {
        const EnemyDef* pDef = lib.Find(name);
        ASSERT_NE(pDef, nullptr) << name;
        EXPECT_EQ(pDef->team, eTeam::Enemy) << name;
        EXPECT_FALSE(pDef->weapons.empty()) << name;
    }
    for (const char* name: { "jeep_proj", "vehicle_btank_shell", "missle_proj_up", "missle_proj_down", "sarubia_grenade" })
    {
        EXPECT_NE(lib.FindProjectile(name), nullptr) << name;
    }
    EXPECT_NE(lib.Find("soldier_bomb"), nullptr);
}

TEST(VehicleDefs, HitPointsAndScoresFollowTheSpecTable)
{
    EnemyDefLibrary lib;
    ASSERT_TRUE(LoadVehicleLib(lib));
    EXPECT_EQ(Need_(lib, "jeep").hp, 80);
    EXPECT_EQ(Need_(lib, "jeep").score, 500);
    EXPECT_EQ(Need_(lib, "btank").hp, 120);
    EXPECT_EQ(Need_(lib, "btank").score, 1000);
    EXPECT_EQ(Need_(lib, "missle_tank").hp, 100);
    EXPECT_EQ(Need_(lib, "missle_tank").score, 1000);
    EXPECT_EQ(Need_(lib, "sarubia").hp, 150);
    EXPECT_EQ(Need_(lib, "sarubia").score, 2000);
}

TEST(VehicleDefs, BehaviorNamesRoundTripThroughTheLoader)
{
    EnemyDefLibrary lib;
    ASSERT_TRUE(LoadVehicleLib(lib));
    const ARRAY<eEnemyBehavior, 4> expected = { eEnemyBehavior::Jeep, eEnemyBehavior::BTank, eEnemyBehavior::MissleTank, eEnemyBehavior::Sarubia };
    for (size_t i = 0; i < 4; ++i)
    {
        EXPECT_EQ(Need_(lib, kVehicleNames[i]).behavior, expected[i]) << kVehicleNames[i];
        const std::optional<eEnemyBehavior> found = FindBehavior(GetBehaviorName(expected[i]));
        ASSERT_TRUE(found.has_value()) << kVehicleNames[i];
        EXPECT_EQ(*found, expected[i]) << kVehicleNames[i];
    }
}

TEST(VehicleDefs, BodiesMatchTheSpecHurtboxes)
{
    EnemyDefLibrary lib;
    ASSERT_TRUE(LoadVehicleLib(lib));
    EXPECT_FLOAT_EQ(Need_(lib, "jeep").body.w, 75.f);
    EXPECT_FLOAT_EQ(Need_(lib, "jeep").body.h, 60.f);
    EXPECT_FLOAT_EQ(Need_(lib, "jeep").body.cy, -30.f);
    EXPECT_FLOAT_EQ(Need_(lib, "btank").body.cy, -50.f);
    EXPECT_FLOAT_EQ(Need_(lib, "sarubia").body.w, 100.f);
    EXPECT_FLOAT_EQ(Need_(lib, "missle_tank").body.h, 100.f);
}

TEST(VehicleDefs, ActivationRulesAreDataDriven)
{
    EnemyDefLibrary lib;
    ASSERT_TRUE(LoadVehicleLib(lib));
    EXPECT_FLOAT_EQ(Need_(lib, "jeep").activation.marginPx, 150.f);
    EXPECT_TRUE(Need_(lib, "jeep").activation.bLatch);
    EXPECT_LT(Need_(lib, "btank").activation.marginPx, 0.f);
    EXPECT_TRUE(Need_(lib, "missle_tank").activation.bAlways);
    EXPECT_FLOAT_EQ(Need_(lib, "sarubia").activation.marginPx, 150.f);
}

TEST(VehicleDefs, EveryVehicleWeaponReferencesARealProjectile)
{
    EnemyDefLibrary lib;
    ASSERT_TRUE(LoadVehicleLib(lib));
    for (const char* name: kVehicleNames)
    {
        for (const WeaponDef& weapon: Need_(lib, name).weapons)
        {
            EXPECT_GE(weapon.projectile, 0) << name << "." << weapon.name.c_str();
            EXPECT_FALSE(weapon.bHitbox) << name << "." << weapon.name.c_str();
        }
    }
    EXPECT_EQ(Need_(lib, "missle_tank").weapons.size(), 2u);
}

TEST(VehicleDefs, RocketHasTheLegacyIgnitionDelayAndBeginClip)
{
    EnemyDefLibrary lib;
    ASSERT_TRUE(LoadVehicleLib(lib));
    const ProjectileDef* pProj = lib.FindProjectile("jeep_proj");
    ASSERT_NE(pProj, nullptr);
    EXPECT_FLOAT_EQ(pProj->speed, 300.f);
    EXPECT_FLOAT_EQ(pProj->startDelaySec, 0.4f);
    EXPECT_FALSE(pProj->begin.name.empty());
    EXPECT_FLOAT_EQ(pProj->size.x, 50.f);
    EXPECT_FLOAT_EQ(pProj->size.y, 20.f);
}

TEST(VehicleDefs, BTankShellIsALobWithFiveHundredGravity)
{
    EnemyDefLibrary lib;
    ASSERT_TRUE(LoadVehicleLib(lib));
    const ProjectileDef* pProj = lib.FindProjectile("vehicle_btank_shell");
    ASSERT_NE(pProj, nullptr);
    EXPECT_FLOAT_EQ(pProj->v0.x, 400.f);
    EXPECT_FLOAT_EQ(pProj->v0.y, -200.f);
    EXPECT_FLOAT_EQ(pProj->gravity, 500.f);
    EXPECT_FLOAT_EQ(pProj->maxFall, 500.f);
}

TEST(VehicleDefs, MissileTankVolleysUseADirectAndAnArcedProjectile)
{
    EnemyDefLibrary lib;
    ASSERT_TRUE(LoadVehicleLib(lib));
    const ProjectileDef* pDown = lib.FindProjectile("missle_proj_down");
    const ProjectileDef* pUp   = lib.FindProjectile("missle_proj_up");
    ASSERT_NE(pDown, nullptr);
    ASSERT_NE(pUp, nullptr);
    EXPECT_FLOAT_EQ(pDown->v0.y, 0.f);
    EXPECT_FLOAT_EQ(pUp->v0.y, -200.f);
    EXPECT_FLOAT_EQ(pDown->gravity, 400.f);
    EXPECT_FLOAT_EQ(pUp->gravity, 400.f);
    EXPECT_FLOAT_EQ(pDown->v0.x, 300.f);
}

TEST(VehicleDefs, RollingGrenadeDeclaresTheSpecFallRollAndExpiry)
{
    EnemyDefLibrary lib;
    ASSERT_TRUE(LoadVehicleLib(lib));
    const ProjectileDef* pProj = lib.FindProjectile("sarubia_grenade");
    ASSERT_NE(pProj, nullptr);
    EXPECT_FLOAT_EQ(pProj->gravity, 300.f);
    EXPECT_FLOAT_EQ(pProj->maxFall, 300.f);
    EXPECT_FLOAT_EQ(pProj->rollSpeed, 100.f);
    EXPECT_FLOAT_EQ(pProj->rollSec, 5.f);
    EXPECT_TRUE(pProj->bExpireFx);
    EXPECT_FALSE(pProj->land.name.empty());
    EXPECT_EQ(pProj->explosion.clip, "PlayerBombExplosion");
}

TEST(VehicleDefs, VehicleDeathsDeclareTheirDelayedEffectsAndRemovalTimes)
{
    EnemyDefLibrary lib;
    ASSERT_TRUE(LoadVehicleLib(lib));
    EXPECT_FLOAT_EQ(Need_(lib, "jeep").death.removeAtSec, 1.7f);
    EXPECT_GE(CountDelayed_(Need_(lib, "jeep")), 5);   // 2 차 폭발 + 금속 파편
    EXPECT_FLOAT_EQ(Need_(lib, "btank").death.removeAtSec, 4.f);
    EXPECT_FLOAT_EQ(Need_(lib, "missle_tank").death.removeAtSec, 2.f);
    EXPECT_FLOAT_EQ(Need_(lib, "sarubia").death.removeAtSec, 2.f);
    EXPECT_GE(Need_(lib, "sarubia").death.fx.size(), 9u);               // 즉시 3 + 지연 6+1
    EXPECT_FLOAT_EQ(Need_(lib, "btank").death.fx[0].offset.y, -40.f);   // 레거시 (0,+35) = 발 기준 -40
}

TEST(VehicleDefs, AnUnknownBehaviorParameterFailsTheLoadWithTheDefinitionName)
{
    const char*                         kJson = R"({
      "version": 1,
      "defs": {
        "dummy_shell": { "kind": "projectile", "box": { "w": 10, "h": 10 }, "speed": 100,
                          "anim": { "fly": { "name": "X", "frames": 1, "sec": 0.1 } }, "onSolid": "none" },
        "probe_tank": {
          "kind": "vehicle", "behavior": "btank", "hp": 10, "score": 0,
          "body": { "w": 10, "h": 10, "cx": 0, "cy": -5 },
          "clips": { "idle": { "name": "A", "frames": 1, "sec": 0.1 }, "open": { "name": "B", "frames": 1, "sec": 0.1 },
                     "fire": { "name": "C", "frames": 1, "sec": 0.1 }, "destroy": { "name": "D", "frames": 1, "sec": 0.1 } },
          "weapons": { "shell": { "projectile": "dummy_shell", "muzzle": [0, 0], "aim": "facing" } },
          "params": { "activateDx": 500, "idleSec": 3, "shotsPerCycle": 2, "fireFrame": 2, "bogusKey": 1 }
        }
      }
    })";
    slug::String                        message;
    const slug::Result<EnemyDefLibrary> lib = EnemyDefLibrary::Parse(kJson, &message);
    EXPECT_FALSE(lib);
    EXPECT_NE(message.find("probe_tank"), slug::String::npos) << message.c_str();
}

TEST(VehicleDefs, EveryVehicleClipNameExistsInTheBakedAnimations)
{
    SLUG_REQUIRE_BAKED(anims);
    EnemyDefLibrary lib;
    ASSERT_TRUE(LoadVehicleLib(lib));
    for (const char* name: kVehicleNames)
    {
        const EnemyDef& def = Need_(lib, name);
        for (const NamedClip& clip: def.clips)
        {
            ExpectClipExists_(anims, clip.clip.name, name);
        }
        for (const FxDef& fx: def.death.fx)
        {
            ExpectClipExists_(anims, fx.clip, name);
        }
        for (const WeaponDef& weapon: def.weapons)
        {
            ExpectClipExists_(anims, weapon.muzzleFx.clip, name);
        }
        if (def.overlayClip >= 0)
        {
            ExpectClipExists_(anims, def.GetClip(def.overlayClip).name, name);
        }
    }
    for (const char* name: { "jeep_proj", "vehicle_btank_shell", "missle_proj_up", "missle_proj_down", "sarubia_grenade" })
    {
        const ProjectileDef* pProj = lib.FindProjectile(name);
        ASSERT_NE(pProj, nullptr) << name;
        ExpectClipExists_(anims, pProj->fly.name, name);
        ExpectClipExists_(anims, pProj->begin.name, name);
        ExpectClipExists_(anims, pProj->land.name, name);
        ExpectClipExists_(anims, pProj->explosion.clip, name);
    }
}

TEST(VehicleDefs, NonLoopClipTimingsMatchTheBakedAnimations)
{
    SLUG_REQUIRE_BAKED(anims);
    EnemyDefLibrary lib;
    ASSERT_TRUE(LoadVehicleLib(lib));
    int checked = 0;
    for (const char* name: kVehicleNames)
    {
        for (const NamedClip& named: Need_(lib, name).clips)
        {
            const ClipDef& clip = named.clip;
            if (clip.bLoop || clip.name.empty())
            {
                continue;
            }
            for (int side = 0; side < (clip.bSided ? 2 : 1); ++side)
            {
                const slug::String resolved = clip.bSided ? ResolveSide(clip.name, side == 0 ? -1 : 1) : clip.name;
                const AnimClip*    pAnim    = anims.GetOrNull(anims.Find(resolved));
                ASSERT_NE(pAnim, nullptr) << resolved.c_str();
                EXPECT_EQ(pAnim->GetFrameCount(), clip.frames) << resolved.c_str();
                EXPECT_NEAR(static_cast<double>(pAnim->GetFrameDurationUnits()), static_cast<double>(clip.frameUnits), 1000.0) << resolved.c_str();
                ++checked;
            }
        }
    }
    EXPECT_GT(checked, 40);
}
