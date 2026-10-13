#pragma once
#include <cstdint>

#include "Base/Types.h"

// 32522.png 에서 잘라낸 클립
// 모든 몸통 클립은 오른쪽을
enum class eSlugClip : uint8_t
{
    Idle,
    Drive,
    AccelFwd,
    AccelBack,
    Elev,
    Jump,
    Land,
    Crouch,
    LowIdle,
    LowDrive,
    LowAccelFwd,
    LowAccelBack,
    Enter,
    HitFront,
    HitBack,
    AirHitA,
    AirHitB,
    VulcanIdle,
    Bullet,
    CannonFx,
    CannonFxAir,
    CannonFxCrouch,
    VulcanDebris,
    Explosion,
    ArrowIn,
    ArrowOut,
    Count
};

struct SlugClipInfo
{
    const char* pName  = "";
    int         frames = 0;
    int         fps    = 30;
};

[[nodiscard]] const SlugClipInfo& GetSlugClipInfo(eSlugClip _clip);

inline constexpr int   kVulcanAngleCount = 32;
inline constexpr float kVulcanAngleStep  = 11.25f;

[[nodiscard]] int    VulcanIndexFromAngle(float _angleDeg);
[[nodiscard]] String GetVulcanShotClipName(int _index);

inline constexpr int kShellRowCount = 9;
inline constexpr int kShellColCount = 10;
[[nodiscard]] String GetShellClipName(int _row);

inline constexpr int   kBulletFrameCount = 17;
inline constexpr float kBulletFrameStep  = 5.625f;
