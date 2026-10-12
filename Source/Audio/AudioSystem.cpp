#include "Audio/AudioSystem.h"

#include <algorithm>

#include "Base/SlugError.h"
#include "Base/Log.h"
#include "Base/PhaseTimer.h"
#include "Core/AsyncLoader.h"
#include "Base/Types.h"

// miniaudio 구현부는 이 파일에서만 만든다 (stb_vorbis 는 헤더 전용으로 선언만 가져온다).
#define STB_VORBIS_HEADER_ONLY
#pragma warning(push, 0)
#include <stb_vorbis.c>
#pragma warning(pop)

#define MINIAUDIO_IMPLEMENTATION
#pragma warning(push)
#pragma warning(disable : 4456 4245 4701)
#include <miniaudio.h>
#pragma warning(pop)

#undef STB_VORBIS_HEADER_ONLY

struct AudioSystem::Backend
{
    ma_engine        engine    = {};
    bool             bEngine   = false;
    Vector<ma_sound> voices    = {};
    Vector<char>     voiceInit = {};
    ma_sound         bgm       = {};
    bool             bBgmInit  = false;

    ~Backend()
    {
        UninitAll();
        if (bEngine)
        {
            ma_engine_uninit(&engine);
            bEngine = false;
        }
    }

    void UninitVoice(
        const size_t _slot)
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
[[nodiscard]] ma_result InitSoundFromFile_(
    ma_engine*      _pEngine,
    const FilePath& _path,
    const ma_uint32 _flags,
    ma_sound*       _pSound)
{
#if defined(_WIN32)
    return ma_sound_init_from_file_w(_pEngine, _path.c_str(), _flags, nullptr, nullptr, _pSound);
#else
    return ma_sound_init_from_file(_pEngine, _path.string().c_str(), _flags, nullptr, nullptr, _pSound);
#endif
}

[[nodiscard]] ma_result RegisterFile_(
    ma_engine*      _pEngine,
    const FilePath& _path,
    const ma_uint32 _flags)
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

class SoundRegisterJob final : public IAsyncLoadJob
{
public:
    SoundRegisterJob(ma_engine* _pEngine, const SoundEntry* _pEntry, int* _pPreloaded);

