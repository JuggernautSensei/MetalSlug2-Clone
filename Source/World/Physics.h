#pragma once
#include <JugX/Typedef.h>
#include <JugX/Vector2.h>

#include "World/Constants.h"
#include "World/Geometry.h"
#include "World/GroundMap.h"

namespace slug
{

// ===========================================
//  Platform / Wall
//   Platform : 위에서만 설 수 있는(원웨이) 또는 사방이 막힌(솔리드) 사각형.
//   Wall     : 사방이 막힌 사각형 (카메라 락 경계, 보이지 않는 벽 등).
// ===========================================

struct Platform
{
    Aabb box     = {};
    bool bOneWay = false;
};

struct Wall
{
    Aabb box = {};
};

// ===========================================
//  PhysicsWorld
//   한 스텝의 충돌 대상 모음. 모두 비소유 참조이므로 호출 동안만 유효하면 된다.
// ===========================================

struct PhysicsWorld
{
    const GroundMap*          pGround   = nullptr;
    jug::Span<const Platform> platforms = {};
    jug::Span<const Wall>     walls     = {};
};

// ===========================================
//  Body
//   운동학적(kinematic) 물체. position 은 발 중앙(하단 중앙)이다.
//   속도 단위는 px/s. 호출자가 의도한 이동 속도를 velocity.x 에, 점프 초속을 velocity.y 에(음수=위) 넣는다.
// ===========================================

struct Body
{
    jug::VECTOR2 position = { 0.f, 0.f };
    jug::VECTOR2 velocity = { 0.f, 0.f };
    jug::VECTOR2 size     = { 24.f, 40.f };   // 충돌 상자 크기 (폭, 높이)

    // 지면(픽셀)에 서기 위해 사용하는 발 센서의 반폭. 경사에서 몸이 뜨는 정도를 결정한다. (최소 0.5 로 취급)
    float footHalfWidth = 1.f;

    float gravityScale = 1.f;

    bool bUseGravity    = true;
    bool bCollideGround = true;   // 픽셀 지면 맵
    bool bCollideSolids = true;   // Wall + 솔리드 Platform
    bool bCollideOneWay = true;   // 원웨이 Platform

    // --- 상태 (Physics::Step 이 갱신) ---
    bool bGrounded        = false;
    int  dropThroughSteps = 0;   // > 0 이면 남은 틱 동안 원웨이를 무시한다

    [[nodiscard]] Aabb GetBox() const
    {
        return Aabb::FromBottomCenter(position, size);
    }

    // 아래 점프: 지정한 틱 수 동안 원웨이 플랫폼을 통과한다.
    void RequestDropThrough(
        const int _steps = PhysicsTuning {}.dropThroughSteps)
    {
        dropThroughSteps = _steps;
    }
};

// ===========================================
//  MoveResult
//   한 스텝 이동의 결과 보고
// ===========================================

struct MoveResult
{
    bool bGroundedNow  = false;
    bool bLanded       = false;   // 공중 → 지면
    bool bLeftGround   = false;   // 지면 → 공중 (점프 또는 낭떠러지)
    bool bHitWallLeft  = false;
    bool bHitWallRight = false;
    bool bHitCeiling   = false;

    int platformIndex = -1;   // 서 있는 Platform 인덱스 (없으면 -1)
    int wallIndex     = -1;   // 서 있는 Wall 인덱스 (없으면 -1)

    // 발 아래 지면의 근사 기울기(dy/dx, 오른쪽으로 내려가면 +)와 위쪽 법선. 픽셀 지면에 서 있을 때만 의미가 있다.
    float        groundSlope  = 0.f;
    jug::VECTOR2 groundNormal = { 0.f, -1.f };
};

// ===========================================
//  Physics
//   결정적 고정 스텝 운동학 이동.
//    1) 가로 이동 (서브스텝 ≤ 1px). 지면에 서 있으면 경사 오르기(step-up)/내려가기(step-down 스냅) 적용.
//    2) 공중이면 중력 → 세로 이동 (서브스텝 ≤ 1px). 착지/천장 처리.
//   픽셀 지면은 "발 센서(footHalfWidth) + 좌우 가장자리(계단 영역 위)" 로만 검사한다 (Metal Slug 식).
// ===========================================

class Physics
{
public:
    [[nodiscard]] static MoveResult Step(Body& _body, const PhysicsWorld& _world, const PhysicsTuning& _tuning = {}, float _dt = kStepSec);

    // 현재 위치 아래 _maxDist 이내의 지면(픽셀/플랫폼)에 발을 붙인다. 스폰 직후 배치용. 성공 시 true.
    static bool SnapToGround(Body& _body, const PhysicsWorld& _world, float _maxDist);
};

}   // namespace slug
