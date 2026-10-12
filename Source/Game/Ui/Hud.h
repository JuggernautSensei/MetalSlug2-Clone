#pragma once
#include <cstdint>

#include "Base/RGBA.h"
#include "Base/Types.h"
#include "Game/Ui/BitmapFont.h"
#include "Game/Ui/UiLayout.h"

class PlayerController;

struct HudModel
{
    bool bShowPlayer   = true;
    int  lives         = 0;
    int  score         = 0;
    bool bInfiniteAmmo = true;
    int  ammo          = 0;
    int  bombs         = 0;
    char weaponLetter  = 0;

    bool  bSlugMode  = false;
    int   cannonAmmo = 0;
    bool  bShowGauge = false;
    float gaugeRatio = 0.f;   // 0..1 (오렌지 채움)

    bool bShowTimer   = false;
    int  timerSeconds = 0;

    bool  bShowBoss   = false;
    float bossHpRatio = 1.f;   // 0..1

    bool   bShowInsertCoin = true;
    String levelSprite     = {};
    int    uiTick          = 0;
};

// 화면 표시용 60초 카운트다운. 사망과 무관하다 (0 이 되어도 아무 일도 없음).
// 원본 영상에서 숫자는 1초가 아니라 약 6.5초마다 한 칸씩 줄고, 구간이 바뀌면 60 으로 돌아간다.
class DisplayTimer
{
public:
    static constexpr int kStartSeconds  = 60;
    static constexpr int kTicksPerDigit = 390;

    void Reset();
    void Tick();

    [[nodiscard]] int GetSeconds() const;
    [[nodiscard]] int GetRemainingTicks() const;

private:
    int m_ticks = kStartSeconds * kTicksPerDigit;
};

struct MissionResultsView
{
    bool    bBoxVisible  = false;
    uint8_t boxAlpha     = 0;
    int     rescued      = 0;
    bool    bCounting    = false;
    int     bonusScore   = 0;
    bool    bRankVisible = false;
    String  rankText     = {};
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
    String       text  = {};
    int          x     = 0;
    int          y     = 0;
    int          w     = 0;
    int          h     = 0;
    int          scale = 1;
    TextStyle    style = {};
    RGBA         color = { 255, 255, 255, 255 };
};

[[nodiscard]] Vector<HudItem> BuildHudItems(const HudModel& _model, const UiLayout& _layout);

[[nodiscard]] bool IsBlinkOn(
    const int _tick,
    const int _periodTicks);

[[nodiscard]] String FormatScore(int _score);
[[nodiscard]] String FormatAmmo(int _ammo);
[[nodiscard]] String FormatBombs(int _bombs);
[[nodiscard]] String FormatTimer(int _seconds);
