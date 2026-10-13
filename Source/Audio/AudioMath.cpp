#include "Audio/AudioMath.h"

AudioRng::AudioRng(const uint32_t _seed)
    : m_state(_seed == 0 ? 1u : _seed)
{
}

uint32_t AudioRng::NextU32()
{
    uint32_t x = m_state;
    x ^= x << 13;
    x ^= x >> 17;
    x ^= x << 5;
    m_state = x;
    return x;
}

float AudioRng::NextUnit()
{
    return static_cast<float>(NextU32() >> 8) * (1.f / 16777216.f);
}

void BgmFader::Reset()
{
    gain       = 1.f;
    ratePerSec = 0.f;
    bFading    = false;
}

void BgmFader::BeginFadeOut(
    const float _seconds)
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

bool BgmFader::Update(
    const float _dtSec)
{
    if (!bFading)
    {
        return false;
    }
    gain = Max(gain + ratePerSec * _dtSec, 0.f);
    return gain <= 0.f;
}

bool BgmFader::IsDone() const
{
    return bFading && gain <= 0.f;
}

float ComputePitch(
    const float _variation,
    const float _unit01)
{
    const float v = Clamp(_variation, 0.f, 0.9f);
    return 1.f + v * (_unit01 * 2.f - 1.f);
}
