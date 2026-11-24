#pragma once
#include <cstdint>

#include "Core/Tuning.h"
#include "World/Constants.h"
#include "World/Geometry.h"

struct CameraTuning
{
    VECTOR2 viewSize     = { 640.f, 448.f };
    VECTOR2 deadzoneHalf = { 24.f, 16.f };
    float   smoothTime   = 0.18f;
    VECTOR2 targetOffset = { 0.f, 0.f };
};

class Camera
{
public:
    Camera() = default;
    explicit Camera(const CameraTuning& _tuning);
    void                              SetTuning(const CameraTuning& _tuning);
    [[nodiscard]] const CameraTuning& GetTuning() const;
    void                              SetBounds(const Aabb& _worldBounds);
    void                              Lock(const Aabb& _lockBounds);
    void                              Unlock();
    [[nodiscard]] bool                IsLocked() const;
    void                              SetForwardOnly(bool _bForwardOnly);
    [[nodiscard]] bool                IsForwardOnly() const;
    void                              SetForwardOnlyY(bool _bForwardOnlyY);
    [[nodiscard]] bool                IsForwardOnlyY() const;
    void                              BeginMove(VECTOR2 _targetCenter, float _seconds);
    void                              CancelMove();
    [[nodiscard]] bool                IsMoving() const;
    void                              LockWithGlide(const Aabb& _lockBounds, float _glideSeconds);
    [[nodiscard]] bool                HasPendingLock() const;
    [[nodiscard]] const Aabb&         GetBounds() const;
    [[nodiscard]] const Aabb&         GetLockBounds() const;
    void                              SetTarget(VECTOR2 _target);
    void                              AddScrollFloor(float _dx);

    // 잠금 영역을 가로로 _dx 만큼 옮긴다
    void                  ShiftLock(float _dx);
    void                  ClearScrollFloor();
    [[nodiscard]] bool    HasScrollFloor() const;
    [[nodiscard]] float   GetScrollFloorX() const;
    void                  SnapToTarget();
    void                  SnapTo(VECTOR2 _viewCenter);
    void                  Update(float _dt = kStepSec);
    [[nodiscard]] VECTOR2 GetPosition() const;
    [[nodiscard]] VECTOR2 GetViewCenter() const;
    [[nodiscard]] Aabb    GetViewRect() const;

private:
    [[nodiscard]] Aabb    GetActiveBounds_(bool& _bHasBounds) const;
    [[nodiscard]] VECTOR2 ClampCenter_(VECTOR2 _center) const;
    [[nodiscard]] VECTOR2 ClampToRect_(VECTOR2 _center, const Aabb& _bounds) const;
    [[nodiscard]] VECTOR2 SnapTopLeft_(VECTOR2 _center) const;
    [[nodiscard]] bool    GetTargetRect_(Aabb& _rect) const;
    void                  SyncEffectiveBounds_();
    void                  UpdateEffectiveBounds_();
    void                  ApproachEdge_(float& _edge, float& _vel, float _target, float _sign);
    void                  StepX_(float _desiredX, float _focusDeltaX);

    CameraTuning m_tuning = {};

    VECTOR2 m_center        = { 320.f, 224.f };
    VECTOR2 m_velocity      = { 0.f, 0.f };
    float   m_prevFocusX    = 0.f;
    float   m_pushSpeed     = 0.f;
    bool    m_bHasPrevFocus = false;
    VECTOR2 m_target        = { 320.f, 224.f };
    Aabb    m_effBounds     = {};
    Aabb    m_effVel        = {};
    bool    m_bHasEff       = false;
    bool    m_bGlide        = false;
    float   m_lastDirX      = 1.f;
    float   m_stepY         = 0.f;   // 직전 틱 세로 걸음

    Aabb m_bounds        = {};
    bool m_bHasBounds    = false;
    Aabb m_lockBounds    = {};
    bool m_bLocked       = false;
    bool m_bForwardOnly  = false;
    bool m_bForwardOnlyY = false;

    bool    m_bMoving        = false;
    VECTOR2 m_moveStart      = { 0.f, 0.f };
    VECTOR2 m_moveStartVel   = { 0.f, 0.f };
    VECTOR2 m_moveTarget     = { 0.f, 0.f };
    VECTOR2 m_moveRaw        = { 0.f, 0.f };
    int     m_moveTotalTicks = 0;
    int     m_moveElapsed    = 0;
    bool    m_bPendingLock   = false;
    Aabb    m_pendingLock    = {};

    bool  m_bScrollFloor = false;
    float m_scrollFloorX = 0.f;
};

[[nodiscard]] CameraTuning MakeCameraTuning(const CameraSettings& _settings);
