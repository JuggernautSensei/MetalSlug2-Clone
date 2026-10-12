#pragma once
#include <cstdint>

#include "Base/Math.h"
#include "Base/Types.h"

// 메탈슬러그 SV-001 의 소유자 번호. 플레이어(1) 다음 번호부터 차량 개별 번호로 쓴다 (kSlugOwnerBase + 인덱스).
inline constexpr uint32_t kSlugOwnerBase = 100;
inline constexpr size_t   kMaxSlugs      = 8;

// 차량의 큰 상태. 조종 중 세부 동작(달리기, 점프, 앉기, 피격 경직)은 애니메이션 상태로 따로 도출한다.
enum class eSlugMode : uint8_t
{
    Parked,       // 주차: 해치가 열려 있고 "IN" 화살표가 뜬다
    Boarding,     // 탑승 연출: 플레이어가 안으로 들어가고 해치가 닫힌다 (조종 불가)
    Driven,       // 조종 중
    Exploding,    // 파괴: 깜빡이다 폭발한다
    Gone          // 폭발이 끝나 사라짐
};

// 몸통 애니메이션 상태 (SlugRender 가 클립으로 바꾼다)
enum class eSlugPose : uint8_t
{
    ParkedOpen,
    BoardingIn,
    Idle,
    Drive,
    AccelStart,
    AccelStop,
    Crouching,    // 앉는 중/서는 중 (눌림 단계 1..7)
    LowIdle,
    LowDrive,
    Jump,
    Land,
    HitFront,
    HitBack,
    AirHitA,
    AirHitB
};

struct SlugBodyTuning
{
    Vec2  standSize     = { 104.f, 84.f };    // 물리 몸통 (발 중앙 원점)
    Vec2  crouchSize    = { 104.f, 48.f };
    Vec2  hurtStand     = { 96.f, 80.f };     // 피격 상자
    Vec2  hurtCrouch    = { 96.f, 44.f };
    float footHalfWidth = 36.f;               // 지면 탐침 반폭 (경사를 양쪽 가장자리로 읽는다)
};

struct SlugMoveTuning
{
    float runSpeed         = 150.f;    // px/s
    float crouchSpeed      = 70.f;     // 앉은(눌린) 채 이동
    float airSpeed         = 150.f;
    int   accelTicks       = 6;        // 정지 -> 최고 속도 (AccelStart 클립 재생)
    int   brakeTicks       = 8;        // 최고 속도 -> 정지 (AccelStop 클립 재생)
    float jumpSpeed        = 500.f;    // px/s 초속
    float gravity          = 1224.f;   // px/s²
    float terminalVelocity = 700.f;
    int   coyoteTicks      = 4;
    int   jumpBufferTicks  = 4;
    int   crouchStepTicks  = 2;        // 눌림 1단계에 걸리는 틱 (8단계 = 앉기 전체)
    int   landTicks        = 12;       // 착지 클립 길이 (이동/앉기로 끊긴다)
    float maxStepUp        = 6.f;
    float maxSnapDown      = 6.f;
};

struct SlugVulcanTuning
{
    Vec2  mountStand        = { -6.f, -86.f };   // 발 원점 기준 발칸 회전 중심(공 중심). 오른쪽을 볼 때, 왼쪽을 볼 때 x 가 뒤집힌다
    Vec2  mountCrouch       = { -6.f, -52.f };
    float barrelLength      = 38.f;              // 회전 중심에서 총구까지
    float sweepDegPerSec    = 540.f;             // 조준각이 목표로 도는 속도
    int   fireIntervalTicks = 5;                 // 연사 간격 (홀드 시 자동 연사)
    float bulletSpeed       = 900.f;
    float bulletRange       = 760.f;
    int   damage            = 3;
    Vec2  bulletSize        = { 22.f, 12.f };
    int   shotFlashTicks    = 8;                 // 발사 한 번의 총구 불꽃 클립 길이 (4프레임)
    int   maxAlive          = 0;                 // 0 = 제한 없음
};

struct SlugCannonTuning
{
    int   ammoStart       = 10;      // 탑승 시 HUD 의 CANNON 수 (원작 V1 3:21.4)
    int   ammoMax         = 99;
    int   cooldownTicks   = 20;
    float speed           = 520.f;   // 포탄 초속 (수평)
    float gravity         = 420.f;   // 포물선 (폭탄 버튼 = 곡사)
    float riseSpeed       = 70.f;    // 위로 던지는 초속 (조준 위 시 증가)
    float upBoost         = 260.f;   // 위 조준 때 추가 상승 초속
    Vec2  hitSize         = { 26.f, 20.f };
    int   damage          = 24;
    Vec2  blastSize       = { 120.f, 100.f };
    float blastUp         = 30.f;
    int   blastTicks      = 10;
    Vec2  muzzleStand     = { 60.f, -58.f };   // 오른쪽을 볼 때 발 원점 기준
    Vec2  muzzleCrouch    = { 62.f, -30.f };
    int   muzzleFlashTicks = 36;
    int   pickupAmmo      = 5;       // 폭탄 보급 상자 하나가 주는 캐논탄 (원작 V1 4:40, 9 -> 14)
};

struct SlugDamageTuning
{
    int   maxHp              = 100;
    int   bulletDamage       = 6;      // 적 총알 한 발이 게이지에서 깎는 양
    int   meleeDamage        = 8;
    int   blastDamage        = 20;
    int   hazardDamage       = 12;
    int   hitInvincibleTicks = 18;     // 피격 직후 짧은 무적 (연타 방지)
    float smokeRatio         = 0.50f;  // 이하: 연기
    float fireRatio          = 0.25f;  // 이하: 불꽃 + OUT 화살표 깜빡임
    int   ejectInvincibleTicks = 90;   // 파괴로 튕겨 나온 플레이어 무적
    int   explodeDelayTicks  = 20;     // 게이지 0 -> 폭발까지 (하얗게 깜빡임)
    int   selfDestructTicks  = 240;    // 위험 상태로 버려진 차량이 스스로 폭발하기까지
    Vec2  explosionBlast     = { 190.f, 150.f };
    int   explosionDamage    = 40;
    int   explosionBlastTicks = 12;
};

struct SlugBoardTuning
{
    Vec2  boardBoxSize       = { 96.f, 90.f };   // 탑승 감지 상자 (차량 위쪽 포함, 발 원점 기준 아래서 위로)
    int   boardTicks         = 22;               // 탑승 연출 길이
    int   exitCooldownTicks  = 45;               // 내린 뒤 다시 타기까지
    float exitJumpSpeed      = 380.f;            // 내릴 때 플레이어가 솟는 초속
    float exitSpeedX         = 90.f;
    int   exitTicks          = 20;               // 내린 뒤 차량이 주차 자세로 되돌아가는 연출
    int   arrowGapPx         = 6;                // 차량 위 화살표 간격
};

struct SlugCrushTuning
{
    float minSpeed       = 40.f;     // 이보다 빨라야 보병을 치는 판정이 나간다
    int   damage         = 12;
    int   rehitTicks     = 30;       // 같은 표적을 다시 칠 수 있는 주기
    float frontReach     = 20.f;     // 몸 앞 가장자리에서 더 나가는 폭
};

struct SlugTuning
{
    SlugBodyTuning   body   = {};
    SlugMoveTuning   move   = {};
    SlugVulcanTuning vulcan = {};
    SlugCannonTuning cannon = {};
    SlugDamageTuning damage = {};
    SlugBoardTuning  board  = {};
    SlugCrushTuning  crush  = {};
};
