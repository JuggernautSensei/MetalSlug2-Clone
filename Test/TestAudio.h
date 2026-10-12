// 테스트용 IAudioSink: 재생 요청을 실제로 소리내지 않고 순서대로 기록한다.
#pragma once
#include "Audio/AudioSink.h"
#include "Base/Types.h"

enum class eAudioEvent : uint8_t
{
    Play,
    PlayBgm,
    StopBgm,
    FadeOutBgm,
    AllStop,
    ChangeBgm
};

struct AudioEvent
{
    eAudioEvent kind           = eAudioEvent::Play;
    String      name           = {};
    float       volume         = 1.f;   // Play
    float       pitchVariation = 0.f;   // Play
    bool        bLoop          = false;
    float       seconds        = 0.f;   // FadeOutBgm
};

class TestAudio final : public IAudioSink
{
public:
    void Play(
        const StringView _name,
        const float      _volume         = 1.f,
        const float      _pitchVariation = 0.f) override;

    void PlayBgm(
        const StringView _name,
        const bool       _bLoop = true) override;

    void StopBgm() override;

    void FadeOutBgm(
        const float _seconds) override;

    void AllStop() override;

    void ChangeBgm(
        const StringView _name,
        const float      _fadeOutSec,
        const bool       _bLoop = true) override;

    [[nodiscard]] const Vector<AudioEvent>& GetEvents() const;
    [[nodiscard]] const String&             GetCurrentBgm() const;

    void Clear();

    [[nodiscard]] int CountPlays(
        const StringView _name) const;

    [[nodiscard]] int CountKind(
        const eAudioEvent _kind) const;

    [[nodiscard]] int GetPlayCount() const;

private:
    Vector<AudioEvent> m_events = {};
    String             m_bgm    = {};
};
