#include "TestAudio.h"

void TestAudio::Play(
    const StringView _name,
    const float      _volume,
    const float      _pitchVariation)
{
    m_events.push_back({ eAudioEvent::Play, String { _name }, _volume, _pitchVariation, false, 0.f });
}

void TestAudio::PlayBgm(
    const StringView _name,
    const bool       _bLoop)
{
    m_events.push_back({ eAudioEvent::PlayBgm, String { _name }, 1.f, 0.f, _bLoop, 0.f });
    m_bgm = String { _name };
}

void TestAudio::StopBgm()
{
    m_events.push_back({ eAudioEvent::StopBgm, {}, 1.f, 0.f, false, 0.f });
    m_bgm.clear();
}

void TestAudio::FadeOutBgm(
    const float _seconds)
{
    m_events.push_back({ eAudioEvent::FadeOutBgm, {}, 1.f, 0.f, false, _seconds });
}

void TestAudio::AllStop()
{
    m_events.push_back({ eAudioEvent::AllStop, {}, 1.f, 0.f, false, 0.f });
    m_bgm.clear();
}

void TestAudio::ChangeBgm(
    const StringView _name,
    const float      _fadeOutSec,
    const bool       _bLoop)
{
    m_events.push_back({ eAudioEvent::ChangeBgm, String { _name }, 1.f, 0.f, _bLoop, _fadeOutSec });
    m_bgm = String { _name };
}

const Vector<AudioEvent>& TestAudio::GetEvents() const
{
    return m_events;
}

const String& TestAudio::GetCurrentBgm() const
{
    return m_bgm;
}

void TestAudio::Clear()
{
    m_events.clear();
}

int TestAudio::CountPlays(
    const StringView _name) const
{
    int count = 0;
    for (const AudioEvent& e: m_events)
    {
        count += (e.kind == eAudioEvent::Play && e.name == _name) ? 1 : 0;
    }
    return count;
}

int TestAudio::CountKind(
    const eAudioEvent _kind) const
{
    int count = 0;
    for (const AudioEvent& e: m_events)
    {
        count += e.kind == _kind ? 1 : 0;
    }
    return count;
}

int TestAudio::GetPlayCount() const
{
    return CountKind(eAudioEvent::Play);
}
