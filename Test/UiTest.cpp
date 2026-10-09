#include <filesystem>
#include <map>

#include <gtest/gtest.h>

#include "Audio/AudioSink.h"
#include "Game/GameState.h"
#include "Game/SandboxState.h"
#include "Game/SimRunner.h"
#include "Game/StateFactory.h"
#include "Game/TitleState.h"
#include "Game/Ui/Banner.h"
#include "Game/Ui/BitmapFont.h"
#include "Game/Ui/FadeController.h"
#include "Game/Ui/Hud.h"
#include "Game/Ui/ScreenFlow.h"
#include "Game/Ui/UiLayout.h"
#include "Game/Ui/UiSystem.h"
#include "PlayerTestUtil.h"

using namespace slug;

namespace
{
    [[nodiscard]] const HudItem* FindItem(
        const jug::Vector<HudItem>& _items,
        const eHudItemKind          _kind,
        const jug::StringView       _text)
    {
        for (const HudItem& item: _items)
        {
            if (item.kind == _kind && item.text == _text)
            {
                return &item;
            }
        }
        return nullptr;
    }

    [[nodiscard]] int CountKind(
        const jug::Vector<HudItem>& _items,
        const eHudItemKind          _kind)
    {
        int count = 0;
        for (const HudItem& item: _items)
        {
            count += item.kind == _kind ? 1 : 0;
        }
        return count;
    }

    // 스프라이트 크기 (Baker 산출물과 같다: PlayerStateUI 124x34 등)
    [[nodiscard]] jug::VECTOR2 SpriteSize(
        const jug::StringView _name)
    {
        static const std::map<jug::String, jug::VECTOR2> kSizes = {
            { "PlayerStateUI_Bomb", { 124.f, 34.f } },
            { "InfiniteUI_Red", { 40.f, 16.f } },
            { "PressStartUI", { 174.f, 30.f } },
            { "PlayerHPUI", { 106.f, 16.f } },
            { "LEVEL_1", { 96.f, 16.f } },
        };
        const auto it = kSizes.find(jug::String { _name });
        return it == kSizes.end() ? jug::VECTOR2 { 0.f, 0.f } : it->second;
    }

    [[nodiscard]] bool InsideScreen(
        const IntRect&  _r,
        const UiLayout& _layout)
    {
        return _r.x >= 0 && _r.y >= 0 && _r.x + _r.w <= _layout.screenW && _r.y + _r.h <= _layout.screenH;
    }

    // 항목 하나가 화면 안에 완전히 들어가는가
    [[nodiscard]] bool ItemFits(
        const HudItem&  _item,
        const UiLayout& _layout)
    {
        switch (_item.kind)
        {
            case eHudItemKind::Text: return InsideScreen(LayoutText(_item.text, _item.x, _item.y, _item.style).bounds, _layout);
            case eHudItemKind::Sprite:
            {
                const jug::VECTOR2 size = SpriteSize(_item.text);
                const int          w    = static_cast<int>(size.x) * _item.scale;
                const int          h    = static_cast<int>(size.y) * _item.scale;
                return InsideScreen({ _item.x - w / 2, _item.y - h / 2, w, h }, _layout);
            }
            case eHudItemKind::TimeDigits:
            {
                const int w = static_cast<int>(_item.text.size()) * _layout.timeGlyphW;
                return InsideScreen({ _item.x - w / 2, _item.y, w, _layout.timeGlyphH }, _layout);
            }
            case eHudItemKind::Rect: return InsideScreen({ _item.x, _item.y, _item.w, _item.h }, _layout);
        }
        return false;
    }

    [[nodiscard]] HudModel MakeModel()
    {
        HudModel model;
        model.lives = 3;
        model.score = 12'340;
        model.bombs = 10;
        return model;
    }
}   // namespace

// ===========================================
//  글리프 매핑 / 폭 계산
// ===========================================

TEST(UiFont, GlyphIndexMapsLettersDigitsAndZero)
{
    EXPECT_EQ(fontmap::GetGlyphIndex('A'), 0);
    EXPECT_EQ(fontmap::GetGlyphIndex('Z'), 25);
    EXPECT_EQ(fontmap::GetGlyphIndex('a'), 0);   // 소문자는 대문자와 같은 글리프
    EXPECT_EQ(fontmap::GetGlyphIndex('O'), 14);
    EXPECT_EQ(fontmap::GetGlyphIndex('1'), 26);
    EXPECT_EQ(fontmap::GetGlyphIndex('9'), 34);
    EXPECT_EQ(fontmap::GetGlyphIndex('0'), 14);   // 0 은 'O' 로 대신한다 (레거시와 동일)
}

TEST(UiFont, UnsupportedCharactersHaveNoGlyph)
{
    for (const char c: { '-', '.', ':', '!', '/', '+', '%', '(', ' ', '\t' })
    {
        EXPECT_EQ(fontmap::GetGlyphIndex(c), -1) << static_cast<int>(c);
        EXPECT_FALSE(fontmap::HasGlyph(c));
    }
}

TEST(UiFont, EveryColumnIsReachableAndStaysInsideTheStrip)
{
    std::map<int, int> hits;
    for (char c = 'A'; c <= 'Z'; ++c)
    {
        ++hits[fontmap::GetGlyphIndex(c)];
    }
    for (char c = '1'; c <= '9'; ++c)
    {
        ++hits[fontmap::GetGlyphIndex(c)];
    }
    EXPECT_EQ(hits.size(), static_cast<size_t>(fontmap::kGlyphColumns));
    for (const auto& [index, count]: hits)
    {
        EXPECT_GE(index, 0);
        EXPECT_LT(index, fontmap::kGlyphColumns);
        EXPECT_EQ(count, 1);
    }
}

