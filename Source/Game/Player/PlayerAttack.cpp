#include "Game/Player/PlayerController.h"

#include "Game/Player/PlayerHitboxes.h"
#include "Render/SpriteBatch.h"
#include "World/Constants.h"

#include <bit>
#include <cmath>

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
    if (m_burstLeft > 0)
    {
        m_shotTicksLeft = 0;
    }
    m_burstLeft  = 0;
    m_burstTimer = 0;
}

void PlayerController::StopFlameSpawn_()
{
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
    EndFlame_();
    ++m_stats.shotsFired;
    m_weapon.OnFired();
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
    const VECTOR2 muzzle    = MakeMuzzle_(*_ctx.pTuning, _flame);
    const VECTOR2 direction = GetAimDirection(m_aim, m_facing);
    const VECTOR2 chest     = { m_body.position.x, m_body.position.y - m_body.size.y * 0.6f };

    const ShotAim shot { kPlayerOwnerId, muzzle, chest, direction, 0.f, 1, m_flameHitId };
    BulletSpawn   spawn = GetWeaponBehavior(eWeapon::Flame).MakeShot(_flame, shot, m_flameRng);
    spawn.z             = ZShift(m_z, kPlayerDepth, kPlayerBulletDepth);
    _ctx.pProjectiles->SpawnBullet(spawn, *_ctx.pCombat);
}

void PlayerController::TickShot_(
    const PlayerContext& _ctx,
    const PlayerCommand& _command)
{
    const Tuning&       tuning = *_ctx.pTuning;
    const WeaponTuning& weapon = tuning.weapons.Get(m_weapon.weapon);

    // 파이어샷 분출은 사격 입력과 별개로 끝까지 돈다
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
    if (!m_weapon.ShouldFire(weapon, alive, _command.bShotBuffered, _command.bShotHeld))
    {
        return;
    }

    if (m_weapon.weapon == eWeapon::Flame)
    {
        StartFlame_(_ctx, weapon);
        m_shotOverlayTicks = weapon.shotOverlayTicks;
        m_shotTicksLeft    = m_shotOverlayTicks;
        ++m_overlaySerial;
        m_input.Consume(eAction::Shot);
        return;
    }

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
    uint64_t seed = 0x9E37'79B9'7F4A'7C15ull ^ static_cast<uint64_t>(m_stats.shotsFired);
    seed          = seed * 6'364'136'223'846'793'005ull + std::bit_cast<uint32_t>(m_body.position.x);
    seed          = seed * 6'364'136'223'846'793'005ull + std::bit_cast<uint32_t>(m_body.position.y);
    return seed;
}

VECTOR2 PlayerController::MakeMuzzle_(
    const Tuning&       _tuning,
    const WeaponTuning& _weapon) const
{
    VECTOR2 muzzle = GetMuzzlePosition(m_body.position, m_facing, m_pose, m_aim, _tuning.muzzle);
    if (m_aim == eAim::Forward)
    {
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

    VECTOR2       muzzle    = MakeMuzzle_(tuning, _weapon);
    VECTOR2       direction = GetAimDirection(m_aim, m_facing);
    const VECTOR2 chest     = { m_body.position.x, m_body.position.y - m_body.size.y * 0.6f };

    const ShotAim shot { kPlayerOwnerId, muzzle, chest, direction, m_aimDegree, m_facing };
    BulletSpawn   spawn = GetWeaponBehavior(m_weapon.weapon).MakeShot(_weapon, shot, m_shotRng);
    spawn.z             = ZShift(m_z, kPlayerDepth, kPlayerBulletDepth);
    _ctx.pProjectiles->SpawnBullet(spawn, *_ctx.pCombat);
    ++m_stats.shotsFired;
    m_weapon.OnFired();
    if (m_weapon.weapon == eWeapon::Pistol)
    {
        m_burstLeft = 0;
    }
}
