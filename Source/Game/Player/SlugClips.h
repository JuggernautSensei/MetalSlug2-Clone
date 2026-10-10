#pragma once
#include <cstdint>

#include "Base/Types.h"

namespace slug
{

// 32522.png (SV-001 시트) 에서 잘라낸 클립. 이름/프레임 수는 Assets/Source/sheets.json 과 같아야 한다 (SlugTest 가 검증).
// 모든 몸통 클립은 오른쪽을 본다. 왼쪽은 그릴 때 좌우 반전한다.
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

// 발칸 회전 스프라이트: 32 방향. 인덱스 i 의 각도(도, 0 = 오른쪽, 위 = +90) = 135 - 11.25 * i (시계 방향으로 진행).
inline constexpr int   kVulcanAngleCount = 32;
inline constexpr float kVulcanAngleStep  = 11.25f;

[[nodiscard]] int   VulcanIndexFromAngle(float _angleDeg);
[[nodiscard]] float VulcanAngleFromIndex(int _index);
[[nodiscard]] String GetVulcanShotClipName(int _index);

// 곡사포탄 스프라이트: 9 각도 행 (0 = 위쪽 90도 ... 8 = 수평 0도) x 10 열 (회색 -> 백열)
inline constexpr int kShellRowCount = 9;
inline constexpr int kShellColCount = 10;
[[nodiscard]] String GetShellClipName(int _row);

// 발칸탄: 17 프레임, 프레임 k 의 방향 = 오른쪽에서 아래로 5.625 * k 도 (k = 16 이면 수직 아래)
inline constexpr int   kBulletFrameCount = 17;
inline constexpr float kBulletFrameStep  = 5.625f;

}   // namespace slug
