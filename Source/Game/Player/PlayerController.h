#pragma once
#include "Base/Types.h"
#include "Base/Math.h"

#include "Core/Input.h"
#include "Core/Tuning.h"
#include "Game/Combat/CombatSystem.h"
#include "Game/Combat/Projectiles.h"
#include "Game/Player/PlayerInput.h"
#include "Game/Player/PlayerRig.h"
#include "Game/Player/PlayerTypes.h"
#include "Game/Player/PlayerWeapons.h"
#include "World/Physics.h"

namespace slug
{
struct PlayerContext
{
    const Tuning*     pTuning      = nullptr;
    PhysicsWorld      physics      = {};
    CombatSystem*     pCombat      = nullptr;
    ProjectileSystem* pProjectiles = nullptr;
    Vec2      respawnFeet  = { 0.f, 0.f };   // 리스폰 시 나타날 발 위치 (카메라 기준으로 호출자가 계산)
};

struct PlayerStats
{
    int shotsFired   = 0;
    int knifeSlashes = 0;
    int bombsThrown  = 0;
    int jumps        = 0;
    int deaths       = 0;
};

class PlayerController
{
public:
    void Reset(const Tuning& _tuning, Vec2 _spawnFeet);

    bool SnapToGround(const PhysicsWorld& _world, float _maxDist);

    void Tick(const PlayerContext& _ctx, const InputHistory& _input);

    bool OnHitEvents(slug::Span<const HitEvent> _events);
    void Kill();

    void AddScore(const int _score) { m_score += _score; }
    void PickUpWeapon(eWeapon _weapon, const Tuning& _tuning);
    void AddBombs(const int _count) { m_weapon.bombs += _count; }
    void SetVictory();

    [[nodiscard]] ePlayerState       GetState() const { return m_state; }
    [[nodiscard]] Vec2       GetFeet() const { return m_body.position; }

    void PushFeetX(float _minX);
    [[nodiscard]] Vec2       GetPrevFeet() const { return m_prevFeet; }
    [[nodiscard]] Vec2       GetVelocity() const { return m_body.velocity; }
    [[nodiscard]] int                GetFacing() const { return m_facing; }
    [[nodiscard]] bool               IsGrounded() const { return m_body.bGrounded; }
    [[nodiscard]] ePose              GetPose() const { return m_pose; }
    [[nodiscard]] eAim               GetAim() const { return m_aim; }
    [[nodiscard]] eOverlay           GetOverlay() const { return m_overlay; }
    [[nodiscard]] const WeaponState& GetWeapon() const { return m_weapon; }
    [[nodiscard]] int                GetLives() const { return m_lives; }
    [[nodiscard]] int                GetScore() const { return m_score; }
    [[nodiscard]] bool               IsAlive() const { return m_state != ePlayerState::Dead; }
    [[nodiscard]] bool               IsInvincible() const { return m_invincibleTicks > 0 || m_state == ePlayerState::Victory; }
    [[nodiscard]] int                GetInvincibleTicks() const { return m_invincibleTicks; }
    void                            ExtendInvincibility(int _ticks) { m_invincibleTicks = (_ticks > m_invincibleTicks) ? _ticks : m_invincibleTicks; }
    [[nodiscard]] bool               IsGameOver() const { return m_bGameOver; }
    [[nodiscard]] int                GetGameOverTicks() const { return m_gameOverTicks; }
    [[nodiscard]] bool               IsKnifeInProgress() const { return m_knifeTick >= 0; }
    [[nodiscard]] bool               IsMoving() const { return m_bMoving; }
    [[nodiscard]] const PlayerStats& GetStats() const { return m_stats; }
    [[nodiscard]] const Body&        GetBody() const { return m_body; }
    [[nodiscard]] Aabb               GetBodyBox() const { return m_body.GetBox(); }
    [[nodiscard]] Hurtbox            MakeHurtbox(const Tuning& _tuning) const;
    [[nodiscard]] bool               IsKnifeTargetInRange() const { return m_bKnifeTargetInRange; }

    [[nodiscard]] RigState MakeRigState() const;

    [[nodiscard]] uint64_t HashState() const;

private:
    void TickAlive_(const PlayerContext& _ctx, const InputHistory& _input);
    void TickDead_(const PlayerContext& _ctx);
    void TickMovement_(const PlayerContext& _ctx, const PlayerCommand& _command);
    void TickActions_(const PlayerContext& _ctx, const PlayerCommand& _command);
    void TickKnife_(const PlayerContext& _ctx, const PlayerCommand& _command);
    void TickBomb_(const PlayerContext& _ctx, const PlayerCommand& _command);
    void TickShot_(const PlayerContext& _ctx, const PlayerCommand& _command);
    void Respawn_(const PlayerContext& _ctx);
    void UpdateAim_(const PlayerCommand& _command);
    void DeriveState_();
    void EndKnife_(const PlayerContext& _ctx);

    Body         m_body      = {};
    Vec2 m_prevFeet  = { 0.f, 0.f };
    Vec2 m_spawnFeet = { 0.f, 0.f };
    MoveResult   m_lastMove  = {};

    PlayerInputBuffer m_input  = {};
    WeaponState       m_weapon = {};
    PlayerStats       m_stats  = {};

    ePlayerState m_state   = ePlayerState::Idle;
    ePose        m_pose    = ePose::Stand;
    eAim         m_aim     = eAim::Forward;
    float        m_aimDegree = 0.f;
    eOverlay     m_overlay = eOverlay::None;
    int          m_facing  = 1;

    int  m_airTicks         = 0;      // 공중에서 끝난 연속 틱 수 (지면이면 0)
    bool m_bCoyoteAvailable = true;
    bool m_bJumping         = false;
    bool m_bAirFromJump     = true;

    int      m_shotTicksLeft       = 0;
    int      m_knifeTick           = -1;   // -1: 베기 없음, 0.. : 발동 후 경과 틱
    uint32_t m_knifeHitId          = kNoHitId;
    bool     m_bSlashSecond        = false;
    int      m_throwTick           = -1;
    bool     m_bKnifeTargetInRange = false;
    uint32_t m_overlaySerial       = 0;

    int                   m_lives           = 0;
    int                   m_score           = 0;
    int                   m_invincibleTicks = 0;
    int                   m_deadTicks       = 0;
    bool                  m_bGameOver       = false;
    int                   m_gameOverTicks   = 0;
    int                   m_idleTicks       = 0;
    uint32_t              m_stateSerial     = 0;
    bool                  m_bMoving         = false;
    int                   m_moveX           = 0;
    bool                  m_bSlashShown     = false;
    bool                  m_bThrowShown     = false;
    slug::Vector<uint32_t> m_pendingRelease  = {};

    const Tuning* m_pTuning = nullptr;
};
}
