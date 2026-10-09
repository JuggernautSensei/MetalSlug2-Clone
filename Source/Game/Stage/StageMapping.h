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
[[nodiscard]] PropSpawnParams MakePropSpawnParams(const ParamValue* _pParams);

[[nodiscard]] Aabb MakeColliderBox(const SpawnRequest& _request);

[[nodiscard]] std::optional<GroundMap> LoadGroundPatchMask(const slug::FilePath& _pngPath);

[[nodiscard]] Vec2 ItemFeetFromCenter(Vec2 _center);
}
