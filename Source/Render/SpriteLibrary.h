#pragma once
#include "Base/Result.h"
#include "Base/Types.h"

#include "Render/Animation.h"

namespace slug
{
struct SpriteInfo
{
    slug::String name        = {};
    bool        bStandalone = false;
    slug::String file        = {};
    int         width       = 0;
    int         height      = 0;
    AnimFrame   frame       = {};
};

class SpriteLibrary
{
public:
    [[nodiscard]] static slug::Result<SpriteLibrary> LoadFromJson(slug::StringView _json);
    [[nodiscard]] static slug::Result<SpriteLibrary> LoadFromDirectory(const slug::FilePath& _bakedRoot);

    void                            AddSprite(SpriteInfo _info);
    [[nodiscard]] const SpriteInfo* Find(slug::StringView _name) const;
    [[nodiscard]] size_t            GetCount() const { return m_sprites.size(); }

private:
    slug::Vector<SpriteInfo>             m_sprites     = {};
    slug::HashMap<slug::String, uint32_t> m_nameToIndex = {};
};
}
