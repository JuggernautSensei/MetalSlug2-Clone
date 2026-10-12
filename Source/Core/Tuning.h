#pragma once
#include <cstdint>

#include "Base/Math.h"
#include "Base/SlugError.h"
#include "Base/Types.h"

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
    int   coyoteTicks      = 4;
    int   jumpBufferTicks  = 4;
    int   shotBufferTicks  = 6;
    int   bombBufferTicks  = 6;
    float maxStepUp        = 4.f;
    float maxSnapDown      = 4.f;
    float jumpHeadroom     = 6.f;
};

struct BodyTuning
{
    Vec2  standSize     = { 20.f, 58.f };
    Vec2  crouchSize    = { 20.f, 36.f };
    Vec2  hurtStand     = { 16.f, 54.f };
    Vec2  hurtCrouch    = { 16.f, 32.f };
    float footHalfWidth = 4.f;
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
    int   damage            = 3;
    int   fireIntervalTicks = 5;
    int   holdRepeatTicks   = 0;       // 0 = 홀드 자동 연사 없음 (연타만 발사)
    int   ammo              = 0;       // 0 = 무한
    float projectileSpeed   = 800.f;   // px/s
    int   maxAlive          = 4;       // 동시에 살아 있는 탄 수 상한 (0 = 무제한)
    bool  bPierce           = false;
    Vec2  hitSize           = { 14.f, 10.f };
    float range             = 420.f;
    int   shotOverlayTicks  = 14;
    Vec2  blastSize         = { 0.f, 0.f };   // 착탄 폭발 상자 (0 = 폭발 없음)
    int   blastDamage       = 0;
    int   blastTicks        = 0;
    // 연속 발사(버스트): 한 번 입력에 나가는 발수는 [burstShotsMin, burstShotsMax] 의 결정적 난수 (0/1 = 단발)
    int   burstShotsMin      = 0;
    int   burstShotsMax      = 0;
    int   burstIntervalTicks = 0;
    float spreadDeg          = 0.f;            // 탄마다 조준각에 더하는 난수 흩뿌림 (±도)
    Vec2  muzzleJitter       = { 0.f, 0.f };   // 탄마다 발사 시작점에 더하는 난수 오프셋 (±px)
    Vec2  muzzleAdjust       = { 0.f, 0.f };
    // 파이어샷 분출 (flame 전용): 입력 1회 = 입자 particleCount 개를 particleIntervalTicks 간격으로 흩뿌린다 (탄약은 1발)
    int   particleCount         = 0;
    int   particleIntervalTicks = 0;
    float particleSpeedMin      = 0.f;   // 입자 속도 범위 (px/s, 결정적 난수)
    float particleSpeedMax      = 0.f;
    int   particleLifeMinTicks  = 0;   // 입자 수명 범위 (틱). 사거리 = 속도 x 수명
    int   particleLifeMaxTicks  = 0;
    int   hitReleaseTicks       = 0;   // 한 입력의 공유 판정 ID 를 이 간격마다 풀어 같은 대상이 다시 맞게 한다 (0 = 풀지 않음)
    Vec2  particleHitSizeEnd    = { 0.f, 0.f };
    float particleScaleStart    = 1.f;   // 수명 시작/끝의 표시 배율 (판정과 무관)
    float particleScaleEnd      = 1.f;
    float speedAccel            = 0.f;   // 발사 후 가속 (px/s², 0 = 등속). 탄 방향으로 speedMax 까지
    float speedMax              = 0.f;   // 가속 상한 (px/s). 0 이면 상한 없음
};

struct WeaponsTuning
{
    WeaponTuning pistol = {};
    WeaponTuning hmg    = {
        .damage             = 2,
        .fireIntervalTicks  = 4,
        .holdRepeatTicks    = 0,
        .ammo               = 200,
        .projectileSpeed    = 640.f,
        .maxAlive           = 0,
        .bPierce            = false,
        .hitSize            = { 14.f, 10.f },
        .range              = 460.f,
        .shotOverlayTicks   = 20,
        .blastSize          = {  0.f,  0.f },
        .blastDamage        = 0,
        .blastTicks         = 0,
        .burstShotsMin      = 3,
        .burstShotsMax      = 5,
        .burstIntervalTicks = 4,
        .muzzleJitter       = {  0.f,  6.f },
        .muzzleAdjust       = {  6.f, 10.f }, // MG 정면 사격 클립의 총구 실측 (44,-51) -> (50,-41)
    };
    WeaponTuning shotgun = {
        14,
        16,
        0,
        30,
        560.f,
        0,
        true,
        { 90.f, 70.f },
        240.f,
        16
    };
    WeaponTuning rocket = {
        .damage            = 0,
        .fireIntervalTicks = 24,
        .holdRepeatTicks   = 0,
        .ammo              = 20,
        .projectileSpeed   = 200.f,
        .maxAlive          = 2,
        .bPierce           = false,
        .hitSize           = { 16.f, 12.f },
        .range             = 420.f,
        .shotOverlayTicks  = 20,
        .blastSize         = { 96.f, 96.f },
        .blastDamage       = 15,
        .blastTicks        = 6,
        .speedAccel        = 1600.f,
        .speedMax          = 900.f,
    };
    // 파이어샷: fireIntervalTicks = 분출 길이 (particleCount x particleIntervalTicks) 라 분출 중 재발사되지 않는다
    WeaponTuning flame = {
        .damage                = 3,
        .fireIntervalTicks     = 24,
        .holdRepeatTicks       = 0,
        .ammo                  = 30,
        .projectileSpeed       = 300.f,
        .maxAlive              = 0,
        .bPierce               = true,
        .hitSize               = {  60.f, 50.f },
        .range                 = 220.f,
        .shotOverlayTicks      = 24,
        .spreadDeg             = 8.f,
        .muzzleJitter          = {   0.f,  4.f },
        .particleCount         = 12,
        .particleIntervalTicks = 2,
        .particleSpeedMin      = 260.f,
        .particleSpeedMax      = 340.f,
        .particleLifeMinTicks  = 22,
        .particleLifeMaxTicks  = 30,
        .hitReleaseTicks       = 6,
        .particleHitSizeEnd    = { 110.f, 90.f },
        .particleScaleStart    = 0.6f,
        .particleScaleEnd      = 1.6f,
    };
    int hmgPickupAmmo     = 200;
    int shotgunPickupAmmo = 30;
    int rocketPickupAmmo  = 20;
    int flamePickupAmmo   = 30;

