#pragma once
#include <JugX/Typedef.h>
#include <JugX/Vector2.h>

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

// ===========================================
//  PlayerContext
//   컨트롤러가 한 틱 동안 쓰는 외부 서비스 (비소유). 틱마다 새로 채워서 넘긴다.
// ===========================================

struct PlayerContext
{
    const Tuning*     pTuning      = nullptr;
    PhysicsWorld      physics      = {};
    CombatSystem*     pCombat      = nullptr;
    ProjectileSystem* pProjectiles = nullptr;
    jug::VECTOR2      respawnFeet  = { 0.f, 0.f };   // 리스폰 시 나타날 발 위치 (카메라 기준으로 호출자가 계산)
};

// 테스트/디버그용 누적 통계
struct PlayerStats
{
    int shotsFired   = 0;
    int knifeSlashes = 0;
    int bombsThrown  = 0;
    int jumps        = 0;
    int deaths       = 0;
};

// ===========================================
//  PlayerController
//   입력 → 상태 → 물리 → 행동(칼/폭탄/사격) → 판정 등록 을 한 틱에 순서대로 처리한다. 상태는 같은 틱에 바뀐다.
//   하위 책임은 분리되어 있다: PlayerInputBuffer(버퍼), WeaponState(무기/탄), PlayerHitboxes(판정 기하), PlayerRig(스프라이트).
//   목숨/점수는 세션 값으로 컨트롤러가 들고 있다 (전역 없음).
//
//   틱 안의 순서 (호출자 기준):
//     combat.BeginTick() → 표적들이 Hurtbox 등록 → player.Tick() → projectiles.Step() → … → combat.Resolve() → player.OnHitEvents()
// ===========================================

class PlayerController
{
public:
    // 새 게임: 목숨/점수/무기를 초기화하고 _spawnFeet 에 배치한다.
    void Reset(const Tuning& _tuning, jug::VECTOR2 _spawnFeet);

    // 스폰 직후 발을 지면에 붙인다 (월드 준비 후 호출). 성공 시 true.
    bool SnapToGround(const PhysicsWorld& _world, float _maxDist);

    void Tick(const PlayerContext& _ctx, const InputHistory& _input);

    // Resolve 결과 반영: 이 플레이어가 맞았으면 사망 처리. 반환: 사망했는가
    bool OnHitEvents(jug::Span<const HitEvent> _events);
    void Kill();   // 직접 사망 처리 (무적/이미 사망이면 무시)

    // --- 외부 이벤트 ---
    void AddScore(const int _score) { m_score += _score; }
    void PickUpWeapon(eWeapon _weapon, const Tuning& _tuning);
    void AddBombs(const int _count) { m_weapon.bombs += _count; }
    void SetVictory();

    // --- 조회 ---
    [[nodiscard]] ePlayerState       GetState() const { return m_state; }
    [[nodiscard]] jug::VECTOR2       GetFeet() const { return m_body.position; }
    [[nodiscard]] jug::VECTOR2       GetPrevFeet() const { return m_prevFeet; }
    [[nodiscard]] jug::VECTOR2       GetVelocity() const { return m_body.velocity; }
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
    [[nodiscard]] bool               IsGameOver() const { return m_bGameOver; }
    [[nodiscard]] int                GetGameOverTicks() const { return m_gameOverTicks; }
    [[nodiscard]] int                GetDeadTicks() const { return m_deadTicks; }
    [[nodiscard]] bool               IsKnifeInProgress() const { return m_knifeTick >= 0; }
    [[nodiscard]] bool               IsKnifeActive() const { return m_bKnifeEmitted; }
    [[nodiscard]] bool               IsMoving() const { return m_bMoving; }
    [[nodiscard]] const PlayerStats& GetStats() const { return m_stats; }
    [[nodiscard]] const Body&        GetBody() const { return m_body; }
    [[nodiscard]] Aabb               GetBodyBox() const { return m_body.GetBox(); }
    [[nodiscard]] Hurtbox            MakeHurtbox(const Tuning& _tuning) const;
    [[nodiscard]] bool               IsKnifeTargetInRange() const { return m_bKnifeTargetInRange; }

    // 이 틱의 리그 입력 (렌더/테스트용)
    [[nodiscard]] RigState MakeRigState() const;

    // 시뮬레이션 상태 해시 (결정성 테스트)
    [[nodiscard]] uint64_t HashState() const;

private:
    // --- 한 틱의 단계들 ---
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

    // --- 데이터 ---
    Body         m_body      = {};
    jug::VECTOR2 m_prevFeet  = { 0.f, 0.f };
    jug::VECTOR2 m_spawnFeet = { 0.f, 0.f };
    MoveResult   m_lastMove  = {};

    PlayerInputBuffer m_input  = {};
    WeaponState       m_weapon = {};
    PlayerStats       m_stats  = {};

    ePlayerState m_state   = ePlayerState::Idle;
    ePose        m_pose    = ePose::Stand;
    eAim         m_aim     = eAim::Forward;
    eOverlay     m_overlay = eOverlay::None;
    int          m_facing  = 1;

    // 점프
    int  m_airTicks         = 0;      // 공중에서 끝난 연속 틱 수 (지면이면 0)
    bool m_bCoyoteAvailable = true;   // 낭떠러지 코요테 허용 (점프로 떠난 경우 false)
    bool m_bJumping         = false;
    bool m_bJumpCut         = false;
    bool m_bAirFromJump     = true;

    // 겹침 동작
    int      m_shotTicksLeft       = 0;
    int      m_knifeTick           = -1;   // -1: 베기 없음, 0.. : 발동 후 경과 틱
    uint32_t m_knifeHitId          = kNoHitId;
    bool     m_bSlashSecond        = false;
    int      m_throwTick           = -1;
    bool     m_bKnifeTargetInRange = false;
    uint32_t m_overlaySerial       = 0;

    // 세션
    int                   m_lives           = 0;
    int                   m_score           = 0;
    int                   m_invincibleTicks = 0;
    int                   m_deadTicks       = 0;
    bool                  m_bGameOver       = false;
    int                   m_gameOverTicks   = 0;
    int                   m_idleTicks       = 0;
    uint32_t              m_stateSerial     = 0;
    bool                  m_bMoving         = false;   // 실제로 가로 이동 중 (벽에 막히면 false)
    int                   m_moveX           = 0;
    bool                  m_bSlashShown     = false;   // 이번 틱에 칼 동작이 보이는가
    bool                  m_bThrowShown     = false;
    bool                  m_bKnifeEmitted   = false;   // 이번 틱에 칼 판정을 냈는가
    jug::Vector<uint32_t> m_pendingRelease  = {};      // 다음 틱 시작에 풀 HitId (이번 틱 Resolve 가 끝난 뒤)

    // 캐시: 이번 틱에 쓰는 tuning (호출자 소유, 틱 안에서만 유효)
    const Tuning* m_pTuning = nullptr;
};

}   // namespace slug
