#pragma once
#include "Base/Result.h"
#include "Base/Types.h"
#include "Core/Input.h"

struct ReplayData
{
    uint64_t           seed      = 1;
    String             stateName = "sandbox";
    Vector<InputFrame> frames    = {};
};

[[nodiscard]] Result<void>       SaveReplay(const FilePath& _path, const ReplayData& _replay);
[[nodiscard]] Result<ReplayData> LoadReplay(const FilePath& _path);

[[nodiscard]] Vector<uint8_t>    SerializeReplay(const ReplayData& _replay);
[[nodiscard]] Result<ReplayData> DeserializeReplay(Span<const uint8_t> _bytes);
