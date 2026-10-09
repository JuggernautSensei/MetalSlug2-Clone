#pragma once
#include <algorithm>
#include <cstdint>

#include "Base/Assert.h"
#include "Base/Types.h"

namespace slug
{

// ===========================================
//  VoiceAllocator
//   동시에 울릴 수 있는 효과음 수를 제한하는 순수 슬롯 관리자 (장치/miniaudio 와 무관, 테스트 가능).
//    - 전체 상한(capacity): 가득 차면 가장 오래된 소리를 끊고 그 슬롯을 재사용한다.
//    - 이름별 상한(maxPerSound): 같은 소리가 너무 겹치면(권총 연사 등) 그 소리의 가장 오래된 것을 끊는다.
// ===========================================

class VoiceAllocator
{
public:
    struct Acquired
    {
        int  slot    = -1;
        bool bStolen = false;   // 이미 재생 중이던 소리를 끊고 얻은 슬롯인가
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

    // 새 소리를 위한 슬롯을 얻는다. _soundId 는 SoundBank 인덱스. 항상 슬롯을 돌려준다(필요하면 오래된 소리를 끊는다).
    [[nodiscard]] Acquired Acquire(
        const int _soundId)
    {
        // 1) 이름별 상한: 같은 소리가 가득 차 있으면 그중 가장 오래된 것을 재사용
        if (CountSound(_soundId) >= m_maxPerSound)
        {
            const int oldest = FindOldest_(_soundId);
            SLUG_ASSERT(oldest >= 0, "stealable voice not found");
            return Take_(oldest, _soundId, true);
        }

        // 2) 빈 슬롯
        for (size_t i = 0; i < m_slots.size(); ++i)
        {
            if (!m_slots[i].bActive)
            {
                return Take_(static_cast<int>(i), _soundId, false);
            }
        }

        // 3) 전체 상한: 가장 오래된 소리를 끊는다
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

    // --- 조회 ---
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

    // _soundId < 0 이면 전체에서, 아니면 그 소리 중에서 가장 오래된 슬롯
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

    slug::Vector<Slot> m_slots       = {};
    int               m_maxPerSound = 4;
    uint64_t          m_nextSerial  = 1;
};

}   // namespace slug
