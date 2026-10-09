#include "Tools/Baker/PngWriter.h"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <stb_image_write.h>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

namespace slug::baker
{

bool DecodeImageBytes(
    const uint8_t* _pData,
    size_t         _size,
    Image&         _outImage)
{
    if (_pData == nullptr || _size == 0 || _size > static_cast<size_t>(INT32_MAX))
    {
        return false;
    }
    int w = 0;
    int h = 0;
    int n = 0;
    stbi_uc* pPixels = stbi_load_from_memory(_pData, static_cast<int>(_size), &w, &h, &n, 4);
    if (pPixels == nullptr)
    {
        return false;
    }
    _outImage        = Image::Make(w, h);
    _outImage.bSourceHasAlpha = (n == 2 || n == 4);
    std::memcpy(_outImage.pixels.data(), pPixels, _outImage.pixels.size());
    stbi_image_free(pPixels);
    return true;
}

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
