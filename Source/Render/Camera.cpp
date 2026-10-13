#include "Render/Camera.h"

#include <cmath>

#include "Base/Math.h"
#include "Render/Renderer.h"

constexpr float kRampStart     = 1.f;
constexpr float kRampAccel     = 0.19f;
constexpr float kDecayPerTick  = 0.80f;
constexpr float kStopSpeed     = 0.05f;
constexpr float kGlideEnterGap = 96.f;
constexpr float kGlideExitGap  = 0.05f;
constexpr float kGlideAccel    = 1.f;
constexpr float kMaxStep       = 12.f;
constexpr float kStepGrowth    = 1.5f;
constexpr float kStepFloor     = 1.f;                   // 걸음이 작을 때 허용되는 최소 증가폭
constexpr float kMaxSpeedSec   = kMaxStep / kStepSec;
constexpr float kBoundsEase    = 2.f;
constexpr float kCorrectionSec = 0.5f;

namespace
{
[[nodiscard]] float RoundToInt_(
    const float _value)
{
    return std::floor(_value + 0.5f);
}

[[nodiscard]] float BrakeSpeed_(
    const float _gap)
{
    return (-kGlideAccel + Sqrt(kGlideAccel * kGlideAccel + 8.f * kGlideAccel * _gap)) * 0.5f;
}

// 한 축의 이동을 가속도 kGlideAccel·속도 kMaxStep 상한 안에서 끝내는 최소 틱 수
[[nodiscard]] int MinMoveTicks_(
    const float _dist,
    const float _v0)
{
    const float d      = Abs(_dist);
    const float v      = Abs(_v0);
    const float tAccel = (4.f * v + Sqrt(16.f * v * v + 24.f * kGlideAccel * d)) / (2.f * kGlideAccel);
    const float tVel   = 1.5f * d / Max(1.f, kMaxStep - v);
    return static_cast<int>(Ceil(Max(tAccel, tVel)));
}

// 임계감쇠 스프링
void SmoothDamp_(
    float&      _current,
    float&      _velocity,
    const float _target,
    const float _smoothTime,
    const float _dt,
    const float _maxSpeed)
{
    const float omega     = 2.f / _smoothTime;
    const float x         = omega * _dt;
    const float e         = 1.f / (1.f + x + 0.48f * x * x + 0.235f * x * x * x);
    const float maxChange = _maxSpeed * _smoothTime;
    const float change    = Clamp(_current - _target, -maxChange, maxChange);
    const float goal      = _current - change;
    const float temp      = (_velocity + omega * change) * _dt;
    _velocity             = (_velocity - omega * temp) * e;
    float out             = goal + (change + temp) * e;

    if ((_target - _current > 0.f) == (out > _target))
    {
        out       = _target;
        _velocity = 0.f;
    }

    if (Abs(out - _target) < 0.001f && Abs(_velocity) < 0.01f)
    {
        out       = _target;
        _velocity = 0.f;
    }
    _current = out;
}

[[nodiscard]] float ApplyDeadzone_(
    const float _center,
    const float _focus,
    const float _half)
{
    if (_focus > _center + _half)
    {
        return _focus - _half;
    }
    if (_focus < _center - _half)
    {
        return _focus + _half;
    }
    return _center;
}
}   // namespace

Camera::Camera(
    const CameraTuning& _tuning)
    : m_tuning(_tuning)
{
    m_center = { _tuning.viewSize.x * 0.5f, _tuning.viewSize.y * 0.5f };
    m_target = m_center;
}

void Camera::SetTuning(
    const CameraTuning& _tuning)
{
    m_tuning = _tuning;
}

void Camera::SetBounds(
    const Aabb& _worldBounds)
{
    m_bounds     = _worldBounds;
    m_bHasBounds = true;
    if (!m_bHasEff && !m_bLocked)
    {
        m_effBounds = m_bounds;
        m_bHasEff   = true;
    }
}

