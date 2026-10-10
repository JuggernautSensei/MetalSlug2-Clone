// UI 테스트: 배너, 폰트, 페이드, HUD, 화면 흐름, 타이틀, 결과 화면 검증
#include <filesystem>
#include <map>

#include <gtest/gtest.h>

#include "Audio/AudioSink.h"
#include "Base/Math.h"
#include "Base/Random.h"
#include "Base/Result.h"
#include "Base/Types.h"
#include "Core/AssetPath.h"
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
#include "TestAudio.h"

using namespace slug;

namespace
{
[[nodiscard]] const HudItem* FindItem_(
    const slug::Vector<HudItem>& _items,
    const eHudItemKind           _kind,
    const slug::StringView       _text)
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

[[nodiscard]] int CountKind_(
    const slug::Vector<HudItem>& _items,
    const eHudItemKind           _kind)
{
    int count = 0;
    for (const HudItem& item: _items)
    {
        count += item.kind == _kind ? 1 : 0;
    }
    return count;
}

[[nodiscard]] Vec2 SpriteSize_(
    const slug::StringView _name)
{
    static const std::map<slug::String, Vec2> kSizes = {
        { "PlayerStateUI_Bomb", { 124.f, 34.f } },
        {     "InfiniteUI_Red",  { 40.f, 16.f } },
        {       "PressStartUI", { 174.f, 30.f } },
        {         "PlayerHPUI", { 106.f, 16.f } },
        {            "LEVEL_1",  { 96.f, 16.f } },
    };
    const std::map<slug::String, Vec2>::const_iterator it = kSizes.find(slug::String { _name });
    return it == kSizes.end() ? Vec2 { 0.f, 0.f } : it->second;
}

[[nodiscard]] bool InsideScreen_(
    const IntRect&  _r,
    const UiLayout& _layout)
{
    return _r.x >= 0 && _r.y >= 0 && _r.x + _r.w <= _layout.screenW && _r.y + _r.h <= _layout.screenH;
}

[[nodiscard]] bool ItemFits_(
    const HudItem&  _item,
    const UiLayout& _layout)
{
    switch (_item.kind)
    {
        case eHudItemKind::Text: return InsideScreen_(LayoutText(_item.text, _item.x, _item.y, _item.style).bounds, _layout);
        case eHudItemKind::Sprite:
        {
            const Vec2 size = SpriteSize_(_item.text);
            const int  w    = static_cast<int>(size.x) * _item.scale;
            const int  h    = static_cast<int>(size.y) * _item.scale;
            return InsideScreen_({ _item.x - w / 2, _item.y - h / 2, w, h }, _layout);
        }
        case eHudItemKind::TimeDigits:
        {
            const int w = static_cast<int>(_item.text.size()) * _layout.timeGlyphW;
            return InsideScreen_({ _item.x - w / 2, _item.y, w, _layout.timeGlyphH }, _layout);
        }
        case eHudItemKind::Rect: return InsideScreen_({ _item.x, _item.y, _item.w, _item.h }, _layout);
    }
    return false;
}

[[nodiscard]] HudModel MakeModel_()
{
    HudModel model;
    model.lives = 3;
    model.score = 12'340;
    model.bombs = 10;
    return model;
}

// Banner/Results 등 단계 흐름을 _tick 에 닿을 때까지 한 틱씩 진행한다. _t 는 지금까지 진행한 틱 수
void AdvanceFlowTo_(
    int&        _t,
    UiSystem&   _ui,
    ScreenFlow& _flow,
    TestAudio&  _audio,
    const int   _tick)
{
    while (_t < _tick)
    {
        ++_t;
        _ui.Tick();
        _flow.Tick(_ui, _audio);
    }
}

// HUD 항목 중 Rect 종류만 골라낸다
[[nodiscard]] slug::Vector<HudItem> HudRects_(
    const slug::Vector<HudItem>& _items)
{
    slug::Vector<HudItem> rects;
    for (const HudItem& item: _items)
    {
        if (item.kind == eHudItemKind::Rect)
        {
            rects.push_back(item);
        }
    }
    return rects;
}
}   // namespace

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
    for (const std::pair<const int, int>& hit: hits)
    {
        EXPECT_GE(hit.first, 0);
        EXPECT_LT(hit.first, fontmap::kGlyphColumns);
        EXPECT_EQ(hit.second, 1);
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
    EXPECT_EQ(FitTextScale("MISSION COMPLETE", 576, 3), 2);
    EXPECT_EQ(FitTextScale("GAME OVER", 576, 3), 3);                                        // 9자 x 48 = 432
    EXPECT_EQ(FitTextScale("GAME OVER", 576, 2), 2);                                        // 상한
    EXPECT_EQ(FitTextScale("A VERY VERY VERY LONG LINE OF BANNER TEXT HERE", 576, 3), 1);   // 안 들어가도 1
    EXPECT_EQ(FitTextScale("SHORT\nA MUCH LONGER SECOND LINE", 640, 3), 1);                 // 가장 긴 줄 기준
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

TEST(UiLayoutText, LeftAlignStartsAtAnchor)
{
    TextStyle        style;
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
    style.align             = eTextAlign::Right;
    const TextLayout layout = LayoutText("1234", 148, 8, style);
    ASSERT_EQ(layout.quads.size(), 4u);
    EXPECT_EQ(layout.quads.back().dst.x + layout.quads.back().dst.w, 148);
    EXPECT_EQ(layout.quads.front().dst.x, 148 - 64);
    EXPECT_EQ(layout.bounds.x + layout.bounds.w, 148);
}

TEST(UiLayoutText, CenterAlignSplitsWidthAroundAnchor)
{
    TextStyle style;
    style.align             = eTextAlign::Center;
    const TextLayout layout = LayoutText("ABCD", 320, 0, style);   // 폭 64
    EXPECT_EQ(layout.quads.front().dst.x, 320 - 32);
    EXPECT_EQ(layout.quads.back().dst.x + 16, 320 + 32);
}

TEST(UiLayoutText, MultiLineCentersEachLineAndStacksWithGap)
{
    TextStyle style;
    style.align             = eTextAlign::Center;
    style.scale             = 2;
    style.lineGap           = 4;
    const TextLayout layout = LayoutText("AB\nABCD", 100, 50, style);
    ASSERT_EQ(layout.lines, 2);
    ASSERT_EQ(layout.quads.size(), 6u);
    EXPECT_EQ(layout.quads[0].dst.x, 68);
    EXPECT_EQ(layout.quads[2].dst.x, 36);
    EXPECT_EQ(layout.quads[0].dst.y, 50);
    EXPECT_EQ(layout.quads[2].dst.y, 50 + 32 + 4 * 2);
    EXPECT_EQ(layout.height, 32 + 8 + 32);
}

TEST(UiLayoutText, FlagsOnlyNonSpaceUnsupportedCharsForFallback)
{
    TextStyle        style;
    const TextLayout layout = LayoutText("A-B C", 0, 0, style);
    ASSERT_EQ(layout.quads.size(), 5u);
    EXPECT_FALSE(layout.quads[0].bFallback);
    EXPECT_TRUE(layout.quads[1].bFallback);   // '-'
    EXPECT_EQ(layout.quads[1].glyph, -1);
    EXPECT_FALSE(layout.quads[3].bFallback);   // ' ' 는 칸만 차지
    EXPECT_EQ(layout.quads[4].dst.x, 4 * 16);
}

TEST(UiLayoutText, LetterSpacingAddsBetweenCharsOnly)
{
    TextStyle style;
    style.letterSpacing     = 2;
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

TEST(UiBitmapFont, GlyphFrameCutsTheStripIntoSixteenPixelCells)
{
    SpriteLibrary library;
    SpriteInfo    strip;
    strip.name   = "Font_White";
    strip.frame  = { 3, 100, 200, 560, 32, 280, 16 };
    strip.width  = 560;
    strip.height = 32;
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
    slug::Result<SpriteLibrary> library = SpriteLibrary::LoadFromDirectory(slug::FindAssetRoot() / "Baked");
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
        EXPECT_EQ(static_cast<float>(pInfo->frame.w), SpriteSize_(name).x) << name;
        EXPECT_EQ(static_cast<float>(pInfo->frame.h), SpriteSize_(name).y) << name;
    }

    BitmapFont font;
    EXPECT_TRUE(font.Init(*library));
    const AnimFrame nine = font.GetTimeDigitFrame(9);
    EXPECT_EQ(nine.x, pTime->frame.x + 9 * 32);
    EXPECT_EQ(nine.w, 32);
}

TEST(UiBanner, AlphaCurveAtExactTicks)
{
    const ARRAY<float, 11> expected = { 0.f, 1.f / 3.f, 2.f / 3.f, 1.f, 1.f, 1.f, 1.f, 1.f, 2.f / 3.f, 1.f / 3.f, 0.f };
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
    banner.Tick();
    EXPECT_LT(banner.GetAlpha(), 1.f);
    banner.Tick();
    EXPECT_TRUE(banner.IsActive());
    banner.Tick();
    EXPECT_FALSE(banner.IsActive());
    EXPECT_FLOAT_EQ(banner.GetAlpha(), 0.f);
}

TEST(UiBanner, QueueRunsSpecsBackToBackAndShowReplaces)
{
    BannerSpec a;
    a.text         = "ONE";
    a.fadeInTicks  = 1;
    a.holdTicks    = 1;
    a.fadeOutTicks = 1;
    BannerSpec b   = a;
    b.text         = "TWO";

    BannerSystem banner;
    banner.Enqueue(a);
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

    banner.Show(a);
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
        style.align             = eTextAlign::Center;
        style.lineGap           = layout.bannerLineGap;
        style.scale             = FitTextScale(text, layout.screenW - 2 * layout.bannerMargin, layout.bannerMaxScale);
        const int        height = MeasureTextHeight(text, style);
        const TextLayout laid   = LayoutText(text, layout.screenW / 2, layout.bannerCenterY - height / 2, style);
        EXPECT_TRUE(InsideScreen_(laid.bounds, layout)) << text;
        EXPECT_GE(style.scale, 1) << text;
    }
}

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

