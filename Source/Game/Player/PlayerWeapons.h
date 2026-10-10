#pragma once
#include "Base/Math.h"
#include "Core/Tuning.h"
#include "Game/Combat/Projectiles.h"
#include "Game/Player/PlayerTypes.h"

namespace slug
{

struct WeaponState
{
    static constexpr int kLongAgo = 1'000'000;

    eWeapon weapon         = eWeapon::Pistol;
    int     ammo           = 0;   // 유한 무기의 남은 탄 (권총은 사용 안 함)
    int     bombs          = 0;
    int     ticksSinceShot = kLongAgo;

    [[nodiscard]] bool HasFiniteAmmo() const;
};

void ResetWeapons(WeaponState& _state, const Tuning& _tuning);

void GiveWeapon(WeaponState& _state, eWeapon _weapon, int _ammo);

void TickWeapon(WeaponState& _state);

[[nodiscard]] bool ShouldFire(
    const WeaponState&  _state,
    const WeaponTuning& _tuning,
    int                 _aliveBullets,
    bool                _bShotBuffered,
    bool                _bShotHeld);

void OnFired(WeaponState& _state);

[[nodiscard]] eWeapon WeaponFromDebugSlot(int _slot);

[[nodiscard]] BulletSpawn MakeBulletSpawn(
    eWeapon             _weapon,
    const WeaponTuning& _tuning,
    uint32_t            _owner,
    Vec2                _muzzle,
    Vec2                _sweepFrom,
    Vec2                _direction);

}   // namespace slug
