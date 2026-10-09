#pragma once
#include <cstdint>

#include <JugX/Typedef.h>

#include "Game/GameState.h"
#include "Game/Ui/Banner.h"
#include "Game/Ui/BitmapFont.h"
#include "Game/Ui/FadeController.h"
#include "Game/Ui/Hud.h"
#include "Game/Ui/UiLayout.h"
#include "Render/SpriteLibrary.h"

namespace slug
{

// ===========================================
//  UiSystem
//   게임 상태 하나가 가지는 UI 묶음: 비트맵 폰트 + 배너 + 화면 페이드 + HUD 그리기.
//   타이밍(배너/페이드/점멸)은 렌더러 없이도 진행되므로 헤드리스 테스트가 그대로 검증한다.
//
//   사용 순서 (상태 안에서):
//     OnEnter : m_ui.Init(_ctx);
//     Tick    : m_ui.Tick();                       // 고정 스텝마다 한 번
//     Render  : (월드 그리기) → m_ui.DrawHud(_ctx, model) → m_ui.DrawOverlays(_ctx)   // 오버레이(페이드→배너)는 항상 마지막
// ===========================================

class UiSystem
{
public:
    // 렌더러가 있으면 <assetRoot>/Baked/sprites.json 에서 폰트/UI 스프라이트를 준비한다. 없어도 안전하다.
    void Init(const GameContext& _ctx);

    // 고정 스텝 한 틱: 배너/페이드 진행, 점멸 시계 증가
    void Tick();

    // --- 그리기 (렌더러가 없으면 아무것도 안 한다) ---
    void DrawHud(GameContext& _ctx, const HudModel& _model);
    void DrawBanner(GameContext& _ctx);
    void DrawFade(GameContext& _ctx);
    void DrawOverlays(GameContext& _ctx);   // 페이드 → 배너 (배너가 가장 위)

    void DrawText(GameContext& _ctx, int _x, int _y, jug::StringView _text, const TextStyle& _style) const;
    // sprites.json 의 정적 스프라이트를 중심 좌표에 _scale 배로 그린다. 없으면 false.
    bool DrawSprite(GameContext& _ctx, jug::StringView _name, int _centerX, int _centerY, int _scale = 1, uint8_t _alpha = 255) const;
    // 큰 독립 텍스처(예: TitleBackground)를 화면 좌표 (_x,_y) 에 그린다. 텍스처는 SpriteBatch 별로 한 번만 로드한다.
    bool DrawStandalone(GameContext& _ctx, jug::StringView _name, int _x, int _y, uint8_t _alpha = 255) const;

    // --- 접근 ---
    [[nodiscard]] BannerSystem&         GetBanner() { return m_banner; }
    [[nodiscard]] const BannerSystem&   GetBanner() const { return m_banner; }
    [[nodiscard]] FadeController&       GetFade() { return m_fade; }
    [[nodiscard]] const FadeController& GetFade() const { return m_fade; }
    [[nodiscard]] BitmapFont&           GetFont() { return m_font; }
    [[nodiscard]] const UiLayout&       GetLayout() const { return m_layout; }
    void                                SetLayout(const UiLayout& _layout) { m_layout = _layout; }
    [[nodiscard]] int                   GetTick() const { return m_tick; }
    [[nodiscard]] bool                  HasSprites() const { return m_bSprites; }

    // 편의: 스펙 없이 배너를 바로 띄운다
    void ShowBanner(jug::StringView _text, float _seconds) { m_banner.Show(MakeBannerSpec(_text, _seconds)); }

private:
    UiLayout      m_layout  = {};
    BitmapFont    m_font    = {};
    SpriteLibrary m_sprites = {};
    BannerSystem  m_banner  = {};
    FadeController m_fade   = {};
    int           m_tick    = 0;
    bool          m_bSprites = false;
};

}   // namespace slug
