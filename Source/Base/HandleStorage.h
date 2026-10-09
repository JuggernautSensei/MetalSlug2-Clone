#pragma once
#include <cstdint>
#include <optional>

#include "Base/Assert.h"
#include "Base/Types.h"

namespace slug
{

// 핸들 인덱스로 직접 접근하는 저장소. 세대가 다르면 없는 것으로 취급한다.
template<typename HandleT, typename T>
class HandleStorage
{
public:
    T& InsertOrReplace(const HandleT _handle, const T& _value)
    {
        if (_handle.GetIndex() >= m_slots.size())
        {
            m_slots.resize(_handle.GetIndex() + 1);
        }
        Slot& slot = m_slots[_handle.GetIndex()];
        slot.value.emplace(_value);
        slot.token = _handle.GetToken();
        return *slot.value;
    }

    [[nodiscard]] bool Contains(const HandleT _handle) const
    {
        return GetOrNull(_handle) != nullptr;
    }

    [[nodiscard]] T& Get(const HandleT _handle)
    {
        SLUG_ASSERT(Contains(_handle), "Get: missing component");
        return *GetOrNull(_handle);
    }
    [[nodiscard]] const T& Get(const HandleT _handle) const
    {
        SLUG_ASSERT(Contains(_handle), "Get: missing component");
        return *GetOrNull(_handle);
    }

    [[nodiscard]] T* GetOrNull(const HandleT _handle)
    {
        Slot* pSlot = SlotIfMatch_(_handle);
        return pSlot ? &*pSlot->value : nullptr;
    }
    [[nodiscard]] const T* GetOrNull(const HandleT _handle) const
    {
        const Slot* pSlot = SlotIfMatch_(_handle);
        return pSlot ? &*pSlot->value : nullptr;
    }

    void Erase(const HandleT _handle)
    {
        if (Slot* pSlot = SlotIfMatch_(_handle))
        {
            pSlot->value.reset();
        }
    }

    void Clear()
    {
        m_slots.clear();
    }

private:
    struct Slot
    {
        std::optional<T> value;
        uint8_t          token = 0;
    };

    Slot* SlotIfMatch_(const HandleT _handle)
    {
        if (_handle.GetIndex() >= m_slots.size())
        {
            return nullptr;
        }
        Slot& slot = m_slots[_handle.GetIndex()];
        return slot.value && slot.token == _handle.GetToken() ? &slot : nullptr;
    }
    const Slot* SlotIfMatch_(const HandleT _handle) const
    {
        if (_handle.GetIndex() >= m_slots.size())
        {
            return nullptr;
        }
        const Slot& slot = m_slots[_handle.GetIndex()];
        return slot.value && slot.token == _handle.GetToken() ? &slot : nullptr;
    }

    Vector<Slot> m_slots = {};
};

}   // namespace slug
