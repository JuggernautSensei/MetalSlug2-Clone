#pragma once
#include <algorithm>
#include <cstdint>

namespace slug
{

// ===========================================
//  오디오 순수 계산 (장치 없이 테스트 가능)
// ===========================================

// 피치 배율. _variation = 0.1 이면 [0.9, 1.1]. _unit01 은 [0,1) 난수 (시뮬레이션 RNG 가 아닌 오디오 전용 RNG 를 쓴다).
[[nodiscard]] constexpr float ComputePitch(
    const float _variation,
    const float _unit01)
{
    const float v = std::clamp(_variation, 0.f, 0.9f);
    return 1.f + v * (_unit01 * 2.f - 1.f);
}

// 오디오 전용 xorshift RNG. 시뮬레이션 RNG 와 완전히 분리되어 있어 소리 요청이 결정성에 영향을 주지 않는다.
class AudioRng
{
public:
    explicit AudioRng(const uint32_t _seed = 0x9E3779B9u)
        : m_state(_seed == 0 ? 1u : _seed)
    {
    }

    [[nodiscard]] uint32_t NextU32()
    {
        uint32_t x = m_state;
        x ^= x << 13;
        x ^= x >> 17;
        x ^= x << 5;
        m_state = x;
        return x;
    }

    // [0, 1)
    [[nodiscard]] float NextUnit()
    {
        return static_cast<float>(NextU32() >> 8) * (1.f / 16777216.f);
    }

private:
    uint32_t m_state;
};

// BGM 페이드 아웃 이득 (선형). 0 에 닿으면 IsDone.
struct BgmFader
{
    float gain       = 1.f;
    float ratePerSec = 0.f;   // 음수: 페이드 아웃 중
    bool  bFading    = false;

    void Reset()
    {
        gain       = 1.f;
        ratePerSec = 0.f;
        bFading    = false;
    }

    // _seconds <= 0 이면 즉시 0
    void BeginFadeOut(const float _seconds)
    {
        if (_seconds <= 0.f)
        {
            gain       = 0.f;
            ratePerSec = 0.f;
        }
        else
        {
            ratePerSec = -gain / _seconds;
        }
        bFading = true;
    }

    // 진행 후 페이드가 방금 끝났으면 true
    [[nodiscard]] bool Update(const float _dtSec)
    {
        if (!bFading)
        {
            return false;
        }
        gain = std::max(0.f, gain + ratePerSec * _dtSec);
        return gain <= 0.f;
    }

    [[nodiscard]] bool IsDone() const { return bFading && gain <= 0.f; }
};

}   // namespace slug
