#include "Game/Player/PlayerWeapons.h"

#include <algorithm>
#include <cmath>

constexpr float kDiagonalRatio = 0.6f;

namespace
{
[[nodiscard]] VECTOR2 OrientSize_(
    const VECTOR2 _size,
    const VECTOR2 _direction)
{
    const float ax = Abs(_direction.x);
    const float ay = Abs(_direction.y);
    if (ax > 0.f && ay > 0.f && Min(ay, ax) / Max(ay, ax) >= kDiagonalRatio)
    {
        const float side = Max(_size.y, _size.x);
        return { side, side };
    }
    if (ay > ax)
    {
        return { _size.y, _size.x };
    }
    return _size;
}
}   // namespace

void WeaponState::Reset(
    const Tuning& _tuning)
{
    *this          = {};
    weapon         = eWeapon::Pistol;
    bombs          = _tuning.bomb.initial;
    ticksSinceShot = WeaponState::kLongAgo;
}

void WeaponState::Give(
    const eWeapon _weapon,
    const int     _ammo)
{
    if (_weapon == eWeapon::Pistol)
    {
        weapon = eWeapon::Pistol;
        ammo   = 0;
        return;
    }

    ammo   = weapon == _weapon ? ammo + _ammo : _ammo;
    weapon = _weapon;
}

void WeaponState::Tick()
{
    if (ticksSinceShot < WeaponState::kLongAgo)
    {
        ++ticksSinceShot;
    }
}

bool WeaponState::ShouldFire(
    const WeaponTuning& _tuning,
    const int           _aliveBullets,
    const bool          _bShotBuffered,
    const bool          _bShotHeld) const
{
    if (_tuning.maxAlive > 0 && _aliveBullets >= _tuning.maxAlive)
    {
        return false;
    }
    if (ticksSinceShot < _tuning.fireIntervalTicks)
    {
        return false;
    }
    if (_bShotBuffered)
    {
        return true;
    }
    return _bShotHeld && _tuning.holdRepeatTicks > 0 && ticksSinceShot >= Max(_tuning.fireIntervalTicks, _tuning.holdRepeatTicks);
}

void WeaponState::OnFired()
{
    ticksSinceShot = 0;
    if (HasFiniteAmmo() && --ammo <= 0)
    {
        weapon = eWeapon::Pistol;
        ammo   = 0;
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
    const VECTOR2       _muzzle,
    const VECTOR2       _sweepFrom,
    const VECTOR2       _direction)
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

bool WeaponState::HasFiniteAmmo() const
{
    return weapon != eWeapon::Pistol;
}

Weapon::Weapon(const eWeapon _kind)
    : m_kind(_kind)
{
}

BulletSpawn Weapon::MakeShot(const WeaponTuning& _tuning, const ShotAim& _aim, RngMt19937&) const
{
    return MakeBulletSpawn(m_kind, _tuning, _aim.owner, _aim.muzzle, _aim.sweepFrom, _aim.direction);
}

namespace
{
class HmgWeapon : public Weapon
{
    JUG_CLASS(HmgWeapon, NO_COPY, NO_MOVE)

public:
    HmgWeapon()
        : Weapon(eWeapon::Hmg)
    {}
    [[nodiscard]] BulletSpawn MakeShot(const WeaponTuning& _tuning, const ShotAim& _aim, RngMt19937& _rng) const override;
};

class FlameWeapon : public Weapon
{
    JUG_CLASS(FlameWeapon, NO_COPY, NO_MOVE)

public:
    FlameWeapon()
        : Weapon(eWeapon::Flame)
    {}
    [[nodiscard]] BulletSpawn MakeShot(const WeaponTuning& _tuning, const ShotAim& _aim, RngMt19937& _rng) const override;
};

BulletSpawn HmgWeapon::MakeShot(const WeaponTuning& _tuning, const ShotAim& _aim, RngMt19937& _rng) const
{
    const float   rad       = _aim.degrees * kDeg2Rad;
    const VECTOR2 direction = { static_cast<float>(_aim.facing) * std::cos(rad), -std::sin(rad) };
    VECTOR2       muzzle    = _aim.muzzle;
    if (_tuning.muzzleJitter.y > 0.f)
    {
        const float offset = Random<float>(_rng, -_tuning.muzzleJitter.y, _tuning.muzzleJitter.y);
        muzzle.x += -direction.y * offset;
        muzzle.y += direction.x * offset;
    }
    return MakeBulletSpawn(eWeapon::Hmg, _tuning, _aim.owner, muzzle, _aim.sweepFrom, direction);
}

BulletSpawn FlameWeapon::MakeShot(
    const WeaponTuning& _tuning,
    const ShotAim&      _aim,
    RngMt19937&         _rng) const
{
    const float   spreadRad = Random<float>(_rng, -_tuning.spreadDeg, _tuning.spreadDeg) * kDeg2Rad;
    const float   c         = std::cos(spreadRad);
    const float   s         = std::sin(spreadRad);
    const VECTOR2 direction = { _aim.direction.x * c - _aim.direction.y * s, _aim.direction.x * s + _aim.direction.y * c };

    VECTOR2 muzzle = _aim.muzzle;
    if (_tuning.muzzleJitter.y > 0.f)
    {
        muzzle.y += Random<float>(_rng, -_tuning.muzzleJitter.y, _tuning.muzzleJitter.y);
    }
    const float speed = Random<float>(_rng, _tuning.particleSpeedMin, _tuning.particleSpeedMax);
    const int   life  = Random<int>(_rng, _tuning.particleLifeMinTicks, _tuning.particleLifeMaxTicks);

    BulletSpawn spawn = MakeBulletSpawn(eWeapon::Flame, _tuning, _aim.owner, muzzle, _aim.sweepFrom, direction);
    spawn.speed       = speed;
    spawn.range       = speed * static_cast<float>(life) / 60.f;
    spawn.sizeEnd     = OrientSize_(_tuning.particleHitSizeEnd, direction);
    spawn.hitId       = _aim.hitId;
    return spawn;
}

}   // namespace

const Weapon& GetWeaponBehavior(const eWeapon _kind)
{
    static const Weapon      pistol { eWeapon::Pistol };
    static const HmgWeapon   hmg;
    static const Weapon      shotgun { eWeapon::Shotgun };
    static const Weapon      rocket { eWeapon::Rocket };
    static const FlameWeapon flame;
    switch (_kind)
    {
        case eWeapon::Pistol: return pistol;
        case eWeapon::Hmg: return hmg;
        case eWeapon::Shotgun: return shotgun;
        case eWeapon::Rocket: return rocket;
        case eWeapon::Flame: return flame;
        case eWeapon::Count: break;
    }
    JUG_ASSERT(false, "Invalid weapon.\n");
    return pistol;
}
