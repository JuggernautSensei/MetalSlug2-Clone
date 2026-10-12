#pragma once
#include "Base/SlugError.h"
#include "Base/Types.h"
#include "Game/Player/SlugTypes.h"

// SV-001 수치를 JSON (Assets/Defs/Slug/Slug.json) 에서 읽는다. 없는 키는 SlugTuning 의 기본값을 유지한다.
[[nodiscard]] Result<SlugTuning> ParseSlugTuning(StringView _json);
[[nodiscard]] Result<SlugTuning> LoadSlugTuning(const FilePath& _path);

// 기본값을 JSON 텍스트로 쓴다 (Slug.json 을 만들 때, 그리고 파일과 코드 기본값이 같은지 검증할 때)
[[nodiscard]] String DumpSlugTuningJson(const SlugTuning& _tuning);

[[nodiscard]] FilePath GetDefaultSlugTuningPath(const FilePath& _assetRoot);
