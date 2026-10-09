#include <bit>
#include <cmath>

#include <gtest/gtest.h>

#include "World/Physics.h"

using namespace slug;

// ===========================================
//  테스트 도우미
// ===========================================

namespace
{
// 윗면이 floorY 인 평지 (폭 w, 높이 h)
GroundMap MakeFlat(
    const int _w      = 400,
    const int _h      = 160,
    const int _floorY = 100)
{
    GroundMap map = GroundMap::MakeEmpty(_w, _h);
    map.FillRect(0, _floorY, _w - 1, _h - 1, true);
    return map;
}

Body MakeBody(
    const float _x,
    const float _feetY)
{
    Body body;
    body.position = { _x, _feetY };
    return body;
}

// 컨트롤러처럼 매 틱 가로 속도를 지정하고 한 스텝 진행
MoveResult Tick(
    Body&               _body,
    const PhysicsWorld& _world,
    const float         _vx = 0.f)
{
    _body.velocity.x = _vx;
    return Physics::Step(_body, _world);
}

PhysicsWorld WorldOf(
    const GroundMap& _map)
{
    PhysicsWorld world;
    world.pGround = &_map;
    return world;
}

// 땅에 붙여서 시작
Body StandingOn(
    const PhysicsWorld& _world,
    const float         _x,
    const float         _y)
{
    Body body = MakeBody(_x, _y);
    EXPECT_TRUE(Physics::SnapToGround(body, _world, 64.f));
    return body;
}
}   // namespace

// ===========================================
//  중력 / 착지
// ===========================================

TEST(Physics, FreeFallAppliesGravityPerStep)
{
    const GroundMap    map   = MakeFlat(400, 4000, 3000);
    const PhysicsWorld world = WorldOf(map);
    Body               body  = MakeBody(100.f, 50.f);

    const MoveResult result = Physics::Step(body, world);
    EXPECT_FALSE(result.bGroundedNow);
    EXPECT_NEAR(body.velocity.y, kGravity * kStepSec, 1e-3f);
    EXPECT_NEAR(body.position.y, 50.f + kGravity * kStepSec * kStepSec, 1e-3f);
}

TEST(Physics, FallVelocityIsClampedToTerminal)
{
    const GroundMap    map   = GroundMap::MakeEmpty(400, 100);
    const PhysicsWorld world = WorldOf(map);
    Body               body  = MakeBody(100.f, 10.f);
    body.bCollideGround      = false;   // 지면 없이 계속 낙하

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
    const GroundMap    map   = MakeFlat();
    const PhysicsWorld world = WorldOf(map);
    Body               body  = MakeBody(100.f, 20.f);

    int landedCount = 0;
    for (int i = 0; i < 120; ++i)
    {
        const MoveResult result = Tick(body, world);
        landedCount += result.bLanded ? 1 : 0;
    }

    EXPECT_EQ(landedCount, 1);
    EXPECT_TRUE(body.bGrounded);
    EXPECT_FLOAT_EQ(body.position.y, 100.f);
    EXPECT_FLOAT_EQ(body.velocity.y, 0.f);
}

TEST(Physics, FastFallDoesNotTunnelThroughThinGround)
{
    // 1px 두께 바닥 + 그 아래는 공기. 터미널 속도(50px/틱)로 떨어져도 뚫지 않아야 한다.
    GroundMap map = GroundMap::MakeEmpty(400, 3000);
    map.FillRect(0, 2000, 399, 2000, true);
    const PhysicsWorld world = WorldOf(map);

    Body body = MakeBody(100.f, 10.f);
    for (int i = 0; i < 400 && !body.bGrounded; ++i)
    {
        (void)Tick(body, world);
    }
    EXPECT_TRUE(body.bGrounded);
    EXPECT_FLOAT_EQ(body.position.y, 2000.f);
}

