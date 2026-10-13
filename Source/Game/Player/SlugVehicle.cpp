#include "Game/Player/SlugVehicle.h"

#include <algorithm>
#include <cmath>

#include "Base/Math.h"
#include "Base/Types.h"
#include "Game/Combat/HitFlash.h"
#include "Game/Player/SlugClips.h"
#include "Game/Player/SlugRender.h"
#include "Render/SpriteBatch.h"

constexpr int kCrouchLevels   = 8;
constexpr int kExplosionTicks = 66;
constexpr int kFxSmokeTicks   = 54;
constexpr int kFxFireTicks    = 40;
constexpr int kSmokeInterval  = 16;
constexpr int kFireInterval   = 11;
constexpr int kHitAgeNone     = -1;

namespace
{
[[nodiscard]] float WrapDegree_(
    const float _degree)
{
    float value = std::fmod(_degree + 180.f, 360.f);
    if (value < 0.f)
    {
        value += 360.f;
    }
    return value - 180.f;
}

[[nodiscard]] VECTOR2 GetDamageFxOffset_(
    const int _index)
{
    static constexpr ARRAY<VECTOR2, 4> kOffsets = {
        {
         { -30.f, -78.f },
         { 18.f, -84.f },
         { -8.f, -66.f },
         { 34.f, -70.f },
         }
    };
    return kOffsets[static_cast<size_t>(_index) % kOffsets.size()];
}
}   // namespace

void SlugVehicle::Init(
    const SlugTuning& _tuning,
    const VECTOR2     _feet,
    const int         _facing,
    const uint32_t    _ownerId,
    const int         _hp)
{
    m_tuning  = _tuning;
    m_ownerId = _ownerId;

    m_mode      = eSlugMode::Parked;
    m_modeTicks = 0;
    m_tick      = 0;

    m_body               = {};
    m_body.position      = _feet;
    m_body.size          = { _tuning.body.standSize.x, _tuning.body.crouchSize.y };
    m_body.footHalfWidth = _tuning.body.footHalfWidth;
    m_prevFeet           = _feet;
    m_lastMove           = {};
    m_facing             = _facing < 0 ? -1 : 1;

    m_input.Reset();

    m_airTicks    = 0;
    m_bCoyote     = true;
    m_bJumping    = false;
    m_landTicks   = 0;
    m_crouchLevel = kCrouchLevels;
    m_crouchTimer = 0;
    m_moveX       = 0;
    m_driveFrame  = 0.f;
    m_accelDir    = 0;
    m_accelPhase  = 0.f;

    m_vulcanAngle    = m_facing > 0 ? 0.f : 180.f;
    m_vulcanTarget   = m_vulcanAngle;
    m_vulcanCooldown = 0;
    m_vulcanShotAge  = 1000;
    m_cannonCooldown = 0;
    m_cannonAmmo     = _tuning.cannon.ammoStart;

    m_hp                = _hp >= 0 ? Min(_hp, _tuning.damage.maxHp) : _tuning.damage.maxHp;
    m_flashTicks        = 0;
    m_invincibleTicks   = 0;
    m_hitAge            = kHitAgeNone;
    m_bHitFront         = true;
    m_exitCooldown      = 0;
    m_parkAnimTicks     = 0;
    m_selfDestructTicks = -1;
    m_bExploded         = false;
    m_blastTicksLeft    = 0;
    m_blastHitId        = kNoHitId;
    m_bReleaseCrush     = false;

    m_crushHitId = kNoHitId;
    m_crushTimer = 0;

    m_pose      = eSlugPose::ParkedOpen;
    m_poseFrame = 0;

    m_eject = {};
    m_stats = {};
    m_fx.clear();
}

bool SlugVehicle::CanBoard() const
{
    return m_mode == eSlugMode::Parked && m_exitCooldown <= 0 && m_selfDestructTicks < 0 && m_hp > 0;
}

void SlugVehicle::BeginBoarding()
{
    m_mode      = eSlugMode::Boarding;
    m_modeTicks = 0;
    m_input.Reset();
    m_moveX             = 0;
    m_body.velocity.x   = 0.f;
    m_crouchLevel       = kCrouchLevels;
    m_crouchTimer       = 0;
    m_vulcanCooldown    = 0;
    m_cannonCooldown    = 0;
    m_parkAnimTicks     = 0;
    m_selfDestructTicks = -1;
}

