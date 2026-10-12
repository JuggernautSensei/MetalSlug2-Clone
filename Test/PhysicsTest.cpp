// Physics 단위 테스트: 중력, 지면 스냅, 경사, 원웨이 플랫폼, 충돌 해소와 결정성 검증
#include <bit>
#include <cmath>

#include <gtest/gtest.h>

#include "Base/Types.h"
#include "World/Physics.h"

namespace
{
[[nodiscard]] GroundMap MakeFlat_(
    const int _w      = 400,
    const int _h      = 160,
    const int _floorY = 100)
{
    GroundMap map = GroundMap::MakeEmpty(_w, _h);
    map.FillRect(0, _floorY, _w - 1, _h - 1, true);
    return map;
}

[[nodiscard]] Body MakeBody_(
    const float _x,
    const float _feetY)
{
    Body body;
    body.position = { _x, _feetY };
    return body;
}

[[nodiscard]] MoveResult Tick_(
    Body&               _body,
    const PhysicsWorld& _world,
    const float         _vx = 0.f)
{
    _body.velocity.x = _vx;
    return Physics::Step(_body, _world);
}

[[nodiscard]] PhysicsWorld WorldOf_(
    const GroundMap& _map)
{
    PhysicsWorld world;
    world.pGround = &_map;
    return world;
}

[[nodiscard]] Body StandingOn_(
    const PhysicsWorld& _world,
    const float         _x,
    const float         _y)
{
    Body body = MakeBody_(_x, _y);
    EXPECT_TRUE(Physics::SnapToGround(body, _world, 64.f));
    return body;
}
}   // namespace

TEST(Physics, FreeFallAppliesGravityPerStep)
{
    const GroundMap    map   = MakeFlat_(400, 4000, 3000);
    const PhysicsWorld world = WorldOf_(map);
    Body               body  = MakeBody_(100.f, 50.f);

    const MoveResult result = Physics::Step(body, world);
    EXPECT_FALSE(result.bGroundedNow);
    EXPECT_NEAR(body.velocity.y, kGravity * kStepSec, 1e-3f);
    EXPECT_NEAR(body.position.y, 50.f + kGravity * kStepSec * kStepSec, 1e-3f);
}

TEST(Physics, FallVelocityIsClampedToTerminal)
{
    const GroundMap    map   = GroundMap::MakeEmpty(400, 100);
    const PhysicsWorld world = WorldOf_(map);
    Body               body  = MakeBody_(100.f, 10.f);
    body.bCollideGround      = false;

    PhysicsTuning tuning;
    for (int i = 0; i < 400; ++i)
    {
        (void)Physics::Step(body, world, tuning);
        ASSERT_LE(body.velocity.y, tuning.terminalVelocity);
    }
    EXPECT_FLOAT_EQ(body.velocity.y, tuning.terminalVelocity);
}

TEST(Physics, LandsExactlyOnFlatGroundAndReportsLandedOnce)
{
    const GroundMap    map   = MakeFlat_();
    const PhysicsWorld world = WorldOf_(map);
    Body               body  = MakeBody_(100.f, 20.f);

    int landedCount = 0;
    for (int i = 0; i < 120; ++i)
    {
        const MoveResult result = Tick_(body, world);
        landedCount += result.bLanded ? 1 : 0;
    }

    EXPECT_EQ(landedCount, 1);
    EXPECT_TRUE(body.bGrounded);
    EXPECT_FLOAT_EQ(body.position.y, 100.f);
    EXPECT_FLOAT_EQ(body.velocity.y, 0.f);
}

TEST(Physics, FastFallDoesNotTunnelThroughThinGround)
{
    GroundMap map = GroundMap::MakeEmpty(400, 3000);
    map.FillRect(0, 2000, 399, 2000, true);
    const PhysicsWorld world = WorldOf_(map);

    Body body = MakeBody_(100.f, 10.f);
    for (int i = 0; i < 400 && !body.bGrounded; ++i)
    {
        (void)Tick_(body, world);
    }
    EXPECT_TRUE(body.bGrounded);
    EXPECT_FLOAT_EQ(body.position.y, 2000.f);
}

