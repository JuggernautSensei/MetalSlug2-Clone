#pragma once
#include "Base/Types.h"

namespace slug
{

// ===========================================
//  SoundNames
//   sounds.json 의 키 (Baker 가 레거시 SoundData.txt + 이름이 분명한 추가 파일을 굽는다).
//   시맨틱 이름 → 실제 키 매핑은 여기 한 곳에서만 바꾼다. 존재하지 않는 키를 요청해도 오디오는 조용히 무시한다.
// ===========================================

namespace sfx
{
    // --- sounds.json 에 있는 것 ---
    inline constexpr const char* kPistolShot   = "PlayerGunShot";
    inline constexpr const char* kFlameShot    = "PlayerGunShot_FS";
    inline constexpr const char* kKnife1       = "PlayerSlash1";
    inline constexpr const char* kKnife2       = "PlayerSlash2";
    inline constexpr const char* kBombExplode  = "PlayerBomb";
    inline constexpr const char* kPlayerDeath  = "PlayerGunDead";
    inline constexpr const char* kReload       = "PlayerReload";
    inline constexpr const char* kPickupCoin   = "Coin";
    inline constexpr const char* kOk           = "OK";
    inline constexpr const char* kVoiceHmg     = "PlayerGun_HM";   // 무기 획득 음성 ("Heavy Machine Gun!")
    inline constexpr const char* kVoiceShotgun = "PlayerGun_SG";
    inline constexpr const char* kVoiceRocket  = "PlayerGun_RL";
    inline constexpr const char* kVoiceFlame   = "PlayerGun_FS";
    inline constexpr const char* kVoiceEnemyCh = "PlayerGun_EC";
    inline constexpr const char* kBgmStage     = "Stage1_BGM";

    // --- Baker 가 SoundData.txt 외에 추가로 굽는 것 (이름이 분명한 파일만) ---
    inline constexpr const char* kGameOver        = "Sfx_GameOver";
    inline constexpr const char* kPowerUp         = "Sfx_PowerUp";
    inline constexpr const char* kMissionComplete = "Sfx_Bingo";   // 성공 징글 (이름이 분명한 파일이 이것뿐)
    inline constexpr const char* kLetsGo          = "Sfx_LetsGo";
    inline constexpr const char* kHereWeGo        = "Sfx_HereWeGo";
    inline constexpr const char* kTitleVoice      = "Title_MS3";

    // --- 아직 에셋이 없는 이름 (요청해도 무시됨. 에셋이 생기면 sounds.json 에 같은 이름을 추가하면 된다) ---
    inline constexpr const char* kBombThrow    = "BombThrow";
    inline constexpr const char* kJump         = "PlayerJump";
    inline constexpr const char* kBulletImpact = "BulletImpact";
    inline constexpr const char* kExplosion    = "Explosion";
    inline constexpr const char* kEnemyDeath   = "EnemyDeath";
    inline constexpr const char* kWarning      = "Warning";
    inline constexpr const char* kMenuSelect   = "MenuSelect";
}   // namespace sfx

// ===========================================
//  IAudioSink
//   게임 로직이 소리를 요청하는 유일한 통로. 구현은 AudioSystem(miniaudio), NullAudio(무시), TestAudio(기록).
//   소리 요청은 시뮬레이션 상태를 바꾸지 않아야 한다 (결정성). 상태 코드는 miniaudio 를 직접 부르지 않는다.
// ===========================================

class IAudioSink
{
public:
    virtual ~IAudioSink() = default;

    // 짧은 효과음 (fire-and-forget). _pitchVariation: 0 이면 원음, 0.1 이면 피치 ±10% 안에서 무작위.
    virtual void Play(slug::StringView _name, float _volume = 1.f, float _pitchVariation = 0.f) = 0;

    virtual void PlayBgm(slug::StringView _name, bool _bLoop = true) = 0;
    virtual void StopBgm()                                          = 0;
    virtual void FadeOutBgm(float _seconds)                         = 0;
    virtual void AllStop()                                          = 0;
};

// ===========================================
//  NullAudio
//   아무것도 하지 않는다 (헤드리스/테스트/오디오 장치 없음). 전역 싱글턴 제공.
// ===========================================

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

// ===========================================
//  TestAudio
//   요청을 순서대로 기록한다 (단위 테스트용).
// ===========================================

enum class eAudioEvent : uint8_t
{
    Play,
    PlayBgm,
    StopBgm,
    FadeOutBgm,
    AllStop
};

struct AudioEvent
{
    eAudioEvent kind           = eAudioEvent::Play;
    slug::String name           = {};
    float       volume         = 1.f;   // Play
    float       pitchVariation = 0.f;   // Play
    bool        bLoop          = false; // PlayBgm
    float       seconds        = 0.f;   // FadeOutBgm
};

class TestAudio final : public IAudioSink
{
public:
    void Play(const slug::StringView _name, const float _volume = 1.f, const float _pitchVariation = 0.f) override
    {
        m_events.push_back({ eAudioEvent::Play, slug::String { _name }, _volume, _pitchVariation, false, 0.f });
    }

    void PlayBgm(const slug::StringView _name, const bool _bLoop = true) override
    {
        m_events.push_back({ eAudioEvent::PlayBgm, slug::String { _name }, 1.f, 0.f, _bLoop, 0.f });
        m_bgm = slug::String { _name };
    }

    void StopBgm() override
    {
        m_events.push_back({ eAudioEvent::StopBgm, {}, 1.f, 0.f, false, 0.f });
        m_bgm.clear();
    }

    void FadeOutBgm(const float _seconds) override
    {
        m_events.push_back({ eAudioEvent::FadeOutBgm, {}, 1.f, 0.f, false, _seconds });
    }

    void AllStop() override
    {
        m_events.push_back({ eAudioEvent::AllStop, {}, 1.f, 0.f, false, 0.f });
        m_bgm.clear();
    }

    // --- 조회 ---
    [[nodiscard]] const slug::Vector<AudioEvent>& GetEvents() const { return m_events; }
    [[nodiscard]] const slug::String&             GetCurrentBgm() const { return m_bgm; }
    void                                         Clear() { m_events.clear(); }

    [[nodiscard]] int CountPlays(const slug::StringView _name) const
    {
        int count = 0;
        for (const AudioEvent& e: m_events)
        {
            count += (e.kind == eAudioEvent::Play && e.name == _name) ? 1 : 0;
        }
        return count;
    }

    [[nodiscard]] int CountKind(const eAudioEvent _kind) const
    {
        int count = 0;
        for (const AudioEvent& e: m_events)
        {
            count += e.kind == _kind ? 1 : 0;
        }
        return count;
    }

    [[nodiscard]] int GetPlayCount() const { return CountKind(eAudioEvent::Play); }

private:
    slug::Vector<AudioEvent> m_events = {};
    slug::String             m_bgm    = {};
};

}   // namespace slug
