#pragma once
#include "Core/Tuning.h"
#include "Game/Combat/Projectiles.h"
#include "Game/Player/PlayerTypes.h"

struct WeaponState
{
    void               Reset(const Tuning& _tuning);
    void               Give(eWeapon _weapon, int _ammo);
    void               Tick();
    [[nodiscard]] bool ShouldFire(const WeaponTuning& _tuning, int _aliveBullets, bool _bShotBuffered, bool _bShotHeld) const;
    void               OnFired();
    [[nodiscard]] bool HasFiniteAmmo() const;

    static constexpr int kLongAgo = 1'000'000;

    eWeapon weapon         = eWeapon::Pistol;
    int     ammo           = 0;
    int     bombs          = 0;
    int     ticksSinceShot = kLongAgo;
};

struct ShotAim
{
    uint32_t owner     = kPlayerOwnerId;
    VECTOR2  muzzle    = {};
    VECTOR2  sweepFrom = {};
    VECTOR2  direction = {};
    float    degrees   = 0.f;
    int      facing    = 1;
    uint32_t hitId     = kNoHitId;
};

class Weapon
{
    JUG_CLASS(Weapon, NO_COPY, NO_MOVE)

public:
    explicit Weapon(eWeapon _kind);
    virtual ~Weapon() = default;
    [[nodiscard]] virtual BulletSpawn MakeShot(const WeaponTuning& _tuning, const ShotAim& _aim, RngMt19937& _rng) const;

private:
    eWeapon m_kind;
};

[[nodiscard]] const Weapon& GetWeaponBehavior(eWeapon _kind);

[[nodiscard]] eWeapon WeaponFromDebugSlot(int _slot);

[[nodiscard]] BulletSpawn MakeBulletSpawn(eWeapon _weapon, const WeaponTuning& _tuning, uint32_t _owner, VECTOR2 _muzzle, VECTOR2 _sweepFrom, VECTOR2 _direction);
