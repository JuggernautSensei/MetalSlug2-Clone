// 스프라이트 배치 테스트: 정렬 순서, 목적지 사각형, 기본 옵션, SDL 렌더링 검증
#include <filesystem>
#include <format>
#include <SDL3/SDL.h>
#include <gtest/gtest.h>

#include "Base/Math.h"
#include "Base/Types.h"
#include "Core/AssetPath.h"
#include "Render/SpriteBatch.h"

namespace
{
[[nodiscard]] AnimFrame MakeFrame_(
    const int _w,
    const int _h,
    const int _pivotX,
    const int _pivotY)
{
    AnimFrame frame;
    frame.w      = _w;
    frame.h      = _h;
    frame.pivotX = _pivotX;
    frame.pivotY = _pivotY;
    return frame;
}

// 정렬 시험용 명령 하나를 큐에 넣는다. seq 는 제출 순서, frame.x 는 식별용 태그
void AddCommand_(
    Vector<SpriteCommand>& _commands,
    const int              _depth,
    const int              _key,
    const int              _tag)
{
    SpriteCommand command;
    command.depth   = _depth;
    command.sortKey = _key;
    command.seq     = static_cast<uint32_t>(_commands.size());
    command.frame.x = _tag;
    _commands.push_back(command);
}
}   // namespace

TEST(SpriteDest, PivotIsSubtractedFromSnappedOrigin)
{
    const AnimFrame frame = MakeFrame_(40, 52, 21, 50);
    const IntRect   rect  = ComputeDestRect(frame, { 100.f, 200.f }, { 0.f, 0.f }, false);
    EXPECT_EQ(rect, (IntRect { 79, 150, 40, 52 }));
}

TEST(SpriteDest, OriginIsFlooredSoSubpixelMotionNeverChangesSize)
{
    const AnimFrame frame = MakeFrame_(10, 10, 5, 10);
    const IntRect   a     = ComputeDestRect(frame, { 100.f, 50.f }, { 0.f, 0.f }, false);
    const IntRect   b     = ComputeDestRect(frame, { 100.99f, 50.99f }, { 0.f, 0.f }, false);
    const IntRect   c     = ComputeDestRect(frame, { 101.f, 51.f }, { 0.f, 0.f }, false);
    EXPECT_EQ(a, b);
    EXPECT_EQ(c.x, a.x + 1);
    EXPECT_EQ(c.y, a.y + 1);
    EXPECT_EQ(a.w, b.w);
    EXPECT_EQ(a.h, c.h);
}

TEST(SpriteDest, NegativeCoordinatesFloorTowardMinusInfinity)
{
    const AnimFrame frame = MakeFrame_(10, 10, 0, 0);
    const IntRect   rect  = ComputeDestRect(frame, { -0.5f, -1.25f }, { 0.f, 0.f }, false);
    EXPECT_EQ(rect.x, -1);
    EXPECT_EQ(rect.y, -2);
}

TEST(SpriteDest, CameraIsSubtracted)
{
    const AnimFrame frame = MakeFrame_(10, 10, 5, 10);
    const IntRect   rect  = ComputeDestRect(frame, { 1000.f, 300.f }, { 960.f, 100.f }, false);
    EXPECT_EQ(rect, (IntRect { 35, 190, 10, 10 }));
}

TEST(SpriteDest, FlipMirrorsAroundTheOriginKeepingThePivotOnIt)
{
    const AnimFrame frame   = MakeFrame_(40, 20, 10, 20);
    const IntRect   normal  = ComputeDestRect(frame, { 100.f, 100.f }, { 0.f, 0.f }, false);
    const IntRect   flipped = ComputeDestRect(frame, { 100.f, 100.f }, { 0.f, 0.f }, true);

    EXPECT_EQ(normal.x, 90);   // [90, 130)
    EXPECT_EQ(normal.x + normal.w, 130);
    EXPECT_EQ(flipped.x, 70);   // [70, 110)
    EXPECT_EQ(flipped.x + flipped.w, 110);
    EXPECT_EQ(100 - normal.x, flipped.x + flipped.w - 100);
    EXPECT_EQ(normal.x + normal.w - 100, 100 - flipped.x);
    EXPECT_EQ(normal.y, flipped.y);
}

