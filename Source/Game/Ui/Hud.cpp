#include "Game/Ui/Hud.h"

#include <algorithm>
#include <format>

#include "Game/Player/PlayerController.h"

namespace slug
{

// ===========================================
//  포맷
// ===========================================

jug::String FormatScore(
    const int _score)
{
    return std::to_string(std::clamp(_score, 0, 9'999'999));
}

jug::String FormatAmmo(
    const int _ammo)
{
    return std::to_string(std::clamp(_ammo, 0, 999));
}

jug::String FormatBombs(
    const int _bombs)
{
    return std::to_string(std::clamp(_bombs, 0, 99));
}

jug::String FormatTimer(
    const int _seconds)
{
    return std::format("{:02}", std::clamp(_seconds, 0, 99));
}

// ===========================================
//  모델
// ===========================================

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
    return model;
}

// ===========================================
//  배치
// ===========================================

namespace
{
    [[nodiscard]] HudItem MakeText_(
        jug::String      _text,
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
        jug::String _name,
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
}   // namespace

jug::Vector<HudItem> BuildHudItems(
    const HudModel& _model,
    const UiLayout& _layout)
{
    jug::Vector<HudItem> items;

    if (_model.bShowPlayer)
    {
        // 점수 (오른쪽 정렬), 목숨
        items.push_back(MakeText_(FormatScore(_model.score), _layout.scoreRightX, _layout.scoreY, eTextAlign::Right, eFontColor::Default));
        items.push_back(MakeText_(std::format("1UP X{}", std::max(0, _model.lives)), _layout.livesX, _layout.livesY, eTextAlign::Left, eFontColor::White));

        // ARMS / BOMB 패널 + 숫자
        items.push_back(MakeSprite_("PlayerStateUI_Bomb", _layout.armsPanelX + 62, _layout.armsPanelY + 17));

        const int digitsY = _layout.armsPanelY + _layout.digitsDY;
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

    // 타이머
    if (_model.bShowTimer)
    {
        HudItem item;
        item.kind = eHudItemKind::TimeDigits;
        item.text = FormatTimer(_model.timerSeconds);
        item.x    = _layout.timerCenterX;
        item.y    = _layout.timerTopY;
        items.push_back(std::move(item));
    }

    // 2P PRESS START 점멸
    if (_model.bShowPressStart2p && IsBlinkOn(_model.uiTick, _layout.pressStartBlinkTicks))
    {
        items.push_back(MakeSprite_("PressStartUI", _layout.pressStartCenterX, _layout.pressStartCenterY));
    }

    // 레벨 표시
    if (!_model.levelSprite.empty())
    {
        items.push_back(MakeSprite_(_model.levelSprite, _layout.levelCenterX, _layout.levelCenterY));
    }

    // 보스 HP 바: 안쪽 채움(먼저) → 틀(위)
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
            fill.color   = ratio > 0.3f ? jug::RGBA { 60, 200, 255, 255 } : jug::RGBA { 255, 70, 50, 255 };
            items.push_back(fill);
        }

        items.push_back(MakeSprite_("PlayerHPUI", left + (kFrameW * scale) / 2, top + (kFrameH * scale) / 2, scale));
    }

    return items;
}

}   // namespace slug
