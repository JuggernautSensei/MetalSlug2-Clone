#pragma once
#include <cstdint>
#include <vector>

#include "Tools/Baker/Image.h"

namespace slug::baker
{
[[nodiscard]] bool EncodePng(const Image& _image, std::vector<uint8_t>& _outBytes);
}
