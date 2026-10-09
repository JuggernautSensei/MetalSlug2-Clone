#include <gtest/gtest.h>

#include <set>

#include "Tools/Baker/AnimMath.h"
#include "Tools/Baker/BmpReader.h"
#include "Tools/Baker/Ground.h"
#include "Tools/Baker/Image.h"
#include "Tools/Baker/JsonWriter.h"
#include "Tools/Baker/LegacyParser.h"
#include "Tools/Baker/Packer.h"

using namespace slug::baker;

TEST(BakerTrim, FindsTightBoundsInsideCell)
{
    Image img = Image::Make(20, 10);
    for (int y = 2; y <= 6; ++y)
    {
        for (int x = 13; x <= 15; ++x)
        {
            img.At(x, y)[3] = 255;
        }
    }
    img.At(2, 9)[3] = 255;   // 다른 셀의 픽셀은 무시되어야 함

    const IRect bounds = FindOpaqueBounds(img, { 10, 0, 10, 10 });
    EXPECT_EQ(bounds, (IRect { 13, 2, 3, 5 }));
}

TEST(BakerTrim, EmptyCellGivesEmptyRect)
{
    const Image img = Image::Make(8, 8);
    EXPECT_TRUE(FindOpaqueBounds(img, { 0, 0, 8, 8 }).IsEmpty());
}

TEST(BakerTrim, RegionOutsideImageIsTransparent)
{
    Image img = Image::Make(8, 8);
    img.At(7, 7)[3] = 255;
    EXPECT_TRUE(FindOpaqueBounds(img, { 8, 0, 8, 8 }).IsEmpty());
    EXPECT_EQ(FindOpaqueBounds(img, { 4, 4, 8, 8 }), (IRect { 7, 7, 1, 1 }));
}

TEST(BakerTrim, AlphaThresholdIgnoresFaintPixels)
{
    Image img = Image::Make(4, 4);
    img.At(0, 0)[3] = 10;
    img.At(2, 2)[3] = 200;
    EXPECT_EQ(FindOpaqueBounds(img, { 0, 0, 4, 4 }, 0), (IRect { 0, 0, 3, 3 }));
    EXPECT_EQ(FindOpaqueBounds(img, { 0, 0, 4, 4 }, 50), (IRect { 2, 2, 1, 1 }));
}

TEST(BakerTrim, CropPadsOutOfImageWithTransparent)
{
    Image img = Image::Make(2, 2);
    for (uint8_t& v: img.pixels) { v = 255; }
    const Image crop = CropImage(img, { 1, 1, 3, 3 });
    ASSERT_EQ(crop.width, 3);
    EXPECT_EQ(crop.At(0, 0)[3], 255);
    EXPECT_EQ(crop.At(1, 0)[3], 0);
    EXPECT_EQ(crop.At(0, 2)[3], 0);
}

TEST(BakerAlpha, ColorKeyMakesMagentaTransparentAndRestOpaque)
{
    Image img = Image::Make(2, 1);
    uint8_t* p = img.pixels.data();
    p[0] = 255; p[1] = 0; p[2] = 255; p[3] = 255;   // 마젠타
    p[4] = 10;  p[5] = 20; p[6] = 30; p[7] = 0;     // 알파 0 이던 일반 색
    ApplyColorKey(img, 255, 0, 255);
    EXPECT_EQ(img.At(0, 0)[3], 0);
    EXPECT_EQ(img.At(0, 0)[0], 0);
    EXPECT_EQ(img.At(1, 0)[3], 255);
    EXPECT_EQ(img.At(1, 0)[0], 10);
}

TEST(BakerPivot, LegacyFormulaHandComputed)
{
    const Pivot p = ComputePivot(85, 58, 20, -19, 5, 8);
    EXPECT_EQ(p.x, 17);
    EXPECT_EQ(p.y, 40);
}

TEST(BakerPivot, OddCellWidthUsesIntegerDivision)
{
    const Pivot p = ComputePivot(75, 69, -10, -10, 0, 0);
    EXPECT_EQ(p.x, 47);
    EXPECT_EQ(p.y, 44);
}

