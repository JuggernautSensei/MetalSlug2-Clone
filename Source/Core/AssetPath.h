#pragma once
#include "Base/Types.h"

namespace slug
{
// Assets 루트 폴더를 찾는다.
// 실행 파일 폴더에서 시작해 상위 5단계까지 올라가며 Assets\Baked\manifest.json 또는 Assets\Tuning\Tuning.json 을 가진
// 폴더의 Assets 를 돌려준다. 찾지 못하면 SetAssetRootFallback 으로 등록된 경로, 그것도 없으면 작업 폴더의 Assets 를 쓴다.
[[nodiscard]] FilePath FindAssetRoot();

// 도구/테스트 전용 최후 폴백 경로를 등록한다. 탐색이 실패했을 때만 사용된다.
void SetAssetRootFallback(
    FilePath _root);
} // namespace slug
