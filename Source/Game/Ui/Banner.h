#pragma once
#include <cstdint>

#include "Base/Types.h"

#include "Game/Ui/BitmapFont.h"

namespace slug
{
struct BannerSpec
{
    slug::String text          = {};
    int         fadeInTicks   = 18;
    int         holdTicks     = 84;
    int         fadeOutTicks  = 18;
    eFontColor  color         = eFontColor::Yellow;
    bool        bBold         = true;
    int         maxScale      = 3;     // 폭에 맞춰 줄이는 상한 배율
    int         centerY       = -1;    // <0 이면 UiLayout::bannerCenterY

    [[nodiscard]] int GetTotalTicks() const { return fadeInTicks + holdTicks + fadeOutTicks; }
};

[[nodiscard]] BannerSpec MakeBannerSpec(slug::StringView _text, float _seconds);

[[nodiscard]] float ComputeBannerAlpha(int _elapsed, int _fadeIn, int _hold, int _fadeOut);

class BannerSystem
{
public:
    void Show(BannerSpec _spec);
    void Enqueue(BannerSpec _spec);
    void Clear();

    void Tick();

    [[nodiscard]] bool              IsActive() const { return m_bActive; }
    [[nodiscard]] int               GetElapsedTicks() const { return m_elapsed; }
    [[nodiscard]] float             GetAlpha() const;
    [[nodiscard]] const BannerSpec& GetCurrent() const { return m_current; }
    [[nodiscard]] size_t            GetQueuedCount() const { return m_queue.size(); }

private:
    void Begin_(BannerSpec _spec);

    BannerSpec              m_current = {};
    slug::Vector<BannerSpec> m_queue   = {};
    int                     m_elapsed = 0;
    bool                    m_bActive = false;
};
}
