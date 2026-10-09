#pragma once
#include <cstdint>
#include <vector>

#include "Tools/Baker/Image.h"

namespace slug::baker
{
[[nodiscard]] bool EncodePng(const Image& _image, std::vector<uint8_t>& _outBytes);

// PNG/BMP 등 stb_image 가 읽는 바이트를 RGBA 8bit 로 디코드한다 (알파 없는 입력은 알파 255).
[[nodiscard]] bool DecodeImageBytes(const uint8_t* _pData, size_t _size, Image& _outImage);
}
