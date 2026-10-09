#include "Render/SpriteLibrary.h"

#include <cmath>

#include <JugX/JsonDeserializer.h>

#include "Core/Error.h"

namespace slug
{

namespace
{
    [[nodiscard]] bool ReadInt_(
        const jug::JsonReader& _object,
        const jug::StringView  _key,
        int&                   _out)
    {
        const jug::Result<double> value = _object.ReadField<double>(_key);
        if (!value)
        {
            return false;
        }
        _out = static_cast<int>(std::llround(value.GetValue()));
        return true;
    }

    [[nodiscard]] jug::Result<SpriteLibrary> Parse_(
        const jug::JsonReader& _root)
    {
        const jug::Result<jug::JsonReader> sprites = _root.FindField("sprites");
        if (!sprites || !sprites->IsObject())
        {
            return jug::Error { eError::Parse };
        }

        SpriteLibrary library;
        for (const auto [name, json]: sprites->GetObject())
        {
            if (!json.IsObject())
            {
                return jug::Error { eError::Parse };
            }

            SpriteInfo info;
            info.name = jug::String { name };
            if (json.HasField("file"))
            {
                const jug::Result<jug::String> file = json.ReadField<jug::String>("file");
                if (!file || !ReadInt_(json, "w", info.width) || !ReadInt_(json, "h", info.height))
                {
                    return jug::Error { eError::Parse };
                }
                info.bStandalone = true;
                info.file        = file.GetValue();
            }
            else
            {
                AnimFrame& f = info.frame;
                if (!ReadInt_(json, "atlas", f.atlas) || !ReadInt_(json, "x", f.x) || !ReadInt_(json, "y", f.y) || !ReadInt_(json, "w", f.w) || !ReadInt_(json, "h", f.h) || !ReadInt_(json, "pivotX", f.pivotX) || !ReadInt_(json, "pivotY", f.pivotY))
                {
                    return jug::Error { eError::Parse };
                }
                info.width  = f.w;
                info.height = f.h;
            }
            library.AddSprite(std::move(info));
        }

        return library;
    }
}   // namespace

jug::Result<SpriteLibrary> SpriteLibrary::LoadFromJson(
    const jug::StringView _json)
{
    jug::Result<jug::JsonDeserializer> doc = jug::JsonDeserializer::Load(_json);
    if (!doc)
    {
        return jug::Error { eError::Parse };
    }
    return Parse_(doc->GetReader());
}

jug::Result<SpriteLibrary> SpriteLibrary::LoadFromDirectory(
    const jug::FilePath& _bakedRoot)
{
    jug::Result<jug::JsonDeserializer> doc = jug::JsonDeserializer::LoadFromFile(_bakedRoot / "sprites.json");
    if (!doc)
    {
        return jug::Error { eError::FileOpen };
    }
    return Parse_(doc->GetReader());
}

void SpriteLibrary::AddSprite(
    SpriteInfo _info)
{
    if (const auto it = m_nameToIndex.find(_info.name); it != m_nameToIndex.end())
    {
        m_sprites[it->second] = std::move(_info);
        return;
    }
    m_nameToIndex.emplace(_info.name, static_cast<uint32_t>(m_sprites.size()));
    m_sprites.push_back(std::move(_info));
}

const SpriteInfo* SpriteLibrary::Find(
    const jug::StringView _name) const
{
    const auto it = m_nameToIndex.find(jug::String { _name });
    return it == m_nameToIndex.end() ? nullptr : &m_sprites[it->second];
}

}   // namespace slug
