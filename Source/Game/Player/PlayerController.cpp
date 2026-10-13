#include "Game/Player/PlayerController.h"

#include <algorithm>

#include "Base/Types.h"
#include "Game/Player/PlayerHitboxes.h"
#include "Render/SpriteBatch.h"
#include "World/Constants.h"

void PlayerController::Reset(
    const Tuning& _tuning,
    const VECTOR2 _spawnFeet)
{
    m_pTuning = &_tuning;

    m_body               = {};
    m_body.position      = _spawnFeet;
    m_body.size          = _tuning.body.standSize;
    m_body.footHalfWidth = _tuning.body.footHalfWidth;
    m_prevFeet           = _spawnFeet;
    m_lastMove           = {};

    m_input.Reset();
    m_weapon.Reset(_tuning);
    m_stats = {};

    m_state     = ePlayerState::Idle;
    m_pose      = ePose::Stand;
    m_aim       = eAim::Forward;
    m_aimDegree = 0.f;
    m_overlay   = eOverlay::None;
    m_facing    = 1;

    m_airTicks         = 0;
    m_bCoyoteAvailable = true;
    m_bJumping         = false;
    m_bAirFromJump     = true;

    m_shotTicksLeft = 0;
    m_burstLeft     = 0;
    m_burstTimer    = 0;
    EndFlame_();
    m_knifeTick           = -1;
    m_knifeHitId          = kNoHitId;
    m_bSlashSecond        = false;
    m_throwTick           = -1;
    m_bKnifeTargetInRange = false;
    m_overlaySerial       = 0;

    m_lives           = _tuning.player.lives;
    m_score           = 0;
    m_invincibleTicks = 0;
    m_deadTicks       = 0;
    m_bGameOver       = false;
    m_gameOverTicks   = 0;
    m_idleTicks       = 0;
    m_stateSerial     = 0;
    m_bMoving         = false;
    m_moveX           = 0;
    m_bSlashShown     = false;
    m_bThrowShown     = false;
    m_bRiding         = false;
    m_pendingRelease.clear();

    m_chutePhase      = eChutePhase::None;
    m_chuteLandFeet   = _spawnFeet;
    m_chuteBlinkTicks = 0;
}

void PlayerController::PickUpWeapon(
    const eWeapon _weapon,
    const Tuning& _tuning)
{
    m_weapon.Give(_weapon, _tuning.weapons.GetPickupAmmo(_weapon));
}

void PlayerController::SetVictory()
{
    if (m_state == ePlayerState::Dead || m_state == ePlayerState::Victory)
    {
        return;
    }
    m_state = ePlayerState::Victory;
    ++m_stateSerial;
    m_chutePhase    = eChutePhase::None;
    m_overlay       = eOverlay::None;
    m_shotTicksLeft = 0;
    m_burstLeft     = 0;
    m_burstTimer    = 0;
    EndFlame_();
    m_throwTick = -1;
    if (m_knifeTick >= 0)
    {
        m_pendingRelease.push_back(m_knifeHitId);
        m_knifeTick = -1;
    }
}

bool PlayerController::OnHitEvents(
    const Span<const HitEvent> _events)
{
    for (const HitEvent& event: _events)
    {
        if (event.target == kPlayerOwnerId && event.attackerTeam != eTeam::Player)
        {
            const bool bWasAlive = m_state != ePlayerState::Dead;
            Kill();
            return bWasAlive && m_state == ePlayerState::Dead;
        }
    }
    return false;
}

void PlayerController::Kill()
{
    if (m_state == ePlayerState::Dead || m_state == ePlayerState::Victory || m_invincibleTicks > 0 || m_chutePhase == eChutePhase::Descend)
    {
        return;
    }
    if (m_godMode != eGodMode::Off)
    {
        return;
    }

    m_state = ePlayerState::Dead;
    ++m_stateSerial;
    ++m_stats.deaths;
    m_deadTicks     = 0;
    m_overlay       = eOverlay::None;
    m_shotTicksLeft = 0;
    m_burstLeft     = 0;
    m_burstTimer    = 0;
    EndFlame_();
    m_throwTick       = -1;
    m_body.velocity.x = 0.f;
    if (m_knifeTick >= 0)
    {
        m_pendingRelease.push_back(m_knifeHitId);
        m_knifeTick = -1;
    }

    --m_lives;
    if (m_lives <= 0)
    {
        m_lives         = 0;
        m_bGameOver     = true;
        m_gameOverTicks = 0;
    }
}

