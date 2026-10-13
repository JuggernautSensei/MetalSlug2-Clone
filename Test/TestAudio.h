// 테스트용 오디오 도우미: AudioSystem(bRecord=true, 장치 없음)이 기록한 요청을 센다.
#pragma once
#include "Audio/AudioSystem.h"
#include "Base/Types.h"

// 전역 오디오(장치 없음)를 기록 모드로 켜고 이벤트를 비운다. 테스트가 끝나면 Main.cpp 의 리스너가 되돌린다.
[[nodiscard]] AudioSystem& RecordAudio();
[[nodiscard]] int    CountPlays(const AudioSystem& _audio, StringView _name);
[[nodiscard]] int    CountKind(const AudioSystem& _audio, eAudioEvent _kind);
[[nodiscard]] String LastBgm(const AudioSystem& _audio);
