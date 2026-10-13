#pragma once
#include "Base/Types.h"

[[nodiscard]] FilePath FindAssetRoot();

void SetAssetRootFallback(FilePath _root);

[[nodiscard]] Result<Vector<uint8_t>> ReadFileBytes(const FilePath& _path);
[[nodiscard]] Result<String>          ReadFileText(const FilePath& _path);
[[nodiscard]] Result<void>            WriteFileBytes(const FilePath& _path, const MemoryView _bytes);
