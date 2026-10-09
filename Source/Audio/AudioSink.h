#pragma once
#include "Base/Types.h"

namespace slug
{
namespace sfx
{
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
    inline constexpr const char* kMissionComplete = "Sfx_Bingo";
    inline constexpr const char* kLetsGo          = "Sfx_LetsGo";
    inline constexpr const char* kHereWeGo        = "Sfx_HereWeGo";
    inline constexpr const char* kTitleVoice      = "Title_MS2";

    // 전용 에셋이 없는 효과는 가장 가까운 기존 베이크 사운드로 대체한다.
    inline constexpr const char* kBombThrow  = "PlayerSlash1";  // 투척 휘익 소리 대용
    inline constexpr const char* kExplosion  = "PlayerBomb";    // 폭발 대용
    inline constexpr const char* kEnemyDeath = "PlayerGunDead"; // 적 사망 대용
    inline constexpr const char* kMenuSelect = "OK";

    // 테스트가 모든 효과 이름이 베이크 사운드로 해석되는지 검사하는 목록 (새 상수는 여기에도 추가)
    inline constexpr const char* kAll[] = {
        kPistolShot, kFlameShot, kKnife1, kKnife2, kBombExplode, kPlayerDeath, kReload, kPickupCoin, kOk,
        kVoiceHmg, kVoiceShotgun, kVoiceRocket, kVoiceFlame, kVoiceEnemyCh, kBgmStage,
        kGameOver, kPowerUp, kMissionComplete, kLetsGo, kHereWeGo, kTitleVoice,
        kBombThrow, kExplosion, kEnemyDeath, kMenuSelect,
    };
}

class IAudioSink
{
public:
    virtual ~IAudioSink() = default;

    virtual void Play(slug::StringView _name, float _volume = 1.f, float _pitchVariation = 0.f) = 0;

    virtual void PlayBgm(slug::StringView _name, bool _bLoop = true) = 0;
    virtual void StopBgm()                                          = 0;
    virtual void FadeOutBgm(float _seconds)                         = 0;
    virtual void AllStop()                                          = 0;
};

class NullAudio final : public IAudioSink
{
public:
    void Play(slug::StringView, float = 1.f, float = 0.f) override {}
    void PlayBgm(slug::StringView, bool = true) override {}
    void StopBgm() override {}
    void FadeOutBgm(float) override {}
    void AllStop() override {}

    [[nodiscard]] static NullAudio& Get()
    {
        static NullAudio s_null;
        return s_null;
    }
};
}
