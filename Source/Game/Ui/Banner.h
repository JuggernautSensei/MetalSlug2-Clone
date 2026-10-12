#pragma once
#include <cstdint>

#include "Base/Types.h"
#include "Game/Ui/BitmapFont.h"

struct BannerSpec
{
    String     text         = {};
    int        fadeInTicks  = 18;
    int        holdTicks    = 84;
    int        fadeOutTicks = 18;
    eFontColor color        = eFontColor::Yellow;
    bool       bBold        = true;
    int        centerY      = -1;   // <0 이면 UiLayout::missionLine1Y

    [[nodiscard]] int GetTotalTicks() const;
};

[[nodiscard]] BannerSpec MakeBannerSpec(StringView _text, float _seconds);

[[nodiscard]] float ComputeBannerAlpha(int _elapsed, int _fadeIn, int _hold, int _fadeOut);

struct GlyphPose
{
    int     dx    = 0;
    int     dy    = 0;
    uint8_t alpha = 0;   // 0..255
};

[[nodiscard]] GlyphPose ComputeMissionGlyphPose(int _elapsed, int _fadeIn, int _hold, int _fadeOut, int _index);

// 미션 폰트 스프라이트 이름 (UI_Mission_A..Z, _1.._9, _Excl, _Question). 지원하지 않는 글자는 빈 문자열.
[[nodiscard]] String MakeMissionGlyphName(char _ch);

class BannerSystem
{
public:
    void Show(BannerSpec _spec);
    void Clear();

    void Tick();

    [[nodiscard]] bool              IsActive() const;
    [[nodiscard]] int               GetElapsedTicks() const;
    [[nodiscard]] float             GetAlpha() const;
    [[nodiscard]] const BannerSpec& GetCurrent() const;

private:
    void Begin_(BannerSpec _spec);

    BannerSpec         m_current = {};
    int                m_elapsed = 0;
    bool               m_bActive = false;
};
