#pragma once
#include "Game/Enemies/BigShieeCfg.h"

// 전함 보스 단계 (Enemy::state 에 int 로 저장). 사망 연출은 life == Dying 으로 따로 다룬다.
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
    Turn,   // 머리를 플레이어 쪽 조준 위치 (Turn 프레임 0 = 왼쪽, 4 = 위, 8 = 오른쪽) 로 한 칸씩 돌린다
    Shot,   // 발사 연출 (Shot1 = 왼쪽, Shot2 = 위, Shot3 = 오른쪽) + 포탄
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

// Enemy::ext 슬롯 배치 (ext 는 48 칸)
inline constexpr int kBigShieeExtHp          = 0;    // [0..6] 포탑 체력
inline constexpr int kBigShieeExtMode        = 7;    // [7..13] eBigShieeTurretMode
inline constexpr int kBigShieeExtTicks       = 14;   // [14..20] 현재 모드 경과 틱
inline constexpr int kBigShieeExtPos         = 21;   // [21..27] 머리 위치 0 (왼쪽) .. 4 (위) .. 8 (오른쪽)
inline constexpr int kBigShieeExtNextVolley  = 28;   // 다음 포격 시각 (aux 기준 틱)
inline constexpr int kBigShieeExtCannonMode  = 29;
inline constexpr int kBigShieeExtCannonTicks = 30;
inline constexpr int kBigShieeExtCannonRow   = 31;   // 발사 중인 사격 행 0..4 (0 = 왼쪽 .. 4 = 오른쪽): 발사 각도/미사일 스프라이트의 유일한 근거
inline constexpr int kBigShieeExtNextMissile = 32;   // 다음 고폭탄 시각 (aux 기준 틱)
inline constexpr int kBigShieeExtDeployStart = 33;
inline constexpr int kBigShieeExtMoving      = 34;   // 1 이면 선체가 달리는 중 (궤도 애니메이션)
inline constexpr int kBigShieeExtLastKill    = 35;
inline constexpr int kBigShieeExtScrollTicks = 36;
inline constexpr int kBigShieeExtDriving     = 38;   // 1 이면 돌진 중
inline constexpr int kBigShieeExtSpeedX      = 37;   // 마지막 틱 선체 월드 속도 (px/s, 반올림)
inline constexpr int kBigShieeExtNextChute   = 39;   // 다음 낙하산 인질 스폰 시각 (aux 기준 틱)
inline constexpr int kBigShieeExtCannonYaw   = 40;
inline constexpr int kBigShieeExtCannonTurn  = 41;
inline constexpr int kBigShieeExtLidStart    = 42;
inline constexpr int kBigShieeExtCasingTick  = 44;
inline constexpr int kBigShieeExtCasingX     = 45;
inline constexpr int kBigShieeExtCasingY     = 46;
inline constexpr int kBigShieeExtNextDmgFx   = 47;   // 다음 소폭발 (딸피 연출) 시각 (aux 기준 틱)
inline constexpr int kBigShieeExtDriftTicks  = 43;

// Enemy 필드 용도: aux = 생성 후 전체 틱, state = 단계, deathTicks = 사망 후 틱, health = 주포 체력 (보스 체력)