TEST(Physics, BottomOfMapIsSolidByDefault)
{
    const GroundMap    map   = GroundMap::MakeEmpty(200, 100);
    const PhysicsWorld world = WorldOf_(map);
    Body               body  = MakeBody_(100.f, 10.f);

    for (int i = 0; i < 120; ++i)
    {
        (void)Tick_(body, world);
    }
    EXPECT_TRUE(body.bGrounded);
    EXPECT_FLOAT_EQ(body.position.y, 100.f);
}

TEST(Physics, BottomOfMapCanBeMadeNonSolid)
{
    GroundMap          map = GroundMap::MakeEmpty(200, 100);
    GroundBoundsPolicy policy;
    policy.bSolidBottom = false;
    map.SetBoundsPolicy(policy);

    const PhysicsWorld world = WorldOf_(map);
    Body               body  = MakeBody_(100.f, 10.f);
    for (int i = 0; i < 120; ++i)
    {
        (void)Tick_(body, world);
    }
    EXPECT_FALSE(body.bGrounded);
    EXPECT_GT(body.position.y, 100.f);
}

TEST(Physics, SnapToGroundPlacesFeetOnSurface)
{
    const GroundMap    map   = MakeFlat_();
    const PhysicsWorld world = WorldOf_(map);

    Body body = MakeBody_(50.f, 90.f);
    EXPECT_TRUE(Physics::SnapToGround(body, world, 20.f));
    EXPECT_FLOAT_EQ(body.position.y, 100.f);
    EXPECT_TRUE(body.bGrounded);

    Body tooFar = MakeBody_(50.f, 20.f);
    EXPECT_FALSE(Physics::SnapToGround(tooFar, world, 20.f));
    EXPECT_FALSE(tooFar.bGrounded);
}

TEST(Physics, WalkingOnFlatGroundIsFrameRateIndependentAtFixedStep)
{
    const GroundMap    map   = MakeFlat_();
    const PhysicsWorld world = WorldOf_(map);
    Body               body  = StandingOn_(world, 20.f, 100.f);

    for (int i = 0; i < 60; ++i)
    {
        const MoveResult result = Tick_(body, world, kRunSpeed);
        ASSERT_TRUE(result.bGroundedNow);
        ASSERT_FALSE(result.bLeftGround);
    }
    EXPECT_NEAR(body.position.x, 20.f + kRunSpeed, 0.05f);
    EXPECT_FLOAT_EQ(body.position.y, 100.f);
}

TEST(Physics, StandingStillStaysGroundedWithoutDrift)
{
    const GroundMap    map   = MakeFlat_();
    const PhysicsWorld world = WorldOf_(map);
    Body               body  = StandingOn_(world, 50.f, 100.f);

    for (int i = 0; i < 300; ++i)
    {
        const MoveResult result = Tick_(body, world);
        ASSERT_TRUE(result.bGroundedNow);
        ASSERT_FALSE(result.bLanded);
    }
    EXPECT_FLOAT_EQ(body.position.x, 50.f);
    EXPECT_FLOAT_EQ(body.position.y, 100.f);
}

namespace
{
[[nodiscard]] GroundMap MakeRamp45_(
    const int _rampStart = 50,
    const int _rampEnd   = 90,
    const int _baseY     = 100)
{
    GroundMap map = GroundMap::MakeEmpty(400, 200);
    for (int c = 0; c < 400; ++c)
    {
        int surface = _baseY;
        if (c >= _rampEnd)
        {
            surface = _baseY - (_rampEnd - _rampStart);
        }
        else if (c >= _rampStart)
        {
            surface = _baseY - (c - _rampStart);
        }
        map.FillRect(c, surface, c, 199, true);
    }
    return map;
}

[[nodiscard]] int RampSurfaceAt_(
    const int _c,
    const int _rampStart = 50,
    const int _rampEnd   = 90,
    const int _baseY     = 100)
{
    if (_c >= _rampEnd)
    {
        return _baseY - (_rampEnd - _rampStart);
    }
    if (_c >= _rampStart)
    {
        return _baseY - (_c - _rampStart);
    }
    return _baseY;
}
}   // namespace

