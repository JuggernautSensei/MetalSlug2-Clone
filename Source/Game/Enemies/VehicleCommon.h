#pragma once
#include "Base/Types.h"
#include "Game/Enemies/EnemyCommon.h"

void BeginVehicleDeath(EnemyState& _e, const EnemyDef& _def, EnemyEnv& _env, int _destroyClip);

void TickVehicleDeath(EnemyState& _e, const EnemyDef& _def, EnemyEnv& _env);

void RequestFxAt(EnemyEnv& _env, StringView _clip, VECTOR2 _at, int _facing, int _depth = 60, float _delaySec = 0.f);

[[nodiscard]] bool IsClipDone(const EnemyState& _e, const EnemyDef& _def);
