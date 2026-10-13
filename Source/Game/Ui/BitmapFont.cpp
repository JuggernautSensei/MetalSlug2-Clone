#include "Game/Ui/BitmapFont.h"

#include <algorithm>
#include <SDL3/SDL.h>

#include "Base/RGBA.h"
#include "Base/Types.h"

namespace
{
[[nodiscard]] RGBA GetFontFallbackColor_(
    const eFontColor _color)
{
    switch (_color)
    {
        case eFontColor::White: return { 255, 255, 255, 255 };
        case eFontColor::RedYellow: return { 255, 120, 40, 255 };
        case eFontColor::YellowWhite: return { 255, 240, 160, 255 };
        case eFontColor::Red: return { 255, 40, 30, 255 };
        case eFontColor::Yellow: return { 255, 230, 20, 255 };
        default: return { 190, 200, 225, 255 };
    }
}

[[nodiscard]] const char* GetFontSpriteName_(
    const eFontColor _color)
{
    switch (_color)
    {
        case eFontColor::White: return "Font_White";
        case eFontColor::RedYellow: return "Font_Red_Yellow";
        case eFontColor::YellowWhite: return "Font_Yellow_White";
        case eFontColor::Red: return "Font_Red";
        case eFontColor::Yellow: return "Font_Yellow";
        default: return "Font_Normal";
    }
}
}   // namespace

TextLayout LayoutText(
    const StringView _text,
    const int        _x,
    const int        _y,
    const TextStyle& _style)
{
    const int scale   = Max(1, _style.scale);
    const int cell    = kGlyphSize * scale;
    const int advance = cell + _style.letterSpacing;
    const int gap     = _style.lineGap * scale;

    TextLayout layout;
    layout.quads.reserve(_text.size());

    int lineIndex = 0;
    int lineStart = 0;
    while (lineStart <= static_cast<int>(_text.size()))
    {
        int lineEnd = lineStart;
        while (lineEnd < static_cast<int>(_text.size()) && _text[static_cast<size_t>(lineEnd)] != '\n')
        {
            ++lineEnd;
        }

        const int length = lineEnd - lineStart;
        const int width  = MeasureChars(length, scale, _style.letterSpacing);
        int       x0     = _x;
        if (_style.align == eTextAlign::Center)
        {
            x0 = _x - width / 2;
        }
        else if (_style.align == eTextAlign::Right)
        {
            x0 = _x - width;
        }
        const int y0 = _y + lineIndex * (cell + gap);

        for (int i = 0; i < length; ++i)
        {
            GlyphQuad quad;
            quad.ch        = _text[static_cast<size_t>(lineStart + i)];
            quad.glyph     = GetGlyphIndex(quad.ch);
            quad.dst       = { x0 + i * advance, y0, cell, cell };
            quad.bFallback = quad.glyph < 0 && quad.ch != ' ';
            layout.quads.push_back(quad);
        }

        layout.width = Max(layout.width, width);
        ++lineIndex;
        lineStart = lineEnd + 1;
    }

    layout.lines  = lineIndex;
    layout.height = lineIndex * cell + (lineIndex - 1) * gap;

    int left = _x;
    if (_style.align == eTextAlign::Center)
    {
        left = _x - layout.width / 2;
    }
    else if (_style.align == eTextAlign::Right)
    {
        left = _x - layout.width;
    }
    layout.bounds = { left, _y, layout.width, layout.height };
    return layout;
}

bool BitmapFont::Init(
    const SpriteLibrary& _sprites)
{
    m_bReady = false;
    for (size_t i = 0; i < static_cast<size_t>(eFontColor::Count); ++i)
    {
        m_strips[i] = {};
        if (const SpriteInfo* pInfo = _sprites.Find(GetFontSpriteName_(static_cast<eFontColor>(i))); pInfo && !pInfo->bStandalone)
        {
            m_strips[i] = pInfo->frame;
            m_bReady    = true;
        }
    }
    m_timeStrip = {};
    if (const SpriteInfo* pInfo = _sprites.Find("TimeNumberUI"); pInfo && !pInfo->bStandalone)
    {
        m_timeStrip = pInfo->frame;
    }
    return m_bReady;
}

