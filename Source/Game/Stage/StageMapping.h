#pragma once
#include <optional>

#include "Base/Math.h"
#include "Base/Types.h"
#include "Game/Props/PropDef.h"
#include "Game/Stage/StageData.h"
#include "Game/Stage/StageHost.h"
#include "World/Geometry.h"
#include "World/GroundMap.h"

namespace slug
{

[[nodiscard]] PropSpawnParams MakePropSpawnParams(const ParamValue* _pParams);

[[nodiscard]] Aabb MakeColliderBox(const SpawnRequest& _request);

[[nodiscard]] std::optional<GroundMap> LoadGroundPatchMask(const FilePath& _pngPath);

[[nodiscard]] Vec2 ItemFeetFromCenter(Vec2 _center);

}   // namespace slug