    void Execute() override;
    void Complete() override;

private:
    ma_engine*        m_pEngine    = nullptr;
    const SoundEntry* m_pEntry     = nullptr;
    int*              m_pPreloaded = nullptr;
    ma_result         m_result     = MA_ERROR;
};

SoundRegisterJob::SoundRegisterJob(
    ma_engine* const        _pEngine,
    const SoundEntry* const _pEntry,
    int* const              _pPreloaded)
    : m_pEngine(_pEngine)
    , m_pEntry(_pEntry)
    , m_pPreloaded(_pPreloaded)
{
}

void SoundRegisterJob::Execute()
{
    m_result = RegisterFile_(m_pEngine, m_pEntry->path, MA_RESOURCE_MANAGER_DATA_SOURCE_FLAG_DECODE);
}

void SoundRegisterJob::Complete()
{
    if (m_result == MA_SUCCESS)
    {
        ++*m_pPreloaded;
    }
    else
    {
        SLUG_LOG_WARN("효과음 로드 실패 {} ({}): {}", m_pEntry->name, m_pEntry->file, ma_result_description(m_result));
    }
}

AudioSystem::AudioSystem() = default;

AudioSystem::~AudioSystem()
{
    Shutdown();
}

Result<void> AudioSystem::Init(
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
    m_warnedMissing.clear();
    m_bInitialized = true;
    m_bDevice      = false;

    if (!_config.bEnabled)
    {
        return Error { kOK };
    }

    std::unique_ptr<Backend> pBackend { new Backend };
    const ma_engine_config   engineConfig = ma_engine_config_init();
    if (const ma_result result = ma_engine_init(&engineConfig, &pBackend->engine); result != MA_SUCCESS)
    {
        SLUG_LOG_WARN("오디오 장치를 열 수 없어 소리 없이 계속합니다 ({})", ma_result_description(result));
        return Error { kOK };
    }
    pBackend->bEngine = true;
    pBackend->voices.resize(static_cast<size_t>(m_voices.GetCapacity()));
    pBackend->voiceInit.assign(static_cast<size_t>(m_voices.GetCapacity()), 0);

    int                    preloaded = 0;
    const ScopedPhaseTimer timer { "sound-decode", "all" };
    for (const SoundEntry& entry: m_bank.GetEntries())
    {
        if (entry.bBgm)
        {
            continue;
        }
        if (_config.pLoader)
        {
            _config.pLoader->Submit(std::make_unique<SoundRegisterJob>(&pBackend->engine, &entry, &preloaded));
        }
        else
        {
            SoundRegisterJob job { &pBackend->engine, &entry, &preloaded };
            job.Execute();
            job.Complete();
        }
    }
    if (_config.pLoader)
    {
        _config.pLoader->WaitAll();
    }

    m_pBackend = std::move(pBackend);
    m_bDevice  = true;
    SLUG_LOG_INFO("오디오: 효과음 {} 개 로드, BGM 스트리밍, 동시 {} 음", preloaded, m_voices.GetCapacity());
    return Error { kOK };
}

Result<void> AudioSystem::InitFromDirectory(
    const FilePath&    _bakedRoot,
    const AudioConfig& _config)
{
    Result<SoundBank> bank = SoundBank::LoadFromDirectory(_bakedRoot);
    if (!bank)
    {
        return Error { bank.GetError() };
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
    m_pBackend.reset();
    m_bDevice      = false;
    m_bInitialized = false;
}

void AudioSystem::Update(
    const float _dtSec)
{
    if (!m_bInitialized)
    {
        return;
    }

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

    if (m_bgmState != eBgmState::Stopped)
    {
        if (m_bgmFade.Update(_dtSec))
        {
            const String pendingName = m_pendingBgm;
            const bool   bPending    = !pendingName.empty() && m_bgmState == eBgmState::FadingOut;
            const bool   bLoop       = m_bPendingLoop;
            StopBgm_();
            if (bPending)
            {
                PlayBgm(pendingName, bLoop);
            }
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

void AudioSystem::Play(
    const StringView _name,
    const float      _volume,
    const float      _pitchVariation)
{
    if (!m_bInitialized)
    {
        return;
    }
    const int index = m_bank.FindIndex(_name);
    if (index < 0)
    {
        if (m_warnedMissing.insert(String { _name }).second)
        {
            SLUG_LOG_TRACE("알 수 없는 효과음 요청: {}", _name);
        }
        return;
    }

    const float volume = Clamp01_(_volume) * m_sfxVolume * m_masterVolume;
    if (volume <= 0.f)
    {
        return;
    }

    const VoiceAllocator::Acquired acquired = m_voices.Acquire(index);
    if (acquired.bStolen && m_bDevice && m_pBackend)
    {
        m_pBackend->UninitVoice(static_cast<size_t>(acquired.slot));
    }
    m_dryRemaining[static_cast<size_t>(acquired.slot)] = kDryVoiceSeconds;

    if (!m_bDevice || !m_pBackend)
    {
        return;
    }

    const SoundEntry& entry  = m_bank.Get(index);
    const size_t      slot   = static_cast<size_t>(acquired.slot);
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
    const StringView _name,
    const bool       _bLoop)
{
    if (!m_bInitialized)
    {
        return;
    }

    const int index = m_bank.FindIndex(_name);
    if (index < 0)
    {
        return;
    }

    if (m_bgmState == eBgmState::Playing && m_bgmName == _name)
    {
        return;
    }

    StopBgm_();
    m_bgmName  = String { _name };
    m_bBgmLoop = _bLoop;
    m_bgmFade.Reset();
    m_bgmState = eBgmState::Playing;

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

void AudioSystem::ChangeBgm(
    const StringView _name,
    const float      _fadeOutSec,
    const bool       _bLoop)
{
    if (!m_bInitialized || m_bank.FindIndex(_name) < 0)
    {
        return;
    }

    if (m_bgmState == eBgmState::Stopped || _fadeOutSec <= 0.f)
    {
        PlayBgm(_name, _bLoop);
        return;
    }

    if (m_bgmState == eBgmState::Playing)
    {
        if (m_bgmName == _name)
        {
            return;
        }
        FadeOutBgm(_fadeOutSec);
    }
    m_pendingBgm   = String { _name };
    m_bPendingLoop = _bLoop;
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

void AudioSystem::StopBgm_()
{
    if (m_pBackend)
    {
        m_pBackend->UninitBgm();
    }
    m_bgmState = eBgmState::Stopped;
    m_bgmName.clear();
    m_pendingBgm.clear();
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

bool AudioSystem::IsInitialized() const
{
    return m_bInitialized;
}

bool AudioSystem::IsBgmFadingOut() const
{
    return m_bgmState == eBgmState::FadingOut;
}

const String& AudioSystem::GetCurrentBgm() const
{
    return m_bgmName;
}
