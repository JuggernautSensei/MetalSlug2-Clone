#pragma once
#include "Game/Enemies/BigShieeCfg.h"

// 전함 보스 단계 (Enemy::state 에 int 로 저장). 사망 연출은 life == Dying 으로 따로 다룬다.
enum class eBigShieePhase : int
{
    Entrance,   // 화면 왼쪽 밖에서 서서히 들어온다 (포탑은 이미 붙어 있고, 입장 중에도 쏜다)
    Battery,    // 입장 끝, 선체 정지 (월드 고정). 포탑 일제는 이어진다. 포탑이 다 부서지면 Deploy
    Deploy,     // 주포가 선체 속에서 올라온다 (선수가 열린다)
    Cannon      // 주포 노출: 플레이어를 조준해 고폭탄을 쏜다. 체력이 0 이 되면 사망 연출
};

enum class eBigShieeTurretMode : int
{
    Idle,
    Prep,   // 사격 준비 (제자리)
    Turn,   // 머리를 플레이어 쪽 조준 위치 (Turn 프레임 0 = 왼쪽, 4 = 위, 8 = 오른쪽) 로 한 칸씩 돌린다
    Shot,   // 발사 연출 (Shot1 = 왼쪽, Shot2 = 위, Shot3 = 오른쪽) + 포탄
    Dead
};

enum class eBigShieeCannonMode : int
{
    Hidden,
    Deploy,   // Boss_Cannon_Deploy
    Aim,      // 전개 직후 플레이어 쪽으로 머리를 돌린다 (Boss_Cannon_Aim 프레임, 한 칸씩)
    Ready,    // 대기: 현재 yaw 프레임 유지, 플레이어가 움직이면 따라 돈다
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
inline constexpr int kBigShieeExtDeployStart = 33;   // 전개 시작 시각 (aux 기준 틱, 아직 없으면 -1)
inline constexpr int kBigShieeExtMoving      = 34;   // 1 이면 선체가 달리는 중 (궤도 애니메이션)
inline constexpr int kBigShieeExtLastKill    = 35;   // 마지막 포탑이 부서진 시각 (aux 기준 틱, 없으면 -1)
inline constexpr int kBigShieeExtScrollTicks = 36;   // 지금까지 카메라 스크롤을 요청한 틱 수 (scrollBudgetPx 와 비교)
inline constexpr int kBigShieeExtDriving     = 38;   // 1 이면 돌진 중
inline constexpr int kBigShieeExtSpeedX      = 37;   // 마지막 틱 선체 월드 속도 (px/s, 반올림)
inline constexpr int kBigShieeExtNextChute   = 39;   // 다음 낙하산 인질 스폰 시각 (aux 기준 틱)
inline constexpr int kBigShieeExtCannonYaw   = 40;   // 주포 머리 방향 0 (왼쪽) .. 8 (오른쪽) = Boss_Cannon_Aim 프레임
inline constexpr int kBigShieeExtCannonTurn  = 41;   // 마지막으로 한 칸 돈 뒤 경과 틱
inline constexpr int kBigShieeExtLidStart    = 42;   // 선수 (뚜껑) 가 열리기 시작한 시각 (aux 기준 틱, 아직 없으면 -1)
inline constexpr int kBigShieeExtCasingTick  = 44;   // 탄피가 튄 시각 (aux 기준 틱, 아직 없으면 0)
inline constexpr int kBigShieeExtCasingX     = 45;   // 탄피 출발 월드 x (반올림)
inline constexpr int kBigShieeExtCasingY     = 46;   // 탄피 출발 월드 y (반올림)
inline constexpr int kBigShieeExtNextDmgFx   = 47;   // 다음 소폭발 (딸피 연출) 시각 (aux 기준 틱)
inline constexpr int kBigShieeExtDriftTicks  = 43;   // 카메라가 멈춘 뒤 선체가 스스로 왼쪽으로 흐른 틱 수 (shuttleDriftSec 에 닿으면 돌진)

// Enemy 필드 용도: aux = 생성 후 전체 틱, state = 단계, deathTicks = 사망 후 틱, health = 주포 체력 (보스 체력)
