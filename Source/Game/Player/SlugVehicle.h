#pragma once
#include "Base/Math.h"
#include "Base/Types.h"
#include "Core/Input.h"
#include "Core/Tuning.h"
#include "Game/Combat/CombatSystem.h"
#include "Game/Combat/Projectiles.h"
#include "Game/Player/PlayerInput.h"
#include "Game/Player/PlayerTypes.h"
#include "Game/Player/SlugTypes.h"
#include "World/Combat.h"
#include "World/Physics.h"

struct SlugContext
{
    const SlugTuning* pTuning      = nullptr;
    const MoveTuning* pInputMove   = nullptr;   // 입력 버퍼 창 길이 (플레이어와 같은 값을 쓴다)
    PhysicsWorld      physics      = {};
    CombatSystem*     pCombat      = nullptr;
    ProjectileSystem* pProjectiles = nullptr;
    eGodMode          godMode      = eGodMode::Off;
};

// 파괴되거나 플레이어가 내릴 때 차량이 호스트에게 남기는 "플레이어를 밖으로 내보내라" 요청
struct SlugEjectRequest
{
    bool bValid          = false;
    bool bDestroyed      = false;   // 게이지가 0 이 되어 튕겨 나옴 (자발적 하차가 아님)
    Vec2 feet            = { 0.f, 0.f };
    Vec2 velocity        = { 0.f, 0.f };
    int  facing          = 1;
    int  invincibleTicks = 0;
};

enum class eSlugFxKind : uint8_t
{
    CannonMuzzle,
    Smoke,
    Fire,
    Explosion,
    Debris
};

// 차량이 소유하는 짧은 시각 효과 (총구 불꽃, 연기, 폭발). 렌더가 클립으로 바꾼다.
struct SlugFx
{
    eSlugFxKind kind     = eSlugFxKind::Smoke;
    Vec2        position = { 0.f, 0.f };
    int         facing   = 1;
    int         variant  = 0;
    int         ageTicks = 0;
    int         maxTicks = 0;
};

// 사운드 연동용 누적 통계 (플레이어 PlayerStats 와 같은 방식으로 호스트가 차이를 본다)
struct SlugStats
{
    int vulcanShots = 0;
    int cannonShots = 0;
    int jumps       = 0;
    int hits        = 0;
    int boards      = 0;
    int exits       = 0;
    int explosions  = 0;
    int emptyClicks = 0;   // 탄이 없는데 캐논 버튼을 누른 횟수
};

class SlugVehicle
{
public:
    void Init(const SlugTuning& _tuning, Vec2 _feet, int _facing, uint32_t _ownerId, int _hp = -1);

    void Tick(const SlugContext& _ctx, const InputHistory& _input);

    // 탑승: 주차 상태에서만 가능하다
    [[nodiscard]] bool CanBoard() const;
    void               BeginBoarding();

    void OnHitEvents(Span<const HitEvent> _events, Span<const Hitbox> _hitboxes);

    // 호스트가 매 틱 Tick 뒤에 한 번 읽어 간다
    [[nodiscard]] SlugEjectRequest TakeEjectRequest();

    void AddCannonAmmo(int _count);
    void PushFeetX(float _minX);
    void PushFeetXMax(float _maxX);
    void ForceHp(int _hp);

    [[nodiscard]] eSlugMode          GetMode() const;
    [[nodiscard]] bool               IsDriven() const;
    [[nodiscard]] bool               IsAlive() const;
    [[nodiscard]] uint32_t           GetOwnerId() const;
    [[nodiscard]] Vec2               GetFeet() const;
    [[nodiscard]] Vec2               GetPrevFeet() const;
    [[nodiscard]] int                GetZ() const;   // 몸통 depth (kDefaultZ: kSlugBodyDepth). 부품/FX/화살표/탄은 같은 이동량을 따른다
    void                             SetZ(int _z);
    [[nodiscard]] Vec2               GetVelocity() const;
    [[nodiscard]] int                GetFacing() const;
    [[nodiscard]] bool               IsGrounded() const;
    [[nodiscard]] int                GetHp() const;
    [[nodiscard]] int                GetMaxHp() const;
    [[nodiscard]] float              GetHpRatio() const;
    [[nodiscard]] int                GetCannonAmmo() const;
    [[nodiscard]] int                GetCannonCooldown() const;
    [[nodiscard]] float              GetVulcanAngle() const;
    [[nodiscard]] float              GetVulcanTargetAngle() const;
    [[nodiscard]] int                GetVulcanShotAge() const;
    [[nodiscard]] bool               IsVulcanVisible() const;
    [[nodiscard]] bool               IsBodyVisible() const;
    [[nodiscard]] int                GetCrouchLevel() const;
    [[nodiscard]] eSlugPose          GetPose() const;
    [[nodiscard]] int                GetPoseFrame() const;
    [[nodiscard]] int                GetFlashTicks() const;
    [[nodiscard]] bool               IsBlinkWhite() const;
    [[nodiscard]] int                GetDamageStage() const;   // 0 정상, 1 연기, 2 불꽃 (OUT 화살표 깜빡임)
    [[nodiscard]] bool               ShouldShowInArrow() const;
    [[nodiscard]] bool               ShouldShowOutArrow() const;
    [[nodiscard]] int                GetTick() const;
    [[nodiscard]] Aabb               GetBodyBox() const;
    [[nodiscard]] Aabb               GetBoardBox() const;
    [[nodiscard]] Hurtbox            MakeHurtbox() const;
    [[nodiscard]] Vec2               GetVulcanMount() const;
    [[nodiscard]] Vec2               GetVulcanMuzzle() const;
    [[nodiscard]] Vec2               GetCannonMuzzle() const;
    [[nodiscard]] const SlugStats&   GetStats() const;
    [[nodiscard]] Span<const SlugFx> GetFx() const;
    [[nodiscard]] const Body&        GetBody() const;

