#include "Audio/VoiceAllocator.h"

VoiceAllocator::VoiceAllocator(
    const int _capacity,
    const int _maxPerSound)
{
    Reset(_capacity, _maxPerSound);
}

void VoiceAllocator::Reset(
    const int _capacity,
    const int _maxPerSound)
{
    m_maxPerSound = Max(1, _maxPerSound);
    m_slots.assign(static_cast<size_t>(Max(1, _capacity)), Slot {});
    m_nextOrder = 1;
}

VoiceAllocator::Acquired VoiceAllocator::Acquire(
    const int _soundId)
{
    int index = -1;
    if (CountSound(_soundId) >= m_maxPerSound)
    {
        index = FindOldest_(_soundId);
        JUG_ASSERT(index >= 0, "stealable voice not found");
    }
    else
    {
        for (size_t i = 0; i < m_slots.size(); ++i)
        {
            if (!m_slots[i].bActive)
            {
                index = static_cast<int>(i);
                break;
            }
        }
        if (index < 0)
        {
            index = FindOldest_(-1);
        }
    }

    Slot&      slot    = m_slots[static_cast<size_t>(index)];
    const bool bStolen = slot.bActive;
    slot              = { true, _soundId, m_nextOrder++ };
    return { index, bStolen };
}

void VoiceAllocator::Release(
    const int _slot)
{
    if (_slot >= 0 && static_cast<size_t>(_slot) < m_slots.size())
    {
        m_slots[static_cast<size_t>(_slot)] = Slot {};
    }
}

int VoiceAllocator::GetCapacity() const
{
    return static_cast<int>(m_slots.size());
}

bool VoiceAllocator::IsActive(
    const int _slot) const
{
    return m_slots[static_cast<size_t>(_slot)].bActive;
}

int VoiceAllocator::GetSoundId(
    const int _slot) const
{
    return m_slots[static_cast<size_t>(_slot)].soundId;
}

int VoiceAllocator::GetActiveCount() const
{
    int count = 0;
    for (const Slot& slot: m_slots)
    {
        count += slot.bActive ? 1 : 0;
    }
    return count;
}

int VoiceAllocator::CountSound(
    const int _soundId) const
{
    int count = 0;
    for (const Slot& slot: m_slots)
    {
        count += (slot.bActive && slot.soundId == _soundId) ? 1 : 0;
    }
    return count;
}

int VoiceAllocator::FindOldest_(
    const int _soundId) const
{
    int      oldest = -1;
    uint64_t order  = UINT64_MAX;
    for (size_t i = 0; i < m_slots.size(); ++i)
    {
        const Slot& slot = m_slots[i];
        if (slot.bActive && (_soundId < 0 || slot.soundId == _soundId) && slot.order < order)
        {
            oldest = static_cast<int>(i);
            order  = slot.order;
        }
    }
    return oldest;
}