TEST(Physics, WalksUpA45DegreeRampWithoutLeavingTheGround)
{
    const GroundMap    map   = MakeRamp45_();
    const PhysicsWorld world = WorldOf_(map);
    Body               body  = StandingOn_(world, 20.f, 100.f);

    for (int i = 0; i < 90; ++i)
    {
        const MoveResult result = Tick_(body, world, kRunSpeed);
        ASSERT_TRUE(result.bGroundedNow) << "tick " << i << " x=" << body.position.x;
        ASSERT_FALSE(result.bHitWallRight) << "tick " << i;

        const int highestCol = CeilToInt(body.position.x + 1.f) - 1;
        ASSERT_FLOAT_EQ(body.position.y, static_cast<float>(RampSurfaceAt_(highestCol))) << "tick " << i << " x=" << body.position.x;
    }

    EXPECT_GT(body.position.x, 90.f);
    EXPECT_FLOAT_EQ(body.position.y, 60.f);
}

TEST(Physics, RampReportsSlopeAndNormal)
{
    const GroundMap    map    = MakeRamp45_();
    const PhysicsWorld world  = WorldOf_(map);
    Body               body   = StandingOn_(world, 70.f, 80.f);
    const MoveResult   result = Tick_(body, world);

    EXPECT_TRUE(result.bGroundedNow);
    EXPECT_NEAR(result.groundSlope, -1.f, 0.01f);
    EXPECT_LT(result.groundNormal.x, -0.6f);
    EXPECT_LT(result.groundNormal.y, -0.6f);
    EXPECT_NEAR(result.groundNormal.x * result.groundNormal.x + result.groundNormal.y * result.groundNormal.y, 1.f, 1e-4f);
}

TEST(Physics, FlatGroundReportsFlatNormal)
{
    const GroundMap    map    = MakeFlat_();
    const PhysicsWorld world  = WorldOf_(map);
    Body               body   = StandingOn_(world, 200.f, 100.f);
    const MoveResult   result = Tick_(body, world);
    EXPECT_FLOAT_EQ(result.groundSlope, 0.f);
    EXPECT_FLOAT_EQ(result.groundNormal.x, 0.f);
    EXPECT_FLOAT_EQ(result.groundNormal.y, -1.f);
}

TEST(Physics, WalksDownARampAndStaysGrounded)
{
    GroundMap map = GroundMap::MakeEmpty(400, 200);
    for (int c = 0; c < 400; ++c)
    {
        const int surface = c < 50 ? 60 : (c < 90 ? 60 + (c - 50) : 100);
        map.FillRect(c, surface, c, 199, true);
    }
    const PhysicsWorld world = WorldOf_(map);
    Body               body  = StandingOn_(world, 20.f, 60.f);

    int leftGroundCount = 0;
    for (int i = 0; i < 90; ++i)
    {
        const MoveResult result = Tick_(body, world, kRunSpeed);
        leftGroundCount += result.bLeftGround ? 1 : 0;
        ASSERT_TRUE(result.bGroundedNow) << "tick " << i;
    }
    EXPECT_EQ(leftGroundCount, 0);
    EXPECT_FLOAT_EQ(body.position.y, 100.f);
}

