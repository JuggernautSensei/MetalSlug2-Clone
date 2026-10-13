#include "Game/Ui/Hud.h"

#include <algorithm>
#include <format>

#include "Base/RGBA.h"
#include "Base/Types.h"
#include "Game/Player/PlayerController.h"

namespace
{
[[nodiscard]] HudItem MakeText_(
    String     _text,
    int        _x,
    int        _y,
    eTextAlign _align,
    eFontColor _color)
{
    HudItem item;
    item.kind        = eHudItemKind::Text;
    item.text        = std::move(_text);
    item.x           = _x;
    item.y           = _y;
    item.style.align = _align;
    item.style.color = _color;
    item.style.bBold = true;
    return item;
}

[[nodiscard]] HudItem MakeSprite_(
    String _name,
    int    _centerX,
    int    _centerY,
    int    _scale = 1)
{
    HudItem item;
    item.kind  = eHudItemKind::Sprite;
    item.text  = std::move(_name);
    item.x     = _centerX;
    item.y     = _centerY;
    item.scale = _scale;
    return item;
}
}   // namespace

String FormatScore(
    int _score)
{
    return std::to_string(Clamp(_score, 0, 9'999'999));
}

String FormatAmmo(
    int _ammo)
{
    return std::to_string(Clamp(_ammo, 0, 999));
}

String FormatBombs(
    int _bombs)
{
    return std::to_string(Clamp(_bombs, 0, 99));
}

String FormatTimer(
    int _seconds)
{
    return std::format("{:02}", Clamp(_seconds, 0, 99));
}

HudModel MakeHudModel(
    const PlayerController& _player)
{
    HudModel model;
    model.bShowPlayer   = true;
    model.lives         = _player.GetLives();
    model.score         = _player.GetScore();
    model.bInfiniteAmmo = !_player.GetWeapon().HasFiniteAmmo();
    model.ammo          = _player.GetWeapon().ammo;
    model.bombs         = _player.GetWeapon().bombs;
    // 원작 HUD 는 무기 글자 없이 탄수 숫자만 보여준다
    return model;
}