    [[nodiscard]] int GetPickupAmmo(const eWeapon _weapon) const;

    [[nodiscard]] const WeaponTuning& Get(const eWeapon _weapon) const;
};

struct KnifeTuning
{
    float range            = 34.f;
    int   damage           = 8;
    float reach            = 52.f;
    float back             = 6.f;
    float bottom           = 4.f;
    float top              = 60.f;
    int   durationTicks    = 18;
    int   activeFirstTick  = 2;
    int   activeLastTick   = 11;
    int   chainWindowTicks = 6;
};

struct BombTuning
{
    int   initial       = 10;
    int   maxInFlight   = 3;
    int   throwTicks    = 12;
    int   releaseTick   = 3;
    Vec2  releaseOffset = { 14.f, -36.f };
    float speedX        = 260.f;
    float speedY        = -330.f;
    int   fuseTicks     = 72;   // 던진 뒤 터질 때까지 (1.2 s)
    float inheritMoveX  = 0.35f;
    float gravity       = 900.f;
    Vec2  hitSize       = { 12.f, 12.f };
    int   damage        = 20;
    Vec2  blastSize     = { 110.f, 130.f };
    float blastUp       = 40.f;
    int   blastTicks    = 10;
};

struct PlayerTuning
{
    int   lives             = 3;
    int   deathTicks        = 110;
    int   respawnInvincible = 120;
    int   blinkPeriodTicks  = 4;
    float respawnOffsetX    = 100.f;
    float respawnDropY      = 160.f;
    int   gameOverTicks     = 180;
    int   idleActionTicks   = 480;
    int   idleActionLength  = 100;   // 대기 연출 길이 (틱). 끝나면 다시 대기 시간을 센다
    // 시작 낙하산: 착지 발 위치보다 이만큼 위에서 일정 속도로 내려온다. 공격키로 낙하산이 깜빡이며 사라지고 조작이 시작된다.
    float chuteStartHeightPx    = 260.f;
    float chuteFallSpeedPxSec   = 110.f;
    int   chuteBlinkTicks       = 24;
    int   chuteBlinkPeriodTicks = 3;
    Vec2  chuteOffset           = { 0.f, -110.f };
};

struct CameraSettings
{
    Vec2  deadzoneHalf = { 24.f, 16.f };
    float smoothTime   = 0.14f;
    float lookAheadX   = 0.f;
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
    float shooterRange         = 380.f;
};

struct ItemTuning
{
    float rollFrictionPerSec = 0.5f;   // 굴러가는 점수 아이템 감속률 (1/s). v -= v * rate * dt
    float rollSpinRefSpeed   = 50.f;
    float rollStopSpeed      = 1.f;
};

struct RigTuning
{
    float airBaseline   = 40.f;
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
    ItemTuning     items   = {};
    RigTuning      rig     = {};
};

[[nodiscard]] Result<Tuning> ParseTuning(StringView _json);
[[nodiscard]] Result<Tuning> LoadTuning(const FilePath& _path);

[[nodiscard]] String DumpTuning(const Tuning& _tuning);

[[nodiscard]] FilePath GetDefaultTuningPath(const FilePath& _assetRoot);

class TuningWatcher
{
public:
    void Init(FilePath _path, int _pollIntervalTicks = 60);

    bool Poll(Tuning& _out);

    [[nodiscard]] int GetReloadCount() const;

private:
    FilePath                        m_path        = {};
    int                             m_interval    = 60;
    int                             m_counter     = 0;
    int                             m_reloadCount = 0;
    bool                            m_bHasStamp   = false;
    std::filesystem::file_time_type m_stamp       = {};
};