TEST(Physics, StepUpToLowCurbButBlockedByTallWall)
{
    GroundMap map = MakeFlat_();
    map.FillRect(150, 97, 249, 99, true);
    map.FillRect(250, 92, 399, 99, true);
    const PhysicsWorld world = WorldOf_(map);

    Body body     = StandingOn_(world, 100.f, 100.f);
    bool bHitWall = false;
    for (int i = 0; i < 150; ++i)
    {
        const MoveResult result = Tick_(body, world, kRunSpeed);
        bHitWall                = bHitWall || result.bHitWallRight;
    }

    EXPECT_TRUE(bHitWall);
    EXPECT_TRUE(body.bGrounded);
    EXPECT_FLOAT_EQ(body.position.y, 97.f);
    EXPECT_LE(body.position.x + 12.f, 250.f + 0.01f);
    EXPECT_GT(body.position.x, 200.f);
}

TEST(Physics, WalksOffALedgeAndFallsThenLands)
{
    GroundMap map = GroundMap::MakeEmpty(400, 200);
    map.FillRect(0, 80, 99, 199, true);
    map.FillRect(100, 150, 399, 199, true);
    const PhysicsWorld world = WorldOf_(map);

    Body body      = StandingOn_(world, 60.f, 80.f);
    int  leftCount = 0;
    int  landCount = 0;
    for (int i = 0; i < 120; ++i)
    {
        const MoveResult result = Tick_(body, world, 120.f);
        leftCount += result.bLeftGround ? 1 : 0;
        landCount += result.bLanded ? 1 : 0;
    }

    EXPECT_EQ(leftCount, 1);
    EXPECT_EQ(landCount, 1);
    EXPECT_TRUE(body.bGrounded);
    EXPECT_FLOAT_EQ(body.position.y, 150.f);
}

TEST(Physics, LedgeEdgeUsesCenterFootSensor)
{
    GroundMap map = GroundMap::MakeEmpty(400, 200);
    map.FillRect(0, 80, 99, 199, true);
    const PhysicsWorld world = WorldOf_(map);

    Body body = StandingOn_(world, 95.f, 80.f);
    for (int i = 0; i < 10; ++i)
    {
        (void)Tick_(body, world);
    }
    EXPECT_TRUE(body.bGrounded);

    body.position.x = 103.f;
    (void)Tick_(body, world);
    EXPECT_FALSE(body.bGrounded);
}

TEST(Physics, SmallDropIsSnappedWhileGrounded)
{
    GroundMap map = GroundMap::MakeEmpty(400, 200);
    map.FillRect(0, 100, 149, 199, true);
    map.FillRect(150, 103, 399, 199, true);
    const PhysicsWorld world = WorldOf_(map);

    Body body = StandingOn_(world, 100.f, 100.f);
    for (int i = 0; i < 40; ++i)
    {
        const MoveResult result = Tick_(body, world, kRunSpeed);
        ASSERT_TRUE(result.bGroundedNow);
        ASSERT_FALSE(result.bLeftGround);
    }
    EXPECT_FLOAT_EQ(body.position.y, 103.f);
}

TEST(Physics, DeepDropIsNotSnapped)
{
    GroundMap map = GroundMap::MakeEmpty(400, 200);
    map.FillRect(0, 100, 149, 199, true);
    map.FillRect(150, 110, 399, 199, true);
    const PhysicsWorld world = WorldOf_(map);

    Body body  = StandingOn_(world, 100.f, 100.f);
    bool bLeft = false;
    for (int i = 0; i < 40; ++i)
    {
        bLeft |= Tick_(body, world, kRunSpeed).bLeftGround;
    }
    EXPECT_TRUE(bLeft);
}

TEST(Physics, JumpReachesExpectedApexAndReturns)
{
    const GroundMap    map   = MakeFlat_();
    const PhysicsWorld world = WorldOf_(map);
    Body               body  = StandingOn_(world, 100.f, 100.f);

    body.velocity.y = -kJumpSpeed;
    float minY      = body.position.y;
    int   ticks     = 0;
    bool  bLeft     = false;
    bool  bLanded   = false;
    for (; ticks < 200; ++ticks)
    {
        const MoveResult result = Tick_(body, world);
        bLeft                   = bLeft || result.bLeftGround;
        minY                    = std::min(minY, body.position.y);
        if (result.bLanded)
        {
            bLanded = true;
            break;
        }
    }

    EXPECT_TRUE(bLeft);
    EXPECT_TRUE(bLanded);
    const float apexHeight = 100.f - minY;
    EXPECT_GT(apexHeight, 120.f);
    EXPECT_LT(apexHeight, 132.f);
    EXPECT_NEAR(static_cast<float>(ticks), 51.f, 3.f);
    EXPECT_FLOAT_EQ(body.position.y, 100.f);
}

