#pragma once
#include <JugX/Result.h>
#include <JugX/Typedef.h>

#include "Render/Animation.h"

namespace slug
{

// ===========================================
//  SpriteInfo / SpriteLibrary
//   sprites.json (Docs/AssetFormat.md) 를 읽어 정적 스프라이트를 이름으로 찾는다.
//    - 아틀라스 사각형 스프라이트: frame 이 AnimFrame 이므로 SpriteBatch::DrawFrame/Submit 에 그대로 쓴다.
//    - 독립 파일(큰 배경): bStandalone = true, file 이 Baked 루트 기준 상대 경로.
// ===========================================

struct SpriteInfo
{
    jug::String name        = {};
    bool        bStandalone = false;
    jug::String file        = {};
    int         width       = 0;
    int         height      = 0;
    AnimFrame   frame       = {};   // bStandalone == false 일 때만 유효
};

class SpriteLibrary
{
public:
    [[nodiscard]] static jug::Result<SpriteLibrary> LoadFromJson(jug::StringView _json);
    [[nodiscard]] static jug::Result<SpriteLibrary> LoadFromDirectory(const jug::FilePath& _bakedRoot);

    void                            AddSprite(SpriteInfo _info);
    [[nodiscard]] const SpriteInfo* Find(jug::StringView _name) const;
    [[nodiscard]] size_t            GetCount() const { return m_sprites.size(); }

private:
    jug::Vector<SpriteInfo>             m_sprites     = {};
    jug::HashMap<jug::String, uint32_t> m_nameToIndex = {};
};

}   // namespace slug
