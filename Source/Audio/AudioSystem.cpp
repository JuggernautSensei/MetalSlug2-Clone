#include "Audio/AudioSystem.h"

// ===========================================
//  miniaudio 구현부 (프로젝트에서 MINIAUDIO_IMPLEMENTATION 은 이 파일 하나뿐)
//   Vorbis(.ogg) 디코딩을 위해 stb_vorbis 를 miniaudio 구현 앞/뒤로 끼워 넣는다.
// ===========================================

#define STB_VORBIS_HEADER_ONLY
#include <stb_vorbis.c>

#define MINIAUDIO_IMPLEMENTATION
#include <miniaudio.h>

#undef STB_VORBIS_HEADER_ONLY
#include <stb_vorbis.c>

#include <algorithm>

#include <JugX/CoreLogger.h>

#include "Core/Error.h"

namespace slug
{

// ===========================================
//  Backend (miniaudio 상태. 장치가 없으면 만들어지지 않는다)
// ===========================================

struct AudioSystem::Backend
{
    ma_engine             engine        = {};
    bool                  bEngine       = false;
    jug::Vector<ma_sound> voices        = {};
    jug::Vector<char>     voiceInit     = {};   // 슬롯의 ma_sound 가 초기화되어 있는가
    ma_sound              bgm           = {};
    bool                  bBgmInit      = false;

    ~Backend()
    {
        UninitAll();
        if (bEngine)
        {
            ma_engine_uninit(&engine);
            bEngine = false;
        }
    }

    void UninitVoice(const size_t _slot)
    {
        if (_slot < voiceInit.size() && voiceInit[_slot])
        {
            ma_sound_uninit(&voices[_slot]);
            voiceInit[_slot] = 0;
        }
    }

    void UninitBgm()
    {
        if (bBgmInit)
        {
            ma_sound_uninit(&bgm);
            bBgmInit = false;
        }
    }

    void UninitAll()
    {
        for (size_t i = 0; i < voiceInit.size(); ++i)
        {
            UninitVoice(i);
        }
        UninitBgm();
    }
};

namespace
{
    // 플랫폼 경로 → miniaudio 파일 열기 (Windows 는 와이드 경로로 UTF-8/ANSI 변환 문제를 피한다)
    [[nodiscard]] ma_result InitSoundFromFile_(
        ma_engine*            _pEngine,
        const jug::FilePath&  _path,
        const ma_uint32       _flags,
        ma_sound*             _pSound)
    {
#if defined(_WIN32)
        return ma_sound_init_from_file_w(_pEngine, _path.c_str(), _flags, nullptr, nullptr, _pSound);
#else
        return ma_sound_init_from_file(_pEngine, _path.string().c_str(), _flags, nullptr, nullptr, _pSound);
#endif
    }

    [[nodiscard]] ma_result RegisterFile_(
        ma_engine*           _pEngine,
        const jug::FilePath& _path,
        const ma_uint32      _flags)
    {
#if defined(_WIN32)
        return ma_resource_manager_register_file_w(ma_engine_get_resource_manager(_pEngine), _path.c_str(), _flags);
#else
        return ma_resource_manager_register_file(ma_engine_get_resource_manager(_pEngine), _path.string().c_str(), _flags);
#endif
    }