Hurtbox PlayerController::MakeHurtbox(
    const Tuning& _tuning) const
{
    Hurtbox hurtbox;
    hurtbox.box           = MakePlayerHurtboxBox(m_body.position, m_pose == ePose::Crouch ? ePose::Crouch : ePose::Stand, _tuning.body);
    hurtbox.team          = eTeam::Player;
    hurtbox.owner         = kPlayerOwnerId;
    hurtbox.bInvulnerable = IsInvincible() || m_godMode == eGodMode::Invincible;
    return hurtbox;
}

void PlayerController::Tick(
    const PlayerContext& _ctx,
    const InputHistory&  _input)
{
    m_pTuning = _ctx.pTuning;
    m_godMode = _ctx.godMode;
    if (_ctx.bInfiniteAmmo)
    {
        m_weapon.bombs = Max(m_weapon.bombs, 10);
        if (m_weapon.HasFiniteAmmo())
        {
            m_weapon.ammo = Max(m_weapon.ammo, 99);
        }
    }

    if (_ctx.pCombat)
    {
        for (const uint32_t id: m_pendingRelease)
        {
            _ctx.pCombat->ReleaseHitId(id);
        }
    }
    m_pendingRelease.clear();

    m_input.Update(_input);
    m_prevFeet    = m_body.position;
    m_bSlashShown = false;
    m_bThrowShown = false;
    m_weapon.Tick();

    if (m_bRiding)
    {
        if (m_invincibleTicks > 0)
        {
            --m_invincibleTicks;
        }
        return;
    }

    if (m_state == ePlayerState::Dead)
    {
        TickDead_(_ctx);
    }
    else
    {
        TickAlive_(_ctx, _input);
    }

    if (_ctx.pCombat && m_state != ePlayerState::Dead)
    {
        _ctx.pCombat->AddHurtbox(MakeHurtbox(*_ctx.pTuning));
    }
    if (m_invincibleTicks > 0)
    {
        --m_invincibleTicks;
    }
}

void PlayerController::TickDead_(
    const PlayerContext& _ctx)
{
    const Tuning& tuning = *_ctx.pTuning;

    m_body.velocity.x = 0.f;
    m_lastMove        = Physics::Step(m_body, _ctx.physics, MakePhysicsTuning(tuning.move));
    ++m_deadTicks;

    if (m_bGameOver)
    {
        ++m_gameOverTicks;
        return;
    }
    if (m_deadTicks >= tuning.player.deathTicks)
    {
        Respawn_(_ctx);
    }
}

void PlayerController::Respawn_(
    const PlayerContext& _ctx)
{
    const Tuning& tuning = *_ctx.pTuning;

    m_body.position  = _ctx.respawnFeet;
    m_body.velocity  = { 0.f, 0.f };
    m_body.bGrounded = false;
    m_body.size      = tuning.body.standSize;
    m_prevFeet       = m_body.position;
    m_lastMove       = {};

    m_weapon.Reset(tuning);
    m_state = ePlayerState::Respawn;
    ++m_stateSerial;
    m_pose             = ePose::Air;
    m_aim              = eAim::Forward;
    m_aimDegree        = 0.f;
    m_overlay          = eOverlay::None;
    m_invincibleTicks  = tuning.player.respawnInvincible;
    m_airTicks         = 1;
    m_bCoyoteAvailable = false;
    m_bJumping         = false;
    m_bAirFromJump     = false;
    m_deadTicks        = 0;
    m_idleTicks        = 0;
}