TEST(UiFont, MeasureCharsIsAdvanceTimesCountMinusSpacingTail)
{
    EXPECT_EQ(fontmap::MeasureChars(0, 1), 0);
    EXPECT_EQ(fontmap::MeasureChars(1, 1), 16);
    EXPECT_EQ(fontmap::MeasureChars(5, 1), 80);
    EXPECT_EQ(fontmap::MeasureChars(5, 2), 160);
    EXPECT_EQ(fontmap::MeasureChars(5, 2, 3), 160 + 4 * 3);
}

TEST(UiFont, FitTextScalePicksLargestScaleThatFits)
{
    EXPECT_EQ(FitTextScale("MISSION COMPLETE", 576, 3), 2);   // 16자 x 16 = 256, 2배 = 512 <= 576, 3배 = 768 초과
    EXPECT_EQ(FitTextScale("GAME OVER", 576, 3), 3);          // 9자 x 48 = 432
    EXPECT_EQ(FitTextScale("GAME OVER", 576, 2), 2);          // 상한
    EXPECT_EQ(FitTextScale("A VERY VERY VERY LONG LINE OF BANNER TEXT HERE", 576, 3), 1);   // 안 들어가도 1
    EXPECT_EQ(FitTextScale("SHORT\nA MUCH LONGER SECOND LINE", 640, 3), 1);                  // 가장 긴 줄 기준
}

TEST(UiFont, MeasureTextSizeUsesWidestLineAndLineGap)
{
    TextStyle style;
    style.scale   = 2;
    style.lineGap = 4;
    EXPECT_EQ(MeasureTextWidth("AB\nABCD", style), 4 * 32);
    EXPECT_EQ(MeasureTextHeight("AB\nABCD", style), 2 * 32 + 1 * 4 * 2);
    EXPECT_EQ(MeasureTextHeight("AB", style), 32);
}

// ===========================================
//  텍스트 배치
// ===========================================

TEST(UiLayoutText, LeftAlignStartsAtAnchor)
{
    TextStyle style;
    const TextLayout layout = LayoutText("ABC", 10, 20, style);
    ASSERT_EQ(layout.quads.size(), 3u);
    EXPECT_EQ(layout.quads[0].dst, (IntRect { 10, 20, 16, 16 }));
    EXPECT_EQ(layout.quads[1].dst.x, 26);
    EXPECT_EQ(layout.quads[2].dst.x, 42);
    EXPECT_EQ(layout.bounds, (IntRect { 10, 20, 48, 16 }));
}

TEST(UiLayoutText, RightAlignEndsAtAnchor)
{
    TextStyle style;
    style.align = eTextAlign::Right;
    const TextLayout layout = LayoutText("1234", 148, 8, style);
    ASSERT_EQ(layout.quads.size(), 4u);
    EXPECT_EQ(layout.quads.back().dst.x + layout.quads.back().dst.w, 148);
    EXPECT_EQ(layout.quads.front().dst.x, 148 - 64);
    EXPECT_EQ(layout.bounds.x + layout.bounds.w, 148);
}

TEST(UiLayoutText, CenterAlignSplitsWidthAroundAnchor)
{
    TextStyle style;
    style.align = eTextAlign::Center;
    const TextLayout layout = LayoutText("ABCD", 320, 0, style);   // 폭 64
    EXPECT_EQ(layout.quads.front().dst.x, 320 - 32);
    EXPECT_EQ(layout.quads.back().dst.x + 16, 320 + 32);
}

TEST(UiLayoutText, MultiLineCentersEachLineAndStacksWithGap)
{
    TextStyle style;
    style.align   = eTextAlign::Center;
    style.scale   = 2;
    style.lineGap = 4;
    const TextLayout layout = LayoutText("AB\nABCD", 100, 50, style);
    ASSERT_EQ(layout.lines, 2);
    ASSERT_EQ(layout.quads.size(), 6u);
    // 1줄: 폭 64 → 100-32, 2줄: 폭 128 → 100-64
    EXPECT_EQ(layout.quads[0].dst.x, 68);
    EXPECT_EQ(layout.quads[2].dst.x, 36);
    EXPECT_EQ(layout.quads[0].dst.y, 50);
    EXPECT_EQ(layout.quads[2].dst.y, 50 + 32 + 4 * 2);
    EXPECT_EQ(layout.height, 32 + 8 + 32);
}

TEST(UiLayoutText, FlagsOnlyNonSpaceUnsupportedCharsForFallback)
{
    TextStyle style;
    const TextLayout layout = LayoutText("A-B C", 0, 0, style);
    ASSERT_EQ(layout.quads.size(), 5u);
    EXPECT_FALSE(layout.quads[0].bFallback);
    EXPECT_TRUE(layout.quads[1].bFallback);    // '-'
    EXPECT_EQ(layout.quads[1].glyph, -1);
    EXPECT_FALSE(layout.quads[3].bFallback);   // ' ' 는 칸만 차지
    EXPECT_EQ(layout.quads[4].dst.x, 4 * 16);
}

TEST(UiLayoutText, LetterSpacingAddsBetweenCharsOnly)
{
    TextStyle style;
    style.letterSpacing = 2;
    const TextLayout layout = LayoutText("ABC", 0, 0, style);
    EXPECT_EQ(layout.quads[1].dst.x, 18);
    EXPECT_EQ(layout.quads[2].dst.x, 36);
    EXPECT_EQ(layout.width, 3 * 16 + 2 * 2);
}

TEST(UiLayoutText, EmptyAndNewlineOnlyStringsAreSafe)
{
    TextStyle style;
    EXPECT_TRUE(LayoutText("", 5, 5, style).quads.empty());
    const TextLayout two = LayoutText("\n", 5, 5, style);
    EXPECT_TRUE(two.quads.empty());
    EXPECT_EQ(two.lines, 2);
}

