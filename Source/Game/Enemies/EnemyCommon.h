#pragma once
#include "Base/Math.h"
#include "Base/Types.h"

#include "Game/Enemies/EnemyBehaviors.h"
#include "Game/Enemies/EnemyWorld.h"

namespace slug
{
[[nodiscard]] inline float DxToPlayer(const Enemy& _e, const EnemyEnv& _env) { return _env.ctx.player.feet.x - _e.body.position.x; }
[[nodiscard]] inline float DyToPlayer(const Enemy& _e, const EnemyEnv& _env) { return _env.ctx.player.feet.y - _e.body.position.y; }
[[nodiscard]] float        DistToPlayer(const Enemy& _e, const EnemyEnv& _env);

[[nodiscard]] int SideToPlayer(const Enemy& _e, const EnemyEnv& _env);
void              FacePlayer(Enemy& _e, const EnemyEnv& _env);

void EnterState(Enemy& _e, int _state);
void SetClip(Enemy& _e, int _clip, bool _bRestart = false);

[[nodiscard]] bool ClipReachedFrame(const Enemy& _e, const EnemyDef& _def, int _frame);
[[nodiscard]] bool ClipFinished(const Enemy& _e, const EnemyDef& _def);

MoveResult StepPhysics(Enemy& _e, const EnemyDef& _def, EnemyEnv& _env);
void       PrepareBody(Enemy& _e, const EnemyDef& _def);
[[nodiscard]] bool WallAhead(const Enemy& _e, const EnemyEnv& _env, float _w, float _h, float _x, float _y);
[[nodiscard]] bool FloorAhead(const Enemy& _e, const EnemyEnv& _env, float _dist, float _drop);

[[nodiscard]] inline int Roll(EnemyEnv& _env, const int _n) { return _env.world.Roll(_n); }
void                     RequestFx(EnemyEnv& _env, const FxDef& _fx, Vec2 _origin, int _facing);
[[nodiscard]] slug::String ResolveSide(const slug::String& _name, int _facing);

bool FireWeapon(Enemy& _e, const EnemyDef& _def, EnemyEnv& _env, int _weapon, Vec2 _dir = { 0.f, 0.f }, Vec2 _muzzleExtra = { 0.f, 0.f });
[[nodiscard]] Vec2 GetMuzzleWorld(const Enemy& _e, const EnemyDef& _def, int _weapon);

void StartMelee(Enemy& _e, const EnemyDef& _def, EnemyEnv& _env, int _weapon);
void TickMelee(Enemy& _e, const EnemyDef& _def, EnemyEnv& _env);
void CancelMelee(Enemy& _e, EnemyEnv& _env);

void BeginSoldierDeath(Enemy& _e, const EnemyDef& _def, EnemyEnv& _env);
void TickSoldierDeath(Enemy& _e, const EnemyDef& _def, EnemyEnv& _env);

bool SubmitNamedPart(const EnemyRenderEnv& _render, slug::StringView _name, Vec2 _origin, int _ticks, bool _bLoop, int _sortKey, int _layer, bool _bFlipX = false);
[[nodiscard]] int GetNamedPartTicks(const AnimationLibrary& _anims, slug::StringView _name);
}
