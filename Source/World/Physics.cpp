#include "World/Physics.h"

#include "Base/Math.h"

constexpr float kOneWayEpsilon = 0.001f;
constexpr float kInf           = 1e30f;

struct StepContext
{
    Body&                body;
    const PhysicsWorld&  world;
    const PhysicsTuning& tuning;
    MoveResult           result;
    bool                 bIgnoreOneWay = false;
};

enum class eSupport
{
    None,
    Found,
    Blocked
};

struct SupportResult
{
    eSupport kind          = eSupport::None;
    float    y             = kInf;
    int      platformIndex = -1;
    int      wallIndex     = -1;
};

namespace
{
[[nodiscard]] float HalfWidthOf_(
    const Body& _body)
{
    return _body.size.x * 0.5f;
}

[[nodiscard]] float FootHalf_(
    const Body& _body)
{
    return Max(_body.footHalfWidth, 0.5f);
}

[[nodiscard]] SupportResult FindSupport_(
    const StepContext& _ctx,
    float              _x,
    float              _yMin,
    float              _yMax,
    float              _yMinOneWay,
    bool               _bCheckBlocked)
{
    const Body& body = _ctx.body;
    const float fh   = FootHalf_(body);

    SupportResult best;

    if (_ctx.world.pGround && body.bCollideGround)
    {
        const GroundMap& ground = *_ctx.world.pGround;

        const int col0 = FloorToInt(_x - fh);
        const int col1 = Max(col0, CeilToInt(_x + fh) - 1);
        const int rLo  = CeilToInt(_yMin);
        const int rHi  = FloorToInt(_yMax);
        if (rHi >= rLo)
        {
            for (int col = col0; col <= col1; ++col)
            {
                if (_bCheckBlocked && ground.IsSolid(col, rLo) && ground.IsSolid(col, rLo - 1))
                {
                    best.kind = eSupport::Blocked;
                    return best;
                }

                if (const std::optional<int> row = ground.FindGroundY(col, rLo, rHi - rLo))
                {
                    const float y = static_cast<float>(*row);
                    if (y < best.y)
                    {
                        best.kind = eSupport::Found;
                        best.y    = y;
                    }
                }
            }
        }
    }

    const float footMin = _x - fh;
    const float footMax = _x + fh;

    for (size_t i = 0; i < _ctx.world.platforms.size(); ++i)
    {
        const Platform& platform = _ctx.world.platforms[i];
        if (platform.bOneWay)
        {
            if (!body.bCollideOneWay || _ctx.bIgnoreOneWay)
            {
                continue;
            }
        }
        else if (!body.bCollideSolids)
        {
            continue;
        }

        if (!(platform.box.min.x < footMax && platform.box.max.x > footMin))
        {
            continue;
        }

        const float top = platform.box.min.y;
        const float lo  = platform.bOneWay ? _yMinOneWay : _yMin;
        if (top >= lo && top <= _yMax && top < best.y)
        {
            best.kind          = eSupport::Found;
            best.y             = top;
            best.platformIndex = static_cast<int>(i);
            best.wallIndex     = -1;
        }
    }

    if (body.bCollideSolids)
    {
        for (size_t i = 0; i < _ctx.world.walls.size(); ++i)
        {
            const Wall& wall = _ctx.world.walls[i];
            if (!(wall.box.min.x < footMax && wall.box.max.x > footMin))
            {
                continue;
            }

            const float top = wall.box.min.y;
            if (top >= _yMin && top <= _yMax && top < best.y)
            {
                best.kind          = eSupport::Found;
                best.y             = top;
                best.platformIndex = -1;
                best.wallIndex     = static_cast<int>(i);
            }
        }
    }

    return best;
}

void TestLateralBlock_(
    const Aabb&           _testBox,
    const Aabb&           _block,
    float                 _oldX,
    float                 _half,
    int                   _dir,
    std::optional<float>& _snapX)
{
    if (!_testBox.Overlaps(_block))
    {
        return;
    }

    const bool bAhead = _dir > 0 ? _block.min.x >= _oldX + _half - kOneWayEpsilon : _block.max.x <= _oldX - _half + kOneWayEpsilon;
    if (!bAhead)
    {
        return;
    }
    const float x = _dir > 0 ? _block.min.x - _half : _block.max.x + _half;
    if (!_snapX || (_dir > 0 ? x < *_snapX : x > *_snapX))
    {
        _snapX = x;
    }
}

void TestCeilingBlock_(
    const Aabb& _block,
    float       _left,
    float       _right,
    float       _top0,
    float       _top1,
    float       _bodyHeight,
    float&      _stopFeet)
{
    if (_block.min.x < _right && _block.max.x > _left && _block.max.y > _top1 && _block.max.y <= _top0 + kOneWayEpsilon && _block.min.y < _top0)
    {
        const float feetAtBlock = _block.max.y + _bodyHeight;
        if (_stopFeet == kInf || feetAtBlock > _stopFeet)
        {
            _stopFeet = feetAtBlock;
        }
    }
}

[[nodiscard]] std::optional<float> FindLateralBlock_(
    const StepContext& _ctx,
    float              _oldX,
    float              _newX,
    float              _feetY,
    int                _dir,
    float              _stepZone)
{
    const Body& body  = _ctx.body;
    const float half  = HalfWidthOf_(body);
    const float left  = _newX - half;
    const float right = _newX + half;
    const float top   = _feetY - body.size.y;
    const float limit = _feetY - _stepZone;

    if (limit <= top)
    {
        return std::nullopt;
    }

    std::optional<float> snapX;

    if (_ctx.world.pGround && body.bCollideGround)
    {
        const GroundMap& ground  = *_ctx.world.pGround;
        const int        col     = _dir > 0 ? CeilToInt(right) - 1 : FloorToInt(left);
        const int        colIn   = col - _dir;
        const int        rowTop  = FloorToInt(top);
        const int        rowFeet = CeilToInt(_feetY);
        if (rowFeet > rowTop)
        {
            const int                span  = rowFeet - 1 - rowTop;
            const std::optional<int> edgeY = ground.FindGroundY(col, rowTop, span);
            if (edgeY)
            {
                const std::optional<int> innerY = ground.FindGroundY(colIn, rowTop, span);
                const int                base   = innerY ? *innerY : rowFeet;
                if (base - *edgeY > FloorToInt(_stepZone))
                {
                    snapX = _dir > 0 ? static_cast<float>(col) - half : static_cast<float>(col + 1) + half;
                }
            }
        }
    }

    if (body.bCollideSolids)
    {
        const Aabb testBox = {
            {  left,   top },
            { right, limit }
        };

        for (const Platform& platform: _ctx.world.platforms)
        {
            if (!platform.bOneWay)
            {
                TestLateralBlock_(testBox, platform.box, _oldX, half, _dir, snapX);
            }
        }
        for (const Wall& wall: _ctx.world.walls)
        {
            TestLateralBlock_(testBox, wall.box, _oldX, half, _dir, snapX);
        }
    }

    return snapX;
}

[[nodiscard]] bool IsHeadBlocked_(
    const StepContext& _ctx,
    float              _x,
    float              _oldFeet,
    float              _newFeet)
{
    const Body& body   = _ctx.body;
    const float half   = HalfWidthOf_(body);
    const float left   = _x - half;
    const float right  = _x + half;
    const float oldTop = _oldFeet - body.size.y;
    const float newTop = _newFeet - body.size.y;

    if (_ctx.world.pGround && body.bCollideGround)
    {
        const int row0 = FloorToInt(newTop);
        const int row1 = FloorToInt(oldTop) - 1;
        if (row1 >= row0 && _ctx.world.pGround->IsRectSolid(FloorToInt(left), row0, CeilToInt(right) - 1, row1))
        {
            return true;
        }
    }

    if (body.bCollideSolids)
    {
        const Aabb region = {
            {  left, newTop },
            { right, oldTop }
        };
        for (const Platform& platform: _ctx.world.platforms)
        {
            if (!platform.bOneWay && region.Overlaps(platform.box))
            {
                return true;
            }
        }
        for (const Wall& wall: _ctx.world.walls)
        {
            if (region.Overlaps(wall.box))
            {
                return true;
            }
        }
    }
    return false;
}

void SetSupport_(
    StepContext&         _ctx,
    const SupportResult& _support)
{
    _ctx.body.position.y      = _support.y;
    _ctx.result.platformIndex = _support.platformIndex;
    _ctx.result.wallIndex     = _support.wallIndex;
}

[[nodiscard]] bool Resettle_(
    StepContext& _ctx,
    float        _x)
{
    Body&       body   = _ctx.body;
    const float feet   = body.position.y;
    const float stepUp = _ctx.tuning.maxStepUp;

    const SupportResult support = FindSupport_(_ctx, _x, feet - stepUp, feet + _ctx.tuning.maxSnapDown, feet - kOneWayEpsilon, true);
    if (support.kind == eSupport::Blocked)
    {
        return false;
    }

    if (support.kind == eSupport::None)
    {
        body.bGrounded            = false;
        _ctx.result.platformIndex = -1;
        _ctx.result.wallIndex     = -1;
        return true;
    }

    if (support.y < feet && IsHeadBlocked_(_ctx, _x, feet, support.y))
    {
        return false;
    }

    SetSupport_(_ctx, support);
    return true;
}

void MoveX_(
    StepContext& _ctx,
    float        _dx)
{
    Body& body = _ctx.body;
    if (_dx == 0.f)
    {
        return;
    }

    const int   dir   = _dx > 0.f ? 1 : -1;
    const int   steps = Max(1, CeilToInt(Abs(_dx)));
    const float s     = _dx / static_cast<float>(steps);

    for (int i = 0; i < steps; ++i)
    {
        const float newX = body.position.x + s;
        const float zone = body.bGrounded ? _ctx.tuning.maxStepUp : 0.f;

        if (const std::optional<float> snapX = FindLateralBlock_(_ctx, body.position.x, newX, body.position.y, dir, zone))
        {
            body.position.x                                                  = dir > 0 ? Max(body.position.x, Min(newX, *snapX)) : Min(body.position.x, Max(newX, *snapX));
            (dir > 0 ? _ctx.result.bHitWallRight : _ctx.result.bHitWallLeft) = true;
            body.velocity.x                                                  = 0.f;
            return;
        }

        if (body.bGrounded)
        {
            const float oldFeet = body.position.y;
            if (!Resettle_(_ctx, newX))
            {
                (dir > 0 ? _ctx.result.bHitWallRight : _ctx.result.bHitWallLeft) = true;
                body.velocity.x                                                  = 0.f;
                body.position.y                                                  = oldFeet;
                return;
            }
        }
        body.position.x = newX;
    }
}

void MoveY_(
    StepContext& _ctx,
    float        _dy)
{
    Body& body = _ctx.body;
    if (_dy == 0.f)
    {
        return;
    }

    const int   steps = Max(1, CeilToInt(Abs(_dy)));
    const float s     = _dy / static_cast<float>(steps);

    for (int i = 0; i < steps; ++i)
    {
        const float f0 = body.position.y;
        const float f1 = f0 + s;

        if (s > 0.f)
        {
            const SupportResult support = FindSupport_(_ctx, body.position.x, f0, f1, f0, false);
            if (support.kind == eSupport::Found)
            {
                SetSupport_(_ctx, support);
                body.velocity.y = 0.f;
                body.bGrounded  = true;
                return;
            }
            body.position.y = f1;
        }
        else
        {
            const float half  = HalfWidthOf_(body);
            const float left  = body.position.x - half;
            const float right = body.position.x + half;
            const float top0  = f0 - body.size.y;
            const float top1  = f1 - body.size.y;

            float stopFeet = kInf;

            if (_ctx.world.pGround && body.bCollideGround)
            {
                const int rowNew = FloorToInt(top1);
                const int rowOld = FloorToInt(top0);
                if (rowNew < rowOld && _ctx.world.pGround->IsRectSolid(FloorToInt(left), rowNew, CeilToInt(right) - 1, rowOld - 1))
                {
                    stopFeet = static_cast<float>(rowNew + 1) + body.size.y;
                }
            }

            if (body.bCollideSolids)
            {

                for (const Platform& platform: _ctx.world.platforms)
                {
                    if (!platform.bOneWay)
                    {
                        TestCeilingBlock_(platform.box, left, right, top0, top1, body.size.y, stopFeet);
                    }
                }
                for (const Wall& wall: _ctx.world.walls)
                {
                    TestCeilingBlock_(wall.box, left, right, top0, top1, body.size.y, stopFeet);
                }
            }

            if (stopFeet != kInf)
            {
                body.position.y = Min(f0, stopFeet);
                if (body.velocity.y < 0.f)
                {
                    body.velocity.y = 0.f;
                }
                _ctx.result.bHitCeiling = true;
                return;
            }
            body.position.y = f1;
        }
    }
}
}   // namespace

