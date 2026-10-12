#pragma once
#include "Base/Math.h"
#include "Base/Types.h"
#include "Game/Enemies/EnemyCommon.h"

void BeginVehicleDeath(
    Enemy&          _e,
    const EnemyDef& _def,
    EnemyEnv&       _env,
    int             _destroyClip);

void TickVehicleDeath(Enemy& _e, const EnemyDef& _def, EnemyEnv& _env);

void RequestFxAt(
    EnemyEnv&  _env,
    StringView _clip,
    Vec2       _at,
    int        _facing,
    int        _depth    = 60,
    float      _delaySec = 0.f);

// 현재 클립을 끝까지 재생했는가
[[nodiscard]] bool IsClipDone(const Enemy& _e, const EnemyDef& _def);