TEST(SpriteDest, FlipOfCenteredPivotIsIdenticalRect)
{
    const AnimFrame frame = MakeFrame_(40, 20, 20, 20);
    EXPECT_EQ(ComputeDestRect(frame, { 50.f, 50.f }, { 0.f, 0.f }, false), ComputeDestRect(frame, { 50.f, 50.f }, { 0.f, 0.f }, true));
}

TEST(SpriteDest, OddWidthFramesDoNotLoseAPixel)
{
    const AnimFrame frame = MakeFrame_(41, 10, 20, 10);
    const IntRect   a     = ComputeDestRect(frame, { 100.f, 100.f }, { 0.f, 0.f }, false);
    const IntRect   b     = ComputeDestRect(frame, { 100.f, 100.f }, { 0.f, 0.f }, true);
    EXPECT_EQ(a.w, 41);
    EXPECT_EQ(a.x, 80);
    EXPECT_EQ(b.x, 100 + 20 - 41);
}

TEST(SpriteSort, OrdersByDepthThenSortKeyThenSubmission)
{
    Vector<SpriteCommand> commands;

    AddCommand_(commands, 2, 0, 0);
    AddCommand_(commands, 1, 5, 1);
    AddCommand_(commands, 1, 5, 2);   // 키 동점: 제출 순서 유지 (안정)
    AddCommand_(commands, 1, 1, 3);
    AddCommand_(commands, 0, 99, 4);
    AddCommand_(commands, 1, 5, 5);

    SortSpriteCommands(commands);
    ASSERT_EQ(commands.size(), 6u);
    const ARRAY<int, 6> expected = { 4, 3, 1, 2, 5, 0 };
    for (size_t i = 0; i < commands.size(); ++i)
    {
        EXPECT_EQ(commands[i].frame.x, expected[i]) << "position " << i;
    }
}

TEST(SpriteSort, BatchSubmitAssignsSequenceAndFlushClearsQueue)
{
    SpriteBatch     batch;
    const AnimFrame frame = MakeFrame_(4, 4, 0, 0);
    batch.Submit(1, 0, frame, { 0.f, 0.f });
    batch.Submit(0, 0, frame, { 1.f, 0.f });
    batch.Submit(1, 0, frame, { 2.f, 0.f });
    ASSERT_EQ(batch.GetPendingCommands().size(), 3u);
    EXPECT_EQ(batch.GetPendingCommands()[0].seq, 0u);
    EXPECT_EQ(batch.GetPendingCommands()[2].seq, 2u);

    batch.Flush(Vec2 { 0.f, 0.f });   // 렌더러 없음 → 그리지 않고 큐만 비운다
    EXPECT_TRUE(batch.GetPendingCommands().empty());
}

TEST(SpriteSort, FlushBelowDepthDrawsOnlyLowLayersAndKeepsTheRestQueued)
{
    SpriteBatch     batch;
    const AnimFrame frame = MakeFrame_(4, 4, 0, 0);
    batch.Submit(5, 0, frame, { 0.f, 0.f });
    batch.Submit(-100, 0, frame, { 1.f, 0.f });
    batch.Submit(-96, 0, frame, { 2.f, 0.f });
    batch.Submit(-90, 0, frame, { 3.f, 0.f });
    batch.Submit(20, 0, frame, { 4.f, 0.f });

    batch.FlushBelowDepth(Vec2 { 0.f, 0.f }, -90);
    ASSERT_EQ(batch.GetPendingCommands().size(), 3u);
    EXPECT_EQ(batch.GetPendingCommands()[0].depth, -90);
    EXPECT_EQ(batch.GetPendingCommands()[1].depth, 5);
    EXPECT_EQ(batch.GetPendingCommands()[2].depth, 20);
}

struct SoftwareTarget
{
    SDL_Surface*  pSurface  = nullptr;
    SDL_Renderer* pRenderer = nullptr;

    explicit SoftwareTarget(
        const int _w,
        const int _h)
    {
        pSurface = SDL_CreateSurface(_w, _h, SDL_PIXELFORMAT_RGBA32);
        if (pSurface)
        {
            pRenderer = SDL_CreateSoftwareRenderer(pSurface);
        }
    }

