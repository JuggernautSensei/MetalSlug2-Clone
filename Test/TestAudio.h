// 테스트용 오디오 도우미: AudioSystem(bRecord=true, 장치 없음)이 기록한 요청을 센다.
#pragma once
#include "Audio/AudioSystem.h"
#include "Base/Types.h"

[[nodiscard]] int    CountPlays(const AudioSystem& _audio, StringView _name);
[[nodiscard]] int    CountKind(const AudioSystem& _audio, eAudioEvent _kind);
[[nodiscard]] String LastBgm(const AudioSystem& _audio);
