// 시트 베이커의 키 처리, 블롭 자동 분할, 피벗/스케일/미러, 프레임 정렬(등록)을 합성 이미지로 검증한다.

#include <gtest/gtest.h>

#include "Tools/Baker/SheetSource.h"

using namespace slug::baker;

namespace
{
// 배경 마젠타 위에 (x,y,w,h) 불투명 블롭을 올린 합성 시트.
[[nodiscard]] Image MakeMagentaSheet_(
    const int                 _w,
    const int                 _h,
    const std::vector<IRect>& _blobs)
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
                p[0]       = 200;
                p[1]       = 100;
                p[2]       = 50;
                p[3]       = 255;
            }
        }
    }
    return img;
}

// 단색(_r,_g,_b) 블롭 하나를 같은 위치에 그린다.
void PaintBlob_(
    Image&        _img,
    const IRect&  _b,
    const uint8_t _r,
    const uint8_t _g,
    const uint8_t _bl)
{
    for (int y = _b.y; y < _b.y + _b.h; ++y)
    {
        for (int x = _b.x; x < _b.x + _b.w; ++x)
        {
            uint8_t* p = _img.At(x, y);
            p[0]       = _r;
            p[1]       = _g;
            p[2]       = _bl;
            p[3]       = 255;
        }
    }
}
}   // namespace

TEST(BakerSheetKey, MagentaKeyMakesBackgroundTransparentAndCountsIt)
{
    const std::vector<IRect> blobs = {
        { 1, 1, 2, 2 }
    };
    Image          img = MakeMagentaSheet_(4, 4, blobs);
    const uint64_t bg  = ApplySheetKey(img, SheetKey {});
    EXPECT_EQ(bg, 12u);
    EXPECT_EQ(img.At(0, 0)[3], 0);
    EXPECT_EQ(img.At(0, 0)[0], 0);
    EXPECT_EQ(img.At(1, 1)[3], 255);
    EXPECT_EQ(img.At(1, 1)[0], 200);
}

TEST(BakerSheetKey, ColorKeyUsesListedColorsWithTolerance)
{
    Image    img = Image::Make(3, 1);
    uint8_t* p   = img.pixels.data();
    p[0]         = 153;
    p[1]         = 217;
    p[2]         = 234;
    p[3]         = 255;   // 배경 (허용 오차 안)
    p[4]         = 152;
    p[5]         = 216;
    p[6]         = 233;
    p[7]         = 255;   // 배경 근처 색
    p[8]         = 10;
    p[9]         = 20;
    p[10]        = 30;
    p[11]        = 255;   // 전경

    SheetKey key;
    key.mode   = eSheetKeyMode::Color;
    key.colors = {
        { 153, 217, 234 }
    };
    key.tol = 1;
    EXPECT_EQ(ApplySheetKey(img, key), 2u);
    EXPECT_EQ(img.At(0, 0)[3], 0);
    EXPECT_EQ(img.At(1, 0)[3], 0);
    EXPECT_EQ(img.At(2, 0)[3], 255);
}

TEST(BakerSheetKey, AlphaModeKeepsOpaqueColorAndClearsZeroAlpha)
{
    Image    img = Image::Make(2, 1);
    uint8_t* p   = img.pixels.data();
    p[0]         = 255;
    p[1]         = 0;
    p[2]         = 255;
    p[3]         = 255;   // 마젠타지만 불투명 -> 전경
    p[4]         = 9;
    p[5]         = 9;
    p[6]         = 9;
    p[7]         = 0;   // 알파 0 -> 배경
    SheetKey key;
    key.mode = eSheetKeyMode::Alpha;
    EXPECT_EQ(ApplySheetKey(img, key), 1u);
    EXPECT_EQ(img.At(0, 0)[3], 255);
    EXPECT_EQ(img.At(1, 0)[0], 0);
}