TEST(UiHud, PistolShowsInfiniteSymbolAndNoAmmoDigits)
{
    const UiLayout              layout;
    const HudModel              model = MakeModel_();
    const slug::Vector<HudItem> items = BuildHudItems(model, layout);

    EXPECT_NE(FindItem_(items, eHudItemKind::Sprite, "InfiniteUI_Red"), nullptr);
    EXPECT_EQ(FindItem_(items, eHudItemKind::Text, "0"), nullptr);
    EXPECT_NE(FindItem_(items, eHudItemKind::Sprite, "PlayerStateUI_Bomb"), nullptr);
}

TEST(UiHud, FiniteWeaponShowsAmmoDigitsRightAlignedInsteadOfInfinity)
{
    const UiLayout layout;
    HudModel       model = MakeModel_();
    model.bInfiniteAmmo  = false;
    model.ammo           = 200;

    const slug::Vector<HudItem> items = BuildHudItems(model, layout);
    EXPECT_EQ(FindItem_(items, eHudItemKind::Sprite, "InfiniteUI_Red"), nullptr);

    const HudItem* pAmmo = FindItem_(items, eHudItemKind::Text, "200");
    ASSERT_NE(pAmmo, nullptr);
    EXPECT_EQ(pAmmo->style.align, eTextAlign::Right);
    EXPECT_EQ(pAmmo->x, layout.armsPanelX + layout.ammoRightDX);
    EXPECT_EQ(pAmmo->y, layout.armsPanelY + layout.digitsDY);
    EXPECT_EQ(pAmmo->style.color, eFontColor::RedYellow);
}