TEST(BakerPivot, DrawingBackReproducesLegacyPosition)
{
    const int      posX = 300, posY = 200;
    const uint32_t cellW = 79, cellH = 65;
    const int      offX = -5, offY = -8, trimX = 7, trimY = 3;

    const int legacyLeft = posX - static_cast<int>(cellW / 2) + offX + trimX;
    const int legacyTop  = posY - static_cast<int>(cellH / 2) + offY + trimY;

    const Pivot p = ComputePivot(cellW, cellH, offX, offY, trimX, trimY);
    EXPECT_EQ(posX - p.x, legacyLeft);
    EXPECT_EQ(posY - p.y, legacyTop);
}

TEST(BakerPivot, StripWidthIsDividedByEndPlusOne)
{
    EXPECT_EQ(ResolveCellWidth(450, 450, 5), 75);    // Width == 텍스처 전체 -> / (End + 1)
    EXPECT_EQ(ResolveCellWidth(450, 75, 5), 75);     // 이미 셀 너비
    EXPECT_EQ(ResolveCellWidth(100, 100, 2), 33);    // 정수 나눗셈
}

TEST(BakerDrift, ConstantAnchorHasZeroDrift)
{
    const FrameBox boxes[] = {
        { 40, 50, 20, 50, false },
        { 44, 46, 22, 46, false },
        { 40, 52, 20, 52, false },
    };
    const DriftResult d = ComputeAnchorDrift(boxes, true, 0);
    EXPECT_DOUBLE_EQ(d.max, 0.0);
}

TEST(BakerDrift, DetectsFootMovementAndSkipsEmpty)
{
    const FrameBox boxes[] = {
        { 40, 50, 20, 50, false },
        { 0, 0, 0, 0, true },
        { 40, 50, 20, 47, false },   // 바닥 앵커 Y: 0 -> +3
    };
    const DriftResult d = ComputeAnchorDrift(boxes, false, 0);
    EXPECT_DOUBLE_EQ(d.y, 3.0);
    EXPECT_DOUBLE_EQ(d.x, 0.0);
    EXPECT_DOUBLE_EQ(d.max, 3.0);
}

TEST(BakerDrift, LoopWrapIsIncluded)
{
    const FrameBox boxes[] = {
        { 10, 10, 5, 10, false },
        { 10, 10, 5, 10, false },
        { 10, 10, 9, 10, false },
    };
    EXPECT_DOUBLE_EQ(ComputeAnchorDrift(boxes, false, 0).x, 4.0);
    EXPECT_DOUBLE_EQ(ComputeAnchorDrift(boxes, true, 0).x, 4.0);

    const FrameBox flat[] = { { 10, 10, 5, 10, false }, { 10, 10, 5, 10, false } };
    EXPECT_DOUBLE_EQ(ComputeAnchorDrift(flat, true, 0).max, 0.0);
}

namespace
{
    [[nodiscard]] bool Overlap_(
        const PackPlacement& _pa,
        const PackSize&      _sa,
        const PackPlacement& _pb,
        const PackSize&      _sb,
        const int            _gap)
    {
        if (_pa.page != _pb.page)
        {
            return false;
        }
        return _pa.x < _pb.x + _sb.w + _gap && _pb.x < _pa.x + _sa.w + _gap
            && _pa.y < _pb.y + _sb.h + _gap && _pb.y < _pa.y + _sa.h + _gap;
    }
}

TEST(BakerPacker, NoOverlapKeepsGapAndStaysInPage)
{
    constexpr int kPage = 256;
    constexpr int kPad  = 1;

    std::vector<PackSize> sizes;
    uint32_t              seed = 12345;
    auto                  rnd  = [&seed](const int _lo, const int _hi)
    {
        seed = seed * 1664525u + 1013904223u;
        return _lo + static_cast<int>((seed >> 8) % static_cast<uint32_t>(_hi - _lo + 1));
    };
    for (int i = 0; i < 400; ++i)
    {
        sizes.push_back({ rnd(1, 60), rnd(1, 60) });
    }

    const PackResult result = PackShelf(sizes, kPage, kPad);
    ASSERT_EQ(result.placements.size(), sizes.size());
    EXPECT_GT(result.pageCount, 1);

    for (size_t i = 0; i < sizes.size(); ++i)
    {
        const PackPlacement& p = result.placements[i];
        ASSERT_TRUE(p.bPlaced);
        EXPECT_GE(p.x, kPad);
        EXPECT_GE(p.y, kPad);
        EXPECT_LE(p.x + sizes[i].w + kPad, kPage);
        EXPECT_LE(p.y + sizes[i].h + kPad, kPage);
        EXPECT_LT(p.page, result.pageCount);
    }
    for (size_t i = 0; i < sizes.size(); ++i)
    {
        for (size_t j = i + 1; j < sizes.size(); ++j)
        {
            ASSERT_FALSE(Overlap_(result.placements[i], sizes[i], result.placements[j], sizes[j], kPad)) << i << " vs " << j;
        }
    }
}