void SlugVehicle::Tick(
    const SlugContext&  _ctx,
    const InputHistory& _input)
{
    ++m_tick;
    m_godMode  = _ctx.godMode;
    m_prevFeet = m_body.position;

    if (m_bReleaseCrush)
    {
        ReleaseCrushId_(_ctx);
        m_bReleaseCrush = false;
    }

    switch (m_mode)
    {
        case eSlugMode::Parked: TickParked_(_ctx); break;
        case eSlugMode::Boarding: TickBoarding_(_ctx); break;
        case eSlugMode::Driven: TickDriven_(_ctx, _input); break;
        case eSlugMode::Exploding: TickExploding_(_ctx); break;
        case eSlugMode::Gone: break;
    }

    TickCommon_(_ctx);
}

void SlugVehicle::TickCommon_(
    const SlugContext& _ctx)
{
    TickHitFlash(m_flashTicks);
    if (m_invincibleTicks > 0)
    {
        --m_invincibleTicks;
    }
    if (m_hitAge != kHitAgeNone)
    {
        ++m_hitAge;
        const eSlugClip clip  = m_bHitFront ? eSlugClip::HitFront : eSlugClip::HitBack;
        const int       ticks = GetSlugClipInfo(clip).frames * 60 / Max(1, GetSlugClipInfo(clip).fps);
        if (m_hitAge >= ticks)
        {
            m_hitAge = kHitAgeNone;
        }
    }
    if (m_vulcanShotAge < 1000)
    {
        ++m_vulcanShotAge;
    }
    if (m_vulcanCooldown > 0)
    {
        --m_vulcanCooldown;
    }
    if (m_cannonCooldown > 0)
    {
        --m_cannonCooldown;
    }
    if (m_exitCooldown > 0)
    {
        --m_exitCooldown;
    }

    UpdateDamageFx_();
    for (SlugFx& fx: m_fx)
    {
        ++fx.ageTicks;
    }
    std::erase_if(m_fx, [](const SlugFx& _fx) { return _fx.ageTicks >= _fx.maxTicks; });

    UpdatePose_();

    if (_ctx.pCombat && m_mode != eSlugMode::Gone && m_mode != eSlugMode::Exploding)
    {
        _ctx.pCombat->AddHurtbox(MakeHurtbox());
    }
}

void SlugVehicle::TickParked_(
    const SlugContext& _ctx)
{
    const PlayerCommand idle = {};
    m_moveX                  = 0;
    m_body.velocity.x        = 0.f;
    UpdateCrouch_(true);
    Move_(_ctx, idle);

    if (m_parkAnimTicks > 0)
    {
        --m_parkAnimTicks;
    }
    ReleaseCrushId_(_ctx);

    // 위험 상태로 버려진 차량은 스스로 폭발한다
    const bool bDanger = m_hp > 0 && GetHpRatio() <= m_tuning.damage.fireRatio;
    if (bDanger && m_selfDestructTicks < 0 && m_exitCooldown <= 0 && m_parkAnimTicks <= 0 && m_stats.exits > 0)
    {
        m_selfDestructTicks = 0;
    }
    if (m_selfDestructTicks >= 0)
    {
        ++m_selfDestructTicks;
        if (m_selfDestructTicks >= m_tuning.damage.selfDestructTicks)
        {
            BeginDestroy_(false);
        }
    }
}

void SlugVehicle::TickBoarding_(
    const SlugContext& _ctx)
{
    ++m_modeTicks;
    const PlayerCommand idle = {};
    m_moveX                  = 0;
    m_body.velocity.x        = 0.f;
    Move_(_ctx, idle);
    if (m_modeTicks >= m_tuning.board.boardTicks)
    {
        m_mode      = eSlugMode::Driven;
        m_modeTicks = 0;
        m_input.Reset();
    }
}

void SlugVehicle::TickDriven_(
    const SlugContext&  _ctx,
    const InputHistory& _input)
{
    ++m_modeTicks;
    m_input.Update(_input);
    const PlayerCommand command = m_input.Read(_input, *_ctx.pInputMove);

    if (command.bUpHeld && command.bJumpBuffered)
    {
        m_input.Consume(eAction::Jump);
        BeginExit_(_ctx);
        return;
    }

    UpdateCrouch_(m_body.bGrounded && command.bDownHeld);
    Move_(_ctx, command);
    UpdateVulcan_(_ctx, command);
    UpdateCannon_(_ctx, command);
    UpdateCrush_(_ctx);
}

