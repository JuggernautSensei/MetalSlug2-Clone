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
inline constexpr const char* kReload       = "PlayerReload";
inline constexpr const char* kPickupCoin   = "Coin";
inline constexpr const char* kOk           = "OK";
inline constexpr const char* kVoiceHmg     = "PlayerGun_HM";
inline constexpr const char* kVoiceShotgun = "PlayerGun_SG";
inline constexpr const char* kVoiceRocket  = "PlayerGun_RL";
inline constexpr const char* kVoiceFlame   = "PlayerGun_FS";
inline constexpr const char* kVoiceEnemyCh = "PlayerGun_EC";
inline constexpr const char* kBgmStage     = "Stage1_BGM";

inline constexpr const char* kGameOver        = "Sfx_GameOver";
inline constexpr const char* kPowerUp         = "Sfx_PowerUp";
inline constexpr const char* kMissionComplete = "Sfx_MissionComplete";
inline constexpr const char* kLetsGo          = "Sfx_LetsGo";
inline constexpr const char* kHereWeGo        = "Sfx_HereWeGo";
inline constexpr const char* kTitleVoice      = "Title_MS2";

// 전용 에셋이 없는 효과는 가장 가까운 기존 베이크 사운드로 대체한다.
inline constexpr const char* kBombThrow  = "PlayerSlash1";    // 투척 휘익 소리 대용
inline constexpr const char* kExplosion  = "PlayerBomb";      // 폭발 대용
inline constexpr const char* kEnemyDeath = "PlayerGunDead";   // 적 사망 대용
inline constexpr const char* kMenuSelect = "OK";

// 테스트가 모든 효과 이름이 베이크 사운드로 해석되는지 검사하는 목록 (새 상수는 여기에도 추가)
inline constexpr ARRAY<const char*, 25> kAll = {
    kPistolShot,
    kFlameShot,
    kKnife1,
    kKnife2,
    kBombExplode,
    kPlayerDeath,
    kReload,
    kPickupCoin,
    kOk,
    kVoiceHmg,
    kVoiceShotgun,
    kVoiceRocket,
    kVoiceFlame,
    kVoiceEnemyCh,
    kBgmStage,
    kGameOver,
    kPowerUp,
    kMissionComplete,
    kLetsGo,
    kHereWeGo,
    kTitleVoice,
    kBombThrow,
    kExplosion,
    kEnemyDeath,
    kMenuSelect,
};

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
    float       volume         = 1.f;
    float       pitchVariation = 0.f;
    bool        bLoop          = false;
    float       seconds        = 0.f;
};

struct AudioConfig
{
    bool     bEnabled     = true;
    int      maxVoices    = 16;   // 동시에 울리는 효과음 상한
    int      maxPerSound  = 4;    // 같은 소리의 동시 상한 (권총 연사 등)
    float    masterVolume = 1.f;
    float    sfxVolume    = 1.f;
    float    bgmVolume    = 0.6f;
    uint32_t pitchSeed    = 0x9E3779B9u;   // 피치 변동용 오디오 전용 RNG (시뮬레이션 RNG 와 무관)
};

class AudioSystem final
{
public:
    explicit AudioSystem(bool _bRecord = false);
    ~AudioSystem();

    AudioSystem(const AudioSystem&)            = delete;
    AudioSystem& operator=(const AudioSystem&) = delete;

    void Init(SoundBank _bank, const AudioConfig& _config);
    [[nodiscard]] Result<void> InitFromDirectory(const FilePath& _bakedRoot, const AudioConfig& _config);
    void                       Shutdown();

    void Update(float _dtSec);

    void Play(StringView _name, float _volume = 1.f, float _pitchVariation = 0.f);
    void PlayBgm(StringView _name, bool _bLoop = true);
    void StopBgm();
    void FadeOutBgm(float _seconds);
    void AllStop();
    void ChangeBgm(StringView _name, float _fadeOutSec, bool _bLoop = true);

    bool               bRecord;   // 테스트가 요청 순서를 검사한다 (장치 없이도 기록)
    Vector<AudioEvent> events  = {};

    [[nodiscard]] bool          IsInitialized() const;
    [[nodiscard]] bool          IsBgmFadingOut() const;
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

    String m_pendingBgm   = {};   // 페이드 아웃이 끝나면 재생할 BGM
    bool   m_bPendingLoop = true;

    Scoped<Backend> m_pBackend;
};