void Camera::Lock(
    const Aabb& _lockBounds)
{
    m_lockBounds = _lockBounds;
    m_bLocked    = true;
    m_effBounds  = _lockBounds;
    m_effVel     = {};
    m_bHasEff    = true;

    if (m_bMoving)
    {
        m_moveTarget = ClampCenter_(m_moveTarget);
        return;
    }
    const VECTOR2 goal = ClampCenter_(m_center);
    // 움직이던 카메라는 멈추지 않고 감속 이동으로 넘긴다
    const bool bNear  = Abs(goal.x - m_center.x) <= 1.f && Abs(goal.y - m_center.y) <= 1.f;
    const bool bStill = Abs(m_velocity.x) <= 1.f && Abs(m_velocity.y) <= 1.f;
    if (bNear && bStill)
    {
        m_center = goal;
    }
    else
    {
        BeginMove(goal, kCorrectionSec);
    }
}

void Camera::Unlock()
{
    m_bLocked      = false;
    m_bPendingLock = false;
}

void Camera::BeginMove(
    const VECTOR2 _targetCenter,
    const float   _seconds)
{
    const int baseTicks = static_cast<int>(std::llround(static_cast<double>(_seconds) / static_cast<double>(kStepSec)));
    if (baseTicks <= 0)
    {
        CancelMove();
        SnapTo(_targetCenter);
        return;
    }

    // 도착점은 "지금" 유효한 영역이 아니라 가고 있는 최종 영역 기준으로 잡는다
    Aabb          targetRect = {};
    const VECTOR2 goal       = GetTargetRect_(targetRect) ? ClampToRect_(_targetCenter, targetRect) : _targetCenter;
    const VECTOR2 v0         = { m_velocity.x, m_velocity.y * kStepSec };   // 현재 속도
    const int     minTicks   = Max(MinMoveTicks_(goal.x - m_center.x, v0.x), MinMoveTicks_(goal.y - m_center.y, v0.y));

    m_bMoving        = true;
    m_bGlide         = false;
    m_moveStart      = m_center;
    m_moveStartVel   = v0;
    m_stepY          = 0.f;
    m_moveRaw        = _targetCenter;
    m_moveTarget     = goal;
    m_moveTotalTicks = Max(baseTicks, minTicks);
    m_moveElapsed    = 0;
    m_velocity       = { 0.f, 0.f };
    m_pushSpeed      = 0.f;
}

void Camera::CancelMove()
{
    m_bMoving = false;
}

void Camera::LockWithGlide(
    const Aabb& _lockBounds,
    const float _glideSeconds)
{
    if (_glideSeconds <= 0.f)
    {
        m_bPendingLock = false;
        Lock(_lockBounds);
        return;
    }
    m_pendingLock  = _lockBounds;
    m_bPendingLock = true;
    BeginMove(_lockBounds.GetCenter(), _glideSeconds);
}

Aabb Camera::GetActiveBounds_(
    bool& _bHasBounds) const
{
    _bHasBounds = m_bHasEff;
    return m_effBounds;
}

bool Camera::GetTargetRect_(
    Aabb& _rect) const
{
    if (m_bLocked)
    {
        _rect = m_lockBounds;
        return true;
    }
    if (!m_bHasBounds)
    {
        return false;
    }
    _rect = m_bounds;
    if (m_bPendingLock)
    {
        // 도착점이 경계에 막혀 멈췄다가 잠금 시점에 화면이 한 번에 튀지
        _rect.min.x = Min(_rect.min.x, m_pendingLock.min.x);
        _rect.min.y = Min(_rect.min.y, m_pendingLock.min.y);
        _rect.max.x = Max(_rect.max.x, m_pendingLock.max.x);
        _rect.max.y = Max(_rect.max.y, m_pendingLock.max.y);
    }
    return true;
}

void Camera::SyncEffectiveBounds_()
{
    m_bHasEff = GetTargetRect_(m_effBounds);
    m_effVel  = {};
}

void Camera::ApproachEdge_(
    float&      _edge,
    float&      _vel,
    const float _target,
    const float _sign)
{
    const float dist = _target - _edge;
    if (dist * _sign <= 0.f)
    {
        _edge = _target;
        _vel  = 0.f;
        return;
    }

    const float dir     = dist < 0.f ? -1.f : 1.f;
    const float vTarget = dir * Min(kBoundsEase, BrakeSpeed_(Abs(dist)));
    _vel                = Clamp(vTarget, _vel - kGlideAccel, _vel + kGlideAccel);
    const float move    = Clamp(_vel, -Abs(dist), Abs(dist));
    _edge += move;
    _vel = move;
}

