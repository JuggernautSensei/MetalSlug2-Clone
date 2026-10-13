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
    int        scale         = 1;
    eTextAlign align         = eTextAlign::Left;
    uint8_t    alpha         = 255;
    int        lineGap       = 4;
    int        letterSpacing = 0;   // 글자 사이 추가 간격
};

inline constexpr int kGlyphSize    = 16;
inline constexpr int kGlyphColumns = 35;
inline constexpr int kGlyphRows    = 2;

[[nodiscard]] int GetGlyphIndex(char _ch);

[[nodiscard]] int MeasureChars(int _charCount, int _scale, int _letterSpacing = 0);

struct GlyphQuad
{
    char    ch        = ' ';
    int     glyph     = -1;
    IntRect dst       = {};
    bool    bFallback = false;
};

struct TextLayout
{
    Vector<GlyphQuad> quads  = {};
    int               width  = 0;
    int               height = 0;
    int               lines  = 0;
    IntRect           bounds = {};
};

[[nodiscard]] TextLayout LayoutText(StringView _text, int _x, int _y, const TextStyle& _style);

class BitmapFont
{
public:
    bool Init(const SpriteLibrary& _sprites);

    [[nodiscard]] bool HasSprites() const;

    void DrawText(Renderer& _renderer, SpriteBatch& _sprites, int _x, int _y, StringView _text, const TextStyle& _style) const;

    void DrawTimeDigits(SpriteBatch& _sprites, int _centerX, int _topY, StringView _digits, uint8_t _alpha = 255) const;

    [[nodiscard]] AnimFrame GetGlyphFrame(eFontColor _color, bool _bBold, int _glyph) const;
    [[nodiscard]] AnimFrame GetTimeDigitFrame(int _digit) const;

private:
    DIRECT_ENUM_ARRAY<eFontColor, AnimFrame> m_strips    = {};
    AnimFrame                                m_timeStrip = {};
    bool                                     m_bReady    = false;
};