    ~SoftwareTarget()
    {
        if (pRenderer)
        {
            SDL_DestroyRenderer(pRenderer);
        }
        if (pSurface)
        {
            SDL_DestroySurface(pSurface);
        }
    }

    [[nodiscard]] bool IsValid() const { return pRenderer != nullptr; }

    void Clear()
    {
        SDL_SetRenderDrawColor(pRenderer, 0, 0, 0, 255);
        SDL_RenderClear(pRenderer);
    }

    [[nodiscard]] uint32_t PixelRgb(
        const int _x,
        const int _y) const
    {
        SDL_FlushRenderer(pRenderer);
        Uint8 r = 0;
        Uint8 g = 0;
        Uint8 b = 0;
        Uint8 a = 0;
        SDL_ReadSurfacePixel(pSurface, _x, _y, &r, &g, &b, &a);
        return (static_cast<uint32_t>(r) << 16) | (static_cast<uint32_t>(g) << 8) | static_cast<uint32_t>(b);
    }
};

namespace
{
[[nodiscard]] Vector<uint8_t> MakeAtlas4x4_()
{
    Vector<uint8_t> pixels(4 * 4 * 4);
    for (int y = 0; y < 4; ++y)
    {
        for (int x = 0; x < 4; ++x)
        {
            uint8_t* pPixel = &pixels[static_cast<size_t>((y * 4 + x) * 4)];
            pPixel[0]       = x == 0 ? 255 : 0;
            pPixel[1]       = x == 0 ? 0 : 255;
            pPixel[2]       = 0;
            pPixel[3]       = 255;
        }
    }
    return pixels;
}
}   // namespace

constexpr uint32_t kRed   = 0xFF0000;
constexpr uint32_t kGreen = 0x00FF00;
constexpr uint32_t kBlack = 0x000000;

TEST(SpriteBatchSdl, DrawsFrameAtSnappedPositionWithPivot)
{
    SoftwareTarget target { 32, 32 };
    if (!target.IsValid())
    {
        GTEST_SKIP() << "소프트웨어 렌더러를 만들 수 없는 환경";
    }

    SpriteBatch batch;
    batch.Init(target.pRenderer);
    const Vector<uint8_t> atlas = MakeAtlas4x4_();
    ASSERT_TRUE(batch.AddAtlasPageFromRgba(4, 4, atlas).IsOK());
    EXPECT_EQ(batch.GetAtlasPageCount(), 1u);

    const AnimFrame frame = MakeFrame_(4, 4, 2, 4);   // 원점 = 프레임 하단 중앙
    target.Clear();
    batch.DrawFrame(frame, { 10.7f, 20.9f }, Vec2 { 0.f, 0.f });

    EXPECT_EQ(target.PixelRgb(8, 16), kRed);
    EXPECT_EQ(target.PixelRgb(9, 16), kGreen);
    EXPECT_EQ(target.PixelRgb(11, 19), kGreen);
    EXPECT_EQ(target.PixelRgb(7, 16), kBlack);
    EXPECT_EQ(target.PixelRgb(12, 16), kBlack);
    EXPECT_EQ(target.PixelRgb(8, 15), kBlack);
    EXPECT_EQ(target.PixelRgb(8, 20), kBlack);
}

