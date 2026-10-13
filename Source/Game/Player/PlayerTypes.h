#pragma once
#include <cstdint>

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

// 에디터 전용 디버그 무적 모드
enum class eGodMode : uint8_t
{
    Off,
    NoDeath,
    Invincible,
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

[[nodiscard]] const char* GetStateName(ePlayerState _state);

[[nodiscard]] const char* GetOverlayName(eOverlay _overlay);

[[nodiscard]] const char* GetAimName(eAim _aim);

[[nodiscard]] const char* GetGodModeName(eGodMode _mode);

inline constexpr uint32_t kPlayerOwnerId = 1;