TEST(Physics, LiftoffClearsGroundedFlagImmediately)
{
    const GroundMap    map   = MakeFlat_();
    const PhysicsWorld world = WorldOf_(map);
    Body               body  = StandingOn_(world, 100.f, 100.f);

    body.velocity.y         = -kJumpSpeed;
    const MoveResult result = Tick_(body, world);
    EXPECT_TRUE(result.bLeftGround);
    EXPECT_FALSE(result.bGroundedNow);
    EXPECT_LT(body.position.y, 100.f);
}

TEST(Physics, CeilingStopsAJumpAndZeroesVelocity)
{
    GroundMap map = MakeFlat_();
    map.FillRect(60, 20, 140, 59, true);
    const PhysicsWorld world = WorldOf_(map);
    Body               body  = StandingOn_(world, 100.f, 100.f);

    body.velocity.y = -kJumpSpeed;
    bool bHit       = false;
    for (int i = 0; i < 30; ++i)
    {
        const MoveResult result = Tick_(body, world);
        if (result.bHitCeiling)
        {
            bHit = true;
            EXPECT_FLOAT_EQ(body.velocity.y, 0.f);
            break;
        }
    }
    ASSERT_TRUE(bHit);
    EXPECT_FLOAT_EQ(body.position.y - body.size.y, 60.f);

    for (int i = 0; i < 80; ++i)
    {
        (void)Tick_(body, world);
    }
    EXPECT_TRUE(body.bGrounded);
    EXPECT_FLOAT_EQ(body.position.y, 100.f);
}

TEST(Physics, WallStopsWalkingFlushAndZeroesVelocity)
{
    GroundMap map = MakeFlat_();
    map.FillRect(200, 0, 399, 99, true);
    const PhysicsWorld world = WorldOf_(map);
    Body               body  = StandingOn_(world, 100.f, 100.f);

    bool bHit = false;
    for (int i = 0; i < 120; ++i)
    {
        bHit |= Tick_(body, world, kRunSpeed).bHitWallRight;
    }
    EXPECT_TRUE(bHit);
    EXPECT_FLOAT_EQ(body.position.x + 12.f, 200.f);
    EXPECT_FLOAT_EQ(body.velocity.x, 0.f);
    EXPECT_TRUE(body.bGrounded);
}

TEST(Physics, LeftWallStopsToo)
{
    GroundMap map = MakeFlat_();
    map.FillRect(0, 0, 49, 99, true);   // 열 0..49 벽 (오른쪽 면 = 50)
    const PhysicsWorld world = WorldOf_(map);
    Body               body  = StandingOn_(world, 150.f, 100.f);

    bool bHit = false;
    for (int i = 0; i < 120; ++i)
    {
        bHit |= Tick_(body, world, -kRunSpeed).bHitWallLeft;
    }
    EXPECT_TRUE(bHit);
    EXPECT_FLOAT_EQ(body.position.x - 12.f, 50.f);
}

TEST(Physics, AirborneBodyCannotPassThroughWallOrSlideIntoLedgeCorner)
{
    GroundMap map = MakeFlat_();
    map.FillRect(200, 40, 399, 99, true);
    const PhysicsWorld world = WorldOf_(map);
    Body               body  = StandingOn_(world, 150.f, 100.f);

    body.velocity.y = -300.f;
    bool bHit       = false;
    for (int i = 0; i < 60; ++i)
    {
        const MoveResult result = Tick_(body, world, kRunSpeed);
        bHit                    = bHit || result.bHitWallRight;
        ASSERT_LE(body.position.x + 12.f, 200.f + 0.001f) << "tick " << i;
    }
    EXPECT_TRUE(bHit);
}