MoveResult Physics::Step(
    Body&                _body,
    const PhysicsWorld&  _world,
    const PhysicsTuning& _tuning,
    float                _dt)
{
    StepContext ctx = { _body, _world, _tuning, {}, false };

    const bool bWasGrounded = _body.bGrounded;

    if (_body.velocity.y < 0.f)
    {
        _body.bGrounded = false;
    }

    if (_body.dropThroughSteps > 0)
    {
        ctx.bIgnoreOneWay = true;
        --_body.dropThroughSteps;
    }

    MoveX_(ctx, _body.velocity.x * _dt);

    if (_body.bGrounded)
    {
        (void)Resettle_(ctx, _body.position.x);
    }

    if (!_body.bGrounded)
    {
        if (_body.bUseGravity)
        {
            _body.velocity.y += _tuning.gravity * _body.gravityScale * _dt;
            _body.velocity.y = Min(_body.velocity.y, _tuning.terminalVelocity);
        }
        MoveY_(ctx, _body.velocity.y * _dt);
    }
    else
    {
        _body.velocity.y = 0.f;
    }

    MoveResult& result  = ctx.result;
    result.bGroundedNow = _body.bGrounded;
    result.bLanded      = !bWasGrounded && _body.bGrounded;
    result.bLeftGround  = bWasGrounded && !_body.bGrounded;
    if (!_body.bGrounded)
    {
        result.platformIndex = -1;
        result.wallIndex     = -1;
    }
    else if (_world.pGround && result.platformIndex < 0 && result.wallIndex < 0)
    {
        constexpr int kSlopeHalfWidth = 4;
        constexpr int kSlopeSearch    = 8;
        const int     gx              = FloorToInt(_body.position.x);
        const int     gy              = FloorToInt(_body.position.y);
        result.groundSlope            = _world.pGround->EstimateSlope(gx, gy, kSlopeHalfWidth, kSlopeSearch);
        result.groundNormal           = _world.pGround->EstimateNormal(gx, gy, kSlopeHalfWidth, kSlopeSearch);
    }
    return result;
}

bool Physics::SnapToGround(
    Body&               _body,
    const PhysicsWorld& _world,
    float               _maxDist)
{
    const PhysicsTuning tuning;
    StepContext         ctx = { _body, _world, tuning, {}, false };

    const SupportResult support = FindSupport_(ctx, _body.position.x, _body.position.y, _body.position.y + _maxDist, _body.position.y, false);
    if (support.kind != eSupport::Found)
    {
        return false;
    }

    _body.position.y = support.y;
    _body.velocity.y = 0.f;
    _body.bGrounded  = true;
    return true;
}

Aabb Body::GetBox() const
{
    return Aabb::FromBottomCenter(position, size);
}

void Body::RequestDropThrough(
    int _steps)
{
    dropThroughSteps = _steps;
}
