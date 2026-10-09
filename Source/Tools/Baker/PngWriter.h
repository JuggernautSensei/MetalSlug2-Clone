#pragma once
#include <cstdint>
#include <vector>

#include "Tools/Baker/Image.h"

namespace slug::baker
{

// stb_image_write 로 RGBA8 이미지를 PNG 바이트로 인코딩한다 (구현은 StbImpl.cpp).
[[nodiscard]] bool EncodePng(const Image& _image, std::vector<uint8_t>& _outBytes);

}   // namespace slug::baker
