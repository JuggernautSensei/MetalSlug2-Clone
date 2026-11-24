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

struct GameOverArtView
{
    bool    bVisible = false;
    uint8_t alpha    = 0;
    String  sprite   = {};
};

class UiSystem
{
public:
    void                                Init();
    void                                Tick();
    void                                DrawHud(const HudModel& _model);
    void                                DrawBanner();
    void                                DrawMissionResults() const;
    void                                DrawFade();
    void                                DrawOverlays();
    void                                DrawString(int _x, int _y, StringView _text, const TextStyle& _style) const;
    bool                                DrawSprite(StringView _name, int _centerX, int _centerY, int _scale = 1, uint8_t _alpha = 255) const;
    bool                                DrawStandalone(StringView _name, int _x, int _y, uint8_t _alpha = 255) const;
    [[nodiscard]] BannerSystem&         GetBanner();
    [[nodiscard]] const BannerSystem&   GetBanner() const;
    [[nodiscard]] FadeController&       GetFade();
    [[nodiscard]] const FadeController& GetFade() const;
    [[nodiscard]] const UiLayout&       GetLayout() const;
    [[nodiscard]] int                   GetTick() const;
    [[nodiscard]] bool                  HasSprites() const;
    void                                ShowBanner(StringView _text, float _seconds);
    void                                SetMissionResults(const MissionResultsView& _view);
    void                                SetGameOverArt(const GameOverArtView& _view);

private:
    void DrawGameOverArt_() const;

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
