#pragma once

namespace slug
{

// ===========================================
//  시뮬레이션 상수
//   단위: 위치 px, 속도 px/s, 가속도 px/s². +y 는 아래쪽.
// ===========================================

inline constexpr float kStepSec = 1.f / 60.f;   // 고정 스텝 (FixedStepClock 과 동일한 60Hz)

inline constexpr float kGravity          = 1400.f;
inline constexpr float kTerminalVelocity = 3000.f;
inline constexpr float kRunSpeed         = 250.f;
inline constexpr float kJumpSpeed        = 600.f;

// ===========================================
//  PhysicsTuning
//   물리 수치 모음. 기본값은 위의 상수이며, 테스트/핫리로드에서 덮어쓸 수 있다.
// ===========================================

struct PhysicsTuning
{
    float gravity          = kGravity;            // px/s²
    float terminalVelocity = kTerminalVelocity;   // px/s (낙하 속도 상한)
    float runSpeed         = kRunSpeed;           // px/s (참고용 기본 이동 속도)
    float jumpSpeed        = kJumpSpeed;          // px/s (참고용 기본 점프 초속)

    // 가로로 1px(서브스텝) 움직일 때 올라설 수 있는 최대 높이 (px). 4 이면 최대 4:1 경사까지 오른다.
    float maxStepUp = 4.f;

    // 지면에 붙어 있을 때 한 틱에 내려가며 스냅할 수 있는 최대 높이 (px). 이보다 깊으면 낭떠러지로 보고 낙하한다.
    float maxSnapDown = 4.f;

    // 아래 점프(원웨이 통과) 요청 시 원웨이를 무시하는 틱 수.
    int dropThroughSteps = 8;
};

}   // namespace slug
