#pragma once
#include "Base/Math.h"

#include "Core/Tuning.h"
#include "Game/Player/PlayerTypes.h"
#include "World/Combat.h"
#include "World/Geometry.h"

namespace slug
{
[[nodiscard]] Aabb MakeKnifeTriggerBox(Vec2 _feet, int _facing, float _bodyHalfWidth, const KnifeTuning& _knife);

[[nodiscard]] Aabb MakeKnifeSwingBox(Vec2 _feet, int _facing, const KnifeTuning& _knife);

[[nodiscard]] Vec2 GetMuzzlePosition(Vec2 _feet, int _facing, ePose _pose, eAim _aim, const MuzzleTuning& _muzzle);

[[nodiscard]] Vec2 GetAimDirection(eAim _aim, int _facing);

[[nodiscard]] Vec2 GetBombReleasePosition(Vec2 _feet, int _facing, ePose _pose, const BombTuning& _bomb);

[[nodiscard]] Aabb         MakePlayerHurtboxBox(Vec2 _feet, ePose _pose, const BodyTuning& _body);

[[nodiscard]] constexpr bool IsKnifeSwingActive(
    const int          _ticksSinceStart,
    const KnifeTuning& _knife)
{
    return _ticksSinceStart >= _knife.activeFirstTick && _ticksSinceStart <= _knife.activeLastTick;
}
}
