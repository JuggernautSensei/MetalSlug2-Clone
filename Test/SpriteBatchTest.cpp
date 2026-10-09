#include <gtest/gtest.h>
#include "Base/Types.h"
#include "Base/Math.h"

#include <SDL3/SDL.h>

#include "Render/SpriteBatch.h"

using namespace slug;

namespace
{
AnimFrame MakeFrame(
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
}

TEST(SpriteDest, PivotIsSubtractedFromSnappedOrigin)
{
    const AnimFrame frame = MakeFrame(40, 52, 21, 50);
    const IntRect   rect  = ComputeDestRect(frame, { 100.f, 200.f }, { 0.f, 0.f }, false);
    EXPECT_EQ(rect, (IntRect { 79, 150, 40, 52 }));
}

TEST(SpriteDest, OriginIsFlooredSoSubpixelMotionNeverChangesSize)
{
    const AnimFrame frame = MakeFrame(10, 10, 5, 10);
    const IntRect   a     = ComputeDestRect(frame, { 100.0f, 50.0f }, { 0.f, 0.f }, false);
    const IntRect   b     = ComputeDestRect(frame, { 100.99f, 50.99f }, { 0.f, 0.f }, false);
    const IntRect   c     = ComputeDestRect(frame, { 101.0f, 51.0f }, { 0.f, 0.f }, false);
    EXPECT_EQ(a, b);
    EXPECT_EQ(c.x, a.x + 1);
    EXPECT_EQ(c.y, a.y + 1);
    EXPECT_EQ(a.w, b.w);
    EXPECT_EQ(a.h, c.h);
}

TEST(SpriteDest, NegativeCoordinatesFloorTowardMinusInfinity)
{
    const AnimFrame frame = MakeFrame(10, 10, 0, 0);
    const IntRect   rect  = ComputeDestRect(frame, { -0.5f, -1.25f }, { 0.f, 0.f }, false);
    EXPECT_EQ(rect.x, -1);
    EXPECT_EQ(rect.y, -2);
}

TEST(SpriteDest, CameraIsSubtracted)
{
    const AnimFrame frame = MakeFrame(10, 10, 5, 10);
    const IntRect   rect  = ComputeDestRect(frame, { 1000.f, 300.f }, { 960.f, 100.f }, false);
    EXPECT_EQ(rect, (IntRect { 35, 190, 10, 10 }));
}

TEST(SpriteDest, FlipMirrorsAroundTheOriginKeepingThePivotOnIt)
{
    const AnimFrame frame   = MakeFrame(40, 20, 10, 20);
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
    const AnimFrame frame = MakeFrame(40, 20, 20, 20);
    EXPECT_EQ(ComputeDestRect(frame, { 50.f, 50.f }, { 0.f, 0.f }, false), ComputeDestRect(frame, { 50.f, 50.f }, { 0.f, 0.f }, true));
}

TEST(SpriteDest, OddWidthFramesDoNotLoseAPixel)
{
    const AnimFrame frame = MakeFrame(41, 10, 20, 10);
    const IntRect   a     = ComputeDestRect(frame, { 100.f, 100.f }, { 0.f, 0.f }, false);
    const IntRect   b     = ComputeDestRect(frame, { 100.f, 100.f }, { 0.f, 0.f }, true);
    EXPECT_EQ(a.w, 41);
    EXPECT_EQ(a.x, 80);
    EXPECT_EQ(b.x, 100 + 20 - 41);
}

TEST(SpriteSort, OrdersByLayerThenSortKeyThenSubmission)
{
    slug::Vector<SpriteCommand> commands;
    auto                       Add = [&](const int _layer, const int _key, const int _tag) {
        SpriteCommand command;
        command.layer   = _layer;
        command.sortKey = _key;
        command.seq     = static_cast<uint32_t>(commands.size());
        command.frame.x = _tag;
        commands.push_back(command);
    };

    Add(2, 0, 0);
    Add(1, 5, 1);
    Add(1, 5, 2);   // 키 동점: 제출 순서 유지 (안정)
    Add(1, 1, 3);
    Add(0, 99, 4);
    Add(1, 5, 5);

    SortSpriteCommands(commands);
    ASSERT_EQ(commands.size(), 6u);
    const int expected[] = { 4, 3, 1, 2, 5, 0 };
    for (size_t i = 0; i < commands.size(); ++i)
    {
        EXPECT_EQ(commands[i].frame.x, expected[i]) << "position " << i;
    }
}

TEST(SpriteSort, BatchSubmitAssignsSequenceAndFlushClearsQueue)
{
    SpriteBatch batch;
    AnimFrame   frame = MakeFrame(4, 4, 0, 0);
    batch.Submit(1, 0, frame, { 0.f, 0.f });
    batch.Submit(0, 0, frame, { 1.f, 0.f });
    batch.Submit(1, 0, frame, { 2.f, 0.f });
    ASSERT_EQ(batch.GetPendingCommands().size(), 3u);
    EXPECT_EQ(batch.GetPendingCommands()[0].seq, 0u);
    EXPECT_EQ(batch.GetPendingCommands()[2].seq, 2u);

    batch.Flush(Vec2 { 0.f, 0.f });   // 렌더러 없음 → 그리지 않고 큐만 비운다
    EXPECT_TRUE(batch.GetPendingCommands().empty());
}

namespace
{
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

slug::Vector<uint8_t> MakeAtlas4x4()
{
    slug::Vector<uint8_t> pixels(4 * 4 * 4);
    for (int y = 0; y < 4; ++y)
    {
        for (int x = 0; x < 4; ++x)
        {
            uint8_t* p = &pixels[static_cast<size_t>((y * 4 + x) * 4)];
            p[0]       = x == 0 ? 255 : 0;
            p[1]       = x == 0 ? 0 : 255;
            p[2]       = 0;
            p[3]       = 255;
        }
    }
    return pixels;
}

constexpr uint32_t kRed   = 0xFF0000;
constexpr uint32_t kGreen = 0x00FF00;
constexpr uint32_t kBlack = 0x000000;
}

TEST(SpriteBatchSdl, DrawsFrameAtSnappedPositionWithPivot)
{
    SoftwareTarget target { 32, 32 };
    if (!target.IsValid())
    {
        GTEST_SKIP() << "소프트웨어 렌더러를 만들 수 없는 환경";
    }

    SpriteBatch batch;
    batch.Init(target.pRenderer);
    const slug::Vector<uint8_t> atlas = MakeAtlas4x4();
    ASSERT_TRUE(batch.AddAtlasPageFromRgba(4, 4, atlas).IsOK());
    EXPECT_EQ(batch.GetAtlasPageCount(), 1u);

    AnimFrame frame = MakeFrame(4, 4, 2, 4);   // 원점 = 프레임 하단 중앙
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

TEST(SpriteBatchSdl, FlipXMirrorsPixelsAroundOrigin)
{
    SoftwareTarget target { 32, 32 };
    if (!target.IsValid())
    {
        GTEST_SKIP() << "소프트웨어 렌더러를 만들 수 없는 환경";
    }

    SpriteBatch batch;
    batch.Init(target.pRenderer);
    const slug::Vector<uint8_t> atlas = MakeAtlas4x4();
    ASSERT_TRUE(batch.AddAtlasPageFromRgba(4, 4, atlas).IsOK());

    AnimFrame frame = MakeFrame(4, 4, 2, 4);
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
    const slug::Vector<uint8_t> atlas = MakeAtlas4x4();
    ASSERT_TRUE(batch.AddAtlasPageFromRgba(4, 4, atlas).IsOK());

    AnimFrame red   = MakeFrame(1, 1, 0, 0);   // atlas (0,0) = 빨강
    AnimFrame green = MakeFrame(1, 1, 0, 0);
    green.x         = 1;   // atlas (1,0) = 초록

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

    AnimFrame empty = MakeFrame(0, 0, 0, 0);
    batch.DrawFrame(empty, { 5.f, 5.f }, Vec2 { 0.f, 0.f });   // 크래시 없이 무시

    AnimFrame noPage = MakeFrame(4, 4, 0, 0);
    noPage.atlas     = 3;
    batch.DrawFrame(noPage, { 5.f, 5.f }, Vec2 { 0.f, 0.f });
    EXPECT_EQ(target.PixelRgb(5, 5), kBlack);
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
    const slug::Vector<uint8_t> tooSmall(8);
    EXPECT_TRUE(batch.AddAtlasPageFromRgba(4, 4, tooSmall).HasError());
    EXPECT_TRUE(batch.AddAtlasPageFromRgba(0, 4, tooSmall).HasError());
    EXPECT_TRUE(batch.LoadAtlasPage("this/file/does/not/exist.png").HasError());
    EXPECT_EQ(batch.GetAtlasPageCount(), 0u);
}
