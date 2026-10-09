#include <gtest/gtest.h>

#include "Tools/Baker/SheetSource.h"

using namespace slug::baker;

namespace
{
    // 배경 마젠타 위에 (x,y,w,h) 불투명 블롭을 올린 합성 시트.
    Image MakeMagentaSheet(const int _w, const int _h, const std::vector<IRect>& _blobs)
    {
        Image img = Image::Make(_w, _h);
        for (size_t i = 0; i < img.pixels.size(); i += 4)
        {
            img.pixels[i]     = 255;
            img.pixels[i + 1] = 0;
            img.pixels[i + 2] = 255;
            img.pixels[i + 3] = 255;
        }
        for (const IRect& b: _blobs)
        {
            for (int y = b.y; y < b.y + b.h; ++y)
            {
                for (int x = b.x; x < b.x + b.w; ++x)
                {
                    uint8_t* p = img.At(x, y);
                    p[0] = 200;
                    p[1] = 100;
                    p[2] = 50;
                    p[3] = 255;
                }
            }
        }
        return img;
    }
}

TEST(BakerSheetKey, MagentaKeyMakesBackgroundTransparentAndCountsIt)
{
    Image img = MakeMagentaSheet(4, 4, { { 1, 1, 2, 2 } });
    const uint64_t bg = ApplySheetKey(img, SheetKey {});
    EXPECT_EQ(bg, 12u);
    EXPECT_EQ(img.At(0, 0)[3], 0);
    EXPECT_EQ(img.At(0, 0)[0], 0);
    EXPECT_EQ(img.At(1, 1)[3], 255);
    EXPECT_EQ(img.At(1, 1)[0], 200);
}

TEST(BakerSheetKey, ColorKeyUsesListedColorsWithTolerance)
{
    Image img = Image::Make(3, 1);
    uint8_t* p = img.pixels.data();
    p[0] = 153; p[1] = 217; p[2] = 234; p[3] = 255;   // 배경 (허용 오차 안)
    p[4] = 152; p[5] = 216; p[6] = 233; p[7] = 255;   // 배경 근처 색
    p[8] = 10;  p[9] = 20;  p[10] = 30; p[11] = 255;  // 전경

    SheetKey key;
    key.mode  = eSheetKeyMode::Color;
    key.colors = { { 153, 217, 234 } };
    key.tol   = 1;
    EXPECT_EQ(ApplySheetKey(img, key), 2u);
    EXPECT_EQ(img.At(0, 0)[3], 0);
    EXPECT_EQ(img.At(1, 0)[3], 0);
    EXPECT_EQ(img.At(2, 0)[3], 255);
}

TEST(BakerSheetKey, AlphaModeKeepsOpaqueColorAndClearsZeroAlpha)
{
    Image img = Image::Make(2, 1);
    uint8_t* p = img.pixels.data();
    p[0] = 255; p[1] = 0; p[2] = 255; p[3] = 255;     // 마젠타지만 불투명 -> 전경
    p[4] = 9;   p[5] = 9; p[6] = 9;   p[7] = 0;       // 알파 0 -> 배경
    SheetKey key;
    key.mode = eSheetKeyMode::Alpha;
    EXPECT_EQ(ApplySheetKey(img, key), 1u);
    EXPECT_EQ(img.At(0, 0)[3], 255);
    EXPECT_EQ(img.At(1, 0)[0], 0);
}

TEST(BakerSheetAuto, ThreeBlobsGiveThreeOrderedRects)
{
    // 두 개는 같은 행(왼->오), 하나는 아래 행
    Image img = MakeMagentaSheet(60, 20, { { 40, 10, 6, 6 }, { 20, 2, 6, 6 }, { 2, 2, 6, 6 } });
    ApplySheetKey(img, SheetKey {});

    const std::vector<IRect> rects = SliceAuto(img, { 0, 0, 60, 20 }, SheetAuto {}, {});
    ASSERT_EQ(rects.size(), 3u);
    EXPECT_EQ(rects[0], (IRect { 2, 2, 6, 6 }));
    EXPECT_EQ(rects[1], (IRect { 20, 2, 6, 6 }));
    EXPECT_EQ(rects[2], (IRect { 40, 10, 6, 6 }));
}

