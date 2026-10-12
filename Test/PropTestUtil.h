// Prop/Item 테스트 공용 도우미: Props.json 로드, 베이크 라이브러리 로드, 히트/바닥/바디 생성.
#pragma once
#include <string>

#include <gtest/gtest.h>

#include "Base/Math.h"
#include "Base/SlugError.h"
#include "Base/Types.h"
#include "Core/AssetPath.h"
#include "Game/Combat/CombatSystem.h"
#include "Game/Props/ItemWorld.h"
#include "Game/Props/PropDef.h"
#include "Game/Props/PropWorld.h"
#include "Render/Animation.h"
#include "Render/SpriteLibrary.h"
#include "World/GroundMap.h"
#include "World/Physics.h"


[[nodiscard]] FilePath PropsJsonPath();

[[nodiscard]] PropDefLibrary LoadPropLib_();

[[nodiscard]] const PropDefLibrary& GetPropLib();

[[nodiscard]] bool LoadLibs(
    AnimationLibrary& _anims,
    SpriteLibrary&    _sprites);

[[nodiscard]] HitEvent MakeHit(
    const uint32_t _target,
    const int      _damage,
    const eHitKind _kind = eHitKind::Bullet);

[[nodiscard]] GroundMap MakeFloorMap(
    const int _w,
    const int _h,
    const int _floorY);

[[nodiscard]] Body MakeBody(
    const Vec2 _feet);
