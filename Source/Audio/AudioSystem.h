#pragma once
#include <memory>

#include "Audio/AudioMath.h"
#include "Audio/AudioSink.h"
#include "Audio/SoundBank.h"
#include "Audio/VoiceAllocator.h"
#include "Base/Result.h"
#include "Base/Types.h"

class AsyncLoader;

struct AudioConfig
{
    bool         bEnabled = true;
    AsyncLoader* pLoader  = nullptr;   // 있으면 효과음 등록(디코드)을 워커 스레드에서 한다
    int      maxVoices    = 16;   // 동시에 울리는 효과음 상한
    int      maxPerSound  = 4;    // 같은 소리의 동시 상한 (권총 연사 등)
    float    masterVolume = 1.f;
    float    sfxVolume    = 1.f;
    float    bgmVolume    = 0.6f;
    uint32_t pitchSeed    = 0x9E3779B9u;   // 피치 변동용 오디오 전용 RNG (시뮬레이션 RNG 와 무관)
};

class AudioSystem final : public IAudioSink
{
public:
    AudioSystem();
    ~AudioSystem() override;

    AudioSystem(const AudioSystem&)            = delete;
    AudioSystem& operator=(const AudioSystem&) = delete;

    [[nodiscard]] Result<void> Init(SoundBank _bank, const AudioConfig& _config);
    [[nodiscard]] Result<void> InitFromDirectory(const FilePath& _bakedRoot, const AudioConfig& _config);
    void                       Shutdown();

    void Update(float _dtSec);

    void Play(StringView _name, float _volume = 1.f, float _pitchVariation = 0.f) override;
    void PlayBgm(StringView _name, bool _bLoop = true) override;
    void StopBgm() override;
    void FadeOutBgm(float _seconds) override;
    void AllStop() override;
    void ChangeBgm(StringView _name, float _fadeOutSec, bool _bLoop = true) override;

    [[nodiscard]] bool IsInitialized() const;
    [[nodiscard]] bool IsBgmFadingOut() const;
    [[nodiscard]] const String& GetCurrentBgm() const;   // 재생 중(페이드 중 포함)인 BGM 이름, 없으면 빈 문자열

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

    bool           m_bInitialized  = false;
    bool           m_bDevice       = false;
    SoundBank      m_bank          = {};
    VoiceAllocator m_voices        = {};
    Vector<float>  m_dryRemaining  = {};
    AudioRng       m_rng           = AudioRng {};
    RbSet<String>  m_warnedMissing = {};

    float m_masterVolume = 1.f;
    float m_sfxVolume    = 1.f;
    float m_bgmVolume    = 0.6f;

    eBgmState m_bgmState = eBgmState::Stopped;
    String    m_bgmName  = {};
    bool      m_bBgmLoop = true;
    BgmFader  m_bgmFade  = {};

    String m_pendingBgm   = {};   // 페이드 아웃이 끝나면 재생할 BGM
    bool   m_bPendingLoop = true;

    std::unique_ptr<Backend> m_pBackend;
};