TEST(SpriteBatchSdl, ScaleShrinksAroundThePivotAndLeavesTheDefaultUnchanged)
{
    SoftwareTarget target { 32, 32 };
    if (!target.IsValid())
    {
        GTEST_SKIP() << "소프트웨어 렌더러를 만들 수 없는 환경";
    }

    SpriteBatch batch;
    batch.Init(target.pRenderer);
    const Vector<uint8_t> atlas = MakeAtlas4x4_();
    ASSERT_TRUE(batch.AddAtlasPageFromRgba(4, 4, atlas).IsOK());
    const AnimFrame frame = MakeFrame_(4, 4, 2, 4);   // 원점 = 프레임 하단 중앙

    // 배율 0.5: 피벗 (10,20) 은 그대로 두고 4x4 가 2x2 로 줄어 (9..10, 18..19) 를 덮는다
    SpriteDrawOptions half;
    half.scale = 0.5f;
    target.Clear();
    batch.DrawFrame(frame, { 10.7f, 20.9f }, Vec2 { 0.f, 0.f }, half);
    EXPECT_EQ(target.PixelRgb(8, 18), kBlack);
    EXPECT_EQ(target.PixelRgb(9, 17), kBlack);
    EXPECT_NE(target.PixelRgb(9, 18), kBlack);
    EXPECT_NE(target.PixelRgb(10, 19), kBlack);
    EXPECT_EQ(target.PixelRgb(11, 18), kBlack);
    EXPECT_EQ(target.PixelRgb(10, 20), kBlack);
    EXPECT_EQ(target.PixelRgb(8, 16), kBlack);

    // 기본값 (1) 은 원래 4x4 (8..11, 16..19) 그대로
    target.Clear();
    batch.DrawFrame(frame, { 10.7f, 20.9f }, Vec2 { 0.f, 0.f }, SpriteDrawOptions {});
    EXPECT_EQ(target.PixelRgb(8, 16), kRed);
    EXPECT_EQ(target.PixelRgb(11, 19), kGreen);
    EXPECT_EQ(target.PixelRgb(12, 16), kBlack);
}

TEST(SpriteDrawOptionsDefaults, ScaleIsOneByDefault)
{
    EXPECT_FLOAT_EQ(SpriteDrawOptions {}.scale, 1.f);
}

TEST(SpriteBatchSdl, FlipXMirrorsPixelsAroundOrigin)
{
    SoftwareTarget target { 32, 32 };
    if (!target.IsValid())
    {
        GTEST_SKIP() << "소프트웨어 렌더러를 만들 수 없는 환경";
    }

    SpriteBatch batch;
    batch.Init(target.pRenderer);
    const Vector<uint8_t> atlas = MakeAtlas4x4_();
    ASSERT_TRUE(batch.AddAtlasPageFromRgba(4, 4, atlas).IsOK());

    const AnimFrame frame = MakeFrame_(4, 4, 2, 4);
    target.Clear();
    SpriteDrawOptions options;
    options.bFlipX = true;
    batch.DrawFrame(frame, { 10.f, 20.f }, Vec2 { 0.f, 0.f }, options);

    EXPECT_EQ(target.PixelRgb(11, 16), kRed);
    EXPECT_EQ(target.PixelRgb(8, 16), kGreen);
}

TEST(SpriteBatchSdl, FlushDrawsInSortedOrderLaterOnTop)
{
    SoftwareTarget target { 32, 32 };
    if (!target.IsValid())
    {
        GTEST_SKIP() << "소프트웨어 렌더러를 만들 수 없는 환경";
    }

    SpriteBatch batch;
    batch.Init(target.pRenderer);
    const Vector<uint8_t> atlas = MakeAtlas4x4_();
    ASSERT_TRUE(batch.AddAtlasPageFromRgba(4, 4, atlas).IsOK());

    const AnimFrame red   = MakeFrame_(1, 1, 0, 0);   // atlas (0,0) = 빨강
    AnimFrame       green = MakeFrame_(1, 1, 0, 0);
    green.x               = 1;   // atlas (1,0) = 초록

    target.Clear();
    batch.Submit(1, 0, red, { 5.f, 5.f });
    batch.Submit(0, 0, green, { 5.f, 5.f });
    batch.Flush(Vec2 { 0.f, 0.f });
    EXPECT_EQ(target.PixelRgb(5, 5), kRed);

    target.Clear();
    batch.Submit(0, 7, red, { 5.f, 5.f });
    batch.Submit(0, 3, green, { 5.f, 5.f });   // sortKey 가 작은 쪽이 아래
    batch.Flush(Vec2 { 0.f, 0.f });
    EXPECT_EQ(target.PixelRgb(5, 5), kRed);
}