TEST(Physics, BottomOfMapIsSolidByDefault)
{
    const GroundMap    map   = GroundMap::MakeEmpty(200, 100);
    const PhysicsWorld world = WorldOf(map);
    Body               body  = MakeBody(100.f, 10.f);

    for (int i = 0; i < 120; ++i)
    {
        (void)Tick(body, world);
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

    const PhysicsWorld world = WorldOf(map);
    Body               body  = MakeBody(100.f, 10.f);
    for (int i = 0; i < 120; ++i)
    {
        (void)Tick(body, world);
    }
    EXPECT_FALSE(body.bGrounded);
    EXPECT_GT(body.position.y, 100.f);
}

TEST(Physics, SnapToGroundPlacesFeetOnSurface)
{
    const GroundMap    map   = MakeFlat();
    const PhysicsWorld world = WorldOf(map);

    Body body = MakeBody(50.f, 90.f);
    EXPECT_TRUE(Physics::SnapToGround(body, world, 20.f));
    EXPECT_FLOAT_EQ(body.position.y, 100.f);
    EXPECT_TRUE(body.bGrounded);

    Body tooFar = MakeBody(50.f, 20.f);
    EXPECT_FALSE(Physics::SnapToGround(tooFar, world, 20.f));
    EXPECT_FALSE(tooFar.bGrounded);
}

// ===========================================
//  평지 이동
// ===========================================

TEST(Physics, WalkingOnFlatGroundIsFrameRateIndependentAtFixedStep)
{
    const GroundMap    map   = MakeFlat();
    const PhysicsWorld world = WorldOf(map);
    Body               body  = StandingOn(world, 20.f, 100.f);

    for (int i = 0; i < 60; ++i)
    {
        const MoveResult result = Tick(body, world, kRunSpeed);
        ASSERT_TRUE(result.bGroundedNow);
        ASSERT_FALSE(result.bLeftGround);
    }
    EXPECT_NEAR(body.position.x, 20.f + kRunSpeed, 0.05f);   // 60틱 = 1초
    EXPECT_FLOAT_EQ(body.position.y, 100.f);
}

TEST(Physics, StandingStillStaysGroundedWithoutDrift)
{
    const GroundMap    map   = MakeFlat();
    const PhysicsWorld world = WorldOf(map);
    Body               body  = StandingOn(world, 50.f, 100.f);

    for (int i = 0; i < 300; ++i)
    {
        const MoveResult result = Tick(body, world);
        ASSERT_TRUE(result.bGroundedNow);
        ASSERT_FALSE(result.bLanded);
    }
    EXPECT_FLOAT_EQ(body.position.x, 50.f);
    EXPECT_FLOAT_EQ(body.position.y, 100.f);
}

// ===========================================
//  경사
// ===========================================

namespace
{
// 열 c 의 지면 윗면이 high 로 올라가는 45° 오르막 (x 가 증가할수록 y 감소). 열 rampStart..rampEnd 에서 1:1.
GroundMap MakeRamp45(
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

int RampSurfaceAt(
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
    const GroundMap    map   = MakeRamp45();
    const PhysicsWorld world = WorldOf(map);
    Body               body  = StandingOn(world, 20.f, 100.f);

    for (int i = 0; i < 90; ++i)
    {
        const MoveResult result = Tick(body, world, kRunSpeed);
        ASSERT_TRUE(result.bGroundedNow) << "tick " << i << " x=" << body.position.x;
        ASSERT_FALSE(result.bHitWallRight) << "tick " << i;

        // 발 센서(footHalfWidth=1)가 덮는 열 중 가장 높은 지면 위에 선다
        const int highestCol = CeilToInt(body.position.x + 1.f) - 1;
        ASSERT_FLOAT_EQ(body.position.y, static_cast<float>(RampSurfaceAt(highestCol))) << "tick " << i << " x=" << body.position.x;
    }

    // 램프 끝(고원)에 도달
    EXPECT_GT(body.position.x, 90.f);
    EXPECT_FLOAT_EQ(body.position.y, 60.f);
}

TEST(Physics, RampReportsSlopeAndNormal)
{
    const GroundMap    map    = MakeRamp45();
    const PhysicsWorld world  = WorldOf(map);
    Body               body   = StandingOn(world, 70.f, 80.f);
    const MoveResult   result = Tick(body, world);

    EXPECT_TRUE(result.bGroundedNow);
    EXPECT_NEAR(result.groundSlope, -1.f, 0.01f);   // 오른쪽으로 갈수록 위로 올라감
    EXPECT_LT(result.groundNormal.x, -0.6f);        // 법선은 왼쪽 위를 향함
    EXPECT_LT(result.groundNormal.y, -0.6f);
    EXPECT_NEAR(result.groundNormal.x * result.groundNormal.x + result.groundNormal.y * result.groundNormal.y, 1.f, 1e-4f);
}

TEST(Physics, FlatGroundReportsFlatNormal)
{
    const GroundMap    map    = MakeFlat();
    const PhysicsWorld world  = WorldOf(map);
    Body               body   = StandingOn(world, 200.f, 100.f);
    const MoveResult   result = Tick(body, world);
    EXPECT_FLOAT_EQ(result.groundSlope, 0.f);
    EXPECT_FLOAT_EQ(result.groundNormal.x, 0.f);
    EXPECT_FLOAT_EQ(result.groundNormal.y, -1.f);
}

TEST(Physics, WalksDownARampAndStaysGrounded)
{
    // 오르막을 거꾸로: 왼쪽이 높고 오른쪽이 낮은 내리막
    GroundMap map = GroundMap::MakeEmpty(400, 200);
    for (int c = 0; c < 400; ++c)
    {
        const int surface = c < 50 ? 60 : (c < 90 ? 60 + (c - 50) : 100);
        map.FillRect(c, surface, c, 199, true);
    }
    const PhysicsWorld world = WorldOf(map);
    Body               body  = StandingOn(world, 20.f, 60.f);

    int leftGroundCount = 0;
    for (int i = 0; i < 90; ++i)
    {
        const MoveResult result = Tick(body, world, kRunSpeed);
        leftGroundCount += result.bLeftGround ? 1 : 0;
        ASSERT_TRUE(result.bGroundedNow) << "tick " << i;
    }
    EXPECT_EQ(leftGroundCount, 0);
    EXPECT_FLOAT_EQ(body.position.y, 100.f);
}

TEST(Physics, StepUpToLowCurbButBlockedByTallWall)
{
    // x >= 150 에 높이 3px 턱, x >= 250 에 높이 8px 벽
    GroundMap map = MakeFlat();
    map.FillRect(150, 97, 249, 99, true);
    map.FillRect(250, 92, 399, 99, true);
    const PhysicsWorld world = WorldOf(map);

    Body body     = StandingOn(world, 100.f, 100.f);
    bool bHitWall = false;
    for (int i = 0; i < 150; ++i)
    {
        const MoveResult result = Tick(body, world, kRunSpeed);
        bHitWall                = bHitWall || result.bHitWallRight;
    }

    EXPECT_TRUE(bHitWall);
    EXPECT_TRUE(body.bGrounded);
    // 3px 턱은 올라섰다 (현재 서 있는 발 높이 = 97)
    EXPECT_FLOAT_EQ(body.position.y, 97.f);
    // 8px 벽 앞에서 멈췄다: 오른쪽 모서리가 벽 열(250) 이내
    EXPECT_LE(body.position.x + 12.f, 250.f + 0.01f);
    EXPECT_GT(body.position.x, 200.f);
}

// ===========================================
//  낭떠러지 / 스냅
// ===========================================

TEST(Physics, WalksOffALedgeAndFallsThenLands)
{
    // x < 100 : 윗면 y=80, x >= 100 : 윗면 y=150
    GroundMap map = GroundMap::MakeEmpty(400, 200);
    map.FillRect(0, 80, 99, 199, true);
    map.FillRect(100, 150, 399, 199, true);
    const PhysicsWorld world = WorldOf(map);

    Body body      = StandingOn(world, 60.f, 80.f);
    int  leftCount = 0;
    int  landCount = 0;
    for (int i = 0; i < 120; ++i)   // x: 60 → 300 (맵 폭 400 안쪽)
    {
        const MoveResult result = Tick(body, world, 120.f);
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
    map.FillRect(0, 80, 99, 199, true);   // 열 99 까지가 땅
    const PhysicsWorld world = WorldOf(map);

    // 몸 반이 허공에 걸쳐도 중심이 땅 위면 선다
    Body body = StandingOn(world, 95.f, 80.f);
    for (int i = 0; i < 10; ++i)
    {
        (void)Tick(body, world);
    }
    EXPECT_TRUE(body.bGrounded);

    // 중심(발 센서)이 완전히 벗어나면 떨어진다
    body.position.x = 103.f;
    (void)Tick(body, world);
    EXPECT_FALSE(body.bGrounded);
}

TEST(Physics, SmallDropIsSnappedWhileGrounded)
{
    // 3px 단차 내리막 (maxSnapDown=4 이내): 공중에 뜨지 않고 붙어서 내려간다
    GroundMap map = GroundMap::MakeEmpty(400, 200);
    map.FillRect(0, 100, 149, 199, true);
    map.FillRect(150, 103, 399, 199, true);
    const PhysicsWorld world = WorldOf(map);

    Body body = StandingOn(world, 100.f, 100.f);
    for (int i = 0; i < 40; ++i)
    {
        const MoveResult result = Tick(body, world, kRunSpeed);
        ASSERT_TRUE(result.bGroundedNow);
        ASSERT_FALSE(result.bLeftGround);
    }
    EXPECT_FLOAT_EQ(body.position.y, 103.f);
}

TEST(Physics, DeepDropIsNotSnapped)
{
    GroundMap map = GroundMap::MakeEmpty(400, 200);
    map.FillRect(0, 100, 149, 199, true);
    map.FillRect(150, 110, 399, 199, true);   // 10px 단차 > maxSnapDown
    const PhysicsWorld world = WorldOf(map);

    Body body  = StandingOn(world, 100.f, 100.f);
    bool bLeft = false;
    for (int i = 0; i < 40; ++i)
    {
        bLeft |= Tick(body, world, kRunSpeed).bLeftGround;
    }
    EXPECT_TRUE(bLeft);
}

// ===========================================
//  점프 / 천장 / 벽
// ===========================================

TEST(Physics, JumpReachesExpectedApexAndReturns)
{
    const GroundMap    map   = MakeFlat();
    const PhysicsWorld world = WorldOf(map);
    Body               body  = StandingOn(world, 100.f, 100.f);

    body.velocity.y = -kJumpSpeed;
    float minY      = body.position.y;
    int   ticks     = 0;
    bool  bLeft     = false;
    bool  bLanded   = false;
    for (; ticks < 200; ++ticks)
    {
        const MoveResult result = Tick(body, world);
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
    // v²/2g = 128.6px (이산화 오차 허용)
    const float apexHeight = 100.f - minY;
    EXPECT_GT(apexHeight, 120.f);
    EXPECT_LT(apexHeight, 132.f);
    // 체공 시간 2v/g ≈ 0.857s ≈ 51틱
    EXPECT_NEAR(static_cast<float>(ticks), 51.f, 3.f);
    EXPECT_FLOAT_EQ(body.position.y, 100.f);
}

TEST(Physics, LiftoffClearsGroundedFlagImmediately)
{
    const GroundMap    map   = MakeFlat();
    const PhysicsWorld world = WorldOf(map);
    Body               body  = StandingOn(world, 100.f, 100.f);

    body.velocity.y         = -kJumpSpeed;
    const MoveResult result = Tick(body, world);
    EXPECT_TRUE(result.bLeftGround);
    EXPECT_FALSE(result.bGroundedNow);
    EXPECT_LT(body.position.y, 100.f);
}

TEST(Physics, CeilingStopsAJumpAndZeroesVelocity)
{
    GroundMap map = MakeFlat();
    map.FillRect(60, 20, 140, 59, true);   // 윗면 20, 아랫면 y=60 (행 59 까지)
    const PhysicsWorld world = WorldOf(map);
    Body               body  = StandingOn(world, 100.f, 100.f);

    body.velocity.y = -kJumpSpeed;
    bool bHit       = false;
    for (int i = 0; i < 30; ++i)
    {
        const MoveResult result = Tick(body, world);
        if (result.bHitCeiling)
        {
            bHit = true;
            EXPECT_FLOAT_EQ(body.velocity.y, 0.f);
            break;
        }
    }
    ASSERT_TRUE(bHit);
    // 머리 윗면이 천장 아랫면(60)에 딱 닿음: feet - 40 == 60
    EXPECT_FLOAT_EQ(body.position.y - body.size.y, 60.f);

    // 이후 아래로 떨어진다
    for (int i = 0; i < 80; ++i)
    {
        (void)Tick(body, world);
    }
    EXPECT_TRUE(body.bGrounded);
    EXPECT_FLOAT_EQ(body.position.y, 100.f);
}

TEST(Physics, WallStopsWalkingFlushAndZeroesVelocity)
{
    GroundMap map = MakeFlat();
    map.FillRect(200, 0, 399, 99, true);   // 열 200 부터 높은 벽
    const PhysicsWorld world = WorldOf(map);
    Body               body  = StandingOn(world, 100.f, 100.f);

    bool bHit = false;
    for (int i = 0; i < 120; ++i)
    {
        bHit |= Tick(body, world, kRunSpeed).bHitWallRight;
    }
    EXPECT_TRUE(bHit);
    EXPECT_FLOAT_EQ(body.position.x + 12.f, 200.f);   // 오른쪽 모서리가 벽면에 정확히 닿음
    EXPECT_FLOAT_EQ(body.velocity.x, 0.f);
    EXPECT_TRUE(body.bGrounded);
}

TEST(Physics, LeftWallStopsToo)
{
    GroundMap map = MakeFlat();
    map.FillRect(0, 0, 49, 99, true);   // 열 0..49 벽 (오른쪽 면 = 50)
    const PhysicsWorld world = WorldOf(map);
    Body               body  = StandingOn(world, 150.f, 100.f);

    bool bHit = false;
    for (int i = 0; i < 120; ++i)
    {
        bHit |= Tick(body, world, -kRunSpeed).bHitWallLeft;
    }
    EXPECT_TRUE(bHit);
    EXPECT_FLOAT_EQ(body.position.x - 12.f, 50.f);
}

TEST(Physics, AirborneBodyCannotPassThroughWallOrSlideIntoLedgeCorner)
{
    GroundMap map = MakeFlat();
    map.FillRect(200, 40, 399, 99, true);   // 윗면이 y=40 인 큰 블록
    const PhysicsWorld world = WorldOf(map);
    Body               body  = StandingOn(world, 150.f, 100.f);

    body.velocity.y = -300.f;   // 정점 높이 ≈ 32px → 블록 윗면(40)보다 낮게 뜬다
    bool bHit       = false;
    for (int i = 0; i < 60; ++i)
    {
        const MoveResult result = Tick(body, world, kRunSpeed);
        bHit                    = bHit || result.bHitWallRight;
        ASSERT_LE(body.position.x + 12.f, 200.f + 0.001f) << "tick " << i;   // 벽 안으로 파고들지 않음
    }
    EXPECT_TRUE(bHit);
}

TEST(Physics, AirborneBodyClearingTheLedgeLandsOnTop)
{
    GroundMap map = MakeFlat();
    map.FillRect(200, 80, 399, 99, true);   // 윗면 y=80, 바닥보다 20px 높은 단
    const PhysicsWorld world = WorldOf(map);
    Body               body  = StandingOn(world, 150.f, 100.f);

    body.velocity.y = -kJumpSpeed;   // 충분히 높이 뜬다
    for (int i = 0; i < 60; ++i)
    {
        (void)Tick(body, world, 150.f);
    }
    EXPECT_TRUE(body.bGrounded);
    EXPECT_FLOAT_EQ(body.position.y, 80.f);
    EXPECT_GT(body.position.x, 200.f);
}

TEST(Physics, AirborneBodyDoesNotStepUpLedges)
{
    // 공중에서는 계단 오르기(step-up) 보정이 없다: 3px 위 턱 모서리에 걸리면 막힌다.
    GroundMap map = MakeFlat();
    map.FillRect(200, 94, 399, 99, true);   // 윗면 y=94 (바닥보다 6px 높음)
    const PhysicsWorld world = WorldOf(map);

    Body             body   = MakeBody(187.f, 98.f);   // 바닥 2px 위 공중, 오른쪽 모서리(199)가 턱 열(200) 바로 앞
    const MoveResult result = Tick(body, world, kRunSpeed);
    EXPECT_TRUE(result.bHitWallRight);
    EXPECT_GE(body.position.y, 98.f);   // 끌어올려지지 않는다
    EXPECT_FLOAT_EQ(body.position.x + 12.f, 200.f);
}

// ===========================================
//  플랫폼 / 벽 상자
// ===========================================

TEST(Physics, OneWayPlatformPassesFromBelowAndLandsFromAbove)
{
    const GroundMap map = MakeFlat();
    Platform        oneWay;
    oneWay.box     = Aabb::FromMinMax({ 100.f, 60.f }, { 200.f, 66.f });
    oneWay.bOneWay = true;

    PhysicsWorld world = WorldOf(map);
    world.platforms    = jug::Span<const Platform> { &oneWay, 1 };

    Body body       = StandingOn(world, 150.f, 100.f);
    body.velocity.y = -kJumpSpeed;

    bool bCeiling = false;
    bool bLanded  = false;
    for (int i = 0; i < 120; ++i)
    {
        const MoveResult result = Tick(body, world);
        bCeiling                = bCeiling || result.bHitCeiling;
        if (result.bLanded)
        {
            bLanded = true;
            EXPECT_EQ(result.platformIndex, 0);
            break;
        }
    }

    EXPECT_FALSE(bCeiling);   // 아래에서 위로는 통과
    EXPECT_TRUE(bLanded);
    EXPECT_FLOAT_EQ(body.position.y, 60.f);   // 윗면 위에 섰다
}

TEST(Physics, OneWayPlatformDoesNotCatchBodiesThatAreBelowTheTop)
{
    const GroundMap map = MakeFlat();
    Platform        oneWay;
    oneWay.box     = Aabb::FromMinMax({ 100.f, 60.f }, { 200.f, 66.f });
    oneWay.bOneWay = true;

    PhysicsWorld world = WorldOf(map);
    world.platforms    = jug::Span<const Platform> { &oneWay, 1 };

    // 발이 이미 윗면보다 아래(80)에 있는 채 낙하/점프 정점에 있는 경우
    Body body = MakeBody(150.f, 80.f);
    for (int i = 0; i < 60; ++i)
    {
        (void)Tick(body, world);
    }
    EXPECT_TRUE(body.bGrounded);
    EXPECT_FLOAT_EQ(body.position.y, 100.f);   // 바닥까지 내려갔다 (원웨이에 걸리지 않음)
}

TEST(Physics, OneWayPlatformDoesNotBlockWalkingSideways)
{
    const GroundMap map = MakeFlat();
    Platform        oneWay;
    oneWay.box     = Aabb::FromMinMax({ 150.f, 70.f }, { 170.f, 76.f });   // 몸통 높이 안쪽에 걸쳐 있음
    oneWay.bOneWay = true;

    PhysicsWorld world = WorldOf(map);
    world.platforms    = jug::Span<const Platform> { &oneWay, 1 };

    Body body = StandingOn(world, 100.f, 100.f);
    for (int i = 0; i < 90; ++i)
    {
        (void)Tick(body, world, kRunSpeed);
    }
    EXPECT_GT(body.position.x, 250.f);
}

TEST(Physics, DropThroughLetsBodyFallOffOneWayPlatform)
{
    const GroundMap map = MakeFlat();
    Platform        oneWay;
    oneWay.box     = Aabb::FromMinMax({ 100.f, 60.f }, { 200.f, 66.f });
    oneWay.bOneWay = true;

    PhysicsWorld world = WorldOf(map);
    world.platforms    = jug::Span<const Platform> { &oneWay, 1 };

    Body body = StandingOn(world, 150.f, 60.f);
    for (int i = 0; i < 5; ++i)
    {
        (void)Tick(body, world);
    }
    ASSERT_TRUE(body.bGrounded);
    ASSERT_FLOAT_EQ(body.position.y, 60.f);

    body.RequestDropThrough();
    for (int i = 0; i < 120; ++i)
    {
        (void)Tick(body, world);
    }
    EXPECT_TRUE(body.bGrounded);
    EXPECT_FLOAT_EQ(body.position.y, 100.f);   // 바닥까지 내려갔고 다시 원웨이에 걸리지 않았다
}

TEST(Physics, SolidPlatformSupportsBlocksSidesAndBumpsHead)
{
    const GroundMap map = MakeFlat();
    Platform        solid;
    solid.box     = Aabb::FromMinMax({ 150.f, 50.f }, { 220.f, 100.f });   // 바닥에 놓인 상자 (높이 50)
    solid.bOneWay = false;

    PhysicsWorld world = WorldOf(map);
    world.platforms    = jug::Span<const Platform> { &solid, 1 };

    // 옆에서 막힘
    Body body = StandingOn(world, 100.f, 100.f);
    bool bHit = false;
    for (int i = 0; i < 90; ++i)
    {
        bHit |= Tick(body, world, kRunSpeed).bHitWallRight;
    }
    EXPECT_TRUE(bHit);
    EXPECT_FLOAT_EQ(body.position.x + 12.f, 150.f);

    // 위에서 착지
    Body faller = MakeBody(180.f, 0.f);
    for (int i = 0; i < 120; ++i)
    {
        (void)Tick(faller, world);
    }
    EXPECT_TRUE(faller.bGrounded);
    EXPECT_FLOAT_EQ(faller.position.y, 50.f);

    // 아래에서 머리를 부딪힘: 높은 곳에 매달린 솔리드
    Platform hanging;
    hanging.box         = Aabb::FromMinMax({ 300.f, 20.f }, { 360.f, 40.f });
    PhysicsWorld world2 = WorldOf(map);
    world2.platforms    = jug::Span<const Platform> { &hanging, 1 };
    Body jumper         = StandingOn(world2, 330.f, 100.f);
    jumper.velocity.y   = -kJumpSpeed;
    bool bCeiling       = false;
    for (int i = 0; i < 40 && !bCeiling; ++i)
    {
        bCeiling = Tick(jumper, world2).bHitCeiling;
    }
    EXPECT_TRUE(bCeiling);
    EXPECT_FLOAT_EQ(jumper.position.y - jumper.size.y, 40.f);
}

TEST(Physics, WalkingOffPlatformEdgeFallsToGround)
{
    const GroundMap map = MakeFlat();
    Platform        solid;
    solid.box = Aabb::FromMinMax({ 100.f, 60.f }, { 160.f, 100.f });

    PhysicsWorld world = WorldOf(map);
    world.platforms    = jug::Span<const Platform> { &solid, 1 };

    Body body = StandingOn(world, 130.f, 60.f);
    ASSERT_FLOAT_EQ(body.position.y, 60.f);

    bool bLeft = false;
    for (int i = 0; i < 120; ++i)
    {
        bLeft |= Tick(body, world, 100.f).bLeftGround;
    }
    EXPECT_TRUE(bLeft);
    EXPECT_TRUE(body.bGrounded);
    EXPECT_FLOAT_EQ(body.position.y, 100.f);
}

TEST(Physics, WallBoxBlocksAndCanBeStoodOn)
{
    const GroundMap map = MakeFlat();
    Wall            wall;
    wall.box = Aabb::FromMinMax({ 200.f, 0.f }, { 210.f, 100.f });

    PhysicsWorld world = WorldOf(map);
    world.walls        = jug::Span<const Wall> { &wall, 1 };

    Body body = StandingOn(world, 100.f, 100.f);
    bool bHit = false;
    for (int i = 0; i < 90; ++i)
    {
        bHit |= Tick(body, world, kRunSpeed).bHitWallRight;
    }
    EXPECT_TRUE(bHit);
    EXPECT_FLOAT_EQ(body.position.x + 12.f, 200.f);

    // 왼쪽에서 접근해도 막힌다
    Body other = StandingOn(world, 300.f, 100.f);
    bHit       = false;
    for (int i = 0; i < 90; ++i)
    {
        bHit |= Tick(other, world, -kRunSpeed).bHitWallLeft;
    }
    EXPECT_TRUE(bHit);
    EXPECT_FLOAT_EQ(other.position.x - 12.f, 210.f);
}

TEST(Physics, BodiesCanOptOutOfCollisionLayers)
{
    GroundMap map = MakeFlat();
    map.FillRect(200, 0, 399, 99, true);
    const PhysicsWorld world = WorldOf(map);

    Body ghost           = MakeBody(100.f, 100.f);
    ghost.bCollideGround = false;
    ghost.bUseGravity    = false;
    for (int i = 0; i < 120; ++i)
    {
        (void)Tick(ghost, world, kRunSpeed);
    }
    EXPECT_GT(ghost.position.x, 250.f);
    EXPECT_FLOAT_EQ(ghost.position.y, 100.f);
}

// ===========================================
//  결정성
// ===========================================

namespace
{
struct Snapshot
{
    uint32_t x;
    uint32_t y;
    uint32_t vx;
    uint32_t vy;
    bool     bGrounded;

    [[nodiscard]] bool operator==(const Snapshot&) const = default;
};

Snapshot Capture(
    const Body& _body)
{
    return { std::bit_cast<uint32_t>(_body.position.x), std::bit_cast<uint32_t>(_body.position.y), std::bit_cast<uint32_t>(_body.velocity.x), std::bit_cast<uint32_t>(_body.velocity.y), _body.bGrounded };
}

// 램프, 낭떠러지, 원웨이, 벽이 섞인 시나리오를 입력 스크립트로 600스텝 돌린다
jug::Vector<Snapshot> RunScenario()
{
    GroundMap map = MakeRamp45(50, 90, 100);
    map.FillRect(260, 100, 330, 199, false);   // 구덩이
    map.FillRect(330, 40, 399, 199, true);
    Platform oneWay;
    oneWay.box     = Aabb::FromMinMax({ 120.f, 50.f }, { 180.f, 56.f });
    oneWay.bOneWay = true;

    PhysicsWorld world = WorldOf(map);
    world.platforms    = jug::Span<const Platform> { &oneWay, 1 };

    Body                  body = MakeBody(20.f, 100.f);
    jug::Vector<Snapshot> history;
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
        (void)Tick(body, world, vx);
        history.push_back(Capture(body));
    }
    return history;
}
}   // namespace

TEST(Physics, SameInputsProduceBitIdenticalResultsAfter600Steps)
{
    const jug::Vector<Snapshot> a = RunScenario();
    const jug::Vector<Snapshot> b = RunScenario();
    ASSERT_EQ(a.size(), 600u);
    ASSERT_EQ(a.size(), b.size());
    for (size_t i = 0; i < a.size(); ++i)
    {
        ASSERT_TRUE(a[i] == b[i]) << "diverged at step " << i;
    }
}

TEST(Physics, ScenarioActuallyMovesTheBody)
{
    // 결정성 테스트가 "아무 것도 안 하는" 시나리오로 통과하지 않도록 최소한의 움직임을 확인
    const jug::Vector<Snapshot> history = RunScenario();
    const float                 endX    = std::bit_cast<float>(history.back().x);
    EXPECT_GT(endX, 100.f);
}
