#include "Game/Ui/Hud.h"
#include "Base/Types.h"
#include "Base/RGBA.h"

#include <algorithm>
#include <format>

#include "Game/Player/PlayerController.h"

namespace slug
{
slug::String FormatScore(
    const int _score)
{
    return std::to_string(std::clamp(_score, 0, 9'999'999));
}

slug::String FormatAmmo(
    const int _ammo)
{
    return std::to_string(std::clamp(_ammo, 0, 999));
}

slug::String FormatBombs(
    const int _bombs)
{
    return std::to_string(std::clamp(_bombs, 0, 99));
}

slug::String FormatTimer(
    const int _seconds)
{
    return std::format("{:02}", std::clamp(_seconds, 0, 99));
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
    switch (_player.GetWeapon().weapon)
    {
        case eWeapon::Hmg: model.weaponLetter = 'H'; break;
        case eWeapon::Shotgun: model.weaponLetter = 'S'; break;
        case eWeapon::Rocket: model.weaponLetter = 'R'; break;
        case eWeapon::Flame: model.weaponLetter = 'F'; break;
        default: break;
    }
    return model;
}

namespace
{
    [[nodiscard]] HudItem MakeText_(
        slug::String      _text,
        const int        _x,
        const int        _y,
        const eTextAlign _align,
        const eFontColor _color)
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
        slug::String _name,
        const int   _centerX,
        const int   _centerY,
        const int   _scale = 1)
    {
        HudItem item;
        item.kind  = eHudItemKind::Sprite;
        item.text  = std::move(_name);
        item.x     = _centerX;
        item.y     = _centerY;
        item.scale = _scale;
        return item;
    }
}

slug::Vector<HudItem> BuildHudItems(
    const HudModel& _model,
    const UiLayout& _layout)
{
    slug::Vector<HudItem> items;

    if (_model.bShowPlayer)
    {
        items.push_back(MakeText_(FormatScore(_model.score), _layout.scoreRightX, _layout.scoreY, eTextAlign::Right, eFontColor::Default));
        items.push_back(MakeText_(std::format("1UP X{}", std::max(0, _model.lives)), _layout.livesX, _layout.livesY, eTextAlign::Left, eFontColor::White));

        items.push_back(MakeSprite_("PlayerStateUI_Bomb", _layout.armsPanelX + 62, _layout.armsPanelY + 17));

        const int digitsY = _layout.armsPanelY + _layout.digitsDY;
        if (_model.weaponLetter != 0)
        {
            items.push_back(MakeText_(slug::String(1, _model.weaponLetter), _layout.armsPanelX + _layout.weaponLetterDX, digitsY, eTextAlign::Left, eFontColor::White));
        }
        if (_model.bInfiniteAmmo)
        {
            items.push_back(MakeSprite_("InfiniteUI_Red", _layout.armsPanelX + _layout.infiniteCenterDX, _layout.armsPanelY + _layout.infiniteCenterDY));
        }
        else
        {
            items.push_back(MakeText_(FormatAmmo(_model.ammo), _layout.armsPanelX + _layout.ammoRightDX, digitsY, eTextAlign::Right, eFontColor::RedYellow));
        }
        items.push_back(MakeText_(FormatBombs(_model.bombs), _layout.armsPanelX + _layout.bombRightDX, digitsY, eTextAlign::Right, eFontColor::RedYellow));
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

    if (_model.bShowPressStart2p && IsBlinkOn(_model.uiTick, _layout.pressStartBlinkTicks))
    {
        items.push_back(MakeSprite_("PressStartUI", _layout.pressStartCenterX, _layout.pressStartCenterY));
    }

    if (!_model.levelSprite.empty())
    {
        items.push_back(MakeSprite_(_model.levelSprite, _layout.levelCenterX, _layout.levelCenterY));
    }

    if (_model.bShowBoss)
    {
        constexpr int kFrameW = 106;   // PlayerHPUI
        constexpr int kFrameH = 16;
        const int     scale   = std::max(1, _layout.bossBarScale);
        const int     left    = _layout.bossBarCenterX - (kFrameW * scale) / 2;
        const int     top     = _layout.bossBarTopY;
        const float   ratio   = std::clamp(_model.bossHpRatio, 0.f, 1.f);
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
            fill.color   = ratio > 0.3f ? slug::RGBA { 60, 200, 255, 255 } : slug::RGBA { 255, 70, 50, 255 };
            items.push_back(fill);
        }

        items.push_back(MakeSprite_("PlayerHPUI", left + (kFrameW * scale) / 2, top + (kFrameH * scale) / 2, scale));
    }

    return items;
}
}
