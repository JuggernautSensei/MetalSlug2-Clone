#pragma once
#include <algorithm>
#include <cstdint>

[[nodiscard]] float ComputePitch(
    const float _variation,
    const float _unit01);

class AudioRng
{
public:
    explicit AudioRng(const uint32_t _seed = 0x9E3779B9u);

    [[nodiscard]] uint32_t NextU32();

    [[nodiscard]] float NextUnit();

private:
    uint32_t m_state;
};

struct BgmFader
{
    float gain       = 1.f;
    float ratePerSec = 0.f;   // 음수: 페이드 아웃 중
    bool  bFading    = false;

    void Reset();

    void BeginFadeOut(const float _seconds);

    [[nodiscard]] bool Update(const float _dtSec);

    [[nodiscard]] bool IsDone() const;
};
