#pragma once
#include "Base/Result.h"
#include "Base/Types.h"

#include "Core/Input.h"

namespace slug
{
struct ReplayData
{
    uint64_t                 seed      = 1;
    slug::String              stateName = "sandbox";
    slug::Vector<InputFrame>  frames    = {};
};

[[nodiscard]] slug::Result<void>       SaveReplay(const slug::FilePath& _path, const ReplayData& _replay);
[[nodiscard]] slug::Result<ReplayData> LoadReplay(const slug::FilePath& _path);

[[nodiscard]] slug::Vector<uint8_t>    SerializeReplay(const ReplayData& _replay);
[[nodiscard]] slug::Result<ReplayData> DeserializeReplay(slug::Span<const uint8_t> _bytes);
}
