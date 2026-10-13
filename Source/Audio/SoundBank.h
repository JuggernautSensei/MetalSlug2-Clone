#pragma once
#include "Base/SlugError.h"
#include "Base/Types.h"

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
    void                                   Add(SoundEntry _entry);
    [[nodiscard]] const SoundEntry*        Find(StringView _name) const;
    [[nodiscard]] int                      FindIndex(StringView _name) const;
    [[nodiscard]] const SoundEntry&        Get(int _index) const;
    [[nodiscard]] size_t                   GetCount() const;
    [[nodiscard]] Span<const SoundEntry>   GetEntries() const;

private:
    using NameMap = HashMap<String, uint32_t>;

    Vector<SoundEntry> m_entries     = {};
    NameMap            m_nameToIndex = {};
};
