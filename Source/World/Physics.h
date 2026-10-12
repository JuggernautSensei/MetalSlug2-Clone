#pragma once
#include "Base/Math.h"
#include "Base/Types.h"
#include "World/Constants.h"
#include "World/Geometry.h"
#include "World/GroundMap.h"

struct Platform
{
    Aabb box     = {};
    bool bOneWay = false;
};

struct Wall
{
    Aabb box = {};
};

struct PhysicsWorld
{
    const GroundMap*     pGround   = nullptr;
    Span<const Platform> platforms = {};
    Span<const Wall>     walls     = {};
};

struct Body
{
    Vec2 position = { 0.f, 0.f };
    Vec2 velocity = { 0.f, 0.f };
    Vec2 size     = { 24.f, 40.f };

    float footHalfWidth = 1.f;

    float gravityScale = 1.f;

    bool bUseGravity    = true;
    bool bCollideGround = true;
    bool bCollideSolids = true;
    bool bCollideOneWay = true;

    bool bGrounded        = false;
    int  dropThroughSteps = 0;   // > 0 이면 남은 틱 동안 원웨이를 무시한다

    [[nodiscard]] Aabb GetBox() const;

    void RequestDropThrough(
        const int _steps = PhysicsTuning {}.dropThroughSteps);
};

struct MoveResult
{
    bool bGroundedNow  = false;
    bool bLanded       = false;
    bool bLeftGround   = false;
    bool bHitWallLeft  = false;
    bool bHitWallRight = false;
    bool bHitCeiling   = false;

    int platformIndex = -1;
    int wallIndex     = -1;

    float groundSlope  = 0.f;
    Vec2  groundNormal = { 0.f, -1.f };
};

class Physics
{
public:
    [[nodiscard]] static MoveResult Step(Body& _body, const PhysicsWorld& _world, const PhysicsTuning& _tuning = {}, float _dt = kStepSec);

    static bool SnapToGround(Body& _body, const PhysicsWorld& _world, float _maxDist);
};
