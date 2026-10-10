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
    bool bShowPlayer   = true;
    int  lives         = 0;   // "1UP=<lives>" (남은 목숨 수. 현재 목숨 포함)
    int  score         = 0;
    bool bInfiniteAmmo = true;
    int  ammo          = 0;   // 유한 무기의 남은 탄
    int  bombs         = 0;
    char weaponLetter  = 0;   // 든 무기 글자 (H/S/R/F). 권총은 0

    bool bShowTimer   = false;
    int  timerSeconds = 0;

    bool  bShowBoss   = false;
    float bossHpRatio = 1.f;   // 0..1

    bool   bShowInsertCoin = true;   // 원본 "INSERT COIN" (우상단, 깜빡이지 않음)
    String levelSprite     = {};
    int    uiTick          = 0;   // 점멸용 단조 증가 틱 (UiSystem::GetTick())
};

// 화면 표시용 60초 카운트다운. 사망과 무관하다 (0 이 되어도 아무 일도 없음).
// 원본 영상에서 숫자는 1초가 아니라 약 6.5초마다 한 칸씩 줄고, 구간이 바뀌면 60 으로 돌아간다.
class DisplayTimer
{
public:
    static constexpr int kStartSeconds  = 60;
    static constexpr int kTicksPerDigit = 390;   // 6.5초 * 60틱

    void Reset();
    void Tick();

    [[nodiscard]] int GetSeconds() const;
    [[nodiscard]] int GetRemainingTicks() const;

private:
    int m_ticks = kStartSeconds * kTicksPerDigit;
};

// 결과 상자 한 프레임의 표시 상태 (순수 계산, UiSystem 이 그린다)
struct MissionResultsView
{
    bool    bBoxVisible  = false;
    uint8_t boxAlpha     = 0;
    int     rescued      = 0;       // 표시 중인 구출 수
    bool    bCounting    = false;   // "x10000" 라벨 표시 (집계 중)
    int     bonusScore   = 0;       // 표시 중인 보너스 점수
    bool    bRankVisible = false;
    String  rankText     = {};   // "GREAT!" 등
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
    int          w     = 0;   // Rect 크기
    int          h     = 0;
    int          scale = 1;   // Sprite 배율
    TextStyle    style = {};
    RGBA         color = { 255, 255, 255, 255 };   // Rect
};

[[nodiscard]] Vector<HudItem> BuildHudItems(const HudModel& _model, const UiLayout& _layout);

[[nodiscard]] constexpr bool IsBlinkOn(
    const int _tick,
    const int _periodTicks)
{
    return _periodTicks <= 0 || ((_tick / _periodTicks) % 2) == 0;
}

[[nodiscard]] String FormatScore(int _score);
[[nodiscard]] String FormatAmmo(int _ammo);
[[nodiscard]] String FormatBombs(int _bombs);
[[nodiscard]] String FormatTimer(int _seconds);

}   // namespace slug
