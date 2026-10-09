#include <cmath>

#include "VehicleTestUtil.h"

using namespace slugtest;

namespace
{
    // 정의 하나를 찾는다 (없으면 테스트 실패)
    const EnemyDef& Need(const EnemyDefLibrary& _lib, const char* _name)
    {
        static const EnemyDef kEmpty {};
        const EnemyDef*       pDef = _lib.Find(_name);
        EXPECT_NE(pDef, nullptr) << _name;
        return pDef ? *pDef : kEmpty;
    }

    // 클립 이름의 {side} 를 두 방향으로 치환해 애니메이션 라이브러리에서 확인한다
    void ExpectClipExists(const AnimationLibrary& _anims, const jug::String& _name, const char* _owner)
    {
        if (_name.empty())
        {
            return;
        }
        const bool bSided = _name.find("{side}") != jug::String::npos;
        for (int side = 0; side < (bSided ? 2 : 1); ++side)
        {
            const jug::String name = bSided ? ResolveSide(_name, side == 0 ? -1 : 1) : _name;
            EXPECT_TRUE(_anims.IsValid(_anims.Find(name))) << _owner << ": " << name.c_str();
        }
    }
}   // namespace

// ===========================================
//  로드 / 이름 / 수치
// ===========================================

TEST(VehicleDefs, LoadsTheDefsDirectoryWithAllFiveVehiclesAndTheirProjectiles)
{
    EnemyDefLibrary lib;
    jug::String     message;
    ASSERT_TRUE(LoadVehicleLib(lib, &message)) << message.c_str();
    for (const char* name: kVehicleNames)
    {
        const EnemyDef* pDef = lib.Find(name);
        ASSERT_NE(pDef, nullptr) << name;
        EXPECT_EQ(pDef->team, eTeam::Enemy) << name;
        EXPECT_TRUE(!pDef->weapons.empty() || std::string(name) == "aparch") << name;
    }
    for (const char* name: { "jeep_proj", "vehicle_btank_shell", "missle_proj_up", "missle_proj_down", "sarubia_grenade", "aparch_missile", "aparch_drop" })
    {
        EXPECT_NE(lib.FindProjectile(name), nullptr) << name;
    }
    // 같은 폴더의 보병 정의도 함께 올라온다 (파일 병합)
    EXPECT_NE(lib.Find("soldier_bomb"), nullptr);
}

TEST(VehicleDefs, HitPointsAndScoresFollowTheSpecTable)
{
    EnemyDefLibrary lib;
    ASSERT_TRUE(LoadVehicleLib(lib));
    EXPECT_EQ(Need(lib, "jeep").hp, 80);
    EXPECT_EQ(Need(lib, "jeep").score, 500);
    EXPECT_EQ(Need(lib, "btank").hp, 120);
    EXPECT_EQ(Need(lib, "btank").score, 1000);
    EXPECT_EQ(Need(lib, "missle_tank").hp, 100);
    EXPECT_EQ(Need(lib, "missle_tank").score, 1000);
    EXPECT_EQ(Need(lib, "sarubia").hp, 150);
    EXPECT_EQ(Need(lib, "sarubia").score, 2000);
    EXPECT_EQ(Need(lib, "aparch").hp, 120);
    EXPECT_EQ(Need(lib, "aparch").score, 2000);
}

TEST(VehicleDefs, BehaviorNamesRoundTripThroughTheLoader)
{
    EnemyDefLibrary lib;
    ASSERT_TRUE(LoadVehicleLib(lib));
    const eEnemyBehavior expected[] = { eEnemyBehavior::Jeep, eEnemyBehavior::BTank, eEnemyBehavior::MissleTank, eEnemyBehavior::Sarubia, eEnemyBehavior::Aparch };
    for (size_t i = 0; i < 5; ++i)
    {
        EXPECT_EQ(Need(lib, kVehicleNames[i]).behavior, expected[i]) << kVehicleNames[i];
        const std::optional<eEnemyBehavior> found = FindBehavior(GetBehaviorName(expected[i]));
        ASSERT_TRUE(found.has_value()) << kVehicleNames[i];
        EXPECT_EQ(*found, expected[i]) << kVehicleNames[i];
    }
}

