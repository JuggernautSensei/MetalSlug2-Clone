#pragma once
#include "Game/Enemies/BigShieeCfg.h"

// 전함 보스 단계
enum class eBigShieePhase : int
{
    Entrance,
    Battery,
    Deploy,
    Cannon
};

enum class eBigShieeTurretMode : int
{
    Idle,
    Prep,
    Turn,   // 머리를 플레이어 쪽 조준 위치 로 한 칸씩 돌린다
    Shot,
    Dead
};

enum class eBigShieeCannonMode : int
{
    Hidden,
    Deploy,
    Aim,
    Ready,
    Shot
};

inline constexpr int kBigShieeExtHp          = 0;    // [0..6] 포탑 체력
inline constexpr int kBigShieeExtMode        = 7;    // [7..13] eBigShieeTurretMode
inline constexpr int kBigShieeExtTicks       = 14;   // [14..20] 현재 모드 경과 틱
inline constexpr int kBigShieeExtPos         = 21;   // [21..27] 머리 위치 0
inline constexpr int kBigShieeExtNextVolley  = 28;   // 다음 포격 시각
inline constexpr int kBigShieeExtCannonMode  = 29;
inline constexpr int kBigShieeExtCannonTicks = 30;
inline constexpr int kBigShieeExtCannonRow   = 31;
inline constexpr int kBigShieeExtNextMissile = 32;   // 다음 고폭탄 시각
inline constexpr int kBigShieeExtDeployStart = 33;
inline constexpr int kBigShieeExtMoving      = 34;
inline constexpr int kBigShieeExtLastKill    = 35;
inline constexpr int kBigShieeExtScrollTicks = 36;
inline constexpr int kBigShieeExtDriving     = 38;
inline constexpr int kBigShieeExtSpeedX      = 37;   // 마지막 틱 선체 월드 속도
inline constexpr int kBigShieeExtNextChute   = 39;   // 다음 낙하산 인질 스폰 시각
inline constexpr int kBigShieeExtCannonYaw   = 40;
inline constexpr int kBigShieeExtCannonTurn  = 41;
inline constexpr int kBigShieeExtLidStart    = 42;
inline constexpr int kBigShieeExtCasingTick  = 44;
inline constexpr int kBigShieeExtCasingX     = 45;
inline constexpr int kBigShieeExtCasingY     = 46;
inline constexpr int kBigShieeExtNextDmgFx   = 47;   // 다음 소폭발 시각
inline constexpr int kBigShieeExtDriftTicks  = 43;