void SlugVehicle::TickExploding_(
    const SlugContext& _ctx)
{
    ++m_modeTicks;
    m_body.velocity.x *= 0.9f;
    const PlayerCommand idle = {};
    Move_(_ctx, idle);

    if (!m_bExploded && m_modeTicks >= m_tuning.damage.explodeDelayTicks)
    {
        m_bExploded = true;
        ++m_stats.explosions;
        SpawnFx_(eSlugFxKind::Explosion, m_body.position, 0, kExplosionTicks);
        SpawnFx_(eSlugFxKind::Debris, GetVulcanMount(), 0, 36);
        m_blastTicksLeft = m_tuning.damage.explosionBlastTicks;
        if (_ctx.pCombat)
        {
            m_blastHitId = _ctx.pCombat->NewHitId();
        }
    }

    if (m_blastTicksLeft > 0 && _ctx.pCombat)
    {
        Hitbox hitbox;
        hitbox.box     = Aabb::FromBottomCenter({ m_body.position.x, m_body.position.y + 20.f }, m_tuning.damage.explosionBlast);
        hitbox.team    = eTeam::Player;
        hitbox.kind    = eHitKind::Blast;
        hitbox.damage  = m_tuning.damage.explosionDamage;
        hitbox.bPierce = true;
        hitbox.owner   = m_ownerId;
        hitbox.id      = m_blastHitId;
        _ctx.pCombat->AddHitbox(hitbox);
        --m_blastTicksLeft;
        if (m_blastTicksLeft == 0)
        {
            _ctx.pCombat->ReleaseHitId(m_blastHitId);
            m_blastHitId = kNoHitId;
        }
    }

    if (m_bExploded && m_blastTicksLeft <= 0 && m_modeTicks >= m_tuning.damage.explodeDelayTicks + kExplosionTicks)
    {
        m_mode = eSlugMode::Gone;
    }
}

void SlugVehicle::UpdateCrouch_(
    const bool _bWantCrouch)
{
    const int target = _bWantCrouch ? kCrouchLevels : 0;
    if (m_crouchLevel == target)
    {
        m_crouchTimer = 0;
        return;
    }
    ++m_crouchTimer;
    if (m_crouchTimer >= Max(1, m_tuning.move.crouchStepTicks))
    {
        m_crouchTimer = 0;
        m_crouchLevel += m_crouchLevel < target ? 1 : -1;
    }
}

void SlugVehicle::Move_(
    const SlugContext&   _ctx,
    const PlayerCommand& _command)
{
    const SlugMoveTuning& move      = m_tuning.move;
    const bool            bGrounded = m_body.bGrounded;

    m_moveX = _command.moveX;
    if (m_moveX != 0)
    {
        m_facing = m_moveX;
    }

    const float maxSpeed = m_crouchLevel >= kCrouchLevels / 2 ? move.crouchSpeed : (bGrounded ? move.runSpeed : move.airSpeed);
    const float target   = static_cast<float>(m_moveX) * maxSpeed;
    float&      vx       = m_body.velocity.x;
    if (bGrounded)
    {
        const bool  bReversing = target == 0.f || (vx != 0.f && ((target > 0.f) != (vx > 0.f)));
        const float rate       = move.runSpeed / static_cast<float>(Max(1, bReversing ? move.brakeTicks : move.accelTicks)) * kStepSec * 60.f;
        const float diff       = target - vx;
        vx += Clamp(diff, -rate, rate);
        m_accelDir = Abs(target) > Abs(vx) + 0.01f ? 1 : (Abs(target) < Abs(vx) - 0.01f ? -1 : 0);
    }
    else
    {
        vx         = target;
        m_accelDir = 0;
    }
    m_accelPhase = Clamp(Abs(vx) / Max(1.f, move.runSpeed) * 3.99f, 0.f, 3.99f);

    const bool bCoyote = m_bCoyote && m_airTicks >= 1 && m_airTicks <= move.coyoteTicks;
    if (m_mode == eSlugMode::Driven && _command.bJumpBuffered && (bGrounded || bCoyote))
    {
        m_body.velocity.y = -move.jumpSpeed;
        m_bJumping        = true;
        m_bCoyote         = false;
        m_landTicks       = 0;
        ++m_stats.jumps;
        m_input.Consume(eAction::Jump);
    }

    const float t        = static_cast<float>(m_crouchLevel) / static_cast<float>(kCrouchLevels);
    m_body.size          = { m_tuning.body.standSize.x, Lerp(m_tuning.body.standSize.y, m_tuning.body.crouchSize.y, t) };
    m_body.footHalfWidth = m_tuning.body.footHalfWidth;
    m_lastMove           = Physics::Step(m_body, _ctx.physics, MakePhysicsTuning(move));

    if (m_body.bGrounded)
    {
        m_airTicks = 0;
        m_bCoyote  = true;
        m_bJumping = false;
        if (m_lastMove.bLanded)
        {
            m_landTicks = move.landTicks;
        }
        else if (m_landTicks > 0)
        {
            --m_landTicks;
        }
    }
    else
    {
        ++m_airTicks;
        m_landTicks = 0;
    }

    if (Abs(vx) > 1.f)
    {
        m_driveFrame += Abs(vx) / Max(1.f, move.runSpeed) * static_cast<float>(GetSlugClipInfo(eSlugClip::Drive).fps) / 60.f;
        const float n = static_cast<float>(GetSlugClipInfo(eSlugClip::Drive).frames);
        if (m_driveFrame >= n)
        {
            m_driveFrame -= n;
        }
    }
}

