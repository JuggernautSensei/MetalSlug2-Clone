#pragma once
#include <cstdint>

namespace slug
{

// ===========================================
//  플레이어 공용 열거형
// ===========================================

enum class ePlayerState : uint8_t
{
    Idle,
    Run,
    Jump,   // 상승/하강 모두 (하강 여부는 속도로 구분)
    Crouch,
    CrouchMove,
    Dead,
    Respawn,   // 리스폰 낙하 (무적)
    Victory,
    Count
};

// 겹침 동작: 상체에 덮어 재생되며 이동/점프 상태를 막지 않는다.
enum class eOverlay : uint8_t
{
    None,
    Shot,
    Slash,
    Throw
};

enum class eAim : uint8_t
{
    Forward,
    Up,
    DiagUp,
    Down,      // 공중에서만
    DiagDown   // 공중에서만
};

enum class ePose : uint8_t
{
    Stand,
    Crouch,
    Air
};

[[nodiscard]] constexpr const char* GetStateName(
    const ePlayerState _state)
{
    switch (_state)
    {
        case ePlayerState::Idle: return "Idle";
        case ePlayerState::Run: return "Run";
        case ePlayerState::Jump: return "Jump";
        case ePlayerState::Crouch: return "Crouch";
        case ePlayerState::CrouchMove: return "CrouchMove";
        case ePlayerState::Dead: return "Dead";
        case ePlayerState::Respawn: return "Respawn";
        case ePlayerState::Victory: return "Victory";
        default: return "?";
    }
}

[[nodiscard]] constexpr const char* GetOverlayName(
    const eOverlay _overlay)
{
    switch (_overlay)
    {
        case eOverlay::Shot: return "Shot";
        case eOverlay::Slash: return "Slash";
        case eOverlay::Throw: return "Throw";
        default: return "-";
    }
}

[[nodiscard]] constexpr const char* GetAimName(
    const eAim _aim)
{
    switch (_aim)
    {
        case eAim::Forward: return "fwd";
        case eAim::Up: return "up";
        case eAim::DiagUp: return "diagUp";
        case eAim::Down: return "down";
        case eAim::DiagDown: return "diagDown";
    }
    return "?";
}

// 소유자 ID 규약 (HitResolver 의 owner). 0 은 kNoOwner.
inline constexpr uint32_t kPlayerOwnerId = 1;

}   // namespace slug
