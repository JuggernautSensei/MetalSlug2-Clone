#pragma once
#include <algorithm>
#include <cstdint>

namespace slug
{
[[nodiscard]] constexpr float ComputePitch(
    const float _variation,
    const float _unit01)
{
    const float v = std::clamp(_variation, 0.f, 0.9f);
    return 1.f + v * (_unit01 * 2.f - 1.f);
}

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

    [[nodiscard]] float NextUnit()
    {
        return static_cast<float>(NextU32() >> 8) * (1.f / 16777216.f);
    }

private:
    uint32_t m_state;
};

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
}