void SlugVehicle::UpdateVulcan_(
    const SlugContext&   _ctx,
    const PlayerCommand& _command)
{
    const bool bLow  = m_crouchLevel >= kCrouchLevels / 2;
    const int  dirX  = _command.moveX != 0 ? _command.moveX : m_facing;
    const bool bUp   = _command.bUpHeld && !bLow;
    const bool bDown = _command.bDownHeld && !m_body.bGrounded;

    float target = dirX > 0 ? 0.f : 180.f;
    if (bUp && !bDown)
    {
        target = _command.moveX != 0 ? (dirX > 0 ? 45.f : 135.f) : 90.f;
    }
    else if (bDown && !bUp)
    {
        target = _command.moveX != 0 ? (dirX > 0 ? -45.f : -135.f) : -90.f;
    }
    m_vulcanTarget = target;

    const float step = m_tuning.vulcan.sweepDegPerSec / 60.f;
    const float diff = WrapDegree_(target - m_vulcanAngle);
    if (Abs(diff) <= step)
    {
        m_vulcanAngle = target;
    }
    else
    {
        m_vulcanAngle += diff > 0.f ? step : -step;
    }
    m_vulcanAngle = WrapDegree_(m_vulcanAngle);

    const bool bWantFire = _command.bShotHeld || _command.bShotBuffered;
    if (bWantFire && m_vulcanCooldown <= 0)
    {
        if (m_tuning.vulcan.maxAlive <= 0 || _ctx.pProjectiles->CountBullets(m_ownerId, 1) < m_tuning.vulcan.maxAlive)
        {
            FireVulcan_(_ctx);
            m_input.Consume(eAction::Shot);
        }
    }
}

void SlugVehicle::FireVulcan_(
    const SlugContext& _ctx)
{
    if (!_ctx.pProjectiles || !_ctx.pCombat)
    {
        return;
    }
    const float   rad = m_vulcanAngle * kDeg2Rad;
    const VECTOR2 dir = { std::cos(rad), -std::sin(rad) };

    BulletSpawn spawn;
    spawn.team      = eTeam::Player;
    spawn.owner     = m_ownerId;
    spawn.position  = GetVulcanMuzzle();
    spawn.sweepFrom = GetVulcanMount();
    spawn.direction = dir;
    spawn.speed     = m_tuning.vulcan.bulletSpeed;
    spawn.damage    = m_tuning.vulcan.damage;
    spawn.size      = m_tuning.vulcan.bulletSize;
    spawn.range     = m_tuning.vulcan.bulletRange;
    spawn.bPierce   = false;
    spawn.tag       = 1;
    spawn.kind      = eHitKind::Bullet;
    spawn.z         = ZShift(m_z, kSlugBodyDepth, kPlayerBulletDepth);
    _ctx.pProjectiles->SpawnBullet(spawn, *_ctx.pCombat);

    m_vulcanCooldown = m_tuning.vulcan.fireIntervalTicks;
    m_vulcanShotAge  = 0;
    ++m_stats.vulcanShots;
}

void SlugVehicle::UpdateCannon_(
    const SlugContext&   _ctx,
    const PlayerCommand& _command)
{
    if (!_command.bBombBuffered)
    {
        return;
    }
    if (m_cannonCooldown > 0)
    {
        return;
    }
    if (m_cannonAmmo <= 0)
    {
        m_input.Consume(eAction::Bomb);
        return;
    }
    FireCannon_(_ctx, _command);
    m_input.Consume(eAction::Bomb);
}

