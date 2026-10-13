#pragma once
#include <cstdint>

#include "Base/Math.h"
#include "Base/Types.h"

inline constexpr uint32_t kSlugOwnerBase = 100;
inline constexpr size_t   kMaxSlugs      = 8;

// 차량의 큰 상태
enum class eSlugMode : uint8_t
{
    Parked,
    Boarding,
    Driven,
    Exploding,
    Gone
};

enum class eSlugPose : uint8_t
{
    ParkedOpen,
    BoardingIn,
    Idle,
    Drive,
    AccelStart,
    AccelStop,
    Crouching,
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
    VECTOR2 standSize     = { 104.f, 84.f };
    VECTOR2 crouchSize    = { 104.f, 48.f };
    VECTOR2 hurtStand     = { 96.f, 80.f };
    VECTOR2 hurtCrouch    = { 96.f, 44.f };
    float   footHalfWidth = 36.f;
};

struct SlugMoveTuning
{
    float runSpeed         = 150.f;
    float crouchSpeed      = 70.f;
    float airSpeed         = 150.f;
    int   accelTicks       = 6;
    int   brakeTicks       = 8;
    float jumpSpeed        = 500.f;
    float gravity          = 1224.f;
    float terminalVelocity = 700.f;
    int   coyoteTicks      = 4;
    int   jumpBufferTicks  = 4;
    int   crouchStepTicks  = 2;
    int   landTicks        = 12;
    float maxStepUp        = 6.f;
    float maxSnapDown      = 6.f;
};

struct SlugVulcanTuning
{
    VECTOR2 mountStand        = { -6.f, -86.f };
    VECTOR2 mountCrouch       = { -6.f, -52.f };
    float   barrelLength      = 38.f;
    float   sweepDegPerSec    = 540.f;
    int     fireIntervalTicks = 5;
    float   bulletSpeed       = 900.f;
    float   bulletRange       = 760.f;
    int     damage            = 3;
    VECTOR2 bulletSize        = { 22.f, 12.f };
    int     shotFlashTicks    = 8;   // 발사 한 번의 총구 불꽃 클립 길이
    int     maxAlive          = 0;
};

struct SlugCannonTuning
{
    int     ammoStart        = 10;   // 탑승 시 HUD 의 CANNON 수
    int     ammoMax          = 99;
    int     cooldownTicks    = 20;
    float   speed            = 520.f;
    float   gravity          = 420.f;
    float   riseSpeed        = 70.f;
    float   upBoost          = 260.f;
    VECTOR2 hitSize          = { 26.f, 20.f };
    int     damage           = 24;
    VECTOR2 blastSize        = { 120.f, 100.f };
    float   blastUp          = 30.f;
    int     blastTicks       = 10;
    VECTOR2 muzzleStand      = { 60.f, -58.f };
    VECTOR2 muzzleCrouch     = { 62.f, -30.f };
    int     muzzleFlashTicks = 36;
    int     pickupAmmo       = 5;   // 폭탄 보급 상자 하나가 주는 캐논탄
};

struct SlugDamageTuning
{
    int     maxHp                = 100;
    int     bulletDamage         = 6;
    int     meleeDamage          = 8;
    int     blastDamage          = 20;
    int     hazardDamage         = 12;
    int     hitInvincibleTicks   = 18;   // 피격 직후 짧은 무적
    float   smokeRatio           = 0.50f;
    float   fireRatio            = 0.25f;
    int     ejectInvincibleTicks = 90;
    int     explodeDelayTicks    = 20;
    int     selfDestructTicks    = 240;
    VECTOR2 explosionBlast       = { 190.f, 150.f };
    int     explosionDamage      = 40;
    int     explosionBlastTicks  = 12;
};

struct SlugBoardTuning
{
    VECTOR2 boardBoxSize      = { 96.f, 90.f };
    int     boardTicks        = 22;
    int     exitCooldownTicks = 45;
    float   exitJumpSpeed     = 380.f;
    float   exitSpeedX        = 90.f;
    int     exitTicks         = 20;
    int     arrowGapPx        = 6;
};

struct SlugCrushTuning
{
    float minSpeed   = 40.f;
    int   damage     = 12;
    int   rehitTicks = 30;
    float frontReach = 20.f;
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
