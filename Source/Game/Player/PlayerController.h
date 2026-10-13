#pragma once
#include "Base/Types.h"
#include "Core/Input.h"
#include "Core/Tuning.h"
#include "Game/Combat/CombatSystem.h"
#include "Game/Combat/Projectiles.h"
#include "Game/Player/PlayerInput.h"
#include "Game/Player/PlayerRig.h"
#include "Game/Player/PlayerTypes.h"
#include "Game/Player/PlayerWeapons.h"
#include "World/Physics.h"

struct PlayerContext
{
    const Tuning*     pTuning       = nullptr;
    PhysicsWorld      physics       = {};
    CombatSystem*     pCombat       = nullptr;
    ProjectileSystem* pProjectiles  = nullptr;
    VECTOR2           respawnFeet   = { 0.f, 0.f };
    eGodMode          godMode       = eGodMode::Off;
    bool              bInfiniteAmmo = false;
};

struct PlayerStats
{
    int shotsFired   = 0;
    int knifeSlashes = 0;
    int bombsThrown  = 0;
    int jumps        = 0;
    int deaths       = 0;
};

// 시작 낙하산 단계
enum class eChutePhase
{
    None,
    Descend,
    Release,
};

class PlayerController
{
    JUG_CLASS(PlayerController, NO_COPY, NO_MOVE)

public:
    PlayerController() = default;
    void                             Reset(const Tuning& _tuning, VECTOR2 _spawnFeet);
    bool                             SnapToGround(const PhysicsWorld& _world, float _maxDist);
    void                             BeginChuteDescent(const Tuning& _tuning);
    void                             Tick(const PlayerContext& _ctx, const InputHistory& _input);
    bool                             OnHitEvents(Span<const HitEvent> _events);
    void                             Kill();
    void                             AddScore(int _score);
    void                             PickUpWeapon(eWeapon _weapon, const Tuning& _tuning);
    void                             AddBombs(int _count);
    void                             SetVictory();
    [[nodiscard]] eChutePhase        GetChutePhase() const;
    [[nodiscard]] bool               IsChuteVisible(const Tuning& _tuning) const;
    [[nodiscard]] VECTOR2            GetLogicalFeet() const;
    void                             BeginRide();
    void                             SetRideFeet(VECTOR2 _feet, int _facing);
    void                             EndRide(VECTOR2 _feet, VECTOR2 _velocity, int _facing, int _invincibleTicks);
    [[nodiscard]] bool               IsRiding() const;
    void                             ExtendInvincibility(int _ticks);
    [[nodiscard]] ePlayerState       GetState() const;
    [[nodiscard]] VECTOR2            GetFeet() const;
    [[nodiscard]] VECTOR2            GetPrevFeet() const;
    [[nodiscard]] int                GetZ() const;
    void                             SetZ(int _z);
    [[nodiscard]] VECTOR2            GetVelocity() const;
    [[nodiscard]] int                GetFacing() const;
    [[nodiscard]] bool               IsGrounded() const;
    [[nodiscard]] ePose              GetPose() const;
    [[nodiscard]] eAim               GetAim() const;
    [[nodiscard]] eOverlay           GetOverlay() const;
    [[nodiscard]] const WeaponState& GetWeapon() const;
    [[nodiscard]] int                GetLives() const;
    [[nodiscard]] int                GetScore() const;
    [[nodiscard]] bool               IsAlive() const;
    [[nodiscard]] bool               IsInvincible() const;
    [[nodiscard]] int                GetInvincibleTicks() const;
    [[nodiscard]] bool               IsGameOver() const;
    [[nodiscard]] int                GetGameOverTicks() const;
    [[nodiscard]] bool               IsKnifeInProgress() const;
    [[nodiscard]] bool               IsMoving() const;
    [[nodiscard]] const PlayerStats& GetStats() const;
    [[nodiscard]] const Body&        GetBody() const;
    [[nodiscard]] Aabb               GetBodyBox() const;
    [[nodiscard]] Hurtbox            MakeHurtbox(const Tuning& _tuning) const;
    [[nodiscard]] bool               IsKnifeTargetInRange() const;
    [[nodiscard]] RigState           MakeRigState() const;
    [[nodiscard]] uint64_t           HashState() const;

private:
    void                  TickAlive_(const PlayerContext& _ctx, const InputHistory& _input);
    void                  TickDead_(const PlayerContext& _ctx);
    void                  TickMovement_(const PlayerContext& _ctx, const PlayerCommand& _command);
    void                  TickActions_(const PlayerContext& _ctx, const PlayerCommand& _command);
    void                  TickKnife_(const PlayerContext& _ctx, const PlayerCommand& _command);
    void                  TickBomb_(const PlayerContext& _ctx, const PlayerCommand& _command);
    void                  TickShot_(const PlayerContext& _ctx, const PlayerCommand& _command);
    void                  FireBullet_(const PlayerContext& _ctx, const WeaponTuning& _weapon);
    [[nodiscard]] VECTOR2 MakeMuzzle_(const Tuning& _tuning, const WeaponTuning& _weapon) const;
    void                  CancelBurst_();
    void                  StartFlame_(const PlayerContext& _ctx, const WeaponTuning& _flame);
    void                  TickFlame_(const PlayerContext& _ctx, const WeaponTuning& _flame);
    void                  SpawnFlameParticle_(const PlayerContext& _ctx, const WeaponTuning& _flame);
    void                  StopFlameSpawn_();
    void                  EndFlame_();
    // 사격 입력 한 번마다
    [[nodiscard]] uint64_t MakeShotSeed_() const;
    void                   Respawn_(const PlayerContext& _ctx);
    void                   UpdateAim_(const PlayerCommand& _command);
    void                   DeriveState_();
    void                   EndKnife_(const PlayerContext& _ctx);
    void                   TickChuteDescend_(const Tuning& _tuning);
    void                   StartChuteRelease_();

