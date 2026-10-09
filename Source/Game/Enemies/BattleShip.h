#pragma once
#include "Base/Math.h"
#include "Game/Enemies/EnemyBehaviors.h"

namespace slug
{
namespace battleship
{

enum eState : int
{
    kEntrance,
    kIdle,
    kRailMove,
    kHeadOpen,
    kCannonStart,
    kCannonIdle,
    kCannonShot,
    kCannonEnd,
    kHeadClose
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

[[nodiscard]] bool IsEnraged(const Enemy& _e);
[[nodiscard]] bool IsFightStarted(const Enemy& _e);
[[nodiscard]] int  GetPatternCount(const Enemy& _e);
[[nodiscard]] int  GetTurretHp(const Enemy& _e, int _index);
[[nodiscard]] int  GetTurretDir(const Enemy& _e, int _index);
[[nodiscard]] float GetCannonAngle(const Enemy& _e);

[[nodiscard]] Vec2 GetOrigin(const Enemy& _e, const EnemyDef& _def);
[[nodiscard]] Vec2 GetTurretOrigin(const Enemy& _e, const EnemyDef& _def, int _index);

[[nodiscard]] constexpr uint32_t TurretOwner(const uint32_t _enemyId, const int _index)
{
    return MakePartOwner(_enemyId, static_cast<uint32_t>(_index));
}
}
}
