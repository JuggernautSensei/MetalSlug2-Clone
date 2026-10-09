#pragma once
#include "Core/AssetPath.h"
#include <string>
#include "Base/Types.h"
#include "Base/Math.h"
#include "Base/Result.h"

#include <gtest/gtest.h>

#include "Game/Combat/CombatSystem.h"
#include "Game/Props/ItemWorld.h"
#include "Game/Props/PropDef.h"
#include "Game/Props/PropWorld.h"
#include "Render/Animation.h"
#include "Render/SpriteLibrary.h"
#include "World/GroundMap.h"
#include "World/Physics.h"

namespace slugtest
{

using namespace slug;

inline slug::FilePath PropsJsonPath()
{
    return slug::FindAssetRoot() / "Defs" / "Props" / "Props.json";
}

inline const PropDefLibrary& GetPropLib()
{
    static const PropDefLibrary lib = []()
    {
        slug::String                     msg;
        slug::Result<PropDefLibrary>     loaded = PropDefLibrary::Load(PropsJsonPath(), &msg);
        if (!loaded)
        {
            ADD_FAILURE() << "Props.json 로드 실패: " << std::string { msg.c_str() };
            return PropDefLibrary {};
        }
        return loaded.Take();
    }();
    return lib;
}

inline bool LoadBakedLibs(
    AnimationLibrary& _anims,
    SpriteLibrary&    _sprites)
{
    slug::Result<AnimationLibrary> anims = AnimationLibrary::LoadFromDirectory(slug::FindAssetRoot() / "Baked");
    slug::Result<SpriteLibrary>    sprites = SpriteLibrary::LoadFromDirectory(slug::FindAssetRoot() / "Baked");
    if (!anims || !sprites)
    {
        return false;
    }
    _anims   = anims.Take();
    _sprites = sprites.Take();
    return true;
}

inline HitEvent MakeHit(
    const uint32_t _target,
    const int      _damage,
    const eHitKind _kind = eHitKind::Bullet)
{
    HitEvent ev;
    ev.damage       = _damage;
    ev.kind         = _kind;
    ev.attackerTeam = eTeam::Player;
    ev.attacker     = 1;
    ev.target       = _target;
    return ev;
}

inline GroundMap MakeFloorMap(
    const int _w,
    const int _h,
    const int _floorY)
{
    GroundMap map = GroundMap::MakeEmpty(_w, _h);
    map.FillRect(0, _floorY, _w - 1, _h - 1, true);
    return map;
}

inline Body MakeBody(
    const Vec2 _feet)
{
    Body body;
    body.position = _feet;
    body.size     = { 24.f, 40.f };
    return body;
}
}