TEST(UiHud, BombScoreAndLivesTexts)
{
    const UiLayout              layout;
    const slug::Vector<HudItem> items = BuildHudItems(MakeModel_(), layout);

    const HudItem* pBombs = FindItem_(items, eHudItemKind::Text, "10");
    ASSERT_NE(pBombs, nullptr);
    EXPECT_EQ(pBombs->x, layout.armsPanelX + layout.bombRightDX);

    const HudItem* pScore = FindItem_(items, eHudItemKind::Text, "12340");
    ASSERT_NE(pScore, nullptr);
    EXPECT_EQ(pScore->x, layout.scoreRightX);
    EXPECT_EQ(pScore->style.align, eTextAlign::Right);

    EXPECT_NE(FindItem_(items, eHudItemKind::Text, "1UP=3"), nullptr);
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
    HudModel       model = MakeModel_();
    EXPECT_EQ(CountKind_(BuildHudItems(model, layout), eHudItemKind::TimeDigits), 0);

    model.bShowTimer                   = true;
    model.timerSeconds                 = 7;
    const slug::Vector<HudItem> items  = BuildHudItems(model, layout);
    const HudItem*              pTimer = FindItem_(items, eHudItemKind::TimeDigits, "07");
    ASSERT_NE(pTimer, nullptr);
    EXPECT_EQ(pTimer->x, layout.timerCenterX);
}

