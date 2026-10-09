#pragma once
#include "Game/Enemies/EnemyBehaviors.h"

namespace slug
{

// ===========================================
//  BattleShip (최종보스 Big Shiee) 공개 규약 — 상태 번호와 Enemy::ext 배치
//   구현은 BattleShip.cpp. 테스트/스테이지 도우미가 이 값을 읽는다.
//   ext 배치: [0] 패턴 카운터(홀수=캐논, 짝수=레일), [1] 레일 목표 슬롯, [2] 현재 슬롯,
//             [3] 캐논 각도 x100 (도), [4] 남은 발수, [5] 전투 시작됨(1), [6] 격앙 소폭발 시계,
//             [8 + 8*i .. ] 포탑 i: +0 HP, +1 방향(0 왼쪽 / 1 가운데 / 2 오른쪽), +2 쿨타임 틱,
//                                 +3 동작 남은 틱, +4 동작 종류(0 없음 / 1 왼쪽 회전 / 2 오른쪽 회전 / 3 발사)
// ===========================================

namespace battleship
{

enum eState : int
{
    kEntrance,     // 레일로 등장 (arena.left + entranceOffsetX → 홈 슬롯)
    kIdle,         // 패턴 사이 대기 (1.6 s, 격앙 0.8 s)
    kRailMove,     // 레일 이동 (다른 슬롯)
    kHeadOpen,     // 머리 → 캐논 전환 (Head_Swiching_Start)
    kCannonStart,  // Cannon_Start
    kCannonIdle,   // 캐논 추적 / 발사 대기
    kCannonShot,   // Cannon_Shot_{k} (탄 생성은 시작 순간)
    kCannonEnd,    // Cannon_End
    kHeadClose     // 캐논 → 머리 (Head_Swiching_End)
};

inline constexpr int kExtPatterns     = 0;
inline constexpr int kExtRailTarget   = 1;
inline constexpr int kExtRailSlot     = 2;
inline constexpr int kExtAngle        = 3;
inline constexpr int kExtShotsLeft    = 4;
inline constexpr int kExtFightStarted = 5;
inline constexpr int kExtEnragedTimer = 6;

inline constexpr int kTurretCount  = 2;
inline constexpr int kTurretBase   = 8;
inline constexpr int kTurretStride = 8;
inline constexpr int kTurretHp     = 0;
inline constexpr int kTurretDir    = 1;
inline constexpr int kTurretCool   = 2;
inline constexpr int kTurretBusy   = 3;
inline constexpr int kTurretAction = 4;   // 0 없음 / 1 왼쪽 회전 / 2 오른쪽 회전 / 3 발사

// 격앙(HP x2 <= 최대)
[[nodiscard]] bool IsEnraged(const Enemy& _e);
[[nodiscard]] bool IsFightStarted(const Enemy& _e);
[[nodiscard]] int  GetPatternCount(const Enemy& _e);
[[nodiscard]] int  GetTurretHp(const Enemy& _e, int _index);
[[nodiscard]] int  GetTurretDir(const Enemy& _e, int _index);
[[nodiscard]] float GetCannonAngle(const Enemy& _e);   // 도 (화면 좌표: -90 = 위)

// 원점 = 발 - (0, originDepth) 및 포탑 원점 (월드)
[[nodiscard]] jug::VECTOR2 GetOrigin(const Enemy& _e, const EnemyDef& _def);
[[nodiscard]] jug::VECTOR2 GetTurretOrigin(const Enemy& _e, const EnemyDef& _def, int _index);

// 포탑 피격 소유자 (보스 ID + 파트 번호). 테스트/도구용
[[nodiscard]] constexpr uint32_t TurretOwner(const uint32_t _enemyId, const int _index)
{
    return MakePartOwner(_enemyId, static_cast<uint32_t>(_index));
}

}   // namespace battleship

}   // namespace slug
