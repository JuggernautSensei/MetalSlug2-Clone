#pragma once
#include "Base/Result.h"
#include "Base/Types.h"

namespace slug
{
struct SoundEntry
{
    slug::String   name = {};
    slug::String   file = {};
    bool          bBgm = false;
    slug::FilePath path = {};
};

class SoundBank
{
public:
    [[nodiscard]] static slug::Result<SoundBank> LoadFromJson(slug::StringView _json, const slug::FilePath& _root = {});
    [[nodiscard]] static slug::Result<SoundBank> LoadFromDirectory(const slug::FilePath& _bakedRoot);

    void Add(SoundEntry _entry);

    [[nodiscard]] const SoundEntry*           Find(slug::StringView _name) const;
    [[nodiscard]] int                         FindIndex(slug::StringView _name) const;
    [[nodiscard]] const SoundEntry&           Get(const int _index) const { return m_entries[static_cast<size_t>(_index)]; }
    [[nodiscard]] size_t                      GetCount() const { return m_entries.size(); }
    [[nodiscard]] bool                        IsEmpty() const { return m_entries.empty(); }
    [[nodiscard]] slug::Span<const SoundEntry> GetEntries() const { return m_entries; }

private:
    slug::Vector<SoundEntry>             m_entries     = {};
    slug::HashMap<slug::String, uint32_t> m_nameToIndex = {};
};
}