// ===========================================
//  BitmapFont: 글리프 프레임 (합성 스프라이트 / 실제 Baked)
// ===========================================

TEST(UiBitmapFont, GlyphFrameCutsTheStripIntoSixteenPixelCells)
{
    SpriteLibrary library;
    SpriteInfo    strip;
    strip.name        = "Font_White";
    strip.frame       = { 3, 100, 200, 560, 32, 280, 16 };
    strip.width       = 560;
    strip.height      = 32;
    library.AddSprite(strip);

    BitmapFont font;
    ASSERT_TRUE(font.Init(library));
    ASSERT_TRUE(font.HasSprites());

    const AnimFrame a = font.GetGlyphFrame(eFontColor::White, true, 0);
    EXPECT_EQ(a.atlas, 3);
    EXPECT_EQ(a.x, 100);
    EXPECT_EQ(a.y, 200);
    EXPECT_EQ(a.w, 16);
    EXPECT_EQ(a.h, 16);

    const AnimFrame nine = font.GetGlyphFrame(eFontColor::White, false, 34);   // '9' 가는 글자
    EXPECT_EQ(nine.x, 100 + 34 * 16);
    EXPECT_EQ(nine.y, 200 + 16);
    EXPECT_EQ(nine.x + nine.w, 100 + 560);   // 스트립 오른쪽 끝을 넘지 않는다

    EXPECT_TRUE(font.GetGlyphFrame(eFontColor::Red, true, 0).IsEmpty());   // 없는 색
    EXPECT_TRUE(font.GetGlyphFrame(eFontColor::White, true, 35).IsEmpty());
    EXPECT_TRUE(font.GetGlyphFrame(eFontColor::White, true, -1).IsEmpty());
}

TEST(UiBitmapFont, WithoutSpritesInitFailsGracefully)
{
    BitmapFont font;
    EXPECT_FALSE(font.Init(SpriteLibrary {}));
    EXPECT_FALSE(font.HasSprites());
    EXPECT_TRUE(font.GetGlyphFrame(eFontColor::White, true, 0).IsEmpty());
    EXPECT_TRUE(font.GetTimeDigitFrame(3).IsEmpty());
}

TEST(UiBitmapFont, BakedFontStripsAndUiSpritesExistWithExpectedSizes)
{
    jug::Result<SpriteLibrary> library = SpriteLibrary::LoadFromDirectory(jug::FilePath { SLUG_ASSET_DIR } / "Baked");
    if (!library)
    {
        GTEST_SKIP() << "Assets/Baked 없음";
    }

    for (const char* name: { "Font_Normal", "Font_White", "Font_Red_Yellow", "Font_Yellow_White", "Font_Red", "Font_Yellow" })
    {
        const SpriteInfo* pInfo = library->Find(name);
        ASSERT_NE(pInfo, nullptr) << name;
        EXPECT_EQ(pInfo->frame.w, fontmap::kGlyphColumns * fontmap::kGlyphSize) << name;
        EXPECT_EQ(pInfo->frame.h, fontmap::kGlyphRows * fontmap::kGlyphSize) << name;
    }
    const SpriteInfo* pTime = library->Find("TimeNumberUI");
    ASSERT_NE(pTime, nullptr);
    EXPECT_EQ(pTime->frame.w, 320);
    EXPECT_EQ(pTime->frame.h, 32);

    for (const char* name: { "PlayerStateUI_Bomb", "InfiniteUI_Red", "PressStartUI", "PlayerHPUI", "LEVEL_1" })
    {
        const SpriteInfo* pInfo = library->Find(name);
        ASSERT_NE(pInfo, nullptr) << name;
        EXPECT_EQ(static_cast<float>(pInfo->frame.w), SpriteSize(name).x) << name;
        EXPECT_EQ(static_cast<float>(pInfo->frame.h), SpriteSize(name).y) << name;
    }

    BitmapFont font;
    EXPECT_TRUE(font.Init(*library));
    const AnimFrame nine = font.GetTimeDigitFrame(9);
    EXPECT_EQ(nine.x, pTime->frame.x + 9 * 32);
    EXPECT_EQ(nine.w, 32);
}

// ===========================================
//  배너 타임라인 (정확한 틱)
// ===========================================

TEST(UiBanner, AlphaCurveAtExactTicks)
{
    // 페이드 인 3, 유지 4, 페이드 아웃 3 (총 10)
    const float expected[] = { 0.f, 1.f / 3.f, 2.f / 3.f, 1.f, 1.f, 1.f, 1.f, 1.f, 2.f / 3.f, 1.f / 3.f, 0.f };
    for (int e = 0; e <= 10; ++e)
    {
        EXPECT_NEAR(ComputeBannerAlpha(e, 3, 4, 3), expected[e], 1e-5f) << "elapsed " << e;
    }
    EXPECT_FLOAT_EQ(ComputeBannerAlpha(-1, 3, 4, 3), 0.f);
    EXPECT_FLOAT_EQ(ComputeBannerAlpha(99, 3, 4, 3), 0.f);
}

TEST(UiBanner, ZeroLengthFadesJumpStraightToFull)
{
    EXPECT_FLOAT_EQ(ComputeBannerAlpha(0, 0, 5, 0), 1.f);
    EXPECT_FLOAT_EQ(ComputeBannerAlpha(4, 0, 5, 0), 1.f);
    EXPECT_FLOAT_EQ(ComputeBannerAlpha(5, 0, 5, 0), 0.f);
}