TEST(VehicleDefs, BodiesMatchTheSpecHurtboxes)
{
    EnemyDefLibrary lib;
    ASSERT_TRUE(LoadVehicleLib(lib));
    // 지프: 75x60 @(0,-30), F=30 / 전차: 100x100 @(0,-50), F=50~75 (기록: 2.3 은 F=75 이나 발 기준 상자는 동일)
    EXPECT_FLOAT_EQ(Need(lib, "jeep").body.w, 75.f);
    EXPECT_FLOAT_EQ(Need(lib, "jeep").body.h, 60.f);
    EXPECT_FLOAT_EQ(Need(lib, "jeep").body.cy, -30.f);
    EXPECT_FLOAT_EQ(Need(lib, "btank").body.cy, -50.f);
    EXPECT_FLOAT_EQ(Need(lib, "sarubia").body.w, 100.f);
    EXPECT_FLOAT_EQ(Need(lib, "missle_tank").body.h, 100.f);
    // 헬기는 기체 중심이 원점
    EXPECT_FLOAT_EQ(Need(lib, "aparch").body.cy, 0.f);
    EXPECT_FLOAT_EQ(Need(lib, "aparch").body.w, 100.f);
}

TEST(VehicleDefs, ActivationRulesAreDataDriven)
{
    EnemyDefLibrary lib;
    ASSERT_TRUE(LoadVehicleLib(lib));
    EXPECT_FLOAT_EQ(Need(lib, "jeep").activation.marginPx, 150.f);
    EXPECT_TRUE(Need(lib, "jeep").activation.bLatch);
    // 전차는 플레이어 x 기준으로 행동이 직접 켠다 (화면 margin 을 끈다)
    EXPECT_LT(Need(lib, "btank").activation.marginPx, 0.f);
    // 미사일 전차는 항상 활성 (사거리는 행동이 본다)
    EXPECT_TRUE(Need(lib, "missle_tank").activation.bAlways);
    EXPECT_FLOAT_EQ(Need(lib, "aparch").activation.marginPx, 400.f);
    EXPECT_FLOAT_EQ(Need(lib, "sarubia").activation.marginPx, 150.f);
}

TEST(VehicleDefs, EveryVehicleWeaponReferencesARealProjectile)
{
    EnemyDefLibrary lib;
    ASSERT_TRUE(LoadVehicleLib(lib));
    for (const char* name: kVehicleNames)
    {
        for (const WeaponDef& weapon: Need(lib, name).weapons)
        {
            EXPECT_GE(weapon.projectile, 0) << name << "." << weapon.name.c_str();
            EXPECT_FALSE(weapon.bHitbox) << name << "." << weapon.name.c_str();
        }
    }
    EXPECT_EQ(Need(lib, "missle_tank").weapons.size(), 2u);
    EXPECT_EQ(Need(lib, "aparch").weapons.size(), 2u);
}

// ===========================================
//  투사체 물리 (Docs/EnemySpecs.md §2)
// ===========================================

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

TEST(VehicleDefs, AparchDropsFallFromRestAndMissilesFlyLeftAtThreeHundred)
{
    EnemyDefLibrary lib;
    ASSERT_TRUE(LoadVehicleLib(lib));
    const ProjectileDef* pDrop = lib.FindProjectile("aparch_drop");
    const ProjectileDef* pMiss = lib.FindProjectile("aparch_missile");
    ASSERT_NE(pDrop, nullptr);
    ASSERT_NE(pMiss, nullptr);
    EXPECT_FLOAT_EQ(pDrop->gravity, 300.f);
    EXPECT_FLOAT_EQ(pDrop->v0.y, 0.f);
    EXPECT_FLOAT_EQ(pDrop->size.x, 30.f);
    EXPECT_FLOAT_EQ(pDrop->size.y, 40.f);
    EXPECT_FLOAT_EQ(pMiss->speed, 300.f);
    EXPECT_FLOAT_EQ(pMiss->size.x, 50.f);
}

