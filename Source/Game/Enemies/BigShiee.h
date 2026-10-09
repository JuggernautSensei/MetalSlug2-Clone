#pragma once
#include "Base/Math.h"
#include "Game/Enemies/EnemyBehaviors.h"

namespace slug
{
namespace bigshiee
{

// 원작 Metal Slug 2 Mission 4 최종보스 "Big Shiee". 상태는 등장 → 포탑 연사 → 캐논 전개 → 캐논 볼리 순서.
enum eState : int
{
    kEntrance,
    kAssault,
    kDeploy,
    kCannon
};

inline constexpr int kTurretCount = kBigShieeTurretCount;   // 포탑 A..G (피격 파트 0..6)
inline constexpr int kCannonPart  = kTurretCount;           // 캐논 헤드 피격 파트 (7)

// Enemy::ext 사용 규약 (ext[24] 안에 맞춘다)
inline constexpr int kExtTurret      = 0;    // ext[0..6]: hp (bit 0..7) | 사격 예고 남은 틱 (bit 8..15)
inline constexpr int kExtQueue       = 7;    // ext[7..14]: 예약 발사 큐 (fireAt | kind | source | dx)
inline constexpr int kExtQueueCount  = 8;
inline constexpr int kExtCannonHp    = 15;
inline constexpr int kExtFlags       = 16;   // bit0 전투 시작, bit1 캐논 파괴
inline constexpr int kExtDriveTarget = 17;   // 선체 주행 목표 (화면 x)
inline constexpr int kExtDriveRest   = 18;   // 도착 후 정지 틱
inline constexpr int kExtLastShot    = 19;   // 마지막 캐논 발사 전투 틱 + 1 (0 = 없음)
inline constexpr int kExtAimBin      = 20;   // 캐논 조준 구간 0..8 (Gun_Aim 프레임)
inline constexpr int kExtVolleyBusy  = 21;   // 직전 볼리의 마지막 트레이서가 사라지는 전투 틱 (이 전까지 새 볼리 금지)

[[nodiscard]] bool  IsEnraged(const Enemy& _e);
[[nodiscard]] bool  IsFightStarted(const Enemy& _e);
[[nodiscard]] bool  IsCannonDestroyed(const Enemy& _e);
[[nodiscard]] int   GetTurretHp(const Enemy& _e, int _index);
[[nodiscard]] int   GetCannonHp(const Enemy& _e);
[[nodiscard]] int   GetVolleyCount(const Enemy& _e);
[[nodiscard]] int   GetQueuedCount(const Enemy& _e);
[[nodiscard]] Vec2  GetOrigin(const Enemy& _e, const EnemyDef& _def);

[[nodiscard]] constexpr uint32_t TurretOwner(const uint32_t _enemyId, const int _index)
{
    return MakePartOwner(_enemyId, static_cast<uint32_t>(_index));
}

[[nodiscard]] constexpr uint32_t CannonOwner(const uint32_t _enemyId)
{
    return MakePartOwner(_enemyId, static_cast<uint32_t>(kCannonPart));
}
}
}
