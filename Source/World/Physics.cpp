#include "World/Physics.h"

#include <JugX/Math.h>

namespace slug
{

namespace
{
    constexpr float kOneWayEpsilon = 0.001f;
    constexpr float kInf           = 1.0e30f;

    // 한 스텝 동안의 작업 문맥
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
        Blocked   // 올라설 수 없는 높이의 턱
    };

    struct SupportResult
    {
        eSupport kind          = eSupport::None;
        float    y             = kInf;
        int      platformIndex = -1;
        int      wallIndex     = -1;
    };

    [[nodiscard]] float HalfWidthOf_(
        const Body& _body)
    {
        return _body.size.x * 0.5f;
    }

    [[nodiscard]] float FootHalf_(
        const Body& _body)
    {
        return jug::Max(_body.footHalfWidth, 0.5f);
    }

    // _yMin ≤ 지면 높이 ≤ _yMax 인 가장 높은 지지면을 찾는다.
    //  - 원웨이는 윗면이 _yMinOneWay 이상일 때만 지지면이 된다 (아래에서 끌어올리지 않기 위함).
    //  - _bCheckBlocked: 탐색 시작 행이 두 줄 연속 고체면 오를 수 없는 턱으로 보고 Blocked 를 반환한다.
    [[nodiscard]] SupportResult FindSupport_(
        const StepContext& _ctx,
        const float        _x,
        const float        _yMin,
        const float        _yMax,
        const float        _yMinOneWay,
        const bool         _bCheckBlocked)
    {
        const Body& body = _ctx.body;
        const float fh   = FootHalf_(body);

        SupportResult best;

        // --- 픽셀 지면 ---
        if (_ctx.world.pGround && body.bCollideGround)
        {
            const GroundMap& ground = *_ctx.world.pGround;

            const int col0 = FloorToInt(_x - fh);
            const int col1 = jug::Max(col0, CeilToInt(_x + fh) - 1);
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

        // --- 솔리드 / 원웨이 플랫폼, 벽 ---
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

    // 가로 이동 후보 위치 _newX 에서 진행 방향 가장자리가 막히는지 검사한다.
    // 막히면 벽면에 딱 붙는 x 좌표를 반환한다. _stepZone 높이(발 쪽)는 검사에서 제외한다 (경사/턱 오르기 허용).
    [[nodiscard]] std::optional<float> FindLateralBlock_(
        const StepContext& _ctx,
        const float        _oldX,
        const float        _newX,
        const float        _feetY,
        const int          _dir,
        const float        _stepZone)
    {
        const Body& body  = _ctx.body;
        const float half  = HalfWidthOf_(body);
        const float left  = _newX - half;
        const float right = _newX + half;
        const float top   = _feetY - body.size.y;
        const float limit = _feetY - _stepZone;   // 이 아래(발 쪽)는 계단 영역

        if (limit <= top)
        {
            return std::nullopt;
        }

        std::optional<float> snapX;

        // 픽셀 지면: 진행 방향 가장자리 열(새로 걸치는 열, 서브스텝이 1px 이하이므로 하나)과 그 안쪽 이웃 열의
        // "몸 높이 범위 안 첫 고체 행" 을 비교한다. 이웃 열보다 _stepZone 이하로만 솟아 있으면 경사/턱이므로 통과,
        // 더 많이 솟아 있으면 벽이다. (가장자리가 발 센서보다 반폭만큼 앞서 있어도 45° 경사가 막히지 않는다)
        if (_ctx.world.pGround && body.bCollideGround)
        {
            const GroundMap& ground  = *_ctx.world.pGround;
            const int        col     = _dir > 0 ? CeilToInt(right) - 1 : FloorToInt(left);
            const int        colIn   = col - _dir;
            const int        rowTop  = FloorToInt(top);
            const int        rowFeet = CeilToInt(_feetY);   // 몸이 차지하는 행은 [rowTop, rowFeet)
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

            auto TestBlock = [&](const Aabb& _block) {
                if (!testBox.Overlaps(_block))
                {
                    return;
                }

                // 이미 옆면이 겹쳐 있는 블록(예: 플랫폼 가장자리를 막 벗어나 떨어지는 중)은 "앞"이 아니므로 막지 않는다.
                // 이동 전 진행 방향 모서리가 블록 면을 넘지 않은 경우에만 벽으로 취급한다.
                const bool bAhead = _dir > 0 ? _block.min.x >= _oldX + half - kOneWayEpsilon : _block.max.x <= _oldX - half + kOneWayEpsilon;
                if (!bAhead)
                {
                    return;
                }
                const float x = _dir > 0 ? _block.min.x - half : _block.max.x + half;
                // 더 먼저 부딪히는(이동 방향 기준 가까운) 쪽을 채택
                if (!snapX || (_dir > 0 ? x < *snapX : x > *snapX))
                {
                    snapX = x;
                }
            };

            for (const Platform& platform: _ctx.world.platforms)
            {
                if (!platform.bOneWay)
                {
                    TestBlock(platform.box);
                }
            }
            for (const Wall& wall: _ctx.world.walls)
            {
                TestBlock(wall.box);
            }
        }

        return snapX;
    }

    // 위로 _oldFeet → _newFeet(위쪽) 올라설 때 머리가 새로 차지하는 영역이 막혀 있는가
    [[nodiscard]] bool IsHeadBlocked_(
        const StepContext& _ctx,
        const float        _x,
        const float        _oldFeet,
        const float        _newFeet)
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

    // 지면에 붙어 있는 상태를 유지한다: 계단 오르기 / 스냅 다운. 지지면이 사라졌으면 공중 상태로 전환.
    // 반환: false 이면 올라설 수 없는 턱 (호출자가 벽으로 처리)
    [[nodiscard]] bool Resettle_(
        StepContext& _ctx,
        const float  _x)
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

        // 위로 올라서는 경우 머리 위 공간 확인
        if (support.y < feet && IsHeadBlocked_(_ctx, _x, feet, support.y))
        {
            return false;
        }

        SetSupport_(_ctx, support);
        return true;
    }

    void MoveX_(
        StepContext& _ctx,
        const float  _dx)
    {
        Body& body = _ctx.body;
        if (_dx == 0.f)
        {
            return;
        }

        const int   dir   = _dx > 0.f ? 1 : -1;
        const int   steps = jug::Max(1, CeilToInt(jug::Abs(_dx)));
        const float s     = _dx / static_cast<float>(steps);

        for (int i = 0; i < steps; ++i)
        {
            const float newX = body.position.x + s;
            const float zone = body.bGrounded ? _ctx.tuning.maxStepUp : 0.f;

            if (const std::optional<float> snapX = FindLateralBlock_(_ctx, body.position.x, newX, body.position.y, dir, zone))
            {
                // 벽면에 딱 붙인다 (이미 파고든 경우는 뒤로 밀지 않는다)
                body.position.x                                                  = dir > 0 ? jug::Max(body.position.x, jug::Min(newX, *snapX)) : jug::Min(body.position.x, jug::Max(newX, *snapX));
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
        const float  _dy)
    {
        Body& body = _ctx.body;
        if (_dy == 0.f)
        {
            return;
        }

        const int   steps = jug::Max(1, CeilToInt(jug::Abs(_dy)));
        const float s     = _dy / static_cast<float>(steps);

        for (int i = 0; i < steps; ++i)
        {
            const float f0 = body.position.y;
            const float f1 = f0 + s;

            if (s > 0.f)
            {
                // 하강: [f0, f1] 안에 지지면이 있으면 착지
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
                // 상승: 머리 위 천장 검사
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
                    auto TestBlock = [&](const Aabb& _block) {
                        // 머리가 블록 아랫면을 통과하려 하는가
                        if (_block.min.x < right && _block.max.x > left && _block.max.y > top1 && _block.max.y <= top0 + kOneWayEpsilon && _block.min.y < top0)
                        {
                            const float feetAtBlock = _block.max.y + body.size.y;
                            if (stopFeet == kInf || feetAtBlock > stopFeet)
                            {
                                stopFeet = feetAtBlock;
                            }
                        }
                    };

                    for (const Platform& platform: _ctx.world.platforms)
                    {
                        if (!platform.bOneWay)
                        {
                            TestBlock(platform.box);
                        }
                    }
                    for (const Wall& wall: _ctx.world.walls)
                    {
                        TestBlock(wall.box);
                    }
                }

                if (stopFeet != kInf)
                {
                    body.position.y = jug::Min(f0, stopFeet);
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

// ===========================================
//  Physics
// ===========================================

MoveResult Physics::Step(
    Body&                _body,
    const PhysicsWorld&  _world,
    const PhysicsTuning& _tuning,
    const float          _dt)
{
    StepContext ctx = { _body, _world, _tuning, {}, false };

    const bool bWasGrounded = _body.bGrounded;

    // 위로 향하는 속도가 있으면 지면을 떠난다 (점프)
    if (_body.velocity.y < 0.f)
    {
        _body.bGrounded = false;
    }

    if (_body.dropThroughSteps > 0)
    {
        ctx.bIgnoreOneWay = true;
        --_body.dropThroughSteps;
    }

    // 1) 가로 이동 (지면 위라면 경사 추종 포함)
    MoveX_(ctx, _body.velocity.x * _dt);

    // 2) 가로로 움직이지 않았더라도 지지면이 유지되는지 확인
    //    (올라설 수 없는 턱에 끼었다면 Resettle_ 이 false 를 돌려주지만 위치는 그대로 유지된다)
    if (_body.bGrounded)
    {
        (void)Resettle_(ctx, _body.position.x);
    }

    // 3) 공중이면 중력 + 세로 이동
    if (!_body.bGrounded)
    {
        if (_body.bUseGravity)
        {
            _body.velocity.y += _tuning.gravity * _body.gravityScale * _dt;
            _body.velocity.y = jug::Min(_body.velocity.y, _tuning.terminalVelocity);
        }
        MoveY_(ctx, _body.velocity.y * _dt);
    }
    else
    {
        _body.velocity.y = 0.f;
    }

    // 결과 정리
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
    const float         _maxDist)
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

}   // namespace slug
