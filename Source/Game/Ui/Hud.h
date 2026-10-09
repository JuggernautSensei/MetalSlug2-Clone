#pragma once
#include <cstdint>

#include "Base/RGBA.h"
#include "Base/Types.h"

#include "Game/Ui/BitmapFont.h"
#include "Game/Ui/UiLayout.h"

namespace slug
{

class PlayerController;

// ===========================================
//  HudModel
//   HUD 가 그리는 값만 담은 평범한 구조체 (게임 로직을 모른다 → 테스트/스테이지 러너가 직접 채울 수 있다).
//   플레이어 값은 MakeHudModel() 이 PlayerController 에서 채우고, 타이머/보스/레벨 같은 값은 스테이지가 채운다.
// ===========================================

struct HudModel
{
    // --- 1P ---
    bool bShowPlayer    = true;
    int  lives          = 0;      // "1UP X<lives>" (남은 목숨 수. 현재 목숨 포함)
    int  score          = 0;
    bool bInfiniteAmmo  = true;   // 권총: 무한대 기호
    int  ammo           = 0;      // 유한 무기의 남은 탄
    int  bombs          = 0;
    char weaponLetter   = 0;      // 든 무기 글자 (H/S/R/F). 권총은 0

    // --- 타이머 (스테이지가 채운다) ---
    bool bShowTimer     = false;
    int  timerSeconds   = 0;

    // --- 보스 HP 바 (스테이지가 채운다) ---
    bool  bShowBoss     = false;
    float bossHpRatio   = 1.f;    // 0..1

    // --- 기타 ---
    bool        bShowPressStart2p = true;   // 2P 칸에 PRESS START 점멸
    slug::String levelSprite       = {};     // "LEVEL_1" 같은 sprites.json 이름. 비면 표시 안 함
    int         uiTick            = 0;      // 점멸용 단조 증가 틱 (UiSystem::GetTick())
};

// PlayerController 에서 1P 값을 채운다 (나머지 필드는 기본값).
[[nodiscard]] HudModel MakeHudModel(const PlayerController& _player);

// ===========================================
//  HudItem / BuildHudItems
//   모델 + 레이아웃 → 그릴 항목 목록. 순수 함수라 렌더러 없이 배치를 검증할 수 있다.
//   좌표 규약: Text = (정렬 기준 x, 위쪽 y), Sprite = (중심 x, 중심 y), TimeDigits = (중심 x, 위쪽 y), Rect = 왼쪽 위.
// ===========================================

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
    slug::String  text  = {};      // Text/TimeDigits: 문자열, Sprite: sprites.json 이름
    int          x     = 0;
    int          y     = 0;
    int          w     = 0;       // Rect 크기
    int          h     = 0;
    int          scale = 1;       // Sprite 배율
    TextStyle    style = {};      // Text
    slug::RGBA    color = { 255, 255, 255, 255 };   // Rect
};

[[nodiscard]] slug::Vector<HudItem> BuildHudItems(const HudModel& _model, const UiLayout& _layout);

// 점멸: 켜짐 구간이면 true. 처음 _periodTicks 틱은 켜짐.
[[nodiscard]] constexpr bool IsBlinkOn(
    const int _tick,
    const int _periodTicks)
{
    return _periodTicks <= 0 || ((_tick / _periodTicks) % 2) == 0;
}

// 점수는 7자리까지만 표시 (레거시와 동일), 탄은 3자리, 폭탄은 2자리
[[nodiscard]] slug::String FormatScore(int _score);
[[nodiscard]] slug::String FormatAmmo(int _ammo);
[[nodiscard]] slug::String FormatBombs(int _bombs);
[[nodiscard]] slug::String FormatTimer(int _seconds);   // 00..99

}   // namespace slug