void PlayerController::TickAlive_(
    const PlayerContext& _ctx,
    const InputHistory&  _input)
{
    PlayerCommand command = m_input.Read(_input, _ctx.pTuning->move);
    if (m_state == ePlayerState::Victory)
    {
        command = {};
    }

    if (m_chutePhase == eChutePhase::Descend)
    {
        if (!command.bShotBuffered)
        {
            TickChuteDescend_(*_ctx.pTuning);
            return;
        }
        m_input.Consume(eAction::Shot);
        StartChuteRelease_();
        command = {};
    }

    TickMovement_(_ctx, command);
    UpdateAim_(command);
    DeriveState_();
    TickActions_(_ctx, command);

    if (m_chutePhase == eChutePhase::Release)
    {
        ++m_chuteBlinkTicks;
        if (m_chuteBlinkTicks >= _ctx.pTuning->player.chuteBlinkTicks)
        {
            m_chutePhase = eChutePhase::None;
        }
    }

    if (m_state == ePlayerState::Idle && m_overlay == eOverlay::None)
    {
        ++m_idleTicks;
        if (m_idleTicks >= _ctx.pTuning->player.idleActionTicks + _ctx.pTuning->player.idleActionLength)
        {
            m_idleTicks = 0;
        }
    }
    else
    {
        m_idleTicks = 0;
    }
}

RigState PlayerController::MakeRigState() const
{
    RigState rig;
    rig.weapon        = m_weapon.weapon;
    rig.state         = m_state;
    rig.pose          = m_pose;
    rig.bMoving       = m_bMoving;
    rig.facing        = m_facing;
    rig.aim           = m_aim;
    rig.overlay       = m_overlay;
    rig.bSlashSecond  = m_bSlashSecond;
    rig.overlaySerial = m_overlaySerial;
    rig.airTicks      = m_airTicks;
    rig.bAirFromJump  = m_bAirFromJump;
    rig.stateSerial   = m_stateSerial;

    if (m_bRiding)
    {
        rig.bVisible = false;
    }
    if (m_pTuning)
    {
        switch (m_overlay)
        {
            case eOverlay::Shot: rig.overlayDurationTicks = m_shotOverlayTicks; break;
            case eOverlay::Slash: rig.overlayDurationTicks = m_pTuning->knife.durationTicks; break;
            case eOverlay::Throw: rig.overlayDurationTicks = m_pTuning->bomb.throwTicks; break;
            case eOverlay::None: break;
        }

        const int period = Max(1, m_pTuning->player.blinkPeriodTicks);
        if (m_invincibleTicks > 0 && m_state != ePlayerState::Victory && !m_bRiding)
        {
            rig.bVisible = ((m_invincibleTicks / period) % 2) == 1;
        }
        rig.bIdleAction = m_state == ePlayerState::Idle && m_idleTicks >= m_pTuning->player.idleActionTicks;
    }
    return rig;
}

uint64_t PlayerController::HashState() const
{
    Fnv1a hasher;
    hasher.Mix(m_body.position.x);
    hasher.Mix(m_body.position.y);
    hasher.Mix(m_body.velocity.x);
    hasher.Mix(m_body.velocity.y);
    const uint8_t state = static_cast<uint8_t>(m_state);
    hasher.Mix(state);
    const uint8_t weapon = static_cast<uint8_t>(m_weapon.weapon);
    hasher.Mix(weapon);
    hasher.Mix(m_weapon.ammo);
    hasher.Mix(m_weapon.bombs);
    hasher.Mix(m_lives);
    hasher.Mix(m_score);
    hasher.Mix(m_invincibleTicks);
    hasher.Mix(m_facing);
    hasher.Mix(m_stats.shotsFired);
    hasher.Mix(m_burstLeft);
    hasher.Mix(m_stats.knifeSlashes);
    hasher.Mix(m_stats.bombsThrown);
    hasher.Mix(m_stats.jumps);
    hasher.Mix(m_stats.deaths);
    const uint8_t chutePhase = static_cast<uint8_t>(m_chutePhase);
    hasher.Mix(chutePhase);
    hasher.Mix(m_chuteBlinkTicks);
    return hasher.Finalize();
}

