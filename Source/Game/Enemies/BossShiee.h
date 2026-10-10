#pragma once
#include "Base/Math.h"
#include "Game/Enemies/EnemyBehaviors.h"

namespace slug
{
namespace bossshiee
{
// 최종보스 Boss Shiee (원작 MS2 Mission 4, Docs/BossLayout.md). 선체는 파괴되지 않고 탄만 흡수한다.
// 파츠: 포탑 A..G (파트 0..6, 각자 체력), 스컬 캐논 헤드 (파트 7, 체력 = Enemy::health).
// 상태: 등장 -> 포탑 사격 (캐논 전개 조건까지) -> 캐논 전개 -> 캐논 볼리 반복 -> 사망 연출 (Dying).
enum eState : int
{
    kEntrance,   // 화면 왼쪽 밖에서 굴러 들어온다
    kAssault,    // 포탑만 사격한다 (포탑 4기 파괴 또는 30 s 뒤 전개)
    kDeploy,     // 캐논 전개 (Cannon_Deploy1 -> 3 -> 6 -> 9)
    kCannon      // 캐논 볼리 + 포탑 사격
};

enum eTurretPhase : int
{
    kTurretIdle,
    kTurretTurning,    // 예고 1: 포탑이 돈다
    kTurretShooting    // 예고 2: 사격 클립 재생 후 발사
};

inline constexpr int kTurretCount = kBossTurretCount;   // 포탑 A..G (파트 0..6)
inline constexpr int kCannonPart  = kBossCannonPart;    // 캐논 헤드 (파트 7)

// Enemy::ext 사용 규약 (ext[24] 안에 맞춘다)
inline constexpr int kExtQueue       = 0;    // ext[0..7]: 예약 발사 큐 (fireAt | kind | source | dx)
inline constexpr int kExtQueueCount  = 8;
inline constexpr int kExtFlags       = 8;    // bit0 전투 시작 (등장 끝), bit1 캐논 파괴
inline constexpr int kExtScrollTimer = 9;    // 스크롤 버스트 주기 틱
inline constexpr int kExtScrollLeft  = 10;   // 남은 스크롤 (1/1000 px)
inline constexpr int kExtNextTurret  = 11;   // 다음 포탑 사격 시각 (전투 시계)
inline constexpr int kExtNextVolley  = 12;   // 다음 캐논 볼리 시각
inline constexpr int kExtVolleyCount = 13;   // 볼리 번호 (주황 포탄 주기)
inline constexpr int kExtAimStart    = 14;   // 캐논 조준 시작 시각 + 1 (0 = 없음)
inline constexpr int kExtAimBin      = 15;   // 캐논 조준 구간 0..8 (Gun_Aim 프레임 / Gun_Shoot 행)
inline constexpr int kExtLastShot    = 16;   // 마지막 캐논 발사 시각 + 1 (0 = 없음)
inline constexpr int kExtTurret      = 17;   // ext[17..23]: 포탑 상태 = hp (bit 0..7) | 단계 (bit 8..9) | 단계 틱 (bit 10..19)

[[nodiscard]] bool IsFightStarted(const Enemy& _e);
[[nodiscard]] bool IsCannonDestroyed(const Enemy& _e);
[[nodiscard]] int  GetTurretHp(const Enemy& _e, int _index);
[[nodiscard]] int  GetTurretPhase(const Enemy& _e, int _index);
[[nodiscard]] int  GetTurretTicks(const Enemy& _e, int _index);   // 현재 단계 경과 틱
[[nodiscard]] int  GetTurretsDestroyed(const Enemy& _e);
[[nodiscard]] int  GetCannonHp(const Enemy& _e);
[[nodiscard]] int  GetVolleyCount(const Enemy& _e);
[[nodiscard]] int  GetScrollLeftPx(const Enemy& _e);
[[nodiscard]] int  GetQueuedCount(const Enemy& _e);
[[nodiscard]] int  GetFightClock(const Enemy& _e);    // 전투 시계 (등장 종료 후 틱)
[[nodiscard]] int  GetAimStartTick(const Enemy& _e);  // 캐논 볼리 조준 시작 시각 (없으면 -1)
[[nodiscard]] Vec2 GetOrigin(const Enemy& _e, const EnemyDef& _def);
[[nodiscard]] Vec2 GetTurretCentre(const Enemy& _e, const EnemyDef& _def, int _index);
[[nodiscard]] Vec2 GetCannonMuzzle(const Enemy& _e, const EnemyDef& _def);

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
