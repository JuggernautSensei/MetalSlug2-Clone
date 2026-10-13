#pragma once
#include <cstdint>

enum class ePlayerState
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
enum class eGodMode
{
    Off,
    NoDeath,
    Invincible,
    Count
};

enum class eOverlay
{
    None,
    Shot,
    Slash,
    Throw
};

enum class eAim
{
    Forward,
    Up,
    DiagUp,
    Down,
    DiagDown
};

enum class ePose
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
