#pragma once
#include "Base/Result.h"
#include "Base/Types.h"
#include "Render/Animation.h"

struct SpriteInfo
{
    String    name        = {};
    bool      bStandalone = false;
    String    file        = {};
    int       width       = 0;
    int       height      = 0;
    AnimFrame frame       = {};
};

class SpriteLibrary
{
public:
    [[nodiscard]] static Result<SpriteLibrary> LoadFromJson(StringView _json);
    [[nodiscard]] static Result<SpriteLibrary> LoadFromDirectory(const FilePath& _bakedRoot);

    void                            AddSprite(SpriteInfo _info);
    [[nodiscard]] const SpriteInfo* Find(StringView _name) const;
    [[nodiscard]] size_t            GetCount() const;

private:
    using NameMap = HashMap<String, uint32_t>;

    Vector<SpriteInfo> m_sprites     = {};
    NameMap            m_nameToIndex = {};
};
