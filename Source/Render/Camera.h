#pragma once
#include <cstdint>

#include "Base/Math.h"

#include "World/Constants.h"
#include "World/Geometry.h"

namespace slug
{
struct CameraTuning
{
    Vec2 viewSize     = { 640.f, 448.f };   // 논리 해상도
    Vec2 deadzoneHalf = { 24.f, 16.f };     // 시점 중심 기준, 목표가 이 안에 있으면 카메라가 움직이지 않는다
    float        smoothTime   = 0.18f;
    Vec2 targetOffset = { 0.f, 0.f };
};

class Camera
{
public:
    Camera() = default;
    explicit Camera(const CameraTuning& _tuning);

    void                              SetTuning(const CameraTuning& _tuning);
    [[nodiscard]] const CameraTuning& GetTuning() const { return m_tuning; }

    void               SetBounds(const Aabb& _worldBounds);
    void               Lock(const Aabb& _lockBounds);
    void               Unlock();
    [[nodiscard]] bool IsLocked() const { return m_bLocked; }

    void               SetForwardOnly(const bool _bForwardOnly) { m_bForwardOnly = _bForwardOnly; }
    [[nodiscard]] bool IsForwardOnly() const { return m_bForwardOnly; }

    void               SetForwardOnlyY(const bool _bForwardOnlyY) { m_bForwardOnlyY = _bForwardOnlyY; }
    [[nodiscard]] bool IsForwardOnlyY() const { return m_bForwardOnlyY; }

    void               BeginMove(Vec2 _targetCenter, float _seconds);
    void               CancelMove();
    [[nodiscard]] bool IsMoving() const { return m_bMoving; }

    void               LockWithGlide(const Aabb& _lockBounds, float _glideSeconds);
    [[nodiscard]] bool HasPendingLock() const { return m_bPendingLock; }

    [[nodiscard]] const Aabb& GetBounds() const { return m_bounds; }
    [[nodiscard]] const Aabb& GetLockBounds() const { return m_lockBounds; }

    void                       SetTarget(const Vec2 _target) { m_target = _target; }

    void SnapToTarget();
    void SnapTo(Vec2 _viewCenter);

    void Update(float _dt = kStepSec);

    void Shake(float _amplitude, float _durationSec);
    void StopShake();

    [[nodiscard]] Vec2 GetPosition() const;

    [[nodiscard]] Vec2 GetPositionNoShake() const;

    [[nodiscard]] Vec2 GetViewCenter() const { return m_center; }
    [[nodiscard]] Aabb         GetViewRect() const;

    [[nodiscard]] Vec2 WorldToScreen(Vec2 _world) const;
    [[nodiscard]] Vec2 ScreenToWorld(Vec2 _screen) const;
    [[nodiscard]] bool         IsVisible(const Aabb& _worldBox, float _margin = 0.f) const;

private:
    [[nodiscard]] Aabb         GetActiveBounds_(bool& _bHasBounds) const;
    [[nodiscard]] Vec2 ClampCenter_(Vec2 _center) const;
    [[nodiscard]] Vec2 SnapTopLeft_(Vec2 _center) const;

    CameraTuning m_tuning = {};

    Vec2 m_center   = { 320.f, 224.f };   // 시점 중심 (실수 상태)
    Vec2 m_velocity = { 0.f, 0.f };
    Vec2 m_target   = { 320.f, 224.f };

    Aabb m_bounds       = {};
    bool m_bHasBounds   = false;
    Aabb m_lockBounds   = {};
    bool m_bLocked      = false;
    bool m_bForwardOnly = false;
    bool m_bForwardOnlyY = false;

    bool         m_bMoving        = false;
    Vec2 m_moveStart      = { 0.f, 0.f };
    Vec2 m_moveTarget     = { 0.f, 0.f };
    int          m_moveTotalTicks = 0;
    int          m_moveElapsed    = 0;
    bool         m_bPendingLock   = false;
    Aabb         m_pendingLock    = {};

    float        m_shakeAmplitude = 0.f;
    float        m_shakeDuration  = 0.f;
    float        m_shakeTime      = 0.f;
    uint32_t     m_shakeSeed      = 0x9E3779B9u;
    Vec2 m_shakeOffset    = { 0.f, 0.f };
};
}