TEST(UiBanner, SystemRunsFadeInHoldFadeOutThenDeactivates)
{
    BannerSystem banner;
    EXPECT_FALSE(banner.IsActive());

    BannerSpec spec;
    spec.text         = "WARNING";
    spec.fadeInTicks  = 3;
    spec.holdTicks    = 4;
    spec.fadeOutTicks = 3;
    banner.Show(spec);

    ASSERT_TRUE(banner.IsActive());
    EXPECT_FLOAT_EQ(banner.GetAlpha(), 0.f);

    for (int tick = 1; tick <= 3; ++tick)
    {
        banner.Tick();
    }
    EXPECT_FLOAT_EQ(banner.GetAlpha(), 1.f);   // 페이드 인 끝

    for (int tick = 4; tick <= 7; ++tick)
    {
        banner.Tick();
        EXPECT_FLOAT_EQ(banner.GetAlpha(), 1.f) << "tick " << tick;
    }
    banner.Tick();   // 8
    EXPECT_LT(banner.GetAlpha(), 1.f);
    banner.Tick();   // 9
    EXPECT_TRUE(banner.IsActive());
    banner.Tick();   // 10 = 총 길이
    EXPECT_FALSE(banner.IsActive());
    EXPECT_FLOAT_EQ(banner.GetAlpha(), 0.f);
}

TEST(UiBanner, QueueRunsSpecsBackToBackAndShowReplaces)
{
    BannerSpec a;
    a.text = "ONE";
    a.fadeInTicks = 1; a.holdTicks = 1; a.fadeOutTicks = 1;
    BannerSpec b = a;
    b.text = "TWO";

    BannerSystem banner;
    banner.Enqueue(a);   // 비어 있으니 즉시 시작
    banner.Enqueue(b);
    EXPECT_EQ(banner.GetCurrent().text, "ONE");
    EXPECT_EQ(banner.GetQueuedCount(), 1u);

    for (int i = 0; i < 3; ++i)
    {
        banner.Tick();
    }
    ASSERT_TRUE(banner.IsActive());
    EXPECT_EQ(banner.GetCurrent().text, "TWO");
    EXPECT_EQ(banner.GetElapsedTicks(), 0);

    banner.Show(a);   // 즉시 교체 + 대기열 비움
    EXPECT_EQ(banner.GetCurrent().text, "ONE");
    EXPECT_EQ(banner.GetQueuedCount(), 0u);

    banner.Clear();
    EXPECT_FALSE(banner.IsActive());
}

TEST(UiBanner, MakeBannerSpecSplitsSecondsIntoFadesAndHold)
{
    const BannerSpec spec = MakeBannerSpec("MISSION 4 START", 3.f);
    EXPECT_EQ(spec.GetTotalTicks(), 180);
    EXPECT_EQ(spec.fadeInTicks, 18);
    EXPECT_EQ(spec.fadeOutTicks, 18);
    EXPECT_EQ(spec.holdTicks, 144);

    const BannerSpec shortSpec = MakeBannerSpec("X", 0.2f);   // 12틱 → 페이드 3 + 6 + 3
    EXPECT_EQ(shortSpec.GetTotalTicks(), 12);
    EXPECT_EQ(shortSpec.fadeInTicks, 3);
    EXPECT_EQ(shortSpec.holdTicks, 6);
}

TEST(UiBanner, StandardBannersFitTheScreenWidth)
{
    const UiLayout layout;
    for (const char* text: { "MISSION 4 START", "MISSION COMPLETE", "WARNING", "GAME OVER", "MISSION 4\nSTART" })
    {
        TextStyle style;
        style.align   = eTextAlign::Center;
        style.lineGap = layout.bannerLineGap;
        style.scale   = FitTextScale(text, layout.screenW - 2 * layout.bannerMargin, layout.bannerMaxScale);
        const int height = MeasureTextHeight(text, style);
        const TextLayout laid = LayoutText(text, layout.screenW / 2, layout.bannerCenterY - height / 2, style);
        EXPECT_TRUE(InsideScreen(laid.bounds, layout)) << text;
        EXPECT_GE(style.scale, 1) << text;
    }
}

// ===========================================
//  페이드
// ===========================================

TEST(UiFade, FadeToBlackReachesOneAtExactTick)
{
    FadeController fade;
    EXPECT_TRUE(fade.IsClear());

    fade.FadeToBlack(4);
    EXPECT_TRUE(fade.IsBusy());
    fade.Tick();
    EXPECT_NEAR(fade.GetAlpha(), 0.25f, 1e-5f);
    fade.Tick();
    EXPECT_NEAR(fade.GetAlpha(), 0.5f, 1e-5f);
    fade.Tick();
    EXPECT_NEAR(fade.GetAlpha(), 0.75f, 1e-5f);
    EXPECT_TRUE(fade.IsBusy());
    fade.Tick();
    EXPECT_FLOAT_EQ(fade.GetAlpha(), 1.f);
    EXPECT_TRUE(fade.IsBlack());
    EXPECT_FALSE(fade.IsBusy());
    EXPECT_EQ(fade.GetAlpha255(), 255);
}

TEST(UiFade, FadeToClearFromBlack)
{
    FadeController fade;
    fade.SetBlack();
    EXPECT_TRUE(fade.IsBlack());
    fade.FadeToClear(2);
    fade.Tick();
    EXPECT_NEAR(fade.GetAlpha(), 0.5f, 1e-5f);
    fade.Tick();
    EXPECT_TRUE(fade.IsClear());
    EXPECT_EQ(fade.GetAlpha255(), 0);
}

TEST(UiFade, ZeroTicksOrAlreadyAtTargetCompletesImmediately)
{
    FadeController fade;
    fade.FadeToBlack(0);
    EXPECT_TRUE(fade.IsBlack());
    fade.FadeToBlack(30);   // 이미 검정
    EXPECT_FALSE(fade.IsBusy());
    fade.FadeToClear(0);
    EXPECT_TRUE(fade.IsClear());
}

