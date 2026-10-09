#include "Game/Player/PlayerController.h"
#include "Base/Types.h"

#include <algorithm>
#include <bit>
#include <cmath>
#include <cstring>

#include "Base/Math.h"

#include "Game/Player/PlayerHitboxes.h"

namespace slug
{

namespace
{
    [[nodiscard]] PhysicsTuning MakePhysicsTuning_(
        const MoveTuning& _move)
    {
        PhysicsTuning tuning;
        tuning.gravity          = _move.gravity;
        tuning.terminalVelocity = _move.terminalVelocity;
        tuning.runSpeed         = _move.runSpeed;
        tuning.jumpSpeed        = _move.jumpSpeed;
        tuning.maxStepUp        = _move.maxStepUp;
        tuning.maxSnapDown      = _move.maxSnapDown;
        return tuning;
    }

    [[nodiscard]] bool IsRegionBlocked_(
        const PhysicsWorld& _world,
        const Aabb&         _region)
    {
        if (_region.IsEmpty())
        {
            return false;
        }
        if (_world.pGround && _world.pGround->IsRectSolid(FloorToInt(_region.min.x), FloorToInt(_region.min.y), CeilToInt(_region.max.x) - 1, CeilToInt(_region.max.y) - 1))
        {
            return true;
        }
        for (const Platform& platform: _world.platforms)
        {
            if (!platform.bOneWay && _region.Overlaps(platform.box))
            {
                return true;
            }
        }
        for (const Wall& wall: _world.walls)
        {
            if (_region.Overlaps(wall.box))
            {
                return true;
            }
        }
        return false;
    }

    template<typename T>
    void HashBytes_(
        uint64_t& _hash,
        const T&  _value)
    {
        unsigned char bytes[sizeof(T)];
        std::memcpy(bytes, &_value, sizeof(T));
        for (const unsigned char byte: bytes)
        {
            _hash ^= byte;
            _hash *= 1099511628211ull;
        }
    }
}

void PlayerController::Reset(
    const Tuning&      _tuning,
    const Vec2 _spawnFeet)
{
    m_pTuning = &_tuning;

    m_body               = {};
    m_body.position      = _spawnFeet;
    m_body.size          = _tuning.body.standSize;
    m_body.footHalfWidth = _tuning.body.footHalfWidth;
    m_prevFeet           = _spawnFeet;
    m_spawnFeet          = _spawnFeet;
    m_lastMove           = {};

    m_input.Reset();
    ResetWeapons(m_weapon, _tuning);
    m_stats = {};

    m_state   = ePlayerState::Idle;
    m_pose    = ePose::Stand;
    m_aim     = eAim::Forward;
    m_aimDegree = 0.f;
    m_overlay = eOverlay::None;
    m_facing  = 1;

    m_airTicks         = 0;
    m_bCoyoteAvailable = true;
    m_bJumping         = false;
    m_bAirFromJump     = true;

    m_shotTicksLeft      = 0;
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
    m_pendingRelease.clear();
}

bool PlayerController::SnapToGround(
    const PhysicsWorld& _world,
    const float         _maxDist)
{
    return Physics::SnapToGround(m_body, _world, _maxDist);
}

void PlayerController::PickUpWeapon(
    const eWeapon _weapon,
    const Tuning& _tuning)
{
    GiveWeapon(m_weapon, _weapon, _tuning.weapons.GetPickupAmmo(_weapon));
}

void PlayerController::SetVictory()
{
    if (m_state == ePlayerState::Dead || m_state == ePlayerState::Victory)
    {
        return;
    }
    m_state = ePlayerState::Victory;
    ++m_stateSerial;
    m_overlay       = eOverlay::None;
    m_shotTicksLeft = 0;
    m_throwTick     = -1;
    if (m_knifeTick >= 0)
    {
        m_pendingRelease.push_back(m_knifeHitId);
        m_knifeTick = -1;
    }
}

bool PlayerController::OnHitEvents(
    const slug::Span<const HitEvent> _events)
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
    if (m_state == ePlayerState::Dead || m_state == ePlayerState::Victory || m_invincibleTicks > 0)
    {
        return;
    }

