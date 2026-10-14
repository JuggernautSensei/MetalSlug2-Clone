#include "Game/Player/PlayerController.h"

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
    const int period = Max(1, _tuning.player.chuteBlinkPeriodTicks);
    return (m_chuteBlinkTicks / period) % 2 == 0;
}

VECTOR2 PlayerController::GetLogicalFeet() const
{
    return m_chutePhase == eChutePhase::Descend ? m_chuteLandFeet : m_body.position;
}

void PlayerController::TickChuteDescend_(
    const Tuning& _tuning)
{
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
        m_aimDegree += Clamp(targetDeg - m_aimDegree, -step, step);
    }
    else
    {
        m_aimDegree = targetDeg;
    }

    const float absDeg = Abs(m_aimDegree);
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
