#include "Game/Player/PlayerWeapons.h"

#include <algorithm>
#include <cmath>

#include "Base/Math.h"

constexpr float kDiagonalRatio = 0.6f;   // 짧은 축/긴 축 비율이 이 이상이면 대각 조준으로 보고 정사각 판정을 쓴다 (약 31~59도)

namespace
{
[[nodiscard]] Vec2 OrientSize_(
    const Vec2 _size,
    const Vec2 _direction)
{
    const float ax = std::abs(_direction.x);
    const float ay = std::abs(_direction.y);
    if (ax > 0.f && ay > 0.f && std::min(ax, ay) / std::max(ax, ay) >= kDiagonalRatio)
    {
        const float side = std::max(_size.x, _size.y);
        return { side, side };
    }
    if (ay > ax)
    {
        return { _size.y, _size.x };
    }
    return _size;
}
}   // namespace

void ResetWeapons(
    WeaponState&  _state,
    const Tuning& _tuning)
{
    _state                = {};
    _state.weapon         = eWeapon::Pistol;
    _state.bombs          = _tuning.bomb.initial;
    _state.ticksSinceShot = WeaponState::kLongAgo;
}

void GiveWeapon(
    WeaponState&  _state,
    const eWeapon _weapon,
    const int     _ammo)
{
    if (_weapon == eWeapon::Pistol)
    {
        _state.weapon = eWeapon::Pistol;
        _state.ammo   = 0;
        return;
    }

    _state.ammo   = _state.weapon == _weapon ? _state.ammo + _ammo : _ammo;
    _state.weapon = _weapon;
}

void TickWeapon(
    WeaponState& _state)
{
    if (_state.ticksSinceShot < WeaponState::kLongAgo)
    {
        ++_state.ticksSinceShot;
    }
}

bool ShouldFire(
    const WeaponState&  _state,
    const WeaponTuning& _tuning,
    const int           _aliveBullets,
    const bool          _bShotBuffered,
    const bool          _bShotHeld)
{
    if (_tuning.maxAlive > 0 && _aliveBullets >= _tuning.maxAlive)
    {
        return false;
    }
    if (_state.ticksSinceShot < _tuning.fireIntervalTicks)
    {
        return false;
    }
    if (_bShotBuffered)
    {
        return true;
    }
    return _bShotHeld && _tuning.holdRepeatTicks > 0 && _state.ticksSinceShot >= Max(_tuning.fireIntervalTicks, _tuning.holdRepeatTicks);
}

void OnFired(
    WeaponState& _state)
{
    _state.ticksSinceShot = 0;
    if (_state.HasFiniteAmmo() && --_state.ammo <= 0)
    {
        _state.weapon = eWeapon::Pistol;
        _state.ammo   = 0;
    }
}

eWeapon WeaponFromDebugSlot(
    const int _slot)
{
    switch (_slot)
    {
        case 1: return eWeapon::Hmg;
        case 2: return eWeapon::Shotgun;
        case 3: return eWeapon::Rocket;
        case 4: return eWeapon::Flame;
        default: return eWeapon::Pistol;
    }
}

BulletSpawn MakeBulletSpawn(
    const eWeapon       _weapon,
    const WeaponTuning& _tuning,
    const uint32_t      _owner,
    const Vec2          _muzzle,
    const Vec2          _sweepFrom,
    const Vec2          _direction)
{
    BulletSpawn spawn;
    spawn.team        = eTeam::Player;
    spawn.owner       = _owner;
    spawn.position    = _muzzle;
    spawn.sweepFrom   = _sweepFrom;
    spawn.direction   = _direction;
    spawn.speed       = _tuning.projectileSpeed;
    spawn.damage      = _tuning.damage;
    spawn.size        = OrientSize_(_tuning.hitSize, _direction);
    spawn.scaleStart  = _tuning.particleScaleStart;
    spawn.scaleEnd    = _tuning.particleScaleEnd;
    spawn.range       = _tuning.range;
    spawn.bPierce     = _tuning.bPierce;
    spawn.tag         = static_cast<uint8_t>(_weapon);
    spawn.kind        = eHitKind::Bullet;
    spawn.blastSize   = _tuning.blastSize;
    spawn.blastDamage = _tuning.blastDamage;
    spawn.blastTicks  = _tuning.blastTicks;
    spawn.bTrail      = _weapon == eWeapon::Rocket;
    spawn.accel       = _tuning.speedAccel;
    spawn.maxSpeed    = _tuning.speedMax;
    return spawn;
}

BulletSpawn MakeFlameParticleSpawn(
    const WeaponTuning& _tuning,
    const uint32_t      _owner,
    const Vec2          _muzzle,
    const Vec2          _sweepFrom,
    const Vec2          _direction,
    const uint32_t      _hitId,
    RngMt19937&         _rng)
{
    // 방향: 기준 방향을 +-spreadDeg 안에서 무작위로 돌린다 (순서 고정: 각도, 세로 지터, 속도, 수명)
    const float spreadRad = Random<float>(_rng, -_tuning.spreadDeg, _tuning.spreadDeg) * kDeg2Rad;
    const float c         = std::cos(spreadRad);
    const float s         = std::sin(spreadRad);
    const Vec2  direction = { _direction.x * c - _direction.y * s, _direction.x * s + _direction.y * c };

    Vec2 muzzle = _muzzle;
    if (_tuning.muzzleJitter.y > 0.f)
    {
        muzzle.y += Random<float>(_rng, -_tuning.muzzleJitter.y, _tuning.muzzleJitter.y);
    }
    const float speed = Random<float>(_rng, _tuning.particleSpeedMin, _tuning.particleSpeedMax);
    const int   life  = Random<int>(_rng, _tuning.particleLifeMinTicks, _tuning.particleLifeMaxTicks);

    BulletSpawn spawn = MakeBulletSpawn(eWeapon::Flame, _tuning, _owner, muzzle, _sweepFrom, direction);
    spawn.speed       = speed;
    spawn.range       = speed * static_cast<float>(life) / 60.f;
    spawn.sizeEnd     = OrientSize_(_tuning.particleHitSizeEnd, direction);
    spawn.hitId       = _hitId;
    return spawn;
}

bool WeaponState::HasFiniteAmmo() const
{
    return weapon != eWeapon::Pistol;
}
