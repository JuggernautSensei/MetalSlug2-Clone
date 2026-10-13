#include "Game/Enemies/EnemyTypes.h"

uint32_t MakePartOwner(
    uint32_t _enemyId,
    uint32_t _partIndex)
{
    return (_enemyId & kEnemyIdMask) | ((_partIndex + 1u) << 24);
}

int GetPartIndex(
    uint32_t _owner)
{
    const uint32_t part = (_owner >> 24) & 0x3Fu;
    return part == 0u ? -1 : static_cast<int>(part) - 1;
}

bool IsEnemyOwner(
    uint32_t _owner)
{
    const uint32_t id = _owner & ~kShieldOwnerBit;
    return id >= kEnemyOwnerBase && id < kShieldOwnerBit;
}

bool EnemyHandle::IsNull() const
{
    return id == 0;
}

VECTOR2 Enemy::GetFeet() const
{
    return body.position;
}

bool Enemy::IsAlive() const
{
    return life == eEnemyLife::Alive;
}

float HitFlashAmount(
    const Enemy& _e)
{
    return IsHitFlashOn(_e.hitFlashTicks) ? 1.f : 0.f;
}

float PartFlashAmount(
    const Enemy& _e,
    int          _part)
{
    if (_part < 0 || _part >= kEnemyFlashPartCount)
    {
        return 0.f;
    }
    return IsHitFlashOn(_e.partFlashTicks[static_cast<size_t>(_part)]) ? 1.f : 0.f;
}

void TriggerPartHitFlash(
    Enemy& _e,
    int    _part)
{
    if (_part >= 0 && _part < kEnemyFlashPartCount)
    {
        (void)StartHitFlash(_e.partFlashTicks[static_cast<size_t>(_part)]);
    }
}
