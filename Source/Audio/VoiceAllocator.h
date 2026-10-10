#pragma once
#include <algorithm>
#include <cstdint>

#include "Base/Assert.h"
#include "Base/Types.h"

namespace slug
{

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
        const int _maxPerSound)
    {
        Reset(_capacity, _maxPerSound);
    }

    void Reset(
        const int _capacity,
        const int _maxPerSound)
    {
        m_maxPerSound = std::max(1, _maxPerSound);
        m_slots.assign(static_cast<size_t>(std::max(1, _capacity)), Slot {});
        m_nextSerial = 1;
    }

    [[nodiscard]] Acquired Acquire(
        const int _soundId)
    {
        if (CountSound(_soundId) >= m_maxPerSound)
        {
            const int oldest = FindOldest_(_soundId);
            SLUG_ASSERT(oldest >= 0, "stealable voice not found");
            return Take_(oldest, _soundId, true);
        }

        for (size_t i = 0; i < m_slots.size(); ++i)
        {
            if (!m_slots[i].bActive)
            {
                return Take_(static_cast<int>(i), _soundId, false);
            }
        }

        return Take_(FindOldest_(-1), _soundId, true);
    }

    void Release(
        const int _slot)
    {
        if (_slot >= 0 && static_cast<size_t>(_slot) < m_slots.size())
        {
            m_slots[static_cast<size_t>(_slot)] = Slot {};
        }
    }

    void ReleaseAll()
    {
        for (Slot& slot: m_slots)
        {
            slot = Slot {};
        }
    }

    [[nodiscard]] int  GetCapacity() const { return static_cast<int>(m_slots.size()); }
    [[nodiscard]] int  GetMaxPerSound() const { return m_maxPerSound; }
    [[nodiscard]] bool IsActive(const int _slot) const { return m_slots[static_cast<size_t>(_slot)].bActive; }
    [[nodiscard]] int  GetSoundId(const int _slot) const { return m_slots[static_cast<size_t>(_slot)].soundId; }

    [[nodiscard]] int GetActiveCount() const
    {
        int count = 0;
        for (const Slot& slot: m_slots)
        {
            count += slot.bActive ? 1 : 0;
        }
        return count;
    }

    [[nodiscard]] int CountSound(
        const int _soundId) const
    {
        int count = 0;
        for (const Slot& slot: m_slots)
        {
            count += (slot.bActive && slot.soundId == _soundId) ? 1 : 0;
        }
        return count;
    }

private:
    struct Slot
    {
        bool     bActive = false;
        int      soundId = -1;
        uint64_t serial  = 0;   // 시작 순서 (작을수록 오래됨)
    };

    [[nodiscard]] int FindOldest_(
        const int _soundId) const
    {
        int      best       = -1;
        uint64_t bestSerial = UINT64_MAX;
        for (size_t i = 0; i < m_slots.size(); ++i)
        {
            const Slot& slot = m_slots[i];
            if (slot.bActive && (_soundId < 0 || slot.soundId == _soundId) && slot.serial < bestSerial)
            {
                best       = static_cast<int>(i);
                bestSerial = slot.serial;
            }
        }
        return best;
    }

    [[nodiscard]] Acquired Take_(
        const int  _slot,
        const int  _soundId,
        const bool _bStolen)
    {
        Slot& slot   = m_slots[static_cast<size_t>(_slot)];
        slot.bActive = true;
        slot.soundId = _soundId;
        slot.serial  = m_nextSerial++;
        return { _slot, _bStolen };
    }

    Vector<Slot> m_slots       = {};
    int          m_maxPerSound = 4;
    uint64_t     m_nextSerial  = 1;
};

}   // namespace slug
