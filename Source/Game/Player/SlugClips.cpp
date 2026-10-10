#include "Game/Player/SlugClips.h"

#include <cmath>
#include <format>

#include "Base/Math.h"
#include "Base/Types.h"

namespace slug
{

namespace
{
    constexpr ARRAY<SlugClipInfo, static_cast<size_t>(eSlugClip::Count)> kClipTable = { {
        { "Slug_Idle", 3, 6 },
        { "Slug_Drive", 20, 20 },
        { "Slug_AccelFwd", 4, 15 },
        { "Slug_AccelBack", 4, 15 },
        { "Slug_Elev", 10, 15 },
        { "Slug_Jump", 4, 12 },
        { "Slug_Land", 4, 15 },
        { "Slug_Crouch", 9, 24 },
        { "Slug_LowIdle", 3, 6 },
        { "Slug_LowDrive", 20, 20 },
        { "Slug_LowAccelFwd", 4, 15 },
        { "Slug_LowAccelBack", 4, 15 },
        { "Slug_Enter", 7, 12 },
        { "Slug_HitFront", 16, 30 },
        { "Slug_HitBack", 10, 30 },
        { "Slug_AirHitA", 14, 30 },
        { "Slug_AirHitB", 12, 30 },
        { "Slug_Vulcan_Idle", 32, 1 },
        { "Slug_Bullet", 17, 30 },
        { "Slug_CannonFx_0", 18, 30 },
        { "Slug_CannonFx_Air", 3, 30 },
        { "Slug_CannonFx_Crouch", 16, 30 },
        { "Slug_VulcanDebris", 11, 24 },
        { "Slug_Explosion", 30, 30 },
        { "Slug_In", 17, 14 },
        { "Slug_Out", 17, 14 },
    } };
}   // namespace

const SlugClipInfo& GetSlugClipInfo(
    const eSlugClip _clip)
{
    return kClipTable[static_cast<size_t>(_clip)];
}

int VulcanIndexFromAngle(
    const float _angleDeg)
{
    const float steps = (135.f - _angleDeg) / kVulcanAngleStep;
    int         index = static_cast<int>(std::lround(steps)) % kVulcanAngleCount;
    if (index < 0)
    {
        index += kVulcanAngleCount;
    }
    return index;
}

float VulcanAngleFromIndex(
    const int _index)
{
    return 135.f - kVulcanAngleStep * static_cast<float>(_index);
}

String GetVulcanShotClipName(
    const int _index)
{
    return String { std::format("Slug_Vulcan_Shot_{:02}", _index) };
}

String GetShellClipName(
    const int _row)
{
    return String { std::format("Slug_Shell_{}", _row) };
}

}   // namespace slug
