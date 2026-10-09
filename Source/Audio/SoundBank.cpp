#include "Audio/SoundBank.h"

#include <JugX/JsonDeserializer.h>

#include "Core/Error.h"

namespace slug
{

namespace
{
    [[nodiscard]] jug::Result<SoundBank> Parse_(
        const jug::JsonReader& _root,
        const jug::FilePath&   _rootDir)
    {
        const jug::Result<jug::JsonReader> sounds = _root.FindField("sounds");
        if (!sounds || !sounds->IsObject())
        {
            return jug::Error { eError::Parse };
        }

        SoundBank bank;
        for (const auto [name, json]: sounds->GetObject())
        {
            if (!json.IsObject())
            {
                return jug::Error { eError::Parse };
            }

            const jug::Result<jug::String> file = json.ReadField<jug::String>("file");
            if (!file || file.GetValue().empty())
            {
                return jug::Error { eError::Parse };
            }

            SoundEntry entry;
            entry.name = jug::String { name };
            entry.file = file.GetValue();
            if (json.HasField("bgm"))
            {
                const jug::Result<bool> bBgm = json.ReadField<bool>("bgm");
                if (!bBgm)
                {
                    return jug::Error { eError::Parse };
                }
                entry.bBgm = bBgm.GetValue();
            }
            entry.path = _rootDir / jug::FilePath { entry.file };
            bank.Add(std::move(entry));
        }
        return bank;
    }
}   // namespace

jug::Result<SoundBank> SoundBank::LoadFromJson(
    const jug::StringView _json,
    const jug::FilePath&  _root)
{
    jug::Result<jug::JsonDeserializer> doc = jug::JsonDeserializer::Load(_json);
    if (!doc)
    {
        return jug::Error { eError::Parse };
    }
    return Parse_(doc->GetReader(), _root);
}

jug::Result<SoundBank> SoundBank::LoadFromDirectory(
    const jug::FilePath& _bakedRoot)
{
    jug::Result<jug::JsonDeserializer> doc = jug::JsonDeserializer::LoadFromFile(_bakedRoot / "sounds.json");
    if (!doc)
    {
        return jug::Error { eError::FileOpen };
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
    const jug::StringView _name) const
{
    const int index = FindIndex(_name);
    return index < 0 ? nullptr : &m_entries[static_cast<size_t>(index)];
}

int SoundBank::FindIndex(
    const jug::StringView _name) const
{
    const auto it = m_nameToIndex.find(jug::String { _name });
    return it == m_nameToIndex.end() ? -1 : static_cast<int>(it->second);
}

}   // namespace slug
