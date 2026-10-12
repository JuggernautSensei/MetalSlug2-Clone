#pragma once
#include <cstdint>

#include "Base/Math.h"
#include "World/Constants.h"
#include "World/Geometry.h"

struct CameraTuning
{
    Vec2  viewSize     = { 640.f, 448.f };   // 논리 해상도
    Vec2  deadzoneHalf = { 24.f, 16.f };     // 시점 중심 기준, 목표가 이 안에 있으면 카메라가 움직이지 않는다
    float smoothTime   = 0.18f;
    Vec2  targetOffset = { 0.f, 0.f };
};

class Camera
{
public:
    Camera() = default;
    explicit Camera(const CameraTuning& _tuning);

    void                              SetTuning(const CameraTuning& _tuning);
    [[nodiscard]] const CameraTuning& GetTuning() const;

    void               SetBounds(const Aabb& _worldBounds);
    void               Lock(const Aabb& _lockBounds);
    void               Unlock();
    [[nodiscard]] bool IsLocked() const;

    void               SetForwardOnly(const bool _bForwardOnly);
    [[nodiscard]] bool IsForwardOnly() const;

    void               SetForwardOnlyY(const bool _bForwardOnlyY);
    [[nodiscard]] bool IsForwardOnlyY() const;

    void               BeginMove(Vec2 _targetCenter, float _seconds);
    void               CancelMove();
    [[nodiscard]] bool IsMoving() const;

    void               LockWithGlide(const Aabb& _lockBounds, float _glideSeconds);
    [[nodiscard]] bool HasPendingLock() const;

    [[nodiscard]] const Aabb& GetBounds() const;
    [[nodiscard]] const Aabb& GetLockBounds() const;

    void SetTarget(const Vec2 _target);

    // 자동 전진 (보스전 스크롤): 카메라 중심 x 의 하한을 _dx 만큼 올린다. 하한보다 왼쪽으로는 가지 않고 (매 틱 강제),
    // 잠금/경계 안에서만 움직인다. 경계 오른쪽 끝에 닿으면 멈춘다.
    void AddScrollFloor(float _dx);

    // 잠금 영역을 가로로 _dx 만큼 옮긴다 (보스전 자동 스크롤: 잠금 사각형이 카메라와 함께 오른쪽으로 간다). 잠금 중이 아니면 무시.
    // 활강 중이면 목표/대기 중인 잠금도 같이 옮긴다.
    void                ShiftLock(float _dx);
    void                ClearScrollFloor();
    [[nodiscard]] bool  HasScrollFloor() const;
    [[nodiscard]] float GetScrollFloorX() const;

    void SnapToTarget();
    void SnapTo(Vec2 _viewCenter);

    void Update(float _dt = kStepSec);

    void Shake(float _amplitude, float _durationSec);
    void StopShake();

    [[nodiscard]] Vec2 GetPosition() const;

    [[nodiscard]] Vec2 GetPositionNoShake() const;

    [[nodiscard]] Vec2 GetViewCenter() const;
    [[nodiscard]] Aabb GetViewRect() const;

    [[nodiscard]] Vec2 WorldToScreen(Vec2 _world) const;
    [[nodiscard]] Vec2 ScreenToWorld(Vec2 _screen) const;
    [[nodiscard]] bool IsVisible(const Aabb& _worldBox, float _margin = 0.f) const;

private:
    [[nodiscard]] Aabb  GetActiveBounds_(bool& _bHasBounds) const;
    [[nodiscard]] Vec2  ClampCenter_(Vec2 _center) const;
    [[nodiscard]] Vec2  ClampToRect_(Vec2 _center, const Aabb& _bounds) const;
    [[nodiscard]] Vec2  SnapTopLeft_(Vec2 _center) const;
    [[nodiscard]] bool  GetTargetRect_(Aabb& _rect) const;
    void                SyncEffectiveBounds_();
    void                UpdateEffectiveBounds_();
    void                ApproachEdge_(float& _edge, float& _vel, float _target, float _sign);
    void                StepX_(float _desiredX, float _focusDeltaX);
    [[nodiscard]] float NextShakeUnit_();   // xorshift 로 [-1, 1] 난수를 만든다

    CameraTuning m_tuning = {};

    Vec2  m_center        = { 320.f, 224.f };   // 시점 중심 (실수 상태)
    Vec2  m_velocity      = { 0.f, 0.f };
    float m_prevFocusX    = 0.f;
    float m_pushSpeed     = 0.f;
    bool  m_bHasPrevFocus = false;
    Vec2  m_target        = { 320.f, 224.f };
    Aabb  m_effBounds     = {};   // 클램프에 실제로 쓰는 영역 (SetBounds/Unlock 은 점진 변화, Lock/SnapTo 는 즉시)
    Aabb  m_effVel        = {};   // 경계 변별 속도 (px/틱), 제동·가속 상한 적용용
    bool  m_bHasEff       = false;
    bool  m_bGlide        = false;   // 큰 간격을 속도 상한이 있는 감쇠 추적으로 따라잡는 중
    float m_lastDirX      = 1.f;
    float m_stepY         = 0.f;   // 직전 틱 세로 걸음 (px/틱)

    Aabb m_bounds        = {};
    bool m_bHasBounds    = false;
    Aabb m_lockBounds    = {};
    bool m_bLocked       = false;
    bool m_bForwardOnly  = false;
    bool m_bForwardOnlyY = false;

    bool m_bMoving        = false;
    Vec2 m_moveStart      = { 0.f, 0.f };
    Vec2 m_moveStartVel   = { 0.f, 0.f };   // 이동 시작 시점의 속도 (px/틱): 달리던 카메라가 멈칫하지 않도록 이어받는다
    Vec2 m_moveTarget     = { 0.f, 0.f };   // 이동 도착점 (매 틱 최종 영역으로 다시 클램프)
    Vec2 m_moveRaw        = { 0.f, 0.f };   // 요청된 원래 도착점
    int  m_moveTotalTicks = 0;
    int  m_moveElapsed    = 0;
    bool m_bPendingLock   = false;
    Aabb m_pendingLock    = {};

    float    m_shakeAmplitude = 0.f;
    float    m_shakeDuration  = 0.f;
    float    m_shakeTime      = 0.f;
    uint32_t m_shakeSeed      = 0x9E3779B9u;
    Vec2     m_shakeOffset    = { 0.f, 0.f };

    bool  m_bScrollFloor = false;
    float m_scrollFloorX = 0.f;   // 카메라 중심 x 하한 (AddScrollFloor 로 누적)
};
