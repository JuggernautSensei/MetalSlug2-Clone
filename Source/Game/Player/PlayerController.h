#pragma once
#include "Base/Math.h"
#include "Base/Random.h"
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
    const Tuning*     pTuning      = nullptr;
    PhysicsWorld      physics      = {};
    CombatSystem*     pCombat      = nullptr;
    ProjectileSystem* pProjectiles = nullptr;
    Vec2              respawnFeet  = { 0.f, 0.f };   // 리스폰 시 나타날 발 위치 (카메라 기준으로 호출자가 계산)
    eGodMode          godMode      = eGodMode::Off;
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

// 시작 낙하산 단계. None: 일반 조작, Descend: 낙하 중 (공격키 외 입력 잠금), Release: 낙하산 깜빡임 소멸 중 (조작 가능).
enum class eChutePhase : uint8_t
{
    None,
    Descend,
    Release,
};

class PlayerController
{
public:
    void Reset(const Tuning& _tuning, Vec2 _spawnFeet);

    bool SnapToGround(const PhysicsWorld& _world, float _maxDist);

    // 스테이지 시작: 현재 발 위치를 착지점으로 두고 chuteStartHeightPx 만큼 위에서 낙하를 시작한다 (SnapToGround 뒤에 호출).
    void BeginChuteDescent(const Tuning& _tuning);

    void Tick(const PlayerContext& _ctx, const InputHistory& _input);

    bool OnHitEvents(Span<const HitEvent> _events);
    void Kill();

    void AddScore(int _score);
    void PickUpWeapon(eWeapon _weapon, const Tuning& _tuning);
    void AddBombs(int _count);
    void SetVictory();

    [[nodiscard]] eChutePhase GetChutePhase() const;
    [[nodiscard]] bool        IsChuteVisible(const Tuning& _tuning) const;
    // 카메라/월드 로직용 발 위치. 낙하 중에는 착지 지점을 준다 (화면이 지면 기준으로 고정되도록).
    [[nodiscard]] Vec2 GetLogicalFeet() const;

    // 차량(SV-001) 탑승: 탑승 중에는 입력/물리/피격이 차량 쪽으로 넘어가고 플레이어는 숨는다.
    void BeginRide();
    void SetRideFeet(Vec2 _feet, int _facing);
    void EndRide(Vec2 _feet, Vec2 _velocity, int _facing, int _invincibleTicks);
    [[nodiscard]] bool IsRiding() const;

    void PushFeetX(float _minX);
    void PushFeetXMax(float _maxX);
    void ExtendInvincibility(int _ticks);

    [[nodiscard]] ePlayerState       GetState() const;
    [[nodiscard]] Vec2               GetFeet() const;
    [[nodiscard]] Vec2               GetPrevFeet() const;
    [[nodiscard]] int                GetZ() const;   // 그리기 depth (kDefaultZ: kPlayerDepth). 탄/폭탄은 같은 이동량을 따른다
    void                             SetZ(int _z);
    [[nodiscard]] Vec2               GetVelocity() const;
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
    void FireBullet_(const PlayerContext& _ctx, const WeaponTuning& _weapon);
    [[nodiscard]] Vec2 MakeMuzzle_(const Tuning& _tuning, const WeaponTuning& _weapon) const;
    void CancelBurst_();
    void StartFlame_(const PlayerContext& _ctx, const WeaponTuning& _flame);
    void TickFlame_(const PlayerContext& _ctx, const WeaponTuning& _flame);
    void SpawnFlameParticle_(const PlayerContext& _ctx, const WeaponTuning& _flame);
    void StopFlameSpawn_();
    void EndFlame_();
    // 사격 입력 한 번마다 시뮬레이션 상태(발사 수, 발 위치)에서 시드를 만든다 (리플레이/해시 결정성 유지)
    [[nodiscard]] uint64_t MakeShotSeed_() const;
    void Respawn_(const PlayerContext& _ctx);
    void UpdateAim_(const PlayerCommand& _command);
    void DeriveState_();
    void EndKnife_(const PlayerContext& _ctx);
    void TickChuteDescend_(const Tuning& _tuning);
    void StartChuteRelease_();

    Body       m_body      = {};
    Vec2       m_prevFeet  = { 0.f, 0.f };
    int        m_z         = kDefaultZ;
    Vec2       m_spawnFeet = { 0.f, 0.f };
    MoveResult m_lastMove  = {};

    eChutePhase m_chutePhase      = eChutePhase::None;
    Vec2        m_chuteLandFeet   = { 0.f, 0.f };   // 낙하가 끝나는 착지 발 위치
    int         m_chuteBlinkTicks = 0;              // Release 경과 틱

    PlayerInputBuffer m_input  = {};
    WeaponState       m_weapon = {};
    PlayerStats       m_stats  = {};

    ePlayerState m_state     = ePlayerState::Idle;
    ePose        m_pose      = ePose::Stand;
    eAim         m_aim       = eAim::Forward;
    float        m_aimDegree = 0.f;
    eOverlay     m_overlay   = eOverlay::None;
    int          m_facing    = 1;

    int  m_airTicks         = 0;   // 공중에서 끝난 연속 틱 수 (지면이면 0)
    bool m_bCoyoteAvailable = true;
    bool m_bJumping         = false;
    bool m_bAirFromJump     = true;

    int      m_shotTicksLeft       = 0;
    int      m_shotOverlayTicks    = 0;       // 현재 사격 상체 동작의 전체 길이 (버스트: 발수 x 간격)
    RngMt19937 m_shotRng           = {};      // 탄 흩뿌림/발수 난수 (버스트 시작마다 MakeShotSeed_ 로 재시드)
    int      m_burstLeft           = 0;       // 연속 사격(버스트)에서 아직 나가지 않은 발수
    int      m_burstTimer          = 0;       // 다음 버스트 탄까지 남은 틱
    bool     m_bBurstAir           = false;   // 버스트를 시작할 때 공중이었는가 (착지/이륙하면 캔슬)
    int        m_flameTick         = -1;      // 파이어샷 분출 경과 틱 (-1 = 분출 없음). 입자 스폰/판정 해제/종료의 기준
    bool       m_bFlameStop        = false;   // 점프/칼/폭탄으로 끊김: 새 입자는 더 나오지 않는다
    bool       m_bFlameAir         = false;   // 분출을 시작할 때 공중이었는가 (지상<->공중 전환이면 끊는다)
    uint32_t   m_flameHitId        = kNoHitId;   // 분출 입자들이 공유하는 판정 ID (releaseTicks 마다 풀려 대상이 다시 맞는다)
    RngMt19937 m_flameRng          = {};      // 입자 방향/속도/수명 난수 (분출마다 MakeShotSeed_ 로 재시드)
    int      m_knifeTick           = -1;   // -1: 베기 없음, 0.. : 발동 후 경과 틱
    uint32_t m_knifeHitId          = kNoHitId;
    bool     m_bSlashSecond        = false;
    int      m_throwTick           = -1;
    bool     m_bKnifeTargetInRange = false;
    uint32_t m_overlaySerial       = 0;

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
    eGodMode      m_godMode = eGodMode::Off;   // 에디터 디버그 (해시 제외)
};
