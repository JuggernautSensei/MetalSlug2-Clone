#include "TestAudio.h"

AudioSystem& RecordAudio()
{
    AudioSystem& audio = AudioSystem::GetSingleton();
    audio.bRecord      = true;
    audio.events.clear();
    return audio;
}

int CountPlays(
    const AudioSystem& _audio,
    const StringView   _name)
{
    int count = 0;
    for (const AudioEvent& e: _audio.events)
    {
        count += (e.kind == eAudioEvent::Play && e.name == _name) ? 1 : 0;
    }
    return count;
}

int CountKind(
    const AudioSystem& _audio,
    const eAudioEvent  _kind)
{
    int count = 0;
    for (const AudioEvent& e: _audio.events)
    {
        count += e.kind == _kind ? 1 : 0;
    }
    return count;
}

String LastBgm(
    const AudioSystem& _audio)
{
    String bgm;
    for (const AudioEvent& e: _audio.events)
    {
        switch (e.kind)
        {
            case eAudioEvent::PlayBgm:
            case eAudioEvent::ChangeBgm:
                bgm = e.name;
                break;
            case eAudioEvent::StopBgm:
            case eAudioEvent::AllStop:
                bgm.clear();
                break;
            default:
                break;
        }
    }
    return bgm;
}
