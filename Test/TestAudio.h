#pragma once
#include "Audio/AudioSystem.h"
#include "Base/Types.h"

[[nodiscard]] AudioSystem& RecordAudio();
[[nodiscard]] int          CountPlays(const AudioSystem& _audio, StringView _name);
[[nodiscard]] int          CountKind(const AudioSystem& _audio, eAudioEvent _kind);
[[nodiscard]] String       LastBgm(const AudioSystem& _audio);
