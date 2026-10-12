#pragma once
#include "Base/Types.h"

// Assets 루트: 실행 파일 폴더에서 상위 5단계까지 올라가며 manifest.json 또는 Tuning/Tuning.json 이 있는 Assets, 없으면 폴백 경로, 그것도 없으면 작업 폴더의 Assets
[[nodiscard]] FilePath FindAssetRoot();

void SetAssetRootFallback(FilePath _root);
