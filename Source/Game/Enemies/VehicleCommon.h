#pragma once
#include "Base/Types.h"
#include "Base/Math.h"

#include "Game/Enemies/EnemyCommon.h"

namespace slug
{
void BeginVehicleDeath(Enemy& _e, const EnemyDef& _def, EnemyEnv& _env, int _destroyClip);

void TickVehicleDeath(Enemy& _e, const EnemyDef& _def, EnemyEnv& _env);

void RequestFxAt(EnemyEnv& _env, slug::StringView _clip, Vec2 _at, int _facing, int _layer = 60, float _delaySec = 0.f);

[[nodiscard]] Vec2 ScreenToWorld(const EnemyEnv& _env, Vec2 _screen);
}