    [[nodiscard]] float Clamp01_(
        const float _v)
    {
        return std::clamp(_v, 0.f, 1.f);
    }
}   // namespace

// ===========================================
//  생성/소멸/초기화
// ===========================================

AudioSystem::AudioSystem() = default;

AudioSystem::~AudioSystem()
{
    Shutdown();
}

jug::Result<void> AudioSystem::Init(
    SoundBank          _bank,
    const AudioConfig& _config)
{
    Shutdown();

    m_bank         = std::move(_bank);
    m_masterVolume = Clamp01_(_config.masterVolume);
    m_sfxVolume    = Clamp01_(_config.sfxVolume);
    m_bgmVolume    = Clamp01_(_config.bgmVolume);
    m_rng          = AudioRng { _config.pitchSeed };
    m_voices.Reset(_config.maxVoices, _config.maxPerSound);
    m_dryRemaining.assign(static_cast<size_t>(m_voices.GetCapacity()), 0.f);
    m_stats        = {};
    m_warnedMissing.clear();
    m_bInitialized = true;
    m_bDevice      = false;

    if (!_config.bEnabled)
    {
        return jug::Error { jug::kOK };
    }

    // 장치 시도. 실패해도 오류가 아니라 dry 모드로 계속한다.
    jug::Scoped<Backend> pBackend { new Backend };
    const ma_engine_config engineConfig = ma_engine_config_init();
    if (const ma_result result = ma_engine_init(&engineConfig, &pBackend->engine); result != MA_SUCCESS)
    {
        JUG_CORE_LOG_WARN("오디오 장치를 열 수 없어 소리 없이 계속합니다 ({})", ma_result_description(result));
        return jug::Error { jug::kOK };
    }
    pBackend->bEngine = true;
    pBackend->voices.resize(static_cast<size_t>(m_voices.GetCapacity()));
    pBackend->voiceInit.assign(static_cast<size_t>(m_voices.GetCapacity()), 0);

    // 짧은 효과음은 미리 메모리에 디코딩해 둔다 (재생 때마다 디스크/디코더를 건드리지 않는다)
    int preloaded = 0;
    for (const SoundEntry& entry: m_bank.GetEntries())
    {
        if (entry.bBgm)
        {
            continue;
        }
        if (const ma_result result = RegisterFile_(&pBackend->engine, entry.path, MA_RESOURCE_MANAGER_DATA_SOURCE_FLAG_DECODE); result == MA_SUCCESS)
        {
            ++preloaded;
        }
        else
        {
            ++m_stats.loadFailed;
            JUG_CORE_LOG_WARN("효과음 로드 실패 {} ({}): {}", entry.name, entry.file, ma_result_description(result));
        }
    }

    m_stats.preloaded = preloaded;
    m_pBackend        = std::move(pBackend);
    m_bDevice         = true;
    JUG_CORE_LOG_INFO("오디오: 효과음 {} 개 로드, BGM 스트리밍, 동시 {} 음", preloaded, m_voices.GetCapacity());
    return jug::Error { jug::kOK };
}

jug::Result<void> AudioSystem::InitFromDirectory(
    const jug::FilePath& _bakedRoot,
    const AudioConfig&   _config)
{
    jug::Result<SoundBank> bank = SoundBank::LoadFromDirectory(_bakedRoot);
    if (!bank)
    {
        return jug::Error { bank.GetError() };
    }
    return Init(bank.Take(), _config);
}

void AudioSystem::Shutdown()
{
    if (!m_bInitialized)
    {
        return;
    }
    AllStop();
    m_pBackend.Reset();
    m_bDevice      = false;
    m_bInitialized = false;
}

// ===========================================
//  프레임 갱신
// ===========================================

void AudioSystem::Update(
    const float _dtSec)
{
    if (!m_bInitialized)
    {
        return;
    }

    // --- 끝난 효과음 회수 ---
    for (int slot = 0; slot < m_voices.GetCapacity(); ++slot)
    {
        if (!m_voices.IsActive(slot))
        {
            continue;
        }

        bool bDone = false;
        if (m_bDevice && m_pBackend)
        {
            const size_t i = static_cast<size_t>(slot);
            bDone          = !m_pBackend->voiceInit[i] || ma_sound_at_end(&m_pBackend->voices[i]) == MA_TRUE;
        }
        else
        {
            float& remaining = m_dryRemaining[static_cast<size_t>(slot)];
            remaining -= _dtSec;
            bDone = remaining <= 0.f;
        }

        if (bDone)
        {
            StopVoice_(slot);
        }
    }

    // --- BGM ---
    if (m_bgmState != eBgmState::Stopped)
    {
        if (m_bgmFade.Update(_dtSec))
        {
            StopBgm_();
        }
        else if (m_bDevice && m_pBackend && m_pBackend->bBgmInit && !m_bBgmLoop && ma_sound_at_end(&m_pBackend->bgm) == MA_TRUE)
        {
            StopBgm_();
        }
        else
        {
            ApplyBgmVolume_();
        }
    }
}

// ===========================================
//  IAudioSink
// ===========================================

void AudioSystem::Play(
    const jug::StringView _name,
    const float           _volume,
    const float           _pitchVariation)
{
    if (!m_bInitialized)
    {
        return;
    }
    ++m_stats.playRequests;

    const int index = m_bank.FindIndex(_name);
    if (index < 0)
    {
        ++m_stats.missing;
        if (m_warnedMissing.insert(jug::String { _name }).second)
        {
            JUG_CORE_LOG_TRACE("알 수 없는 효과음 요청: {}", _name);
        }
        return;
    }

    const float volume = Clamp01_(_volume) * m_sfxVolume * m_masterVolume;
    if (volume <= 0.f)
    {
        return;
    }

    const VoiceAllocator::Acquired acquired = m_voices.Acquire(index);
    if (acquired.bStolen)
    {
        ++m_stats.stolen;
        if (m_bDevice && m_pBackend)
        {
            m_pBackend->UninitVoice(static_cast<size_t>(acquired.slot));
        }
    }
    ++m_stats.started;
    m_dryRemaining[static_cast<size_t>(acquired.slot)] = kDryVoiceSeconds;

    if (!m_bDevice || !m_pBackend)
    {
        return;
    }

    const SoundEntry& entry = m_bank.Get(index);
    const size_t      slot  = static_cast<size_t>(acquired.slot);
    ma_sound*         pSound = &m_pBackend->voices[slot];

    const ma_uint32 flags = MA_SOUND_FLAG_NO_SPATIALIZATION | (entry.bBgm ? MA_SOUND_FLAG_STREAM : MA_SOUND_FLAG_DECODE);
    if (InitSoundFromFile_(&m_pBackend->engine, entry.path, flags, pSound) != MA_SUCCESS)
    {
        m_voices.Release(acquired.slot);
        return;
    }
    m_pBackend->voiceInit[slot] = 1;

    ma_sound_set_volume(pSound, volume);
    if (_pitchVariation > 0.f)
    {
        ma_sound_set_pitch(pSound, ComputePitch(_pitchVariation, m_rng.NextUnit()));
    }
    ma_sound_start(pSound);
}

void AudioSystem::PlayBgm(
    const jug::StringView _name,
    const bool            _bLoop)
{
    if (!m_bInitialized)
    {
        return;
    }

    const int index = m_bank.FindIndex(_name);
    if (index < 0)
    {
        ++m_stats.missing;
        return;
    }

    // 이미 같은 곡이 (페이드 아웃 중이 아니게) 재생 중이면 끊김 없이 유지한다
    if (m_bgmState == eBgmState::Playing && m_bgmName == _name)
    {
        return;
    }

    StopBgm_();
    m_bgmName  = jug::String { _name };
    m_bBgmLoop = _bLoop;
    m_bgmFade.Reset();
    m_bgmState = eBgmState::Playing;
    ++m_stats.bgmStarts;

    if (!m_bDevice || !m_pBackend)
    {
        return;
    }

    const SoundEntry& entry = m_bank.Get(index);
    if (InitSoundFromFile_(&m_pBackend->engine, entry.path, MA_SOUND_FLAG_STREAM | MA_SOUND_FLAG_NO_SPATIALIZATION, &m_pBackend->bgm) != MA_SUCCESS)
    {
        m_bgmState = eBgmState::Stopped;
        m_bgmName.clear();
        return;
    }
    m_pBackend->bBgmInit = true;
    ma_sound_set_looping(&m_pBackend->bgm, _bLoop ? MA_TRUE : MA_FALSE);
    ApplyBgmVolume_();
    ma_sound_start(&m_pBackend->bgm);
}

void AudioSystem::StopBgm()
{
    StopBgm_();
}

void AudioSystem::FadeOutBgm(
    const float _seconds)
{
    if (m_bgmState != eBgmState::Playing)
    {
        return;
    }
    if (_seconds <= 0.f)
    {
        StopBgm_();
        return;
    }
    m_bgmState = eBgmState::FadingOut;
    m_bgmFade.BeginFadeOut(_seconds);
}

void AudioSystem::AllStop()
{
    for (int slot = 0; slot < m_voices.GetCapacity(); ++slot)
    {
        if (m_voices.IsActive(slot))
        {
            StopVoice_(slot);
        }
    }
    StopBgm_();
}

// ===========================================
//  볼륨
// ===========================================

void AudioSystem::SetMasterVolume(
    const float _volume)
{
    m_masterVolume = Clamp01_(_volume);
    ApplyBgmVolume_();
}

void AudioSystem::SetSfxVolume(
    const float _volume)
{
    m_sfxVolume = Clamp01_(_volume);
}

void AudioSystem::SetBgmVolume(
    const float _volume)
{
    m_bgmVolume = Clamp01_(_volume);
    ApplyBgmVolume_();
}

// ===========================================
//  내부
// ===========================================

void AudioSystem::StopBgm_()
{
    if (m_pBackend)
    {
        m_pBackend->UninitBgm();
    }
    m_bgmState = eBgmState::Stopped;
    m_bgmName.clear();
    m_bgmFade.Reset();
}

void AudioSystem::ApplyBgmVolume_()
{
    if (m_bDevice && m_pBackend && m_pBackend->bBgmInit)
    {
        ma_sound_set_volume(&m_pBackend->bgm, m_masterVolume * m_bgmVolume * m_bgmFade.gain);
    }
}

void AudioSystem::StopVoice_(
    const int _slot)
{
    if (m_bDevice && m_pBackend)
    {
        m_pBackend->UninitVoice(static_cast<size_t>(_slot));
    }
    m_voices.Release(_slot);
}

}   // namespace slug
