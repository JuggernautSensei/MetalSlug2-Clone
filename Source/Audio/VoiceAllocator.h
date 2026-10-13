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

    VoiceAllocator(int _capacity, int _maxPerSound);

    void Reset(int _capacity, int _maxPerSound);

    [[nodiscard]] Acquired Acquire(int _soundId);

    void Release(int _slot);

    [[nodiscard]] int  GetCapacity() const;
    [[nodiscard]] bool IsActive(int _slot) const;
    [[nodiscard]] int  GetSoundId(int _slot) const;

    [[nodiscard]] int GetActiveCount() const;

    [[nodiscard]] int CountSound(int _soundId) const;

private:
    struct Slot
    {
        bool     bActive = false;
        int      soundId = -1;
        uint64_t serial  = 0;   // 시작 순서
    };

    [[nodiscard]] int FindOldest_(int _soundId) const;

    [[nodiscard]] Acquired Take_(int _slot, int _soundId, bool _bStolen);

    Vector<Slot> m_slots       = {};
    int          m_maxPerSound = 4;
    uint64_t     m_nextSerial  = 1;
};
