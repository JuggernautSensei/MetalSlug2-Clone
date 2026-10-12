#pragma once
#include <algorithm>
#include <cstdint>

#include "Base/Types.h"

class VoiceAllocator
{
public:
    struct Acquired
    {
        int  slot    = -1;
        bool bStolen = false;
    };

    VoiceAllocator() = default;

    VoiceAllocator(
        const int _capacity,
        const int _maxPerSound);

    void Reset(
        const int _capacity,
        const int _maxPerSound);

    [[nodiscard]] Acquired Acquire(
        const int _soundId);

    void Release(
        const int _slot);

    void ReleaseAll();

    [[nodiscard]] int  GetCapacity() const;
    [[nodiscard]] bool IsActive(const int _slot) const;
    [[nodiscard]] int  GetSoundId(const int _slot) const;

    [[nodiscard]] int GetActiveCount() const;

    [[nodiscard]] int CountSound(
        const int _soundId) const;

private:
    struct Slot
    {
        bool     bActive = false;
        int      soundId = -1;
        uint64_t serial  = 0;   // 시작 순서 (작을수록 오래됨)
    };

    [[nodiscard]] int FindOldest_(
        const int _soundId) const;

    [[nodiscard]] Acquired Take_(
        const int  _slot,
        const int  _soundId,
        const bool _bStolen);

    Vector<Slot> m_slots       = {};
    int          m_maxPerSound = 4;
    uint64_t     m_nextSerial  = 1;
};
