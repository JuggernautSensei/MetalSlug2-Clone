#include "Render/Camera.h"

#include <cmath>

#include <JugX/Math.h>

namespace slug
{

namespace
{
    [[nodiscard]] float RoundToInt_(
        const float _value)
    {
        return std::floor(_value + 0.5f);
    }

    // 임계 감쇠 스프링 (Unity SmoothDamp 와 같은 근사식). dt 가 고정이면 결정적이다.
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

        // 목표를 넘어서지 않도록 한다
        if ((_target - _current > 0.f) == (out > _target))
        {
            out       = _target;
            _velocity = 0.f;
        }

        // 아주 가까우면 붙인다 (끝없이 미세하게 움직여 정수 스냅이 깜박이는 것을 방지)
        if (std::abs(out - _target) < 0.001f && std::abs(_velocity) < 0.01f)
        {
            out       = _target;
            _velocity = 0.f;
        }
        _current = out;
    }

    // 데드존 규칙: 목표가 데드존 안이면 현재 중심 유지, 밖이면 목표가 데드존 경계에 걸리는 위치로 이동
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
}

void Camera::ClearBounds()
{
    m_bHasBounds = false;
}

void Camera::Lock(
    const Aabb& _lockBounds)
{
    m_lockBounds = _lockBounds;
    m_bLocked    = true;
}

void Camera::Unlock()
{
    m_bLocked = false;
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

jug::VECTOR2 Camera::ClampCenter_(
    const jug::VECTOR2 _center) const
{
    bool       bHasBounds = false;
    const Aabb bounds     = GetActiveBounds_(bHasBounds);
    if (!bHasBounds)
    {
        return _center;
    }

    const jug::VECTOR2 half   = { m_tuning.viewSize.x * 0.5f, m_tuning.viewSize.y * 0.5f };
    jug::VECTOR2       result = _center;

    const float minX = bounds.min.x + half.x;
    const float maxX = bounds.max.x - half.x;
    result.x         = minX > maxX ? (bounds.min.x + bounds.max.x) * 0.5f : jug::Clamp(result.x, minX, maxX);

    const float minY = bounds.min.y + half.y;
    const float maxY = bounds.max.y - half.y;
    result.y         = minY > maxY ? (bounds.min.y + bounds.max.y) * 0.5f : jug::Clamp(result.y, minY, maxY);
    return result;
}

jug::VECTOR2 Camera::SnapTopLeft_(
    const jug::VECTOR2 _center) const
{
    jug::VECTOR2 topLeft = { RoundToInt_(_center.x - m_tuning.viewSize.x * 0.5f), RoundToInt_(_center.y - m_tuning.viewSize.y * 0.5f) };

    bool       bHasBounds = false;
    const Aabb bounds     = GetActiveBounds_(bHasBounds);
    if (bHasBounds)
    {
        // 정수로 스냅한 뒤에도 경계 밖으로 나가지 않도록 안쪽으로 올림/내림한다
        const float minX = std::ceil(bounds.min.x);
        const float maxX = std::floor(bounds.max.x - m_tuning.viewSize.x);
        const float minY = std::ceil(bounds.min.y);
        const float maxY = std::floor(bounds.max.y - m_tuning.viewSize.y);
        topLeft.x        = minX > maxX ? RoundToInt_((bounds.min.x + bounds.max.x - m_tuning.viewSize.x) * 0.5f) : jug::Clamp(topLeft.x, minX, maxX);
        topLeft.y        = minY > maxY ? RoundToInt_((bounds.min.y + bounds.max.y - m_tuning.viewSize.y) * 0.5f) : jug::Clamp(topLeft.y, minY, maxY);
    }
    return topLeft;
}

void Camera::SnapTo(
    const jug::VECTOR2 _viewCenter)
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
    const jug::VECTOR2 focus = { m_target.x + m_tuning.targetOffset.x, m_target.y + m_tuning.targetOffset.y };

    jug::VECTOR2 desired = {
        ApplyDeadzone_(m_center.x, focus.x, m_tuning.deadzoneHalf.x),
        ApplyDeadzone_(m_center.y, focus.y, m_tuning.deadzoneHalf.y)
    };

    if (m_bForwardOnly)
    {
        desired.x = jug::Max(desired.x, m_center.x);
    }
    desired = ClampCenter_(desired);

    if (m_tuning.smoothTime <= 0.f)
    {
        m_center   = desired;
        m_velocity = { 0.f, 0.f };
    }
    else
    {
        SmoothDamp_(m_center.x, m_velocity.x, desired.x, m_tuning.smoothTime, _dt);
        SmoothDamp_(m_center.y, m_velocity.y, desired.y, m_tuning.smoothTime, _dt);
    }

    // 경계/락이 바뀌었을 수 있으므로 항상 클램프 (경계가 전진 전용보다 우선)
    m_center = ClampCenter_(m_center);

    // 흔들림
    if (m_shakeTime < m_shakeDuration)
    {
        m_shakeTime += _dt;
        const float k = jug::Max(0.f, 1.f - m_shakeTime / m_shakeDuration);
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

jug::VECTOR2 Camera::GetPositionNoShake() const
{
    return SnapTopLeft_(m_center);
}

jug::VECTOR2 Camera::GetPosition() const
{
    jug::VECTOR2 position = SnapTopLeft_(m_center);
    if (m_shakeOffset.x == 0.f && m_shakeOffset.y == 0.f)
    {
        return position;
    }

    // 흔들림을 더해도 경계 밖이 보이지 않도록 다시 클램프한다
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
            position.x = jug::Clamp(position.x, minX, maxX);
        }
        if (minY <= maxY)
        {
            position.y = jug::Clamp(position.y, minY, maxY);
        }
    }
    return position;
}

Aabb Camera::GetViewRect() const
{
    return Aabb::FromMinSize(GetPosition(), m_tuning.viewSize);
}

jug::VECTOR2 Camera::WorldToScreen(
    const jug::VECTOR2 _world) const
{
    const jug::VECTOR2 position = GetPosition();
    return { _world.x - position.x, _world.y - position.y };
}

jug::VECTOR2 Camera::ScreenToWorld(
    const jug::VECTOR2 _screen) const
{
    const jug::VECTOR2 position = GetPosition();
    return { _screen.x + position.x, _screen.y + position.y };
}

bool Camera::IsVisible(
    const Aabb& _worldBox,
    const float _margin) const
{
    return GetViewRect().Expanded(_margin).Overlaps(_worldBox);
}

}   // namespace slug