TEST(BakerSheetAuto, ThreeBlobsGiveThreeOrderedRects)
{
    // 두 개는 같은 행(왼->오), 하나는 아래 행
    const std::vector<IRect> blobs = {
        { 40, 10, 6, 6 },
        { 20,  2, 6, 6 },
        {  2,  2, 6, 6 }
    };
    Image img = MakeMagentaSheet_(60, 20, blobs);
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
    const std::vector<IRect> blobs = {
        { 2, 2, 5, 5 },
        { 8, 2, 5, 5 }
    };
    Image img = MakeMagentaSheet_(30, 10, blobs);
    ApplySheetKey(img, SheetKey {});

    SheetAuto merged;
    merged.mergeGap              = 2;
    merged.minPixels             = 1;
    const std::vector<IRect> one = SliceAuto(img, { 0, 0, 30, 10 }, merged, {});
    ASSERT_EQ(one.size(), 1u);
    EXPECT_EQ(one[0], (IRect { 2, 2, 11, 5 }));

    SheetAuto separate = merged;
    separate.mergeGap  = 0;
    EXPECT_EQ(SliceAuto(img, { 0, 0, 30, 10 }, separate, {}).size(), 2u);
}

TEST(BakerSheetAuto, ExcludeRemovesRegionAndMinPixelsDropsSpecks)
{
    const std::vector<IRect> blobs = {
        {  2, 2, 6, 6 },
        { 16, 2, 6, 6 },
        { 30, 2, 1, 1 }
    };
    Image img = MakeMagentaSheet_(40, 12, blobs);
    ApplySheetKey(img, SheetKey {});

    // 16..21 블롭 제외 -> 2번 블롭만 남고, 1x1 점(1px)은 minPixels 30 으로 버려짐
    const std::vector<IRect> excluded = {
        { 14, 0, 10, 12 }
    };
    const std::vector<IRect> rects = SliceAuto(img, { 0, 0, 40, 12 }, SheetAuto {}, excluded);
    ASSERT_EQ(rects.size(), 1u);
    EXPECT_EQ(rects[0], (IRect { 2, 2, 6, 6 }));
}

