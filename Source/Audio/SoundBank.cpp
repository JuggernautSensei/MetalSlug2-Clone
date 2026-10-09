#include "Audio/SoundBank.h"
#include "Base/Types.h"

#include "Base/Json.h"

#include "Base/Error.h"

namespace slug
{

namespace
{
    [[nodiscard]] slug::Result<SoundBank> Parse_(
        const slug::JsonReader& _root,
        const slug::FilePath&   _rootDir)
    {
        const slug::Result<slug::JsonReader> sounds = _root.FindField("sounds");
        if (!sounds || !sounds->IsObject())
        {
            return slug::Error { eError::Parse };
        }

        SoundBank bank;
        for (const auto [name, json]: sounds->GetObject())
        {
            if (!json.IsObject())
            {
                return slug::Error { eError::Parse };
            }

            const slug::Result<slug::String> file = json.ReadField<slug::String>("file");
            if (!file || file.GetValue().empty())
            {
                return slug::Error { eError::Parse };
            }

            SoundEntry entry;
            entry.name = slug::String { name };
            entry.file = file.GetValue();
            if (json.HasField("bgm"))
            {
                const slug::Result<bool> bBgm = json.ReadField<bool>("bgm");
                if (!bBgm)
                {
                    return slug::Error { eError::Parse };
                }
                entry.bBgm = bBgm.GetValue();
            }
            entry.path = _rootDir / slug::FilePath { entry.file };
            bank.Add(std::move(entry));
        }
        return bank;
    }
}

slug::Result<SoundBank> SoundBank::LoadFromJson(
    const slug::StringView _json,
    const slug::FilePath&  _root)
{
    slug::Result<slug::JsonDeserializer> doc = slug::JsonDeserializer::Load(_json);
    if (!doc)
    {
        return slug::Error { eError::Parse };
    }
    return Parse_(doc->GetReader(), _root);
}

slug::Result<SoundBank> SoundBank::LoadFromDirectory(
    const slug::FilePath& _bakedRoot)
{
    slug::Result<slug::JsonDeserializer> doc = slug::JsonDeserializer::LoadFromFile(_bakedRoot / "sounds.json");
    if (!doc)
    {
        return slug::Error { eError::FileOpen };
    }
    return Parse_(doc->GetReader(), _bakedRoot);
}

void SoundBank::Add(
    SoundEntry _entry)
{
    if (const auto it = m_nameToIndex.find(_entry.name); it != m_nameToIndex.end())
    {
        m_entries[it->second] = std::move(_entry);
        return;
    }
    m_nameToIndex.emplace(_entry.name, static_cast<uint32_t>(m_entries.size()));
    m_entries.push_back(std::move(_entry));
}

const SoundEntry* SoundBank::Find(
    const slug::StringView _name) const
{
    const int index = FindIndex(_name);
    return index < 0 ? nullptr : &m_entries[static_cast<size_t>(index)];
}

int SoundBank::FindIndex(
    const slug::StringView _name) const
{
    const auto it = m_nameToIndex.find(slug::String { _name });
    return it == m_nameToIndex.end() ? -1 : static_cast<int>(it->second);
}
}
