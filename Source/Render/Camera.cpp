#include "Render/Camera.h"

#include <cmath>

#include "Base/Math.h"

namespace slug
{

namespace
{
    [[nodiscard]] float RoundToInt_(
        const float _value)
    {
        return std::floor(_value + 0.5f);
    }

    void SmoothDamp_(
        float&      _current,
        float&      _velocity,
        const float _target,
        const float _smoothTime,
        const float _dt)
    {
        const float omega = 2.f / _smoothTime;
        const float x     = omega * _dt;
        const float e     = 1.f / (1.f + x + 0.48f * x * x + 0.235f * x * x * x);

        const float change = _current - _target;
        const float temp   = (_velocity + omega * change) * _dt;
        _velocity          = (_velocity - omega * temp) * e;
        float out          = _target + (change + temp) * e;

        if ((_target - _current > 0.f) == (out > _target))
        {
            out       = _target;
            _velocity = 0.f;
        }

        if (std::abs(out - _target) < 0.001f && std::abs(_velocity) < 0.01f)
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
}

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
}

void Camera::Lock(
    const Aabb& _lockBounds)
{
    m_lockBounds = _lockBounds;
    m_bLocked    = true;
}

void Camera::Unlock()
{
    m_bLocked      = false;
    m_bPendingLock = false;
}

void Camera::BeginMove(
    const Vec2 _targetCenter,
    const float        _seconds)
{
    const int ticks = static_cast<int>(std::llround(static_cast<double>(_seconds) / static_cast<double>(kStepSec)));
    if (ticks <= 0)
    {
        CancelMove();
        SnapTo(_targetCenter);
        return;
    }
    m_bMoving        = true;
    m_moveStart      = m_center;
    m_moveTarget     = _targetCenter;
    m_moveTotalTicks = ticks;
    m_moveElapsed    = 0;
    m_velocity       = { 0.f, 0.f };
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
    if (m_bLocked)
    {
        _bHasBounds = true;
        return m_lockBounds;
    }
    _bHasBounds = m_bHasBounds;
    return m_bounds;
}

Vec2 Camera::ClampCenter_(
    const Vec2 _center) const
{
    bool       bHasBounds = false;
    const Aabb bounds     = GetActiveBounds_(bHasBounds);
    if (!bHasBounds)
    {
        return _center;
    }

    const Vec2 half   = { m_tuning.viewSize.x * 0.5f, m_tuning.viewSize.y * 0.5f };
    Vec2       result = _center;

    const float minX = bounds.min.x + half.x;
    const float maxX = bounds.max.x - half.x;
    result.x         = minX > maxX ? (bounds.min.x + bounds.max.x) * 0.5f : slug::Clamp(result.x, minX, maxX);

    const float minY = bounds.min.y + half.y;
    const float maxY = bounds.max.y - half.y;
    result.y         = minY > maxY ? (bounds.min.y + bounds.max.y) * 0.5f : slug::Clamp(result.y, minY, maxY);
    return result;
}

Vec2 Camera::SnapTopLeft_(
    const Vec2 _center) const
{
    Vec2 topLeft = { RoundToInt_(_center.x - m_tuning.viewSize.x * 0.5f), RoundToInt_(_center.y - m_tuning.viewSize.y * 0.5f) };

    bool       bHasBounds = false;
    const Aabb bounds     = GetActiveBounds_(bHasBounds);
    if (bHasBounds)
    {
        const float minX = std::ceil(bounds.min.x);
        const float maxX = std::floor(bounds.max.x - m_tuning.viewSize.x);
        const float minY = std::ceil(bounds.min.y);
        const float maxY = std::floor(bounds.max.y - m_tuning.viewSize.y);
        topLeft.x        = minX > maxX ? RoundToInt_((bounds.min.x + bounds.max.x - m_tuning.viewSize.x) * 0.5f) : slug::Clamp(topLeft.x, minX, maxX);
        topLeft.y        = minY > maxY ? RoundToInt_((bounds.min.y + bounds.max.y - m_tuning.viewSize.y) * 0.5f) : slug::Clamp(topLeft.y, minY, maxY);
    }
    return topLeft;
}

void Camera::SnapTo(
    const Vec2 _viewCenter)
{
    m_center   = ClampCenter_(_viewCenter);
    m_velocity = { 0.f, 0.f };
}

void Camera::SnapToTarget()
{
    SnapTo({ m_target.x + m_tuning.targetOffset.x, m_target.y + m_tuning.targetOffset.y });
}

void Camera::Update(
    const float _dt)
{
    if (m_bMoving)
    {
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
            const float t = static_cast<float>(m_moveElapsed) / static_cast<float>(m_moveTotalTicks);
            m_center      = { m_moveStart.x + (m_moveTarget.x - m_moveStart.x) * t, m_moveStart.y + (m_moveTarget.y - m_moveStart.y) * t };
        }
        m_velocity = { 0.f, 0.f };
    }
    else
    {
        const Vec2 focus = { m_target.x + m_tuning.targetOffset.x, m_target.y + m_tuning.targetOffset.y };

        Vec2 desired = {
            ApplyDeadzone_(m_center.x, focus.x, m_tuning.deadzoneHalf.x),
            ApplyDeadzone_(m_center.y, focus.y, m_tuning.deadzoneHalf.y)
        };

        if (m_bForwardOnly)
        {
            desired.x = slug::Max(desired.x, m_center.x);
        }
        if (m_bForwardOnlyY)
        {
            desired.y = slug::Max(desired.y, m_center.y);
        }
        desired = ClampCenter_(desired);

        if (m_tuning.smoothTime <= 0.f)
        {
            m_center   = desired;
            m_velocity = { 0.f, 0.f };
        }
        else
        {
            // 영상 분석: 스크롤은 2→7(영상 px/프레임) 으로 약 0.3 s 에 걸쳐 오르고, 플레이어가 멈추면 즉시 멈춘다
            constexpr float kRampStart   = 1.2f;    // px/틱
            constexpr float kRampAccel   = 0.19f;   // px/틱²
            constexpr float kSnapGap     = 120.f;
            const float     gap          = desired.x - m_center.x;
            const float     playerStep   = m_bHasPrevFocus ? std::abs(focus.x - m_prevFocusX) : 0.f;
            m_prevFocusX                 = focus.x;
            m_bHasPrevFocus              = true;
            if (std::abs(gap) < 0.001f)
            {
                m_pushSpeed = 0.f;
            }
            else if (std::abs(gap) > kSnapGap)
            {
                m_center.x  = desired.x;
                m_pushSpeed = 0.f;
            }
            else
            {
                m_pushSpeed = m_pushSpeed <= 0.f ? kRampStart : m_pushSpeed + kRampAccel;
                const float step = slug::Min(std::abs(gap), slug::Min(m_pushSpeed, playerStep));
                m_center.x += gap > 0.f ? step : -step;
            }
            m_velocity.x = 0.f;
            SmoothDamp_(m_center.y, m_velocity.y, desired.y, m_tuning.smoothTime, _dt);
        }
    }