TEST(UiFade, ReversingMidFadeStartsFromCurrentAlpha)
{
    FadeController fade;
    fade.FadeToBlack(10);
    for (int i = 0; i < 5; ++i)
    {
        fade.Tick();
    }
    ASSERT_NEAR(fade.GetAlpha(), 0.5f, 1e-5f);
    fade.FadeToClear(5);
    fade.Tick();
    EXPECT_NEAR(fade.GetAlpha(), 0.4f, 1e-5f);
    for (int i = 0; i < 4; ++i)
    {
        fade.Tick();
    }
    EXPECT_TRUE(fade.IsClear());
}

TEST(UiFade, IdleTicksDoNothing)
{
    FadeController fade;
    fade.SetBlack();
    for (int i = 0; i < 10; ++i)
    {
        fade.Tick();
    }
    EXPECT_TRUE(fade.IsBlack());
    EXPECT_FLOAT_EQ(fade.GetAlpha(), 1.f);
}

// ===========================================
//  HUD 모델 → 배치
// ===========================================

TEST(UiHud, PistolShowsInfiniteSymbolAndNoAmmoDigits)
{
    const UiLayout             layout;
    HudModel                   model = MakeModel();
    const jug::Vector<HudItem> items = BuildHudItems(model, layout);

    EXPECT_NE(FindItem(items, eHudItemKind::Sprite, "InfiniteUI_Red"), nullptr);
    EXPECT_EQ(FindItem(items, eHudItemKind::Text, "0"), nullptr);
    EXPECT_NE(FindItem(items, eHudItemKind::Sprite, "PlayerStateUI_Bomb"), nullptr);
}

TEST(UiHud, FiniteWeaponShowsAmmoDigitsRightAlignedInsteadOfInfinity)
{
    const UiLayout layout;
    HudModel       model = MakeModel();
    model.bInfiniteAmmo  = false;
    model.ammo           = 200;

    const jug::Vector<HudItem> items = BuildHudItems(model, layout);
    EXPECT_EQ(FindItem(items, eHudItemKind::Sprite, "InfiniteUI_Red"), nullptr);

    const HudItem* pAmmo = FindItem(items, eHudItemKind::Text, "200");
    ASSERT_NE(pAmmo, nullptr);
    EXPECT_EQ(pAmmo->style.align, eTextAlign::Right);
    EXPECT_EQ(pAmmo->x, layout.armsPanelX + layout.ammoRightDX);
    EXPECT_EQ(pAmmo->y, layout.armsPanelY + layout.digitsDY);
    EXPECT_EQ(pAmmo->style.color, eFontColor::RedYellow);
}

TEST(UiHud, BombScoreAndLivesTexts)
{
    const UiLayout             layout;
    const jug::Vector<HudItem> items = BuildHudItems(MakeModel(), layout);

    const HudItem* pBombs = FindItem(items, eHudItemKind::Text, "10");
    ASSERT_NE(pBombs, nullptr);
    EXPECT_EQ(pBombs->x, layout.armsPanelX + layout.bombRightDX);

    const HudItem* pScore = FindItem(items, eHudItemKind::Text, "12340");
    ASSERT_NE(pScore, nullptr);
    EXPECT_EQ(pScore->x, layout.scoreRightX);
    EXPECT_EQ(pScore->style.align, eTextAlign::Right);

    EXPECT_NE(FindItem(items, eHudItemKind::Text, "1UP X3"), nullptr);
}

TEST(UiHud, ValuesAreClampedToTheirDigitBudget)
{
    EXPECT_EQ(FormatScore(123'456'789), "9999999");
    EXPECT_EQ(FormatScore(-5), "0");
    EXPECT_EQ(FormatAmmo(12345), "999");
    EXPECT_EQ(FormatBombs(150), "99");
    EXPECT_EQ(FormatTimer(5), "05");
    EXPECT_EQ(FormatTimer(120), "99");
    EXPECT_EQ(FormatTimer(-3), "00");
}

TEST(UiHud, TimerOnlyWhenRequested)
{
    const UiLayout layout;
    HudModel       model = MakeModel();
    EXPECT_EQ(CountKind(BuildHudItems(model, layout), eHudItemKind::TimeDigits), 0);

    model.bShowTimer   = true;
    model.timerSeconds = 7;
    const jug::Vector<HudItem> items = BuildHudItems(model, layout);
    const HudItem*             pTimer = FindItem(items, eHudItemKind::TimeDigits, "07");
    ASSERT_NE(pTimer, nullptr);
    EXPECT_EQ(pTimer->x, layout.timerCenterX);
}

TEST(UiHud, PressStartBlinksWithConfiguredPeriod)
{
    const UiLayout layout;
    HudModel       model = MakeModel();

    model.uiTick = 0;
    EXPECT_NE(FindItem(BuildHudItems(model, layout), eHudItemKind::Sprite, "PressStartUI"), nullptr);
    model.uiTick = layout.pressStartBlinkTicks - 1;
    EXPECT_NE(FindItem(BuildHudItems(model, layout), eHudItemKind::Sprite, "PressStartUI"), nullptr);
    model.uiTick = layout.pressStartBlinkTicks;
    EXPECT_EQ(FindItem(BuildHudItems(model, layout), eHudItemKind::Sprite, "PressStartUI"), nullptr);
    model.uiTick = layout.pressStartBlinkTicks * 2;
    EXPECT_NE(FindItem(BuildHudItems(model, layout), eHudItemKind::Sprite, "PressStartUI"), nullptr);

    model.bShowPressStart2p = false;
    model.uiTick            = 0;
    EXPECT_EQ(FindItem(BuildHudItems(model, layout), eHudItemKind::Sprite, "PressStartUI"), nullptr);
}