TEST(VehicleDefs, VehicleDeathsDeclareTheirDelayedEffectsAndRemovalTimes)
{
    EnemyDefLibrary lib;
    ASSERT_TRUE(LoadVehicleLib(lib));
    const auto CountDelayed = [](const EnemyDef& _def) {
        int count = 0;
        for (const FxDef& fx: _def.death.fx)
        {
            count += fx.delaySec > 0.f ? 1 : 0;
        }
        return count;
    };
    EXPECT_FLOAT_EQ(Need(lib, "jeep").death.removeAtSec, 1.7f);
    EXPECT_GE(CountDelayed(Need(lib, "jeep")), 5);   // 2 차 폭발 + 금속 파편
    EXPECT_FLOAT_EQ(Need(lib, "btank").death.removeAtSec, 4.f);
    EXPECT_FLOAT_EQ(Need(lib, "missle_tank").death.removeAtSec, 2.f);
    EXPECT_FLOAT_EQ(Need(lib, "sarubia").death.removeAtSec, 2.f);
    EXPECT_GE(Need(lib, "sarubia").death.fx.size(), 9u);   // 즉시 3 + 지연 6+1
    EXPECT_FLOAT_EQ(Need(lib, "btank").death.fx[0].offset.y, -40.f);   // 레거시 (0,+35) = 발 기준 -40
}

// ===========================================
//  오류 보고 / 클립 이름 / 타이밍 (Baked 가 있을 때)
// ===========================================

TEST(VehicleDefs, AnUnknownBehaviorParameterFailsTheLoadWithTheDefinitionName)
{
    const char* kJson = R"({
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
    jug::String message;
    const jug::Result<EnemyDefLibrary> lib = EnemyDefLibrary::Parse(kJson, &message);
    EXPECT_FALSE(lib);
    EXPECT_NE(message.find("probe_tank"), jug::String::npos) << message.c_str();
}

TEST(VehicleDefs, EveryVehicleClipNameExistsInTheBakedAnimations)
{
    SLUG_REQUIRE_BAKED(anims);
    EnemyDefLibrary lib;
    ASSERT_TRUE(LoadVehicleLib(lib));
    for (const char* name: kVehicleNames)
    {
        const EnemyDef& def = Need(lib, name);
        for (const NamedClip& clip: def.clips)
        {
            ExpectClipExists(anims, clip.clip.name, name);
        }
        for (const FxDef& fx: def.death.fx)
        {
            ExpectClipExists(anims, fx.clip, name);
        }
        for (const WeaponDef& weapon: def.weapons)
        {
            ExpectClipExists(anims, weapon.muzzleFx.clip, name);
        }
        if (def.overlayClip >= 0)
        {
            ExpectClipExists(anims, def.GetClip(def.overlayClip).name, name);
        }
    }
    for (const char* name: { "jeep_proj", "vehicle_btank_shell", "missle_proj_up", "missle_proj_down", "sarubia_grenade", "aparch_missile", "aparch_drop" })
    {
        const ProjectileDef* pProj = lib.FindProjectile(name);
        ASSERT_NE(pProj, nullptr) << name;
        ExpectClipExists(anims, pProj->fly.name, name);
        ExpectClipExists(anims, pProj->begin.name, name);
        ExpectClipExists(anims, pProj->land.name, name);
        ExpectClipExists(anims, pProj->explosion.clip, name);
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
        for (const NamedClip& named: Need(lib, name).clips)
        {
            const ClipDef& clip = named.clip;
            if (clip.bLoop || clip.name.empty())
            {
                continue;   // 루프 클립은 길이가 의미 없다 (상태는 타이머로 전이)
            }
            for (int side = 0; side < (clip.bSided ? 2 : 1); ++side)
            {
                const jug::String resolved = clip.bSided ? ResolveSide(clip.name, side == 0 ? -1 : 1) : clip.name;
                const AnimClip*   pAnim    = anims.GetOrNull(anims.Find(resolved));
                ASSERT_NE(pAnim, nullptr) << resolved.c_str();
                EXPECT_EQ(pAnim->GetFrameCount(), clip.frames) << resolved.c_str();
                EXPECT_NEAR(static_cast<double>(pAnim->GetFrameDurationUnits()), static_cast<double>(clip.frameUnits), 1000.0) << resolved.c_str();
                ++checked;
            }
        }
    }
    EXPECT_GT(checked, 40);
}
