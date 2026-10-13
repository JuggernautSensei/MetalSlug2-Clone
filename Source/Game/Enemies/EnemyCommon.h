#pragma once
#include "Base/Types.h"
#include "Game/Enemies/Enemy.h"
#include "Game/Enemies/EnemyWorld.h"

[[nodiscard]] float DxToPlayer(const EnemyState& _e, const EnemyEnv& _env);

[[nodiscard]] float DyToPlayer(const EnemyState& _e, const EnemyEnv& _env);

[[nodiscard]] float DistToPlayer(const EnemyState& _e, const EnemyEnv& _env);

[[nodiscard]] int SideToPlayer(const EnemyState& _e, const EnemyEnv& _env);
void              FacePlayer(EnemyState& _e, const EnemyEnv& _env);

void EnterState(EnemyState& _e, int _state);
void SetClip(EnemyState& _e, int _clip, bool _bRestart = false);

[[nodiscard]] bool ClipReachedFrame(const EnemyState& _e, const EnemyDef& _def, int _frame);
[[nodiscard]] bool ClipFinished(const EnemyState& _e, const EnemyDef& _def);

[[nodiscard]] bool ClimbStep(EnemyState& _e, const EnemyDef& _def);
void               ClimbLeap(EnemyState& _e, const EnemyDef& _def);

[[nodiscard]] bool OnRoof(const EnemyState& _e);
[[nodiscard]] bool AtRoofEdge(const EnemyState& _e);
[[nodiscard]] bool CanLeaveRoof(const EnemyState& _e);

void HoldOnRoof(EnemyState& _e);

MoveResult StepPhysics(EnemyState& _e, const EnemyDef& _def, EnemyEnv& _env);
void       PrepareBody(EnemyState& _e, const EnemyDef& _def);

[[nodiscard]] bool WallAhead(const EnemyState& _e, const EnemyEnv& _env, float _w, float _h, float _x, float _y);

[[nodiscard]] bool FloorAhead(const EnemyState& _e, const EnemyEnv& _env, float _dist, float _drop);

[[nodiscard]] int Roll(EnemyEnv& _env, int _n);

void RequestFx(EnemyEnv& _env, const FxDef& _fx, VECTOR2 _origin, int _facing);

[[nodiscard]] String ResolveSide(const String& _name, int _facing);

bool FireWeapon(EnemyState& _e, const EnemyDef& _def, EnemyEnv& _env, int _weapon, VECTOR2 _dir = { 0.f, 0.f }, VECTOR2 _muzzleExtra = { 0.f, 0.f });

[[nodiscard]] VECTOR2 GetMuzzleWorld(const EnemyState& _e, const EnemyDef& _def, int _weapon);

void TickWeaponEffects(const EnemyState& _e, const EnemyDef& _def, EnemyEnv& _env);

void StartMelee(EnemyState& _e, const EnemyDef& _def, EnemyEnv& _env, int _weapon);

void TickMelee(EnemyState& _e, const EnemyDef& _def, EnemyEnv& _env);
void CancelMelee(EnemyState& _e, EnemyEnv& _env);

void BeginSoldierDeath(EnemyState& _e, const EnemyDef& _def, EnemyEnv& _env);
void TickSoldierDeath(EnemyState& _e, const EnemyDef& _def, EnemyEnv& _env);

bool SubmitNamedPart(const EnemyRenderEnv& _render, StringView _name, VECTOR2 _origin, int _ticks, bool _bLoop, int _sortKey, int _depth, bool _bFlipX = false, float _partFlash = 0.f);

[[nodiscard]] int GetNamedPartTicks(const AnimationLibrary& _anims, StringView _name);