void PlayerController::BeginRide()
{
    if (m_state == ePlayerState::Dead || m_state == ePlayerState::Victory)
    {
        return;
    }
    m_bRiding = true;
    ++m_stateSerial;
    m_overlay       = eOverlay::None;
    m_shotTicksLeft = 0;
    m_burstLeft     = 0;
    m_burstTimer    = 0;
    EndFlame_();
    m_throwTick     = -1;
    m_idleTicks     = 0;
    m_body.velocity = { 0.f, 0.f };
    if (m_knifeTick >= 0)
    {
        m_pendingRelease.push_back(m_knifeHitId);
        m_knifeTick = -1;
    }
}

void PlayerController::SetRideFeet(
    const VECTOR2 _feet,
    const int     _facing)
{
    m_body.position = _feet;
    m_prevFeet      = _feet;
    m_facing        = _facing < 0 ? -1 : 1;
}

void PlayerController::EndRide(
    const VECTOR2 _feet,
    const VECTOR2 _velocity,
    const int     _facing,
    const int     _invincibleTicks)
{
    if (!m_bRiding)
    {
        return;
    }
    m_bRiding          = false;
    m_body.position    = _feet;
    m_body.velocity    = _velocity;
    m_body.bGrounded   = false;
    m_body.size        = m_pTuning ? m_pTuning->body.standSize : m_body.size;
    m_prevFeet         = _feet;
    m_facing           = _facing < 0 ? -1 : 1;
    m_state            = ePlayerState::Jump;
    m_pose             = ePose::Air;
    m_airTicks         = 1;
    m_bCoyoteAvailable = false;
    m_bJumping         = true;
    m_bAirFromJump     = true;
    m_idleTicks        = 0;
    ++m_stateSerial;
    ExtendInvincibility(_invincibleTicks);
}

bool PlayerController::IsRiding() const
{
    return m_bRiding;
}

void PlayerController::AddScore(
    const int _score)
{
    m_score += _score;
}

void PlayerController::AddBombs(
    const int _count)
{
    m_weapon.bombs += _count;
}

ePlayerState PlayerController::GetState() const
{
    return m_state;
}

VECTOR2 PlayerController::GetFeet() const
{
    return m_body.position;
}

VECTOR2 PlayerController::GetPrevFeet() const
{
    return m_prevFeet;
}

VECTOR2 PlayerController::GetVelocity() const
{
    return m_body.velocity;
}

int PlayerController::GetFacing() const
{
    return m_facing;
}

bool PlayerController::IsGrounded() const
{
    return m_body.bGrounded;
}

ePose PlayerController::GetPose() const
{
    return m_pose;
}

eAim PlayerController::GetAim() const
{
    return m_aim;
}

eOverlay PlayerController::GetOverlay() const
{
    return m_overlay;
}

const WeaponState& PlayerController::GetWeapon() const
{
    return m_weapon;
}

int PlayerController::GetLives() const
{
    return m_lives;
}

int PlayerController::GetScore() const
{
    return m_score;
}

bool PlayerController::IsAlive() const
{
    return m_state != ePlayerState::Dead;
}

bool PlayerController::IsInvincible() const
{
    return m_invincibleTicks > 0 || m_state == ePlayerState::Victory;
}

int PlayerController::GetInvincibleTicks() const
{
    return m_invincibleTicks;
}

void PlayerController::ExtendInvincibility(
    const int _ticks)
{
    m_invincibleTicks = (_ticks > m_invincibleTicks) ? _ticks : m_invincibleTicks;
}

bool PlayerController::IsGameOver() const
{
    return m_bGameOver;
}

int PlayerController::GetGameOverTicks() const
{
    return m_gameOverTicks;
}

bool PlayerController::IsKnifeInProgress() const
{
    return m_knifeTick >= 0;
}

bool PlayerController::IsMoving() const
{
    return m_bMoving;
}

const PlayerStats& PlayerController::GetStats() const
{
    return m_stats;
}

const Body& PlayerController::GetBody() const
{
    return m_body;
}

Aabb PlayerController::GetBodyBox() const
{
    return m_body.GetBox();
}

bool PlayerController::IsKnifeTargetInRange() const
{
    return m_bKnifeTargetInRange;
}

int PlayerController::GetZ() const
{
    return m_z;
}

void PlayerController::SetZ(
    const int _z)
{
    m_z = _z;
}
