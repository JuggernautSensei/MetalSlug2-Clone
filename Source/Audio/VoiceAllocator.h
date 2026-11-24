#pragma once
#include <cstdint>

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
    void                   Reset(int _capacity, int _maxPerSound);
    [[nodiscard]] Acquired Acquire(int _soundId);
    void                   Release(int _slot);
    [[nodiscard]] int      GetCapacity() const;
    [[nodiscard]] bool     IsActive(int _slot) const;
    [[nodiscard]] int      GetSoundId(int _slot) const;
    [[nodiscard]] int      GetActiveCount() const;
    [[nodiscard]] int      CountSound(int _soundId) const;

private:
    [[nodiscard]] int FindOldest_(int _soundId) const;

    struct Slot
    {
        bool     bActive = false;
        int      soundId = -1;
        uint64_t order   = 0;
    };

    Vector<Slot> m_slots       = {};
    int          m_maxPerSound = 4;
    uint64_t     m_nextOrder   = 1;
};