TEST(UiHud, InsertCoinIsStaticTopRightTextThatNeverBlinks)
{
    const UiLayout layout;
    HudModel       model = MakeModel_();

    for (const int tick: { 0, layout.pressStartBlinkTicks, layout.pressStartBlinkTicks * 3 })
    {
        model.uiTick                      = tick;
        const slug::Vector<HudItem> items = BuildHudItems(model, layout);
        const HudItem*              pCoin = FindItem_(items, eHudItemKind::Text, "INSERT COIN");
        ASSERT_NE(pCoin, nullptr) << tick;
        EXPECT_EQ(pCoin->x, layout.insertCoinRightX);
        EXPECT_EQ(pCoin->y, layout.insertCoinY);
        EXPECT_EQ(pCoin->style.align, eTextAlign::Right);
    }
    EXPECT_EQ(FindItem_(BuildHudItems(model, layout), eHudItemKind::Sprite, "PressStartUI"), nullptr);

    model.bShowInsertCoin = false;
    EXPECT_EQ(FindItem_(BuildHudItems(model, layout), eHudItemKind::Text, "INSERT COIN"), nullptr);
}

TEST(UiHud, DisplayTimerCountsDownSlowlyAndNeverGoesNegative)
{
    DisplayTimer timer;
    EXPECT_EQ(timer.GetSeconds(), 60);

    for (int i = 0; i < DisplayTimer::kTicksPerDigit; ++i)
    {
        timer.Tick();
    }
    EXPECT_EQ(timer.GetSeconds(), 59);   // 6.5초에 한 칸

    for (int i = 0; i < DisplayTimer::kStartSeconds * DisplayTimer::kTicksPerDigit + 100; ++i)
    {
        timer.Tick();
    }
    EXPECT_EQ(timer.GetSeconds(), 0);
    EXPECT_EQ(timer.GetRemainingTicks(), 0);

    timer.Reset();
    EXPECT_EQ(timer.GetSeconds(), 60);
}

TEST(UiHud, PlayerBlockCanBeHiddenAndLevelSpriteOptional)
{
    const UiLayout layout;
    HudModel       model  = MakeModel_();
    model.bShowPlayer     = false;
    model.bShowInsertCoin = false;
    EXPECT_TRUE(BuildHudItems(model, layout).empty());

    model.levelSprite                 = "LEVEL_1";
    const slug::Vector<HudItem> items = BuildHudItems(model, layout);
    ASSERT_EQ(items.size(), 1u);
    EXPECT_EQ(items[0].text, "LEVEL_1");
    EXPECT_EQ(items[0].x, layout.levelCenterX);
}

TEST(UiHud, BossBarFillScalesWithRatioAndFrameDrawsLast)
{
    const UiLayout layout;
    HudModel       model = MakeModel_();
    model.bShowBoss      = true;

    model.bossHpRatio                 = 1.f;
    const slug::Vector<HudItem> full  = BuildHudItems(model, layout);
    model.bossHpRatio                 = 0.5f;
    const slug::Vector<HudItem> half  = BuildHudItems(model, layout);
    model.bossHpRatio                 = 0.f;
    const slug::Vector<HudItem> empty = BuildHudItems(model, layout);

    const slug::Vector<HudItem> fullRects  = HudRects_(full);
    const slug::Vector<HudItem> halfRects  = HudRects_(half);
    const slug::Vector<HudItem> emptyRects = HudRects_(empty);

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
        EXPECT_TRUE(ItemFits_(item, layout)) << "kind " << static_cast<int>(item.kind) << " text '" << item.text << "' at " << item.x << "," << item.y;
    }
}

