#pragma once
#include "Base/Result.h"
#include "Base/Types.h"

namespace slug
{

struct SoundEntry
{
    String   name = {};
    String   file = {};
    bool     bBgm = false;
    FilePath path = {};
};

class SoundBank
{
public:
    [[nodiscard]] static Result<SoundBank> LoadFromJson(StringView _json, const FilePath& _root = {});
    [[nodiscard]] static Result<SoundBank> LoadFromDirectory(const FilePath& _bakedRoot);

    void Add(SoundEntry _entry);

    [[nodiscard]] const SoundEntry*      Find(StringView _name) const;
    [[nodiscard]] int                    FindIndex(StringView _name) const;
    [[nodiscard]] const SoundEntry&      Get(int _index) const { return m_entries[static_cast<size_t>(_index)]; }
    [[nodiscard]] size_t                 GetCount() const { return m_entries.size(); }
    [[nodiscard]] bool                   IsEmpty() const { return m_entries.empty(); }
    [[nodiscard]] Span<const SoundEntry> GetEntries() const { return m_entries; }

private:
    using NameMap = HashMap<String, uint32_t>;

    Vector<SoundEntry> m_entries     = {};
    NameMap            m_nameToIndex = {};
};

}   // namespace slug