AnimFrame BitmapFont::GetGlyphFrame(
    const eFontColor _color,
    const bool       _bBold,
    const int        _glyph) const
{
    const AnimFrame& strip = m_strips[static_cast<size_t>(_color)];
    if (strip.IsEmpty() || _glyph < 0 || _glyph >= kGlyphColumns)
    {
        return {};
    }

    AnimFrame frame = strip;
    frame.x         = strip.x + _glyph * kGlyphSize;
    frame.y         = strip.y + (_bBold ? 0 : 1) * kGlyphSize;
    frame.w         = kGlyphSize;
    frame.h         = kGlyphSize;
    frame.pivotX    = 0;
    frame.pivotY    = 0;
    return frame;
}

AnimFrame BitmapFont::GetTimeDigitFrame(
    const int _digit) const
{
    if (m_timeStrip.IsEmpty() || _digit < 0 || _digit > 9)
    {
        return {};
    }

    constexpr int kCell = 32;
    AnimFrame     frame = m_timeStrip;
    frame.x             = m_timeStrip.x + _digit * kCell;
    frame.w             = kCell;
    frame.h             = Min(kCell, m_timeStrip.h);
    frame.pivotX        = 0;
    frame.pivotY        = 0;
    return frame;
}

void BitmapFont::DrawText(
    Renderer&        _renderer,
    SpriteBatch&     _sprites,
    const int        _x,
    const int        _y,
    const StringView _text,
    const TextStyle& _style) const
{
    const TextLayout layout = LayoutText(_text, _x, _y, _style);

    SpriteDrawOptions options;
    options.tint.a = _style.alpha;

    SDL_Renderer* pNative = _renderer.GetNative();
    const int     scale   = Max(1, _style.scale);

    for (const GlyphQuad& quad: layout.quads)
    {
        if (quad.glyph >= 0 && m_bReady)
        {
            const AnimFrame frame = GetGlyphFrame(_style.color, _style.bBold, quad.glyph);
            _sprites.DrawFrameRect(frame, quad.dst, options);
        }
        else if (quad.ch != ' ' && pNative)
        {
            RGBA color = GetFontFallbackColor_(_style.color);
            color.a    = _style.alpha;

            const float          k    = static_cast<float>(scale * 2);
            const ARRAY<char, 2> text = { quad.ch, '\0' };
            SDL_SetRenderDrawColor(pNative, color.r, color.g, color.b, color.a);
            SDL_SetRenderScale(pNative, k, k);
            SDL_RenderDebugText(pNative, static_cast<float>(quad.dst.x) / k, static_cast<float>(quad.dst.y) / k, text.data());
            SDL_SetRenderScale(pNative, 1.f, 1.f);
        }
    }
}

void BitmapFont::DrawTimeDigits(
    SpriteBatch&     _sprites,
    const int        _centerX,
    const int        _topY,
    const StringView _digits,
    const uint8_t    _alpha) const
{
    constexpr int kCell = 32;

    int count = 0;
    for (const char c: _digits)
    {
        count += (c >= '0' && c <= '9') ? 1 : 0;
    }

    SpriteDrawOptions options;
    options.tint.a = _alpha;

    int x = _centerX - (count * kCell) / 2;
    for (const char c: _digits)
    {
        if (c < '0' || c > '9')
        {
            continue;
        }
        const AnimFrame frame = GetTimeDigitFrame(c - '0');
        _sprites.DrawFrameRect(frame, { x, _topY, kCell, frame.h }, options);
        x += kCell;
    }
}

bool BitmapFont::HasSprites() const
{
    return m_bReady;
}

int GetGlyphIndex(
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

int MeasureChars(
    const int _charCount,
    const int _scale,
    const int _letterSpacing)
{
    return _charCount <= 0 ? 0 : _charCount * kGlyphSize * _scale + (_charCount - 1) * _letterSpacing;
}