TEST(Physics, AirborneBodyClearingTheLedgeLandsOnTop)
{
    GroundMap map = MakeFlat_();
    map.FillRect(200, 80, 399, 99, true);
    const PhysicsWorld world = WorldOf_(map);
    Body               body  = StandingOn_(world, 150.f, 100.f);

    body.velocity.y = -kJumpSpeed;
    for (int i = 0; i < 60; ++i)
    {
        (void)Tick_(body, world, 150.f);
    }
    EXPECT_TRUE(body.bGrounded);
    EXPECT_FLOAT_EQ(body.position.y, 80.f);
    EXPECT_GT(body.position.x, 200.f);
}

TEST(Physics, AirborneBodyDoesNotStepUpLedges)
{
    GroundMap map = MakeFlat_();
    map.FillRect(200, 94, 399, 99, true);
    const PhysicsWorld world = WorldOf_(map);

    Body             body   = MakeBody_(187.f, 98.f);
    const MoveResult result = Tick_(body, world, kRunSpeed);
    EXPECT_TRUE(result.bHitWallRight);
    EXPECT_GE(body.position.y, 98.f);
    EXPECT_FLOAT_EQ(body.position.x + 12.f, 200.f);
}

TEST(Physics, OneWayPlatformPassesFromBelowAndLandsFromAbove)
{
    const GroundMap map = MakeFlat_();
    Platform        oneWay;
    oneWay.box     = Aabb::FromMinMax({ 100.f, 60.f }, { 200.f, 66.f });
    oneWay.bOneWay = true;

    PhysicsWorld world = WorldOf_(map);
    world.platforms    = Span<const Platform> { &oneWay, 1 };

    Body body       = StandingOn_(world, 150.f, 100.f);
    body.velocity.y = -kJumpSpeed;

    bool bCeiling = false;
    bool bLanded  = false;
    for (int i = 0; i < 120; ++i)
    {
        const MoveResult result = Tick_(body, world);
        bCeiling                = bCeiling || result.bHitCeiling;
        if (result.bLanded)
        {
            bLanded = true;
            EXPECT_EQ(result.platformIndex, 0);
            break;
        }
    }

    EXPECT_FALSE(bCeiling);
    EXPECT_TRUE(bLanded);
    EXPECT_FLOAT_EQ(body.position.y, 60.f);
}

TEST(Physics, OneWayPlatformDoesNotCatchBodiesThatAreBelowTheTop)
{
    const GroundMap map = MakeFlat_();
    Platform        oneWay;
    oneWay.box     = Aabb::FromMinMax({ 100.f, 60.f }, { 200.f, 66.f });
    oneWay.bOneWay = true;

    PhysicsWorld world = WorldOf_(map);
    world.platforms    = Span<const Platform> { &oneWay, 1 };

    Body body = MakeBody_(150.f, 80.f);
    for (int i = 0; i < 60; ++i)
    {
        (void)Tick_(body, world);
    }
    EXPECT_TRUE(body.bGrounded);
    EXPECT_FLOAT_EQ(body.position.y, 100.f);
}

TEST(Physics, OneWayPlatformDoesNotBlockWalkingSideways)
{
    const GroundMap map = MakeFlat_();
    Platform        oneWay;
    oneWay.box     = Aabb::FromMinMax({ 150.f, 70.f }, { 170.f, 76.f });
    oneWay.bOneWay = true;

    PhysicsWorld world = WorldOf_(map);
    world.platforms    = Span<const Platform> { &oneWay, 1 };

    Body body = StandingOn_(world, 100.f, 100.f);
    for (int i = 0; i < 90; ++i)
    {
        (void)Tick_(body, world, kRunSpeed);
    }
    EXPECT_GT(body.position.x, 250.f);
}

