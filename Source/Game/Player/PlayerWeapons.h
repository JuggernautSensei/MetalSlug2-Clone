#pragma once
#include "Base/Math.h"
#include "Base/Random.h"
#include "Core/Tuning.h"
#include "Game/Combat/Projectiles.h"
#include "Game/Player/PlayerTypes.h"

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

// 파이어샷 분출 입자 하나 (방향/속도/수명은 _rng 에서 결정적으로 뽑는다, _hitId 는 분출 전체가 공유)
[[nodiscard]] BulletSpawn MakeFlameParticleSpawn(
    const WeaponTuning& _tuning,
    uint32_t            _owner,
    Vec2                _muzzle,
    Vec2                _sweepFrom,
    Vec2                _direction,
    uint32_t            _hitId,
    RngMt19937&         _rng);
