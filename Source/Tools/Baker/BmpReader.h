#pragma once
#include <cstdint>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <vector>

#include "Base/Result.h"

#include "Base/Error.h"
#include "Tools/Baker/Image.h"

namespace slug::baker
{

// ===========================================
//  BMP 디코더 (1/4/8 팔레트, 24, 32 비트 + BITFIELDS)
//    stb_image 는 알파가 전부 0 인 32비트 BMP 를 불투명으로 바꾸는 휴리스틱이 있어 직접 디코딩한다.
// ===========================================

namespace bmp_detail
{
    [[nodiscard]] inline uint32_t ReadU32(
        const uint8_t* _p)
    {
        return static_cast<uint32_t>(_p[0]) | (static_cast<uint32_t>(_p[1]) << 8) | (static_cast<uint32_t>(_p[2]) << 16) | (static_cast<uint32_t>(_p[3]) << 24);
    }

    [[nodiscard]] inline uint16_t ReadU16(
        const uint8_t* _p)
    {
        return static_cast<uint16_t>(_p[0] | (_p[1] << 8));
    }

    struct Mask
    {
        uint32_t mask  = 0;
        int      shift = 0;
        int      bits  = 0;

        void Setup(const uint32_t _mask)
        {
            mask  = _mask;
            shift = 0;
            bits  = 0;
            if (_mask == 0)
            {
                return;
            }
            uint32_t m = _mask;
            while ((m & 1u) == 0)
            {
                m >>= 1;
                ++shift;
            }
            while ((m & 1u) != 0)
            {
                m >>= 1;
                ++bits;
            }
        }

