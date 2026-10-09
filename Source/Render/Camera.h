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
    [[nodiscard]] Vec2 ClampToRect_(Vec2 _center, const Aabb& _bounds) const;
    [[nodiscard]] Vec2 SnapTopLeft_(Vec2 _center) const;
    [[nodiscard]] bool GetTargetRect_(Aabb& _rect) const;
    void               SyncEffectiveBounds_();
    void               UpdateEffectiveBounds_();
    void               ApproachEdge_(float& _edge, float& _vel, float _target, float _sign);
    void               StepX_(float _desiredX, float _focusDeltaX);

    CameraTuning m_tuning = {};

    Vec2 m_center   = { 320.f, 224.f };   // 시점 중심 (실수 상태)
    Vec2 m_velocity = { 0.f, 0.f };
    float m_prevFocusX    = 0.f;
    float m_pushSpeed     = 0.f;
    bool  m_bHasPrevFocus = false;
    Vec2 m_target   = { 320.f, 224.f };
    Aabb  m_effBounds = {};        // 클램프에 실제로 쓰는 영역 (SetBounds/Unlock 은 점진 변화, Lock/SnapTo 는 즉시)
    Aabb  m_effVel    = {};        // 경계 변별 속도 (px/틱), 제동·가속 상한 적용용
    bool  m_bHasEff   = false;
    bool  m_bGlide    = false;     // 큰 간격을 속도 상한이 있는 감쇠 추적으로 따라잡는 중
    float m_lastDirX  = 1.f;
    float m_stepY     = 0.f;       // 직전 틱 세로 걸음 (px/틱)

    Aabb m_bounds       = {};
    bool m_bHasBounds   = false;
    Aabb m_lockBounds   = {};
    bool m_bLocked      = false;
    bool m_bForwardOnly = false;
    bool m_bForwardOnlyY = false;

    bool         m_bMoving        = false;
    Vec2 m_moveStart      = { 0.f, 0.f };
    Vec2 m_moveStartVel   = { 0.f, 0.f };   // 이동 시작 시점의 속도 (px/틱): 달리던 카메라가 멈칫하지 않도록 이어받는다
    Vec2 m_moveTarget     = { 0.f, 0.f };   // 이동 도착점 (매 틱 최종 영역으로 다시 클램프)
    Vec2 m_moveRaw        = { 0.f, 0.f };   // 요청된 원래 도착점
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
