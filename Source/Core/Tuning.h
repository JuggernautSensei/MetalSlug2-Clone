#pragma once
#include <cstdint>

#include "Base/Result.h"
#include "Base/Types.h"
#include "Base/Math.h"

namespace slug
{
enum class eWeapon : uint8_t
{
    Pistol,
    Hmg,
    Shotgun,
    Rocket,
    Flame,
    Count
};

inline constexpr size_t kWeaponCount = static_cast<size_t>(eWeapon::Count);

struct MoveTuning
{
    float runSpeed         = 250.f;    // px/s, 입력 즉시 최고 속도 (메탈슬러그식 디지털 이동)
    float crouchSpeed      = 125.f;    // px/s
    float airSpeed         = 250.f;    // px/s, 공중 제어 100%
    float jumpSpeed        = 600.f;    // px/s 초속
    float gravity          = 1400.f;   // px/s²
    float terminalVelocity = 1000.f;   // px/s
    int   coyoteTicks      = 4;        // 낭떠러지를 벗어난 뒤에도 점프를 허용하는 틱 수
    int   jumpBufferTicks  = 4;        // 착지 직전에 누른 점프를 기억하는 틱 수
    int   shotBufferTicks  = 6;        // 사격 락 중에 누른 사격을 기억하는 틱 수
    int   bombBufferTicks  = 6;
    float maxStepUp        = 4.f;   // Physics 경사/턱 오르기 높이
    float maxSnapDown      = 4.f;   // Physics 내리막 스냅 높이
    float jumpHeadroom     = 6.f;   // 머리 위 이만큼 안에 천장이 있으면 점프하지 않는다
};

struct BodyTuning
{
    Vec2 standSize     = { 20.f, 58.f };   // 물리 몸통 (발 중앙 원점)
    Vec2 crouchSize    = { 20.f, 36.f };
    Vec2 hurtStand     = { 16.f, 54.f };   // 피격 상자 (몸통보다 약간 작게: 플레이어에게 관대)
    Vec2 hurtCrouch    = { 16.f, 32.f };
    float        footHalfWidth = 4.f;
};

struct MuzzleSet
{
    Vec2 forward  = { 44.f, -51.f };
    Vec2 up       = { 0.f, -106.f };
    Vec2 diagUp   = { 40.f, -62.f };
    Vec2 down     = { 2.f, -2.f };
    Vec2 diagDown = { 20.f, -10.f };
};

struct MuzzleTuning
{
    MuzzleSet stand  = {};
    MuzzleSet crouch = {
        { 38.f, -36.f },
        {  0.f, -80.f },
        { 34.f, -50.f },
        {  2.f,  -2.f },
        { 20.f, -10.f }
    };
    MuzzleSet air = {
        { 44.f,  -50.f },
        {  0.f, -104.f },
        { 40.f,  -60.f },
        {  2.f,   -2.f },
        { 20.f,  -10.f }
    };
};

struct WeaponTuning
{
    int          damage            = 3;
    int          fireIntervalTicks = 4;       // 연타해도 이보다 빨리 쏠 수 없다
    int          holdRepeatTicks   = 6;
    int          ammo              = 0;       // 0 = 무한
    float        projectileSpeed   = 800.f;   // px/s
    int          maxAlive          = 4;       // 동시에 살아 있는 탄 수 상한 (0 = 무제한)
    bool         bPierce           = false;
    Vec2 hitSize           = { 14.f, 10.f };
    float        range             = 420.f;   // 사거리 px
    int          shotOverlayTicks  = 14;      // 사격 상체 동작 길이
    Vec2 blastSize         = { 0.f, 0.f };   // 착탄 폭발 상자 (0 = 폭발 없음)
    int          blastDamage       = 0;
    int          blastTicks        = 0;
};

struct WeaponsTuning
{
    WeaponTuning pistol = {};
    WeaponTuning hmg    = {
        2,
        3,
        3,
        200,
        640.f,
        0,
        false,
        { 14.f, 10.f },
        460.f,
        10
    };
    WeaponTuning shotgun = {
        14,
        16,
        20,
        30,
        560.f,
        0,
        true,
        { 30.f, 24.f },
        200.f,
        16
    };
    WeaponTuning rocket = {
        0,
        24,
        24,
        20,
        300.f,
        2,
        false,
        { 16.f, 12.f },
        420.f,
        20,
        { 96.f, 96.f },
        15,
        6
    };
    WeaponTuning flame = {
        3,
        3,
        3,
        30,
        180.f,
        0,
        true,
        { 40.f, 24.f },
        200.f,
        14
    };
    int hmgPickupAmmo     = 200;
    int shotgunPickupAmmo = 30;
    int rocketPickupAmmo  = 20;
    int flamePickupAmmo   = 30;

    [[nodiscard]] int GetPickupAmmo(
        const eWeapon _weapon) const
    {
        switch (_weapon)
        {
            case eWeapon::Hmg: return hmgPickupAmmo;
            case eWeapon::Shotgun: return shotgunPickupAmmo;
            case eWeapon::Rocket: return rocketPickupAmmo;
            case eWeapon::Flame: return flamePickupAmmo;
            default: return 0;
        }
    }