TEST(BakerPacker, OversizedRectIsNotPlacedAndZeroSizeIsFree)
{
    const PackSize    sizes[] = { { 300, 10 }, { 0, 0 }, { 10, 10 } };
    const PackResult  result  = PackShelf(sizes, 256, 1);
    EXPECT_FALSE(result.placements[0].bPlaced);
    EXPECT_TRUE(result.placements[1].bPlaced);
    EXPECT_TRUE(result.placements[2].bPlaced);
    EXPECT_EQ(result.pageCount, 1);
}

TEST(BakerGround, ThresholdRules)
{
    const GroundThreshold th;
    EXPECT_TRUE(IsSolidPixel(0, 0, 255, 255, th));
    EXPECT_TRUE(IsSolidPixel(96, 96, 192, 128, th));     // 경계값 포함
    EXPECT_FALSE(IsSolidPixel(97, 0, 255, 255, th));
    EXPECT_FALSE(IsSolidPixel(0, 0, 191, 255, th));
    EXPECT_FALSE(IsSolidPixel(0, 0, 255, 127, th));      // 반투명 미만
    EXPECT_FALSE(IsSolidPixel(255, 255, 255, 255, th));  // 흰색 = 공기
}

TEST(BakerGround, BitPackingIsRowMajorLsbLeft)
{
    Image img = Image::Make(10, 2);
    for (size_t i = 0; i < img.pixels.size(); i += 4) { img.pixels[i + 3] = 0; }
    auto solid = [&img](const int _x, const int _y)
    {
        uint8_t* p = img.At(_x, _y);
        p[0] = 0; p[1] = 0; p[2] = 255; p[3] = 255;
    };
    solid(0, 0);
    solid(9, 0);
    solid(3, 1);

    GroundStats stats;
    const auto  bits = BuildGroundBits(img, GroundThreshold {}, &stats);
    ASSERT_EQ(bits.size(), 4u);   // ceil(10/8) = 2 바이트 * 2 행
    EXPECT_EQ(bits[0], 0b00000001);
    EXPECT_EQ(bits[1], 0b00000010);   // x=9 -> 두 번째 바이트의 bit 1
    EXPECT_EQ(bits[2], 0b00001000);
    EXPECT_EQ(bits[3], 0);
    EXPECT_EQ(stats.solidCount, 3u);
    EXPECT_EQ(stats.solidBounds, (IRect { 0, 0, 10, 2 }));
}

TEST(BakerGround, FileHeaderLayout)
{
    const std::vector<uint8_t> bits = { 0xAB, 0xCD };
    const auto                 file = BuildGroundFile(0x0102, 0x0304, bits);
    ASSERT_EQ(file.size(), 14u);
    EXPECT_EQ(std::string(file.begin(), file.begin() + 4), "GRND");
    EXPECT_EQ(file[4], 0x02);
    EXPECT_EQ(file[5], 0x01);
    EXPECT_EQ(file[8], 0x04);
    EXPECT_EQ(file[9], 0x03);
    EXPECT_EQ(file[12], 0xAB);
    EXPECT_EQ(file[13], 0xCD);
}

