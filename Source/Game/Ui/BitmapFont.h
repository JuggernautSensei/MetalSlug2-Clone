#pragma once
#include <cstdint>

#include "Base/RGBA.h"
#include "Base/Types.h"
#include "Render/Animation.h"
#include "Render/Renderer.h"
#include "Render/SpriteBatch.h"
#include "Render/SpriteLibrary.h"

enum class eTextAlign : uint8_t
{
    Left,
    Center,
    Right
};

enum class eFontColor : uint8_t
{
    Default,
    White,
    RedYellow,
    YellowWhite,
    Red,
    Yellow,
    Count
};

struct TextStyle
{
    eFontColor color         = eFontColor::White;
    bool       bBold         = true;
    int        scale         = 1;   // 정수 배율 (>= 1)
    eTextAlign align         = eTextAlign::Left;
    uint8_t    alpha         = 255;   // 페이드용 불투명도
    int        lineGap       = 4;
    int        letterSpacing = 0;   // 글자 사이 추가 간격 (px, 배율 무관)
};

inline constexpr int kGlyphSize    = 16;
inline constexpr int kGlyphColumns = 35;
inline constexpr int kGlyphRows    = 2;   // 0 = 굵게, 1 = 가늘게

[[nodiscard]] int GetGlyphIndex(
    const char _ch);

[[nodiscard]] int MeasureChars(
    const int _charCount,
    const int _scale,
    const int _letterSpacing = 0);

struct GlyphQuad
{
    char    ch        = ' ';
    int     glyph     = -1;   // 열 인덱스. -1: 스프라이트 글리프 없음
    IntRect dst       = {};
    bool    bFallback = false;
};

struct TextLayout
{
    Vector<GlyphQuad> quads  = {};
    int               width  = 0;   // 가장 넓은 줄의 폭
    int               height = 0;   // 전체 높이
    int               lines  = 0;
    IntRect           bounds = {};
};

[[nodiscard]] TextLayout LayoutText(
    StringView       _text,
    int              _x,
    int              _y,
    const TextStyle& _style);

class BitmapFont
{
public:
    bool Init(const SpriteLibrary& _sprites);

    [[nodiscard]] bool HasSprites() const;

    void DrawText(
        Renderer&        _renderer,
        SpriteBatch&     _sprites,
        int              _x,
        int              _y,
        StringView       _text,
        const TextStyle& _style) const;

    void DrawTimeDigits(
        SpriteBatch& _sprites,
        int          _centerX,
        int          _topY,
        StringView   _digits,
        uint8_t      _alpha = 255) const;

    [[nodiscard]] AnimFrame GetGlyphFrame(eFontColor _color, bool _bBold, int _glyph) const;
    [[nodiscard]] AnimFrame GetTimeDigitFrame(int _digit) const;

private:
    ARRAY<AnimFrame, static_cast<size_t>(eFontColor::Count)> m_strips    = {};
    AnimFrame                                                m_timeStrip = {};
    bool                                                     m_bReady    = false;
};