void SlugVehicle::FireCannon_(
    const SlugContext&   _ctx,
    const PlayerCommand& _command)
{
    if (!_ctx.pProjectiles)
    {
        return;
    }
    const SlugCannonTuning& cannon = m_tuning.cannon;

    float vy      = -cannon.riseSpeed;
    int   variant = 0;
    if (_command.bUpHeld && m_crouchLevel < kCrouchLevels / 2)
    {
        vy      = -(cannon.riseSpeed + cannon.upBoost);
        variant = 4;
    }
    else if (_command.bDownHeld && !m_body.bGrounded)
    {
        vy      = cannon.riseSpeed;
        variant = 1;
    }
    if (!m_body.bGrounded && variant == 0)
    {
        variant = 5;
    }
    if (m_crouchLevel >= kCrouchLevels / 2)
    {
        variant = 6;
    }

    BombSpawn spawn;
    spawn.team       = eTeam::Player;
    spawn.owner      = m_ownerId;
    spawn.position   = GetCannonMuzzle();
    spawn.velocity   = { static_cast<float>(m_facing) * cannon.speed, vy };
    spawn.gravity    = cannon.gravity;
    spawn.hitSize    = cannon.hitSize;
    spawn.damage     = cannon.damage;
    spawn.blastSize  = cannon.blastSize;
    spawn.blastUp    = cannon.blastUp;
    spawn.blastTicks = cannon.blastTicks;
    spawn.bBounce    = false;
    spawn.fuseTicks  = 0;
    spawn.z          = ZShift(m_z, kSlugBodyDepth, kPlayerBombDepth);
    _ctx.pProjectiles->SpawnBomb(spawn);

    SpawnFx_(eSlugFxKind::CannonMuzzle, spawn.position, variant, cannon.muzzleFlashTicks);

    --m_cannonAmmo;
    m_cannonCooldown = cannon.cooldownTicks;
    ++m_stats.cannonShots;
}

void SlugVehicle::UpdateCrush_(
    const SlugContext& _ctx)
{
    const SlugCrushTuning& crush = m_tuning.crush;
    if (!_ctx.pCombat)
    {
        return;
    }
    const bool bActive = m_body.bGrounded && Abs(m_body.velocity.x) >= crush.minSpeed && m_moveX != 0 && m_crouchLevel < kCrouchLevels / 2;
    if (!bActive)
    {
        ReleaseCrushId_(_ctx);
        m_crushTimer = 0;
        return;
    }

    if (m_crushHitId == kNoHitId || m_crushTimer >= crush.rehitTicks)
    {
        ReleaseCrushId_(_ctx);
        m_crushHitId = _ctx.pCombat->NewHitId();
        m_crushTimer = 0;
    }
    ++m_crushTimer;

    const float half   = m_tuning.body.hurtStand.x * 0.5f;
    const float height = m_body.size.y;
    const float frontX = m_body.position.x + static_cast<float>(m_facing) * half;
    const float x0     = m_facing > 0 ? frontX - 28.f : frontX - crush.frontReach;
    const float x1     = m_facing > 0 ? frontX + crush.frontReach : frontX + 28.f;

    Hitbox hitbox;
    hitbox.box     = Aabb::FromMinMax({ x0, m_body.position.y - height * 0.9f }, { x1, m_body.position.y - 2.f });
    hitbox.team    = eTeam::Player;
    hitbox.kind    = eHitKind::Melee;
    hitbox.damage  = crush.damage;
    hitbox.bPierce = true;
    hitbox.owner   = m_ownerId;
    hitbox.id      = m_crushHitId;
    _ctx.pCombat->AddHitbox(hitbox);
}

void SlugVehicle::ReleaseCrushId_(
    const SlugContext& _ctx)
{
    if (m_crushHitId != kNoHitId && _ctx.pCombat)
    {
        _ctx.pCombat->ReleaseHitId(m_crushHitId);
    }
    m_crushHitId = kNoHitId;
}

void SlugVehicle::BeginExit_(
    const SlugContext& _ctx)
{
    ReleaseCrushId_(_ctx);

    m_eject.bValid          = true;
    m_eject.bDestroyed      = false;
    m_eject.feet            = { m_body.position.x, m_body.position.y - m_body.size.y * 0.5f };
    m_eject.velocity        = { static_cast<float>(m_facing) * m_tuning.board.exitSpeedX, -m_tuning.board.exitJumpSpeed };
    m_eject.facing          = m_facing;
    m_eject.invincibleTicks = 0;

    m_mode          = eSlugMode::Parked;
    m_modeTicks     = 0;
    m_exitCooldown  = m_tuning.board.exitCooldownTicks;
    m_parkAnimTicks = m_tuning.board.exitTicks;
    m_moveX         = 0;
    ++m_stats.exits;
}

void SlugVehicle::BeginDestroy_(
    const bool _bEjectPlayer)
{
    if (m_mode == eSlugMode::Exploding || m_mode == eSlugMode::Gone)
    {
        return;
    }
    if (_bEjectPlayer && (m_mode == eSlugMode::Driven || m_mode == eSlugMode::Boarding))
    {
        m_eject.bValid          = true;
        m_eject.bDestroyed      = true;
        m_eject.feet            = { m_body.position.x, m_body.position.y - m_body.size.y * 0.6f };
        m_eject.velocity        = { -static_cast<float>(m_facing) * m_tuning.board.exitSpeedX, -m_tuning.board.exitJumpSpeed };
        m_eject.facing          = m_facing;
        m_eject.invincibleTicks = m_tuning.damage.ejectInvincibleTicks;
    }
    m_hp            = 0;
    m_mode          = eSlugMode::Exploding;
    m_modeTicks     = 0;
    m_bExploded     = false;
    m_bReleaseCrush = true;
    m_moveX         = 0;
}