TEST(Physics, DropThroughLetsBodyFallOffOneWayPlatform)
{
    const GroundMap map = MakeFlat_();
    Platform        oneWay;
    oneWay.box     = Aabb::FromMinMax({ 100.f, 60.f }, { 200.f, 66.f });
    oneWay.bOneWay = true;

    PhysicsWorld world = WorldOf_(map);
    world.platforms    = Span<const Platform> { &oneWay, 1 };

    Body body = StandingOn_(world, 150.f, 60.f);
    for (int i = 0; i < 5; ++i)
    {
        (void)Tick_(body, world);
    }
    ASSERT_TRUE(body.bGrounded);
    ASSERT_FLOAT_EQ(body.position.y, 60.f);

    body.RequestDropThrough();
    for (int i = 0; i < 120; ++i)
    {
        (void)Tick_(body, world);
    }
    EXPECT_TRUE(body.bGrounded);
    EXPECT_FLOAT_EQ(body.position.y, 100.f);
}

TEST(Physics, SolidPlatformSupportsBlocksSidesAndBumpsHead)
{
    const GroundMap map = MakeFlat_();
    Platform        solid;
    solid.box     = Aabb::FromMinMax({ 150.f, 50.f }, { 220.f, 100.f });
    solid.bOneWay = false;

    PhysicsWorld world = WorldOf_(map);
    world.platforms    = Span<const Platform> { &solid, 1 };

    Body body = StandingOn_(world, 100.f, 100.f);
    bool bHit = false;
    for (int i = 0; i < 90; ++i)
    {
        bHit |= Tick_(body, world, kRunSpeed).bHitWallRight;
    }
    EXPECT_TRUE(bHit);
    EXPECT_FLOAT_EQ(body.position.x + 12.f, 150.f);

    Body faller = MakeBody_(180.f, 0.f);
    for (int i = 0; i < 120; ++i)
    {
        (void)Tick_(faller, world);
    }
    EXPECT_TRUE(faller.bGrounded);
    EXPECT_FLOAT_EQ(faller.position.y, 50.f);

    Platform hanging;
    hanging.box         = Aabb::FromMinMax({ 300.f, 20.f }, { 360.f, 40.f });
    PhysicsWorld world2 = WorldOf_(map);
    world2.platforms    = Span<const Platform> { &hanging, 1 };
    Body jumper         = StandingOn_(world2, 330.f, 100.f);
    jumper.velocity.y   = -kJumpSpeed;
    bool bCeiling       = false;
    for (int i = 0; i < 40 && !bCeiling; ++i)
    {
        bCeiling = Tick_(jumper, world2).bHitCeiling;
    }
    EXPECT_TRUE(bCeiling);
    EXPECT_FLOAT_EQ(jumper.position.y - jumper.size.y, 40.f);
}

TEST(Physics, WalkingOffPlatformEdgeFallsToGround)
{
    const GroundMap map = MakeFlat_();
    Platform        solid;
    solid.box = Aabb::FromMinMax({ 100.f, 60.f }, { 160.f, 100.f });

    PhysicsWorld world = WorldOf_(map);
    world.platforms    = Span<const Platform> { &solid, 1 };

    Body body = StandingOn_(world, 130.f, 60.f);
    ASSERT_FLOAT_EQ(body.position.y, 60.f);

    bool bLeft = false;
    for (int i = 0; i < 120; ++i)
    {
        bLeft |= Tick_(body, world, 100.f).bLeftGround;
    }
    EXPECT_TRUE(bLeft);
    EXPECT_TRUE(body.bGrounded);
    EXPECT_FLOAT_EQ(body.position.y, 100.f);
}