TEST(SpriteBatchSdl, EmptyFramesAndMissingPagesAreSkipped)
{
    SoftwareTarget target { 16, 16 };
    if (!target.IsValid())
    {
        GTEST_SKIP() << "소프트웨어 렌더러를 만들 수 없는 환경";
    }

    SpriteBatch batch;
    batch.Init(target.pRenderer);
    target.Clear();

    const AnimFrame empty = MakeFrame_(0, 0, 0, 0);
    batch.DrawFrame(empty, { 5.f, 5.f }, Vec2 { 0.f, 0.f });   // 크래시 없이 무시

    AnimFrame noPage = MakeFrame_(4, 4, 0, 0);
    noPage.atlas     = 3;
    batch.DrawFrame(noPage, { 5.f, 5.f }, Vec2 { 0.f, 0.f });
    EXPECT_EQ(target.PixelRgb(5, 5), kBlack);
}

// 가로로 아주 긴 단독 텍스처(Stage0_Main 14094px) 는 내부에서 띠로 쪼개 그린다: 띠 경계(2048 배수)를 가로질러도 열이 어긋나지 않아야 한다
TEST(SpriteBatchSdl, WideStandaloneTextureIsSeamlessAcrossStrips)
{
    SoftwareTarget target { 64, 4 };
    if (!target.IsValid())
    {
        GTEST_SKIP() << "소프트웨어 렌더러를 만들 수 없는 환경";
    }

    constexpr int   kW = 5000;
    constexpr int   kH = 4;
    Vector<uint8_t> pixels(static_cast<size_t>(kW) * kH * 4);
    for (int y = 0; y < kH; ++y)
    {
        for (int x = 0; x < kW; ++x)
        {
            uint8_t* pPixel = &pixels[static_cast<size_t>((y * kW + x) * 4)];
            pPixel[0]       = static_cast<uint8_t>(x & 0xFF);
            pPixel[1]       = static_cast<uint8_t>((x >> 8) & 0xFF);
            pPixel[2]       = 7;
            pPixel[3]       = 255;
        }
    }

    SpriteBatch batch;
    batch.Init(target.pRenderer);
    const Result<int> handle = batch.AddStandaloneTextureFromRgba(kW, kH, pixels);
    ASSERT_FALSE(handle.HasError());

    const ARRAY<int, 3> cams = { 2030, 4070, 4950 };
    for (const int cam: cams)
    {
        target.Clear();
        batch.DrawTexture(*handle, { 0.f, 0.f }, Vec2 { static_cast<float>(cam), 0.f }, 64, kH);
        const int visibleW = std::min(64, kW - cam);
        for (int sx = 0; sx < visibleW; ++sx)
        {
            const int      wx       = cam + sx;
            const uint32_t expected = (static_cast<uint32_t>(wx & 0xFF) << 16) | (static_cast<uint32_t>((wx >> 8) & 0xFF) << 8) | 7u;
            ASSERT_EQ(target.PixelRgb(sx, 2), expected) << "cam=" << cam << " sx=" << sx;
        }
    }
}

TEST(SpriteBatchSdl, RejectsInvalidAtlasData)
{
    SoftwareTarget target { 16, 16 };
    if (!target.IsValid())
    {
        GTEST_SKIP() << "소프트웨어 렌더러를 만들 수 없는 환경";
    }

    SpriteBatch batch;
    batch.Init(target.pRenderer);
    const Vector<uint8_t> tooSmall(8);
    EXPECT_TRUE(batch.AddAtlasPageFromRgba(4, 4, tooSmall).HasError());
    EXPECT_TRUE(batch.AddAtlasPageFromRgba(0, 4, tooSmall).HasError());
    EXPECT_EQ(batch.GetAtlasPageCount(), 0u);
}

TEST(SpriteDrawOptionsDefaults, FlashIsOffByDefault)
{
    const SpriteDrawOptions options;
    EXPECT_FLOAT_EQ(options.flashAmount, 0.f);
    EXPECT_FALSE(options.bFlipX);
}

