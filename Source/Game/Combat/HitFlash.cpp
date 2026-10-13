#include "Game/Combat/HitFlash.h"

bool IsHitFlashOn(
    const int _ticksLeft)
{
    return _ticksLeft > kHitFlashGapTicks && _ticksLeft < kHitFlashTicks;
}

bool StartHitFlash(
    int& _ticksLeft)
{
    if (IsHitFlashOn(_ticksLeft) || _ticksLeft == kHitFlashTicks)
    {
        return false;
    }
    _ticksLeft = kHitFlashTicks;
    return true;
}

void TickHitFlash(
    int& _ticksLeft)
{
    if (_ticksLeft > 0)
    {
        --_ticksLeft;
    }
}