    m_state = ePlayerState::Dead;
    ++m_stateSerial;
    ++m_stats.deaths;
    m_deadTicks       = 0;
    m_overlay         = eOverlay::None;
    m_shotTicksLeft   = 0;
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
    hurtbox.bInvulnerable = IsInvincible();
    return hurtbox;
}

void PlayerController::Tick(
    const PlayerContext& _ctx,
    const InputHistory&  _input)
{
    m_pTuning = _ctx.pTuning;

    if (_ctx.pCombat)
    {
        for (const uint32_t id: m_pendingRelease)
        {
            _ctx.pCombat->ReleaseHitId(id);
        }
    }
    m_pendingRelease.clear();

    m_input.Update(_input);
    m_prevFeet      = m_body.position;
    m_bSlashShown   = false;
    m_bThrowShown   = false;
    TickWeapon(m_weapon);

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
    m_lastMove        = Physics::Step(m_body, _ctx.physics, MakePhysicsTuning_(tuning.move));
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

    ResetWeapons(m_weapon, tuning);
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

    TickMovement_(_ctx, command);
    UpdateAim_(command);
    DeriveState_();
    TickActions_(_ctx, command);

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

void PlayerController::TickMovement_(
    const PlayerContext& _ctx,
    const PlayerCommand& _command)
{
    const Tuning&     tuning = *_ctx.pTuning;
    const MoveTuning& move   = tuning.move;

    const bool bGroundedBefore = m_body.bGrounded;

    bool bCrouch = bGroundedBefore && _command.bDownHeld && m_state != ePlayerState::Respawn;
    if (!bCrouch && m_pose == ePose::Crouch && bGroundedBefore)
    {
        const Aabb headRegion = Aabb::FromMinMax(
            { m_body.position.x - tuning.body.standSize.x * 0.5f, m_body.position.y - tuning.body.standSize.y },
            { m_body.position.x + tuning.body.standSize.x * 0.5f, m_body.position.y - tuning.body.crouchSize.y });
        bCrouch = IsRegionBlocked_(_ctx.physics, headRegion);
    }

    m_moveX = _command.moveX;
    if (m_moveX != 0)
    {
        m_facing = m_moveX;
    }
    const bool  bFiring = _command.bShotHeld || m_shotTicksLeft > 0;
    const float speed   = bCrouch ? (bFiring ? 0.f : move.crouchSpeed) : (bGroundedBefore ? move.runSpeed : move.airSpeed);
    m_body.velocity.x = static_cast<float>(m_moveX) * speed;

    const bool bCoyote = m_bCoyoteAvailable && m_airTicks >= 1 && m_airTicks <= move.coyoteTicks;

    bool bWillLand = false;
    if (_command.bJumpBuffered && !bGroundedBefore && m_body.velocity.y >= 0.f)
    {
        Body probe = m_body;
        bWillLand  = Physics::Step(probe, _ctx.physics, MakePhysicsTuning_(move)).bLanded;
    }

    if (_command.bJumpBuffered && (bGroundedBefore || bCoyote || bWillLand))
    {
        const bool bOnOneWay = bGroundedBefore && _command.bDownHeld && m_lastMove.platformIndex >= 0 && static_cast<size_t>(m_lastMove.platformIndex) < _ctx.physics.platforms.size() && _ctx.physics.platforms[static_cast<size_t>(m_lastMove.platformIndex)].bOneWay;
        if (bOnOneWay)
        {
            m_body.RequestDropThrough();
            m_input.Consume(eAction::Jump);
        }
        else
        {
            const Aabb headRegion = Aabb::FromMinMax(
                { m_body.position.x - tuning.body.standSize.x * 0.5f, m_body.position.y - tuning.body.standSize.y - move.jumpHeadroom },
                { m_body.position.x + tuning.body.standSize.x * 0.5f, m_body.position.y - tuning.body.standSize.y });
            if (!IsRegionBlocked_(_ctx.physics, headRegion))
            {
                m_body.velocity.y  = -move.jumpSpeed;
                m_bJumping         = true;
                m_bCoyoteAvailable = false;
                m_bAirFromJump     = true;
                bCrouch            = false;
                ++m_stats.jumps;
                m_input.Consume(eAction::Jump);
            }
        }
    }

    m_body.size          = bCrouch ? tuning.body.crouchSize : tuning.body.standSize;
    m_body.footHalfWidth = tuning.body.footHalfWidth;
    m_lastMove           = Physics::Step(m_body, _ctx.physics, MakePhysicsTuning_(move));

    if (m_body.bGrounded)
    {
        m_airTicks         = 0;
        m_bCoyoteAvailable = true;
        m_bJumping         = false;
        m_pose            = bCrouch ? ePose::Crouch : ePose::Stand;
    }
    else
    {
        if (bGroundedBefore && !m_bJumping)
        {
            m_bAirFromJump = false;
        }
        ++m_airTicks;
        m_pose = ePose::Air;
    }

    m_bMoving = m_moveX != 0 && !(m_moveX > 0 ? m_lastMove.bHitWallRight : m_lastMove.bHitWallLeft);
}

void PlayerController::UpdateAim_(
    const PlayerCommand& _command)
{
    const bool bHmg        = m_weapon.weapon == eWeapon::Hmg;
    float      targetDeg   = 0.f;
    if (m_pose != ePose::Crouch)
    {
        if (_command.bUpHeld && !_command.bDownHeld)
        {
            targetDeg = 90.f;
        }
        else if (_command.bDownHeld && !_command.bUpHeld && m_pose == ePose::Air)
        {
            targetDeg = -90.f;
        }
    }

    if (bHmg)
    {
        constexpr float kSweepPerTick = 250.f / 60.f;
        constexpr float kReturnPerTick = 1000.f / 60.f;
        const float     step           = m_shotTicksLeft > 0 || _command.bShotHeld ? kSweepPerTick : kReturnPerTick;
        m_aimDegree += std::clamp(targetDeg - m_aimDegree, -step, step);
    }
    else
    {
        m_aimDegree = targetDeg;
    }

    const float absDeg = std::abs(m_aimDegree);
    if (absDeg < 22.5f)
    {
        m_aim = eAim::Forward;
    }
    else if (absDeg < 67.5f)
    {
        m_aim = m_aimDegree > 0.f ? eAim::DiagUp : eAim::DiagDown;
    }
    else
    {
        m_aim = m_aimDegree > 0.f ? eAim::Up : eAim::Down;
    }
}

void PlayerController::DeriveState_()
{
    if (m_state == ePlayerState::Dead || m_state == ePlayerState::Victory)
    {
        return;
    }
    if (m_state == ePlayerState::Respawn && !m_body.bGrounded)
    {
        return;
    }

    if (!m_body.bGrounded)
    {
        m_state = ePlayerState::Jump;
    }
    else if (m_pose == ePose::Crouch)
    {
        m_state = m_bMoving ? ePlayerState::CrouchMove : ePlayerState::Crouch;
    }
    else
    {
        m_state = m_bMoving ? ePlayerState::Run : ePlayerState::Idle;
    }
}

void PlayerController::TickActions_(
    const PlayerContext& _ctx,
    const PlayerCommand& _command)
{
    if (m_state == ePlayerState::Victory || !_ctx.pCombat || !_ctx.pProjectiles)
    {
        m_overlay = eOverlay::None;
        return;
    }

    TickKnife_(_ctx, _command);
    TickBomb_(_ctx, _command);
    TickShot_(_ctx, _command);

    m_overlay = m_bSlashShown ? eOverlay::Slash : (m_bThrowShown ? eOverlay::Throw : (m_shotTicksLeft > 0 ? eOverlay::Shot : eOverlay::None));
    if (m_shotTicksLeft > 0)
    {
        --m_shotTicksLeft;
    }
}

void PlayerController::EndKnife_(
    const PlayerContext&)
{
    if (m_knifeTick >= 0)
    {
        m_pendingRelease.push_back(m_knifeHitId);
    }
    m_knifeTick  = -1;
    m_knifeHitId = kNoHitId;
}

void PlayerController::TickKnife_(
    const PlayerContext& _ctx,
    const PlayerCommand& _command)
{
    const KnifeTuning& knife  = _ctx.pTuning->knife;
    const float        half   = m_body.size.x * 0.5f;
    CombatSystem&      combat = *_ctx.pCombat;

    m_bKnifeTargetInRange = combat.HasTargetIn(eTeam::Player, eHitKind::Melee, MakeKnifeTriggerBox(m_body.position, m_facing, half, knife), kPlayerOwnerId);

    if (m_knifeTick >= knife.durationTicks)
    {
        EndKnife_(_ctx);
    }

    const bool bWant  = _command.bShotBuffered || _command.bShotHeld;
    const bool bChain = m_knifeTick >= 0 && m_knifeTick >= knife.durationTicks - knife.chainWindowTicks;
    if (m_bKnifeTargetInRange && bWant && (m_knifeTick < 0 || bChain))
    {
        EndKnife_(_ctx);
        m_knifeTick    = 0;
        m_knifeHitId   = combat.NewHitId();
        m_bSlashSecond = !m_bSlashSecond;
        ++m_overlaySerial;
        ++m_stats.knifeSlashes;
        m_input.Consume(eAction::Shot);
        m_throwTick = -1;
    }

    if (m_knifeTick >= 0)
    {
        m_bSlashShown = true;
        if (IsKnifeSwingActive(m_knifeTick, knife))
        {
            Hitbox hitbox;
            hitbox.box     = MakeKnifeSwingBox(m_body.position, m_facing, knife);
            hitbox.team    = eTeam::Player;
            hitbox.kind    = eHitKind::Melee;
            hitbox.damage  = knife.damage;
            hitbox.bPierce = true;
            hitbox.owner   = kPlayerOwnerId;
            hitbox.id      = m_knifeHitId;
            combat.AddHitbox(hitbox);
        }
        ++m_knifeTick;
    }
}

void PlayerController::TickBomb_(
    const PlayerContext& _ctx,
    const PlayerCommand& _command)
{
    const BombTuning& bomb = _ctx.pTuning->bomb;

    if (m_throwTick < 0 && _command.bBombBuffered && m_weapon.bombs > 0 && m_knifeTick < 0 && _ctx.pProjectiles->CountBombs(kPlayerOwnerId) < bomb.maxInFlight)
    {
        m_throwTick = 0;
        ++m_overlaySerial;
        m_input.Consume(eAction::Bomb);
    }

    if (m_throwTick >= 0)
    {
        m_bThrowShown = true;
        if (m_throwTick == bomb.releaseTick)
        {
            BombSpawn spawn;
            spawn.team       = eTeam::Player;
            spawn.owner      = kPlayerOwnerId;
            spawn.position   = GetBombReleasePosition(m_body.position, m_facing, m_pose, bomb);
            spawn.velocity   = { static_cast<float>(m_facing) * bomb.speedX + m_body.velocity.x * bomb.inheritMoveX, bomb.speedY };
            spawn.gravity    = bomb.gravity;
            spawn.hitSize    = bomb.hitSize;
            spawn.damage     = bomb.damage;
            spawn.blastSize  = bomb.blastSize;
            spawn.blastUp    = bomb.blastUp;
            spawn.blastTicks = bomb.blastTicks;
            spawn.bBounce    = true;
            spawn.fuseTicks  = bomb.fuseTicks;
            _ctx.pProjectiles->SpawnBomb(spawn);
            --m_weapon.bombs;
            ++m_stats.bombsThrown;
        }
        ++m_throwTick;
        if (m_throwTick >= bomb.throwTicks)
        {
            m_throwTick = -1;
        }
    }
}

void PlayerController::TickShot_(
    const PlayerContext& _ctx,
    const PlayerCommand& _command)
{
    if (m_knifeTick >= 0 || m_throwTick >= 0 || m_bSlashShown || m_bThrowShown)
    {
        return;
    }

    const Tuning&       tuning = *_ctx.pTuning;
    const WeaponTuning& weapon = tuning.weapons.Get(m_weapon.weapon);
    const int           alive  = _ctx.pProjectiles->CountBullets(kPlayerOwnerId, static_cast<uint8_t>(m_weapon.weapon));

    if (!ShouldFire(m_weapon, weapon, alive, _command.bShotBuffered, _command.bShotHeld))
    {
        return;
    }

    const Vec2 muzzle    = GetMuzzlePosition(m_body.position, m_facing, m_pose, m_aim, tuning.muzzle);
    Vec2       direction = GetAimDirection(m_aim, m_facing);
    if (m_weapon.weapon == eWeapon::Hmg)
    {
        const float rad = m_aimDegree * (3.14159265f / 180.f);
        direction       = { static_cast<float>(m_facing) * std::cos(rad), -std::sin(rad) };
    }
    const Vec2 chest     = { m_body.position.x, m_body.position.y - m_body.size.y * 0.6f };

    _ctx.pProjectiles->SpawnBullet(MakeBulletSpawn(m_weapon.weapon, weapon, kPlayerOwnerId, muzzle, chest, direction), *_ctx.pCombat);
    m_shotTicksLeft = weapon.shotOverlayTicks;
    ++m_overlaySerial;
    ++m_stats.shotsFired;
    m_input.Consume(eAction::Shot);
    OnFired(m_weapon);
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

    if (m_pTuning)
    {
        switch (m_overlay)
        {
            case eOverlay::Shot: rig.overlayDurationTicks = m_pTuning->weapons.Get(m_weapon.weapon).shotOverlayTicks; break;
            case eOverlay::Slash: rig.overlayDurationTicks = m_pTuning->knife.durationTicks; break;
            case eOverlay::Throw: rig.overlayDurationTicks = m_pTuning->bomb.throwTicks; break;
            case eOverlay::None: break;
        }

        const int period = slug::Max(1, m_pTuning->player.blinkPeriodTicks);
        if (m_invincibleTicks > 0 && m_state != ePlayerState::Victory)
        {
            rig.bVisible = ((m_invincibleTicks / period) % 2) == 1;
        }
        rig.bIdleAction = m_state == ePlayerState::Idle && m_idleTicks >= m_pTuning->player.idleActionTicks;
    }
    return rig;
}

uint64_t PlayerController::HashState() const
{
    uint64_t hash = 14695981039346656037ull;
    HashBytes_(hash, m_body.position.x);
    HashBytes_(hash, m_body.position.y);
    HashBytes_(hash, m_body.velocity.x);
    HashBytes_(hash, m_body.velocity.y);
    HashBytes_(hash, static_cast<uint8_t>(m_state));
    HashBytes_(hash, static_cast<uint8_t>(m_weapon.weapon));
    HashBytes_(hash, m_weapon.ammo);
    HashBytes_(hash, m_weapon.bombs);
    HashBytes_(hash, m_lives);
    HashBytes_(hash, m_score);
    HashBytes_(hash, m_invincibleTicks);
    HashBytes_(hash, m_facing);
    HashBytes_(hash, m_stats.shotsFired);
    HashBytes_(hash, m_stats.knifeSlashes);
    HashBytes_(hash, m_stats.bombsThrown);
    HashBytes_(hash, m_stats.jumps);
    HashBytes_(hash, m_stats.deaths);
    return hash;
}

void PlayerController::PushFeetX(
    const float _minX)
{
    if (m_body.position.x < _minX)
    {
        m_body.position.x = _minX;
        if (m_body.velocity.x < 0.f)
        {
            m_body.velocity.x = 0.f;
        }
    }
}
}
