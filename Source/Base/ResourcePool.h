#pragma once
#include <cstdint>
#include <optional>

#include "Base/Assert.h"
#include "Base/Types.h"

namespace slug
{
template<typename HandleT, typename T>
class ResourcePool
{
public:
    [[nodiscard]] HandleT Emplace()
    {
        uint32_t index = 0;
        if (!m_freeList.empty())
        {
            index = m_freeList.back();
            m_freeList.pop_back();
        }
        else
        {
            index = static_cast<uint32_t>(m_slots.size());
            m_slots.emplace_back();
        }
        Slot& slot = m_slots[index];
        slot.value.emplace();
        ++m_aliveCount;
        return HandleT { index, slot.token };
    }

    void Erase(const HandleT _handle)
    {
        SLUG_ASSERT(IsValid(_handle), "Erase: invalid handle");
        if (!IsValid(_handle))
        {
            return;
        }
        Slot& slot = m_slots[_handle.GetIndex()];
        slot.value.reset();
        ++slot.token;
        --m_aliveCount;
        m_freeList.push_back(_handle.GetIndex());
    }

    [[nodiscard]] bool IsValid(const HandleT _handle) const
    {
        if (_handle.IsNull() || _handle.GetIndex() >= m_slots.size())
        {
            return false;
        }
        const Slot& slot = m_slots[_handle.GetIndex()];
        return slot.value.has_value() && slot.token == _handle.GetToken();
    }

    [[nodiscard]] T& Get(const HandleT _handle)
    {
        SLUG_ASSERT(IsValid(_handle), "Get: invalid handle");
        return *m_slots[_handle.GetIndex()].value;
    }
    [[nodiscard]] const T& Get(const HandleT _handle) const
    {
        SLUG_ASSERT(IsValid(_handle), "Get: invalid handle");
        return *m_slots[_handle.GetIndex()].value;
    }

    [[nodiscard]] T* GetOrNull(const HandleT _handle)
    {
        return IsValid(_handle) ? &*m_slots[_handle.GetIndex()].value : nullptr;
    }
    [[nodiscard]] const T* GetOrNull(const HandleT _handle) const
    {
        return IsValid(_handle) ? &*m_slots[_handle.GetIndex()].value : nullptr;
    }

    [[nodiscard]] size_t size() const { return m_aliveCount; }

    void clear()
    {
        m_slots.clear();
        m_freeList.clear();
        m_aliveCount = 0;
    }

private:
    struct Slot
    {
        std::optional<T> value;
        uint8_t          token = 0;
    };

    Vector<Slot>     m_slots      = {};
    Vector<uint32_t> m_freeList   = {};
    size_t           m_aliveCount = 0;
};
}