    m_center = ClampCenter_(m_center);

    if (m_shakeTime < m_shakeDuration)
    {
        m_shakeTime += _dt;
        const float k = slug::Max(0.f, 1.f - m_shakeTime / m_shakeDuration);
        const float a = m_shakeAmplitude * k;

        auto NextUnit = [this]() {
            m_shakeSeed ^= m_shakeSeed << 13;
            m_shakeSeed ^= m_shakeSeed >> 17;
            m_shakeSeed ^= m_shakeSeed << 5;
            return static_cast<float>(m_shakeSeed & 0xFFFFu) / 32767.5f - 1.f;   // [-1, 1]
        };
        m_shakeOffset = { RoundToInt_(NextUnit() * a), RoundToInt_(NextUnit() * a) };
    }
    else
    {
        m_shakeOffset = { 0.f, 0.f };
    }
}

void Camera::Shake(
    const float _amplitude,
    const float _durationSec)
{
    if (_amplitude <= 0.f || _durationSec <= 0.f)
    {
        StopShake();
        return;
    }
    m_shakeAmplitude = _amplitude;
    m_shakeDuration  = _durationSec;
    m_shakeTime      = 0.f;
}

void Camera::StopShake()
{
    m_shakeAmplitude = 0.f;
    m_shakeDuration  = 0.f;
    m_shakeTime      = 0.f;
    m_shakeOffset    = { 0.f, 0.f };
}

Vec2 Camera::GetPositionNoShake() const
{
    return SnapTopLeft_(m_center);
}

Vec2 Camera::GetPosition() const
{
    Vec2 position = SnapTopLeft_(m_center);
    if (m_shakeOffset.x == 0.f && m_shakeOffset.y == 0.f)
    {
        return position;
    }

    position += m_shakeOffset;
    bool       bHasBounds = false;
    const Aabb bounds     = GetActiveBounds_(bHasBounds);
    if (bHasBounds)
    {
        const float minX = std::ceil(bounds.min.x);
        const float maxX = std::floor(bounds.max.x - m_tuning.viewSize.x);
        const float minY = std::ceil(bounds.min.y);
        const float maxY = std::floor(bounds.max.y - m_tuning.viewSize.y);
        if (minX <= maxX)
        {
            position.x = slug::Clamp(position.x, minX, maxX);
        }
        if (minY <= maxY)
        {
            position.y = slug::Clamp(position.y, minY, maxY);
        }
    }
    return position;
}

Aabb Camera::GetViewRect() const
{
    return Aabb::FromMinSize(GetPosition(), m_tuning.viewSize);
}

Vec2 Camera::WorldToScreen(
    const Vec2 _world) const
{
    const Vec2 position = GetPosition();
    return { _world.x - position.x, _world.y - position.y };
}

Vec2 Camera::ScreenToWorld(
    const Vec2 _screen) const
{
    const Vec2 position = GetPosition();
    return { _screen.x + position.x, _screen.y + position.y };
}

bool Camera::IsVisible(
    const Aabb& _worldBox,
    const float _margin) const
{
    return GetViewRect().Expanded(_margin).Overlaps(_worldBox);
}
}
