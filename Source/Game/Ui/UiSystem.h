#pragma once
#include <cstdint>

#include "Base/Types.h"
#include "Game/GameState.h"
#include "Game/Ui/Banner.h"
#include "Game/Ui/BitmapFont.h"
#include "Game/Ui/FadeController.h"
#include "Game/Ui/Hud.h"
#include "Game/Ui/UiLayout.h"
#include "Render/SpriteLibrary.h"

// 게임 오버 전면 아트 (UI_GameOver_* 스프라이트). 화면을 덮도록 확대해 가운데 기준으로 그린다.
struct GameOverArtView
{
    bool    bVisible = false;
    uint8_t alpha    = 0;
    String  sprite   = {};
};

class UiSystem
{
public:
    void Init(const GameContext& _ctx);

    void Tick();

    void DrawHud(GameContext& _ctx, const HudModel& _model);
    void DrawBanner(GameContext& _ctx);
    void DrawMissionResults(GameContext& _ctx) const;
    void DrawFade(GameContext& _ctx);
    void DrawOverlays(GameContext& _ctx);

    void DrawText(
        GameContext&     _ctx,
        int              _x,
        int              _y,
        StringView       _text,
        const TextStyle& _style) const;
    bool DrawSprite(
        GameContext& _ctx,
        StringView   _name,
        int          _centerX,
        int          _centerY,
        int          _scale = 1,
        uint8_t      _alpha = 255) const;
    bool DrawStandalone(
        GameContext& _ctx,
        StringView   _name,
        int          _x,
        int          _y,
        uint8_t      _alpha = 255) const;

    [[nodiscard]] BannerSystem&         GetBanner();
    [[nodiscard]] const BannerSystem&   GetBanner() const;
    [[nodiscard]] FadeController&       GetFade();
    [[nodiscard]] const FadeController& GetFade() const;
    [[nodiscard]] const UiLayout&       GetLayout() const;
    [[nodiscard]] int                   GetTick() const;
    [[nodiscard]] bool                  HasSprites() const;

    void ShowBanner(StringView _text, float _seconds);
    void SetMissionResults(const MissionResultsView& _view);
    void SetGameOverArt(const GameOverArtView& _view);

private:
    void DrawGameOverArt_(GameContext& _ctx) const;

    UiLayout           m_layout   = {};
    BitmapFont         m_font     = {};
    SpriteLibrary      m_sprites  = {};
    BannerSystem       m_banner   = {};
    MissionResultsView m_results  = {};
    GameOverArtView    m_art      = {};
    FadeController     m_fade     = {};
    int                m_tick     = 0;
    bool               m_bSprites = false;
};