TEST(SpriteBatchSdl, HitFlashDrawsOrangeRampVariantOnlyWhenRequested)
{
    SoftwareTarget target { 32, 32 };
    if (!target.IsValid())
    {
        GTEST_SKIP() << "소프트웨어 렌더러를 만들 수 없는 환경";
    }

    SpriteBatch batch;
    batch.Init(target.pRenderer);
    const Vector<uint8_t> gray = { 100, 100, 100, 255 };
    ASSERT_TRUE(batch.AddAtlasPageFromRgba(1, 1, gray).IsOK());
    const AnimFrame frame = MakeFrame_(1, 1, 0, 1);

    target.Clear();
    batch.DrawFrame(frame, { 10.f, 20.f }, Vec2 { 0.f, 0.f });
    EXPECT_EQ(target.PixelRgb(10, 19), 0x646464u);

    target.Clear();
    SpriteDrawOptions options;
    options.flashAmount = 1.f;
    batch.DrawFrame(frame, { 10.f, 20.f }, Vec2 { 0.f, 0.f }, options);
    // 평균 밝기 100 -> 램프 (약 225,137,44): 밝기 100 회색이 주황으로 바뀐다
    const RGBA     expected = ApplyHitFlashRamp({ 100, 100, 100, 255 });
    const uint32_t px       = target.PixelRgb(10, 19);
    EXPECT_NEAR(static_cast<int>((px >> 16) & 0xFFu), expected.r, 2);
    EXPECT_NEAR(static_cast<int>((px >> 8) & 0xFFu), expected.g, 2);
    EXPECT_NEAR(static_cast<int>(px & 0xFFu), expected.b, 2);
    EXPECT_GT(expected.r, 200);
    EXPECT_LT(expected.b, 80);

    target.Clear();
    batch.DrawFrame(frame, { 10.f, 20.f }, Vec2 { 0.f, 0.f });
    EXPECT_EQ(target.PixelRgb(10, 19), 0x646464u) << "플래시 뒤에도 일반 프레임은 원래 색이어야 한다";
}

TEST(SpriteBatchSdl, HitFlashRampIsMonotoneKeepsAlphaAndMapsBlackToDarkOrange)
{
    int prevR = -1;
    for (int v = 0; v <= 255; ++v)
    {
        const RGBA c = ApplyHitFlashRamp({ static_cast<uint8_t>(v), static_cast<uint8_t>(v), static_cast<uint8_t>(v), 77 });
        EXPECT_EQ(c.a, 77);
        EXPECT_GE(static_cast<int>(c.r), prevR) << "v=" << v;
        EXPECT_GE(c.r, c.g);
        EXPECT_GE(c.g, c.b);
        prevR = c.r;
    }
    const RGBA black = ApplyHitFlashRamp({ 0, 0, 0, 255 });
    EXPECT_GT(black.r, 120) << "원작: 어두운 픽셀도 붉은 주황으로 크게 떠오른다";
    EXPECT_LT(black.b, 10);
}

// 지형 텍스처보다 뒤 레이어는 텍스처 아래에 깔리고, 나머지는 텍스처 위에 그려진다
TEST(SpriteBatchSdl, FlushBelowDepthLetsTheTerrainTextureCoverLowLayers)
{
    SoftwareTarget target { 16, 16 };
    if (!target.IsValid())
    {
        GTEST_SKIP() << "소프트웨어 렌더러를 만들 수 없는 환경";
    }

    SpriteBatch batch;
    batch.Init(target.pRenderer);
    const Vector<uint8_t> atlas = MakeAtlas4x4_();
    ASSERT_TRUE(batch.AddAtlasPageFromRgba(4, 4, atlas).IsOK());

    Vector<uint8_t> terrain(4 * 4 * 4, 0);   // 왼쪽 2열 불투명 파랑, 오른쪽 2열 투명
    for (int y = 0; y < 4; ++y)
    {
        for (int x = 0; x < 2; ++x)
        {
            uint8_t* pPixel = &terrain[static_cast<size_t>((y * 4 + x) * 4)];
            pPixel[2]       = 255;
            pPixel[3]       = 255;
        }
    }
    const Result<int> handle = batch.AddStandaloneTextureFromRgba(4, 4, terrain);
    ASSERT_FALSE(handle.HasError());

    const AnimFrame red   = MakeFrame_(1, 1, 0, 0);
    AnimFrame       green = MakeFrame_(1, 1, 0, 0);
    green.x               = 1;

    target.Clear();
    batch.Submit(-100, 0, red, { 0.f, 0.f });   // 지형 아래: 불투명 구간에서는 가려진다
    batch.Submit(-100, 0, red, { 3.f, 0.f });   // 지형 아래지만 투명 구간이라 보인다
    batch.Submit(10, 0, green, { 1.f, 1.f });   // 지형 위
    batch.FlushBelowDepth(Vec2 { 0.f, 0.f }, -90);
    batch.DrawTexture(*handle, { 0.f, 0.f }, Vec2 { 0.f, 0.f }, 16, 16);
    batch.Flush(Vec2 { 0.f, 0.f });

    EXPECT_EQ(target.PixelRgb(0, 0), 0x0000FFu);
    EXPECT_EQ(target.PixelRgb(3, 0), kRed);
    EXPECT_EQ(target.PixelRgb(1, 1), kGreen);
}

