#pragma once
#include "Base/Result.h"

#include "Tools/Baker/BakerData.h"

namespace slug::baker
{

// ===========================================
//  Baker 파이프라인
// ===========================================

// 레거시 리소스를 읽어 _options.outRoot 에 Baked 에셋(Docs/AssetFormat.md)과 리포트를 생성한다.
[[nodiscard]] slug::Result<void> RunBaker(const BakerOptions& _options);

// 컨택트 시트 / pivot_report.json / bake_summary.txt (BakerReport.cpp)
[[nodiscard]] slug::Result<void> WriteReports(const BakerOptions& _options, BakeState& _state);

}   // namespace slug::baker