        [[nodiscard]] uint8_t Extract(const uint32_t _pixel) const
        {
            if (bits == 0)
            {
                return 0;
            }
            const uint32_t v   = (_pixel & mask) >> shift;
            const uint32_t max = (bits >= 32) ? 0xFFFFFFFFu : ((1u << bits) - 1u);
            return static_cast<uint8_t>((static_cast<uint64_t>(v) * 255u + max / 2u) / max);
        }
    };
}   // namespace bmp_detail

[[nodiscard]] inline slug::Result<Image> DecodeBmp(
    const std::vector<uint8_t>& _file)
{
    using namespace bmp_detail;

    if (_file.size() < 26 || _file[0] != 'B' || _file[1] != 'M')
    {
        return slug::eError::Parse;
    }

    const uint32_t dataOffset = ReadU32(&_file[10]);
    const uint32_t hdrSize    = ReadU32(&_file[14]);
    if (hdrSize < 40 || _file.size() < 14u + hdrSize)
    {
        return slug::eError::Parse;   // BITMAPCOREHEADER 등은 지원하지 않음
    }

    const int      width    = static_cast<int>(ReadU32(&_file[18]));
    const int      rawH     = static_cast<int>(ReadU32(&_file[22]));
    const uint16_t bpp      = ReadU16(&_file[28]);
    const uint32_t compress = ReadU32(&_file[30]);
    uint32_t       colorsUsed = ReadU32(&_file[46]);
    const bool     bTopDown = rawH < 0;
    const int      height   = bTopDown ? -rawH : rawH;

    if (width <= 0 || height <= 0 || width > 65536 || height > 65536)
    {
        return slug::eError::InvalidData;
    }
    if (compress != 0 && compress != 3)
    {
        return slug::eError::InvalidData;   // RLE 미지원
    }

    // 채널 마스크
    Mask mr, mg, mb, ma;
    if (bpp == 32 || bpp == 16)
    {
        if (compress == 3)
        {
            const size_t maskPos = 14u + 40u;
            if (_file.size() < maskPos + 12u)
            {
                return slug::eError::Parse;
            }
            mr.Setup(ReadU32(&_file[maskPos]));
            mg.Setup(ReadU32(&_file[maskPos + 4]));
            mb.Setup(ReadU32(&_file[maskPos + 8]));
            if (hdrSize >= 56 && _file.size() >= maskPos + 16u)
            {
                ma.Setup(ReadU32(&_file[maskPos + 12]));
            }
        }
        else if (bpp == 32)
        {
            mr.Setup(0x00FF0000u);
            mg.Setup(0x0000FF00u);
            mb.Setup(0x000000FFu);
            ma.Setup(0xFF000000u);
        }
        else
        {
            mr.Setup(0x7C00u);
            mg.Setup(0x03E0u);
            mb.Setup(0x001Fu);
        }
    }

    // 팔레트
    std::vector<uint32_t> palette;
    if (bpp <= 8)
    {
        if (colorsUsed == 0)
        {
            colorsUsed = 1u << bpp;
        }
        const size_t palPos = 14u + hdrSize;
        if (_file.size() < palPos + static_cast<size_t>(colorsUsed) * 4u)
        {
            return slug::eError::Parse;
        }
        palette.resize(colorsUsed);
        for (uint32_t i = 0; i < colorsUsed; ++i)
        {
            palette[i] = ReadU32(&_file[palPos + static_cast<size_t>(i) * 4u]);
        }
    }
    else if (bpp != 16 && bpp != 24 && bpp != 32)
    {
        return slug::eError::InvalidData;
    }

    const size_t rowBytes = ((static_cast<size_t>(width) * bpp + 31u) / 32u) * 4u;
    if (static_cast<size_t>(dataOffset) + rowBytes * static_cast<size_t>(height) > _file.size())
    {
        return slug::eError::Parse;
    }

    Image img = Image::Make(width, height);
    img.bSourceHasAlpha = (bpp == 32 && ma.bits > 0);

    for (int y = 0; y < height; ++y)
    {
        const int      dstY = bTopDown ? y : (height - 1 - y);
        const uint8_t* pSrc = &_file[dataOffset + rowBytes * static_cast<size_t>(y)];
        uint8_t*       pDst = img.At(0, dstY);

        for (int x = 0; x < width; ++x, pDst += 4)
        {
            switch (bpp)
            {
                case 32:
                {
                    const uint32_t px = ReadU32(pSrc + static_cast<size_t>(x) * 4u);
                    pDst[0]           = mr.Extract(px);
                    pDst[1]           = mg.Extract(px);
                    pDst[2]           = mb.Extract(px);
                    pDst[3]           = (ma.bits > 0) ? ma.Extract(px) : 255;
                    break;
                }
                case 24:
                {
                    const uint8_t* p = pSrc + static_cast<size_t>(x) * 3u;
                    pDst[0]          = p[2];
                    pDst[1]          = p[1];
                    pDst[2]          = p[0];
                    pDst[3]          = 255;
                    break;
                }
                case 16:
                {
                    const uint32_t px = ReadU16(pSrc + static_cast<size_t>(x) * 2u);
                    pDst[0]           = mr.Extract(px);
                    pDst[1]           = mg.Extract(px);
                    pDst[2]           = mb.Extract(px);
                    pDst[3]           = 255;
                    break;
                }
                default:
                {
                    const int      perByte = 8 / bpp;
                    const uint8_t  byte    = pSrc[x / perByte];
                    const int      slot    = x % perByte;
                    const int      shift   = 8 - bpp * (slot + 1);
                    const uint32_t index   = (byte >> shift) & ((1u << bpp) - 1u);
                    const uint32_t color   = (index < palette.size()) ? palette[index] : 0u;
                    pDst[0]                = static_cast<uint8_t>((color >> 16) & 0xFFu);
                    pDst[1]                = static_cast<uint8_t>((color >> 8) & 0xFFu);
                    pDst[2]                = static_cast<uint8_t>(color & 0xFFu);
                    pDst[3]                = 255;
                    break;
                }
            }
        }
    }
    return img;
}

[[nodiscard]] inline slug::Result<Image> LoadBmpFile(
    const std::filesystem::path& _path)
{
    std::ifstream stream(_path, std::ios::binary | std::ios::ate);
    if (!stream)
    {
        return slug::eError::FileOpen;
    }

    const std::streamsize size = stream.tellg();
    if (size <= 0)
    {
        return slug::eError::FileRead;
    }

    std::vector<uint8_t> file(static_cast<size_t>(size));
    stream.seekg(0);
    if (!stream.read(reinterpret_cast<char*>(file.data()), size))
    {
        return slug::eError::FileRead;
    }
    return DecodeBmp(file);
}

}   // namespace slug::baker
