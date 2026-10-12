#include "Audio/AudioSink.h"

void NullAudio::Play(StringView, float, float)
{
}

void NullAudio::PlayBgm(StringView, bool)
{
}

void NullAudio::StopBgm()
{
}

void NullAudio::FadeOutBgm(float)
{
}

void NullAudio::AllStop()
{
}

void NullAudio::ChangeBgm(StringView, float, bool)
{
}

NullAudio& NullAudio::Get()
{
    static NullAudio s_null;
    return s_null;
}
