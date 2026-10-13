#include "PropTestUtil.h"

FilePath PropsJsonPath()
{
    return FindAssetRoot() / "Defs" / "Props" / "Props.json";
}

PropDefLibrary LoadPropLib_()
{
    String                 msg;
    Result<PropDefLibrary> loaded = PropDefLibrary::Load(PropsJsonPath(), &msg);
    if (!loaded)
    {
        ADD_FAILURE() << "Props.json 로드 실패: " << String { msg.c_str() };
        return PropDefLibrary {};
    }
    return loaded.Take();
}

const PropDefLibrary& GetPropLib()
{
    static const PropDefLibrary lib = LoadPropLib_();
    return lib;
}

bool LoadLibs(
    AnimationLibrary& _anims,
    SpriteLibrary&    _sprites)
{
    Result<AnimationLibrary> anims   = AnimationLibrary::LoadFromDirectory(FindAssetRoot());
    Result<SpriteLibrary>    sprites = SpriteLibrary::LoadFromDirectory(FindAssetRoot());
    if (!anims || !sprites)
    {
        return false;
    }
    _anims   = anims.Take();
    _sprites = sprites.Take();
    return true;
}

HitEvent MakeHit(
    uint32_t _target,
    int      _damage,
    eHitKind _kind)
{
    HitEvent ev;
    ev.damage       = _damage;
    ev.kind         = _kind;
    ev.attackerTeam = eTeam::Player;
    ev.attacker     = 1;
    ev.target       = _target;
    return ev;
}

GroundMap MakeFloorMap(
    int _w,
    int _h,
    int _floorY)
{
    GroundMap map = GroundMap::MakeEmpty(_w, _h);
    map.FillRect(0, _floorY, _w - 1, _h - 1, true);
    return map;
}

Body MakeBody(
    VECTOR2 _feet)
{
    Body body;
    body.position = _feet;
    body.size     = { 24.f, 40.f };
    return body;
}
