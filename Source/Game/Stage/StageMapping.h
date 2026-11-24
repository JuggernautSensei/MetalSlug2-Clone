#pragma once
#include <optional>

#include "Base/Types.h"
#include "Game/Props/PropDef.h"
#include "Game/Stage/StageData.h"
#include "Game/Stage/StageHost.h"
#include "World/Geometry.h"
#include "World/GroundMap.h"

[[nodiscard]] PropSpawnParams MakePropSpawnParams(const ParamValue* _pParams, int _z = kDefaultZ);

[[nodiscard]] Aabb MakeColliderBox(const SpawnRequest& _request);

[[nodiscard]] std::optional<GroundMap> LoadGroundMask(const FilePath& _pngPath);

[[nodiscard]] VECTOR2 ItemFeetFromCenter(VECTOR2 _center);
