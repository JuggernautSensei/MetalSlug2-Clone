#pragma once
#include "Base/Math.h"
#include "Base/Types.h"
#include "Game/Enemies/EnemyBehaviors.h"
#include "Game/Enemies/EnemyWorld.h"

[[nodiscard]] float DxToPlayer(const Enemy& _e, const EnemyEnv& _env);

[[nodiscard]] float DyToPlayer(const Enemy& _e, const EnemyEnv& _env);

[[nodiscard]] float DistToPlayer(const Enemy& _e, const EnemyEnv& _env);

[[nodiscard]] int SideToPlayer(const Enemy& _e, const EnemyEnv& _env);
void              FacePlayer(Enemy& _e, const EnemyEnv& _env);

void EnterState(Enemy& _e, int _state);

void SetClip(Enemy& _e, int _clip, bool _bRestart = false);

[[nodiscard]] bool ClipReachedFrame(const Enemy& _e, const EnemyDef& _def, int _frame);
[[nodiscard]] bool ClipFinished(const Enemy& _e, const EnemyDef& _def);

[[nodiscard]] bool ClimbStep(Enemy& _e, const EnemyDef& _def);
// 계단을 다
void ClimbLeap(Enemy& _e, const EnemyDef& _def);

[[nodiscard]] bool OnRoof(const Enemy& _e);
[[nodiscard]] bool AtRoofEdge(const Enemy& _e);
[[nodiscard]] bool CanLeaveRoof(const Enemy& _e);
// 지붕 위에서 걸어 발판 밖 으로 떨어지지 않도록 x 를 발판 안에 가둔다
void HoldOnRoof(Enemy& _e);

MoveResult StepPhysics(Enemy& _e, const EnemyDef& _def, EnemyEnv& _env);
void       PrepareBody(Enemy& _e, const EnemyDef& _def);

[[nodiscard]] bool WallAhead(const Enemy& _e, const EnemyEnv& _env, float _w, float _h, float _x, float _y);

[[nodiscard]] bool FloorAhead(const Enemy& _e, const EnemyEnv& _env, float _dist, float _drop);

[[nodiscard]] int Roll(EnemyEnv& _env, int _n);

void RequestFx(EnemyEnv& _env, const FxDef& _fx, VECTOR2 _origin, int _facing);

[[nodiscard]] String ResolveSide(const String& _name, int _facing);

bool FireWeapon(Enemy& _e, const EnemyDef& _def, EnemyEnv& _env, int _weapon, VECTOR2 _dir = { 0.f, 0.f }, VECTOR2 _muzzleExtra = { 0.f, 0.f });

[[nodiscard]] VECTOR2 GetMuzzleWorld(const Enemy& _e, const EnemyDef& _def, int _weapon);

void TickWeaponEffects(const Enemy& _e, const EnemyDef& _def, EnemyEnv& _env);

void StartMelee(Enemy& _e, const EnemyDef& _def, EnemyEnv& _env, int _weapon);

void TickMelee(Enemy& _e, const EnemyDef& _def, EnemyEnv& _env);
void CancelMelee(Enemy& _e, EnemyEnv& _env);

void BeginSoldierDeath(Enemy& _e, const EnemyDef& _def, EnemyEnv& _env);
void TickSoldierDeath(Enemy& _e, const EnemyDef& _def, EnemyEnv& _env);

bool SubmitNamedPart(const EnemyRenderEnv& _render, StringView _name, VECTOR2 _origin, int _ticks, bool _bLoop, int _sortKey, int _depth, bool _bFlipX = false, float _partFlash = 0.f);

[[nodiscard]] int GetNamedPartTicks(const AnimationLibrary& _anims, StringView _name);