TEST(BakerSheetAuto, MergeGapJoinsNearPiecesOnlyWithinGap)
{
    // 가로 간격 1px -> mergeGap 2 이면 하나, mergeGap 0 이면 둘
    Image img = MakeMagentaSheet(30, 10, { { 2, 2, 5, 5 }, { 8, 2, 5, 5 } });
    ApplySheetKey(img, SheetKey {});

    SheetAuto merged;
    merged.mergeGap  = 2;
    merged.minPixels = 1;
    const std::vector<IRect> one = SliceAuto(img, { 0, 0, 30, 10 }, merged, {});
    ASSERT_EQ(one.size(), 1u);
    EXPECT_EQ(one[0], (IRect { 2, 2, 11, 5 }));

    SheetAuto separate = merged;
    separate.mergeGap  = 0;
    EXPECT_EQ(SliceAuto(img, { 0, 0, 30, 10 }, separate, {}).size(), 2u);
}

TEST(BakerSheetAuto, ExcludeRemovesRegionAndMinPixelsDropsSpecks)
{
    Image img = MakeMagentaSheet(40, 12, { { 2, 2, 6, 6 }, { 16, 2, 6, 6 }, { 30, 2, 1, 1 } });
    ApplySheetKey(img, SheetKey {});

    // 16..21 블롭 제외 -> 2번 블롭만 남고, 1x1 점(1px)은 minPixels 30 으로 버려짐
    const std::vector<IRect> rects = SliceAuto(img, { 0, 0, 40, 12 }, SheetAuto {}, { { 14, 0, 10, 12 } });
    ASSERT_EQ(rects.size(), 1u);
    EXPECT_EQ(rects[0], (IRect { 2, 2, 6, 6 }));
}

TEST(BakerSheetAuto, RegionLimitsSearch)
{
    Image img = MakeMagentaSheet(40, 12, { { 2, 2, 6, 6 }, { 20, 2, 6, 6 } });
    ApplySheetKey(img, SheetKey {});
    const std::vector<IRect> rects = SliceAuto(img, { 10, 0, 30, 12 }, SheetAuto {}, {});
    ASSERT_EQ(rects.size(), 1u);
    EXPECT_EQ(rects[0], (IRect { 20, 2, 6, 6 }));
}

TEST(BakerSheetPivot, FootIsBottomCentreAndCenterIsMiddle)
{
    const Pivot foot = ComputeSheetPivot(7, 9, eSheetPivot::Foot);
    EXPECT_EQ(foot.x, 3);
    EXPECT_EQ(foot.y, 9);
    const Pivot center = ComputeSheetPivot(7, 9, eSheetPivot::Center);
    EXPECT_EQ(center.x, 3);
    EXPECT_EQ(center.y, 4);
}

TEST(BakerSheetMirror, FlipSwapsColumnsAndNameMapsLeftRight)
{
    Image img = Image::Make(3, 1);
    img.At(0, 0)[0] = 1;
    img.At(2, 0)[0] = 9;
    FlipHorizontal(img);
    EXPECT_EQ(img.At(0, 0)[0], 9);
    EXPECT_EQ(img.At(2, 0)[0], 1);

    std::string out;
    EXPECT_TRUE(MirrorClipName("Order_Run_Right", out));
    EXPECT_EQ(out, "Order_Run_Left");
    EXPECT_TRUE(MirrorClipName("Parker_Turn_Left_2", out));
    EXPECT_EQ(out, "Parker_Turn_Right_2");
    EXPECT_FALSE(MirrorClipName("Parker_Run", out));
    EXPECT_FALSE(MirrorClipName("Parkerright", out));
}

TEST(BakerSheetCollision, LegacyNamesAreNeverOverwrittenWithoutReplace)
{
    EXPECT_EQ(ResolveSheetName(false, false), eSheetNameAction::Add);
    EXPECT_EQ(ResolveSheetName(false, true), eSheetNameAction::Add);
    EXPECT_EQ(ResolveSheetName(true, false), eSheetNameAction::Skip);
    EXPECT_EQ(ResolveSheetName(true, true), eSheetNameAction::Replace);
}
