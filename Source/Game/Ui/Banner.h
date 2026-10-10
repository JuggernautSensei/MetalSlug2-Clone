#pragma once
#include <cstdint>

#include "Base/Types.h"
#include "Game/Ui/BitmapFont.h"

namespace slug
{

struct BannerSpec
{
    String     text         = {};
    int        fadeInTicks  = 18;
    int        holdTicks    = 84;
    int        fadeOutTicks = 18;
    eFontColor color        = eFontColor::Yellow;
    bool       bBold        = true;
    int        maxScale     = 3;    // 폭에 맞춰 줄이는 상한 배율
    int        centerY      = -1;   // <0 이면 UiLayout::bannerCenterY

    [[nodiscard]] int GetTotalTicks() const;
};

[[nodiscard]] BannerSpec MakeBannerSpec(StringView _text, float _seconds);

[[nodiscard]] float ComputeBannerAlpha(
    int _elapsed,
    int _fadeIn,
    int _hold,
    int _fadeOut);

// 미션 폰트 글자 하나의 연출 상태. 진입 시 사방에서 날아와 자리에 앉고, 퇴장 시 날아가며 사라진다.
struct GlyphPose
{
    int     dx    = 0;   // 자리 기준 가로 오프셋 (px)
    int     dy    = 0;   // 자리 기준 세로 오프셋 (px)
    uint8_t alpha = 0;   // 0..255
};

// _index: 배너 안 글자 순번 (글자마다 방향과 지연이 달라진다). 페이드 길이는 BannerSpec 과 같다.
[[nodiscard]] GlyphPose ComputeMissionGlyphPose(
    int _elapsed,
    int _fadeIn,
    int _hold,
    int _fadeOut,
    int _index);

// 미션 폰트 스프라이트 이름 (UI_Mission_A..Z, _1.._9, _Excl, _Question). 지원하지 않는 글자는 빈 문자열.
[[nodiscard]] String MakeMissionGlyphName(char _ch);

class BannerSystem
{
public:
    void Show(BannerSpec _spec);
    void Enqueue(BannerSpec _spec);
    void Clear();

    void Tick();

    [[nodiscard]] bool              IsActive() const;
    [[nodiscard]] int               GetElapsedTicks() const;
    [[nodiscard]] float             GetAlpha() const;
    [[nodiscard]] const BannerSpec& GetCurrent() const;
    [[nodiscard]] size_t            GetQueuedCount() const;

private:
    void Begin_(BannerSpec _spec);

    BannerSpec         m_current = {};
    Vector<BannerSpec> m_queue   = {};
    int                m_elapsed = 0;
    bool               m_bActive = false;
};

}   // namespace slug