void Camera::UpdateEffectiveBounds_()
{
    Aabb target = {};
    if (!GetTargetRect_(target))
    {
        m_bHasEff = false;
        return;
    }
    if (!m_bHasEff)
    {
        m_effBounds = target;
        m_effVel    = {};
        m_bHasEff   = true;
        return;
    }

    ApproachEdge_(m_effBounds.min.x, m_effVel.min.x, target.min.x, 1.f);
    ApproachEdge_(m_effBounds.min.y, m_effVel.min.y, target.min.y, 1.f);
    ApproachEdge_(m_effBounds.max.x, m_effVel.max.x, target.max.x, -1.f);
    ApproachEdge_(m_effBounds.max.y, m_effVel.max.y, target.max.y, -1.f);
}

VECTOR2 Camera::ClampToRect_(
    const VECTOR2 _center,
    const Aabb&   _bounds) const
{
    const VECTOR2 half   = { m_tuning.viewSize.x * 0.5f, m_tuning.viewSize.y * 0.5f };
    VECTOR2       result = _center;

    const float minX = _bounds.min.x + half.x;
    const float maxX = _bounds.max.x - half.x;
    result.x         = minX > maxX ? (_bounds.min.x + _bounds.max.x) * 0.5f : Clamp(result.x, minX, maxX);

    const float minY = _bounds.min.y + half.y;
    const float maxY = _bounds.max.y - half.y;
    result.y         = minY > maxY ? (_bounds.min.y + _bounds.max.y) * 0.5f : Clamp(result.y, minY, maxY);
    return result;
}

VECTOR2 Camera::ClampCenter_(
    const VECTOR2 _center) const
{
    bool       bHasBounds = false;
    const Aabb bounds     = GetActiveBounds_(bHasBounds);
    if (!bHasBounds)
    {
        return _center;
    }
    return ClampToRect_(_center, bounds);
}
VECTOR2 Camera::SnapTopLeft_(
    const VECTOR2 _center) const
{
    VECTOR2 topLeft = { RoundToInt_(_center.x - m_tuning.viewSize.x * 0.5f), RoundToInt_(_center.y - m_tuning.viewSize.y * 0.5f) };

    bool       bHasBounds = false;
    const Aabb bounds     = GetActiveBounds_(bHasBounds);
    if (bHasBounds)
    {
        const float minX = std::ceil(bounds.min.x);
        const float maxX = std::floor(bounds.max.x - m_tuning.viewSize.x);
        const float minY = std::ceil(bounds.min.y);
        const float maxY = std::floor(bounds.max.y - m_tuning.viewSize.y);
        topLeft.x        = minX > maxX ? RoundToInt_((bounds.min.x + bounds.max.x - m_tuning.viewSize.x) * 0.5f) : Clamp(topLeft.x, minX, maxX);
        topLeft.y        = minY > maxY ? RoundToInt_((bounds.min.y + bounds.max.y - m_tuning.viewSize.y) * 0.5f) : Clamp(topLeft.y, minY, maxY);
    }
    return topLeft;
}

void Camera::AddScrollFloor(
    const float _dx)
{
    if (!m_bScrollFloor)
    {
        m_scrollFloorX = m_center.x;
        m_bScrollFloor = true;
    }
    m_scrollFloorX += _dx;
}

void Camera::ShiftLock(
    const float _dx)
{
    if (m_bPendingLock)
    {
        m_pendingLock.min.x += _dx;
        m_pendingLock.max.x += _dx;
    }
    if (m_bMoving)
    {
        m_moveTarget.x += _dx;
        m_moveRaw.x += _dx;
    }
    if (!m_bLocked)
    {
        return;
    }
    m_lockBounds.min.x += _dx;
    m_lockBounds.max.x += _dx;
    m_effBounds.min.x += _dx;
    m_effBounds.max.x += _dx;
}

void Camera::SnapTo(
    const VECTOR2 _viewCenter)
{
    m_bMoving      = false;
    m_bScrollFloor = false;
    m_bPendingLock = false;
    m_bGlide       = false;
    SyncEffectiveBounds_();
    m_center    = ClampCenter_(_viewCenter);
    m_stepY     = 0.f;
    m_velocity  = { 0.f, 0.f };
    m_pushSpeed = 0.f;
}

