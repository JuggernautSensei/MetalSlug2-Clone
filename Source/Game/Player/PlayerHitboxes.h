#pragma once
#include "Core/Tuning.h"
#include "Game/Player/PlayerTypes.h"
#include "World/Combat.h"
#include "World/Geometry.h"

[[nodiscard]] Aabb MakeKnifeTriggerBox(VECTOR2 _feet, int _facing, float _bodyHalfWidth, const KnifeTuning& _knife);

[[nodiscard]] Aabb MakeKnifeSwingBox(VECTOR2 _feet, int _facing, const KnifeTuning& _knife);

[[nodiscard]] VECTOR2 GetMuzzlePosition(VECTOR2 _feet, int _facing, ePose _pose, eAim _aim, const MuzzleTuning& _muzzle);

[[nodiscard]] VECTOR2 GetAimDirection(eAim _aim, int _facing);

[[nodiscard]] VECTOR2 GetBombReleasePosition(VECTOR2 _feet, int _facing, ePose _pose, const BombTuning& _bomb);

[[nodiscard]] Aabb MakePlayerHurtboxBox(VECTOR2 _feet, ePose _pose, const BodyTuning& _body);

[[nodiscard]] bool IsKnifeSwingActive(int _ticksSinceStart, const KnifeTuning& _knife);
