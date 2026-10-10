#include "Game/Stage/StageMapping.h"

#include <cstdint>
#include <fstream>

#include "Base/Math.h"
#include "Base/Types.h"

#define STB_IMAGE_STATIC
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

namespace slug
{

namespace
{
    [[nodiscard]] Vec2 ReadVec2_(
        const ParamValue& _value,
        const Vec2        _default)
    {
        if (!_value.IsArray() || _value.GetSize() < 2)
        {
            return _default;
        }
        return { _value.At(0).AsFloat(), _value.At(1).AsFloat() };
    }

    [[nodiscard]] PropBoxOverride ReadBox_(
        const ParamValue& _value)
    {
        PropBoxOverride box = {};
        if (!_value.IsObject())
        {
            return box;
        }
        if (const ParamValue* pSize = _value.Find("size"))
        {
            box.size = ReadVec2_(*pSize, box.size);
        }
        if (const ParamValue* pOffset = _value.Find("offset"))
        {
            box.offset = ReadVec2_(*pOffset, box.offset);
        }
        return box;
    }
}   // namespace

PropSpawnParams MakePropSpawnParams(
    const ParamValue* const _pParams)
{
    PropSpawnParams out = {};
    if (!_pParams || !_pParams->IsObject())
    {
        return out;
    }

    if (const ParamValue* pHp = _pParams->Find("hp"))
    {
        out.hp = pHp->AsInt(-1);
    }
    if (const ParamValue* pSprites = _pParams->Find("sprites"); pSprites && pSprites->IsArray())
    {
        if (pSprites->GetSize() >= 1)
        {
            out.idleSprite = pSprites->At(0).AsString();
        }
        if (pSprites->GetSize() >= 2)
        {
            out.destroySprite = pSprites->At(1).AsString();
        }
    }
    if (const ParamValue* pHitbox = _pParams->Find("hitbox"))
    {
        out.hitbox = ReadBox_(*pHitbox);
    }
    if (const ParamValue* pStandBox = _pParams->Find("standBox"))
    {
        out.standBox = ReadBox_(*pStandBox);
    }
    if (const ParamValue* pExplode = _pParams->Find("explodeOnDeath"))
    {
        out.bExplodeOnDeath = pExplode->AsBool(false);
    }
    if (const ParamValue* pVariant = _pParams->Find("variant"))
    {
        out.variant = pVariant->AsInt(-1);
    }
    return out;
}

Aabb MakeColliderBox(
    const SpawnRequest& _request)
{
    const float w = _request.size.x > 0.f ? _request.size.x : 16.f;
    const float h = _request.size.y > 0.f ? _request.size.y : 16.f;
    if (_request.positionMode == ePositionMode::Feet)
    {
        return Aabb::FromBottomCenter(_request.position, { w, h });
    }
    return Aabb::FromCenterSize(_request.position, { w, h });
}

std::optional<GroundMap> LoadGroundPatchMask(
    const FilePath& _pngPath)
{
    std::ifstream file { _pngPath, std::ios::binary | std::ios::ate };
    if (!file)
    {
        return std::nullopt;
    }
    const std::streamsize size = file.tellg();
    if (size <= 0)
    {
        return std::nullopt;
    }

    Vector<unsigned char> encoded(static_cast<size_t>(size));
    file.seekg(0, std::ios::beg);
    if (!file.read(reinterpret_cast<char*>(encoded.data()), size))
    {
        return std::nullopt;
    }

    int      width    = 0;
    int      height   = 0;
    int      channels = 0;
    stbi_uc* pPixels  = stbi_load_from_memory(encoded.data(), static_cast<int>(encoded.size()), &width, &height, &channels, 4);
    if (!pPixels || width <= 0 || height <= 0)
    {
        return std::nullopt;
    }

    GroundMap mask = GroundMap::MakeEmpty(width, height);
    for (int y = 0; y < height; ++y)
    {
        for (int x = 0; x < width; ++x)
        {
            const size_t index = (static_cast<size_t>(y) * static_cast<size_t>(width) + static_cast<size_t>(x)) * 4;
            const bool   bBlue = pPixels[index] < 80 && pPixels[index + 1] < 80 && pPixels[index + 2] >= 200;
            mask.SetSolid(x, y, bBlue && pPixels[index + 3] >= 128);
        }
    }
    stbi_image_free(pPixels);
    return mask;
}

Vec2 ItemFeetFromCenter(
    const Vec2 _center)
{
    return { _center.x, _center.y + 16.f };
}
}   // namespace slug