TEST(UiHud, PlayerBlockCanBeHiddenAndLevelSpriteOptional)
{
    const UiLayout layout;
    HudModel       model = MakeModel();
    model.bShowPlayer       = false;
    model.bShowPressStart2p = false;
    EXPECT_TRUE(BuildHudItems(model, layout).empty());

    model.levelSprite = "LEVEL_1";
    const jug::Vector<HudItem> items = BuildHudItems(model, layout);
    ASSERT_EQ(items.size(), 1u);
    EXPECT_EQ(items[0].text, "LEVEL_1");
    EXPECT_EQ(items[0].x, layout.levelCenterX);
}

TEST(UiHud, BossBarFillScalesWithRatioAndFrameDrawsLast)
{
    const UiLayout layout;
    HudModel       model = MakeModel();
    model.bShowBoss      = true;

    model.bossHpRatio = 1.f;
    jug::Vector<HudItem> full = BuildHudItems(model, layout);
    model.bossHpRatio = 0.5f;
    jug::Vector<HudItem> half = BuildHudItems(model, layout);
    model.bossHpRatio = 0.f;
    jug::Vector<HudItem> empty = BuildHudItems(model, layout);

    // [..., 바탕 Rect, (채움 Rect), PlayerHPUI 틀]
    const auto rectsOf = [](const jug::Vector<HudItem>& _items) {
        jug::Vector<HudItem> rects;
        for (const HudItem& item: _items)
        {
            if (item.kind == eHudItemKind::Rect)
            {
                rects.push_back(item);
            }
        }
        return rects;
    };
    const jug::Vector<HudItem> fullRects  = rectsOf(full);
    const jug::Vector<HudItem> halfRects  = rectsOf(half);
    const jug::Vector<HudItem> emptyRects = rectsOf(empty);

    ASSERT_EQ(fullRects.size(), 2u);
    ASSERT_EQ(halfRects.size(), 2u);
    ASSERT_EQ(emptyRects.size(), 1u);   // 비어 있으면 채움 없음
    EXPECT_EQ(fullRects[1].w, layout.bossBarInnerW * layout.bossBarScale);
    EXPECT_NEAR(halfRects[1].w, fullRects[1].w / 2, 1);
    EXPECT_EQ(full.back().text, "PlayerHPUI");
    EXPECT_EQ(full.back().kind, eHudItemKind::Sprite);
    EXPECT_EQ(full.back().scale, layout.bossBarScale);
}

TEST(UiHud, ExtremeValuesStillFitInsideTheScreen)
{
    const UiLayout layout;
    HudModel       model;
    model.lives         = 99;
    model.score         = 9'999'999;
    model.bInfiniteAmmo = false;
    model.ammo          = 999;
    model.bombs         = 99;
    model.bShowTimer    = true;
    model.timerSeconds  = 99;
    model.bShowBoss     = true;
    model.bossHpRatio   = 1.f;
    model.levelSprite   = "LEVEL_1";
    model.uiTick        = 0;

    for (const HudItem& item: BuildHudItems(model, layout))
    {
        EXPECT_TRUE(ItemFits(item, layout)) << "kind " << static_cast<int>(item.kind) << " text '" << item.text << "' at " << item.x << "," << item.y;
    }
}

TEST(UiHud, TypicalModelFitsAndPanelDoesNotOverlapScoreBlock)
{
    const UiLayout layout;
    for (const HudItem& item: BuildHudItems(MakeModel(), layout))
    {
        EXPECT_TRUE(ItemFits(item, layout)) << item.text;
    }

    // 점수 7자리 왼쪽 끝은 목숨 줄과 같은 x 대에 있고, ARMS 패널은 점수 오른쪽 끝보다 오른쪽에서 시작한다.
    EXPECT_GE(layout.armsPanelX, layout.scoreRightX);
    const IntRect score = LayoutText("9999999", layout.scoreRightX, layout.scoreY, [] {
        TextStyle s;
        s.align = eTextAlign::Right;
        return s;
    }()).bounds;
    EXPECT_GE(score.x, 0);
}

TEST(UiHud, MakeHudModelReadsPlayerController)
{
    slugtest::Sim sim;
    const HudModel model = MakeHudModel(sim.Player());
    EXPECT_TRUE(model.bInfiniteAmmo);   // 시작 무기는 권총
    EXPECT_EQ(model.lives, 3);
    EXPECT_EQ(model.bombs, 10);
    EXPECT_EQ(model.score, 0);

    sim.Player().AddScore(1500);
    sim.Player().AddBombs(3);
    sim.Player().PickUpWeapon(eWeapon::Hmg, sim.Tune());
    const HudModel after = MakeHudModel(sim.Player());
    EXPECT_FALSE(after.bInfiniteAmmo);
    EXPECT_GT(after.ammo, 0);
    EXPECT_EQ(after.score, 1500);
    EXPECT_EQ(after.bombs, 13);
}

// ===========================================
//  UiSystem (헤드리스에서도 타이밍은 진행)
// ===========================================

TEST(UiSystem, HeadlessTicksAdvanceBannerFadeAndBlinkClock)
{
    GameContext ctx;   // 렌더러 없음
    UiSystem    ui;
    ui.Init(ctx);
    EXPECT_FALSE(ui.HasSprites());

    ui.ShowBanner("WARNING", 1.f);
    ui.GetFade().FadeToBlack(10);
    for (int i = 0; i < 10; ++i)
    {
        ui.Tick();
    }
    EXPECT_EQ(ui.GetTick(), 10);
    EXPECT_TRUE(ui.GetFade().IsBlack());
    EXPECT_TRUE(ui.GetBanner().IsActive());
    EXPECT_GT(ui.GetBanner().GetAlpha(), 0.f);

    // 그리기는 아무 일도 하지 않는다 (크래시 없음)
    ui.DrawHud(ctx, MakeModel());
    ui.DrawOverlays(ctx);
    EXPECT_FALSE(ui.DrawSprite(ctx, "PressStartUI", 0, 0));
}

