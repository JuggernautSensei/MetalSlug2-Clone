#pragma once
#include <cstdint>

#include "Base/RGBA.h"
#include "Base/Types.h"

#include "Game/Ui/BitmapFont.h"
#include "Game/Ui/UiLayout.h"

namespace slug
{

class PlayerController;

struct HudModel
{
    bool bShowPlayer    = true;
    int  lives          = 0;      // "1UP X<lives>" (남은 목숨 수. 현재 목숨 포함)
    int  score          = 0;
    bool bInfiniteAmmo  = true;
    int  ammo           = 0;      // 유한 무기의 남은 탄
    int  bombs          = 0;
    char weaponLetter   = 0;      // 든 무기 글자 (H/S/R/F). 권총은 0

    bool bShowTimer     = false;
    int  timerSeconds   = 0;

    bool  bShowBoss     = false;
    float bossHpRatio   = 1.f;    // 0..1

    bool        bShowPressStart2p = true;
    slug::String levelSprite       = {};
    int         uiTick            = 0;      // 점멸용 단조 증가 틱 (UiSystem::GetTick())
};

[[nodiscard]] HudModel MakeHudModel(const PlayerController& _player);

enum class eHudItemKind : uint8_t
{
    Text,
    Sprite,
    TimeDigits,
    Rect
};

struct HudItem
{
    eHudItemKind kind  = eHudItemKind::Text;
    slug::String  text  = {};
    int          x     = 0;
    int          y     = 0;
    int          w     = 0;       // Rect 크기
    int          h     = 0;
    int          scale = 1;       // Sprite 배율
    TextStyle    style = {};
    slug::RGBA    color = { 255, 255, 255, 255 };   // Rect
};

[[nodiscard]] slug::Vector<HudItem> BuildHudItems(const HudModel& _model, const UiLayout& _layout);

[[nodiscard]] constexpr bool IsBlinkOn(
    const int _tick,
    const int _periodTicks)
{
    return _periodTicks <= 0 || ((_tick / _periodTicks) % 2) == 0;
}

[[nodiscard]] slug::String FormatScore(int _score);
[[nodiscard]] slug::String FormatAmmo(int _ammo);
[[nodiscard]] slug::String FormatBombs(int _bombs);
[[nodiscard]] slug::String FormatTimer(int _seconds);
}