void SlugVehicle::OnHitEvents(
    const Span<const HitEvent> _events,
    const Span<const Hitbox>   _hitboxes)
{
    for (const HitEvent& event: _events)
    {
        if (event.target != m_ownerId || event.attackerTeam == eTeam::Player)
        {
            continue;
        }
        if (m_mode != eSlugMode::Driven && m_mode != eSlugMode::Parked)
        {
            continue;
        }

        int damage = m_tuning.damage.bulletDamage;
        switch (event.kind)
        {
            case eHitKind::Melee: damage = m_tuning.damage.meleeDamage; break;
            case eHitKind::Blast: damage = m_tuning.damage.blastDamage; break;
            case eHitKind::Hazard: damage = m_tuning.damage.hazardDamage; break;
            case eHitKind::Bullet:
            case eHitKind::Count: break;
        }

        bool bFront = true;
        if (event.hitboxIndex < _hitboxes.size())
        {
            const VECTOR2 center = _hitboxes[event.hitboxIndex].box.GetCenter();
            bFront               = (center.x - m_body.position.x) * static_cast<float>(m_facing) >= 0.f;
        }
        ApplyHit_(damage, bFront);
    }
}

void SlugVehicle::ApplyHit_(
    const int  _damage,
    const bool _bFromFront)
{
    if (m_invincibleTicks > 0 || m_hp <= 0 || m_godMode == eGodMode::Invincible)
    {
        return;
    }
    m_hp = Max(m_godMode == eGodMode::NoDeath ? 1 : 0, m_hp - _damage);
    StartHitFlash(m_flashTicks);
    m_invincibleTicks = m_tuning.damage.hitInvincibleTicks;
    m_bHitFront       = _bFromFront;
    m_hitAge          = 0;
    ++m_stats.hits;

    if (m_hp <= 0)
    {
        BeginDestroy_(true);
    }
}

void SlugVehicle::UpdateDamageFx_()
{
    if (m_mode == eSlugMode::Gone || m_mode == eSlugMode::Exploding)
    {
        return;
    }
    const int stage = GetDamageStage();
    if (stage >= 1 && m_tick % kSmokeInterval == 0)
    {
        SpawnFx_(eSlugFxKind::Smoke, m_body.position + GetDamageFxOffset_(m_tick / kSmokeInterval), 0, kFxSmokeTicks);
    }
    if (stage >= 2 && m_tick % kFireInterval == 0)
    {
        SpawnFx_(eSlugFxKind::Fire, m_body.position + GetDamageFxOffset_(m_tick / kFireInterval + 1), 0, kFxFireTicks);
    }
}

void SlugVehicle::SpawnFx_(
    const eSlugFxKind _kind,
    const VECTOR2     _position,
    const int         _variant,
    const int         _maxTicks)
{
    SlugFx fx;
    fx.kind     = _kind;
    fx.position = _position;
    fx.facing   = m_facing;
    fx.variant  = _variant;
    fx.ageTicks = 0;
    fx.maxTicks = _maxTicks;
    m_fx.push_back(fx);
}