// ===========================================
//  ScreenFlow: GAME OVER / MISSION COMPLETE
// ===========================================

TEST(UiFlow, GameOverTotalIsThreeSecondsAndEndsInDone)
{
    const ScreenFlowConfig config = MakeGameOverFlowConfig();
    EXPECT_EQ(config.GetTotalTicks(), 180);   // 3초 후 타이틀 (PlayerSpec)

    GameContext ctx;
    UiSystem    ui;
    ui.Init(ctx);
    TestAudio audio;
    ScreenFlow flow;

    flow.Start(config, ui, audio);
    // delay 0: Start 와 같은 틱에 배너 시작 + 징글
    EXPECT_EQ(flow.GetPhase(), eFlowPhase::Banner);
    EXPECT_TRUE(ui.GetBanner().IsActive());
    EXPECT_EQ(ui.GetBanner().GetCurrent().text, "GAME OVER");
    EXPECT_EQ(audio.CountPlays(sfx::kGameOver), 1);
    EXPECT_EQ(audio.CountKind(eAudioEvent::FadeOutBgm), 1);

    int doneAt = -1;
    for (int tick = 1; tick <= 400 && doneAt < 0; ++tick)
    {
        ui.Tick();
        flow.Tick(ui, audio);
        if (flow.IsDone())
        {
            doneAt = tick;
        }
    }
    EXPECT_EQ(doneAt, 180);
    EXPECT_TRUE(ui.GetFade().IsBlack());
    EXPECT_FALSE(ui.GetBanner().IsActive());
    EXPECT_FALSE(flow.IsRunning());
}

TEST(UiFlow, PhasesChangeAtExactTicks)
{
    ScreenFlowConfig config;
    config.bannerText = "TEST";
    config.delayTicks = 5;
    config.bannerTicks = 10;
    config.fadeTicks = 4;
    config.holdTicks = 3;
    config.sfx = "Jingle";

    GameContext ctx;
    UiSystem    ui;
    ui.Init(ctx);
    TestAudio  audio;
    ScreenFlow flow;
    flow.Start(config, ui, audio);

    EXPECT_EQ(flow.GetPhase(), eFlowPhase::Delay);
    EXPECT_FALSE(ui.GetBanner().IsActive());
    EXPECT_EQ(audio.CountPlays("Jingle"), 0);

    const struct { int tick; eFlowPhase phase; } expected[] = {
        { 4, eFlowPhase::Delay },
        { 5, eFlowPhase::Banner },
        { 14, eFlowPhase::Banner },
        { 15, eFlowPhase::FadeOut },
        { 18, eFlowPhase::FadeOut },
        { 19, eFlowPhase::Hold },
        { 21, eFlowPhase::Hold },
        { 22, eFlowPhase::Done },
    };
    int t = 0;
    for (const auto& step: expected)
    {
        while (t < step.tick)
        {
            ++t;
            ui.Tick();
            flow.Tick(ui, audio);
        }
        EXPECT_EQ(flow.GetPhase(), step.phase) << "tick " << step.tick;
    }
    EXPECT_EQ(audio.CountPlays("Jingle"), 1);   // 배너 시작 때 한 번만
    EXPECT_TRUE(ui.GetFade().IsBlack());
}

TEST(UiFlow, MissionCompleteWaitsForVictoryPoseThenBanner)
{
    const ScreenFlowConfig config = MakeMissionCompleteFlowConfig();
    EXPECT_EQ(config.bannerText, "MISSION COMPLETE");

    GameContext ctx;
    UiSystem    ui;
    ui.Init(ctx);
    TestAudio  audio;
    ScreenFlow flow;
    flow.Start(config, ui, audio);

    EXPECT_EQ(flow.GetPhase(), eFlowPhase::Delay);
    EXPECT_EQ(audio.CountKind(eAudioEvent::StopBgm), 1);   // bgmFadeSeconds == 0 → 즉시 정지
    for (int tick = 1; tick < config.delayTicks; ++tick)
    {
        ui.Tick();
        flow.Tick(ui, audio);
    }
    EXPECT_FALSE(ui.GetBanner().IsActive());
    ui.Tick();
    flow.Tick(ui, audio);
    EXPECT_TRUE(ui.GetBanner().IsActive());
    EXPECT_EQ(audio.CountPlays(sfx::kMissionComplete), 1);
}

TEST(UiFlow, TickBeforeStartAndAfterDoneIsHarmless)
{
    GameContext ctx;
    UiSystem    ui;
    ui.Init(ctx);
    TestAudio  audio;
    ScreenFlow flow;
    flow.Tick(ui, audio);   // Start 전
    EXPECT_EQ(flow.GetPhase(), eFlowPhase::Idle);
    EXPECT_FALSE(flow.IsRunning());

    ScreenFlowConfig config;
    config.bannerTicks = 1;
    config.fadeTicks   = 1;
    config.holdTicks   = 0;
    flow.Start(config, ui, audio);
    for (int i = 0; i < 10; ++i)
    {
        flow.Tick(ui, audio);
    }
    EXPECT_TRUE(flow.IsDone());
    const int elapsed = flow.GetElapsedTicks();
    flow.Tick(ui, audio);
    EXPECT_EQ(flow.GetElapsedTicks(), elapsed);   // Done 이후에는 더 세지 않는다
}

