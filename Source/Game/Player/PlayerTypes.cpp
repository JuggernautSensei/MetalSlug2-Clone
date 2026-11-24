#include "Game/Player/PlayerTypes.h"

const char* GetStateName(
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

const char* GetOverlayName(
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

const char* GetGodModeName(
    const eGodMode _mode)
{
    switch (_mode)
    {
        case eGodMode::NoDeath: return "No death";
        case eGodMode::Invincible: return "Invincible";
        default: return "Off";
    }
}

const char* GetAimName(
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