void Camera::SnapToTarget()
{
    SnapTo({ m_target.x + m_tuning.targetOffset.x, m_target.y + m_tuning.targetOffset.y });
}

void Camera::StepX_(
    const float _desiredX,
    const float _focusDeltaX)
{
    const float gap = _desiredX - m_center.x;
    if (!m_bGlide && Abs(gap) > kGlideEnterGap)
    {
        m_bGlide = true;
    }

    if (m_bGlide)
    {
        // 속도는 틱당 kGlideAccel 이상 바뀌지
        const float dir     = gap < 0.f ? -1.f : 1.f;
        const float vTarget = dir * Min(kMaxStep, BrakeSpeed_(Abs(gap)));
        const float vNext   = Clamp(vTarget, m_velocity.x - kGlideAccel, m_velocity.x + kGlideAccel);
        const float move    = Clamp(vNext, -Abs(gap), Abs(gap));
        m_center.x += move;
        m_velocity.x = move;

        if (Abs(_desiredX - m_center.x) < kGlideExitGap)
        {
            m_center.x  = _desiredX;
            m_bGlide    = false;
            m_pushSpeed = 0.f;
        }
        return;
    }

    const float dir = gap < 0.f ? -1.f : 1.f;
    if (dir != m_lastDirX)
    {
        m_pushSpeed = 0.f;
        m_lastDirX  = dir;
    }

    // 플레이어가 추적 방향으로 움직인 양만큼 가속한다
    // 멈추거나 되돌아가면 틱마다
    const float moveX = _focusDeltaX * dir;
    if (moveX > m_pushSpeed)
    {
        const float accel = m_pushSpeed <= 0.f ? kRampStart : m_pushSpeed + kRampAccel;
        m_pushSpeed       = Min(accel, moveX);
    }
    else
    {
        m_pushSpeed = Max(moveX, m_pushSpeed * kDecayPerTick);
    }
    if (m_pushSpeed < kStopSpeed)
    {
        m_pushSpeed = 0.f;
    }

    // 한 틱에 직전 속도의 1.5배를 넘어 빨라지지 않는다
    const float limit = Max(kStepFloor, kStepGrowth * Abs(m_velocity.x));
    const float stepAbs = Min(Min(Abs(gap), m_pushSpeed), Min(limit, BrakeSpeed_(Abs(gap))));
    m_velocity.x        = dir * stepAbs;
    m_center.x += m_velocity.x;
}