TEST(UiHud, TypicalModelFitsAndPanelDoesNotOverlapScoreBlock)
{
    const UiLayout layout;
    for (const HudItem& item: BuildHudItems(MakeModel_(), layout))
    {
        EXPECT_TRUE(ItemFits_(item, layout)) << item.text;
    }

    EXPECT_GE(layout.armsPanelX, layout.scoreRightX);
    TextStyle rightStyle;
    rightStyle.align    = eTextAlign::Right;
    const IntRect score = LayoutText("9999999", layout.scoreRightX, layout.scoreY, rightStyle).bounds;
    EXPECT_GE(score.x, 0);
}

TEST(UiHud, MakeHudModelReadsPlayerController)
{
    slugtest::Sim  sim;
    const HudModel model = MakeHudModel(sim.Player());
    EXPECT_TRUE(model.bInfiniteAmmo);
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

TEST(UiSystem, HeadlessTicksAdvanceBannerFadeAndBlinkClock)
{
    GameContext ctx;
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

    ui.DrawHud(ctx, MakeModel_());
    ui.DrawOverlays(ctx);
    EXPECT_FALSE(ui.DrawSprite(ctx, "PressStartUI", 0, 0));
}

TEST(UiFlow, GameOverTotalIsThreeSecondsAndEndsInDone)
{
    const ScreenFlowConfig config = MakeGameOverFlowConfig();
    EXPECT_EQ(config.GetTotalTicks(), 270);   // 페이드 인 0.5 s + 홀드 3 s + 페이드 아웃 0.67 s + 대기 0.33 s 후 타이틀
    EXPECT_EQ(config.artSprite, "UI_GameOver_Helmet");

    GameContext ctx;
    UiSystem    ui;
    ui.Init(ctx);
    TestAudio  audio;
    ScreenFlow flow;

    flow.Start(config, ui, audio);
    EXPECT_EQ(flow.GetPhase(), eFlowPhase::Art);
    EXPECT_FALSE(ui.GetBanner().IsActive());   // 배너 대신 전면 아트
    EXPECT_TRUE(flow.GetArtView().bVisible);
    EXPECT_EQ(flow.GetArtView().alpha, 0);   // 페이드 인 시작
    EXPECT_EQ(audio.CountPlays(sfx::kGameOver), 1);
    EXPECT_EQ(audio.CountKind(eAudioEvent::FadeOutBgm), 1);

    int doneAt = -1;
    int fullAt = -1;
    for (int tick = 1; tick <= 400 && doneAt < 0; ++tick)
    {
        ui.Tick();
        flow.Tick(ui, audio);
        if (fullAt < 0 && flow.GetArtView().alpha == 255)
        {
            fullAt = tick;
        }
        if (flow.IsDone())
        {
            doneAt = tick;
        }
    }
    EXPECT_EQ(fullAt, 30);
    EXPECT_EQ(doneAt, 270);
    EXPECT_TRUE(ui.GetFade().IsBlack());
    EXPECT_FALSE(ui.GetBanner().IsActive());
    EXPECT_FALSE(flow.IsRunning());
    EXPECT_FALSE(flow.GetArtView().bVisible);   // 끝난 뒤에는 아트를 그리지 않는다
}

TEST(UiFlow, PhasesChangeAtExactTicks)
{
    ScreenFlowConfig config;
    config.bannerText  = "TEST";
    config.delayTicks  = 5;
    config.bannerTicks = 10;
    config.fadeTicks   = 4;
    config.holdTicks   = 3;
    config.sfx         = "Jingle";

    GameContext ctx;
    UiSystem    ui;
    ui.Init(ctx);
    TestAudio  audio;
    ScreenFlow flow;
    flow.Start(config, ui, audio);

    EXPECT_EQ(flow.GetPhase(), eFlowPhase::Delay);
    EXPECT_FALSE(ui.GetBanner().IsActive());
    EXPECT_EQ(audio.CountPlays("Jingle"), 0);

    struct Step
    {
        int        tick  = 0;
        eFlowPhase phase = eFlowPhase::Delay;
    };
    const ARRAY<Step, 8> expected = {
        {
         { 4, eFlowPhase::Delay },
         { 5, eFlowPhase::Banner },
         { 14, eFlowPhase::Banner },
         { 15, eFlowPhase::FadeOut },
         { 18, eFlowPhase::FadeOut },
         { 19, eFlowPhase::Hold },
         { 21, eFlowPhase::Hold },
         { 22, eFlowPhase::Done },
         }
    };
    int t = 0;
    for (const Step& step: expected)
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

TEST(UiFlow, MissionCompleteShowsResultsBeforeTheCompleteBanner)
{
    const ScreenFlowConfig config = MakeMissionCompleteFlowConfig(16);
    EXPECT_EQ(config.bannerText, "MISSION 4\nCOMPLETE!");
    EXPECT_EQ(config.resultsTicks, MissionResultsTicks(16));
    EXPECT_EQ(config.GetTotalTicks(), 60 + 276 + 210 + 90 + 30);

    GameContext ctx;
    UiSystem    ui;
    ui.Init(ctx);
    TestAudio  audio;
    ScreenFlow flow;
    flow.Start(config, ui, audio);

    EXPECT_EQ(flow.GetPhase(), eFlowPhase::Delay);
    EXPECT_EQ(audio.CountKind(eAudioEvent::StopBgm), 1);   // bgmFadeSeconds == 0 → 즉시 정지
    EXPECT_FALSE(flow.GetResultsView().bBoxVisible);

    int t = 0;

    AdvanceFlowTo_(t, ui, flow, audio, 59);
    EXPECT_EQ(flow.GetPhase(), eFlowPhase::Delay);
    EXPECT_FALSE(ui.GetBanner().IsActive());

    AdvanceFlowTo_(t, ui, flow, audio, 60);
    EXPECT_EQ(flow.GetPhase(), eFlowPhase::Results);
    EXPECT_FALSE(ui.GetBanner().IsActive());
    EXPECT_TRUE(flow.GetResultsView().bBoxVisible);
    EXPECT_EQ(flow.GetResultsView().rescued, 0);

    AdvanceFlowTo_(t, ui, flow, audio, 60 + 12);
    EXPECT_EQ(flow.GetResultsView().rescued, 1);

    AdvanceFlowTo_(t, ui, flow, audio, 60 + 276 - 1);
    EXPECT_EQ(flow.GetPhase(), eFlowPhase::Results);
    EXPECT_TRUE(flow.GetResultsView().bBoxVisible);   // 결과 단계 끝 직전까지 상자 표시

    AdvanceFlowTo_(t, ui, flow, audio, 60 + 276);
    EXPECT_EQ(flow.GetPhase(), eFlowPhase::Banner);
    EXPECT_FALSE(flow.GetResultsView().bBoxVisible);
    EXPECT_TRUE(ui.GetBanner().IsActive());
    EXPECT_EQ(ui.GetBanner().GetCurrent().text, "MISSION 4\nCOMPLETE!");
    EXPECT_EQ(audio.CountPlays(sfx::kMissionComplete), 1);

    AdvanceFlowTo_(t, ui, flow, audio, config.GetTotalTicks());
    EXPECT_TRUE(flow.IsDone());
}

TEST(UiFlow, MissionCompleteWithoutPowsStillShowsAnEmptyBox)
{
    const ScreenFlowConfig config = MakeMissionCompleteFlowConfig(0);
    EXPECT_EQ(config.rescuedCount, 0);
    EXPECT_EQ(config.resultsTicks, MissionResultsTicks(0));

    const MissionResultsView view = MakeMissionResultsView(0, MissionResultsTicks(0) - 1);
    EXPECT_TRUE(view.bBoxVisible);
    EXPECT_EQ(view.rescued, 0);
    EXPECT_EQ(view.bonusScore, 0);
    EXPECT_FALSE(view.bRankVisible);
}

TEST(UiResults, RescueTallyCountsUpAndReachesGreatWithSixteen)
{
    EXPECT_EQ(MissionBonusScore(16), 380000);   // 영상: 16명, 최종 380000 (GREAT!)
    EXPECT_EQ(MissionRankText(16), "GREAT!");
    EXPECT_TRUE(MissionRankText(3).empty());
    EXPECT_EQ(MissionBonusScore(3), 30000);

    const int countEnd = 12 + 16 * 6;
    EXPECT_EQ(MakeMissionResultsView(16, 12).rescued, 1);
    EXPECT_EQ(MakeMissionResultsView(16, 12 + 6 * 7).rescued, 8);
    const MissionResultsView last = MakeMissionResultsView(16, countEnd - 1);
    EXPECT_EQ(last.rescued, 16);
    EXPECT_TRUE(last.bCounting);
    EXPECT_EQ(last.bonusScore, 16 * 10000);
    EXPECT_FALSE(last.bRankVisible);

    const MissionResultsView rank = MakeMissionResultsView(16, countEnd + 18);
    EXPECT_FALSE(rank.bCounting);
    EXPECT_EQ(rank.bonusScore, 380000);
    EXPECT_TRUE(rank.bRankVisible);
    EXPECT_EQ(rank.rankText, "GREAT!");

    EXPECT_FALSE(MakeMissionResultsView(16, MissionResultsTicks(16)).bBoxVisible);
    EXPECT_FALSE(MakeMissionResultsView(16, -1).bBoxVisible);
}

TEST(UiResults, ScreenFlowExposesResultsViewOnlyDuringResultsPhase)
{
    GameContext ctx;
    UiSystem    ui;
    ui.Init(ctx);
    TestAudio  audio;
    ScreenFlow flow;
    flow.Start(MakeMissionCompleteFlowConfig(5), ui, audio);
    EXPECT_FALSE(flow.GetResultsView().bBoxVisible);   // 지연 단계
    for (int i = 0; i < 60; ++i)
    {
        ui.Tick();
        flow.Tick(ui, audio);
    }
    EXPECT_EQ(flow.GetPhase(), eFlowPhase::Results);
    EXPECT_TRUE(flow.GetResultsView().bBoxVisible);
    EXPECT_EQ(flow.GetResultsView().rescued, 0);
    for (int i = 0; i < 12; ++i)
    {
        ui.Tick();
        flow.Tick(ui, audio);
    }
    EXPECT_EQ(flow.GetResultsView().rescued, 1);
}

TEST(UiBanner, MissionGlyphNamesMapLettersDigitsAndPunctuation)
{
    EXPECT_EQ(MakeMissionGlyphName('A'), "UI_Mission_A");
    EXPECT_EQ(MakeMissionGlyphName('z'), "UI_Mission_Z");
    EXPECT_EQ(MakeMissionGlyphName('4'), "UI_Mission_4");
    EXPECT_EQ(MakeMissionGlyphName('0'), "UI_Mission_O");
    EXPECT_EQ(MakeMissionGlyphName('!'), "UI_Mission_Excl");
    EXPECT_EQ(MakeMissionGlyphName('?'), "UI_Mission_Question");
    EXPECT_TRUE(MakeMissionGlyphName('#').empty());
    EXPECT_TRUE(MakeMissionGlyphName(' ').empty());
}

TEST(UiBanner, GlyphPoseFliesInThenSitsThenFliesOut)
{
    constexpr int kIn   = 18;
    constexpr int kHold = 100;
    constexpr int kOut  = 18;

    const GlyphPose before = ComputeMissionGlyphPose(-1, kIn, kHold, kOut, 0);
    EXPECT_EQ(before.alpha, 0);

    const GlyphPose start = ComputeMissionGlyphPose(0, kIn, kHold, kOut, 0);
    EXPECT_EQ(start.alpha, 0);
    EXPECT_NE(start.dy, 0);   // 글자 0 은 위에서 내려온다

    const GlyphPose sitting = ComputeMissionGlyphPose(kIn + 10, kIn, kHold, kOut, 0);
    EXPECT_EQ(sitting.dx, 0);
    EXPECT_EQ(sitting.dy, 0);
    EXPECT_EQ(sitting.alpha, 255);

    const GlyphPose gone = ComputeMissionGlyphPose(kIn + kHold + kOut, kIn, kHold, kOut, 0);
    EXPECT_EQ(gone.alpha, 0);

    const GlyphPose leaving = ComputeMissionGlyphPose(kIn + kHold + kOut / 2, kIn, kHold, kOut, 0);
    EXPECT_GT(leaving.alpha, 0);
    EXPECT_LT(leaving.alpha, 255);
}

TEST(UiBanner, EachGlyphHasItsOwnDirectionAndDelay)
{
    const GlyphPose a = ComputeMissionGlyphPose(0, 18, 60, 18, 0);
    const GlyphPose b = ComputeMissionGlyphPose(0, 18, 60, 18, 2);
    EXPECT_TRUE(a.dx != b.dx || a.dy != b.dy);
    const GlyphPose late = ComputeMissionGlyphPose(4, 18, 60, 18, 6);
    EXPECT_EQ(late.alpha, 0);   // 6번째 글자는 아직 출발 전
}

TEST(UiFlow, TickBeforeStartAndAfterDoneIsHarmless)
{
    GameContext ctx;
    UiSystem    ui;
    ui.Init(ctx);
    TestAudio  audio;
    ScreenFlow flow;
    flow.Tick(ui, audio);
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
    EXPECT_EQ(flow.GetElapsedTicks(), elapsed);
}

namespace
{
struct TitleSim
{
    explicit TitleSim(
        TitleConfig _config = {})
        : rng(uint64_t { 1 })
    {
        ctx.pRng   = &rng;
        ctx.pAudio = &audio;
        pTitle     = new TitleState { std::move(_config) };
        runner     = std::unique_ptr<SimRunner> {
            new SimRunner { ctx, std::unique_ptr<IGameState> { pTitle } }
        };
    }

    void Step(
        const eAction _action,
        const bool    _bHeld)
    {
        InputFrame frame;
        frame.Set(_action, _bHeld);
        runner->Step(frame);
    }

    void Idle(
        const int _ticks)
    {
        for (int i = 0; i < _ticks; ++i)
        {
            runner->Step({});
        }
    }

    [[nodiscard]] bool IsStillTitle() const { return runner->GetState() == pTitle; }

    slug::RngMt19937           rng;
    GameContext                ctx;
    TestAudio                  audio;
    TitleState*                pTitle = nullptr;
    std::unique_ptr<SimRunner> runner = nullptr;
};

[[nodiscard]] TitleConfig SandboxTitleConfig_()
{
    TitleConfig config;
    config.nextState = "sandbox";
    return config;
}
}   // namespace

TEST(UiTitle, StaysOnTitleWithoutInput)
{
    TitleSim sim { SandboxTitleConfig_() };
    sim.Idle(300);
    EXPECT_TRUE(sim.IsStillTitle());
    EXPECT_FALSE(sim.pTitle->IsStarting());
}

TEST(UiTitle, ShotPressStartsFadeThenSwitchesStateExactlyAfterFade)
{
    TitleConfig config    = SandboxTitleConfig_();
    config.startFadeTicks = 24;
    TitleSim sim { config };
    sim.Idle(30);

    sim.Step(eAction::Shot, true);
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
    TitleSim sim { SandboxTitleConfig_() };
    sim.Idle(20);
    sim.Step(eAction::Start, true);
    EXPECT_TRUE(sim.pTitle->IsStarting());
    sim.Step(eAction::Start, true);
    sim.Step(eAction::Start, true);
    EXPECT_EQ(sim.audio.CountPlays(sfx::kOk), 1);
}

TEST(UiTitle, InputsDuringEntryDelayAreIgnored)
{
    TitleConfig config     = SandboxTitleConfig_();
    config.inputDelayTicks = 12;
    TitleSim sim { config };
    sim.Step(eAction::Shot, true);
    sim.Step(eAction::Shot, false);
    sim.Idle(5);
    EXPECT_FALSE(sim.pTitle->IsStarting());

    sim.Idle(10);   // 지연 이후
    sim.Step(eAction::Shot, true);
    EXPECT_TRUE(sim.pTitle->IsStarting());
}

TEST(UiTitle, MovementKeysDoNotStartTheGame)
{
    TitleSim sim { SandboxTitleConfig_() };
    sim.Idle(20);
    sim.Step(eAction::Right, true);
    sim.Step(eAction::Jump, true);
    sim.Step(eAction::Bomb, true);
    sim.Idle(5);
    EXPECT_FALSE(sim.pTitle->IsStarting());
}

TEST(UiTitle, AutoSelectionFallsBackToSandboxWhenStage0IsNotRegistered)
{
    TitleSim sim;
    sim.Idle(20);
    sim.Step(eAction::Start, true);
    const slug::String& selected = sim.pTitle->GetSelectedStateName();
    EXPECT_TRUE(selected == "stage0" || selected == "sandbox") << selected;
}

TEST(UiTitle, EntersWithBlackAndFadesIn)
{
    TitleSim sim { SandboxTitleConfig_() };
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