namespace
{
[[nodiscard]] Vector<FilePath> AtlasPagePaths_(
    const int _count)
{
    Vector<FilePath> paths;
    for (int i = 0; i < _count; ++i)
    {
        paths.push_back(FindAssetRoot() / "Atlas" / std::format("atlas_{:03}.png", i % 2));
    }
    return paths;
}
}   // namespace

TEST(DecodeImages, ParallelAndSequentialResultsAreIdenticalInRequestOrder)
{
    Vector<FilePath> paths = AtlasPagePaths_(5);
    if (!std::filesystem::exists(paths[0]) || !std::filesystem::exists(paths[1]))
    {
        GTEST_SKIP() << "Atlas 페이지가 없는 환경";
    }

    const Vector<Result<DecodedImage>> sequential = DecodeImages(paths, true, 1);
    const Vector<Result<DecodedImage>> parallel   = DecodeImages(paths, true, 4);
    ASSERT_EQ(sequential.size(), paths.size());
    ASSERT_EQ(parallel.size(), paths.size());
    for (size_t i = 0; i < paths.size(); ++i)
    {
        ASSERT_TRUE(sequential[i].HasValue());
        ASSERT_TRUE(parallel[i].HasValue());
        EXPECT_EQ(parallel[i]->width, sequential[i]->width);
        EXPECT_EQ(parallel[i]->rgba, sequential[i]->rgba);
        EXPECT_EQ(parallel[i]->flashRgba, sequential[i]->flashRgba);
        EXPECT_EQ(sequential[i]->flashRgba.size(), sequential[i]->rgba.size());
        EXPECT_EQ(sequential[i]->rgba, sequential[i % 2]->rgba) << "같은 파일이 같은 순서 칸에 들어간다";
    }
    EXPECT_NE(sequential[0]->rgba, sequential[1]->rgba);

    const Vector<Result<DecodedImage>> noFlash = DecodeImages(Span<const FilePath> { paths.data(), 1 }, false, 2);
    ASSERT_TRUE(noFlash[0].HasValue());
    EXPECT_TRUE(noFlash[0]->flashRgba.empty());
}

TEST(DecodeImages, MissingFileIsReportedAsFileOpenAtItsOwnPosition)
{
    Vector<FilePath> paths = { FindAssetRoot() / "Atlas" / "no_such_page.png", FindAssetRoot() / "Atlas" / "no_such_page2.png" };
    for (const int numWorkers: { 1, 4 })
    {
        const Vector<Result<DecodedImage>> results = DecodeImages(paths, true, numWorkers);
        ASSERT_EQ(results.size(), 2u);
        for (const Result<DecodedImage>& result: results)
        {
            ASSERT_TRUE(result.HasError());
            EXPECT_TRUE(HasErrorCode(result.GetError(), eError::FileOpen));
        }
    }
}

TEST(SpriteBatchSdl, AddAtlasPageFromDecodedImageDrawsLikeTheRgbaPath)
{
    SoftwareTarget target { 32, 32 };
    if (!target.IsValid())
    {
        GTEST_SKIP() << "소프트웨어 렌더러를 만들 수 없는 환경";
    }

    DecodedImage image;
    image.width  = 4;
    image.height = 4;
    image.rgba   = MakeAtlas4x4_();

    SpriteBatch batch;
    batch.Init(target.pRenderer);
    ASSERT_TRUE(batch.AddAtlasPage(image).IsOK());
    EXPECT_EQ(batch.GetAtlasPageCount(), 1u);

    const AnimFrame frame = MakeFrame_(4, 4, 2, 4);
    target.Clear();
    batch.DrawFrame(frame, { 10.7f, 20.9f }, Vec2 { 0.f, 0.f });
    EXPECT_EQ(target.PixelRgb(8, 16), kRed);
    EXPECT_EQ(target.PixelRgb(9, 16), kGreen);
    EXPECT_EQ(target.PixelRgb(7, 16), kBlack);
}