    [[nodiscard]] const WeaponTuning& Get(
        const eWeapon _weapon) const
    {
        switch (_weapon)
        {
            case eWeapon::Hmg: return hmg;
            case eWeapon::Shotgun: return shotgun;
            case eWeapon::Rocket: return rocket;
            case eWeapon::Flame: return flame;
            default: return pistol;
        }
    }
};

struct KnifeTuning
{
    float range            = 34.f;   // 몸통 앞 가장자리에서 자동 발동 거리
    int   damage           = 8;
    float reach            = 52.f;
    float back             = 6.f;    // 몸 중심에서 뒤쪽 여유
    float bottom           = 4.f;    // 발에서 판정 아래 끝까지
    float top              = 60.f;   // 발에서 판정 위 끝까지
    int   durationTicks    = 18;     // 한 번 베기의 전체 길이 (이 동안 다른 사격/칼 잠금)
    int   activeFirstTick  = 2;      // 판정 시작 (동작 시작 후 몇 번째 틱, 0 부터)
    int   activeLastTick   = 11;     // 판정 종료 (포함)
    int   chainWindowTicks = 6;      // 끝나기 전 이 틱부터 다음 베기 입력을 받아 바로 이어 벤다
};

struct BombTuning
{
    int          initial       = 10;
    int          maxInFlight   = 3;
    int          throwTicks    = 12;   // 던지기 동작 길이
    int          releaseTick   = 3;    // 동작 시작 후 폭탄이 손을 떠나는 틱
    Vec2 releaseOffset = { 14.f, -36.f };
    float        speedX        = 260.f;
    float        speedY        = -330.f;
    int          fuseTicks     = 72;   // 던진 뒤 터질 때까지 (레거시 1.2 s)
    float        inheritMoveX  = 0.35f;   // 플레이어 속도 계승 비율
    float        gravity       = 900.f;
    Vec2 hitSize       = { 12.f, 12.f };   // 비행 중 적과 닿는 크기
    int          damage        = 20;
    Vec2 blastSize     = { 110.f, 130.f };
    float        blastUp       = 40.f;   // 폭발 중심을 착탄점에서 위로 올리는 양
    int          blastTicks    = 10;     // 폭발 판정 유지 틱
};

struct PlayerTuning
{
    int   lives             = 3;
    int   deathTicks        = 110;     // 사망 연출 길이 후 리스폰
    int   respawnInvincible = 120;     // 리스폰 무적 틱
    int   blinkPeriodTicks  = 4;       // 무적 깜빡임 주기
    float respawnOffsetX    = 100.f;   // 카메라 왼쪽 + 이 값
    float respawnDropY      = 40.f;    // 카메라 위쪽에서 이만큼 위
    int   gameOverTicks     = 180;
    int   idleActionTicks   = 480;   // 가만히 있을 때 대기 연출 시작
    int   idleActionLength  = 100;   // 대기 연출 길이 (틱). 끝나면 다시 대기 시간을 센다
};

struct CameraSettings
{
    Vec2 deadzoneHalf = { 24.f, 16.f };
    float        smoothTime   = 0.14f;
    float        lookAheadX   = 0.f;    // 진행 방향 앞쪽으로 보는 양
};

struct SandboxTuning
{
    int   gruntHp              = 1;
    int   jeepHp               = 20;
    int   busHp                = 60;
    int   dummyRespawnTicks    = 240;
    int   shooterIntervalTicks = 150;
    float enemyBulletSpeed     = 220.f;
    int   enemyBulletDamage    = 1;
    float shooterRange         = 380.f;   // 사수가 플레이어를 향해 쏘기 시작하는 가로 거리
};

struct RigTuning
{
    float airBaseline   = 40.f;   // 공중에서 원점이 발보다 몇 px 위인가 (스프라이트 정렬용)
    int   diagUpVariant = 4;
};

struct Tuning
{
    MoveTuning     move    = {};
    BodyTuning     body    = {};
    MuzzleTuning   muzzle  = {};
    WeaponsTuning  weapons = {};
    KnifeTuning    knife   = {};
    BombTuning     bomb    = {};
    PlayerTuning   player  = {};
    CameraSettings camera  = {};
    SandboxTuning  sandbox = {};
    RigTuning      rig     = {};
};

[[nodiscard]] slug::Result<Tuning> ParseTuning(slug::StringView _json);
[[nodiscard]] slug::Result<Tuning> LoadTuning(const slug::FilePath& _path);

[[nodiscard]] slug::String DumpTuning(const Tuning& _tuning);

[[nodiscard]] slug::FilePath GetDefaultTuningPath(const slug::FilePath& _assetRoot);

class TuningWatcher
{
public:
    void Init(slug::FilePath _path, int _pollIntervalTicks = 60);

    bool Poll(Tuning& _out);

    [[nodiscard]] int                  GetReloadCount() const { return m_reloadCount; }

private:
    slug::FilePath                   m_path        = {};
    int                             m_interval    = 60;
    int                             m_counter     = 0;
    int                             m_reloadCount = 0;
    bool                            m_bHasStamp   = false;
    std::filesystem::file_time_type m_stamp       = {};
};
}
