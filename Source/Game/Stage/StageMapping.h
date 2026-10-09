#pragma once
#include <optional>

#include "Base/Types.h"
#include "Base/Math.h"

#include "Game/Props/PropDef.h"
#include "Game/Stage/StageData.h"
#include "Game/Stage/StageHost.h"
#include "World/GroundMap.h"
#include "World/Geometry.h"

namespace slug
{

// ===========================================
//  StageMapping
//   스테이지 스크립트(SpawnRequest) 를 실제 월드의 입력으로 바꾸는 순수 함수들.
//   StageWorldHost / StageState 가 쓴다. 전부 부작용이 없다 (파일 읽기 함수 제외).
// ===========================================

// 스크립트 params (ParamValue 객체) → 소품 스폰 파라미터.
//   hp (숫자), sprites ([idle] 또는 [idle, destroyed]), hitbox / standBox ({size:[w,h], offset:[x,y]}),
//   explodeOnDeath (bool), variant (정수). 없는 키는 def 기본값(-1 / 빈 문자열)이다.
[[nodiscard]] PropSpawnParams MakePropSpawnParams(const ParamValue* _pParams);

// collider 상자: LegacyCenter 면 position 이 중심, Feet 면 바닥 중앙. size 는 [w,h].
[[nodiscard]] Aabb MakeColliderBox(const SpawnRequest& _request);

// 지면 패치 마스크: Baked/Textures/<sprite>.png 의 파란 픽셀(레거시 RGB_BLUE 규칙, 알파 >= 128) 을 고체로 만든다. 나머지는 공기.
// 읽지 못하면 nullopt. (ApplyGroundPatch 는 공기 칸도 지면을 공기로 바꾸므로 집 파괴 시 발자국이 비워진다.)
[[nodiscard]] std::optional<GroundMap> LoadGroundPatchMask(const slug::FilePath& _pngPath);

// 아이템 스폰 기준점: 스크립트의 legacyCenter(상자 중심) → 발 중앙 (32x32 상자 기준 +16)
[[nodiscard]] Vec2 ItemFeetFromCenter(Vec2 _center);

}   // namespace slug