TEST(BakerSheetAuto, RegionLimitsSearch)
{
    const std::vector<IRect> blobs = {
        {  2, 2, 6, 6 },
        { 20, 2, 6, 6 }
    };
    Image img = MakeMagentaSheet_(40, 12, blobs);
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
    Image img       = Image::Make(3, 1);
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

TEST(BakerSheetScale, ClipValueOverridesSheetAndZeroInherits)
{
    EXPECT_EQ(ResolveSheetScale(2, 0), 2);
    EXPECT_EQ(ResolveSheetScale(2, 1), 1);
    EXPECT_EQ(ResolveSheetScale(1, 3), 3);
    EXPECT_EQ(ResolveSheetScale(1, 0), 1);
}

TEST(BakerSheetScale, NearestUpscaleRepeatsEachPixelBlock)
{
    Image src       = Image::Make(2, 1);
    src.At(0, 0)[0] = 10;
    src.At(0, 0)[3] = 255;
    src.At(1, 0)[0] = 20;
    src.At(1, 0)[3] = 0;
    const Image up  = ScaleNearest(src, 2);
    EXPECT_EQ(up.width, 4);
    EXPECT_EQ(up.height, 2);
    for (int y = 0; y < 2; ++y)
    {
        EXPECT_EQ(up.At(0, y)[0], 10);
        EXPECT_EQ(up.At(1, y)[0], 10);
        EXPECT_EQ(up.At(2, y)[0], 20);
        EXPECT_EQ(up.At(3, y)[3], 0);
    }
}

TEST(BakerSheetScale, ScaledFrameDoublesSizeAndFootPivot)
{
    // 6x5 시트, 3x2 블롭 (1,1). 피벗은 트림 전 사각형(6x5) 기준: 발 = (w/2 - trimX, h - trimY).
    const std::vector<IRect> blobs = {
        { 1, 1, 3, 2 }
    };
    Image sheet = MakeMagentaSheet_(6, 5, blobs);
    ApplySheetKey(sheet, SheetKey {});
    const std::vector<IRect> rects = {
        { 0, 0, 6, 5 }
    };

    const BuiltFrames one = BuildSheetFrames(sheet, rects, eSheetPivot::Foot, false, 1);
    EXPECT_EQ(one.frames[0].w, 3);
    EXPECT_EQ(one.frames[0].h, 2);
    EXPECT_EQ(one.frames[0].pivotX, 2);   // 3 - 1
    EXPECT_EQ(one.frames[0].pivotY, 4);   // 5 - 1

    // 2x: 블롭이 (2,2) 에 6x4 로 커지고, 사각형은 12x10. 발 피벗 = (6-2, 10-2) = (4,8). 셀 크기도 2배.
    const BuiltFrames two = BuildSheetFrames(sheet, rects, eSheetPivot::Foot, false, 2);
    EXPECT_EQ(two.frames[0].trimX, 2);
    EXPECT_EQ(two.frames[0].trimY, 2);
    EXPECT_EQ(two.frames[0].w, 6);
    EXPECT_EQ(two.frames[0].h, 4);
    EXPECT_EQ(two.frames[0].pivotX, 4);
    EXPECT_EQ(two.frames[0].pivotY, 8);
    EXPECT_EQ(two.cellW, 12);
    EXPECT_EQ(two.cellH, 10);
    EXPECT_EQ(two.trimmed[0].width, 6);
    EXPECT_EQ(two.trimmed[0].height, 4);

    // 중심: (6-2, 5-2) = (4,3)
    const BuiltFrames center = BuildSheetFrames(sheet, rects, eSheetPivot::Center, false, 2);
    EXPECT_EQ(center.frames[0].pivotX, 4);
    EXPECT_EQ(center.frames[0].pivotY, 3);
}

TEST(BakerSheetMirror, MirrorOfScaledFrameFlipsPixelsAndPivotAroundScaledWidth)
{
    // 블롭 좌상단 픽셀(시트 (1,1))에 마커 7. 3배 확대 시 트림 프레임 w=9, 피벗 x = 18/2 - 3 = 6 (foot), 뒤집으면 9 - 6 = 3.
    const std::vector<IRect> blobs = {
        { 1, 1, 3, 2 }
    };
    Image sheet = MakeMagentaSheet_(6, 5, blobs);
    ApplySheetKey(sheet, SheetKey {});
    sheet.At(1, 1)[0]              = 7;
    const std::vector<IRect> rects = {
        { 0, 0, 6, 5 }
    };

    const BuiltFrames plain  = BuildSheetFrames(sheet, rects, eSheetPivot::Foot, false, 3);
    const BuiltFrames mirror = BuildSheetFrames(sheet, rects, eSheetPivot::Foot, true, 3);
    EXPECT_EQ(plain.frames[0].w, 9);
    EXPECT_EQ(plain.frames[0].pivotX, 6);
    EXPECT_EQ(plain.trimmed[0].At(0, 0)[0], 7);
    EXPECT_EQ(plain.trimmed[0].At(8, 0)[0], 200);
    EXPECT_EQ(mirror.frames[0].pivotX, 3);
    EXPECT_EQ(mirror.frames[0].pivotY, plain.frames[0].pivotY);
    EXPECT_EQ(mirror.trimmed[0].At(8, 0)[0], 7);
    EXPECT_EQ(mirror.trimmed[0].At(0, 0)[0], 200);
}

TEST(BakerSheetRegister, SearchFindsTheBestShiftAndPreferBaseKeepsZeroOnTies)
{
    // 같은 30x30 사각형 두 장. 기준 위치를 (12,-7) 로 주면 겹침 최대는 (0,0) 이다.
    Image a = Image::Make(40, 40);
    Image b = Image::Make(40, 40);
    PaintBlob_(a, { 0, 0, 30, 30 }, 90, 180, 40);
    PaintBlob_(b, { 0, 0, 30, 30 }, 90, 180, 40);

    const AlignResult far = BestAlign(a, b, 12, -7, 40, false);
    EXPECT_EQ(far.dx, 0);
    EXPECT_EQ(far.dy, 0);
    EXPECT_GT(far.score, 0.99);

    // 기준 위치가 이미 최고점이면 그대로 둔다.
    const AlignResult zero = BestAlign(a, b, 0, 0, 40, true);
    EXPECT_EQ(zero.dx, 0);
    EXPECT_EQ(zero.dy, 0);
}

TEST(BakerSheetRegister, SameSquareAtDifferentOffsetsAndSizesCoincidesAfterRegistration)
{
    // 세 셀(크기가 다름)에 같은 6x6 사각형이 다른 위치에 있고, 셀마다 꼬리 1px 이 달라 트림 bbox 가 다르다.
    Image sheet = MakeMagentaSheet_(100, 40, {});
    PaintBlob_(sheet, { 3, 2, 6, 6 }, 200, 100, 50);
    PaintBlob_(sheet, { 37, 11, 6, 6 }, 200, 100, 50);
    PaintBlob_(sheet, { 43, 16, 2, 1 }, 0, 255, 0);   // 꼬리: 트림 bbox 를 키운다 (폭 6 -> 8, 중심 피벗 w/2 가 3 -> 4 로 달라진다)
    PaintBlob_(sheet, { 80, 5, 6, 6 }, 200, 100, 50);
    PaintBlob_(sheet, { 86, 5, 2, 1 }, 0, 255, 0);
    ApplySheetKey(sheet, SheetKey {});
    const std::vector<IRect> rects = {
        {  0, 0, 20, 20 },
        { 30, 0, 16, 20 },
        { 60, 0, 30, 24 }
    };

    SheetRegisterOpts reg;
    reg.bEnabled            = true;
    const BuiltFrames built = BuildSheetFrames(sheet, rects, eSheetPivot::Foot, false, 1, reg);
    ASSERT_EQ(built.frames.size(), 3u);
    EXPECT_TRUE(built.reg.bRegistered);
    EXPECT_EQ(built.reg.alignedFrames, 2);
    EXPECT_TRUE(built.reg.fallbackFrames.empty());

    // 그리기 좌상단 = origin - pivot. 같은 사각형이 같은 화면 위치에 와야 한다.
    for (size_t i = 1; i < built.frames.size(); ++i)
    {
        EXPECT_EQ(-built.frames[i].pivotX, -built.frames[0].pivotX) << "frame " << i;
        EXPECT_EQ(-built.frames[i].pivotY, -built.frames[0].pivotY) << "frame " << i;
    }
    // 트림 전 기준 피벗(정렬 전)에서는 꼬리 때문에 어긋났다는 것을 측정이 보고해야 한다.
    EXPECT_GT(built.reg.shiftBefore, 0.0);
    EXPECT_LE(built.reg.shiftAfter, 1.0);
}

TEST(BakerSheetRegister, IdenticalSizeCellsStayOnRectPivotWithOrWithoutRegistration)
{
    // 10x10 셀 세 개, 같은 위치의 4x4 블롭.
    // 정렬 끔: 사각형 발 피벗 = (5 - 2, 10 - 3) = (3, 7).
    // 정렬 켬: 공통 원점 = 합집합(= 블롭) 의 발 중앙 -> (2, 4). 프레임끼리는 모두 같은 값이어야 한다.
    const std::vector<IRect> blobs = {
        {  2, 3, 4, 4 },
        { 12, 3, 4, 4 },
        { 22, 3, 4, 4 }
    };
    Image sheet = MakeMagentaSheet_(40, 10, blobs);
    ApplySheetKey(sheet, SheetKey {});
    const std::vector<IRect> rects = {
        {  0, 0, 10, 10 },
        { 10, 0, 10, 10 },
        { 20, 0, 10, 10 }
    };

    SheetRegisterOpts reg;
    reg.bEnabled          = true;
    const BuiltFrames on  = BuildSheetFrames(sheet, rects, eSheetPivot::Foot, false, 1, reg);
    const BuiltFrames off = BuildSheetFrames(sheet, rects, eSheetPivot::Foot, false, 1, SheetRegisterOpts {});
    for (size_t i = 0; i < rects.size(); ++i)
    {
        EXPECT_EQ(on.frames[i].pivotX, 2) << "frame " << i;
        EXPECT_EQ(on.frames[i].pivotY, 4) << "frame " << i;
        EXPECT_EQ(off.frames[i].pivotX, 3) << "frame " << i;
        EXPECT_EQ(off.frames[i].pivotY, 7) << "frame " << i;
    }
    EXPECT_EQ(on.reg.alignedFrames, 2);
    EXPECT_DOUBLE_EQ(on.reg.shiftBefore, 0.0);
    EXPECT_FALSE(off.reg.bRegistered);
}

TEST(BakerSheetRegister, LowOverlapFrameFallsBackToRectPivot)
{
    // 같은 위치가 아니고 색이 달라 점수가 0.5 미만: 두 번째 프레임은 정렬하지 않고 사각형 피벗을 쓴다.
    Image sheet = MakeMagentaSheet_(20, 10, {});
    PaintBlob_(sheet, { 1, 1, 3, 3 }, 200, 100, 50);
    PaintBlob_(sheet, { 11, 6, 3, 3 }, 0, 0, 255);
    ApplySheetKey(sheet, SheetKey {});
    const std::vector<IRect> rects = {
        {  0, 0, 10, 10 },
        { 10, 0, 10, 10 }
    };

    SheetRegisterOpts reg;
    reg.bEnabled            = true;
    const BuiltFrames built = BuildSheetFrames(sheet, rects, eSheetPivot::Foot, false, 1, reg);
    EXPECT_EQ(built.reg.alignedFrames, 0);
    ASSERT_EQ(built.reg.fallbackFrames.size(), 1u);
    EXPECT_EQ(built.reg.fallbackFrames[0], 1u);
    // 정렬하지 않은 프레임은 사각형 기준 위치(T) 를 그대로 쓴다. 공통 원점은 공유하므로 두 프레임의 피벗 차이 = 사각형 기준점 차이.
    // 프레임 0: 트림 (1,1), 프레임 1: 트림 (1,6), 기준점 중심 (5,5) -> 차이 = (0, -5).
    EXPECT_EQ(built.frames[1].pivotX - built.frames[0].pivotX, 0);
    EXPECT_EQ(built.frames[1].pivotY - built.frames[0].pivotY, -5);
}

TEST(BakerSheetRegister, AnchorOverrideSetsTheOriginInFirstFrameTrimCoords)
{
    const std::vector<IRect> blobs = {
        {  3, 2, 6, 6 },
        { 23, 2, 6, 6 }
    };
    Image sheet = MakeMagentaSheet_(40, 20, blobs);
    ApplySheetKey(sheet, SheetKey {});
    const std::vector<IRect> rects = {
        {  0, 0, 20, 20 },
        { 20, 0, 20, 20 }
    };

    SheetRegisterOpts reg;
    reg.bEnabled            = true;
    reg.bHasAnchor          = true;
    reg.anchorX             = 1;
    reg.anchorY             = 1;
    const BuiltFrames built = BuildSheetFrames(sheet, rects, eSheetPivot::Foot, false, 1, reg);
    EXPECT_EQ(built.frames[0].pivotX, 1);
    EXPECT_EQ(built.frames[0].pivotY, 1);
    EXPECT_EQ(built.frames[1].pivotX, 1);
    EXPECT_EQ(built.frames[1].pivotY, 1);
}

TEST(BakerSheetRegister, DefaultRulesDecideWhetherAClipRegisters)
{
    SheetSpec sheet;
    sheet.noRegisterPrefixes = { "Fx_" };

    SheetClipSpec run;
    run.name = "Parker_Run";
    EXPECT_TRUE(ResolveSheetRegister(sheet, run, 3));
    EXPECT_FALSE(ResolveSheetRegister(sheet, run, 1));   // 한 프레임

    SheetClipSpec fx = run;
    fx.name          = "Fx_Burst";
    EXPECT_FALSE(ResolveSheetRegister(sheet, fx, 5));

    SheetClipSpec effect = run;
    effect.bEffect       = true;
    EXPECT_FALSE(ResolveSheetRegister(sheet, effect, 5));

    SheetClipSpec sprite = run;
    sprite.bAsSprite     = true;
    EXPECT_FALSE(ResolveSheetRegister(sheet, sprite, 1));

    SheetClipSpec forced = fx;
    forced.bRegisterSet  = true;
    forced.bRegister     = true;
    EXPECT_TRUE(ResolveSheetRegister(sheet, forced, 5));
}
