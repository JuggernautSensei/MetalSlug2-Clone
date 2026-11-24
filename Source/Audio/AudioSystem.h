#pragma once
#include <memory>

#include "Audio/AudioMath.h"
#include "Audio/SoundBank.h"
#include "Audio/VoiceAllocator.h"
#include "Base/SlugError.h"
#include "Base/Types.h"

inline constexpr const char* kPistolShot   = "PlayerGunShot";
inline constexpr const char* kFlameShot    = "PlayerGunShot_FS";
inline constexpr const char* kKnife1       = "PlayerSlash1";
inline constexpr const char* kKnife2       = "PlayerSlash2";
inline constexpr const char* kBombExplode  = "PlayerBomb";
inline constexpr const char* kPlayerDeath  = "PlayerGunDead";
inline constexpr const char* kPickupCoin   = "Coin";
inline constexpr const char* kOk           = "OK";
inline constexpr const char* kVoiceHmg     = "PlayerGun_HM";
inline constexpr const char* kVoiceShotgun = "PlayerGun_SG";
inline constexpr const char* kVoiceRocket  = "PlayerGun_RL";
inline constexpr const char* kVoiceFlame   = "PlayerGun_FS";
inline constexpr const char* kBgmStage     = "Stage1_BGM";

inline constexpr const char* kGameOver        = "Sfx_GameOver";
inline constexpr const char* kPowerUp         = "Sfx_PowerUp";
inline constexpr const char* kMissionComplete = "Sfx_MissionComplete";
inline constexpr const char* kTitleVoice      = "Title_MS2";

// 전용 에셋이 없는 효과는 가장 가까운 기존 베이크 사운드로 대체
inline constexpr const char* kBombThrow  = "PlayerSlash1";    // 투척 휘익 소리 대용
inline constexpr const char* kExplosion  = "PlayerBomb";      // 폭발 대용
inline constexpr const char* kEnemyDeath = "PlayerGunDead";   // 적 사망 대용

enum class eAudioEvent
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
    float       volume         = 1.f;
    float       pitchVariation = 0.f;
    bool        bLoop          = false;
    float       seconds        = 0.f;
};

struct AudioConfig
{
    bool     bEnabled     = true;
    int      maxVoices    = 16;
    int      maxPerSound  = 4;
    float    masterVolume = 1.f;
    float    sfxVolume    = 1.f;
    float    bgmVolume    = 0.6f;
    uint32_t pitchSeed    = 0x9E3779B9u;   // 피치 변동용 오디오 전용 RNG
};

class AudioSystem
{
public:
    explicit AudioSystem(bool _bRecord = false);
    ~AudioSystem();
    [[nodiscard]] static AudioSystem& GetSingleton();
    AudioSystem(const AudioSystem&)                          = delete;
    AudioSystem&               operator=(const AudioSystem&) = delete;
    void                       Init(SoundBank _bank, const AudioConfig& _config);
    [[nodiscard]] Result<void> InitFromDirectory(const FilePath& _bakedRoot, const AudioConfig& _config);
    void                       Shutdown();
    void                       Update(float _dtSec);
    void                       Play(StringView _name, float _volume = 1.f, float _pitchVariation = 0.f);
    void                       PlayBgm(StringView _name, bool _bLoop = true);
    void                       StopBgm();
    void                       FadeOutBgm(float _seconds);
    void                       AllStop();
    void                       ChangeBgm(StringView _name, float _fadeOutSec, bool _bLoop = true);

    bool               bRecord;   // 테스트가 요청 순서를 검사한다
    Vector<AudioEvent> events = {};

    [[nodiscard]] bool          IsInitialized() const;
    [[nodiscard]] bool          IsBgmFadingOut() const;
    [[nodiscard]] const String& GetCurrentBgm() const;

private:
    struct Backend;

    enum class eBgmState
    {
        Stopped,
        Playing,
        FadingOut
    };

    static constexpr float kDryVoiceSeconds = 0.5f;

    void PlayBgm_(StringView _name, bool _bLoop);
    void AllStop_();
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

    String m_pendingBgm   = {};
    bool   m_bPendingLoop = true;

    Scoped<Backend> m_pBackend;
};
