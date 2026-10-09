#pragma once
#include <cstdint>

#include <JugX/Typedef.h>

#include "Game/Ui/BitmapFont.h"

namespace slug
{

// ===========================================
//  Banner
//   화면 중앙의 큰 글씨 (줄바꿈 지원). 고정 스텝 틱 단위 타임라인: 페이드 인 → 유지 → 페이드 아웃.
//   모든 게임 상태가 같은 틱 경로로 진행시키므로 헤드리스 테스트에서도 정확히 같은 틱에 알파가 정해진다.
// ===========================================

struct BannerSpec
{
    jug::String text          = {};
    int         fadeInTicks   = 18;
    int         holdTicks     = 84;
    int         fadeOutTicks  = 18;
    eFontColor  color         = eFontColor::Yellow;
    bool        bBold         = true;
    int         maxScale      = 3;     // 폭에 맞춰 줄이는 상한 배율
    int         centerY       = -1;    // <0 이면 UiLayout::bannerCenterY

    [[nodiscard]] int GetTotalTicks() const { return fadeInTicks + holdTicks + fadeOutTicks; }
};

// 총 길이(초)로 스펙을 만든다: 페이드 각각 min(0.3s, 총길이/4), 나머지는 유지 (StageFormat 의 banner {text, seconds}).
[[nodiscard]] BannerSpec MakeBannerSpec(jug::StringView _text, float _seconds);

// 경과 틱 e (Show 직후 0) 에서의 불투명도 [0,1].
//   e < in           : e / in            (e = 0 에서 0)
//   in <= e < in+hold: 1
//   그 뒤            : (total - e) / out (e = total 에서 0)
[[nodiscard]] float ComputeBannerAlpha(int _elapsed, int _fadeIn, int _hold, int _fadeOut);

class BannerSystem
{
public:
    // 현재 배너를 즉시 교체하고 대기열을 비운다.
    void Show(BannerSpec _spec);
    // 현재 배너가 끝난 뒤 이어서 보여준다 (배너가 없으면 즉시 시작).
    void Enqueue(BannerSpec _spec);
    void Clear();

    // 고정 스텝 한 틱 진행
    void Tick();

    [[nodiscard]] bool              IsActive() const { return m_bActive; }
    [[nodiscard]] int               GetElapsedTicks() const { return m_elapsed; }
    [[nodiscard]] float             GetAlpha() const;
    [[nodiscard]] const BannerSpec& GetCurrent() const { return m_current; }
    [[nodiscard]] size_t            GetQueuedCount() const { return m_queue.size(); }

private:
    void Begin_(BannerSpec _spec);

    BannerSpec              m_current = {};
    jug::Vector<BannerSpec> m_queue   = {};
    int                     m_elapsed = 0;
    bool                    m_bActive = false;
};

}   // namespace slug
