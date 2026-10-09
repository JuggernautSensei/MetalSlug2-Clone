#include "Render/SpriteLibrary.h"
#include "Base/Types.h"

#include <cmath>

#include "Base/Json.h"

#include "Base/Error.h"

namespace slug
{

namespace
{
    [[nodiscard]] bool ReadInt_(
        const slug::JsonReader& _object,
        const slug::StringView  _key,
        int&                   _out)
    {
        const slug::Result<double> value = _object.ReadField<double>(_key);
        if (!value)
        {
            return false;
        }
        _out = static_cast<int>(std::llround(value.GetValue()));
        return true;
    }

    [[nodiscard]] slug::Result<SpriteLibrary> Parse_(
        const slug::JsonReader& _root)
    {
        const slug::Result<slug::JsonReader> sprites = _root.FindField("sprites");
        if (!sprites || !sprites->IsObject())
        {
            return slug::Error { eError::Parse };
        }

        SpriteLibrary library;
        for (const auto [name, json]: sprites->GetObject())
        {
            if (!json.IsObject())
            {
                return slug::Error { eError::Parse };
            }

            SpriteInfo info;
            info.name = slug::String { name };
            if (json.HasField("file"))
            {
                const slug::Result<slug::String> file = json.ReadField<slug::String>("file");
                if (!file || !ReadInt_(json, "w", info.width) || !ReadInt_(json, "h", info.height))
                {
                    return slug::Error { eError::Parse };
                }
                info.bStandalone = true;
                info.file        = file.GetValue();
            }
            else
            {
                AnimFrame& f = info.frame;
                if (!ReadInt_(json, "atlas", f.atlas) || !ReadInt_(json, "x", f.x) || !ReadInt_(json, "y", f.y) || !ReadInt_(json, "w", f.w) || !ReadInt_(json, "h", f.h) || !ReadInt_(json, "pivotX", f.pivotX) || !ReadInt_(json, "pivotY", f.pivotY))
                {
                    return slug::Error { eError::Parse };
                }
                info.width  = f.w;
                info.height = f.h;
            }
            library.AddSprite(std::move(info));
        }

        return library;
    }
}   // namespace

slug::Result<SpriteLibrary> SpriteLibrary::LoadFromJson(
    const slug::StringView _json)
{
    slug::Result<slug::JsonDeserializer> doc = slug::JsonDeserializer::Load(_json);
    if (!doc)
    {
        return slug::Error { eError::Parse };
    }
    return Parse_(doc->GetReader());
}

slug::Result<SpriteLibrary> SpriteLibrary::LoadFromDirectory(
    const slug::FilePath& _bakedRoot)
{
    slug::Result<slug::JsonDeserializer> doc = slug::JsonDeserializer::LoadFromFile(_bakedRoot / "sprites.json");
    if (!doc)
    {
        return slug::Error { eError::FileOpen };
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
    const slug::StringView _name) const
{
    const auto it = m_nameToIndex.find(slug::String { _name });
    return it == m_nameToIndex.end() ? nullptr : &m_sprites[it->second];
}

}   // namespace slug
