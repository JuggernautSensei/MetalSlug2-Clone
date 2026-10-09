#pragma once
#include <string>

#include <gtest/gtest.h>

#include "Game/Combat/CombatSystem.h"
#include "Game/Props/ItemWorld.h"
#include "Game/Props/PropDef.h"
#include "Game/Props/PropWorld.h"
#include "Render/Animation.h"
#include "Render/SpriteLibrary.h"
#include "World/GroundMap.h"
#include "World/Physics.h"

// ===========================================
//  소품 테스트 도우미 (Props.json 로드, 피격 이벤트 생성, 평지 맵)
// ===========================================

namespace slugtest
{

using namespace slug;

inline jug::FilePath PropsJsonPath()
{
    return jug::FilePath { SLUG_ASSET_DIR } / "Defs" / "Props" / "Props.json";
}

// Props.json 은 저장소에 항상 있으므로 실패하면 테스트가 실패하도록 빈 라이브러리를 돌려준다.
inline const PropDefLibrary& GetPropLib()
{
    static const PropDefLibrary lib = []()
    {
        jug::String                     msg;
        jug::Result<PropDefLibrary>     loaded = PropDefLibrary::Load(PropsJsonPath(), &msg);
        if (!loaded)
        {
            ADD_FAILURE() << "Props.json 로드 실패: " << std::string { msg.c_str() };
            return PropDefLibrary {};
        }
        return loaded.Take();
    }();
    return lib;
}

// Baked 애셋 (애니/스프라이트). 없으면 false (테스트는 GTEST_SKIP 해야 한다).
inline bool LoadBakedLibs(
    AnimationLibrary& _anims,
    SpriteLibrary&    _sprites)
{
    jug::Result<AnimationLibrary> anims = AnimationLibrary::LoadFromDirectory(jug::FilePath { SLUG_ASSET_DIR } / "Baked");
    jug::Result<SpriteLibrary>    sprites = SpriteLibrary::LoadFromDirectory(jug::FilePath { SLUG_ASSET_DIR } / "Baked");
    if (!anims || !sprites)
    {
        return false;
    }
    _anims   = anims.Take();
    _sprites = sprites.Take();
    return true;
}

// 피격 이벤트 (판정 해소 결과를 흉내냄). 공격자는 플레이어(1).
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

// 바닥 (floorY 부터 아래가 고체) 이 있는 빈 맵
inline GroundMap MakeFloorMap(
    const int _w,
    const int _h,
    const int _floorY)
{
    GroundMap map = GroundMap::MakeEmpty(_w, _h);
    map.FillRect(0, _floorY, _w - 1, _h - 1, true);
    return map;
}

// 엔진 기본 Body (플레이어 근사: 24x40)
inline Body MakeBody(
    const jug::VECTOR2 _feet)
{
    Body body;
    body.position = _feet;
    body.size     = { 24.f, 40.f };
    return body;
}

}   // namespace slugtest
