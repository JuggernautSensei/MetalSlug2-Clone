#include "Render/SpriteLibrary.h"

#include <cmath>

#include "Base/SlugError.h"
#include "Base/Json.h"
#include "Base/PhaseTimer.h"
#include "Base/Types.h"

namespace
{
[[nodiscard]] bool ReadInt_(
    const JsonReader& _object,
    const StringView  _key,
    int&              _out)
{
    const Result<double> value = _object.ReadField<double>(_key);
    if (!value)
    {
        return false;
    }
    _out = static_cast<int>(std::llround(value.GetValue()));
    return true;
}

[[nodiscard]] Result<SpriteLibrary> Parse_(
    const JsonReader& _root)
{
    const Result<JsonReader> sprites = _root.FindField("sprites");
    if (!sprites || !sprites->IsObject())
    {
        return Error { eError::Parse };
    }

    SpriteLibrary library;
    for (const std::pair<StringView, JsonReader>& item: sprites->GetObject())
    {
        const StringView  name = item.first;
        const JsonReader& json = item.second;
        if (!json.IsObject())
        {
            return Error { eError::Parse };
        }

        SpriteInfo info;
        info.name = String { name };
        if (json.HasField("file"))
        {
            const Result<String> file = json.ReadField<String>("file");
            if (!file || !ReadInt_(json, "w", info.width) || !ReadInt_(json, "h", info.height))
            {
                return Error { eError::Parse };
            }
            info.bStandalone = true;
            info.file        = file.GetValue();
        }
        else
        {
            AnimFrame& f = info.frame;
            if (!ReadInt_(json, "atlas", f.atlas) || !ReadInt_(json, "x", f.x) || !ReadInt_(json, "y", f.y) || !ReadInt_(json, "w", f.w) || !ReadInt_(json, "h", f.h) || !ReadInt_(json, "pivotX", f.pivotX) || !ReadInt_(json, "pivotY", f.pivotY))
            {
                return Error { eError::Parse };
            }
            info.width  = f.w;
            info.height = f.h;
        }
        library.AddSprite(std::move(info));
    }

    return library;
}
}   // namespace

Result<SpriteLibrary> SpriteLibrary::LoadFromJson(
    const StringView _json)
{
    Result<JsonDeserializer> doc = JsonDeserializer::Load(_json);
    if (!doc)
    {
        return Error { eError::Parse };
    }
    return Parse_(doc->GetReader());
}

Result<SpriteLibrary> SpriteLibrary::LoadFromDirectory(
    const FilePath& _bakedRoot)
{
    Result<JsonDeserializer> doc = JsonDeserializer::LoadFromFile(_bakedRoot / "sprites.json");
    if (!doc)
    {
        return Error { eError::FileOpen };
    }
    const ScopedPhaseTimer timer { "sprites-parse", "sprites.json" };
    return Parse_(doc->GetReader());
}

void SpriteLibrary::AddSprite(
    SpriteInfo _info)
{
    if (const NameMap::iterator it = m_nameToIndex.find(_info.name); it != m_nameToIndex.end())
    {
        m_sprites[it->second] = std::move(_info);
        return;
    }
    m_nameToIndex.emplace(_info.name, static_cast<uint32_t>(m_sprites.size()));
    m_sprites.push_back(std::move(_info));
}

const SpriteInfo* SpriteLibrary::Find(
    const StringView _name) const
{
    const NameMap::const_iterator it = m_nameToIndex.find(String { _name });
    return it == m_nameToIndex.end() ? nullptr : &m_sprites[it->second];
}

size_t SpriteLibrary::GetCount() const
{
    return m_sprites.size();
}
