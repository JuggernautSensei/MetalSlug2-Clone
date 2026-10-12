// 중력 특성 테스트: 지상형 정의는 공중 스폰 시 지면으로 떨어지고, 비중력 정의는 떨어지지 않는다
#include "VehicleTestUtil.h"

struct GravityCase
{
    const char* name     = "";
    bool        bGravity = true;
    bool        bSpawn   = true;   // false 면 플래그만 검사 (비행체/보스는 자체 이동이 있다)
};

static constexpr ARRAY<GravityCase, 15> kCases = {
    {
     { "soldier_bomb", true, true },
     { "soldier_sword", true, true },
     { "soldier_missile", true, true },
     { "soldier_bzk", true, true },
     { "shielder", true, true },
     { "pow", true, true },
     { "jeep", true, true },
     { "btank", true, true },
     { "missle_tank", true, true },
     { "sarubia", true, true },
     { "soldier_hang", false, true },
     { "flying_bzk", false, false },
     { "heli", false, false },
     { "heli_bomb", false, false },
     { "bigshiee", false, false },
     }
};

TEST(GravityTrait, DefFlagFollowsTheRule)
{
    VehicleRig rig;
    ASSERT_TRUE(rig.IsLoaded()) << rig.GetMessage().c_str();
    for (const GravityCase& c: kCases)
    {
        const EnemyDef* pDef = rig.Lib().Find(c.name);
        ASSERT_NE(pDef, nullptr) << c.name;
        EXPECT_EQ(pDef->physics.bGravity, c.bGravity) << c.name;
    }
}

TEST(GravityTrait, AirSpawnFallsToGroundOnlyWhenGravityIsOn)
{
    for (const GravityCase& c: kCases)
    {
        if (!c.bSpawn)
        {
            continue;
        }
        VehicleRig rig;
        ASSERT_TRUE(rig.IsLoaded()) << rig.GetMessage().c_str();
        const float       startY = kVehicleGroundY - 80.f;
        const EnemyHandle handle = rig.Spawn(c.name, { 300.f, startY });
        rig.Step(120);
        const Enemy* pEnemy = rig.E(handle);
        ASSERT_NE(pEnemy, nullptr) << c.name;
        if (c.bGravity)
        {
            EXPECT_TRUE(pEnemy->bGrounded) << c.name;
            EXPECT_NEAR(pEnemy->body.position.y, kVehicleGroundY, 1.f) << c.name;
        }
        else
        {
            EXPECT_NEAR(pEnemy->body.position.y, startY, 1.f) << c.name;
        }
    }
}
