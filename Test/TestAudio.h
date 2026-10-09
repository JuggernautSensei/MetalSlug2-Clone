#pragma once
#include "Audio/AudioSink.h"
#include "Base/Types.h"

namespace slug
{
enum class eAudioEvent : uint8_t
{
    Play,
    PlayBgm,
    StopBgm,
    FadeOutBgm,
    AllStop
};

struct AudioEvent
{
    eAudioEvent kind           = eAudioEvent::Play;
    slug::String name           = {};
    float       volume         = 1.f;   // Play
    float       pitchVariation = 0.f;   // Play
    bool        bLoop          = false;
    float       seconds        = 0.f;   // FadeOutBgm
};

class TestAudio final : public IAudioSink
{
public:
    void Play(const slug::StringView _name, const float _volume = 1.f, const float _pitchVariation = 0.f) override
    {
        m_events.push_back({ eAudioEvent::Play, slug::String { _name }, _volume, _pitchVariation, false, 0.f });
    }

    void PlayBgm(const slug::StringView _name, const bool _bLoop = true) override
    {
        m_events.push_back({ eAudioEvent::PlayBgm, slug::String { _name }, 1.f, 0.f, _bLoop, 0.f });
        m_bgm = slug::String { _name };
    }

    void StopBgm() override
    {
        m_events.push_back({ eAudioEvent::StopBgm, {}, 1.f, 0.f, false, 0.f });
        m_bgm.clear();
    }

    void FadeOutBgm(const float _seconds) override
    {
        m_events.push_back({ eAudioEvent::FadeOutBgm, {}, 1.f, 0.f, false, _seconds });
    }

    void AllStop() override
    {
        m_events.push_back({ eAudioEvent::AllStop, {}, 1.f, 0.f, false, 0.f });
        m_bgm.clear();
    }

    [[nodiscard]] const slug::Vector<AudioEvent>& GetEvents() const { return m_events; }
    [[nodiscard]] const slug::String&             GetCurrentBgm() const { return m_bgm; }
    void                                         Clear() { m_events.clear(); }

    [[nodiscard]] int CountPlays(const slug::StringView _name) const
    {
        int count = 0;
        for (const AudioEvent& e: m_events)
        {
            count += (e.kind == eAudioEvent::Play && e.name == _name) ? 1 : 0;
        }
        return count;
    }

    [[nodiscard]] int CountKind(const eAudioEvent _kind) const
    {
        int count = 0;
        for (const AudioEvent& e: m_events)
        {
            count += e.kind == _kind ? 1 : 0;
        }
        return count;
    }

    [[nodiscard]] int GetPlayCount() const { return CountKind(eAudioEvent::Play); }

private:
    slug::Vector<AudioEvent> m_events = {};
    slug::String             m_bgm    = {};
};
}