    Body       m_body     = {};
    VECTOR2    m_prevFeet = { 0.f, 0.f };
    int        m_z        = kDefaultZ;
    MoveResult m_lastMove = {};

    eChutePhase m_chutePhase      = eChutePhase::None;
    VECTOR2     m_chuteLandFeet   = { 0.f, 0.f };
    int         m_chuteBlinkTicks = 0;

    PlayerInputBuffer m_input  = {};
    WeaponState       m_weapon = {};
    PlayerStats       m_stats  = {};

    ePlayerState m_state     = ePlayerState::Idle;
    ePose        m_pose      = ePose::Stand;
    eAim         m_aim       = eAim::Forward;
    float        m_aimDegree = 0.f;
    eOverlay     m_overlay   = eOverlay::None;
    int          m_facing    = 1;

    int  m_airTicks         = 0;
    bool m_bCoyoteAvailable = true;
    bool m_bJumping         = false;
    bool m_bAirFromJump     = true;

    int        m_shotTicksLeft       = 0;
    int        m_shotOverlayTicks    = 0;
    RngMt19937 m_shotRng             = RngMt19937 { std::mt19937_64::default_seed };
    int        m_burstLeft           = 0;
    int        m_burstTimer          = 0;
    bool       m_bBurstAir           = false;
    int        m_flameTick           = -1;   // 파이어샷 분출 경과 틱
    bool       m_bFlameStop          = false;
    bool       m_bFlameAir           = false;
    uint32_t   m_flameHitId          = kNoHitId;
    RngMt19937 m_flameRng            = RngMt19937 { std::mt19937_64::default_seed };
    int        m_knifeTick           = -1;
    uint32_t   m_knifeHitId          = kNoHitId;
    bool       m_bSlashSecond        = false;
    int        m_throwTick           = -1;
    bool       m_bKnifeTargetInRange = false;
    uint32_t   m_overlaySerial       = 0;

    int              m_lives           = 0;
    int              m_score           = 0;
    int              m_invincibleTicks = 0;
    int              m_deadTicks       = 0;
    bool             m_bGameOver       = false;
    int              m_gameOverTicks   = 0;
    int              m_idleTicks       = 0;
    uint32_t         m_stateSerial     = 0;
    bool             m_bMoving         = false;
    int              m_moveX           = 0;
    bool             m_bSlashShown     = false;
    bool             m_bThrowShown     = false;
    bool             m_bRiding         = false;
    Vector<uint32_t> m_pendingRelease  = {};

    const Tuning* m_pTuning = nullptr;
    eGodMode      m_godMode = eGodMode::Off;   // 에디터 디버그
};
