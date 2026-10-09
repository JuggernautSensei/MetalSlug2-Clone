#pragma once
#include "Core/Tuning.h"
#include "Game/Combat/Projectiles.h"
#include "Game/Player/PlayerTypes.h"

namespace slug
{

// ===========================================
//  WeaponState
//   플레이어가 들고 있는 무기와 탄약 (값 타입). 발사 규칙은 아래 순수 함수들이 정한다.
// ===========================================

struct WeaponState
{
    static constexpr int kLongAgo = 1'000'000;

    eWeapon weapon         = eWeapon::Pistol;
    int     ammo           = 0;   // 유한 무기의 남은 탄 (권총은 사용 안 함)
    int     bombs          = 0;
    int     ticksSinceShot = kLongAgo;

    [[nodiscard]] bool HasFiniteAmmo() const { return weapon != eWeapon::Pistol; }
};

// 새 게임/리스폰: 권총, 폭탄 초기값
void ResetWeapons(WeaponState& _state, const Tuning& _tuning);

// 무기 획득 (같은 무기면 탄 추가)
void GiveWeapon(WeaponState& _state, eWeapon _weapon, int _ammo);

// 매 틱 호출 (쿨다운 진행)
void TickWeapon(WeaponState& _state);

// 지금 쏠 수 있는가.
//   - 연타(버퍼된 눌림): 마지막 발사 후 fireIntervalTicks 가 지났을 때
//   - 홀드 반복: holdRepeatTicks > 0 이고 홀드 중이며 max(fireInterval, holdRepeat) 가 지났을 때
//   - 화면 내 탄 수가 maxAlive 이상이면 쏘지 못한다 (maxAlive == 0 이면 무제한)
[[nodiscard]] bool ShouldFire(const WeaponState& _state, const WeaponTuning& _tuning, int _aliveBullets, bool _bShotBuffered, bool _bShotHeld);

// 발사 처리: 쿨다운 리셋, 탄 소모, 소진되면 권총으로 복귀
void OnFired(WeaponState& _state);

// 디버그 무기 지급 슬롯 (F7..F10: 1 HMG, 2 샷건, 3 로켓, 4 화염). 그 밖의 값은 권총.
[[nodiscard]] eWeapon WeaponFromDebugSlot(int _slot);

// 탄 생성 정보
[[nodiscard]] BulletSpawn MakeBulletSpawn(eWeapon _weapon, const WeaponTuning& _tuning, uint32_t _owner, jug::VECTOR2 _muzzle, jug::VECTOR2 _sweepFrom, jug::VECTOR2 _direction);

}   // namespace slug