TEST(SpriteBatchSdl, AddAtlasPageUsesTheGivenFlashImageOrBuildsOne)
{
    SoftwareTarget target { 8, 8 };
    if (!target.IsValid())
    {
        GTEST_SKIP() << "소프트웨어 렌더러를 만들 수 없는 환경";
    }

    DecodedImage built;
    built.width  = 1;
    built.height = 1;
    built.rgba   = { 128, 128, 128, 255 };

    DecodedImage given = built;
    given.flashRgba    = { 1, 2, 3, 255 };

    SpriteBatch batch;
    batch.Init(target.pRenderer);
    ASSERT_TRUE(batch.AddAtlasPage(built).IsOK());
    ASSERT_TRUE(batch.AddAtlasPage(given).IsOK());

    AnimFrame frame0 = MakeFrame_(1, 1, 0, 0);
    AnimFrame frame1 = frame0;
    frame1.atlas     = 1;
    SpriteDrawOptions flash;
    flash.flashAmount = 1.f;

    target.Clear();
    batch.DrawFrame(frame0, { 0.f, 0.f }, Vec2 { 0.f, 0.f }, flash);
    batch.DrawFrame(frame1, { 2.f, 0.f }, Vec2 { 0.f, 0.f }, flash);
    const RGBA expected = ApplyHitFlashRamp({ 128, 128, 128, 255 });
    EXPECT_EQ(target.PixelRgb(0, 0), (static_cast<uint32_t>(expected.r) << 16) | (static_cast<uint32_t>(expected.g) << 8) | expected.b);
    EXPECT_EQ(target.PixelRgb(2, 0), 0x010203u);
}

TEST(SpriteBatchSdl, AddStandaloneTextureReturnsSequentialHandlesAndRejectsBadImages)
{
    SoftwareTarget target { 8, 8 };
    if (!target.IsValid())
    {
        GTEST_SKIP() << "소프트웨어 렌더러를 만들 수 없는 환경";
    }

    DecodedImage image;
    image.width  = 2;
    image.height = 2;
    image.rgba   = Vector<uint8_t>(2 * 2 * 4, 255);

    SpriteBatch batch;
    batch.Init(target.pRenderer);
    const Result<int> first  = batch.AddStandaloneTexture(image);
    const Result<int> second = batch.AddStandaloneTexture(image);
    ASSERT_TRUE(first.HasValue());
    ASSERT_TRUE(second.HasValue());
    EXPECT_EQ(first.GetValue(), 0);
    EXPECT_EQ(second.GetValue(), 1);

    DecodedImage tooSmall;
    tooSmall.width  = 4;
    tooSmall.height = 4;
    tooSmall.rgba   = Vector<uint8_t>(8, 0);
    EXPECT_TRUE(batch.AddStandaloneTexture(tooSmall).HasError());
    EXPECT_TRUE(batch.AddAtlasPage(tooSmall).HasError());
    EXPECT_EQ(batch.GetAtlasPageCount(), 0u);
}

TEST(SpriteBatchSdl, LoadAtlasPagesParallelAddsEveryPageInChunks)
{
    SoftwareTarget target { 8, 8 };
    if (!target.IsValid())
    {
        GTEST_SKIP() << "소프트웨어 렌더러를 만들 수 없는 환경";
    }
    Result<AnimationLibrary> library = AnimationLibrary::LoadFromDirectory(FindAssetRoot());
    if (!library)
    {
        GTEST_SKIP() << "에셋이 없는 환경";
    }

    for (const int numWorkers: { 1, 3 })
    {
        SpriteBatch batch;
        batch.Init(target.pRenderer);
        ASSERT_TRUE(batch.LoadAtlasPagesParallel(library.GetValue(), numWorkers));
        EXPECT_EQ(batch.GetAtlasPageCount(), library->GetAtlasPages().size());
    }
}