void Camera::Update(
    const float _dt)
{
    UpdateEffectiveBounds_();

    if (m_bMoving)
    {
        // 도착점은 이동 중에도 최종 영역 안으로 유지
        Aabb targetRect = {};
        if (GetTargetRect_(targetRect))
        {
            const VECTOR2 goalNow = ClampToRect_(m_moveRaw, targetRect);
            m_moveTarget.x += Clamp(goalNow.x - m_moveTarget.x, -kBoundsEase, kBoundsEase);
            m_moveTarget.y += Clamp(goalNow.y - m_moveTarget.y, -kBoundsEase, kBoundsEase);
        }

        ++m_moveElapsed;
        if (m_moveElapsed >= m_moveTotalTicks)
        {
            m_center  = m_moveTarget;
            m_bMoving = false;
            if (m_bPendingLock)
            {
                m_bPendingLock = false;
                Lock(m_pendingLock);
            }
        }
        else
        {
            const float T   = static_cast<float>(m_moveTotalTicks);
            const float s   = static_cast<float>(m_moveElapsed) / T;
            const float s2  = s * s;
            const float s3  = s2 * s;
            const float h00 = 2.f * s3 - 3.f * s2 + 1.f;
            const float h10 = (s3 - 2.f * s2 + s) * T;
            const float h01 = -2.f * s3 + 3.f * s2;
            m_center        = {
                h00 * m_moveStart.x + h10 * m_moveStartVel.x + h01 * m_moveTarget.x,
                h00 * m_moveStart.y + h10 * m_moveStartVel.y + h01 * m_moveTarget.y
            };
        }
        m_velocity  = { 0.f, 0.f };
        m_pushSpeed = 0.f;
    }
    else
    {
        const VECTOR2 before      = m_center;
        const VECTOR2 focus       = { m_target.x + m_tuning.targetOffset.x, m_target.y + m_tuning.targetOffset.y };
        const float   focusDeltaX = m_bHasPrevFocus ? focus.x - m_prevFocusX : 0.f;
        m_prevFocusX              = focus.x;
        m_bHasPrevFocus           = true;

        VECTOR2 desired = {
            ApplyDeadzone_(m_center.x, focus.x, m_tuning.deadzoneHalf.x),
            ApplyDeadzone_(m_center.y, focus.y, m_tuning.deadzoneHalf.y)
        };

        if (m_bForwardOnly)
        {
            desired.x = Max(desired.x, m_center.x);
        }
        if (m_bForwardOnlyY)
        {
            desired.y = Max(desired.y, m_center.y);
        }
        if (m_bScrollFloor)
        {
            desired.x = Max(desired.x, m_scrollFloorX);
        }
        desired = ClampCenter_(desired);

        if (m_tuning.smoothTime <= 0.f)
        {
            m_center    = desired;
            m_velocity  = { 0.f, 0.f };
            m_pushSpeed = 0.f;
            m_bGlide    = false;
        }
        else
        {
            StepX_(desired.x, focusDeltaX);
            const float yBefore = m_center.y;
            SmoothDamp_(m_center.y, m_velocity.y, desired.y, m_tuning.smoothTime, _dt, kMaxSpeedSec);

            // 세로 걸음도 틱당 변화가 kGlideAccel 이하
            const float gapY   = desired.y - yBefore;
            float       stepY  = m_center.y - yBefore;
            stepY              = Clamp(stepY, m_stepY - kGlideAccel, m_stepY + kGlideAccel);
            const float reachY = Min(Abs(gapY), BrakeSpeed_(Abs(gapY)));
            stepY              = Clamp(stepY, -reachY, reachY);
            m_center.y         = yBefore + stepY;
            m_stepY            = stepY;
        }
        m_center = ClampCenter_(m_center);
        if (m_bScrollFloor && m_center.x < m_scrollFloorX)
        {
            m_center.x   = m_scrollFloorX;
            m_center     = ClampCenter_(m_center);
            m_velocity.x = 0.f;
            m_pushSpeed  = 0.f;
        }

        if (m_bForwardOnly && !m_bLocked && m_center.x < before.x)
        {
            m_center.x = before.x;
        }
    }
}

VECTOR2 Camera::GetPosition() const
{
    return SnapTopLeft_(m_center);
}

Aabb Camera::GetViewRect() const
{
    return Aabb::FromMinSize(GetPosition(), m_tuning.viewSize);
}

const CameraTuning& Camera::GetTuning() const
{
    return m_tuning;
}

bool Camera::IsLocked() const
{
    return m_bLocked;
}

void Camera::SetForwardOnly(
    const bool _bForwardOnly)
{
    m_bForwardOnly = _bForwardOnly;
}

bool Camera::IsForwardOnly() const
{
    return m_bForwardOnly;
}

void Camera::SetForwardOnlyY(
    const bool _bForwardOnlyY)
{
    m_bForwardOnlyY = _bForwardOnlyY;
}

bool Camera::IsForwardOnlyY() const
{
    return m_bForwardOnlyY;
}

bool Camera::IsMoving() const
{
    return m_bMoving;
}

bool Camera::HasPendingLock() const
{
    return m_bPendingLock;
}

const Aabb& Camera::GetBounds() const
{
    return m_bounds;
}

const Aabb& Camera::GetLockBounds() const
{
    return m_lockBounds;
}

void Camera::SetTarget(
    const VECTOR2 _target)
{
    m_target = _target;
}

void Camera::ClearScrollFloor()
{
    m_bScrollFloor = false;
}

bool Camera::HasScrollFloor() const
{
    return m_bScrollFloor;
}

float Camera::GetScrollFloorX() const
{
    return m_scrollFloorX;
}

VECTOR2 Camera::GetViewCenter() const
{
    return m_center;
}

CameraTuning MakeCameraTuning(
    const CameraSettings& _settings)
{
    CameraTuning tuning = {};
    tuning.viewSize     = { static_cast<float>(Renderer::kLogicalWidth), static_cast<float>(Renderer::kLogicalHeight) };
    tuning.deadzoneHalf = _settings.deadzoneHalf;
    tuning.smoothTime   = _settings.smoothTime;
    return tuning;
}
