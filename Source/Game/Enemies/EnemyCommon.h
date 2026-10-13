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

// 계단 병사 오르기 한 틱: 스폰 -> arena.min -> arena.max -> (climbEnd) 를 직선으로 걷는다 (aux = 구간). 끝에 닿으면 true
[[nodiscard]] bool ClimbStep(Enemy& _e, const EnemyDef& _def);
// 계단을 다 오른 병사의 전방 점프: 버스 쪽으로 점프해 승강대 벽 (버스 몸체) 을 넘는다. 착지하면 일반 AI
void ClimbLeap(Enemy& _e, const EnemyDef& _def);

// 지붕 위 계단 병사: 지붕 발판 (roofSpan, 윗면 = climbEnd.y) 에 서 있는지 / 끝까지 걸어 더 못 가는지 / 점프 베기로 버스 밖에 내릴 수 있는 자리인지
[[nodiscard]] bool OnRoof(const Enemy& _e);
[[nodiscard]] bool AtRoofEdge(const Enemy& _e);
[[nodiscard]] bool CanLeaveRoof(const Enemy& _e);
// 지붕 위에서 걸어 발판 밖 (= 버스 몸통 안) 으로 떨어지지 않도록 x 를 발판 안에 가둔다 (물리 한 틱 뒤에 부른다)
void HoldOnRoof(Enemy& _e);

MoveResult StepPhysics(Enemy& _e, const EnemyDef& _def, EnemyEnv& _env);
void       PrepareBody(Enemy& _e, const EnemyDef& _def);

[[nodiscard]] bool WallAhead(const Enemy& _e, const EnemyEnv& _env, float _w, float _h, float _x, float _y);

[[nodiscard]] bool FloorAhead(const Enemy& _e, const EnemyEnv& _env, float _dist, float _drop);

[[nodiscard]] int Roll(EnemyEnv& _env, const int _n);

void RequestFx(EnemyEnv& _env, const FxDef& _fx, VECTOR2 _origin, int _facing);

[[nodiscard]] String ResolveSide(const String& _name, int _facing);

bool FireWeapon(Enemy& _e, const EnemyDef& _def, EnemyEnv& _env, int _weapon, VECTOR2 _dir = { 0.f, 0.f }, VECTOR2 _muzzleExtra = { 0.f, 0.f });

[[nodiscard]] VECTOR2 GetMuzzleWorld(const Enemy& _e, const EnemyDef& _def, int _weapon);

// 무기의 effects: motionClip 의 frame 이 시작되는 틱에 이펙트를 낸다 (effects 가 없으면 아무것도 안 한다)
void TickWeaponEffects(const Enemy& _e, const EnemyDef& _def, EnemyEnv& _env);

void StartMelee(Enemy& _e, const EnemyDef& _def, EnemyEnv& _env, int _weapon);

void TickMelee(Enemy& _e, const EnemyDef& _def, EnemyEnv& _env);
void CancelMelee(Enemy& _e, EnemyEnv& _env);

void BeginSoldierDeath(Enemy& _e, const EnemyDef& _def, EnemyEnv& _env);
void TickSoldierDeath(Enemy& _e, const EnemyDef& _def, EnemyEnv& _env);

// _partFlash: 이 부위만의 추가 플래시 양 (PartFlashAmount). 적 전체 플래시(_render.flash)와 max 로 합친다.
bool SubmitNamedPart(
    const EnemyRenderEnv& _render,
    StringView            _name,
    VECTOR2               _origin,
    int                   _ticks,
    bool                  _bLoop,
    int                   _sortKey,
    int                   _depth,
    bool                  _bFlipX    = false,
    float                 _partFlash = 0.f);

[[nodiscard]] int GetNamedPartTicks(const AnimationLibrary& _anims, StringView _name);
