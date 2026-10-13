#include "Game/Ui/Banner.h"

#include <algorithm>
#include <cmath>

#include "Base/Types.h"

constexpr int kMissionFlyDistance = 170;
constexpr int kMissionStagger     = 2;   // 글자 사이 진입 지연
constexpr int kMissionOutStagger  = 1;   // 글자 사이 퇴장 지연

constexpr ARRAY<int, 4> kDirX = { 0, 0, -1, 1 };
constexpr ARRAY<int, 4> kDirY = { -1, 1, 0, 0 };

BannerSpec MakeBannerSpec(
    StringView _text,
    float      _seconds)
{
    constexpr int kTicksPerSec = 60;

    const int total = Max(1, static_cast<int>(std::lround(_seconds * static_cast<float>(kTicksPerSec))));
    const int fade  = Min(18, total / 4);

    BannerSpec spec;
    spec.text         = String { _text };
    spec.fadeInTicks  = fade;
    spec.fadeOutTicks = fade;
    spec.holdTicks    = total - 2 * fade;
    return spec;
}

float ComputeBannerAlpha(
    int _elapsed,
    int _fadeIn,
    int _hold,
    int _fadeOut)
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

GlyphPose ComputeMissionGlyphPose(
    int _elapsed,
    int _fadeIn,
    int _hold,
    int _fadeOut,
    int _index)
{
    const int dir   = ((_index % 4) + 4) % 4;
    const int outAt = _fadeIn + _hold;
    const int total = outAt + _fadeOut;

    GlyphPose pose;
    if (_elapsed < 0 || _elapsed >= total)
    {
        pose.dx    = kDirX[dir] * kMissionFlyDistance;
        pose.dy    = kDirY[dir] * kMissionFlyDistance;
        pose.alpha = 0;
        return pose;
    }

    if (_elapsed < _fadeIn)
    {
        const int delay = Min(_index * kMissionStagger, Max(0, _fadeIn / 2));
        const int span  = Max(1, _fadeIn - delay);
        if (_elapsed < delay)
        {
            pose.dx    = kDirX[dir] * kMissionFlyDistance;
            pose.dy    = kDirY[dir] * kMissionFlyDistance;
            pose.alpha = 0;
            return pose;
        }
        const float t = Min(static_cast<float>(_elapsed - delay) / static_cast<float>(span), 1.f);
        const float k = 1.f - (1.f - t) * (1.f - t) * (1.f - t);
        pose.dx       = static_cast<int>(std::lround(static_cast<float>(kDirX[dir] * kMissionFlyDistance) * (1.f - k)));
        pose.dy       = static_cast<int>(std::lround(static_cast<float>(kDirY[dir] * kMissionFlyDistance) * (1.f - k)));
        pose.alpha    = static_cast<uint8_t>(Clamp(t, 0.f, 1.f) * 255.f + 0.5f);
        return pose;
    }

    if (_elapsed < outAt)
    {
        pose.alpha = 255;
        return pose;
    }

    const int delay = Min(_index * kMissionOutStagger, Max(0, _fadeOut / 2));
    const int span  = Max(1, _fadeOut - delay);
    const int local = _elapsed - outAt - delay;
    if (local <= 0)
    {
        pose.alpha = 255;
        return pose;
    }
    const float t = Min(static_cast<float>(local) / static_cast<float>(span), 1.f);
    const float k = t * t * t;
    pose.dx       = static_cast<int>(std::lround(static_cast<float>(kDirX[dir] * kMissionFlyDistance) * k));
    pose.dy       = static_cast<int>(std::lround(static_cast<float>(kDirY[dir] * kMissionFlyDistance) * k));
    pose.alpha    = static_cast<uint8_t>(Clamp(1.f - t, 0.f, 1.f) * 255.f + 0.5f);
    return pose;
}

String MakeMissionGlyphName(
    char _ch)
{
    String     name = "UI_Mission_";
    const char up   = (_ch >= 'a' && _ch <= 'z') ? static_cast<char>(_ch - 'a' + 'A') : _ch;
    if ((up >= 'A' && up <= 'Z') || (up >= '1' && up <= '9'))
    {
        name.push_back(up);
    }
    else if (up == '0')
    {
        name.push_back('O');
    }
    else if (up == '!')
    {
        name += "Excl";
    }
    else if (up == '?')
    {
        name += "Question";
    }
    else
    {
        name.clear();
    }
    return name;
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
    Begin_(std::move(_spec));
}

void BannerSystem::Clear()
{
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

bool BannerSystem::IsActive() const
{
    return m_bActive;
}

int BannerSystem::GetElapsedTicks() const
{
    return m_elapsed;
}

const BannerSpec& BannerSystem::GetCurrent() const
{
    return m_current;
}

int BannerSpec::GetTotalTicks() const
{
    return fadeInTicks + holdTicks + fadeOutTicks;
}