Vector<HudItem> BuildHudItems(
    const HudModel& _model,
    const UiLayout& _layout)
{
    Vector<HudItem> items;

    if (_model.bShowPlayer)
    {
        items.push_back(MakeText_(FormatScore(_model.score), _layout.scoreRightX, _layout.scoreY, eTextAlign::Right, eFontColor::Default));
        items.push_back(MakeText_(std::format("1UP={}", Max(0, _model.lives)), _layout.livesX, _layout.livesY, eTextAlign::Left, eFontColor::White));

        items.push_back(MakeSprite_(_model.bSlugMode ? "PlayerStateUI_Cannon" : "PlayerStateUI_Bomb", _layout.armsPanelX + 62, _layout.armsPanelY + 17));

        const int digitsY = _layout.armsPanelY + _layout.digitsDY;
        if (_model.weaponLetter != 0)
        {
            items.push_back(MakeText_(String(1, _model.weaponLetter), _layout.armsPanelX + _layout.weaponLetterDX, digitsY, eTextAlign::Left, eFontColor::White));
        }
        if (_model.bInfiniteAmmo || _model.bSlugMode)
        {
            items.push_back(MakeSprite_("InfiniteUI_Red", _layout.armsPanelX + _layout.infiniteCenterDX, _layout.armsPanelY + _layout.infiniteCenterDY));
        }
        else
        {
            items.push_back(MakeText_(FormatAmmo(_model.ammo), _layout.armsPanelX + _layout.ammoRightDX, digitsY, eTextAlign::Right, eFontColor::RedYellow));
        }
        items.push_back(MakeText_(FormatBombs(_model.bSlugMode ? _model.cannonAmmo : _model.bombs), _layout.armsPanelX + _layout.bombRightDX, digitsY, eTextAlign::Right, eFontColor::RedYellow));
    }

    if (_model.bShowGauge)
    {
        const float ratio = Clamp(_model.gaugeRatio, 0.f, 1.f);
        const int   fillW = static_cast<int>(static_cast<float>(_layout.gaugeInnerW) * ratio + 0.5f);
        HudItem     back;
        back.kind  = eHudItemKind::Rect;
        back.x     = _layout.gaugeX + _layout.gaugeInnerX;
        back.y     = _layout.gaugeY + _layout.gaugeInnerY;
        back.w     = _layout.gaugeInnerW;
        back.h     = _layout.gaugeInnerH;
        back.color = { 40, 46, 66, 255 };
        items.push_back(back);
        if (fillW > 0)
        {
            HudItem fill = back;
            fill.w       = fillW;
            fill.h       = _layout.gaugeInnerH / 2;
            fill.color   = { 255, 150, 30, 255 };
            items.push_back(fill);
            HudItem fillLow = fill;
            fillLow.y       = back.y + fill.h;
            fillLow.h       = _layout.gaugeInnerH - fill.h;
            fillLow.color   = { 255, 96, 8, 255 };
            items.push_back(fillLow);
        }
        items.push_back(MakeSprite_("PlayerHPUI", _layout.gaugeX + 53, _layout.gaugeY + 8));
    }

    if (_model.bShowTimer)
    {
        HudItem item;
        item.kind = eHudItemKind::TimeDigits;
        item.text = FormatTimer(_model.timerSeconds);
        item.x    = _layout.timerCenterX;
        item.y    = _layout.timerTopY;
        items.push_back(std::move(item));
    }

    if (_model.bShowInsertCoin)
    {
        items.push_back(MakeText_("INSERT COIN", _layout.insertCoinRightX, _layout.insertCoinY, eTextAlign::Right, eFontColor::White));
    }

    if (!_model.levelSprite.empty())
    {
        items.push_back(MakeSprite_(_model.levelSprite, _layout.levelCenterX, _layout.levelCenterY));
    }

    if (_model.bShowBoss)
    {
        constexpr int kFrameW = 106;
        constexpr int kFrameH = 16;
        const int     scale   = Max(1, _layout.bossBarScale);
        const int     left    = _layout.bossBarCenterX - (kFrameW * scale) / 2;
        const int     top     = _layout.bossBarTopY;
        const float   ratio   = Clamp(_model.bossHpRatio, 0.f, 1.f);
        const int     fillW   = static_cast<int>(static_cast<float>(_layout.bossBarInnerW * scale) * ratio + 0.5f);

        HudItem back;
        back.kind  = eHudItemKind::Rect;
        back.x     = left + _layout.bossBarInnerX * scale;
        back.y     = top + _layout.bossBarInnerY * scale;
        back.w     = _layout.bossBarInnerW * scale;
        back.h     = _layout.bossBarInnerH * scale;
        back.color = { 24, 24, 40, 255 };
        items.push_back(back);

        if (fillW > 0)
        {
            HudItem fill = back;
            fill.w       = fillW;
            fill.color   = ratio > 0.3f ? RGBA { 60, 200, 255, 255 } : RGBA { 255, 70, 50, 255 };
            items.push_back(fill);
        }

        items.push_back(MakeSprite_("PlayerHPUI", left + (kFrameW * scale) / 2, top + (kFrameH * scale) / 2, scale));
    }

    return items;
}

void DisplayTimer::Reset()
{
    m_ticks = kStartSeconds * kTicksPerDigit;
}

void DisplayTimer::Tick()
{
    if (m_ticks > 0)
    {
        --m_ticks;
    }
}

int DisplayTimer::GetSeconds() const
{
    return (m_ticks + kTicksPerDigit - 1) / kTicksPerDigit;
}

int DisplayTimer::GetRemainingTicks() const
{
    return m_ticks;
}

bool IsBlinkOn(
    int _tick,
    int _periodTicks)
{
    return _periodTicks <= 0 || ((_tick / _periodTicks) % 2) == 0;
}
