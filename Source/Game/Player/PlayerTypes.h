#pragma once
#include <cstdint>

namespace slug
{

enum class ePlayerState : uint8_t
{
    Idle,
    Run,
    Jump,
    Crouch,
    CrouchMove,
    Dead,
    Respawn,
    Victory,
    Count
};

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
    Down,
    DiagDown
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

inline constexpr uint32_t kPlayerOwnerId = 1;

}   // namespace slug