// ===========================================
//  TitleState (SimRunner 로 구동)
// ===========================================

namespace
{
    struct TitleSim
    {
        explicit TitleSim(TitleConfig _config = {})
            : rng(uint64_t { 1 })
        {
            ctx.pRng   = &rng;
            ctx.pAudio = &audio;
            pTitle     = new TitleState { std::move(_config) };
            runner     = jug::Scoped<SimRunner> { new SimRunner { ctx, jug::Scoped<IGameState> { pTitle } } };
        }

        void Step(const eAction _action, const bool _bHeld)
        {
            InputFrame frame;
            frame.Set(_action, _bHeld);
            runner->Step(frame);
        }

        void Idle(const int _ticks)
        {
            for (int i = 0; i < _ticks; ++i)
            {
                runner->Step({});
            }
        }

        [[nodiscard]] bool IsStillTitle() const { return runner->GetState() == pTitle; }

        jug::RngMt19937        rng;
        GameContext            ctx;
        TestAudio              audio;
        TitleState*            pTitle = nullptr;
        jug::Scoped<SimRunner> runner = nullptr;
    };

    [[nodiscard]] TitleConfig SandboxTitleConfig()
    {
        TitleConfig config;
        config.nextState = "sandbox";
        return config;
    }
}   // namespace

TEST(UiTitle, StaysOnTitleWithoutInput)
{
    TitleSim sim { SandboxTitleConfig() };
    sim.Idle(300);
    EXPECT_TRUE(sim.IsStillTitle());
    EXPECT_FALSE(sim.pTitle->IsStarting());
}

TEST(UiTitle, ShotPressStartsFadeThenSwitchesStateExactlyAfterFade)
{
    TitleConfig config = SandboxTitleConfig();
    config.startFadeTicks = 24;
    TitleSim sim { config };
    sim.Idle(30);

    sim.Step(eAction::Shot, true);   // 눌림 틱 T
    EXPECT_TRUE(sim.pTitle->IsStarting());
    EXPECT_EQ(sim.pTitle->GetSelectedStateName(), "sandbox");
    EXPECT_EQ(sim.audio.CountPlays(sfx::kOk), 1);
    sim.Step(eAction::Shot, false);

    int switchedAt = -1;
    for (int tick = 2; tick <= 60 && switchedAt < 0; ++tick)   // T + 1 이 tick 1 이었음
    {
        sim.Idle(1);
        if (!sim.IsStillTitle())
        {
            switchedAt = tick;
        }
    }
    EXPECT_EQ(switchedAt, 24);   // T + startFadeTicks
    EXPECT_FALSE(sim.IsStillTitle());
    EXPECT_NE(sim.runner->GetState(), nullptr);
}

TEST(UiTitle, StartActionAlsoWorksAndHeldKeyDoesNotRetrigger)
{
    TitleSim sim { SandboxTitleConfig() };
    sim.Idle(20);
    sim.Step(eAction::Start, true);
    EXPECT_TRUE(sim.pTitle->IsStarting());
    sim.Step(eAction::Start, true);   // 계속 누름 → 효과음 한 번만
    sim.Step(eAction::Start, true);
    EXPECT_EQ(sim.audio.CountPlays(sfx::kOk), 1);
}

TEST(UiTitle, InputsDuringEntryDelayAreIgnored)
{
    TitleConfig config = SandboxTitleConfig();
    config.inputDelayTicks = 12;
    TitleSim sim { config };
    sim.Step(eAction::Shot, true);   // tick 1
    sim.Step(eAction::Shot, false);
    sim.Idle(5);
    EXPECT_FALSE(sim.pTitle->IsStarting());

    sim.Idle(10);   // 지연 이후
    sim.Step(eAction::Shot, true);
    EXPECT_TRUE(sim.pTitle->IsStarting());
}

TEST(UiTitle, MovementKeysDoNotStartTheGame)
{
    TitleSim sim { SandboxTitleConfig() };
    sim.Idle(20);
    sim.Step(eAction::Right, true);
    sim.Step(eAction::Jump, true);
    sim.Step(eAction::Bomb, true);
    sim.Idle(5);
    EXPECT_FALSE(sim.pTitle->IsStarting());
}

TEST(UiTitle, AutoSelectionFallsBackToSandboxWhenStage0IsNotRegistered)
{
    TitleSim sim;   // nextState 비어 있음 → stage0 가 있으면 stage0, 없으면 sandbox
    sim.Idle(20);
    sim.Step(eAction::Start, true);
    const jug::String& selected = sim.pTitle->GetSelectedStateName();
    EXPECT_TRUE(selected == "stage0" || selected == "sandbox") << selected;
}

TEST(UiTitle, EntersWithBlackAndFadesIn)
{
    TitleSim sim { SandboxTitleConfig() };
    EXPECT_GT(sim.pTitle->GetUi().GetFade().GetAlpha(), 0.9f);   // 검정에서 시작
    sim.Idle(sim.pTitle->GetUi().GetFade().IsBusy() ? 40 : 0);
    EXPECT_TRUE(sim.pTitle->GetUi().GetFade().IsClear());
    EXPECT_EQ(sim.audio.CountPlays(sfx::kTitleVoice), 1);
}

TEST(UiTitle, IsRegisteredInStateFactory)
{
    EXPECT_NE(CreateState("title"), nullptr);
}

TEST(UiTitle, TextLayoutStacksInsideScreen)
{
    const UiLayout layout;
    EXPECT_LT(layout.titleNameY + 32, layout.titlePressStartY);
    EXPECT_LT(layout.titlePressStartY + 16, layout.titleHintY);
    EXPECT_LE(layout.titleHintY + 16, 448);
}
