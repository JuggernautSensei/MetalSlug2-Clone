#include "Game/Player/PlayerController.h"

#include <algorithm>
#include <bit>
#include <cmath>

#include "Base/Math.h"
#include "Base/Types.h"
#include "Game/Player/PlayerHitboxes.h"
#include "Render/SpriteBatch.h"
#include "World/Constants.h"

namespace
{
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

}   // namespace

void PlayerController::Reset(
    const Tuning& _tuning,
    const Vec2    _spawnFeet)
{
    m_pTuning = &_tuning;

    m_body               = {};
    m_body.position      = _spawnFeet;
    m_body.size          = _tuning.body.standSize;
    m_body.footHalfWidth = _tuning.body.footHalfWidth;
    m_prevFeet           = _spawnFeet;
    m_lastMove           = {};

    m_input.Reset();
    ResetWeapons(m_weapon, _tuning);
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

bool PlayerController::SnapToGround(
    const PhysicsWorld& _world,
    const float         _maxDist)
{
    return Physics::SnapToGround(m_body, _world, _maxDist);
}

void PlayerController::BeginChuteDescent(
    const Tuning& _tuning)
{
    m_chuteLandFeet = m_body.position;

    m_body.position.y -= _tuning.player.chuteStartHeightPx;
    m_body.velocity  = { 0.f, 0.f };
    m_body.bGrounded = false;
    m_prevFeet       = m_body.position;

    m_chutePhase       = eChutePhase::Descend;
    m_chuteBlinkTicks  = 0;
    m_state            = ePlayerState::Jump;
    m_pose             = ePose::Air;
    m_airTicks         = 1;
    m_bCoyoteAvailable = false;
    m_bJumping         = false;
    m_bAirFromJump     = false;
    m_bMoving          = false;
    m_moveX            = 0;
}

eChutePhase PlayerController::GetChutePhase() const
{
    return m_chutePhase;
}

bool PlayerController::IsChuteVisible(
    const Tuning& _tuning) const
{
    if (m_chutePhase == eChutePhase::Descend)
    {
        return true;
    }
    if (m_chutePhase != eChutePhase::Release)
    {
        return false;
    }
    // 깜빡임: 켜짐 period 틱, 꺼짐 period 틱을 반복. 총 길이가 지나면 Tick 에서 None 이 된다.
    const int period = std::max(1, _tuning.player.chuteBlinkPeriodTicks);
    return (m_chuteBlinkTicks / period) % 2 == 0;
}

Vec2 PlayerController::GetLogicalFeet() const
{
    return m_chutePhase == eChutePhase::Descend ? m_chuteLandFeet : m_body.position;
}

void PlayerController::TickChuteDescend_(
    const Tuning& _tuning)
{
    // 낙하 중에는 물리/충돌을 타지 않는다. 일정한 낮은 속도로 착지점까지 내려간다.
    m_body.position.y += _tuning.player.chuteFallSpeedPxSec * kStepSec;
    m_body.velocity = { 0.f, 0.f };
    m_lastMove      = {};
    m_state         = ePlayerState::Jump;
    m_pose          = ePose::Air;
    m_airTicks      = 1;

    if (m_body.position.y >= m_chuteLandFeet.y)
    {
        m_body.position  = m_chuteLandFeet;
        m_body.bGrounded = true;
        StartChuteRelease_();
    }
}

void PlayerController::StartChuteRelease_()
{
    m_chutePhase      = eChutePhase::Release;
    m_chuteBlinkTicks = 0;
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
    // 낙하 중에는 적/탄에 맞지 않는다 (착지 전 조작 시작 전까지 무적)
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
    TickWeapon(m_weapon);

    if (m_bRiding)
    {
        // 차량 탑승 중: 입력/전투는 SlugVehicle 이 맡는다. 플레이어는 숨고 피격 상자도 내지 않는다.
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

    if (m_chutePhase == eChutePhase::Descend)
    {
        if (!command.bShotBuffered)
        {
            TickChuteDescend_(*_ctx.pTuning);
            return;
        }
        // 낙하 중 공격키: 낙하산을 깜빡이며 떼고 이 틱부터 조작을 연다. 누른 공격키로 발사하지는 않는다.
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
    const bool  bFiring = _command.bShotHeld || m_shotTicksLeft > 0 || m_throwTick >= 0 || _command.bBombBuffered;
    const float speed   = bCrouch ? (bFiring ? 0.f : move.crouchSpeed) : (bGroundedBefore ? move.runSpeed : move.airSpeed);
    m_body.velocity.x   = static_cast<float>(m_moveX) * speed;

    const bool bCoyote = m_bCoyoteAvailable && m_airTicks >= 1 && m_airTicks <= move.coyoteTicks;

    bool bWillLand = false;
    if (_command.bJumpBuffered && !bGroundedBefore && m_body.velocity.y >= 0.f)
    {
        Body probe = m_body;
        bWillLand  = Physics::Step(probe, _ctx.physics, MakePhysicsTuning(move)).bLanded;
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
    m_lastMove           = Physics::Step(m_body, _ctx.physics, MakePhysicsTuning(move));

    if (m_body.bGrounded)
    {
        m_airTicks         = 0;
        m_bCoyoteAvailable = true;
        m_bJumping         = false;
        m_pose             = bCrouch ? ePose::Crouch : ePose::Stand;
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
    const bool bHmg      = m_weapon.weapon == eWeapon::Hmg;
    float      targetDeg = 0.f;
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
        constexpr float kSweepPerTick  = 350.f / 60.f;
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
        EndFlame_();
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
            spawn.z          = ZShift(m_z, kPlayerDepth, kPlayerBombDepth);
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

void PlayerController::CancelBurst_()
{
    // 점프/칼/폭탄으로 버스트가 끊기면 남은 탄을 버리고 사격 연출도 즉시 끝낸다 (이동은 끊지 않는다: 원작은 달리며 연사)
    if (m_burstLeft > 0)
    {
        m_shotTicksLeft = 0;
    }
    m_burstLeft  = 0;
    m_burstTimer = 0;
}

void PlayerController::StopFlameSpawn_()
{
    // 분출 중 점프/칼/폭탄이 끼면 새 입자는 더 나오지 않는다. 이미 나간 입자와 판정 해제 타이밍은 끝까지 진행한다
    if (m_flameTick < 0)
    {
        return;
    }
    m_bFlameStop    = true;
    m_shotTicksLeft = 0;
}

void PlayerController::EndFlame_()
{
    if (m_flameHitId != kNoHitId)
    {
        m_pendingRelease.push_back(m_flameHitId);
    }
    m_flameHitId = kNoHitId;
    m_flameTick  = -1;
    m_bFlameStop = false;
}

void PlayerController::StartFlame_(
    const PlayerContext& _ctx,
    const WeaponTuning&  _flame)
{
    // 입력 1회 = 탄약 1발, 발사 카운트 1. 입자 개수만큼 탄약/카운트/사운드를 쓰지 않는다
    // (분출 길이(fireIntervalTicks) 가 지난 뒤에만 여기 도달하므로, 남은 이전 분출은 이 시점에 판정을 푼다)
    EndFlame_();
    ++m_stats.shotsFired;
    OnFired(m_weapon);
    m_flameRng.Reseed(MakeShotSeed_());
    m_flameHitId = _ctx.pCombat->NewHitId();
    m_flameTick  = 0;
    m_bFlameAir  = m_pose == ePose::Air;
    TickFlame_(_ctx, _flame);
}

void PlayerController::TickFlame_(
    const PlayerContext& _ctx,
    const WeaponTuning&  _flame)
{
    // 분출 타임라인: 0 부터 interval 간격으로 입자 spawnEnd 까지, 판정 ID 는 releaseTicks 마다 풀고, 마지막 입자가 죽으면 끝낸다
    const int interval = Max(1, _flame.particleIntervalTicks);
    const int spawnEnd = Max(0, _flame.particleCount) * interval;
    if (m_flameTick < spawnEnd && m_flameTick % interval == 0 && !m_bFlameStop)
    {
        SpawnFlameParticle_(_ctx, _flame);
    }
    if (_flame.hitReleaseTicks > 0 && m_flameTick > 0 && m_flameTick < spawnEnd && m_flameTick % _flame.hitReleaseTicks == 0)
    {
        _ctx.pCombat->ReleaseHitId(m_flameHitId);
    }
    ++m_flameTick;
    if (m_flameTick > spawnEnd + _flame.particleLifeMaxTicks)
    {
        EndFlame_();
    }
}

void PlayerController::SpawnFlameParticle_(
    const PlayerContext& _ctx,
    const WeaponTuning&  _flame)
{
    const Vec2 muzzle    = MakeMuzzle_(*_ctx.pTuning, _flame);
    const Vec2 direction = GetAimDirection(m_aim, m_facing);
    const Vec2 chest     = { m_body.position.x, m_body.position.y - m_body.size.y * 0.6f };

    BulletSpawn spawn = MakeFlameParticleSpawn(_flame, kPlayerOwnerId, muzzle, chest, direction, m_flameHitId, m_flameRng);
    spawn.z           = ZShift(m_z, kPlayerDepth, kPlayerBulletDepth);
    _ctx.pProjectiles->SpawnBullet(spawn, *_ctx.pCombat);
}

void PlayerController::TickShot_(
    const PlayerContext& _ctx,
    const PlayerCommand& _command)
{
    const Tuning&       tuning = *_ctx.pTuning;
    const WeaponTuning& weapon = tuning.weapons.Get(m_weapon.weapon);

    // 파이어샷 분출은 사격 입력과 별개로 끝까지 돈다 (달리며 쏴도 계속 분출). 점프/칼/폭탄이면 새 입자만 멈춘다
    if (m_flameTick >= 0)
    {
        if (m_knifeTick >= 0 || m_throwTick >= 0 || m_bSlashShown || m_bThrowShown || (m_pose == ePose::Air) != m_bFlameAir)
        {
            StopFlameSpawn_();
        }
        TickFlame_(_ctx, tuning.weapons.flame);
    }

    if (m_knifeTick >= 0 || m_throwTick >= 0 || m_bSlashShown || m_bThrowShown)
    {
        CancelBurst_();
        return;
    }

    // 버스트 진행 중: 공중/지상이 바뀌면 캔슬, 아니면 간격마다 한 발씩 (이동 중에도 남은 발수를 계속 쏜다)
    if (m_burstLeft > 0)
    {
        const bool bAir = m_pose == ePose::Air;
        if (bAir != m_bBurstAir || m_weapon.weapon == eWeapon::Pistol)
        {
            CancelBurst_();
            return;
        }
        if (--m_burstTimer <= 0)
        {
            FireBullet_(_ctx, weapon);
            if (--m_burstLeft > 0)
            {
                m_burstTimer = weapon.burstIntervalTicks;
            }
        }
        return;
    }

    const int alive = _ctx.pProjectiles->CountBullets(kPlayerOwnerId, static_cast<uint8_t>(m_weapon.weapon));
    if (!ShouldFire(m_weapon, weapon, alive, _command.bShotBuffered, _command.bShotHeld))
    {
        return;
    }

    // 파이어샷: 입력 1회 = 입자 분출 시작 (탄약 1발). 사격 모션 길이 = 분출 길이
    if (m_weapon.weapon == eWeapon::Flame)
    {
        StartFlame_(_ctx, weapon);
        m_shotOverlayTicks = weapon.shotOverlayTicks;
        m_shotTicksLeft    = m_shotOverlayTicks;
        ++m_overlaySerial;
        m_input.Consume(eAction::Shot);
        return;
    }

    // 헤비머신건: 한 번 입력 = 3~5발 버스트 (발수는 결정적 난수, 첫 발은 즉시). 사격 모션 길이 = 발수 x 간격
    const bool bBurst = weapon.burstShotsMax > 1 && m_weapon.weapon != eWeapon::Pistol;
    int        shots  = 1;
    if (bBurst)
    {
        m_shotRng.Reseed(MakeShotSeed_());
        shots = Random<int>(m_shotRng, weapon.burstShotsMin, weapon.burstShotsMax);
    }

    FireBullet_(_ctx, weapon);
    m_shotOverlayTicks = bBurst ? shots * weapon.burstIntervalTicks : weapon.shotOverlayTicks;
    m_shotTicksLeft    = m_shotOverlayTicks;
    ++m_overlaySerial;
    m_input.Consume(eAction::Shot);

    if (bBurst && shots > 1)
    {
        m_burstLeft  = shots - 1;
        m_burstTimer = weapon.burstIntervalTicks;
        m_bBurstAir  = m_pose == ePose::Air;
    }
}

uint64_t PlayerController::MakeShotSeed_() const
{
    // 상태만으로 정해지는 시드: 누적 발사 수 + 발 위치 비트 (같은 입력 시퀀스는 항상 같은 탄도)
    uint64_t seed = 0x9E37'79B9'7F4A'7C15ull ^ static_cast<uint64_t>(m_stats.shotsFired);
    seed          = seed * 6'364'136'223'846'793'005ull + std::bit_cast<uint32_t>(m_body.position.x);
    seed          = seed * 6'364'136'223'846'793'005ull + std::bit_cast<uint32_t>(m_body.position.y);
    return seed;
}

Vec2 PlayerController::MakeMuzzle_(
    const Tuning&       _tuning,
    const WeaponTuning& _weapon) const
{
    Vec2 muzzle = GetMuzzlePosition(m_body.position, m_facing, m_pose, m_aim, _tuning.muzzle);
    if (m_aim == eAim::Forward)
    {
        // 정면 사격 클립의 실제 총구 보정 (무기별 튜닝, 왼쪽을 향하면 x 반전)
        muzzle.x += _weapon.muzzleAdjust.x * static_cast<float>(m_facing);
        muzzle.y += _weapon.muzzleAdjust.y;
    }
    return muzzle;
}

void PlayerController::FireBullet_(
    const PlayerContext& _ctx,
    const WeaponTuning&  _weapon)
{
    const Tuning& tuning = *_ctx.pTuning;

    Vec2 muzzle    = MakeMuzzle_(tuning, _weapon);
    Vec2 direction = GetAimDirection(m_aim, m_facing);
    if (m_weapon.weapon == eWeapon::Hmg)
    {
        const float rad = m_aimDegree * (3.14159265f / 180.f);
        direction       = { static_cast<float>(m_facing) * std::cos(rad), -std::sin(rad) };
        if (_weapon.muzzleJitter.y > 0.f)
        {
            const float offset = Random<float>(m_shotRng) * (_weapon.muzzleJitter.y - -_weapon.muzzleJitter.y) + -_weapon.muzzleJitter.y;
            muzzle.x += -direction.y * offset;
            muzzle.y += direction.x * offset;
        }
    }
    const Vec2 chest = { m_body.position.x, m_body.position.y - m_body.size.y * 0.6f };

    BulletSpawn spawn = MakeBulletSpawn(m_weapon.weapon, _weapon, kPlayerOwnerId, muzzle, chest, direction);
    spawn.z           = ZShift(m_z, kPlayerDepth, kPlayerBulletDepth);
    _ctx.pProjectiles->SpawnBullet(spawn, *_ctx.pCombat);
    ++m_stats.shotsFired;
    OnFired(m_weapon);
    if (m_weapon.weapon == eWeapon::Pistol)
    {
        m_burstLeft = 0;
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
    uint64_t hash = 14'695'981'039'346'656'037ull;
    HashBytes(hash, m_body.position.x);
    HashBytes(hash, m_body.position.y);
    HashBytes(hash, m_body.velocity.x);
    HashBytes(hash, m_body.velocity.y);
    HashBytes(hash, static_cast<uint8_t>(m_state));
    HashBytes(hash, static_cast<uint8_t>(m_weapon.weapon));
    HashBytes(hash, m_weapon.ammo);
    HashBytes(hash, m_weapon.bombs);
    HashBytes(hash, m_lives);
    HashBytes(hash, m_score);
    HashBytes(hash, m_invincibleTicks);
    HashBytes(hash, m_facing);
    HashBytes(hash, m_stats.shotsFired);
    HashBytes(hash, m_burstLeft);
    HashBytes(hash, m_stats.knifeSlashes);
    HashBytes(hash, m_stats.bombsThrown);
    HashBytes(hash, m_stats.jumps);
    HashBytes(hash, m_stats.deaths);
    HashBytes(hash, static_cast<uint8_t>(m_chutePhase));
    HashBytes(hash, m_chuteBlinkTicks);
    return hash;
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
    const Vec2 _feet,
    const int  _facing)
{
    m_body.position = _feet;
    m_prevFeet      = _feet;
    m_facing        = _facing < 0 ? -1 : 1;
}

void PlayerController::EndRide(
    const Vec2 _feet,
    const Vec2 _velocity,
    const int  _facing,
    const int  _invincibleTicks)
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

Vec2 PlayerController::GetFeet() const
{
    return m_body.position;
}

Vec2 PlayerController::GetPrevFeet() const
{
    return m_prevFeet;
}

Vec2 PlayerController::GetVelocity() const
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