TEST(Physics, WallBoxBlocksAndCanBeStoodOn)
{
    const GroundMap map = MakeFlat_();
    Wall            wall;
    wall.box = Aabb::FromMinMax({ 200.f, 0.f }, { 210.f, 100.f });

    PhysicsWorld world = WorldOf_(map);
    world.walls        = Span<const Wall> { &wall, 1 };

    Body body = StandingOn_(world, 100.f, 100.f);
    bool bHit = false;
    for (int i = 0; i < 90; ++i)
    {
        bHit |= Tick_(body, world, kRunSpeed).bHitWallRight;
    }
    EXPECT_TRUE(bHit);
    EXPECT_FLOAT_EQ(body.position.x + 12.f, 200.f);

    Body other = StandingOn_(world, 300.f, 100.f);
    bHit       = false;
    for (int i = 0; i < 90; ++i)
    {
        bHit |= Tick_(other, world, -kRunSpeed).bHitWallLeft;
    }
    EXPECT_TRUE(bHit);
    EXPECT_FLOAT_EQ(other.position.x - 12.f, 210.f);
}

TEST(Physics, BodiesCanOptOutOfCollisionLayers)
{
    GroundMap map = MakeFlat_();
    map.FillRect(200, 0, 399, 99, true);
    const PhysicsWorld world = WorldOf_(map);

    Body ghost           = MakeBody_(100.f, 100.f);
    ghost.bCollideGround = false;
    ghost.bUseGravity    = false;
    for (int i = 0; i < 120; ++i)
    {
        (void)Tick_(ghost, world, kRunSpeed);
    }
    EXPECT_GT(ghost.position.x, 250.f);
    EXPECT_FLOAT_EQ(ghost.position.y, 100.f);
}

struct Snapshot
{
    uint32_t x;
    uint32_t y;
    uint32_t vx;
    uint32_t vy;
    bool     bGrounded;

    [[nodiscard]] bool operator==(const Snapshot&) const = default;
};

namespace
{
[[nodiscard]] Snapshot Capture_(
    const Body& _body)
{
    return { std::bit_cast<uint32_t>(_body.position.x), std::bit_cast<uint32_t>(_body.position.y), std::bit_cast<uint32_t>(_body.velocity.x), std::bit_cast<uint32_t>(_body.velocity.y), _body.bGrounded };
}

[[nodiscard]] Vector<Snapshot> RunScenario_()
{
    GroundMap map = MakeRamp45_(50, 90, 100);
    map.FillRect(260, 100, 330, 199, false);
    map.FillRect(330, 40, 399, 199, true);
    Platform oneWay;
    oneWay.box     = Aabb::FromMinMax({ 120.f, 50.f }, { 180.f, 56.f });
    oneWay.bOneWay = true;

    PhysicsWorld world = WorldOf_(map);
    world.platforms    = Span<const Platform> { &oneWay, 1 };

    Body             body = MakeBody_(20.f, 100.f);
    Vector<Snapshot> history;
    for (int i = 0; i < 600; ++i)
    {
        float vx = 0.f;
        if ((i / 90) % 3 != 2)
        {
            vx = kRunSpeed;
        }
        if (i % 70 == 0 && body.bGrounded)
        {
            body.velocity.y = -kJumpSpeed;
        }
        if (i % 200 == 150)
        {
            body.RequestDropThrough();
        }
        (void)Tick_(body, world, vx);
        history.push_back(Capture_(body));
    }
    return history;
}
}   // namespace

TEST(Physics, SameInputsProduceBitIdenticalResultsAfter600Steps)
{
    const Vector<Snapshot> a = RunScenario_();
    const Vector<Snapshot> b = RunScenario_();
    ASSERT_EQ(a.size(), 600u);
    ASSERT_EQ(a.size(), b.size());
    for (size_t i = 0; i < a.size(); ++i)
    {
        ASSERT_TRUE(a[i] == b[i]) << "diverged at step " << i;
    }
}

TEST(Physics, ScenarioActuallyMovesTheBody)
{
    const Vector<Snapshot> history = RunScenario_();
    const float            endX    = std::bit_cast<float>(history.back().x);
    EXPECT_GT(endX, 100.f);
}
