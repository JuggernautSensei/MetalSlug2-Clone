#include "Game/Ui/Banner.h"

#include <algorithm>
#include <cmath>

#include "Base/Types.h"

namespace slug
{

namespace
{
    constexpr int kMissionFlyDistance = 170;   // 진입/퇴장 시작 거리 (px)
    constexpr int kMissionStagger     = 2;     // 글자 사이 진입 지연 (틱)
    constexpr int kMissionOutStagger  = 1;     // 글자 사이 퇴장 지연 (틱)

    // 글자마다 고정된 날아오는 방향 (상, 하, 좌, 우 순환)
    constexpr ARRAY<int, 4> kDirX = { 0, 0, -1, 1 };
    constexpr ARRAY<int, 4> kDirY = { -1, 1, 0, 0 };
}   // namespace

BannerSpec MakeBannerSpec(
    const StringView _text,
    const float      _seconds)
{
    constexpr int kTicksPerSec = 60;

    const int total = std::max(1, static_cast<int>(std::lround(_seconds * static_cast<float>(kTicksPerSec))));
    const int fade  = std::min(18, total / 4);   // 0.3초 또는 총길이의 1/4 중 작은 쪽

    BannerSpec spec;
    spec.text         = String { _text };
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

GlyphPose ComputeMissionGlyphPose(
    const int _elapsed,
    const int _fadeIn,
    const int _hold,
    const int _fadeOut,
    const int _index)
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
        const int delay = std::min(_index * kMissionStagger, std::max(0, _fadeIn / 2));
        const int span  = std::max(1, _fadeIn - delay);
        if (_elapsed < delay)
        {
            pose.dx    = kDirX[dir] * kMissionFlyDistance;
            pose.dy    = kDirY[dir] * kMissionFlyDistance;
            pose.alpha = 0;
            return pose;
        }
        const float t = std::min(1.f, static_cast<float>(_elapsed - delay) / static_cast<float>(span));
        const float k = 1.f - (1.f - t) * (1.f - t) * (1.f - t);   // easeOutCubic
        pose.dx       = static_cast<int>(std::lround(static_cast<float>(kDirX[dir] * kMissionFlyDistance) * (1.f - k)));
        pose.dy       = static_cast<int>(std::lround(static_cast<float>(kDirY[dir] * kMissionFlyDistance) * (1.f - k)));
        pose.alpha    = static_cast<uint8_t>(std::clamp(t, 0.f, 1.f) * 255.f + 0.5f);
        return pose;
    }

    if (_elapsed < outAt)
    {
        pose.alpha = 255;   // 정지 상태 (dx/dy 0)
        return pose;
    }

    const int delay = std::min(_index * kMissionOutStagger, std::max(0, _fadeOut / 2));
    const int span  = std::max(1, _fadeOut - delay);
    const int local = _elapsed - outAt - delay;
    if (local <= 0)
    {
        pose.alpha = 255;
        return pose;
    }
    const float t = std::min(1.f, static_cast<float>(local) / static_cast<float>(span));
    const float k = t * t * t;   // easeInCubic
    pose.dx       = static_cast<int>(std::lround(static_cast<float>(kDirX[dir] * kMissionFlyDistance) * k));
    pose.dy       = static_cast<int>(std::lround(static_cast<float>(kDirY[dir] * kMissionFlyDistance) * k));
    pose.alpha    = static_cast<uint8_t>(std::clamp(1.f - t, 0.f, 1.f) * 255.f + 0.5f);
    return pose;
}

String MakeMissionGlyphName(
    const char _ch)
{
    String     name = "UI_Mission_";
    const char up   = (_ch >= 'a' && _ch <= 'z') ? static_cast<char>(_ch - 'a' + 'A') : _ch;
    if ((up >= 'A' && up <= 'Z') || (up >= '1' && up <= '9'))
    {
        name.push_back(up);
    }
    else if (up == '0')
    {
        name.push_back('O');   // 원본 폰트에 0 이 없으므로 O 로 대신한다 (BitmapFont 와 같은 규칙)
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

size_t BannerSystem::GetQueuedCount() const
{
    return m_queue.size();
}

int BannerSpec::GetTotalTicks() const
{
    return fadeInTicks + holdTicks + fadeOutTicks;
}

}   // namespace slug
