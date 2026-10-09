#pragma once
#include <JugX/Vector2.h>

#include "Game/Enemies/EnemyBehaviors.h"
#include "Game/Enemies/EnemyWorld.h"

namespace slug
{

// ===========================================
//  행동 함수 공용 도우미 (Soldier.cpp, Bzk.cpp ... 가 함께 쓴다)
// ===========================================

// --- 플레이어 상대량 ---
[[nodiscard]] inline float DxToPlayer(const Enemy& _e, const EnemyEnv& _env) { return _env.ctx.player.feet.x - _e.body.position.x; }
// + 면 플레이어가 아래에 있다 (발 기준)
[[nodiscard]] inline float DyToPlayer(const Enemy& _e, const EnemyEnv& _env) { return _env.ctx.player.feet.y - _e.body.position.y; }
[[nodiscard]] float        DistToPlayer(const Enemy& _e, const EnemyEnv& _env);   // 유클리드 (발-발)

// 플레이어 쪽(-1/+1). 가로 거리가 데드존 이내면 현재 방향 유지 (좌우 떨림 방지).
[[nodiscard]] int SideToPlayer(const Enemy& _e, const EnemyEnv& _env);
void              FacePlayer(Enemy& _e, const EnemyEnv& _env);

// --- 상태 / 클립 ---
void EnterState(Enemy& _e, int _state);
// 클립 교체. 같은 클립이면 _bRestart 가 아닌 한 진행을 유지한다.
void SetClip(Enemy& _e, int _clip, bool _bRestart = false);

// 클립의 이벤트 프레임에 이번 틱에 처음 진입했는가 (상태 진입 틱 = 0 기준 clipTicks)
[[nodiscard]] bool ClipReachedFrame(const Enemy& _e, const EnemyDef& _def, int _frame);
[[nodiscard]] bool ClipFinished(const Enemy& _e, const EnemyDef& _def);

// --- 물리 / 지형 ---
MoveResult StepPhysics(Enemy& _e, const EnemyDef& _def, EnemyEnv& _env);
void       PrepareBody(Enemy& _e, const EnemyDef& _def);   // size / footHalfWidth / gravity 플래그를 def 에서 맞춘다
// 전방 프로브 상자(발 기준, 오른쪽 기준 미러)가 지형(픽셀 지면/솔리드)에 닿는가
[[nodiscard]] bool WallAhead(const Enemy& _e, const EnemyEnv& _env, float _w, float _h, float _x, float _y);
// 발 앞쪽 _dist 지점에 _drop 이내의 바닥(지면/플랫폼)이 있는가
[[nodiscard]] bool FloorAhead(const Enemy& _e, const EnemyEnv& _env, float _dist, float _drop);

// --- 난수 / 이벤트 / 이펙트 ---
[[nodiscard]] inline int Roll(EnemyEnv& _env, const int _n) { return _env.world.Roll(_n); }
void                     RequestFx(EnemyEnv& _env, const FxDef& _fx, jug::VECTOR2 _origin, int _facing);
[[nodiscard]] jug::String ResolveSide(const jug::String& _name, int _facing);

// --- 공격 ---
// 무기에서 투사체를 하나 쏜다. _dir 은 aim == Given 일 때의 단위 방향(월드). 쏘았으면 true.
// _muzzleExtra 는 무기 머즐 위치에 더하는 월드 오프셋 (바주카: 조준 방향으로 50px 앞).
bool FireWeapon(Enemy& _e, const EnemyDef& _def, EnemyEnv& _env, int _weapon, jug::VECTOR2 _dir = { 0.f, 0.f }, jug::VECTOR2 _muzzleExtra = { 0.f, 0.f });
[[nodiscard]] jug::VECTOR2 GetMuzzleWorld(const Enemy& _e, const EnemyDef& _def, int _weapon);

// 근접 무기 발동: 무기의 hitboxLifeSec 동안 매 틱 TickMelee 가 Hitbox 를 등록한다.
void StartMelee(Enemy& _e, const EnemyDef& _def, EnemyEnv& _env, int _weapon);
void TickMelee(Enemy& _e, const EnemyDef& _def, EnemyEnv& _env);
void CancelMelee(Enemy& _e, EnemyEnv& _env);

// --- 병사류 사망 연출 (Soldier / Bzk / Hang / FlyingBzk / Shielder 공용) ---
void BeginSoldierDeath(Enemy& _e, const EnemyDef& _def, EnemyEnv& _env);
void TickSoldierDeath(Enemy& _e, const EnemyDef& _def, EnemyEnv& _env);

// --- 부가 파츠 렌더 (보스 리그 등) ---
// 이름은 anim.json 클립을 먼저 찾고(_ticks 만큼 재생, 비루프는 마지막 프레임에 머문다), 없으면 sprites.json 정적 스프라이트.
// _origin 은 파츠 원점(월드 좌표, 피벗 기준). 이름을 찾지 못하면 아무것도 그리지 않고 false.
bool SubmitNamedPart(const EnemyRenderEnv& _render, jug::StringView _name, jug::VECTOR2 _origin, int _ticks, bool _bLoop, int _sortKey, int _layer, bool _bFlipX = false);
// 클립의 전체 길이(틱). 정적 스프라이트/없는 이름은 0.
[[nodiscard]] int GetNamedPartTicks(const AnimationLibrary& _anims, jug::StringView _name);

}   // namespace slug