void SlugVehicle::UpdatePose_()
{
    const bool bGrounded = m_body.bGrounded;

    if (m_mode == eSlugMode::Boarding)
    {
        m_pose = eSlugPose::BoardingIn;
        const int third = Max(1, m_tuning.board.boardTicks / 3);
        m_poseFrame     = 3 + Min(2, m_modeTicks / third);
        return;
    }
    if (m_mode == eSlugMode::Parked && m_crouchLevel >= kCrouchLevels && m_parkAnimTicks <= 0)
    {
        m_pose      = eSlugPose::ParkedOpen;
        m_poseFrame = 0;
        return;
    }

    if (m_hitAge != kHitAgeNone && m_crouchLevel == 0)
    {
        if (bGrounded)
        {
            m_pose = m_bHitFront ? eSlugPose::HitFront : eSlugPose::HitBack;
        }
        else
        {
            m_pose = m_bHitFront ? eSlugPose::AirHitA : eSlugPose::AirHitB;
        }
        eSlugClip clip = eSlugClip::HitFront;
        switch (m_pose)
        {
            case eSlugPose::HitBack: clip = eSlugClip::HitBack; break;
            case eSlugPose::AirHitA: clip = eSlugClip::AirHitA; break;
            case eSlugPose::AirHitB: clip = eSlugClip::AirHitB; break;
            default: break;
        }
        const SlugClipInfo& info = GetSlugClipInfo(clip);
        m_poseFrame              = Clamp(m_hitAge * info.fps / 60, 0, info.frames - 1);
        return;
    }

    if (!bGrounded)
    {
        m_pose         = eSlugPose::Jump;
        const float vy = m_body.velocity.y;
        m_poseFrame    = vy < -260.f ? 0 : (vy < -60.f ? 1 : (vy < 80.f ? 2 : 3));
        return;
    }

    if (m_crouchLevel > 0 && m_crouchLevel < kCrouchLevels)
    {
        m_pose      = eSlugPose::Crouching;
        m_poseFrame = m_crouchLevel;
        return;
    }
    if (m_crouchLevel >= kCrouchLevels)
    {
        const bool bMoving = Abs(m_body.velocity.x) > 1.f;
        m_pose             = bMoving ? eSlugPose::LowDrive : eSlugPose::LowIdle;
        m_poseFrame        = bMoving ? static_cast<int>(m_driveFrame) : (m_tick / 10) % 3;
        return;
    }

    if (m_landTicks > 0 && m_moveX == 0)
    {
        m_pose      = eSlugPose::Land;
        const int n = GetSlugClipInfo(eSlugClip::Land).frames;
        m_poseFrame = Clamp((m_tuning.move.landTicks - m_landTicks) * n / Max(1, m_tuning.move.landTicks), 0, n - 1);
        return;
    }

    const float speed = Abs(m_body.velocity.x);
    if (speed > 1.f)
    {
        if (m_accelDir > 0 && speed < m_tuning.move.runSpeed - 1.f)
        {
            m_pose      = eSlugPose::AccelStart;
            m_poseFrame = Clamp(static_cast<int>(m_accelPhase), 0, 3);
        }
        else if (m_accelDir < 0)
        {
            m_pose      = eSlugPose::AccelStop;
            m_poseFrame = Clamp(static_cast<int>(m_accelPhase), 0, 3);
        }
        else
        {
            m_pose      = eSlugPose::Drive;
            m_poseFrame = static_cast<int>(m_driveFrame) % GetSlugClipInfo(eSlugClip::Drive).frames;
        }
        return;
    }
    m_pose      = eSlugPose::Idle;
    m_poseFrame = (m_tick / 10) % GetSlugClipInfo(eSlugClip::Idle).frames;
}

SlugEjectRequest SlugVehicle::TakeEjectRequest()
{
    const SlugEjectRequest request = m_eject;
    m_eject                        = {};
    return request;
}

void SlugVehicle::AddCannonAmmo(
    const int _count)
{
    m_cannonAmmo = Clamp(m_cannonAmmo + _count, 0, m_tuning.cannon.ammoMax);
}

eSlugMode SlugVehicle::GetMode() const
{
    return m_mode;
}

bool SlugVehicle::IsAlive() const
{
    return m_mode != eSlugMode::Gone && m_mode != eSlugMode::Exploding;
}

VECTOR2 SlugVehicle::GetFeet() const
{
    return m_body.position;
}

int SlugVehicle::GetZ() const
{
    return m_z;
}

void SlugVehicle::SetZ(
    const int _z)
{
    m_z = _z;
}

VECTOR2 SlugVehicle::GetPrevFeet() const
{
    return m_prevFeet;
}

VECTOR2 SlugVehicle::GetVelocity() const
{
    return m_body.velocity;
}

int SlugVehicle::GetFacing() const
{
    return m_facing;
}

bool SlugVehicle::IsGrounded() const
{
    return m_body.bGrounded;
}

float SlugVehicle::GetHpRatio() const
{
    return m_tuning.damage.maxHp > 0 ? static_cast<float>(m_hp) / static_cast<float>(m_tuning.damage.maxHp) : 0.f;
}

int SlugVehicle::GetCannonAmmo() const
{
    return m_cannonAmmo;
}

float SlugVehicle::GetVulcanAngle() const
{
    return m_vulcanAngle;
}

int SlugVehicle::GetVulcanShotAge() const
{
    return m_vulcanShotAge;
}

bool SlugVehicle::IsVulcanVisible() const
{
    if (m_mode == eSlugMode::Driven)
    {
        return true;
    }
    if (m_mode == eSlugMode::Boarding)
    {
        return m_modeTicks * 2 >= m_tuning.board.boardTicks;
    }
    return m_mode == eSlugMode::Exploding && !m_bExploded;
}

bool SlugVehicle::IsBodyVisible() const
{
    return m_mode != eSlugMode::Gone && !m_bExploded;
}

eSlugPose SlugVehicle::GetPose() const
{
    return m_pose;
}

int SlugVehicle::GetPoseFrame() const
{
    return m_poseFrame;
}

