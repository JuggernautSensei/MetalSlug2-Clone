#pragma once
#include <JugX/Result.h>
#include <JugX/Scoped.h>
#include <JugX/Typedef.h>

#include "Audio/AudioMath.h"
#include "Audio/AudioSink.h"
#include "Audio/SoundBank.h"
#include "Audio/VoiceAllocator.h"

namespace slug
{

// ===========================================
//  AudioConfig / AudioStats
// ===========================================

struct AudioConfig
{
    bool     bEnabled      = true;    // false: 장치를 열지 않는다 (요청 처리/통계만 수행하는 dry 모드)
    int      maxVoices     = 16;      // 동시에 울리는 효과음 상한
    int      maxPerSound   = 4;       // 같은 소리의 동시 상한 (권총 연사 등)
    float    masterVolume  = 1.f;
    float    sfxVolume     = 1.f;
    float    bgmVolume     = 0.6f;
    uint32_t pitchSeed     = 0x9E3779B9u;   // 피치 변동용 오디오 전용 RNG (시뮬레이션 RNG 와 무관)
};

struct AudioStats
{
    int playRequests = 0;   // Play 호출 수
    int started      = 0;   // 실제로 시작된(또는 dry 모드에서 슬롯을 얻은) 소리
    int stolen       = 0;   // 상한 때문에 끊긴 소리
    int missing      = 0;   // 뱅크에 없는 이름 요청
    int bgmStarts    = 0;
    int preloaded    = 0;   // 시작 시 메모리에 디코딩된 효과음 수 (장치가 있을 때만)
    int loadFailed   = 0;   // 디코딩에 실패한 효과음 수
};

// ===========================================
//  AudioSystem
//   miniaudio(ma_engine) 기반 IAudioSink 구현.
//    - 짧은 SFX 는 시작 시 메모리에 디코딩해 두고(리소스 매니저 등록), BGM 은 스트리밍한다. wav/mp3/ogg(stb_vorbis) 지원.
//    - 장치가 없거나 bEnabled == false 면 "dry 모드": 뱅크/보이스 정책/통계는 그대로 동작하고 소리만 나지 않는다. 항상 안전하다.
//    - Update(dt) 는 렌더 프레임마다(시뮬레이션 틱이 아니라) 한 번 불러 끝난 소리를 회수하고 BGM 페이드를 진행한다.
// ===========================================

class AudioSystem final : public IAudioSink
{
public:
    AudioSystem();
    ~AudioSystem() override;

    AudioSystem(const AudioSystem&)            = delete;
    AudioSystem& operator=(const AudioSystem&) = delete;

    // 뱅크를 직접 주입 (테스트/도구). 장치 초기화 실패는 오류가 아니라 dry 모드로 내려간다.
    [[nodiscard]] jug::Result<void> Init(SoundBank _bank, const AudioConfig& _config);
    // <bakedRoot>/sounds.json 을 읽는다. 파일이 없으면 오류(호출자가 NullAudio 로 대체 가능).
    [[nodiscard]] jug::Result<void> InitFromDirectory(const jug::FilePath& _bakedRoot, const AudioConfig& _config);
    void                            Shutdown();

    void Update(float _dtSec);

    // --- IAudioSink ---
    void Play(jug::StringView _name, float _volume = 1.f, float _pitchVariation = 0.f) override;
    void PlayBgm(jug::StringView _name, bool _bLoop = true) override;
    void StopBgm() override;
    void FadeOutBgm(float _seconds) override;
    void AllStop() override;

    // --- 볼륨 ---
    void               SetMasterVolume(float _volume);
    void               SetSfxVolume(float _volume);
    void               SetBgmVolume(float _volume);
    [[nodiscard]] float GetMasterVolume() const { return m_masterVolume; }
    [[nodiscard]] float GetSfxVolume() const { return m_sfxVolume; }
    [[nodiscard]] float GetBgmVolume() const { return m_bgmVolume; }

    // --- 조회 ---
    [[nodiscard]] bool               IsInitialized() const { return m_bInitialized; }
    [[nodiscard]] bool               HasDevice() const { return m_bDevice; }
    [[nodiscard]] const SoundBank&   GetBank() const { return m_bank; }
    [[nodiscard]] const AudioStats&  GetStats() const { return m_stats; }
    [[nodiscard]] int                GetActiveVoiceCount() const { return m_voices.GetActiveCount(); }
    [[nodiscard]] const VoiceAllocator& GetVoices() const { return m_voices; }
    [[nodiscard]] bool               IsBgmPlaying() const { return m_bgmState != eBgmState::Stopped; }
    [[nodiscard]] const jug::String& GetBgmName() const { return m_bgmName; }
    [[nodiscard]] float              GetBgmGain() const { return m_bgmFade.gain; }

private:
    struct Backend;

    enum class eBgmState : uint8_t
    {
        Stopped,
        Playing,
        FadingOut
    };

    // dry 모드에서 보이스가 점유하는 시간 (실제 길이를 모르므로 고정)
    static constexpr float kDryVoiceSeconds = 0.5f;

    void StopBgm_();
    void ApplyBgmVolume_();
    void StopVoice_(int _slot);

    bool          m_bInitialized = false;
    bool          m_bDevice      = false;
    SoundBank     m_bank         = {};
    AudioStats    m_stats        = {};
    VoiceAllocator m_voices      = {};
    jug::Vector<float> m_dryRemaining = {};   // dry 모드 보이스 남은 시간
    AudioRng      m_rng          = AudioRng {};
    jug::RbSet<jug::String> m_warnedMissing = {};

    float m_masterVolume = 1.f;
    float m_sfxVolume    = 1.f;
    float m_bgmVolume    = 0.6f;

    eBgmState   m_bgmState = eBgmState::Stopped;
    jug::String m_bgmName  = {};
    bool        m_bBgmLoop = true;
    BgmFader    m_bgmFade  = {};

    jug::Scoped<Backend> m_pBackend = nullptr;
};

}   // namespace slug
