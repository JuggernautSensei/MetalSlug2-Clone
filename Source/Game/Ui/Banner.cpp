#include "Game/Ui/Banner.h"
#include "Base/Types.h"

#include <algorithm>
#include <cmath>

namespace slug
{

BannerSpec MakeBannerSpec(
    const slug::StringView _text,
    const float           _seconds)
{
    constexpr int kTicksPerSec = 60;

    const int total = std::max(1, static_cast<int>(std::lround(_seconds * static_cast<float>(kTicksPerSec))));
    const int fade  = std::min(18, total / 4);   // 0.3초 또는 총길이의 1/4 중 작은 쪽

    BannerSpec spec;
    spec.text         = slug::String { _text };
    spec.fadeInTicks  = fade;
    spec.fadeOutTicks = fade;
    spec.holdTicks    = total - 2 * fade;
    return spec;
}

float ComputeBannerAlpha(
    const int _elapsed,
    const int _fadeIn,
    const int _hold,
    const int _fadeOut)
{
    const int total = _fadeIn + _hold + _fadeOut;
    if (_elapsed < 0 || _elapsed >= total)
    {
        return 0.f;
    }
    if (_elapsed < _fadeIn)
    {
        return static_cast<float>(_elapsed) / static_cast<float>(_fadeIn);
    }
    if (_elapsed < _fadeIn + _hold)
    {
        return 1.f;
    }
    return static_cast<float>(total - _elapsed) / static_cast<float>(_fadeOut);
}

void BannerSystem::Begin_(
    BannerSpec _spec)
{
    m_current = std::move(_spec);
    m_elapsed = 0;
    m_bActive = m_current.GetTotalTicks() > 0;
}

void BannerSystem::Show(
    BannerSpec _spec)
{
    m_queue.clear();
    Begin_(std::move(_spec));
}

void BannerSystem::Enqueue(
    BannerSpec _spec)
{
    if (!m_bActive)
    {
        Begin_(std::move(_spec));
        return;
    }
    m_queue.push_back(std::move(_spec));
}

void BannerSystem::Clear()
{
    m_queue.clear();
    m_bActive = false;
    m_elapsed = 0;
    m_current = {};
}

void BannerSystem::Tick()
{
    if (!m_bActive)
    {
        return;
    }

    ++m_elapsed;
    if (m_elapsed < m_current.GetTotalTicks())
    {
        return;
    }

    if (!m_queue.empty())
    {
        BannerSpec next = std::move(m_queue.front());
        m_queue.erase(m_queue.begin());
        Begin_(std::move(next));
        return;
    }

    m_bActive = false;
    m_elapsed = 0;
}

float BannerSystem::GetAlpha() const
{
    if (!m_bActive)
    {
        return 0.f;
    }
    return ComputeBannerAlpha(m_elapsed, m_current.fadeInTicks, m_current.holdTicks, m_current.fadeOutTicks);
}
}
