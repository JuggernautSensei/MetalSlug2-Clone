#include <cmath>

#include "Base/Types.h"
#include "VehicleTestUtil.h"

constexpr float kFloorY = 378.f;

namespace
{
[[nodiscard]] ShotSpawn MakeRoller_(
    float _dirX,
    int   _rollTicks,
    bool  _bExpireFx)
{
    ShotSpawn spawn;
    spawn.team            = eTeam::Enemy;
    spawn.owner           = 5;
    spawn.position        = { 300.f, 200.f };
    spawn.gravity         = 300.f;
    spawn.maxFallSpeed    = 300.f;
    spawn.size            = { 25.f, 25.f };
    spawn.damage          = 1;
    spawn.lifetimeTicks   = 1000;
    spawn.bExplodeOnSolid = false;
    spawn.rollSpeed       = 100.f;
    spawn.rollDirX        = _dirX;
    spawn.rollTicks       = _rollTicks;
    spawn.bExpireFx       = _bExpireFx;
    spawn.visual          = 3;
    return spawn;
}
}   // namespace

struct Field
{
    GroundMap        ground = MakeFlatGround(1200, 448, 378);
    CombatSystem     combat = {};
    ProjectileSystem ps     = {};

    void Step(
        int _count)
    {
        for (int i = 0; i < _count; ++i)
        {
            combat.BeginTick();
            ps.Step(&ground, combat, kStepSec);
        }
    }

    [[nodiscard]] const Shot* First() const
    {
        const Span<const Shot> shots = ps.GetShots();
        return shots.empty() ? nullptr : &shots[0];
    }
};

TEST(VehicleProjectile, RollingShotFallsStraightThenLandsOnTheFloor)
{
    Field f;
    f.ps.SpawnShot(MakeRoller_(-1.f, 600, false), f.combat);
    f.Step(40);
    const Shot* pShot = f.First();
    ASSERT_NE(pShot, nullptr);
    EXPECT_FALSE(pShot->bLanded);
    EXPECT_NEAR(pShot->velocity.x, 0.f, 1e-3f) << "낙하 중에는 수평 속도가 없다";
    f.Step(40);
    pShot = f.First();
    ASSERT_NE(pShot, nullptr);
    EXPECT_TRUE(pShot->bLanded);
    EXPECT_GT(pShot->position.y, kFloorY - 8.f);
    EXPECT_LE(pShot->position.y, kFloorY);
}

TEST(VehicleProjectile, RollingShotKeepsRollingAtTheSpeedInTheChosenDirection)
{
    Field f;
    f.ps.SpawnShot(MakeRoller_(-1.f, 600, false), f.combat);
    f.Step(120);
    const float x0 = f.First()->position.x;
    f.Step(60);
    const float x1 = f.First()->position.x;
    EXPECT_NEAR(x0 - x1, 100.f, 3.f) << "왼쪽으로 100 px/s";
}

TEST(VehicleProjectile, RollingShotCanRollRightWhenItsDirectionSaysSo)
{
    Field f;
    f.ps.SpawnShot(MakeRoller_(1.f, 600, false), f.combat);
    f.Step(120);
    const float x0 = f.First()->position.x;
    f.Step(60);
    EXPECT_NEAR(f.First()->position.x - x0, 100.f, 3.f);
}

TEST(VehicleProjectile, RollingShotIsNotRemovedByTheGroundItRollsOn)
{
    Field f;
    f.ps.SpawnShot(MakeRoller_(-1.f, 600, false), f.combat);
    f.Step(300);
    ASSERT_FALSE(f.ps.GetShots().empty());
    EXPECT_TRUE(f.First()->bAlive);
}

TEST(VehicleProjectile, RollingShotExpiresRollTicksAfterLandingWithItsFlashWhenRequested)
{
    Field f;
    f.ps.SpawnShot(MakeRoller_(-1.f, 120, true), f.combat);
    f.Step(90);
    ASSERT_TRUE(f.First() != nullptr && f.First()->bLanded);
    const int landedAge = f.First()->landedAge;
    f.Step(60);
    EXPECT_FALSE(f.ps.GetShots().empty()) << "착지 후 120 틱 전에는 남아 있다";
    f.Step(80);
    EXPECT_TRUE(f.ps.GetShots().empty());
    EXPECT_GE(landedAge, 55);
    const Vector<ShotFx> fx = f.ps.TakeShotFx();
    EXPECT_EQ(fx.size(), 1u) << "수명 종료 연출 1회";
}

TEST(VehicleProjectile, RollingShotExpiresSilentlyByDefault)
{
    Field f;
    f.ps.SpawnShot(MakeRoller_(-1.f, 60, false), f.combat);
    f.Step(200);
    EXPECT_TRUE(f.ps.GetShots().empty());
    EXPECT_TRUE(f.ps.TakeShotFx().empty());
}

TEST(VehicleProjectile, LifetimeExpiryOfAPlainShotStaysSilentUnlessRequested)
{
    Field     f;
    ShotSpawn spawn     = MakeRoller_(-1.f, 0, false);
    spawn.rollSpeed     = 0.f;
    spawn.gravity       = 0.f;
    spawn.velocity      = { 0.f, 0.f };
    spawn.lifetimeTicks = 30;
    f.ps.SpawnShot(spawn, f.combat);
    f.Step(40);
    EXPECT_TRUE(f.ps.GetShots().empty());
    EXPECT_TRUE(f.ps.TakeShotFx().empty());
}

TEST(VehicleProjectile, ANonRollingShotStillBreaksOnTheGroundWithAFlash)
{
    Field     f;
    ShotSpawn spawn       = MakeRoller_(-1.f, 0, false);
    spawn.rollSpeed       = 0.f;
    spawn.bExplodeOnSolid = true;
    f.ps.SpawnShot(spawn, f.combat);
    f.Step(120);
    EXPECT_TRUE(f.ps.GetShots().empty());
    EXPECT_EQ(f.ps.TakeShotFx().size(), 1u);
}
