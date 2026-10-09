#pragma once
#include "Base/Result.h"
#include <memory>
#include "Base/Types.h"

#include "Audio/AudioMath.h"
#include "Audio/AudioSink.h"
#include "Audio/SoundBank.h"
#include "Audio/VoiceAllocator.h"

namespace slug
{
struct AudioConfig
{
    bool     bEnabled      = true;
    int      maxVoices     = 16;      // 동시에 울리는 효과음 상한
    int      maxPerSound   = 4;       // 같은 소리의 동시 상한 (권총 연사 등)
    float    masterVolume  = 1.f;
    float    sfxVolume     = 1.f;
    float    bgmVolume     = 0.6f;
    uint32_t pitchSeed     = 0x9E3779B9u;   // 피치 변동용 오디오 전용 RNG (시뮬레이션 RNG 와 무관)
};

class AudioSystem final : public IAudioSink
{
public:
    AudioSystem();
    ~AudioSystem() override;

    AudioSystem(const AudioSystem&)            = delete;
    AudioSystem& operator=(const AudioSystem&) = delete;

    [[nodiscard]] slug::Result<void> Init(SoundBank _bank, const AudioConfig& _config);
    [[nodiscard]] slug::Result<void> InitFromDirectory(const slug::FilePath& _bakedRoot, const AudioConfig& _config);
    void                            Shutdown();

    void Update(float _dtSec);

    void Play(slug::StringView _name, float _volume = 1.f, float _pitchVariation = 0.f) override;
    void PlayBgm(slug::StringView _name, bool _bLoop = true) override;
    void StopBgm() override;
    void FadeOutBgm(float _seconds) override;
    void AllStop() override;

    [[nodiscard]] bool IsInitialized() const { return m_bInitialized; }

private:
    struct Backend;

    enum class eBgmState : uint8_t
    {
        Stopped,
        Playing,
        FadingOut
    };

    static constexpr float kDryVoiceSeconds = 0.5f;

    void StopBgm_();
    void ApplyBgmVolume_();
    void StopVoice_(int _slot);

    bool          m_bInitialized = false;
    bool          m_bDevice      = false;
    SoundBank     m_bank         = {};
    VoiceAllocator m_voices      = {};
    slug::Vector<float> m_dryRemaining = {};
    AudioRng      m_rng          = AudioRng {};
    slug::RbSet<slug::String> m_warnedMissing = {};

    float m_masterVolume = 1.f;
    float m_sfxVolume    = 1.f;
    float m_bgmVolume    = 0.6f;

    eBgmState   m_bgmState = eBgmState::Stopped;
    slug::String m_bgmName  = {};
    bool        m_bBgmLoop = true;
    BgmFader    m_bgmFade  = {};

    std::unique_ptr<Backend> m_pBackend = nullptr;
};
}
