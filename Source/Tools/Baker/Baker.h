#pragma once
#include "Base/Result.h"

#include "Tools/Baker/BakerData.h"

namespace slug::baker
{
[[nodiscard]] slug::Result<void> RunBaker(const BakerOptions& _options);

[[nodiscard]] slug::Result<void> WriteReports(const BakerOptions& _options, BakeState& _state);
}