int SlugVehicle::GetFlashTicks() const
{
    return m_flashTicks;
}

bool SlugVehicle::IsBlinkWhite() const
{
    if (m_mode == eSlugMode::Exploding)
    {
        return !m_bExploded && ((m_modeTicks / 3) % 2) == 0;
    }
    if (m_selfDestructTicks >= 0)
    {
        return ((m_selfDestructTicks / 6) % 2) == 0;
    }
    return false;
}

int SlugVehicle::GetDamageStage() const
{
    if (m_hp <= 0)
    {
        return 0;
    }
    const float ratio = GetHpRatio();
    if (ratio <= m_tuning.damage.fireRatio)
    {
        return 2;
    }
    return ratio <= m_tuning.damage.smokeRatio ? 1 : 0;
}

bool SlugVehicle::ShouldShowInArrow() const
{
    return m_mode == eSlugMode::Parked && CanBoard();
}

bool SlugVehicle::ShouldShowOutArrow() const
{
    return m_mode == eSlugMode::Driven && GetDamageStage() >= 2 && ((m_tick / 30) % 2) == 0;
}

int SlugVehicle::GetTick() const
{
    return m_tick;
}

Aabb SlugVehicle::GetBodyBox() const
{
    return m_body.GetBox();
}

Aabb SlugVehicle::GetBoardBox() const
{
    return Aabb::FromBottomCenter(m_body.position, m_tuning.board.boardBoxSize);
}

Hurtbox SlugVehicle::MakeHurtbox() const
{
    const float   t    = static_cast<float>(m_crouchLevel) / static_cast<float>(kCrouchLevels);
    const VECTOR2 size = { m_tuning.body.hurtStand.x, Lerp(m_tuning.body.hurtStand.y, m_tuning.body.hurtCrouch.y, t) };

    Hurtbox hurtbox;
    hurtbox.box           = Aabb::FromBottomCenter(m_body.position, size);
    hurtbox.team          = eTeam::Player;
    hurtbox.owner         = m_ownerId;
    hurtbox.bInvulnerable = m_invincibleTicks > 0 || m_mode == eSlugMode::Boarding;
    return hurtbox;
}

VECTOR2 SlugVehicle::MountOffset_() const
{
    const float t = static_cast<float>(m_crouchLevel) / static_cast<float>(kCrouchLevels);
    return { Lerp(m_tuning.vulcan.mountStand.x, m_tuning.vulcan.mountCrouch.x, t), Lerp(m_tuning.vulcan.mountStand.y, m_tuning.vulcan.mountCrouch.y, t) };
}

VECTOR2 SlugVehicle::GetVulcanMount() const
{
    const VECTOR2 offset = MountOffset_();
    return { m_body.position.x + offset.x * static_cast<float>(m_facing), m_body.position.y + offset.y };
}

VECTOR2 SlugVehicle::GetVulcanMuzzle() const
{
    const float   rad   = m_vulcanAngle * kDeg2Rad;
    const VECTOR2 mount = GetVulcanMount();
    return { mount.x + std::cos(rad) * m_tuning.vulcan.barrelLength, mount.y - std::sin(rad) * m_tuning.vulcan.barrelLength };
}

VECTOR2 SlugVehicle::GetCannonMuzzle() const
{
    const float   t      = static_cast<float>(m_crouchLevel) / static_cast<float>(kCrouchLevels);
    const VECTOR2 offset = { Lerp(m_tuning.cannon.muzzleStand.x, m_tuning.cannon.muzzleCrouch.x, t), Lerp(m_tuning.cannon.muzzleStand.y, m_tuning.cannon.muzzleCrouch.y, t) };
    return { m_body.position.x + offset.x * static_cast<float>(m_facing), m_body.position.y + offset.y };
}

const SlugStats& SlugVehicle::GetStats() const
{
    return m_stats;
}

Span<const SlugFx> SlugVehicle::GetFx() const
{
    return m_fx;
}

const Body& SlugVehicle::GetBody() const
{
    return m_body;
}

uint64_t SlugVehicle::HashState() const
{
    Fnv1a hasher;
    hasher.Mix(m_body.position.x);
    hasher.Mix(m_body.position.y);
    hasher.Mix(m_body.velocity.x);
    hasher.Mix(m_body.velocity.y);
    hasher.Mix(m_mode);
    hasher.Mix(m_hp);
    hasher.Mix(m_cannonAmmo);
    hasher.Mix(m_vulcanAngle);
    hasher.Mix(m_crouchLevel);
    hasher.Mix(m_facing);
    hasher.Mix(m_stats.vulcanShots);
    hasher.Mix(m_stats.cannonShots);
    return hasher.Finalize();
}
