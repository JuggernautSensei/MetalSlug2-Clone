#include "Game/Player/PlayerWeapons.h"

#include <JugX/Math.h>

namespace slug
{

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
    return _bShotHeld && _tuning.holdRepeatTicks > 0 && _state.ticksSinceShot >= jug::Max(_tuning.fireIntervalTicks, _tuning.holdRepeatTicks);
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
    const jug::VECTOR2  _muzzle,
    const jug::VECTOR2  _sweepFrom,
    const jug::VECTOR2  _direction)
{
    BulletSpawn spawn;
    spawn.team      = eTeam::Player;
    spawn.owner     = _owner;
    spawn.position  = _muzzle;
    spawn.sweepFrom = _sweepFrom;
    spawn.direction = _direction;
    spawn.speed     = _tuning.projectileSpeed;
    spawn.damage    = _tuning.damage;
    spawn.size      = _tuning.hitSize;
    spawn.range     = _tuning.range;
    spawn.bPierce   = _tuning.bPierce;
    spawn.tag         = static_cast<uint8_t>(_weapon);
    spawn.kind        = eHitKind::Bullet;
    spawn.blastSize   = _tuning.blastSize;
    spawn.blastDamage = _tuning.blastDamage;
    spawn.blastTicks  = _tuning.blastTicks;
    spawn.bTrail      = _weapon == eWeapon::Rocket;
    return spawn;
}

}   // namespace slug
