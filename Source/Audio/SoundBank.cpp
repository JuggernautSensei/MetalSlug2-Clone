#include "Audio/SoundBank.h"

#include "Base/SlugError.h"
#include "Base/Json.h"
#include "Base/Types.h"

namespace
{
[[nodiscard]] Result<SoundBank> Parse_(
    const JsonReader& _root,
    const FilePath&   _rootDir)
{
    const Result<JsonReader> sounds = _root.FindField("sounds");
    if (!sounds || !sounds->IsObject())
    {
        return Error { eError::Parse };
    }

    SoundBank bank;
    for (const std::pair<StringView, JsonReader>& item: sounds->GetObject())
    {
        const StringView  name = item.first;
        const JsonReader& json = item.second;
        if (!json.IsObject())
        {
            return Error { eError::Parse };
        }

        const Result<String> file = json.ReadField<String>("file");
        if (!file || file.GetValue().empty())
        {
            return Error { eError::Parse };
        }

        SoundEntry entry;
        entry.name = String { name };
        entry.file = file.GetValue();
        if (json.HasField("bgm"))
        {
            const Result<bool> bBgm = json.ReadField<bool>("bgm");
            if (!bBgm)
            {
                return Error { eError::Parse };
            }
            entry.bBgm = bBgm.GetValue();
        }
        entry.path = _rootDir / FilePath { entry.file };
        bank.Add(std::move(entry));
    }
    return bank;
}
}   // namespace

Result<SoundBank> SoundBank::LoadFromJson(
    const StringView _json,
    const FilePath&  _root)
{
    Result<JsonDeserializer> doc = JsonDeserializer::Load(_json);
    if (!doc)
    {
        return Error { eError::Parse };
    }
    return Parse_(doc->GetReader(), _root);
}

Result<SoundBank> SoundBank::LoadFromDirectory(
    const FilePath& _bakedRoot)
{
    Result<JsonDeserializer> doc = JsonDeserializer::LoadFromFile(_bakedRoot / "sounds.json");
    if (!doc)
    {
        return Error { eError::FileOpen };
    }
    return Parse_(doc->GetReader(), _bakedRoot);
}

void SoundBank::Add(
    SoundEntry _entry)
{
    if (const NameMap::iterator it = m_nameToIndex.find(_entry.name); it != m_nameToIndex.end())
    {
        m_entries[it->second] = std::move(_entry);
        return;
    }
    m_nameToIndex.emplace(_entry.name, static_cast<uint32_t>(m_entries.size()));
    m_entries.push_back(std::move(_entry));
}

const SoundEntry* SoundBank::Find(
    const StringView _name) const
{
    const int index = FindIndex(_name);
    return index < 0 ? nullptr : &m_entries[static_cast<size_t>(index)];
}

int SoundBank::FindIndex(
    const StringView _name) const
{
    const NameMap::const_iterator it = m_nameToIndex.find(String { _name });
    return it == m_nameToIndex.end() ? -1 : static_cast<int>(it->second);
}

const SoundEntry& SoundBank::Get(int _index) const
{
    return m_entries[static_cast<size_t>(_index)];
}

size_t SoundBank::GetCount() const
{
    return m_entries.size();
}

bool SoundBank::IsEmpty() const
{
    return m_entries.empty();
}

Span<const SoundEntry> SoundBank::GetEntries() const
{
    return m_entries;
}
