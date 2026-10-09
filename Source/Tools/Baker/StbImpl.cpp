#include "Tools/Baker/PngWriter.h"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <stb_image_write.h>

namespace slug::baker
{

namespace
{
    void AppendBytes_(
        void*       _pContext,
        void*       _pData,
        const int   _size)
    {
        auto* pOut = static_cast<std::vector<uint8_t>*>(_pContext);
        auto* pSrc = static_cast<const uint8_t*>(_pData);
        pOut->insert(pOut->end(), pSrc, pSrc + _size);
    }
}

bool EncodePng(
    const Image&          _image,
    std::vector<uint8_t>& _outBytes)
{
    _outBytes.clear();
    if (_image.IsEmpty())
    {
        return false;
    }
    return stbi_write_png_to_func(&AppendBytes_, &_outBytes, _image.width, _image.height, 4, _image.pixels.data(), _image.width * 4) != 0;
}
}