TEST(BakerParser, ParsesAnimeBlocksWithCrlf)
{
    const std::string text =
        "File Name:Player_Crouch_Idle_DG_Left\r\nTexture Name:Player_Crouch_Idle_DG_Left\r\nStart:0\r\nEnd:5\r\nWidth:75\r\nHeight:69\r\n"
        "FrameDuration:0.1\r\nLoop:1\r\nOptionIndex:0\r\nOffsetX:-10\r\nOffsetY:-10\r\n\r\n"
        "File Name:B\r\nTexture Name:T\r\nStart:2\r\nEnd:4\r\nWidth:10\r\nHeight:20\r\nFrameDuration:0.06\r\nLoop:0\r\nOptionIndex:3\r\nOffsetX:4\r\nOffsetY:5\r\n";

    std::vector<std::string> warnings;
    const auto               anims = ParseAnimeData(text, "x.anime", warnings);
    EXPECT_TRUE(warnings.empty());
    ASSERT_EQ(anims.size(), 2u);
    EXPECT_EQ(anims[0].name, "Player_Crouch_Idle_DG_Left");
    EXPECT_EQ(anims[0].end, 5);
    EXPECT_EQ(anims[0].width, 75);
    EXPECT_EQ(anims[0].offsetX, -10);
    EXPECT_DOUBLE_EQ(anims[0].frameDuration, 0.1);
    EXPECT_TRUE(anims[0].bLoop);
    EXPECT_FALSE(anims[1].bLoop);
    EXPECT_EQ(anims[1].start, 2);
    EXPECT_EQ(anims[1].optionIndex, 3);
}

TEST(BakerParser, SpriteDefaultSizeAndTextureData)
{
    std::vector<std::string> warnings;
    const auto sprites = ParseSpriteData("File Name:S\nTexture Name:T\nWidth:default\nHeight:14\nColumn:1\nLine:2\n", "s.sprite", warnings);
    ASSERT_EQ(sprites.size(), 1u);
    EXPECT_EQ(sprites[0].width, -1);
    EXPECT_EQ(sprites[0].height, 14);
    EXPECT_EQ(sprites[0].column, 1);
    EXPECT_EQ(sprites[0].line, 2);

    const auto textures = ParseTextureData("File Name:A\nFile Path:Effect\\A.bmp\nUseTransparent:2\nTransparent:default\n", warnings);
    ASSERT_EQ(textures.size(), 1u);
    EXPECT_EQ(textures[0].path, "Effect/A.bmp");
    EXPECT_EQ(textures[0].mode, eTransparentMode::Alpha);
}

TEST(BakerJson, EscapesAndNests)
{
    JsonWriter w;
    w.BeginObject();
    w.Field("a", 1);
    w.Key("o");
    w.BeginObject(true);
    w.Field("s", "q\"\\\n");
    w.Field("f", 0.06);
    w.Field("b", true);
    w.EndObject();
    w.Key("arr");
    w.BeginArray(true);
    w.Value(1);
    w.Value(2);
    w.EndArray();
    w.EndObject();

    const std::string expected =
        "{\n"
        "  \"a\": 1,\n"
        "  \"o\": { \"s\": \"q\\\"\\\\\\n\", \"f\": 0.06, \"b\": true },\n"
        "  \"arr\": [ 1, 2 ]\n"
        "}\n";
    EXPECT_EQ(w.Str(), expected);
}

TEST(BakerBmp, Decodes32BitWithAlphaBottomUp)
{
    std::vector<uint8_t> f(14 + 40 + 16, 0);
    f[0] = 'B'; f[1] = 'M';
    f[10] = 54;
    f[14] = 40;
    f[18] = 2; f[22] = 2; f[26] = 1; f[28] = 32;
    const uint8_t px[16] = { 0, 0, 255, 255,   0, 255, 0, 0,   255, 0, 0, 128,   255, 0, 255, 255 };
    for (int i = 0; i < 16; ++i) { f[54 + static_cast<size_t>(i)] = px[i]; }

    auto result = DecodeBmp(f);
    ASSERT_TRUE(result.HasValue());
    const Image& img = *result;
    EXPECT_TRUE(img.bSourceHasAlpha);
    EXPECT_EQ(img.At(0, 0)[2], 255);   // 파랑
    EXPECT_EQ(img.At(0, 0)[3], 128);
    EXPECT_EQ(img.At(1, 0)[0], 255);   // 마젠타 R
    EXPECT_EQ(img.At(1, 0)[2], 255);   // 마젠타 B
    EXPECT_EQ(img.At(0, 1)[0], 255);   // 빨강
    EXPECT_EQ(img.At(0, 1)[3], 255);
    EXPECT_EQ(img.At(1, 1)[1], 255);   // 초록
    EXPECT_EQ(img.At(1, 1)[3], 0);
}