    [[nodiscard]] uint64_t HashState() const;

private:
    void TickParked_(const SlugContext& _ctx);
    void TickBoarding_(const SlugContext& _ctx);
    void TickDriven_(const SlugContext& _ctx, const InputHistory& _input);
    void TickExploding_(const SlugContext& _ctx);
    void TickCommon_(const SlugContext& _ctx);

    void Move_(const SlugContext& _ctx, const PlayerCommand& _command);
    void UpdateCrouch_(bool _bWantCrouch);
    void UpdateVulcan_(const SlugContext& _ctx, const PlayerCommand& _command);
    void UpdateCannon_(const SlugContext& _ctx, const PlayerCommand& _command);
    void UpdateCrush_(const SlugContext& _ctx);
    void UpdatePose_();
    void UpdateDamageFx_();
    void FireVulcan_(const SlugContext& _ctx);
    void FireCannon_(const SlugContext& _ctx, const PlayerCommand& _command);
    void BeginExit_(const SlugContext& _ctx);
    void BeginDestroy_(bool _bEjectPlayer);
    void ReleaseCrushId_(const SlugContext& _ctx);
    void ApplyHit_(int _damage, bool _bFromFront);
    void SpawnFx_(eSlugFxKind _kind, Vec2 _position, int _variant, int _maxTicks);

    [[nodiscard]] Vec2 MountOffset_() const;

    SlugTuning m_tuning  = {};
    uint32_t   m_ownerId = kSlugOwnerBase;

    eSlugMode m_mode      = eSlugMode::Parked;
    int       m_modeTicks = 0;
    int       m_tick      = 0;

    Body       m_body     = {};
    Vec2       m_prevFeet = { 0.f, 0.f };
    int        m_z        = kDefaultZ;
    MoveResult m_lastMove = {};
    int        m_facing   = 1;

    PlayerInputBuffer m_input = {};

    int   m_airTicks    = 0;
    bool  m_bCoyote     = true;
    bool  m_bJumping    = false;
    int   m_landTicks   = 0;
    int   m_crouchLevel = 8;   // 0 = 선 자세, 8 = 완전히 눌린 자세 (Slug_Crouch 프레임 번호)
    int   m_crouchTimer = 0;
    int   m_moveX       = 0;
    float m_driveFrame  = 0.f;
    int   m_accelDir    = 0;     // +1 가속 중, -1 감속 중, 0 일정
    float m_accelPhase  = 0.f;   // 가속/감속 프레임 위치 0..3

    float m_vulcanAngle    = 0.f;   // 도, 0 = 오른쪽, +90 = 위 (몸이 보는 방향과 무관)
    float m_vulcanTarget   = 0.f;
    int   m_vulcanCooldown = 0;
    int   m_vulcanShotAge  = 1000;   // 마지막 발사 뒤 경과 틱 (총구 불꽃 클립 위치)
    int   m_cannonCooldown = 0;
    int   m_cannonAmmo     = 0;

    int      m_hp                = 100;
    int      m_flashTicks        = 0;
    int      m_invincibleTicks   = 0;
    eGodMode m_godMode           = eGodMode::Off;   // 에디터 디버그 (해시 제외)
    int      m_hitAge            = -1;              // 피격 경직 클립 경과 틱 (-1 = 없음)
    bool     m_bHitFront         = true;
    int      m_exitCooldown      = 0;
    int      m_parkAnimTicks     = 0;
    int      m_selfDestructTicks = -1;
    bool     m_bExploded         = false;
    int      m_blastTicksLeft    = 0;
    uint32_t m_blastHitId        = kNoHitId;
    bool     m_bReleaseCrush     = false;

    uint32_t m_crushHitId = kNoHitId;
    int      m_crushTimer = 0;

    eSlugPose m_pose      = eSlugPose::ParkedOpen;
    int       m_poseFrame = 0;

    SlugEjectRequest m_eject = {};
    SlugStats        m_stats = {};
    Vector<SlugFx>   m_fx    = {};
};
