#pragma once
#include <cstdint>

#include <JugX/Vector2.h>

#include "World/Constants.h"
#include "World/Geometry.h"

namespace slug
{

// ===========================================
//  CameraTuning
// ===========================================

struct CameraTuning
{
    jug::VECTOR2 viewSize     = { 640.f, 448.f };   // 논리 해상도
    jug::VECTOR2 deadzoneHalf = { 24.f, 16.f };     // 시점 중심 기준, 목표가 이 안에 있으면 카메라가 움직이지 않는다
    float        smoothTime   = 0.18f;              // 임계 감쇠 추적의 근사 도달 시간 (초). 0 이하면 즉시 따라간다
    jug::VECTOR2 targetOffset = { 0.f, 0.f };       // 목표점을 시점 중심에서 얼마나 벗어나게 볼지 (예: 진행 방향 앞쪽)
};

// ===========================================
//  Camera
//   월드 → 화면 변환을 정수 좌표로 스냅하여 제공한다 (스프라이트 떨림 방지).
//    - 내부 상태(실수)는 고정 스텝으로 갱신: 데드존 → 임계 감쇠 추적 → 경계 클램프.
//    - GetPosition() 은 흔들림(Shake)을 더한 뒤 정수로 스냅한 시점의 좌상단이다.
//    - 전진 전용(forward-only) 모드에서는 추적으로 왼쪽으로 되돌아가지 않는다 (경계/락 클램프는 예외).
//    - Lock(bounds) 는 화면이 머물 수 있는 월드 사각형을 좁힌다 (보스전, 구간 락). Unlock() 으로 기본 경계 복귀.
// ===========================================

class Camera
{
public:
    Camera() = default;
    explicit Camera(const CameraTuning& _tuning);

    // --- 설정 ---
    void                              SetTuning(const CameraTuning& _tuning);
    [[nodiscard]] const CameraTuning& GetTuning() const { return m_tuning; }

    void               SetBounds(const Aabb& _worldBounds);   // 기본 경계 (레벨 전체)
    void               ClearBounds();
    void               Lock(const Aabb& _lockBounds);   // 기본 경계보다 우선하는 임시 경계
    void               Unlock();
    [[nodiscard]] bool IsLocked() const { return m_bLocked; }

    void               SetForwardOnly(const bool _bForwardOnly) { m_bForwardOnly = _bForwardOnly; }
    [[nodiscard]] bool IsForwardOnly() const { return m_bForwardOnly; }

    // --- 추적 ---
    void                       SetTarget(const jug::VECTOR2 _target) { m_target = _target; }
    [[nodiscard]] jug::VECTOR2 GetTarget() const { return m_target; }

    // 목표를 즉시 중심으로 맞춘다 (속도 초기화, 데드존 무시). 경계 클램프는 적용된다.
    void SnapToTarget();
    void SnapTo(jug::VECTOR2 _viewCenter);

    // 고정 스텝 갱신
    void Update(float _dt = kStepSec);

    // --- 흔들림 ---
    // _amplitude px 로 시작하여 _durationSec 동안 선형으로 줄어드는 결정적 흔들림.
    void Shake(float _amplitude, float _durationSec);
    void StopShake();

    // --- 출력 ---
    // 정수로 스냅된 시점 좌상단 (월드 좌표). 값은 항상 정수이며 경계 안에 있다.
    [[nodiscard]] jug::VECTOR2 GetPosition() const;

    // 흔들림 없는 스냅된 위치 (UI/로직용)
    [[nodiscard]] jug::VECTOR2 GetPositionNoShake() const;

    [[nodiscard]] jug::VECTOR2 GetViewCenter() const { return m_center; }   // 내부 실수 상태
    [[nodiscard]] Aabb         GetViewRect() const;                         // 스냅된 위치 기준 화면 영역 (월드)

    [[nodiscard]] jug::VECTOR2 WorldToScreen(jug::VECTOR2 _world) const;
    [[nodiscard]] jug::VECTOR2 ScreenToWorld(jug::VECTOR2 _screen) const;
    [[nodiscard]] bool         IsVisible(const Aabb& _worldBox, float _margin = 0.f) const;

private:
    [[nodiscard]] Aabb         GetActiveBounds_(bool& _bHasBounds) const;
    [[nodiscard]] jug::VECTOR2 ClampCenter_(jug::VECTOR2 _center) const;
    [[nodiscard]] jug::VECTOR2 SnapTopLeft_(jug::VECTOR2 _center) const;   // 중심 → 스냅·클램프된 좌상단

    CameraTuning m_tuning = {};

    jug::VECTOR2 m_center   = { 320.f, 224.f };   // 시점 중심 (실수 상태)
    jug::VECTOR2 m_velocity = { 0.f, 0.f };
    jug::VECTOR2 m_target   = { 320.f, 224.f };

    Aabb m_bounds       = {};
    bool m_bHasBounds   = false;
    Aabb m_lockBounds   = {};
    bool m_bLocked      = false;
    bool m_bForwardOnly = false;

    // 흔들림
    float        m_shakeAmplitude = 0.f;
    float        m_shakeDuration  = 0.f;
    float        m_shakeTime      = 0.f;
    uint32_t     m_shakeSeed      = 0x9E3779B9u;
    jug::VECTOR2 m_shakeOffset    = { 0.f, 0.f };
};

}   // namespace slug
