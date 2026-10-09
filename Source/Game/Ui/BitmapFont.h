#pragma once
#include <cstdint>

#include "Base/RGBA.h"
#include "Base/Types.h"

#include "Render/Animation.h"
#include "Render/Renderer.h"
#include "Render/SpriteBatch.h"
#include "Render/SpriteLibrary.h"

namespace slug
{

// ===========================================
//  글리프 매핑 / 텍스트 배치 (순수 계산, 렌더러 불필요 → 단위 테스트 대상)
//
//   레거시 Font_*.bmp 는 16x16 격자 35열 x 2행 (위 행 = 굵게, 아래 행 = 가늘게).
//   열: A~Z = 0..25, 1~9 = 26..34. 0 은 없어서 'O'(14) 로 대신한다 (레거시와 동일).
//   그 외 문자(구두점 등)는 스프라이트 글리프가 없다 → 디버그 폰트로 대체하고 칸만 차지한다.
//   모든 글자의 전진 폭은 같다: glyphSize * scale + letterSpacing (공백도 한 칸).
// ===========================================

enum class eTextAlign : uint8_t
{
    Left,     // x = 줄 왼쪽
    Center,   // x = 줄 중심
    Right     // x = 줄 오른쪽
};

enum class eFontColor : uint8_t
{
    Default,       // Font_Normal (회청색 그라디언트)
    White,         // Font_White
    RedYellow,     // Font_Red_Yellow (탄약 숫자)
    YellowWhite,   // Font_Yellow_White
    Red,           // Font_Red
    Yellow,        // Font_Yellow (배너 기본)
    Count
};

struct TextStyle
{
    eFontColor color         = eFontColor::White;
    bool       bBold         = true;    // 굵은 행 (false: 가는 행)
    int        scale         = 1;       // 정수 배율 (>= 1)
    eTextAlign align         = eTextAlign::Left;
    uint8_t    alpha         = 255;     // 페이드용 불투명도
    int        lineGap       = 4;       // 줄 사이 간격 (배율 1 기준 px, 실제 간격 = lineGap * scale)
    int        letterSpacing = 0;       // 글자 사이 추가 간격 (px, 배율 무관)
};

namespace fontmap
{
    inline constexpr int kGlyphSize    = 16;
    inline constexpr int kGlyphColumns = 35;
    inline constexpr int kGlyphRows    = 2;   // 0 = 굵게, 1 = 가늘게

    // 문자 → 열 인덱스 (대소문자 무시). 글리프가 없으면 -1.
    [[nodiscard]] constexpr int GetGlyphIndex(
        const char _ch)
    {
        const char c = (_ch >= 'a' && _ch <= 'z') ? static_cast<char>(_ch - 'a' + 'A') : _ch;
        if (c >= 'A' && c <= 'Z')
        {
            return c - 'A';
        }
        if (c >= '1' && c <= '9')
        {
            return 26 + (c - '1');
        }
        if (c == '0')
        {
            return 'O' - 'A';
        }
        return -1;
    }

    [[nodiscard]] constexpr bool HasGlyph(const char _ch) { return GetGlyphIndex(_ch) >= 0; }

    // 줄 하나의 폭
    [[nodiscard]] constexpr int MeasureChars(
        const int _charCount,
        const int _scale,
        const int _letterSpacing = 0)
    {
        return _charCount <= 0 ? 0 : _charCount * kGlyphSize * _scale + (_charCount - 1) * _letterSpacing;
    }
}   // namespace fontmap

struct GlyphQuad
{
    char    ch        = ' ';
    int     glyph     = -1;       // 열 인덱스. -1: 스프라이트 글리프 없음
    IntRect dst       = {};       // 화면 사각형 (한 칸)
    bool    bFallback = false;    // true: 공백이 아닌데 글리프가 없어 디버그 폰트로 그려야 함
};

struct TextLayout
{
    slug::Vector<GlyphQuad> quads  = {};
    int                    width  = 0;   // 가장 넓은 줄의 폭
    int                    height = 0;   // 전체 높이
    int                    lines  = 0;
    IntRect                bounds = {};  // 전체를 감싸는 사각형
};

// 문자열(\n 으로 줄바꿈)을 배치한다. _x, _y: 정렬 기준점(_style.align 에 따라 줄의 왼쪽/중심/오른쪽 x), y 는 첫 줄의 위쪽.
[[nodiscard]] TextLayout LayoutText(slug::StringView _text, int _x, int _y, const TextStyle& _style);

// 문자열이 차지하는 크기 (배치 없이)
[[nodiscard]] slug::Vector<int> SplitLineLengths(slug::StringView _text);
[[nodiscard]] int              MeasureTextWidth(slug::StringView _text, const TextStyle& _style);
[[nodiscard]] int              MeasureTextHeight(slug::StringView _text, const TextStyle& _style);

// 폭이 _maxWidth 에 들어가는 최대 정수 배율 (1.._maxScale)
[[nodiscard]] int FitTextScale(slug::StringView _text, int _maxWidth, int _maxScale, int _letterSpacing = 0);

// 폰트 색상의 대표 RGBA (디버그 폰트 대체 글자 색)
[[nodiscard]] slug::RGBA GetFontFallbackColor(eFontColor _color);

// 폰트 색상 → sprites.json 이름
[[nodiscard]] const char* GetFontSpriteName(eFontColor _color);

// ===========================================
//  BitmapFont
//   sprites.json 의 Font_* / TimeNumberUI 스트립에서 글리프를 잘라 그린다. 스프라이트가 없으면(헤드리스/에셋 없음)
//   모든 글자를 디버그 폰트로 대체하되 배치(칸)는 동일하다.
// ===========================================

class BitmapFont
{
public:
    // 스프라이트 라이브러리에서 폰트 스트립을 찾는다. 하나라도 찾으면 true.
    bool Init(const SpriteLibrary& _sprites);

    [[nodiscard]] bool HasSprites() const { return m_bReady; }

    // 문자열을 그린다. _y 는 첫 줄 위쪽. 스프라이트 글리프는 SpriteBatch 로, 대체 글자는 Renderer 의 디버그 폰트로 그린다.
    void DrawText(Renderer& _renderer, SpriteBatch& _sprites, int _x, int _y, slug::StringView _text, const TextStyle& _style) const;

    // 숫자 문자열(0-9)을 TimeNumberUI(32x32) 로 그린다. _centerX 가 폭의 중심. 숫자 이외 문자는 건너뛴다.
    void DrawTimeDigits(SpriteBatch& _sprites, int _centerX, int _topY, slug::StringView _digits, uint8_t _alpha = 255) const;

    // 글리프 프레임 (테스트/도구용). 없으면 빈 프레임.
    [[nodiscard]] AnimFrame GetGlyphFrame(eFontColor _color, bool _bBold, int _glyph) const;
    [[nodiscard]] AnimFrame GetTimeDigitFrame(int _digit) const;

private:
    AnimFrame m_strips[static_cast<size_t>(eFontColor::Count)] = {};
    AnimFrame m_timeStrip                                      = {};
    bool      m_bReady                                         = false;
};

}   // namespace slug
