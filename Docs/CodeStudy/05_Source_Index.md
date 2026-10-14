# 전체 소스 파일·함수 색인

2026-10-14. 기준 커밋: `0ef7104`. 이 색인은 코드 학습용 탐색 지도이며, 자동 추출 결과만으로 기능의 호출 가능성이나 정확성을 증명하지 않는다. 상세 설명은 01~03 문서를 읽는다.
전체 194파일 · 45405물리줄 · 추출 항목 3825개.


- 게임 소스와 에디터 소스를 포함한다. Vendor/JugX는 외부 라이브러리로 별도 취급한다.
- 헤더에는 타입 선언과 API 위치, 구현 파일에는 함수 정의 위치를 표시한다.
- 매크로·템플릿·복잡한 생성자·연산자 표기는 자동 추출에서 누락될 수 있으므로 실제 파일을 함께 확인한다.

## Source/Audio/AudioMath.cpp

[AudioMath.cpp](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Audio/AudioMath.cpp>) · 67줄

| 위치 | 선언·함수 |
|---|---|
| [L8](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Audio/AudioMath.cpp:8>) | `uint32_t AudioRng::NextU32()` |
| [L18](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Audio/AudioMath.cpp:18>) | `float AudioRng::NextUnit()` |
| [L23](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Audio/AudioMath.cpp:23>) | `void BgmFader::Reset()` |
| [L30](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Audio/AudioMath.cpp:30>) | `void BgmFader::BeginFadeOut( const float _seconds)` |
| [L45](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Audio/AudioMath.cpp:45>) | `bool BgmFader::Update( const float _dtSec)` |
| [L56](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Audio/AudioMath.cpp:56>) | `bool BgmFader::IsDone() const` |
| [L61](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Audio/AudioMath.cpp:61>) | `float ComputePitch( const float _variation, const float _unit01)` |

## Source/Audio/AudioMath.h

[AudioMath.h](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Audio/AudioMath.h>) · 28줄

| 위치 | 선언·함수 |
|---|---|
| [L5](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Audio/AudioMath.h:5>) | `[[nodiscard]] float ComputePitch(float _variation, float _unit01);` |
| [L7](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Audio/AudioMath.h:7>) | `class AudioRng` |
| [L10](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Audio/AudioMath.h:10>) | `explicit AudioRng(uint32_t _seed = 0x9E3779B9u);` |
| [L11](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Audio/AudioMath.h:11>) | `[[nodiscard]] uint32_t NextU32();` |
| [L12](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Audio/AudioMath.h:12>) | `[[nodiscard]] float    NextUnit();` |
| [L18](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Audio/AudioMath.h:18>) | `struct BgmFader` |
| [L24](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Audio/AudioMath.h:24>) | `void               Reset();` |
| [L25](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Audio/AudioMath.h:25>) | `void               BeginFadeOut(float _seconds);` |
| [L26](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Audio/AudioMath.h:26>) | `[[nodiscard]] bool Update(float _dtSec);` |
| [L27](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Audio/AudioMath.h:27>) | `[[nodiscard]] bool IsDone() const;` |

## Source/Audio/AudioSystem.cpp

[AudioSystem.cpp](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Audio/AudioSystem.cpp>) · 497줄

| 위치 | 선언·함수 |
|---|---|
| [L36](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Audio/AudioSystem.cpp:36>) | `void UninitVoice( const size_t _slot)` |
| [L46](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Audio/AudioSystem.cpp:46>) | `void UninitBgm()` |
| [L55](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Audio/AudioSystem.cpp:55>) | `void UninitAll()` |
| [L67](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Audio/AudioSystem.cpp:67>) | `[[nodiscard]] ma_result InitSoundFromFile_( ma_engine* _pEngine, const FilePath& _path, const ma_uint32 _flags, ma_sound* _pSound)` |
| [L80](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Audio/AudioSystem.cpp:80>) | `[[nodiscard]] ma_result RegisterFile_( ma_engine* _pEngine, const FilePath& _path, const ma_uint32 _flags)` |
| [L92](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Audio/AudioSystem.cpp:92>) | `[[nodiscard]] float Clamp01_( const float _v)` |
| [L104](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Audio/AudioSystem.cpp:104>) | `AudioSystem& AudioSystem::GetSingleton()` |
| [L115](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Audio/AudioSystem.cpp:115>) | `void AudioSystem::Init( SoundBank _bank, const AudioConfig& _config)` |
| [L172](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Audio/AudioSystem.cpp:172>) | `Result<void> AudioSystem::InitFromDirectory( const FilePath& _bakedRoot, const AudioConfig& _config)` |
| [L185](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Audio/AudioSystem.cpp:185>) | `void AudioSystem::Shutdown()` |
| [L197](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Audio/AudioSystem.cpp:197>) | `void AudioSystem::Update( const float _dtSec)` |
| [L244](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Audio/AudioSystem.cpp:244>) | `else if (m_bDevice && m_pBackend && m_pBackend->bBgmInit && !m_bBgmLoop && ma_sound_at_end(&m_pBackend->bgm) == MA_TRUE)` |
| [L255](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Audio/AudioSystem.cpp:255>) | `void AudioSystem::Play( const StringView _name, const float _volume, const float _pitchVariation)` |
| [L316](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Audio/AudioSystem.cpp:316>) | `void AudioSystem::PlayBgm( const StringView _name, const bool _bLoop)` |
| [L327](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Audio/AudioSystem.cpp:327>) | `void AudioSystem::PlayBgm_( const StringView _name, const bool _bLoop)` |
| [L371](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Audio/AudioSystem.cpp:371>) | `void AudioSystem::StopBgm()` |
| [L380](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Audio/AudioSystem.cpp:380>) | `void AudioSystem::FadeOutBgm( const float _seconds)` |
| [L400](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Audio/AudioSystem.cpp:400>) | `void AudioSystem::ChangeBgm( const StringView _name, const float _fadeOutSec, const bool _bLoop)` |
| [L433](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Audio/AudioSystem.cpp:433>) | `void AudioSystem::AllStop()` |
| [L442](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Audio/AudioSystem.cpp:442>) | `void AudioSystem::AllStop_()` |
| [L454](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Audio/AudioSystem.cpp:454>) | `void AudioSystem::StopBgm_()` |
| [L466](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Audio/AudioSystem.cpp:466>) | `void AudioSystem::ApplyBgmVolume_()` |
| [L474](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Audio/AudioSystem.cpp:474>) | `void AudioSystem::StopVoice_( const int _slot)` |
| [L484](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Audio/AudioSystem.cpp:484>) | `bool AudioSystem::IsInitialized() const` |
| [L489](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Audio/AudioSystem.cpp:489>) | `bool AudioSystem::IsBgmFadingOut() const` |
| [L494](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Audio/AudioSystem.cpp:494>) | `const String& AudioSystem::GetCurrentBgm() const` |

## Source/Audio/AudioSystem.h

[AudioSystem.h](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Audio/AudioSystem.h>) · 130줄

| 위치 | 선언·함수 |
|---|---|
| [L34](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Audio/AudioSystem.h:34>) | `enum class eAudioEvent` |
| [L44](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Audio/AudioSystem.h:44>) | `struct AudioEvent` |
| [L54](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Audio/AudioSystem.h:54>) | `struct AudioConfig` |
| [L65](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Audio/AudioSystem.h:65>) | `class AudioSystem` |
| [L68](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Audio/AudioSystem.h:68>) | `explicit AudioSystem(bool _bRecord = false);` |
| [L69](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Audio/AudioSystem.h:69>) | `~AudioSystem();` |
| [L70](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Audio/AudioSystem.h:70>) | `[[nodiscard]] static AudioSystem& GetSingleton();` |
| [L71](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Audio/AudioSystem.h:71>) | `AudioSystem(const AudioSystem&)                          = delete;` |
| [L72](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Audio/AudioSystem.h:72>) | `AudioSystem&               operator=(const AudioSystem&) = delete;` |
| [L73](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Audio/AudioSystem.h:73>) | `void                       Init(SoundBank _bank, const AudioConfig& _config);` |
| [L74](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Audio/AudioSystem.h:74>) | `[[nodiscard]] Result<void> InitFromDirectory(const FilePath& _bakedRoot, const AudioConfig& _config);` |
| [L75](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Audio/AudioSystem.h:75>) | `void                       Shutdown();` |
| [L76](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Audio/AudioSystem.h:76>) | `void                       Update(float _dtSec);` |
| [L77](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Audio/AudioSystem.h:77>) | `void                       Play(StringView _name, float _volume = 1.f, float _pitchVariation = 0.f);` |
| [L78](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Audio/AudioSystem.h:78>) | `void                       PlayBgm(StringView _name, bool _bLoop = true);` |
| [L79](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Audio/AudioSystem.h:79>) | `void                       StopBgm();` |
| [L80](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Audio/AudioSystem.h:80>) | `void                       FadeOutBgm(float _seconds);` |
| [L81](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Audio/AudioSystem.h:81>) | `void                       AllStop();` |
| [L82](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Audio/AudioSystem.h:82>) | `void                       ChangeBgm(StringView _name, float _fadeOutSec, bool _bLoop = true);` |
| [L87](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Audio/AudioSystem.h:87>) | `[[nodiscard]] bool          IsInitialized() const;` |
| [L88](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Audio/AudioSystem.h:88>) | `[[nodiscard]] bool          IsBgmFadingOut() const;` |
| [L89](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Audio/AudioSystem.h:89>) | `[[nodiscard]] const String& GetCurrentBgm() const;` |
| [L92](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Audio/AudioSystem.h:92>) | `struct Backend;` |
| [L94](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Audio/AudioSystem.h:94>) | `enum class eBgmState` |
| [L103](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Audio/AudioSystem.h:103>) | `void PlayBgm_(StringView _name, bool _bLoop);` |
| [L104](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Audio/AudioSystem.h:104>) | `void AllStop_();` |
| [L105](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Audio/AudioSystem.h:105>) | `void StopBgm_();` |
| [L106](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Audio/AudioSystem.h:106>) | `void ApplyBgmVolume_();` |
| [L107](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Audio/AudioSystem.h:107>) | `void StopVoice_(int _slot);` |

## Source/Audio/SoundBank.cpp

[SoundBank.cpp](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Audio/SoundBank.cpp>) · 117줄

| 위치 | 선언·함수 |
|---|---|
| [L9](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Audio/SoundBank.cpp:9>) | `[[nodiscard]] Result<SoundBank> Parse_( const JsonReader& _root, const FilePath& _rootDir)` |
| [L54](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Audio/SoundBank.cpp:54>) | `Result<SoundBank> SoundBank::LoadFromJson( const StringView _json, const FilePath& _root)` |
| [L66](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Audio/SoundBank.cpp:66>) | `Result<SoundBank> SoundBank::LoadFromDirectory( const FilePath& _bakedRoot)` |
| [L77](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Audio/SoundBank.cpp:77>) | `void SoundBank::Add( SoundEntry _entry)` |
| [L89](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Audio/SoundBank.cpp:89>) | `const SoundEntry* SoundBank::Find( const StringView _name) const` |
| [L96](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Audio/SoundBank.cpp:96>) | `int SoundBank::FindIndex( const StringView _name) const` |
| [L103](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Audio/SoundBank.cpp:103>) | `const SoundEntry& SoundBank::Get( const int _index) const` |
| [L109](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Audio/SoundBank.cpp:109>) | `size_t SoundBank::GetCount() const` |
| [L114](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Audio/SoundBank.cpp:114>) | `Span<const SoundEntry> SoundBank::GetEntries() const` |

## Source/Audio/SoundBank.h

[SoundBank.h](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Audio/SoundBank.h>) · 30줄

| 위치 | 선언·함수 |
|---|---|
| [L5](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Audio/SoundBank.h:5>) | `struct SoundEntry` |
| [L13](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Audio/SoundBank.h:13>) | `class SoundBank` |
| [L16](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Audio/SoundBank.h:16>) | `[[nodiscard]] static Result<SoundBank> LoadFromJson(StringView _json, const FilePath& _root = {});` |
| [L17](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Audio/SoundBank.h:17>) | `[[nodiscard]] static Result<SoundBank> LoadFromDirectory(const FilePath& _bakedRoot);` |
| [L18](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Audio/SoundBank.h:18>) | `void                                   Add(SoundEntry _entry);` |
| [L19](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Audio/SoundBank.h:19>) | `[[nodiscard]] const SoundEntry*        Find(StringView _name) const;` |
| [L20](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Audio/SoundBank.h:20>) | `[[nodiscard]] int                      FindIndex(StringView _name) const;` |
| [L21](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Audio/SoundBank.h:21>) | `[[nodiscard]] const SoundEntry&        Get(int _index) const;` |
| [L22](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Audio/SoundBank.h:22>) | `[[nodiscard]] size_t                   GetCount() const;` |
| [L23](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Audio/SoundBank.h:23>) | `[[nodiscard]] Span<const SoundEntry>   GetEntries() const;` |

## Source/Audio/VoiceAllocator.cpp

[VoiceAllocator.cpp](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Audio/VoiceAllocator.cpp>) · 112줄

| 위치 | 선언·함수 |
|---|---|
| [L10](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Audio/VoiceAllocator.cpp:10>) | `void VoiceAllocator::Reset( const int _capacity, const int _maxPerSound)` |
| [L19](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Audio/VoiceAllocator.cpp:19>) | `VoiceAllocator::Acquired VoiceAllocator::Acquire( const int _soundId)` |
| [L50](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Audio/VoiceAllocator.cpp:50>) | `void VoiceAllocator::Release( const int _slot)` |
| [L59](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Audio/VoiceAllocator.cpp:59>) | `int VoiceAllocator::GetCapacity() const` |
| [L64](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Audio/VoiceAllocator.cpp:64>) | `bool VoiceAllocator::IsActive( const int _slot) const` |
| [L70](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Audio/VoiceAllocator.cpp:70>) | `int VoiceAllocator::GetSoundId( const int _slot) const` |
| [L76](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Audio/VoiceAllocator.cpp:76>) | `int VoiceAllocator::GetActiveCount() const` |
| [L86](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Audio/VoiceAllocator.cpp:86>) | `int VoiceAllocator::CountSound( const int _soundId) const` |
| [L97](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Audio/VoiceAllocator.cpp:97>) | `int VoiceAllocator::FindOldest_( const int _soundId) const` |

## Source/Audio/VoiceAllocator.h

[VoiceAllocator.h](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Audio/VoiceAllocator.h>) · 37줄

| 위치 | 선언·함수 |
|---|---|
| [L4](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Audio/VoiceAllocator.h:4>) | `class VoiceAllocator` |
| [L7](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Audio/VoiceAllocator.h:7>) | `struct Acquired` |
| [L13](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Audio/VoiceAllocator.h:13>) | `VoiceAllocator() = default;` |
| [L14](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Audio/VoiceAllocator.h:14>) | `VoiceAllocator(int _capacity, int _maxPerSound);` |
| [L15](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Audio/VoiceAllocator.h:15>) | `void                   Reset(int _capacity, int _maxPerSound);` |
| [L16](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Audio/VoiceAllocator.h:16>) | `[[nodiscard]] Acquired Acquire(int _soundId);` |
| [L17](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Audio/VoiceAllocator.h:17>) | `void                   Release(int _slot);` |
| [L18](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Audio/VoiceAllocator.h:18>) | `[[nodiscard]] int      GetCapacity() const;` |
| [L19](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Audio/VoiceAllocator.h:19>) | `[[nodiscard]] bool     IsActive(int _slot) const;` |
| [L20](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Audio/VoiceAllocator.h:20>) | `[[nodiscard]] int      GetSoundId(int _slot) const;` |
| [L21](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Audio/VoiceAllocator.h:21>) | `[[nodiscard]] int      GetActiveCount() const;` |
| [L22](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Audio/VoiceAllocator.h:22>) | `[[nodiscard]] int      CountSound(int _soundId) const;` |
| [L25](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Audio/VoiceAllocator.h:25>) | `[[nodiscard]] int FindOldest_(int _soundId) const;` |
| [L27](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Audio/VoiceAllocator.h:27>) | `struct Slot` |

## Source/Base/JsonFields.cpp

[JsonFields.cpp](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Base/JsonFields.cpp>) · 200줄

| 위치 | 선언·함수 |
|---|---|
| [L10](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Base/JsonFields.cpp:10>) | `[[nodiscard]] bool Probe_( const JsonReader& _object, const StringView _key, const bool _bRequired, StringView& _reason, bool* const _pFound)` |
| [L30](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Base/JsonFields.cpp:30>) | `bool HasKey( const JsonReader& _object, const StringView _key)` |
| [L37](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Base/JsonFields.cpp:37>) | `Result<JsonReader> TryField( const JsonReader& _object, const StringView _key)` |
| [L48](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Base/JsonFields.cpp:48>) | `bool ReadInt( const JsonReader& _object, const StringView _key, int& _out)` |
| [L62](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Base/JsonFields.cpp:62>) | `bool ReadNumberField( const JsonReader& _object, const StringView _key, const bool _bRequired, double& _out, StringView& _reason, bool* const _pFound)` |
| [L84](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Base/JsonFields.cpp:84>) | `bool ReadFloatField( const JsonReader& _object, const StringView _key, const bool _bRequired, float& _out, StringView& _reason)` |
| [L104](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Base/JsonFields.cpp:104>) | `bool ReadBoolField( const JsonReader& _object, const StringView _key, const bool _bRequired, bool& _out, StringView& _reason, bool* const _pFound)` |
| [L126](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Base/JsonFields.cpp:126>) | `bool ReadStringField( const JsonReader& _object, const StringView _key, const bool _bRequired, String& _out, StringView& _reason, bool* const _pFound)` |
| [L148](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Base/JsonFields.cpp:148>) | `bool ReadVec2Field( const JsonReader& _object, const StringView _key, const bool _bRequired, VECTOR2& _out, StringView& _reason, bool* const _pFound)` |
| [L181](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Base/JsonFields.cpp:181>) | `bool CheckJsonKeys( const JsonReader& _object, const Span<const StringView> _allowed, StringView& _badKey)` |

## Source/Base/JsonFields.h

[JsonFields.h](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Base/JsonFields.h>) · 33줄

| 위치 | 선언·함수 |
|---|---|
| [L9](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Base/JsonFields.h:9>) | `[[nodiscard]] bool               HasKey(const JsonReader& _object, StringView _key);` |
| [L10](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Base/JsonFields.h:10>) | `[[nodiscard]] Result<JsonReader> TryField(const JsonReader& _object, StringView _key);` |
| [L15](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Base/JsonFields.h:15>) | `const Result<JsonReader> field = TryField(_object, _key);` |
| [L18](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Base/JsonFields.h:18>) | `return field.GetError();` |
| [L20](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Base/JsonFields.h:20>) | `return field->Read<T>();` |
| [L24](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Base/JsonFields.h:24>) | `[[nodiscard]] bool ReadInt(const JsonReader& _object, StringView _key, int& _out);` |
| [L26](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Base/JsonFields.h:26>) | `[[nodiscard]] bool ReadNumberField(const JsonReader& _object, StringView _key, bool _bRequired, double& _out, StringView& _reason, bool* _pFound = nullptr);` |
| [L27](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Base/JsonFields.h:27>) | `[[nodiscard]] bool ReadFloatField(const JsonReader& _object, StringView _key, bool _bRequired, float& _out, StringView& _reason);` |
| [L28](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Base/JsonFields.h:28>) | `[[nodiscard]] bool ReadBoolField(const JsonReader& _object, StringView _key, bool _bRequired, bool& _out, StringView& _reason, bool* _pFound = nullptr);` |
| [L29](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Base/JsonFields.h:29>) | `[[nodiscard]] bool ReadStringField(const JsonReader& _object, StringView _key, bool _bRequired, String& _out, StringView& _reason, bool* _pFound = nullptr);` |
| [L30](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Base/JsonFields.h:30>) | `[[nodiscard]] bool ReadVec2Field(const JsonReader& _object, StringView _key, bool _bRequired, VECTOR2& _out, StringView& _reason, bool* _pFound = nullptr);` |
| [L33](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Base/JsonFields.h:33>) | `[[nodiscard]] bool CheckJsonKeys(const JsonReader& _object, Span<const StringView> _allowed, StringView& _badKey);` |

## Source/Base/Log.cpp

[Log.cpp](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Base/Log.cpp>) · 113줄

| 위치 | 선언·함수 |
|---|---|
| [L6](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Base/Log.cpp:6>) | `void LogMessage( const eLogLevel _level, const StringView _message)` |
| [L67](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Base/Log.cpp:67>) | `void InstallCoreLogger()` |
| [L78](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Base/Log.cpp:78>) | `bool OpenEventLog( const StringView _path)` |
| [L87](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Base/Log.cpp:87>) | `bool IsEventLogOpen()` |
| [L92](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Base/Log.cpp:92>) | `void SetEventLogTick( const unsigned long long _tick)` |
| [L98](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Base/Log.cpp:98>) | `void WriteEventLog( const StringView _event, const StringView _fields)` |

## Source/Base/Log.h

[Log.h](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Base/Log.h>) · 35줄

| 위치 | 선언·함수 |
|---|---|
| [L6](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Base/Log.h:6>) | `void LogMessage(eLogLevel _level, StringView _message);` |
| [L9](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Base/Log.h:9>) | `void InstallCoreLogger();` |
| [L14](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Base/Log.h:14>) | `LogMessage(_level, std::format(_format, std::forward<Args>(_args)...));` |
| [L17](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Base/Log.h:17>) | `bool OpenEventLog(StringView _path);` |
| [L18](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Base/Log.h:18>) | `bool IsEventLogOpen();` |
| [L19](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Base/Log.h:19>) | `void SetEventLogTick(unsigned long long _tick);` |
| [L20](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Base/Log.h:20>) | `void WriteEventLog(StringView _event, StringView _fields);` |
| [L27](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Base/Log.h:27>) | `WriteEventLog(_event, std::format(_fields, std::forward<Args>(_args)...));` |

## Source/Base/Pch.cpp

[Pch.cpp](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Base/Pch.cpp>) · 1줄

PCH·매크로 구현 또는 자동 추출 대상 외 구문이다. 파일 본문을 확인한다.

## Source/Base/Pch.h

[Pch.h](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Base/Pch.h>) · 10줄

PCH·매크로 구현 또는 자동 추출 대상 외 구문이다. 파일 본문을 확인한다.

## Source/Base/SlugError.cpp

[SlugError.cpp](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Base/SlugError.cpp>) · 38줄

| 위치 | 선언·함수 |
|---|---|
| [L3](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Base/SlugError.cpp:3>) | `StringView SlugErrorCategory::GetName() const noexcept` |
| [L8](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Base/SlugError.cpp:8>) | `String SlugErrorCategory::MakeMessage( const int _err) const` |
| [L29](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Base/SlugError.cpp:29>) | `bool HasErrorCode( const Error& _error, const eError _code)` |

## Source/Base/SlugError.h

[SlugError.h](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Base/SlugError.h>) · 28줄

| 위치 | 선언·함수 |
|---|---|
| [L3](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Base/SlugError.h:3>) | `enum class eError` |
| [L19](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Base/SlugError.h:19>) | `class SlugErrorCategory : public jug::IErrorCategory` |
| [L22](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Base/SlugError.h:22>) | `[[nodiscard]] StringView GetName() const noexcept override;` |
| [L23](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Base/SlugError.h:23>) | `[[nodiscard]] String     MakeMessage(int _err) const override;` |
| [L26](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Base/SlugError.h:26>) | `JUG_DEFINE_ERROR_ENUM(eError, SlugErrorCategory);` |
| [L28](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Base/SlugError.h:28>) | `[[nodiscard]] bool HasErrorCode(const Error& _error, eError _code);` |

## Source/Base/Types.h

[Types.h](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Base/Types.h>) · 29줄

| 위치 | 선언·함수 |
|---|---|
| [L25](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Base/Types.h:25>) | `struct EnemyPartDepth` |

## Source/Core/App.cpp

[App.cpp](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Core/App.cpp>) · 533줄

| 위치 | 선언·함수 |
|---|---|
| [L19](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Core/App.cpp:19>) | `[[nodiscard]] int ChooseWindowScale_()` |
| [L37](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Core/App.cpp:37>) | `int App::Run( const int _argc, char** _argv)` |
| [L74](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Core/App.cpp:74>) | `bool App::InitSystems_()` |
| [L88](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Core/App.cpp:88>) | `void App::PollEvents_()` |
| [L120](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Core/App.cpp:120>) | `void App::ParseArgs_()` |
| [L130](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Core/App.cpp:130>) | `else if (args[i] == "--seed" && bHasValue)` |
| [L134](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Core/App.cpp:134>) | `else if (args[i] == "--editor")` |
| [L142](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Core/App.cpp:142>) | `else if (args[i] == "--editor-anim" && bHasValue)` |
| [L146](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Core/App.cpp:146>) | `else if (args[i] == "--editor-tab" && bHasValue)` |
| [L150](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Core/App.cpp:150>) | `else if (args[i] == "--editor-stage" && bHasValue)` |
| [L154](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Core/App.cpp:154>) | `else if (args[i] == "--editor-god" && bHasValue)` |
| [L158](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Core/App.cpp:158>) | `else if (args[i] == "--editor-lab-spawn" && bHasValue)` |
| [L162](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Core/App.cpp:162>) | `else if (args[i] == "--no-audio")` |
| [L166](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Core/App.cpp:166>) | `else if (args[i] == "--no-async-load")` |
| [L170](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Core/App.cpp:170>) | `else if (args[i] == "--trace-load")` |
| [L174](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Core/App.cpp:174>) | `else if (args[i] == "--capture" && bHasValue)` |
| [L178](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Core/App.cpp:178>) | `else if (args[i] == "--event-log" && bHasValue)` |
| [L185](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Core/App.cpp:185>) | `else if (args[i] == "--capture-frame" && bHasValue)` |
| [L189](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Core/App.cpp:189>) | `else if (args[i] == "--weapon" && bHasValue)` |
| [L193](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Core/App.cpp:193>) | `else if (args[i] == "--hold" && bHasValue)` |
| [L200](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Core/App.cpp:200>) | `else if (name == "K")` |
| [L204](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Core/App.cpp:204>) | `else if (name == "Up")` |
| [L208](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Core/App.cpp:208>) | `else if (name == "Down")` |
| [L212](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Core/App.cpp:212>) | `else if (name == "Left")` |
| [L216](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Core/App.cpp:216>) | `else if (name == "Right")` |
| [L221](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Core/App.cpp:221>) | `else if (args[i] == "--warp" && bHasValue)` |
| [L226](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Core/App.cpp:226>) | `else if (args[i] == "--warp-x" && bHasValue)` |
| [L230](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Core/App.cpp:230>) | `else if (args[i] == "--life" && bHasValue)` |
| [L234](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Core/App.cpp:234>) | `else if (args[i] == "--warp-boss")` |
| [L239](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Core/App.cpp:239>) | `else if (args[i] == "--capture-ticks" && bHasValue)` |
| [L261](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Core/App.cpp:261>) | `void App::Init_()` |
| [L298](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Core/App.cpp:298>) | `else if (m_bEditor && !m_editorAnim.empty())` |
| [L310](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Core/App.cpp:310>) | `else if (m_bEditor && m_editorGod == "invincible")` |
| [L357](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Core/App.cpp:357>) | `void App::Shutdown_()` |
| [L366](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Core/App.cpp:366>) | `void App::OnKeyDown_( const SDL_Scancode _key, const bool _bRepeat)` |
| [L412](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Core/App.cpp:412>) | `void App::OnKeyUp_( const SDL_Scancode _key)` |
| [L418](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Core/App.cpp:418>) | `void App::OnFocusLost_()` |
| [L423](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Core/App.cpp:423>) | `void App::Update_( const float _deltaTimeSec)` |
| [L455](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Core/App.cpp:455>) | `else if (g_debug.bPaused)` |
| [L483](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Core/App.cpp:483>) | `void App::Tick_()` |
| [L494](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Core/App.cpp:494>) | `void App::Render_( const float _alpha)` |
| [L522](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Core/App.cpp:522>) | `else if (!m_capturePath.empty() && m_renderFrameCount == m_captureFrame)` |

## Source/Core/App.h

[App.h](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Core/App.h>) · 65줄

| 위치 | 선언·함수 |
|---|---|
| [L15](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Core/App.h:15>) | `class App` |
| [L18](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Core/App.h:18>) | `int Run(int _argc, char** _argv);` |
| [L21](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Core/App.h:21>) | `[[nodiscard]] bool InitSystems_();` |
| [L22](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Core/App.h:22>) | `void               Init_();` |
| [L23](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Core/App.h:23>) | `void               Shutdown_();` |
| [L24](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Core/App.h:24>) | `void               PollEvents_();` |
| [L25](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Core/App.h:25>) | `void               OnKeyDown_(SDL_Scancode _key, bool _bRepeat);` |
| [L26](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Core/App.h:26>) | `void               OnKeyUp_(SDL_Scancode _key);` |
| [L27](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Core/App.h:27>) | `void               OnFocusLost_();` |
| [L28](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Core/App.h:28>) | `void               Update_(float _deltaTimeSec);` |
| [L29](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Core/App.h:29>) | `void               ParseArgs_();` |
| [L30](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Core/App.h:30>) | `void               Tick_();` |
| [L31](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Core/App.h:31>) | `void               Render_(float _alpha);` |

## Source/Core/AssetPath.cpp

[AssetPath.cpp](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Core/AssetPath.cpp>) · 127줄

| 위치 | 선언·함수 |
|---|---|
| [L19](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Core/AssetPath.cpp:19>) | `std::optional<FilePath>& FallbackRoot_()` |
| [L25](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Core/AssetPath.cpp:25>) | `FilePath ExecutableDir_()` |
| [L40](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Core/AssetPath.cpp:40>) | `bool IsAssetRoot_( const FilePath& _assets)` |
| [L48](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Core/AssetPath.cpp:48>) | `[[nodiscard]] Result<Container> ReadAll_( const FilePath& _path)` |
| [L71](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Core/AssetPath.cpp:71>) | `FilePath FindAssetRoot()` |
| [L95](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Core/AssetPath.cpp:95>) | `void SetAssetRootFallback( FilePath _root)` |
| [L101](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Core/AssetPath.cpp:101>) | `Result<Vector<uint8_t>> ReadFileBytes( const FilePath& _path)` |
| [L107](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Core/AssetPath.cpp:107>) | `Result<String> ReadFileText( const FilePath& _path)` |
| [L113](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Core/AssetPath.cpp:113>) | `Result<void> WriteFileBytes( const FilePath& _path, const MemoryView _bytes)` |

## Source/Core/AssetPath.h

[AssetPath.h](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Core/AssetPath.h>) · 10줄

| 위치 | 선언·함수 |
|---|---|
| [L4](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Core/AssetPath.h:4>) | `[[nodiscard]] FilePath FindAssetRoot();` |
| [L6](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Core/AssetPath.h:6>) | `void SetAssetRootFallback(FilePath _root);` |
| [L8](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Core/AssetPath.h:8>) | `[[nodiscard]] Result<Vector<uint8_t>> ReadFileBytes(const FilePath& _path);` |
| [L9](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Core/AssetPath.h:9>) | `[[nodiscard]] Result<String>          ReadFileText(const FilePath& _path);` |
| [L10](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Core/AssetPath.h:10>) | `[[nodiscard]] Result<void>            WriteFileBytes(const FilePath& _path, MemoryView _bytes);` |

## Source/Core/FixedStep.cpp

[FixedStep.cpp](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Core/FixedStep.cpp>) · 31줄

| 위치 | 선언·함수 |
|---|---|
| [L3](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Core/FixedStep.cpp:3>) | `int FixedStepClock::Advance( const double _deltaTimeSec)` |
| [L23](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Core/FixedStep.cpp:23>) | `float FixedStepClock::GetAlpha() const` |
| [L28](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Core/FixedStep.cpp:28>) | `void FixedStepClock::Reset()` |

## Source/Core/FixedStep.h

[FixedStep.h](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Core/FixedStep.h>) · 16줄

| 위치 | 선언·함수 |
|---|---|
| [L3](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Core/FixedStep.h:3>) | `class FixedStepClock` |
| [L10](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Core/FixedStep.h:10>) | `[[nodiscard]] int   Advance(double _deltaTimeSec);` |
| [L11](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Core/FixedStep.h:11>) | `[[nodiscard]] float GetAlpha() const;` |
| [L12](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Core/FixedStep.h:12>) | `void                Reset();` |

## Source/Core/Input.cpp

[Input.cpp](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Core/Input.cpp>) · 144줄

| 위치 | 선언·함수 |
|---|---|
| [L30](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Core/Input.cpp:30>) | `void InputSystem::OnKeyDown( const SDL_Scancode _key)` |
| [L40](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Core/Input.cpp:40>) | `void InputSystem::OnKeyUp( const SDL_Scancode _key)` |
| [L50](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Core/Input.cpp:50>) | `void InputSystem::ClearAll()` |
| [L55](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Core/Input.cpp:55>) | `bool InputSystem::IsKeyDown_( const SDL_Scancode _key) const` |
| [L62](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Core/Input.cpp:62>) | `InputFrame InputSystem::Sample() const` |
| [L75](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Core/Input.cpp:75>) | `void InputHistory::Push( const InputFrame _frame)` |
| [L83](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Core/Input.cpp:83>) | `void InputHistory::Clear()` |
| [L90](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Core/Input.cpp:90>) | `const InputFrame& InputHistory::GetCurrent() const` |
| [L95](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Core/Input.cpp:95>) | `bool InputHistory::IsHeld( const eAction _action) const` |
| [L101](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Core/Input.cpp:101>) | `bool InputHistory::WasPressed( const eAction _action) const` |
| [L107](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Core/Input.cpp:107>) | `bool InputHistory::WasPressedWithin( const eAction _action, const int _frames) const` |
| [L126](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Core/Input.cpp:126>) | `bool InputFrame::IsHeld( const eAction _action) const` |
| [L132](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Core/Input.cpp:132>) | `void InputFrame::Set( const eAction _action, const bool _bHeld)` |

## Source/Core/Input.h

[Input.h](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Core/Input.h>) · 63줄

| 위치 | 선언·함수 |
|---|---|
| [L8](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Core/Input.h:8>) | `enum class eAction : uint16_t` |
| [L22](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Core/Input.h:22>) | `struct InputFrame` |
| [L26](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Core/Input.h:26>) | `[[nodiscard]] bool           IsHeld(eAction _action) const;` |
| [L27](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Core/Input.h:27>) | `void                         Set(eAction _action, bool _bHeld);` |
| [L28](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Core/Input.h:28>) | `[[nodiscard]] constexpr bool operator==(const InputFrame&) const = default;` |
| [L31](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Core/Input.h:31>) | `class InputSystem` |
| [L34](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Core/Input.h:34>) | `void                     OnKeyDown(SDL_Scancode _key);` |
| [L35](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Core/Input.h:35>) | `void                     OnKeyUp(SDL_Scancode _key);` |
| [L36](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Core/Input.h:36>) | `void                     ClearAll();` |
| [L37](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Core/Input.h:37>) | `[[nodiscard]] InputFrame Sample() const;` |
| [L42](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Core/Input.h:42>) | `[[nodiscard]] bool IsKeyDown_(SDL_Scancode _key) const;` |
| [L47](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Core/Input.h:47>) | `class InputHistory` |
| [L52](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Core/Input.h:52>) | `void                            Push(InputFrame _frame);` |
| [L53](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Core/Input.h:53>) | `void                            Clear();` |
| [L54](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Core/Input.h:54>) | `[[nodiscard]] const InputFrame& GetCurrent() const;` |
| [L55](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Core/Input.h:55>) | `[[nodiscard]] bool              IsHeld(eAction _action) const;` |
| [L56](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Core/Input.h:56>) | `[[nodiscard]] bool              WasPressed(eAction _action) const;` |
| [L57](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Core/Input.h:57>) | `[[nodiscard]] bool              WasPressedWithin(eAction _action, int _frames) const;` |

## Source/Core/Tuning.cpp

[Tuning.cpp](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Core/Tuning.cpp>) · 534줄

| 위치 | 선언·함수 |
|---|---|
| [L16](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Core/Tuning.cpp:16>) | `void VisitWeapon_( V& _v, W& _w)` |
| [L54](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Core/Tuning.cpp:54>) | `void VisitMuzzleSet_( V& _v, S& _s)` |
| [L66](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Core/Tuning.cpp:66>) | `void Visit_( V& _v, T& _t)` |
| [L209](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Core/Tuning.cpp:209>) | `explicit TuningReadVisitor( const JsonReader& _root)` |
| [L216](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Core/Tuning.cpp:216>) | `void Begin( const StringView _key)` |
| [L234](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Core/Tuning.cpp:234>) | `void End()` |
| [L239](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Core/Tuning.cpp:239>) | `void Field( const StringView _key, float& _value)` |
| [L250](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Core/Tuning.cpp:250>) | `void Field( const StringView _key, int& _value)` |
| [L261](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Core/Tuning.cpp:261>) | `void Field( const StringView _key, bool& _value)` |
| [L279](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Core/Tuning.cpp:279>) | `void Field( const StringView _key, VECTOR2& _value)` |
| [L310](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Core/Tuning.cpp:310>) | `[[nodiscard]] bool HasError() const` |
| [L313](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Core/Tuning.cpp:313>) | `bool ReadNumber_( const StringView _key, double& _out)` |
| [L339](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Core/Tuning.cpp:339>) | `void Begin( const StringView _key)` |
| [L345](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Core/Tuning.cpp:345>) | `void End()` |
| [L350](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Core/Tuning.cpp:350>) | `void Field( const StringView _key, const float _value)` |
| [L357](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Core/Tuning.cpp:357>) | `void Field( const StringView _key, const int _value)` |
| [L364](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Core/Tuning.cpp:364>) | `void Field( const StringView _key, const bool _value)` |
| [L371](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Core/Tuning.cpp:371>) | `void Field( const StringView _key, const VECTOR2 _value)` |
| [L378](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Core/Tuning.cpp:378>) | `[[nodiscard]] const String& GetText() const` |
| [L381](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Core/Tuning.cpp:381>) | `void Line_( const StringView _key, const String& _value)` |
| [L402](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Core/Tuning.cpp:402>) | `[[nodiscard]] Result<Tuning> ParseRoot_( const JsonReader& _root)` |
| [L416](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Core/Tuning.cpp:416>) | `Result<Tuning> ParseTuning( const StringView _json)` |
| [L427](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Core/Tuning.cpp:427>) | `Result<Tuning> LoadTuning( const FilePath& _path)` |
| [L438](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Core/Tuning.cpp:438>) | `String DumpTuning( const Tuning& _tuning)` |
| [L446](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Core/Tuning.cpp:446>) | `FilePath GetDefaultTuningPath( const FilePath& _assetRoot)` |
| [L452](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Core/Tuning.cpp:452>) | `void TuningWatcher::Init( FilePath _path, const int _pollIntervalTicks)` |
| [L471](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Core/Tuning.cpp:471>) | `bool TuningWatcher::Poll( Tuning& _out)` |
| [L505](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Core/Tuning.cpp:505>) | `int WeaponsTuning::GetPickupAmmo( const eWeapon _weapon) const` |
| [L518](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Core/Tuning.cpp:518>) | `const WeaponTuning& WeaponsTuning::Get( const eWeapon _weapon) const` |
| [L531](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Core/Tuning.cpp:531>) | `int TuningWatcher::GetReloadCount() const` |

## Source/Core/Tuning.h

[Tuning.h](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Core/Tuning.h>) · 310줄

| 위치 | 선언·함수 |
|---|---|
| [L7](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Core/Tuning.h:7>) | `enum class eWeapon` |
| [L17](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Core/Tuning.h:17>) | `struct MoveTuning` |
| [L34](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Core/Tuning.h:34>) | `struct BodyTuning` |
| [L43](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Core/Tuning.h:43>) | `struct MuzzleSet` |
| [L52](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Core/Tuning.h:52>) | `struct MuzzleTuning` |
| [L71](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Core/Tuning.h:71>) | `struct WeaponTuning` |
| [L106](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Core/Tuning.h:106>) | `struct WeaponsTuning` |
| [L187](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Core/Tuning.h:187>) | `[[nodiscard]] int                 GetPickupAmmo(eWeapon _weapon) const;` |
| [L188](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Core/Tuning.h:188>) | `[[nodiscard]] const WeaponTuning& Get(eWeapon _weapon) const;` |
| [L191](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Core/Tuning.h:191>) | `struct KnifeTuning` |
| [L205](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Core/Tuning.h:205>) | `struct BombTuning` |
| [L224](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Core/Tuning.h:224>) | `struct PlayerTuning` |
| [L242](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Core/Tuning.h:242>) | `struct CameraSettings` |
| [L249](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Core/Tuning.h:249>) | `struct SandboxTuning` |
| [L261](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Core/Tuning.h:261>) | `struct ItemTuning` |
| [L268](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Core/Tuning.h:268>) | `struct RigTuning` |
| [L274](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Core/Tuning.h:274>) | `struct Tuning` |
| [L289](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Core/Tuning.h:289>) | `[[nodiscard]] Result<Tuning> ParseTuning(StringView _json);` |
| [L290](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Core/Tuning.h:290>) | `[[nodiscard]] Result<Tuning> LoadTuning(const FilePath& _path);` |
| [L292](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Core/Tuning.h:292>) | `[[nodiscard]] String DumpTuning(const Tuning& _tuning);` |
| [L294](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Core/Tuning.h:294>) | `[[nodiscard]] FilePath GetDefaultTuningPath(const FilePath& _assetRoot);` |
| [L296](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Core/Tuning.h:296>) | `class TuningWatcher` |
| [L299](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Core/Tuning.h:299>) | `void              Init(FilePath _path, int _pollIntervalTicks = 60);` |
| [L300](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Core/Tuning.h:300>) | `bool              Poll(Tuning& _out);` |
| [L301](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Core/Tuning.h:301>) | `[[nodiscard]] int GetReloadCount() const;` |

## Source/Editor/AnimDocument.cpp

[AnimDocument.cpp](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Editor/AnimDocument.cpp>) · 318줄

| 위치 | 선언·함수 |
|---|---|
| [L12](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Editor/AnimDocument.cpp:12>) | `[[nodiscard]] Result<AnimFrame> ParseFrame_( const JsonReader& _frame)` |
| [L34](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Editor/AnimDocument.cpp:34>) | `[[nodiscard]] Result<AnimEditClip> ParseClip_( const StringView _name, const JsonReader& _clip)` |
| [L92](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Editor/AnimDocument.cpp:92>) | `void AppendNumber_( String& _out, const double _value)` |
| [L101](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Editor/AnimDocument.cpp:101>) | `void AppendString_( String& _out, const StringView _text)` |
| [L117](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Editor/AnimDocument.cpp:117>) | `void AppendField_( String& _out, const StringView _key, const int _value)` |
| [L126](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Editor/AnimDocument.cpp:126>) | `Result<void> AnimDocument::LoadFromText( const StringView _json)` |
| [L166](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Editor/AnimDocument.cpp:166>) | `Result<void> AnimDocument::Load( const FilePath& _path)` |
| [L177](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Editor/AnimDocument.cpp:177>) | `String AnimDocument::Serialize() const` |
| [L211](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Editor/AnimDocument.cpp:211>) | `Result<void> AnimDocument::Save( const FilePath& _path) const` |
| [L218](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Editor/AnimDocument.cpp:218>) | `int AnimDocument::FindClip( const StringView _name) const` |
| [L231](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Editor/AnimDocument.cpp:231>) | `void AnimDocument::PushUndo_( const int _clip)` |
| [L247](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Editor/AnimDocument.cpp:247>) | `void AnimDocument::SetPivot( const int _clip, const int _frame, const int _pivotX, const int _pivotY)` |
| [L269](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Editor/AnimDocument.cpp:269>) | `void AnimDocument::ApplyPivotToAll( const int _clip, const int _pivotX, const int _pivotY)` |
| [L282](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Editor/AnimDocument.cpp:282>) | `void AnimDocument::SetFrameDuration( const int _clip, const double _duration)` |
| [L290](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Editor/AnimDocument.cpp:290>) | `void AnimDocument::SetLoop( const int _clip, const bool _bLoop)` |
| [L298](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Editor/AnimDocument.cpp:298>) | `void AnimDocument::SetLoopStart( const int _clip, const int _loopStart)` |
| [L306](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Editor/AnimDocument.cpp:306>) | `void AnimDocument::Undo()` |

## Source/Editor/AnimDocument.h

[AnimDocument.h](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Editor/AnimDocument.h>) · 59줄

| 위치 | 선언·함수 |
|---|---|
| [L5](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Editor/AnimDocument.h:5>) | `struct AnimEditClip` |
| [L19](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Editor/AnimDocument.h:19>) | `class AnimDocument` |
| [L22](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Editor/AnimDocument.h:22>) | `[[nodiscard]] Result<void>             Load(const FilePath& _path);` |
| [L23](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Editor/AnimDocument.h:23>) | `[[nodiscard]] Result<void>             Save(const FilePath& _path) const;` |
| [L24](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Editor/AnimDocument.h:24>) | `[[nodiscard]] Result<void>             LoadFromText(StringView _json);` |
| [L25](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Editor/AnimDocument.h:25>) | `[[nodiscard]] String                   Serialize() const;` |
| [L26](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Editor/AnimDocument.h:26>) | `[[nodiscard]] Span<const AnimEditClip> GetClips() const;` |
| [L27](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Editor/AnimDocument.h:27>) | `[[nodiscard]] int                      FindClip(StringView _name) const;` |
| [L28](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Editor/AnimDocument.h:28>) | `[[nodiscard]] bool                     IsDirty() const;` |
| [L29](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Editor/AnimDocument.h:29>) | `[[nodiscard]] bool                     CanUndo() const;` |
| [L30](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Editor/AnimDocument.h:30>) | `void                                   SetPivot(int _clip, int _frame, int _pivotX, int _pivotY);` |
| [L31](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Editor/AnimDocument.h:31>) | `void                                   ApplyPivotToAll(int _clip, int _pivotX, int _pivotY);` |
| [L32](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Editor/AnimDocument.h:32>) | `void                                   SetFrameDuration(int _clip, double _duration);` |
| [L33](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Editor/AnimDocument.h:33>) | `void                                   SetLoop(int _clip, bool _bLoop);` |
| [L34](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Editor/AnimDocument.h:34>) | `void                                   SetLoopStart(int _clip, int _loopStart);` |
| [L35](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Editor/AnimDocument.h:35>) | `void                                   Undo();` |
| [L36](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Editor/AnimDocument.h:36>) | `void                                   ClearDirty();` |
| [L37](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Editor/AnimDocument.h:37>) | `void                                   ApplyClipTo(int _clip, AnimationLibrary& _library) const;` |
| [L38](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Editor/AnimDocument.h:38>) | `void                                   ApplyTo(AnimationLibrary& _library) const;` |
| [L39](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Editor/AnimDocument.h:39>) | `[[nodiscard]] int                      GetRevision() const;` |
| [L40](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Editor/AnimDocument.h:40>) | `[[nodiscard]] int                      GetLastChangedClip() const;` |
| [L43](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Editor/AnimDocument.h:43>) | `struct UndoEntry` |
| [L49](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Editor/AnimDocument.h:49>) | `void PushUndo_(int _clip);` |

## Source/Editor/AnimEditor.cpp

[AnimEditor.cpp](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Editor/AnimEditor.cpp>) · 605줄

| 위치 | 선언·함수 |
|---|---|
| [L17](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Editor/AnimEditor.cpp:17>) | `void AnimEditor::Init( SDL_Renderer* _pRenderer)` |
| [L24](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Editor/AnimEditor.cpp:24>) | `void AnimEditor::Open( const FilePath& _path)` |
| [L35](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Editor/AnimEditor.cpp:35>) | `void AnimEditor::SetLibrary( AnimationLibrary* _pLibrary)` |
| [L41](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Editor/AnimEditor.cpp:41>) | `AnimDocument& AnimEditor::GetDocument()` |
| [L46](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Editor/AnimEditor.cpp:46>) | `const String& AnimEditor::GetStatus() const` |
| [L51](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Editor/AnimEditor.cpp:51>) | `void AnimEditor::RememberMtime_()` |
| [L57](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Editor/AnimEditor.cpp:57>) | `void AnimEditor::SyncLibrary_()` |
| [L79](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Editor/AnimEditor.cpp:79>) | `void AnimEditor::Poll()` |
| [L91](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Editor/AnimEditor.cpp:91>) | `bool AnimEditor::CheckExternal()` |
| [L109](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Editor/AnimEditor.cpp:109>) | `void AnimEditor::Reload()` |
| [L133](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Editor/AnimEditor.cpp:133>) | `void AnimEditor::Shutdown()` |
| [L148](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Editor/AnimEditor.cpp:148>) | `void AnimEditor::SelectClip( const StringView _name)` |
| [L159](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Editor/AnimEditor.cpp:159>) | `SDL_Texture* AnimEditor::GetPage_( const int _atlas)` |
| [L201](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Editor/AnimEditor.cpp:201>) | `void AnimEditor::Save_()` |
| [L216](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Editor/AnimEditor.cpp:216>) | `void AnimEditor::Draw()` |
| [L241](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Editor/AnimEditor.cpp:241>) | `void AnimEditor::DrawList_()` |
| [L265](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Editor/AnimEditor.cpp:265>) | `void AnimEditor::DrawDetail_()` |
| [L354](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Editor/AnimEditor.cpp:354>) | `else if (clip.bLoop)` |
| [L426](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Editor/AnimEditor.cpp:426>) | `void AnimEditor::DrawCanvas_( const AnimEditClip& _clip)` |
| [L444](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Editor/AnimEditor.cpp:444>) | `else if (wheel < 0.f)` |
| [L496](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Editor/AnimEditor.cpp:496>) | `void AnimEditor::HandleKeys_( const int _clipIndex)` |
| [L544](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Editor/AnimEditor.cpp:544>) | `Span<const AnimEditClip> AnimDocument::GetClips() const` |
| [L549](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Editor/AnimEditor.cpp:549>) | `bool AnimDocument::IsDirty() const` |
| [L554](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Editor/AnimEditor.cpp:554>) | `bool AnimDocument::CanUndo() const` |
| [L559](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Editor/AnimEditor.cpp:559>) | `void AnimDocument::ClearDirty()` |
| [L564](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Editor/AnimEditor.cpp:564>) | `int AnimDocument::GetRevision() const` |
| [L569](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Editor/AnimEditor.cpp:569>) | `int AnimDocument::GetLastChangedClip() const` |
| [L574](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Editor/AnimEditor.cpp:574>) | `void AnimDocument::ApplyClipTo( const int _clip, AnimationLibrary& _library) const` |
| [L598](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Editor/AnimEditor.cpp:598>) | `void AnimDocument::ApplyTo( AnimationLibrary& _library) const` |

## Source/Editor/AnimEditor.h

[AnimEditor.h](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Editor/AnimEditor.h>) · 60줄

| 위치 | 선언·함수 |
|---|---|
| [L9](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Editor/AnimEditor.h:9>) | `struct SDL_Renderer;` |
| [L10](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Editor/AnimEditor.h:10>) | `struct SDL_Texture;` |
| [L14](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Editor/AnimEditor.h:14>) | `class AnimEditor` |
| [L17](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Editor/AnimEditor.h:17>) | `void                        Init(SDL_Renderer* _pRenderer);` |
| [L18](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Editor/AnimEditor.h:18>) | `void                        Shutdown();` |
| [L19](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Editor/AnimEditor.h:19>) | `void                        SelectClip(StringView _name);` |
| [L20](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Editor/AnimEditor.h:20>) | `void                        SetLibrary(AnimationLibrary* _pLibrary);` |
| [L21](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Editor/AnimEditor.h:21>) | `void                        Open(const FilePath& _path);` |
| [L22](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Editor/AnimEditor.h:22>) | `void                        Poll();` |
| [L23](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Editor/AnimEditor.h:23>) | `[[nodiscard]] bool          CheckExternal();` |
| [L24](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Editor/AnimEditor.h:24>) | `void                        Reload();` |
| [L25](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Editor/AnimEditor.h:25>) | `[[nodiscard]] AnimDocument& GetDocument();` |
| [L26](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Editor/AnimEditor.h:26>) | `[[nodiscard]] const String& GetStatus() const;` |
| [L29](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Editor/AnimEditor.h:29>) | `void Draw();` |
| [L32](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Editor/AnimEditor.h:32>) | `void                       DrawList_();` |
| [L33](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Editor/AnimEditor.h:33>) | `void                       DrawDetail_();` |
| [L34](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Editor/AnimEditor.h:34>) | `void                       DrawCanvas_(const AnimEditClip& _clip);` |
| [L35](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Editor/AnimEditor.h:35>) | `void                       HandleKeys_(int _clipIndex);` |
| [L36](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Editor/AnimEditor.h:36>) | `void                       Save_();` |
| [L37](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Editor/AnimEditor.h:37>) | `void                       SyncLibrary_();` |
| [L38](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Editor/AnimEditor.h:38>) | `void                       RememberMtime_();` |
| [L39](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Editor/AnimEditor.h:39>) | `[[nodiscard]] SDL_Texture* GetPage_(int _atlas);` |

## Source/Editor/DepthEditor.cpp

[DepthEditor.cpp](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Editor/DepthEditor.cpp>) · 311줄

| 위치 | 선언·함수 |
|---|---|
| [L27](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Editor/DepthEditor.cpp:27>) | `void AddConst_( Vector<DepthRow>& _rows, const char* _name, const int _value)` |
| [L38](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Editor/DepthEditor.cpp:38>) | `void Collect_( const Vector<JsonNode>& _nodes, const int _index, const String& _path, const int _file, const String& _fileName, Vector<DepthRow>& _out)` |
| [L74](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Editor/DepthEditor.cpp:74>) | `void DepthEditor::Load_( const FilePath& _assetRoot)` |
| [L90](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Editor/DepthEditor.cpp:90>) | `void DepthEditor::Rebuild_()` |
| [L128](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Editor/DepthEditor.cpp:128>) | `void DepthEditor::Patch_( const int _row, const int _value)` |
| [L139](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Editor/DepthEditor.cpp:139>) | `void DepthEditor::PatchPart_( const StringView _def, const StringView _part, const EnemyPartDepth& _value, const EnemyPartDepth& _default)` |
| [L183](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Editor/DepthEditor.cpp:183>) | `void DepthEditor::DrawParts_( EnemyLab& _lab)` |
| [L239](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Editor/DepthEditor.cpp:239>) | `void DepthEditor::Draw( EnemyLab& _lab)` |

## Source/Editor/DepthEditor.h

[DepthEditor.h](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Editor/DepthEditor.h>) · 34줄

| 위치 | 선언·함수 |
|---|---|
| [L4](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Editor/DepthEditor.h:4>) | `class EnemyLab;` |
| [L6](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Editor/DepthEditor.h:6>) | `struct DepthRow` |
| [L15](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Editor/DepthEditor.h:15>) | `class DepthEditor` |
| [L18](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Editor/DepthEditor.h:18>) | `void Draw(EnemyLab& _lab);` |
| [L21](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Editor/DepthEditor.h:21>) | `void Load_(const FilePath& _assetRoot);` |
| [L22](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Editor/DepthEditor.h:22>) | `void Rebuild_();` |
| [L23](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Editor/DepthEditor.h:23>) | `void Patch_(int _row, int _value);` |
| [L24](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Editor/DepthEditor.h:24>) | `void DrawParts_(EnemyLab& _lab);` |
| [L25](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Editor/DepthEditor.h:25>) | `void PatchPart_(StringView _def, StringView _part, const EnemyPartDepth& _value, const EnemyPartDepth& _default);` |

## Source/Editor/Editor.cpp

[Editor.cpp](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Editor/Editor.cpp>) · 252줄

| 위치 | 선언·함수 |
|---|---|
| [L13](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Editor/Editor.cpp:13>) | `bool Editor::Init( SDL_Window* _pWindow, SDL_Renderer* _pRenderer)` |
| [L41](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Editor/Editor.cpp:41>) | `void Editor::Shutdown()` |
| [L58](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Editor/Editor.cpp:58>) | `void Editor::ProcessEvent( const SDL_Event& _event)` |
| [L67](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Editor/Editor.cpp:67>) | `void Editor::SetAnimLibrary( AnimationLibrary* _pAnims)` |
| [L74](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Editor/Editor.cpp:74>) | `void Editor::SelectAnimation( const char* _name)` |
| [L80](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Editor/Editor.cpp:80>) | `void Editor::SelectTab( const char* _name)` |
| [L86](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Editor/Editor.cpp:86>) | `void Editor::SelectStage( const char* _name)` |
| [L92](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Editor/Editor.cpp:92>) | `void Editor::SetGodMode( const eGodMode _mode)` |
| [L98](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Editor/Editor.cpp:98>) | `eGodMode Editor::GetGodMode() const` |
| [L103](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Editor/Editor.cpp:103>) | `EnemyLab& Editor::GetLab()` |
| [L108](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Editor/Editor.cpp:108>) | `void Editor::DrawLabOverlay( Renderer& _renderer)` |
| [L119](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Editor/Editor.cpp:119>) | `bool Editor::OwnsKeyboard() const` |
| [L124](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Editor/Editor.cpp:124>) | `void Editor::Frame( const float _fps, const uint64_t _tick)` |
| [L174](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Editor/Editor.cpp:174>) | `void Editor::DrawMenuBar_()` |
| [L196](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Editor/Editor.cpp:196>) | `void Editor::DrawEditorWindow_( const float _fps, const uint64_t _tick)` |

## Source/Editor/Editor.h

[Editor.h](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Editor/Editor.h>) · 57줄

| 위치 | 선언·함수 |
|---|---|
| [L15](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Editor/Editor.h:15>) | `struct SDL_Window;` |
| [L16](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Editor/Editor.h:16>) | `struct SDL_Renderer;` |
| [L18](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Editor/Editor.h:18>) | `class Renderer;` |
| [L20](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Editor/Editor.h:20>) | `class Editor` |
| [L23](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Editor/Editor.h:23>) | `[[nodiscard]] bool      Init(SDL_Window* _pWindow, SDL_Renderer* _pRenderer);` |
| [L24](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Editor/Editor.h:24>) | `void                    Shutdown();` |
| [L25](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Editor/Editor.h:25>) | `void                    ProcessEvent(const SDL_Event& _event);` |
| [L26](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Editor/Editor.h:26>) | `void                    SelectAnimation(const char* _name);` |
| [L27](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Editor/Editor.h:27>) | `void                    SelectTab(const char* _name);` |
| [L28](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Editor/Editor.h:28>) | `void                    SetAnimLibrary(AnimationLibrary* _pAnims);` |
| [L29](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Editor/Editor.h:29>) | `void                    SelectStage(const char* _name);` |
| [L30](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Editor/Editor.h:30>) | `void                    SetGodMode(eGodMode _mode);` |
| [L31](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Editor/Editor.h:31>) | `[[nodiscard]] eGodMode  GetGodMode() const;` |
| [L32](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Editor/Editor.h:32>) | `[[nodiscard]] EnemyLab& GetLab();` |
| [L33](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Editor/Editor.h:33>) | `void                    DrawLabOverlay(Renderer& _renderer);` |
| [L36](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Editor/Editor.h:36>) | `void               Frame(float _fps, uint64_t _tick);` |
| [L37](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Editor/Editor.h:37>) | `[[nodiscard]] bool OwnsKeyboard() const;` |
| [L40](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Editor/Editor.h:40>) | `void DrawMenuBar_();` |
| [L41](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Editor/Editor.h:41>) | `void DrawEditorWindow_(float _fps, uint64_t _tick);` |

## Source/Editor/EnemyLab.cpp

[EnemyLab.cpp](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Editor/EnemyLab.cpp>) · 697줄

| 위치 | 선언·함수 |
|---|---|
| [L22](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Editor/EnemyLab.cpp:22>) | `void LabDrawText_( Renderer& _renderer, const VECTOR2 _topLeft, const StringView _text, const RGBA _color)` |
| [L32](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Editor/EnemyLab.cpp:32>) | `[[nodiscard]] const char* LabLifeName_( const eEnemyLife _life)` |
| [L45](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Editor/EnemyLab.cpp:45>) | `[[nodiscard]] String LabMakeStateText_( const Enemy& _enemy)` |
| [L53](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Editor/EnemyLab.cpp:53>) | `void EnemyLab::Init( SimRunner* _pRunner, const String& _stateName, const FilePath& _assetRoot)` |
| [L66](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Editor/EnemyLab.cpp:66>) | `void EnemyLab::SetAutoSpawn( const String& _defName)` |
| [L72](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Editor/EnemyLab.cpp:72>) | `EnemyHandle EnemyLab::Spawn( SandboxState& _state, const StringView _defName, const VECTOR2 _feet, const int _facing, const int _z)` |
| [L91](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Editor/EnemyLab.cpp:91>) | `EnemyHandle EnemyLab::SpawnNearPlayer( SandboxState& _state, const StringView _defName, const float _offsetX, const int _facing, const int _z)` |
| [L102](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Editor/EnemyLab.cpp:102>) | `void EnemyLab::KillAll( SandboxState& _state)` |
| [L108](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Editor/EnemyLab.cpp:108>) | `void EnemyLab::Clear( SandboxState& _state)` |
| [L116](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Editor/EnemyLab.cpp:116>) | `int EnemyLab::RespawnLast( SandboxState& _state)` |
| [L133](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Editor/EnemyLab.cpp:133>) | `bool EnemyLab::ReloadDefs( SandboxState& _state)` |
| [L146](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Editor/EnemyLab.cpp:146>) | `bool EnemyLab::ReloadDefsKeep( SandboxState& _state)` |
| [L159](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Editor/EnemyLab.cpp:159>) | `void EnemyLab::SetAutoReloadBlocked( const bool _bBlocked)` |
| [L165](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Editor/EnemyLab.cpp:165>) | `void EnemyLab::SetMouseCaptured( const bool _bCaptured)` |
| [L171](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Editor/EnemyLab.cpp:171>) | `int EnemyLab::GetDefIndex() const` |
| [L176](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Editor/EnemyLab.cpp:176>) | `void EnemyLab::PollDefFiles_( SandboxState& _state)` |
| [L205](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Editor/EnemyLab.cpp:205>) | `void EnemyLab::ForgetSet()` |
| [L210](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Editor/EnemyLab.cpp:210>) | `void EnemyLab::SelectAt( const SandboxState& _state, const VECTOR2 _world)` |
| [L232](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Editor/EnemyLab.cpp:232>) | `EnemyHandle EnemyLab::GetSelected() const` |
| [L237](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Editor/EnemyLab.cpp:237>) | `size_t EnemyLab::GetSetCount() const` |
| [L242](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Editor/EnemyLab.cpp:242>) | `void EnemyLab::SetPaused( const bool _bPaused)` |
| [L248](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Editor/EnemyLab.cpp:248>) | `void EnemyLab::RequestSteps( const int _count)` |
| [L257](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Editor/EnemyLab.cpp:257>) | `void EnemyLab::SetTimeScale( const float _scale)` |
| [L263](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Editor/EnemyLab.cpp:263>) | `float EnemyLab::GetTimeScale() const` |
| [L268](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Editor/EnemyLab.cpp:268>) | `int EnemyLab::TakeSteps( const int _naturalSteps)` |
| [L281](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Editor/EnemyLab.cpp:281>) | `const FilePath& EnemyLab::GetAssetRoot() const` |
| [L286](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Editor/EnemyLab.cpp:286>) | `SandboxState* EnemyLab::GetSandbox() const` |
| [L295](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Editor/EnemyLab.cpp:295>) | `void EnemyLab::EnterLab()` |
| [L310](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Editor/EnemyLab.cpp:310>) | `void EnemyLab::LeaveLab()` |
| [L325](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Editor/EnemyLab.cpp:325>) | `void EnemyLab::Draw()` |
| [L400](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Editor/EnemyLab.cpp:400>) | `void EnemyLab::DrawSpawnUi_( SandboxState& _state)` |
| [L483](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Editor/EnemyLab.cpp:483>) | `void EnemyLab::DrawInspector_( SandboxState& _state)` |
| [L517](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Editor/EnemyLab.cpp:517>) | `void EnemyLab::HandleMouse_( Renderer& _renderer, SandboxState& _state)` |
| [L554](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Editor/EnemyLab.cpp:554>) | `void EnemyLab::DrawMark( Renderer& _renderer, const Aabb& _world, const VECTOR2 _anchor, const RGBA _color, const StringView _label) const` |
| [L580](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Editor/EnemyLab.cpp:580>) | `void EnemyLab::DrawOverlay( Renderer& _renderer)` |

## Source/Editor/EnemyLab.h

[EnemyLab.h](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Editor/EnemyLab.h>) · 86줄

| 위치 | 선언·함수 |
|---|---|
| [L5](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Editor/EnemyLab.h:5>) | `class Renderer;` |
| [L6](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Editor/EnemyLab.h:6>) | `class SandboxState;` |
| [L7](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Editor/EnemyLab.h:7>) | `class SimRunner;` |
| [L9](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Editor/EnemyLab.h:9>) | `struct EnemyLabSpawn` |
| [L17](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Editor/EnemyLab.h:17>) | `class EnemyLab` |
| [L20](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Editor/EnemyLab.h:20>) | `void                          Init(SimRunner* _pRunner, const String& _stateName, const FilePath& _assetRoot);` |
| [L21](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Editor/EnemyLab.h:21>) | `void                          SetAutoSpawn(const String& _defName);` |
| [L22](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Editor/EnemyLab.h:22>) | `EnemyHandle                   Spawn(SandboxState& _state, StringView _defName, VECTOR2 _feet, int _facing, int _z = kDefaultZ);` |
| [L23](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Editor/EnemyLab.h:23>) | `EnemyHandle                   SpawnNearPlayer(SandboxState& _state, StringView _defName, float _offsetX, int _facing, int _z = kDefaultZ);` |
| [L24](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Editor/EnemyLab.h:24>) | `void                          KillAll(SandboxState& _state);` |
| [L25](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Editor/EnemyLab.h:25>) | `void                          Clear(SandboxState& _state);` |
| [L26](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Editor/EnemyLab.h:26>) | `int                           RespawnLast(SandboxState& _state);` |
| [L27](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Editor/EnemyLab.h:27>) | `bool                          ReloadDefs(SandboxState& _state);` |
| [L28](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Editor/EnemyLab.h:28>) | `bool                          ReloadDefsKeep(SandboxState& _state);` |
| [L29](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Editor/EnemyLab.h:29>) | `void                          SetAutoReloadBlocked(bool _bBlocked);` |
| [L30](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Editor/EnemyLab.h:30>) | `void                          SetMouseCaptured(bool _bCaptured);` |
| [L31](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Editor/EnemyLab.h:31>) | `[[nodiscard]] int             GetDefIndex() const;` |
| [L32](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Editor/EnemyLab.h:32>) | `void                          ForgetSet();` |
| [L33](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Editor/EnemyLab.h:33>) | `void                          SelectAt(const SandboxState& _state, VECTOR2 _world);` |
| [L34](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Editor/EnemyLab.h:34>) | `[[nodiscard]] EnemyHandle     GetSelected() const;` |
| [L35](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Editor/EnemyLab.h:35>) | `[[nodiscard]] size_t          GetSetCount() const;` |
| [L36](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Editor/EnemyLab.h:36>) | `[[nodiscard]] const FilePath& GetAssetRoot() const;` |
| [L37](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Editor/EnemyLab.h:37>) | `void                          SetPaused(bool _bPaused);` |
| [L38](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Editor/EnemyLab.h:38>) | `void                          RequestSteps(int _count);` |
| [L39](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Editor/EnemyLab.h:39>) | `void                          SetTimeScale(float _scale);` |
| [L40](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Editor/EnemyLab.h:40>) | `[[nodiscard]] float           GetTimeScale() const;` |
| [L41](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Editor/EnemyLab.h:41>) | `[[nodiscard]] int             TakeSteps(int _naturalSteps);` |
| [L42](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Editor/EnemyLab.h:42>) | `[[nodiscard]] SandboxState*   GetSandbox() const;` |
| [L43](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Editor/EnemyLab.h:43>) | `void                          EnterLab();` |
| [L44](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Editor/EnemyLab.h:44>) | `void                          LeaveLab();` |
| [L45](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Editor/EnemyLab.h:45>) | `void                          Draw();` |
| [L46](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Editor/EnemyLab.h:46>) | `void                          DrawOverlay(Renderer& _renderer);` |
| [L47](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Editor/EnemyLab.h:47>) | `void                          DrawMark(Renderer& _renderer, const Aabb& _world, VECTOR2 _anchor, RGBA _color, StringView _label) const;` |
| [L50](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Editor/EnemyLab.h:50>) | `void DrawSpawnUi_(SandboxState& _state);` |
| [L51](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Editor/EnemyLab.h:51>) | `void DrawInspector_(SandboxState& _state);` |
| [L52](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Editor/EnemyLab.h:52>) | `void HandleMouse_(Renderer& _renderer, SandboxState& _state);` |
| [L53](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Editor/EnemyLab.h:53>) | `void PollDefFiles_(SandboxState& _state);` |

## Source/Editor/GroundEditor.cpp

[GroundEditor.cpp](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Editor/GroundEditor.cpp>) · 986줄

| 위치 | 선언·함수 |
|---|---|
| [L31](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Editor/GroundEditor.cpp:31>) | `void AppendBigEndian32_( Vector<uint8_t>& _out, const uint32_t _value)` |
| [L41](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Editor/GroundEditor.cpp:41>) | `void AppendPngChunk_( Vector<uint8_t>& _out, const char* _type, const Vector<uint8_t>& _body)` |
| [L57](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Editor/GroundEditor.cpp:57>) | `[[nodiscard]] bool EncodePalettePng_( const Vector<uint8_t>& _pixels, const int _width, const int _height, Vector<uint8_t>& _out)` |
| [L99](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Editor/GroundEditor.cpp:99>) | `Result<void> GroundMaskDocument::Load( const FilePath& _path)` |
| [L127](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Editor/GroundEditor.cpp:127>) | `Result<void> GroundMaskDocument::Save( const FilePath& _path)` |
| [L145](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Editor/GroundEditor.cpp:145>) | `void GroundMaskDocument::Create( const int _width, const int _height)` |
| [L162](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Editor/GroundEditor.cpp:162>) | `int GroundMaskDocument::GetWidth() const` |
| [L167](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Editor/GroundEditor.cpp:167>) | `int GroundMaskDocument::GetHeight() const` |
| [L172](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Editor/GroundEditor.cpp:172>) | `bool GroundMaskDocument::IsInside( const int _x, const int _y) const` |
| [L179](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Editor/GroundEditor.cpp:179>) | `bool GroundMaskDocument::IsSolid( const int _x, const int _y) const` |
| [L186](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Editor/GroundEditor.cpp:186>) | `bool GroundMaskDocument::IsDirty() const` |
| [L191](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Editor/GroundEditor.cpp:191>) | `bool GroundMaskDocument::CanUndo() const` |
| [L196](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Editor/GroundEditor.cpp:196>) | `bool GroundMaskDocument::CanRedo() const` |
| [L201](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Editor/GroundEditor.cpp:201>) | `const Vector<uint8_t>& GroundMaskDocument::GetPixels() const` |
| [L206](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Editor/GroundEditor.cpp:206>) | `void GroundMaskDocument::BeginStroke()` |
| [L212](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Editor/GroundEditor.cpp:212>) | `void GroundMaskDocument::EndStroke()` |
| [L229](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Editor/GroundEditor.cpp:229>) | `void GroundMaskDocument::Stamp( const int _x, const int _y, const int _brush, const bool _bSolid)` |
| [L256](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Editor/GroundEditor.cpp:256>) | `void GroundMaskDocument::DrawLine( const int _x0, const int _y0, const int _x1, const int _y1, const int _brush, const bool _bSolid)` |
| [L301](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Editor/GroundEditor.cpp:301>) | `void GroundMaskDocument::FillRect( const int _x0, const int _y0, const int _x1, const int _y1, const bool _bSolid)` |
| [L326](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Editor/GroundEditor.cpp:326>) | `void GroundMaskDocument::FloodFill( const int _x, const int _y, const bool _bSolid)` |
| [L375](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Editor/GroundEditor.cpp:375>) | `void GroundMaskDocument::Undo()` |
| [L389](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Editor/GroundEditor.cpp:389>) | `void GroundMaskDocument::Redo()` |
| [L403](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Editor/GroundEditor.cpp:403>) | `bool GroundMaskDocument::TakeDirtyColumns( int& _x0, int& _x1)` |
| [L418](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Editor/GroundEditor.cpp:418>) | `void GroundMaskDocument::SetPixel_( const int _x, const int _y, const uint8_t _value)` |
| [L437](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Editor/GroundEditor.cpp:437>) | `void GroundMaskDocument::MarkColumn_( const int _x)` |
| [L450](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Editor/GroundEditor.cpp:450>) | `void GroundEditor::Init( SDL_Renderer* _pRenderer)` |
| [L466](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Editor/GroundEditor.cpp:466>) | `void GroundEditor::Shutdown()` |
| [L472](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Editor/GroundEditor.cpp:472>) | `void GroundEditor::ClearTextures_()` |
| [L487](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Editor/GroundEditor.cpp:487>) | `void GroundEditor::LoadMask_( const String& _name)` |
| [L527](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Editor/GroundEditor.cpp:527>) | `void GroundEditor::LoadBackground_( const String& _maskName)` |
| [L588](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Editor/GroundEditor.cpp:588>) | `void GroundEditor::UploadMaskColumns_( const int _x0, const int _x1)` |
| [L620](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Editor/GroundEditor.cpp:620>) | `void GroundEditor::Save_()` |
| [L631](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Editor/GroundEditor.cpp:631>) | `void GroundEditor::Draw()` |
| [L661](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Editor/GroundEditor.cpp:661>) | `void GroundEditor::HandleKeys_()` |
| [L702](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Editor/GroundEditor.cpp:702>) | `void GroundEditor::DrawPanel_()` |
| [L766](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Editor/GroundEditor.cpp:766>) | `void GroundEditor::DrawView_()` |
| [L904](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Editor/GroundEditor.cpp:904>) | `void GroundEditor::HandleCanvasInput_( const int _pixelX, const int _pixelY, const bool _bHovered)` |
| [L971](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Editor/GroundEditor.cpp:971>) | `void GroundEditor::CommitDrag_( const int _pixelX, const int _pixelY)` |
| [L979](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Editor/GroundEditor.cpp:979>) | `else if (m_tool == eGroundTool::Line)` |

## Source/Editor/GroundEditor.h

[GroundEditor.h](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Editor/GroundEditor.h>) · 118줄

| 위치 | 선언·함수 |
|---|---|
| [L7](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Editor/GroundEditor.h:7>) | `struct SDL_Renderer;` |
| [L8](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Editor/GroundEditor.h:8>) | `struct SDL_Texture;` |
| [L10](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Editor/GroundEditor.h:10>) | `struct GroundPixelChange` |
| [L17](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Editor/GroundEditor.h:17>) | `class GroundMaskDocument` |
| [L20](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Editor/GroundEditor.h:20>) | `[[nodiscard]] Result<void>           Load(const FilePath& _path);` |
| [L21](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Editor/GroundEditor.h:21>) | `[[nodiscard]] Result<void>           Save(const FilePath& _path);` |
| [L22](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Editor/GroundEditor.h:22>) | `void                                 Create(int _width, int _height);` |
| [L23](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Editor/GroundEditor.h:23>) | `[[nodiscard]] int                    GetWidth() const;` |
| [L24](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Editor/GroundEditor.h:24>) | `[[nodiscard]] int                    GetHeight() const;` |
| [L25](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Editor/GroundEditor.h:25>) | `[[nodiscard]] bool                   IsInside(int _x, int _y) const;` |
| [L26](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Editor/GroundEditor.h:26>) | `[[nodiscard]] bool                   IsSolid(int _x, int _y) const;` |
| [L27](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Editor/GroundEditor.h:27>) | `[[nodiscard]] bool                   IsDirty() const;` |
| [L28](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Editor/GroundEditor.h:28>) | `[[nodiscard]] bool                   CanUndo() const;` |
| [L29](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Editor/GroundEditor.h:29>) | `[[nodiscard]] bool                   CanRedo() const;` |
| [L30](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Editor/GroundEditor.h:30>) | `[[nodiscard]] const Vector<uint8_t>& GetPixels() const;` |
| [L33](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Editor/GroundEditor.h:33>) | `void               BeginStroke();` |
| [L34](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Editor/GroundEditor.h:34>) | `void               EndStroke();` |
| [L35](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Editor/GroundEditor.h:35>) | `void               Stamp(int _x, int _y, int _brush, bool _bSolid);` |
| [L36](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Editor/GroundEditor.h:36>) | `void               DrawLine(int _x0, int _y0, int _x1, int _y1, int _brush, bool _bSolid);` |
| [L37](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Editor/GroundEditor.h:37>) | `void               FillRect(int _x0, int _y0, int _x1, int _y1, bool _bSolid);` |
| [L38](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Editor/GroundEditor.h:38>) | `void               FloodFill(int _x, int _y, bool _bSolid);` |
| [L39](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Editor/GroundEditor.h:39>) | `void               Undo();` |
| [L40](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Editor/GroundEditor.h:40>) | `void               Redo();` |
| [L41](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Editor/GroundEditor.h:41>) | `[[nodiscard]] bool TakeDirtyColumns(int& _x0, int& _x1);` |
| [L44](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Editor/GroundEditor.h:44>) | `void SetPixel_(int _x, int _y, uint8_t _value);` |
| [L45](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Editor/GroundEditor.h:45>) | `void MarkColumn_(int _x);` |
| [L60](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Editor/GroundEditor.h:60>) | `enum class eGroundTool` |
| [L69](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Editor/GroundEditor.h:69>) | `class GroundEditor` |
| [L72](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Editor/GroundEditor.h:72>) | `void Init(SDL_Renderer* _pRenderer);` |
| [L73](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Editor/GroundEditor.h:73>) | `void Shutdown();` |
| [L74](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Editor/GroundEditor.h:74>) | `void Draw();` |
| [L77](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Editor/GroundEditor.h:77>) | `struct MaskStrip` |
| [L84](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Editor/GroundEditor.h:84>) | `void LoadMask_(const String& _name);` |
| [L85](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Editor/GroundEditor.h:85>) | `void LoadBackground_(const String& _maskName);` |
| [L86](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Editor/GroundEditor.h:86>) | `void ClearTextures_();` |
| [L87](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Editor/GroundEditor.h:87>) | `void UploadMaskColumns_(int _x0, int _x1);` |
| [L88](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Editor/GroundEditor.h:88>) | `void Save_();` |
| [L89](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Editor/GroundEditor.h:89>) | `void DrawPanel_();` |
| [L90](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Editor/GroundEditor.h:90>) | `void DrawView_();` |
| [L91](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Editor/GroundEditor.h:91>) | `void HandleKeys_();` |
| [L92](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Editor/GroundEditor.h:92>) | `void HandleCanvasInput_(int _pixelX, int _pixelY, bool _bHovered);` |
| [L93](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Editor/GroundEditor.h:93>) | `void CommitDrag_(int _pixelX, int _pixelY);` |

## Source/Editor/HitboxDocument.cpp

[HitboxDocument.cpp](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Editor/HitboxDocument.cpp>) · 349줄

| 위치 | 선언·함수 |
|---|---|
| [L9](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Editor/HitboxDocument.cpp:9>) | `bool IsNumberNode_( const Vector<JsonNode>& _nodes, const int _index)` |
| [L16](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Editor/HitboxDocument.cpp:16>) | `int FindKey_( const JsonNode& _object, const StringView _key)` |
| [L30](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Editor/HitboxDocument.cpp:30>) | `double ParseNumber_( const String& _text)` |
| [L38](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Editor/HitboxDocument.cpp:38>) | `String RenderExtraBox_( const ExtraBoxDef& _box)` |
| [L62](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Editor/HitboxDocument.cpp:62>) | `String RenderExtraBoxes_( const Vector<ExtraBoxDef>& _boxes)` |
| [L73](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Editor/HitboxDocument.cpp:73>) | `bool IsSizeKey_( const String& _key)` |
| [L80](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Editor/HitboxDocument.cpp:80>) | `bool HitboxEntry::HasField( const eHitboxField _field) const` |
| [L86](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Editor/HitboxDocument.cpp:86>) | `Result<void> HitboxDocument::Load( const FilePath& _path, const StringView _defaultOwner)` |
| [L98](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Editor/HitboxDocument.cpp:98>) | `Result<void> HitboxDocument::LoadFromText( const StringView _json, const StringView _defaultOwner)` |
| [L115](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Editor/HitboxDocument.cpp:115>) | `Result<void> HitboxDocument::Save( const FilePath& _path)` |
| [L127](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Editor/HitboxDocument.cpp:127>) | `bool HitboxDocument::SetValue( const int _entry, const eHitboxField _field, const double _value)` |
| [L150](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Editor/HitboxDocument.cpp:150>) | `bool HitboxDocument::ApplyExtraBoxes( const StringView _def, const Vector<ExtraBoxDef>& _boxes)` |
| [L227](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Editor/HitboxDocument.cpp:227>) | `void HitboxDocument::Revert()` |
| [L233](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Editor/HitboxDocument.cpp:233>) | `const String& HitboxDocument::GetText() const` |
| [L238](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Editor/HitboxDocument.cpp:238>) | `Span<const HitboxEntry> HitboxDocument::GetEntries() const` |
| [L243](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Editor/HitboxDocument.cpp:243>) | `bool HitboxDocument::IsDirty() const` |
| [L248](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Editor/HitboxDocument.cpp:248>) | `bool HitboxDocument::Reparse_()` |
| [L261](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Editor/HitboxDocument.cpp:261>) | `void HitboxDocument::Collect_( const Vector<JsonNode>& _nodes, const int _index, const String& _path, const String& _owner)` |

## Source/Editor/HitboxDocument.h

[HitboxDocument.h](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Editor/HitboxDocument.h>) · 47줄

| 위치 | 선언·함수 |
|---|---|
| [L6](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Editor/HitboxDocument.h:6>) | `enum class eHitboxField` |
| [L15](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Editor/HitboxDocument.h:15>) | `struct HitboxEntry` |
| [L23](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Editor/HitboxDocument.h:23>) | `[[nodiscard]] bool HasField(eHitboxField _field) const;` |
| [L26](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Editor/HitboxDocument.h:26>) | `class HitboxDocument` |
| [L29](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Editor/HitboxDocument.h:29>) | `[[nodiscard]] Result<void>            Load(const FilePath& _path, StringView _defaultOwner);` |
| [L30](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Editor/HitboxDocument.h:30>) | `[[nodiscard]] Result<void>            LoadFromText(StringView _json, StringView _defaultOwner);` |
| [L31](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Editor/HitboxDocument.h:31>) | `[[nodiscard]] Result<void>            Save(const FilePath& _path);` |
| [L32](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Editor/HitboxDocument.h:32>) | `bool                                  SetValue(int _entry, eHitboxField _field, double _value);` |
| [L33](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Editor/HitboxDocument.h:33>) | `void                                  Revert();` |
| [L34](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Editor/HitboxDocument.h:34>) | `bool                                  ApplyExtraBoxes(StringView _def, const Vector<ExtraBoxDef>& _boxes);` |
| [L35](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Editor/HitboxDocument.h:35>) | `[[nodiscard]] const String&           GetText() const;` |
| [L36](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Editor/HitboxDocument.h:36>) | `[[nodiscard]] Span<const HitboxEntry> GetEntries() const;` |
| [L37](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Editor/HitboxDocument.h:37>) | `[[nodiscard]] bool                    IsDirty() const;` |
| [L40](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Editor/HitboxDocument.h:40>) | `[[nodiscard]] bool Reparse_();` |
| [L41](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Editor/HitboxDocument.h:41>) | `void               Collect_(const Vector<JsonNode>& _nodes, int _index, const String& _path, const String& _owner);` |

## Source/Editor/HitboxEditor.cpp

[HitboxEditor.cpp](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Editor/HitboxEditor.cpp>) · 553줄

| 위치 | 선언·함수 |
|---|---|
| [L42](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Editor/HitboxEditor.cpp:42>) | `HitboxTitle MakeHitboxTitle_( const HitboxEntry& _entry)` |
| [L52](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Editor/HitboxEditor.cpp:52>) | `else if (key == "crouchSize")` |
| [L56](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Editor/HitboxEditor.cpp:56>) | `else if (key == "hurtStand")` |
| [L60](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Editor/HitboxEditor.cpp:60>) | `else if (key == "hurtCrouch")` |
| [L64](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Editor/HitboxEditor.cpp:64>) | `else if (key == "body")` |
| [L68](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Editor/HitboxEditor.cpp:68>) | `else if (key == "hurtbox")` |
| [L72](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Editor/HitboxEditor.cpp:72>) | `else if (key == "hitSize" \|\| key == "hitbox")` |
| [L88](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Editor/HitboxEditor.cpp:88>) | `RGBA HitboxKindColor_( const eHitboxKind _kind)` |
| [L102](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Editor/HitboxEditor.cpp:102>) | `void HitboxEditor::Load_( const FilePath& _assetRoot)` |
| [L117](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Editor/HitboxEditor.cpp:117>) | `void HitboxEditor::ApplyLive_( SandboxState& _state, const HitboxEntry& _entry)` |
| [L132](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Editor/HitboxEditor.cpp:132>) | `else if (_entry.label == "body.crouchSize")` |
| [L136](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Editor/HitboxEditor.cpp:136>) | `else if (_entry.label == "body.hurtStand")` |
| [L140](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Editor/HitboxEditor.cpp:140>) | `else if (_entry.label == "body.hurtCrouch")` |
| [L156](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Editor/HitboxEditor.cpp:156>) | `else if (_entry.label == "defs." + _entry.owner + ".hurtbox")` |
| [L160](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Editor/HitboxEditor.cpp:160>) | `else if (_entry.label == "defs." + _entry.owner + ".params.cannonBoxSize")` |
| [L167](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Editor/HitboxEditor.cpp:167>) | `void HitboxEditor::Draw( EnemyLab& _lab)` |
| [L184](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Editor/HitboxEditor.cpp:184>) | `void HitboxEditor::DrawEntries_( EnemyLab& _lab, SandboxState* _pState)` |
| [L351](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Editor/HitboxEditor.cpp:351>) | `void HitboxEditor::DrawExtraBoxes_( SandboxState& _state, HitboxDocument& _doc)` |
| [L425](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Editor/HitboxEditor.cpp:425>) | `void HitboxEditor::DrawExtraOverlay_( EnemyLab& _lab, Renderer& _renderer)` |
| [L508](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Editor/HitboxEditor.cpp:508>) | `void HitboxEditor::DrawOverlay( EnemyLab& _lab, Renderer& _renderer)` |

## Source/Editor/HitboxEditor.h

[HitboxEditor.h](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Editor/HitboxEditor.h>) · 43줄

| 위치 | 선언·함수 |
|---|---|
| [L7](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Editor/HitboxEditor.h:7>) | `class EnemyLab;` |
| [L8](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Editor/HitboxEditor.h:8>) | `class Renderer;` |
| [L9](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Editor/HitboxEditor.h:9>) | `class SandboxState;` |
| [L13](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Editor/HitboxEditor.h:13>) | `class HitboxEditor` |
| [L16](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Editor/HitboxEditor.h:16>) | `void Draw(EnemyLab& _lab);` |
| [L17](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Editor/HitboxEditor.h:17>) | `void DrawOverlay(EnemyLab& _lab, Renderer& _renderer);` |
| [L20](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Editor/HitboxEditor.h:20>) | `void Load_(const FilePath& _assetRoot);` |
| [L21](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Editor/HitboxEditor.h:21>) | `void ApplyLive_(SandboxState& _state, const HitboxEntry& _entry);` |
| [L22](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Editor/HitboxEditor.h:22>) | `void DrawEntries_(EnemyLab& _lab, SandboxState* _pState);` |
| [L23](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Editor/HitboxEditor.h:23>) | `void DrawExtraBoxes_(SandboxState& _state, HitboxDocument& _doc);` |
| [L24](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Editor/HitboxEditor.h:24>) | `void DrawExtraOverlay_(EnemyLab& _lab, Renderer& _renderer);` |

## Source/Editor/ImGuiSdlRendererViewports.cpp

[ImGuiSdlRendererViewports.cpp](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Editor/ImGuiSdlRendererViewports.cpp>) · 126줄

| 위치 | 선언·함수 |
|---|---|
| [L16](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Editor/ImGuiSdlRendererViewports.cpp:16>) | `static void MoveToWindowOrigin( ImDrawData* _pDraw)` |
| [L39](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Editor/ImGuiSdlRendererViewports.cpp:39>) | `static void CreateTarget( ImGuiViewport* _pViewport)` |
| [L45](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Editor/ImGuiSdlRendererViewports.cpp:45>) | `static void DestroyTarget( ImGuiViewport* _pViewport)` |
| [L57](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Editor/ImGuiSdlRendererViewports.cpp:57>) | `static void RenderTarget( ImGuiViewport* _pViewport, void*)` |
| [L103](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Editor/ImGuiSdlRendererViewports.cpp:103>) | `void ImGuiSdlRendererViewports_Init( SDL_Renderer* _pRenderer)` |
| [L114](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Editor/ImGuiSdlRendererViewports.cpp:114>) | `void ImGuiSdlRendererViewports_RenderMain( ImDrawData* _pDraw)` |
| [L121](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Editor/ImGuiSdlRendererViewports.cpp:121>) | `void ImGuiSdlRendererViewports_Shutdown()` |

## Source/Editor/ImGuiSdlRendererViewports.h

[ImGuiSdlRendererViewports.h](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Editor/ImGuiSdlRendererViewports.h>) · 12줄

| 위치 | 선언·함수 |
|---|---|
| [L3](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Editor/ImGuiSdlRendererViewports.h:3>) | `struct SDL_Renderer;` |
| [L5](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Editor/ImGuiSdlRendererViewports.h:5>) | `struct ImDrawData;` |
| [L10](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Editor/ImGuiSdlRendererViewports.h:10>) | `void ImGuiSdlRendererViewports_Init(SDL_Renderer* _pRenderer);` |
| [L12](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Editor/ImGuiSdlRendererViewports.h:12>) | `void ImGuiSdlRendererViewports_Shutdown();` |

## Source/Editor/JsonScan.cpp

[JsonScan.cpp](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Editor/JsonScan.cpp>) · 163줄

| 위치 | 선언·함수 |
|---|---|
| [L15](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Editor/JsonScan.cpp:15>) | `int JsonScanner::Parse()` |
| [L26](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Editor/JsonScan.cpp:26>) | `void JsonScanner::SkipWs_()` |
| [L34](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Editor/JsonScan.cpp:34>) | `bool JsonScanner::ReadString_( String& _out)` |
| [L52](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Editor/JsonScan.cpp:52>) | `int JsonScanner::Value_( const int _depth)` |
| [L118](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Editor/JsonScan.cpp:118>) | `else if (c == '"')` |
| [L149](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Editor/JsonScan.cpp:149>) | `String FormatJsonNumber( const double _value)` |

## Source/Editor/JsonScan.h

[JsonScan.h](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Editor/JsonScan.h>) · 33줄

| 위치 | 선언·함수 |
|---|---|
| [L5](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Editor/JsonScan.h:5>) | `struct JsonNode` |
| [L15](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Editor/JsonScan.h:15>) | `class JsonScanner` |
| [L18](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Editor/JsonScan.h:18>) | `explicit JsonScanner(StringView _text);` |
| [L19](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Editor/JsonScan.h:19>) | `int Parse();` |
| [L24](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Editor/JsonScan.h:24>) | `void SkipWs_();` |
| [L25](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Editor/JsonScan.h:25>) | `bool ReadString_(String& _out);` |
| [L26](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Editor/JsonScan.h:26>) | `int  Value_(int _depth);` |
| [L33](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Editor/JsonScan.h:33>) | `[[nodiscard]] String FormatJsonNumber(double _value);` |

## Source/Editor/MotionDocument.cpp

[MotionDocument.cpp](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Editor/MotionDocument.cpp>) · 505줄

| 위치 | 선언·함수 |
|---|---|
| [L13](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Editor/MotionDocument.cpp:13>) | `double MotionRound_( const double _value)` |
| [L19](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Editor/MotionDocument.cpp:19>) | `int MotionKey_( const JsonNode& _node, const StringView _key)` |
| [L38](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Editor/MotionDocument.cpp:38>) | `int MotionPath_( const Vector<JsonNode>& _nodes, const int _root, const Vector<String>& _path)` |
| [L68](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Editor/MotionDocument.cpp:68>) | `void MotionInsertMember_( String& _text, const Vector<JsonNode>& _nodes, const int _object, const String& _member)` |
| [L84](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Editor/MotionDocument.cpp:84>) | `bool MotionPatchNumber_( String& _text, const Vector<String>& _path, const double _value)` |
| [L110](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Editor/MotionDocument.cpp:110>) | `bool MotionPatchBool_( String& _text, const Vector<String>& _path, const bool _value)` |
| [L131](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Editor/MotionDocument.cpp:131>) | `void MotionCollectParams_( const Vector<JsonNode>& _nodes, const int _object, const Vector<String>& _path, const String& _label, const int _depth, Vector<MotionParam>& _out)` |
| [L153](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Editor/MotionDocument.cpp:153>) | `else if (kid.type == 'l' && (kid.str == "true" \|\| kid.str == "false"))` |
| [L159](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Editor/MotionDocument.cpp:159>) | `else if (kid.type == 'a' && kid.kids.size() == 2 && _nodes[static_cast<size_t>(kid.kids[0])].type == 'n' && _nodes[static_cast<size_t>(kid.kids[1])].type == 'n')` |
| [L166](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Editor/MotionDocument.cpp:166>) | `else if (kid.type == 'o' && _depth < 3)` |
| [L173](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Editor/MotionDocument.cpp:173>) | `bool MotionPatchVec2_( String& _text, Vector<String> _path, const VECTOR2 _value)` |
| [L187](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Editor/MotionDocument.cpp:187>) | `String MotionRenderEffects_( const Vector<WeaponEffect>& _effects)` |
| [L205](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Editor/MotionDocument.cpp:205>) | `bool MotionSameEffects_( const Vector<WeaponEffect>& _a, const Vector<WeaponEffect>& _b)` |
| [L225](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Editor/MotionDocument.cpp:225>) | `bool MotionPatchString_( String& _text, const Vector<String>& _object, const StringView _key, const String& _value)` |
| [L251](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Editor/MotionDocument.cpp:251>) | `else if (!_value.empty())` |
| [L258](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Editor/MotionDocument.cpp:258>) | `bool MotionPatchEffects_( String& _text, const Vector<String>& _weapon, const Vector<WeaponEffect>& _effects)` |
| [L311](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Editor/MotionDocument.cpp:311>) | `Result<void> MotionDocument::Load( const FilePath& _path)` |
| [L322](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Editor/MotionDocument.cpp:322>) | `Result<void> MotionDocument::LoadFromText( const StringView _json)` |
| [L330](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Editor/MotionDocument.cpp:330>) | `Result<void> MotionDocument::Save( const FilePath& _path)` |
| [L342](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Editor/MotionDocument.cpp:342>) | `bool MotionDocument::ApplyWeapon( const StringView _def, const StringView _weapon, const MotionWeaponValues& _values)` |
| [L367](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Editor/MotionDocument.cpp:367>) | `bool MotionDocument::ApplyClipSec( const StringView _def, const StringView _clip, const double _sec)` |
| [L375](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Editor/MotionDocument.cpp:375>) | `bool MotionDocument::ApplyPartDepth( const StringView _def, const StringView _part, const int _depth, const int _sort)` |
| [L387](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Editor/MotionDocument.cpp:387>) | `bool MotionSetInt_( String& _text, const Vector<String>& _object, const StringView _key, const int _value, const int _default)` |
| [L413](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Editor/MotionDocument.cpp:413>) | `bool MotionDocument::SetPartDepth( const StringView _def, const StringView _part, const int _depth, const int _sort, const int _defaultDepth, const int _defaultSort)` |
| [L458](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Editor/MotionDocument.cpp:458>) | `Vector<MotionParam> MotionDocument::ListParams( const StringView _def) const` |
| [L477](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Editor/MotionDocument.cpp:477>) | `bool MotionDocument::SetParam( const StringView _def, const MotionParam& _param)` |
| [L497](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Editor/MotionDocument.cpp:497>) | `const String& MotionDocument::GetText() const` |
| [L502](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Editor/MotionDocument.cpp:502>) | `bool MotionDocument::IsDirty() const` |

## Source/Editor/MotionDocument.h

[MotionDocument.h](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Editor/MotionDocument.h>) · 43줄

| 위치 | 선언·함수 |
|---|---|
| [L6](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Editor/MotionDocument.h:6>) | `struct MotionWeaponValues` |
| [L15](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Editor/MotionDocument.h:15>) | `struct MotionParam` |
| [L25](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Editor/MotionDocument.h:25>) | `class MotionDocument` |
| [L28](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Editor/MotionDocument.h:28>) | `[[nodiscard]] Result<void>        Load(const FilePath& _path);` |
| [L29](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Editor/MotionDocument.h:29>) | `[[nodiscard]] Result<void>        LoadFromText(StringView _json);` |
| [L30](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Editor/MotionDocument.h:30>) | `[[nodiscard]] Result<void>        Save(const FilePath& _path);` |
| [L31](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Editor/MotionDocument.h:31>) | `bool                              ApplyWeapon(StringView _def, StringView _weapon, const MotionWeaponValues& _values);` |
| [L32](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Editor/MotionDocument.h:32>) | `bool                              ApplyClipSec(StringView _def, StringView _clip, double _sec);` |
| [L33](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Editor/MotionDocument.h:33>) | `bool                              ApplyPartDepth(StringView _def, StringView _part, int _depth, int _sort);` |
| [L34](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Editor/MotionDocument.h:34>) | `bool                              SetPartDepth(StringView _def, StringView _part, int _depth, int _sort, int _defaultDepth, int _defaultSort);` |
| [L35](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Editor/MotionDocument.h:35>) | `[[nodiscard]] Vector<MotionParam> ListParams(StringView _def) const;` |
| [L36](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Editor/MotionDocument.h:36>) | `bool                              SetParam(StringView _def, const MotionParam& _param);` |
| [L37](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Editor/MotionDocument.h:37>) | `[[nodiscard]] const String&       GetText() const;` |
| [L38](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Editor/MotionDocument.h:38>) | `[[nodiscard]] bool                IsDirty() const;` |

## Source/Editor/MotionEditor.cpp

[MotionEditor.cpp](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Editor/MotionEditor.cpp>) · 718줄

| 위치 | 선언·함수 |
|---|---|
| [L23](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Editor/MotionEditor.cpp:23>) | `MotionWeaponValues MotionRead_( const WeaponDef& _w)` |
| [L35](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Editor/MotionEditor.cpp:35>) | `bool MotionIsAttackClip_( const String& _name)` |
| [L49](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Editor/MotionEditor.cpp:49>) | `ImVec4 MotionColor_( const RGBA _c)` |
| [L59](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Editor/MotionEditor.cpp:59>) | `EnemyPartDepth* MotionPart_( BusConfig& _c, const StringView _name)` |
| [L99](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Editor/MotionEditor.cpp:99>) | `void MotionEditor::SetAnimLibrary( const AnimationLibrary* _pAnims)` |
| [L105](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Editor/MotionEditor.cpp:105>) | `void MotionEditor::Load_( const FilePath& _assetRoot)` |
| [L124](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Editor/MotionEditor.cpp:124>) | `void MotionEditor::MarkChanged_( const String& _key)` |
| [L137](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Editor/MotionEditor.cpp:137>) | `void MotionEditor::Save_( EnemyLab& _lab)` |
| [L168](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Editor/MotionEditor.cpp:168>) | `else if (kind == "c")` |
| [L211](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Editor/MotionEditor.cpp:211>) | `const char* MotionParamKo_( const String& _key)` |
| [L253](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Editor/MotionEditor.cpp:253>) | `String MotionParamPrefix_( const String& _label)` |
| [L265](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Editor/MotionEditor.cpp:265>) | `void MotionEditor::DrawParams_( EnemyLab& _lab, const String& _def)` |
| [L311](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Editor/MotionEditor.cpp:311>) | `else if (p.type == 'v')` |
| [L317](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Editor/MotionEditor.cpp:317>) | `else if (p.bInt)` |
| [L356](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Editor/MotionEditor.cpp:356>) | `void MotionEditor::DrawParts_( EnemyDef& _def)` |
| [L384](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Editor/MotionEditor.cpp:384>) | `void MotionEditor::DrawWeapon_( EnemyLab& _lab, EnemyDef& _def, const int _weapon)` |
| [L534](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Editor/MotionEditor.cpp:534>) | `void MotionEditor::Draw( EnemyLab& _lab)` |
| [L557](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Editor/MotionEditor.cpp:557>) | `else if (_lab.GetDefIndex() >= 0 && _lab.GetDefIndex() < static_cast<int>(pState->GetEnemyLib().GetDefCount()))` |
| [L615](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Editor/MotionEditor.cpp:615>) | `void MotionEditor::DrawOverlay( EnemyLab& _lab, Renderer& _renderer)` |
| [L669](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Editor/MotionEditor.cpp:669>) | `else if (kind >= 2)` |
| [L708](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Editor/MotionEditor.cpp:708>) | `else if (m_pick == ePick::MuzzleFx)` |
| [L712](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Editor/MotionEditor.cpp:712>) | `else if (m_pickEffect < static_cast<int>(w.effects.size()))` |

## Source/Editor/MotionEditor.h

[MotionEditor.h](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Editor/MotionEditor.h>) · 55줄

| 위치 | 선언·함수 |
|---|---|
| [L6](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Editor/MotionEditor.h:6>) | `class AnimationLibrary;` |
| [L7](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Editor/MotionEditor.h:7>) | `class EnemyLab;` |
| [L8](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Editor/MotionEditor.h:8>) | `class Renderer;` |
| [L12](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Editor/MotionEditor.h:12>) | `class MotionEditor` |
| [L15](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Editor/MotionEditor.h:15>) | `void SetAnimLibrary(const AnimationLibrary* _pAnims);` |
| [L16](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Editor/MotionEditor.h:16>) | `void Draw(EnemyLab& _lab);` |
| [L17](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Editor/MotionEditor.h:17>) | `void DrawOverlay(EnemyLab& _lab, Renderer& _renderer);` |
| [L20](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Editor/MotionEditor.h:20>) | `enum class ePick` |
| [L28](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Editor/MotionEditor.h:28>) | `void Load_(const FilePath& _assetRoot);` |
| [L29](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Editor/MotionEditor.h:29>) | `void Save_(EnemyLab& _lab);` |
| [L30](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Editor/MotionEditor.h:30>) | `void MarkChanged_(const String& _key);` |
| [L31](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Editor/MotionEditor.h:31>) | `void DrawWeapon_(EnemyLab& _lab, EnemyDef& _def, int _weapon);` |
| [L32](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Editor/MotionEditor.h:32>) | `void DrawParts_(EnemyDef& _def);` |
| [L33](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Editor/MotionEditor.h:33>) | `void DrawParams_(EnemyLab& _lab, const String& _def);` |

## Source/Editor/StageDocument.cpp

[StageDocument.cpp](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Editor/StageDocument.cpp>) · 874줄

| 위치 | 선언·함수 |
|---|---|
| [L21](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Editor/StageDocument.cpp:21>) | `void SetRectOutput(Vector<StageEditRect>& _rects)` |
| [L23](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Editor/StageDocument.cpp:23>) | `[[nodiscard]] int Find(const int _obj, const StringView _key) const` |
| [L40](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Editor/StageDocument.cpp:40>) | `[[nodiscard]] String Str(const int _node) const` |
| [L49](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Editor/StageDocument.cpp:49>) | `[[nodiscard]] bool Num(const int _node, double& _out) const` |
| [L59](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Editor/StageDocument.cpp:59>) | `void AddRect(const int _obj, const String& _type)` |
| [L87](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Editor/StageDocument.cpp:87>) | `void AddEntity(const int _obj, const int _section, const String& _where, const bool _bPlayer)` |
| [L148](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Editor/StageDocument.cpp:148>) | `void Action(const int _obj, const int _section, const String& _where)` |
| [L155](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Editor/StageDocument.cpp:155>) | `else if (type == "spawnWave")` |
| [L166](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Editor/StageDocument.cpp:166>) | `else if ((type == "lockCamera" \|\| type == "setBounds") && m_pRects != nullptr)` |
| [L170](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Editor/StageDocument.cpp:170>) | `else if (type == "gate")` |
| [L192](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Editor/StageDocument.cpp:192>) | `[[nodiscard]] String WsBefore_( const String& _text, const int _pos)` |
| [L204](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Editor/StageDocument.cpp:204>) | `[[nodiscard]] bool IsWs_( const char _c)` |
| [L219](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Editor/StageDocument.cpp:219>) | `void ApplyReplacements_( String& _text, Vector<Replacement>& _list)` |
| [L233](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Editor/StageDocument.cpp:233>) | `bool StageEntity::IsPlaceable() const` |
| [L238](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Editor/StageDocument.cpp:238>) | `const String& StageDocument::GetText() const` |
| [L243](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Editor/StageDocument.cpp:243>) | `Span<const StageEntity> StageDocument::GetEntities() const` |
| [L248](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Editor/StageDocument.cpp:248>) | `Span<const StageEditSection> StageDocument::GetSections() const` |
| [L253](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Editor/StageDocument.cpp:253>) | `int StageDocument::GetWorldWidth() const` |
| [L258](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Editor/StageDocument.cpp:258>) | `int StageDocument::GetWorldHeight() const` |
| [L263](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Editor/StageDocument.cpp:263>) | `const String& StageDocument::GetBackground() const` |
| [L268](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Editor/StageDocument.cpp:268>) | `const String& StageDocument::GetGroundInitial() const` |
| [L273](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Editor/StageDocument.cpp:273>) | `Span<const String> StageDocument::GetGroundMaps() const` |
| [L278](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Editor/StageDocument.cpp:278>) | `bool StageDocument::IsDirty() const` |
| [L283](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Editor/StageDocument.cpp:283>) | `bool StageDocument::CanUndo() const` |
| [L288](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Editor/StageDocument.cpp:288>) | `bool StageDocument::Reparse_()` |
| [L384](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Editor/StageDocument.cpp:384>) | `Result<void> StageDocument::LoadFromText( const StringView _json)` |
| [L403](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Editor/StageDocument.cpp:403>) | `Result<void> StageDocument::Load( const FilePath& _path)` |
| [L414](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Editor/StageDocument.cpp:414>) | `Result<void> StageDocument::Save( const FilePath& _path)` |
| [L426](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Editor/StageDocument.cpp:426>) | `int StageDocument::FindEntityAt_( const int _begin) const` |
| [L439](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Editor/StageDocument.cpp:439>) | `bool StageDocument::IsIdUsed_( const StringView _id) const` |
| [L446](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Editor/StageDocument.cpp:446>) | `bool StageDocument::SetPosition( const int _entity, const double _x, const double _y, const bool _bUndo)` |
| [L489](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Editor/StageDocument.cpp:489>) | `int StageDocument::Add( const StringView _def, const StringView _kind, const int _section, const double _x, const double _y)` |
| [L534](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Editor/StageDocument.cpp:534>) | `int StageDocument::Duplicate( const int _entity, const double _dx, const double _dy)` |
| [L592](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Editor/StageDocument.cpp:592>) | `bool StageDocument::Delete( const int _entity)` |
| [L647](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Editor/StageDocument.cpp:647>) | `bool StageDocument::Patch_( Vector<Replacement>& _list, const bool _bUndo)` |
| [L674](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Editor/StageDocument.cpp:674>) | `bool StageDocument::SetZ( const int _entity, const int _z, const int _defaultZ, const bool _bUndo)` |
| [L716](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Editor/StageDocument.cpp:716>) | `else if (_z != e.z)` |
| [L721](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Editor/StageDocument.cpp:721>) | `else if (_z != _defaultZ)` |
| [L733](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Editor/StageDocument.cpp:733>) | `bool StageDocument::UnlockGround( const int _entity, const double _y, const bool _bUndo)` |
| [L761](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Editor/StageDocument.cpp:761>) | `bool StageDocument::MoveSection( const int _section, const double _dx, const bool _bTrigger, const bool _bRects, const bool _bSpawns, const bool _bUndo)` |
| [L806](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Editor/StageDocument.cpp:806>) | `bool StageDocument::SetTriggerX( const int _section, const double _x, const bool _bUndo)` |
| [L824](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Editor/StageDocument.cpp:824>) | `bool StageDocument::SetRect( const int _section, const int _rect, const ARRAY<double, 4>& _v, const bool _bUndo)` |
| [L846](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Editor/StageDocument.cpp:846>) | `bool StageDocument::SetGlide( const int _section, const int _rect, const double _glide, const bool _bUndo)` |
| [L865](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Editor/StageDocument.cpp:865>) | `void StageDocument::Undo()` |

## Source/Editor/StageDocument.h

[StageDocument.h](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Editor/StageDocument.h>) · 119줄

| 위치 | 선언·함수 |
|---|---|
| [L5](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Editor/StageDocument.h:5>) | `struct Replacement;` |
| [L7](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Editor/StageDocument.h:7>) | `struct StageEntity` |
| [L36](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Editor/StageDocument.h:36>) | `[[nodiscard]] bool IsPlaceable() const;` |
| [L39](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Editor/StageDocument.h:39>) | `struct StageEditRect` |
| [L50](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Editor/StageDocument.h:50>) | `struct StageSpriteFrame` |
| [L61](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Editor/StageDocument.h:61>) | `struct StageEditSection` |
| [L73](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Editor/StageDocument.h:73>) | `class StageDocument` |
| [L76](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Editor/StageDocument.h:76>) | `[[nodiscard]] Result<void>                 Load(const FilePath& _path);` |
| [L77](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Editor/StageDocument.h:77>) | `[[nodiscard]] Result<void>                 LoadFromText(StringView _json);` |
| [L78](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Editor/StageDocument.h:78>) | `[[nodiscard]] Result<void>                 Save(const FilePath& _path);` |
| [L79](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Editor/StageDocument.h:79>) | `[[nodiscard]] const String&                GetText() const;` |
| [L80](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Editor/StageDocument.h:80>) | `[[nodiscard]] Span<const StageEntity>      GetEntities() const;` |
| [L81](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Editor/StageDocument.h:81>) | `[[nodiscard]] Span<const StageEditSection> GetSections() const;` |
| [L82](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Editor/StageDocument.h:82>) | `[[nodiscard]] int                          GetWorldWidth() const;` |
| [L83](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Editor/StageDocument.h:83>) | `[[nodiscard]] int                          GetWorldHeight() const;` |
| [L84](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Editor/StageDocument.h:84>) | `[[nodiscard]] const String&                GetBackground() const;` |
| [L85](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Editor/StageDocument.h:85>) | `[[nodiscard]] const String&                GetGroundInitial() const;` |
| [L86](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Editor/StageDocument.h:86>) | `[[nodiscard]] Span<const String>           GetGroundMaps() const;` |
| [L87](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Editor/StageDocument.h:87>) | `[[nodiscard]] bool                         IsDirty() const;` |
| [L88](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Editor/StageDocument.h:88>) | `[[nodiscard]] bool                         CanUndo() const;` |
| [L89](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Editor/StageDocument.h:89>) | `bool                                       SetPosition(int _entity, double _x, double _y, bool _bUndo = true);` |
| [L90](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Editor/StageDocument.h:90>) | `bool                                       SetZ(int _entity, int _z, int _defaultZ, bool _bUndo = true);` |
| [L92](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Editor/StageDocument.h:92>) | `bool UnlockGround(int _entity, double _y, bool _bUndo = true);` |
| [L93](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Editor/StageDocument.h:93>) | `int  Add(StringView _def, StringView _kind, int _section, double _x, double _y);` |
| [L94](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Editor/StageDocument.h:94>) | `int  Duplicate(int _entity, double _dx, double _dy);` |
| [L95](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Editor/StageDocument.h:95>) | `bool Delete(int _entity);` |
| [L96](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Editor/StageDocument.h:96>) | `void Undo();` |
| [L97](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Editor/StageDocument.h:97>) | `bool MoveSection(int _section, double _dx, bool _bTrigger, bool _bRects, bool _bSpawns, bool _bUndo = true);` |
| [L98](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Editor/StageDocument.h:98>) | `bool SetTriggerX(int _section, double _x, bool _bUndo = true);` |
| [L99](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Editor/StageDocument.h:99>) | `bool SetRect(int _section, int _rect, const ARRAY<double, 4>& _v, bool _bUndo = true);` |
| [L100](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Editor/StageDocument.h:100>) | `bool SetGlide(int _section, int _rect, double _glide, bool _bUndo = true);` |
| [L103](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Editor/StageDocument.h:103>) | `[[nodiscard]] bool Reparse_();` |
| [L104](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Editor/StageDocument.h:104>) | `bool               Patch_(Vector<Replacement>& _list, bool _bUndo);` |
| [L105](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Editor/StageDocument.h:105>) | `void               PushUndo_();` |
| [L106](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Editor/StageDocument.h:106>) | `[[nodiscard]] int  FindEntityAt_(int _begin) const;` |
| [L107](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Editor/StageDocument.h:107>) | `[[nodiscard]] bool IsIdUsed_(StringView _id) const;` |

## Source/Editor/StageEditor.cpp

[StageEditor.cpp](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Editor/StageEditor.cpp>) · 1421줄

| 위치 | 선언·함수 |
|---|---|
| [L27](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Editor/StageEditor.cpp:27>) | `[[nodiscard]] ImU32 KindColor_( const String& _kind)` |
| [L61](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Editor/StageEditor.cpp:61>) | `[[nodiscard]] const Vector<String>& Kinds_()` |
| [L67](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Editor/StageEditor.cpp:67>) | `bool ComboStr_( const char* _label, int& _index, const Vector<String>& _items)` |
| [L89](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Editor/StageEditor.cpp:89>) | `[[nodiscard]] double Snap_( const double _value, const double _step)` |
| [L96](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Editor/StageEditor.cpp:96>) | `[[nodiscard]] int ReadInt_( const JsonReader& _r, const char* _key, const int _default)` |
| [L105](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Editor/StageEditor.cpp:105>) | `[[nodiscard]] bool ReadFrame_( const JsonReader& _r, StageSpriteFrame& _out)` |
| [L136](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Editor/StageEditor.cpp:136>) | `[[nodiscard]] String VisualName_( const JsonReader& _r)` |
| [L150](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Editor/StageEditor.cpp:150>) | `[[nodiscard]] String ClipName_( const JsonReader& _clips, const char* _key)` |
| [L166](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Editor/StageEditor.cpp:166>) | `[[nodiscard]] String IdleClip_( const JsonReader& _defs, const JsonReader& _def, const int _depth)` |
| [L195](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Editor/StageEditor.cpp:195>) | `[[nodiscard]] String FirstClip_( const JsonReader& _def)` |
| [L214](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Editor/StageEditor.cpp:214>) | `void StageEditor::Init( SDL_Renderer* _pRenderer)` |
| [L241](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Editor/StageEditor.cpp:241>) | `void StageEditor::Shutdown()` |
| [L252](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Editor/StageEditor.cpp:252>) | `void StageEditor::SelectStage( const StringView _name)` |
| [L261](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Editor/StageEditor.cpp:261>) | `void StageEditor::ClearTextures_()` |
| [L277](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Editor/StageEditor.cpp:277>) | `void StageEditor::LoadDefs_()` |
| [L394](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Editor/StageEditor.cpp:394>) | `void StageEditor::LoadFrames_()` |
| [L436](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Editor/StageEditor.cpp:436>) | `String StageEditor::SpriteName_( const StringView _kind, const StringView _def) const` |
| [L445](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Editor/StageEditor.cpp:445>) | `else if (_kind == "prop" \|\| _kind == "item")` |
| [L457](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Editor/StageEditor.cpp:457>) | `const StageEditor::Page* StageEditor::GetPage_( const String& _file)` |
| [L483](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Editor/StageEditor.cpp:483>) | `bool StageEditor::DrawSprite_( ImDrawList* _pDraw, const StageEntity& _e, const float _px, const float _py, const bool _bSelected)` |
| [L547](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Editor/StageEditor.cpp:547>) | `bool StageEditor::PickSection_( const float _mx, const float _my, const float _originX, const float _originY, int& _section, int& _rect, int& _mask) const` |
| [L603](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Editor/StageEditor.cpp:603>) | `void StageEditor::LoadStrips_( const FilePath& _path, const bool _bGroundMask, Vector<Strip>& _out, int& _w, int& _h)` |
| [L667](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Editor/StageEditor.cpp:667>) | `void StageEditor::LoadGround_( const String& _name)` |
| [L684](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Editor/StageEditor.cpp:684>) | `void StageEditor::LoadStage_( const String& _name)` |
| [L723](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Editor/StageEditor.cpp:723>) | `void StageEditor::Save_()` |
| [L729](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Editor/StageEditor.cpp:729>) | `int StageEditor::GroundY_( const int _x, const int _fromY) const` |
| [L747](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Editor/StageEditor.cpp:747>) | `int StageEditor::DefaultZ_( const StageEntity& _e) const` |
| [L761](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Editor/StageEditor.cpp:761>) | `int StageEditor::EffectiveZ_( const StageEntity& _e) const` |
| [L767](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Editor/StageEditor.cpp:767>) | `double StageEditor::MarkerY_( const StageEntity& _e) const` |
| [L778](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Editor/StageEditor.cpp:778>) | `const char* StageEditor::DefaultKind_( const StringView _def) const` |
| [L798](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Editor/StageEditor.cpp:798>) | `void StageEditor::Select_( const int _entity, const bool _bCenter)` |
| [L814](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Editor/StageEditor.cpp:814>) | `void StageEditor::Draw()` |
| [L865](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Editor/StageEditor.cpp:865>) | `void StageEditor::DrawPanel_()` |
| [L951](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Editor/StageEditor.cpp:951>) | `else if (!e.bHasX)` |
| [L1075](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Editor/StageEditor.cpp:1075>) | `void StageEditor::DrawView_()` |
| [L1325](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Editor/StageEditor.cpp:1325>) | `else if (hit >= 0)` |
| [L1381](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Editor/StageEditor.cpp:1381>) | `else if (m_rectDrag < static_cast<int>(sections[static_cast<size_t>(m_secDrag)].rects.size()))` |

## Source/Editor/StageEditor.h

[StageEditor.h](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Editor/StageEditor.h>) · 107줄

| 위치 | 선언·함수 |
|---|---|
| [L8](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Editor/StageEditor.h:8>) | `struct SDL_Renderer;` |
| [L9](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Editor/StageEditor.h:9>) | `struct SDL_Texture;` |
| [L10](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Editor/StageEditor.h:10>) | `struct ImDrawList;` |
| [L14](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Editor/StageEditor.h:14>) | `class StageEditor` |
| [L17](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Editor/StageEditor.h:17>) | `void Init(SDL_Renderer* _pRenderer);` |
| [L18](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Editor/StageEditor.h:18>) | `void Shutdown();` |
| [L19](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Editor/StageEditor.h:19>) | `void SelectStage(StringView _name);` |
| [L20](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Editor/StageEditor.h:20>) | `void Draw();` |
| [L23](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Editor/StageEditor.h:23>) | `struct Strip` |
| [L29](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Editor/StageEditor.h:29>) | `struct Page` |
| [L35](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Editor/StageEditor.h:35>) | `struct DefInfo` |
| [L42](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Editor/StageEditor.h:42>) | `void                      LoadStage_(const String& _name);` |
| [L43](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Editor/StageEditor.h:43>) | `void                      LoadGround_(const String& _name);` |
| [L44](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Editor/StageEditor.h:44>) | `void                      LoadDefs_();` |
| [L45](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Editor/StageEditor.h:45>) | `void                      LoadFrames_();` |
| [L46](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Editor/StageEditor.h:46>) | `[[nodiscard]] String      SpriteName_(StringView _kind, StringView _def) const;` |
| [L47](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Editor/StageEditor.h:47>) | `[[nodiscard]] const Page* GetPage_(const String& _file);` |
| [L48](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Editor/StageEditor.h:48>) | `[[nodiscard]] int         DefaultZ_(const StageEntity& _e) const;` |
| [L49](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Editor/StageEditor.h:49>) | `[[nodiscard]] int         EffectiveZ_(const StageEntity& _e) const;` |
| [L50](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Editor/StageEditor.h:50>) | `[[nodiscard]] bool        PickSection_(float _mx, float _my, float _originX, float _originY, int& _section, int& _rect, int& _mask) const;` |
| [L51](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Editor/StageEditor.h:51>) | `[[nodiscard]] bool        DrawSprite_(ImDrawList* _pDraw, const StageEntity& _e, float _px, float _py, bool _bSelected);` |
| [L52](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Editor/StageEditor.h:52>) | `void                      ClearTextures_();` |
| [L53](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Editor/StageEditor.h:53>) | `void                      Save_();` |
| [L54](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Editor/StageEditor.h:54>) | `void                      DrawPanel_();` |
| [L55](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Editor/StageEditor.h:55>) | `void                      DrawView_();` |
| [L56](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Editor/StageEditor.h:56>) | `void                      Select_(int _entity, bool _bCenter);` |
| [L57](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Editor/StageEditor.h:57>) | `[[nodiscard]] int         GroundY_(int _x, int _fromY) const;` |
| [L58](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Editor/StageEditor.h:58>) | `[[nodiscard]] double      MarkerY_(const StageEntity& _e) const;` |
| [L59](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Editor/StageEditor.h:59>) | `void                      LoadStrips_(const FilePath& _path, bool _bGroundMask, Vector<Strip>& _out, int& _w, int& _h);` |
| [L60](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Editor/StageEditor.h:60>) | `[[nodiscard]] const char* DefaultKind_(StringView _def) const;` |

## Source/Game/Combat/BulletVisual.cpp

[BulletVisual.cpp](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Combat/BulletVisual.cpp>) · 203줄

| 위치 | 선언·함수 |
|---|---|
| [L23](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Combat/BulletVisual.cpp:23>) | `[[nodiscard]] const AnimFrame* PickFlameFrame_( const AnimationLibrary* _pAnims, const Bullet& _bullet)` |
| [L58](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Combat/BulletVisual.cpp:58>) | `[[nodiscard]] const AnimFrame* PickClipFrame_( const AnimationLibrary* _pAnims, const String& _name, const int _ageTicks, const bool _bLoop)` |
| [L75](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Combat/BulletVisual.cpp:75>) | `bool SubmitPlayerBullet( SpriteBatch& _sprites, const AnimationLibrary* _pAnims, const SpriteLibrary& _spriteLib, const Bullet& _bullet, const VECTOR2 _pos, const int _depth)` |
| [L132](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Combat/BulletVisual.cpp:132>) | `else if (degrees < -180.f)` |
| [L166](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Combat/BulletVisual.cpp:166>) | `void PlayShotSfx( AudioSystem& _audio, const eWeapon _weapon, const int _shotIndex)` |
| [L186](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Combat/BulletVisual.cpp:186>) | `void PlayWeaponPickupSfx( AudioSystem& _audio, const eWeapon _weapon)` |

## Source/Game/Combat/BulletVisual.h

[BulletVisual.h](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Combat/BulletVisual.h>) · 14줄

| 위치 | 선언·함수 |
|---|---|
| [L5](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Combat/BulletVisual.h:5>) | `class AnimationLibrary;` |
| [L6](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Combat/BulletVisual.h:6>) | `class AudioSystem;` |
| [L7](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Combat/BulletVisual.h:7>) | `class SpriteBatch;` |
| [L8](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Combat/BulletVisual.h:8>) | `class SpriteLibrary;` |
| [L10](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Combat/BulletVisual.h:10>) | `bool SubmitPlayerBullet(SpriteBatch& _sprites, const AnimationLibrary* _pAnims, const SpriteLibrary& _spriteLib, const Bullet& _bullet, VECTOR2 _pos, int _depth);` |
| [L12](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Combat/BulletVisual.h:12>) | `void PlayShotSfx(AudioSystem& _audio, eWeapon _weapon, int _shotIndex);` |
| [L14](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Combat/BulletVisual.h:14>) | `void PlayWeaponPickupSfx(AudioSystem& _audio, eWeapon _weapon);` |

## Source/Game/Combat/CombatSystem.cpp

[CombatSystem.cpp](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Combat/CombatSystem.cpp>) · 78줄

| 위치 | 선언·함수 |
|---|---|
| [L5](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Combat/CombatSystem.cpp:5>) | `void CombatSystem::BeginTick()` |
| [L12](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Combat/CombatSystem.cpp:12>) | `const Vector<HitEvent>& CombatSystem::Resolve()` |
| [L18](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Combat/CombatSystem.cpp:18>) | `bool CombatSystem::HasTargetIn( const eTeam _attackerTeam, const eHitKind _kind, const Aabb& _box, const uint32_t _ignoreOwner) const` |
| [L42](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Combat/CombatSystem.cpp:42>) | `void CombatSystem::AddHurtbox( const Hurtbox& _hurtbox)` |
| [L48](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Combat/CombatSystem.cpp:48>) | `void CombatSystem::AddHitbox( const Hitbox& _hitbox)` |
| [L54](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Combat/CombatSystem.cpp:54>) | `uint32_t CombatSystem::NewHitId()` |
| [L59](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Combat/CombatSystem.cpp:59>) | `void CombatSystem::ReleaseHitId( const uint32_t _hitId)` |
| [L65](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Combat/CombatSystem.cpp:65>) | `Span<const Hurtbox> CombatSystem::GetHurtboxes() const` |
| [L70](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Combat/CombatSystem.cpp:70>) | `Span<const Hitbox> CombatSystem::GetHitboxes() const` |
| [L75](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Combat/CombatSystem.cpp:75>) | `HitResolver& CombatSystem::GetResolver()` |

## Source/Game/Combat/CombatSystem.h

[CombatSystem.h](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Combat/CombatSystem.h>) · 25줄

| 위치 | 선언·함수 |
|---|---|
| [L5](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Combat/CombatSystem.h:5>) | `class CombatSystem` |
| [L8](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Combat/CombatSystem.h:8>) | `void                              BeginTick();` |
| [L9](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Combat/CombatSystem.h:9>) | `void                              AddHurtbox(const Hurtbox& _hurtbox);` |
| [L10](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Combat/CombatSystem.h:10>) | `void                              AddHitbox(const Hitbox& _hitbox);` |
| [L11](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Combat/CombatSystem.h:11>) | `const Vector<HitEvent>&           Resolve();` |
| [L12](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Combat/CombatSystem.h:12>) | `[[nodiscard]] uint32_t            NewHitId();` |
| [L13](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Combat/CombatSystem.h:13>) | `void                              ReleaseHitId(uint32_t _hitId);` |
| [L14](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Combat/CombatSystem.h:14>) | `[[nodiscard]] bool                HasTargetIn(eTeam _attackerTeam, eHitKind _kind, const Aabb& _box, uint32_t _ignoreOwner = kNoOwner) const;` |
| [L15](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Combat/CombatSystem.h:15>) | `[[nodiscard]] Span<const Hurtbox> GetHurtboxes() const;` |
| [L16](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Combat/CombatSystem.h:16>) | `[[nodiscard]] Span<const Hitbox>  GetHitboxes() const;` |
| [L17](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Combat/CombatSystem.h:17>) | `[[nodiscard]] HitResolver&        GetResolver();` |

## Source/Game/Combat/HitFlash.cpp

[HitFlash.cpp](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Combat/HitFlash.cpp>) · 27줄

| 위치 | 선언·함수 |
|---|---|
| [L3](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Combat/HitFlash.cpp:3>) | `bool IsHitFlashOn( const int _ticksLeft)` |
| [L9](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Combat/HitFlash.cpp:9>) | `bool StartHitFlash( int& _ticksLeft)` |
| [L20](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Combat/HitFlash.cpp:20>) | `void TickHitFlash( int& _ticksLeft)` |

## Source/Game/Combat/HitFlash.h

[HitFlash.h](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Combat/HitFlash.h>) · 13줄

| 위치 | 선언·함수 |
|---|---|
| [L9](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Combat/HitFlash.h:9>) | `[[nodiscard]] bool IsHitFlashOn(int _ticksLeft);` |
| [L11](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Combat/HitFlash.h:11>) | `bool StartHitFlash(int& _ticksLeft);` |
| [L13](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Combat/HitFlash.h:13>) | `void TickHitFlash(int& _ticksLeft);` |

## Source/Game/Combat/Projectiles.cpp

[Projectiles.cpp](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Combat/Projectiles.cpp>) · 613줄

| 위치 | 선언·함수 |
|---|---|
| [L12](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Combat/Projectiles.cpp:12>) | `[[nodiscard]] bool SegmentHitsGround_( const GroundMap& _ground, const VECTOR2 _from, const VECTOR2 _to, VECTOR2& _outLastFree)` |
| [L39](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Combat/Projectiles.cpp:39>) | `[[nodiscard]] bool SegmentHitsPlatform_( Span<const Platform> _platforms, const VECTOR2 _from, const VECTOR2 _to, VECTOR2& _outTop)` |
| [L67](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Combat/Projectiles.cpp:67>) | `[[nodiscard]] Aabb SweptBox_( const VECTOR2 _a, const VECTOR2 _b, const VECTOR2 _size)` |
| [L78](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Combat/Projectiles.cpp:78>) | `void ProjectileSystem::Clear()` |
| [L88](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Combat/Projectiles.cpp:88>) | `void ProjectileSystem::SpawnBullet( const BulletSpawn& _spawn, CombatSystem& _combat)` |
| [L121](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Combat/Projectiles.cpp:121>) | `void ProjectileSystem::SpawnBomb( const BombSpawn& _spawn)` |
| [L132](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Combat/Projectiles.cpp:132>) | `void ProjectileSystem::SpawnShot( const ShotSpawn& _spawn, CombatSystem& _combat)` |
| [L146](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Combat/Projectiles.cpp:146>) | `void ProjectileSystem::ExplodeBomb_( Bomb& _bomb, const VECTOR2 _at, CombatSystem& _combat)` |
| [L165](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Combat/Projectiles.cpp:165>) | `void ProjectileSystem::SpawnShotBlast_( const ShotSpawn& _params, const VECTOR2 _at, CombatSystem& _combat)` |
| [L185](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Combat/Projectiles.cpp:185>) | `void ProjectileSystem::SpawnBulletBlast_( const Bullet& _bullet, CombatSystem& _combat)` |
| [L206](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Combat/Projectiles.cpp:206>) | `void ProjectileSystem::Step( const GroundMap* _pGround, CombatSystem& _combat, const float _dt, const Span<const Platform> _platforms)` |
| [L240](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Combat/Projectiles.cpp:240>) | `else if (_pGround)` |
| [L331](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Combat/Projectiles.cpp:331>) | `else if ((_pGround \|\| !_platforms.empty()) && (params.bExplodeOnSolid \|\| bRoll) && (shot.position.x != shot.renderPrev.x \|\| shot.position.y != shot.renderPrev.y))` |
| [L495](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Combat/Projectiles.cpp:495>) | `void ProjectileSystem::OnHitEvents( const Span<const HitEvent> _events, CombatSystem& _combat)` |
| [L538](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Combat/Projectiles.cpp:538>) | `int ProjectileSystem::CountBullets( const uint32_t _owner, const uint8_t _tag) const` |
| [L553](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Combat/Projectiles.cpp:553>) | `int ProjectileSystem::CountBombs( const uint32_t _owner) const` |
| [L567](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Combat/Projectiles.cpp:567>) | `int ProjectileSystem::CountShots( const uint32_t _owner) const` |
| [L581](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Combat/Projectiles.cpp:581>) | `Vector<ShotFx> ProjectileSystem::TakeShotFx()` |
| [L588](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Combat/Projectiles.cpp:588>) | `Vector<FxRequest> ProjectileSystem::TakeFx()` |
| [L595](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Combat/Projectiles.cpp:595>) | `Span<const Bullet> ProjectileSystem::GetBullets() const` |
| [L600](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Combat/Projectiles.cpp:600>) | `Span<const Bomb> ProjectileSystem::GetBombs() const` |
| [L605](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Combat/Projectiles.cpp:605>) | `Span<const Blast> ProjectileSystem::GetBlasts() const` |
| [L610](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Combat/Projectiles.cpp:610>) | `Span<const Shot> ProjectileSystem::GetShots() const` |

## Source/Game/Combat/Projectiles.h

[Projectiles.h](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Combat/Projectiles.h>) · 202줄

| 위치 | 선언·함수 |
|---|---|
| [L8](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Combat/Projectiles.h:8>) | `struct BulletSpawn` |
| [L35](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Combat/Projectiles.h:35>) | `struct Bullet` |
| [L67](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Combat/Projectiles.h:67>) | `struct BombSpawn` |
| [L84](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Combat/Projectiles.h:84>) | `struct Bomb` |
| [L94](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Combat/Projectiles.h:94>) | `struct Blast` |
| [L104](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Combat/Projectiles.h:104>) | `struct ShotSpawn` |
| [L136](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Combat/Projectiles.h:136>) | `struct Shot` |
| [L150](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Combat/Projectiles.h:150>) | `struct ShotFx` |
| [L157](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Combat/Projectiles.h:157>) | `enum class eFxKind` |
| [L165](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Combat/Projectiles.h:165>) | `struct FxRequest` |
| [L172](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Combat/Projectiles.h:172>) | `class ProjectileSystem` |
| [L175](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Combat/Projectiles.h:175>) | `void                             Clear();` |
| [L176](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Combat/Projectiles.h:176>) | `void                             SpawnBullet(const BulletSpawn& _spawn, CombatSystem& _combat);` |
| [L177](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Combat/Projectiles.h:177>) | `void                             SpawnBomb(const BombSpawn& _spawn);` |
| [L178](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Combat/Projectiles.h:178>) | `void                             SpawnShot(const ShotSpawn& _spawn, CombatSystem& _combat);` |
| [L179](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Combat/Projectiles.h:179>) | `void                             Step(const GroundMap* _pGround, CombatSystem& _combat, float _dt, Span<const Platform> _platforms = {});` |
| [L180](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Combat/Projectiles.h:180>) | `void                             OnHitEvents(Span<const HitEvent> _events, CombatSystem& _combat);` |
| [L181](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Combat/Projectiles.h:181>) | `[[nodiscard]] int                CountBullets(uint32_t _owner, uint8_t _tag) const;` |
| [L182](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Combat/Projectiles.h:182>) | `[[nodiscard]] int                CountBombs(uint32_t _owner) const;` |
| [L183](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Combat/Projectiles.h:183>) | `[[nodiscard]] int                CountShots(uint32_t _owner) const;` |
| [L184](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Combat/Projectiles.h:184>) | `[[nodiscard]] Span<const Bullet> GetBullets() const;` |
| [L185](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Combat/Projectiles.h:185>) | `[[nodiscard]] Span<const Bomb>   GetBombs() const;` |
| [L186](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Combat/Projectiles.h:186>) | `[[nodiscard]] Span<const Blast>  GetBlasts() const;` |
| [L187](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Combat/Projectiles.h:187>) | `[[nodiscard]] Span<const Shot>   GetShots() const;` |
| [L188](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Combat/Projectiles.h:188>) | `[[nodiscard]] Vector<FxRequest>  TakeFx();` |
| [L189](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Combat/Projectiles.h:189>) | `[[nodiscard]] Vector<ShotFx>     TakeShotFx();` |
| [L192](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Combat/Projectiles.h:192>) | `void ExplodeBomb_(Bomb& _bomb, VECTOR2 _at, CombatSystem& _combat);` |
| [L193](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Combat/Projectiles.h:193>) | `void SpawnBulletBlast_(const Bullet& _bullet, CombatSystem& _combat);` |
| [L194](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Combat/Projectiles.h:194>) | `void SpawnShotBlast_(const ShotSpawn& _params, VECTOR2 _at, CombatSystem& _combat);` |

## Source/Game/DemoState.cpp

[DemoState.cpp](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/DemoState.cpp>) · 61줄

| 위치 | 선언·함수 |
|---|---|
| [L5](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/DemoState.cpp:5>) | `void DemoState::Tick( const InputHistory& _input)` |
| [L35](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/DemoState.cpp:35>) | `void DemoState::Render( const float _alpha)` |
| [L46](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/DemoState.cpp:46>) | `bool DemoState::IsFinished() const` |
| [L51](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/DemoState.cpp:51>) | `VECTOR2 DemoState::GetPosition() const` |
| [L56](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/DemoState.cpp:56>) | `uint64_t DemoState::DebugHash() const` |

## Source/Game/DemoState.h

[DemoState.h](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/DemoState.h>) · 19줄

| 위치 | 선언·함수 |
|---|---|
| [L4](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/DemoState.h:4>) | `class DemoState : public IGameState` |
| [L7](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/DemoState.h:7>) | `void                   Tick(const InputHistory& _input) override;` |
| [L8](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/DemoState.h:8>) | `void                   Render(float _alpha) override;` |
| [L9](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/DemoState.h:9>) | `[[nodiscard]] bool     IsFinished() const override;` |
| [L10](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/DemoState.h:10>) | `[[nodiscard]] VECTOR2  GetPosition() const;` |
| [L11](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/DemoState.h:11>) | `[[nodiscard]] uint64_t DebugHash() const override;` |

## Source/Game/Enemies/BigShiee.cpp

[BigShiee.cpp](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/BigShiee.cpp>) · 1380줄

| 위치 | 선언·함수 |
|---|---|
| [L51](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/BigShiee.cpp:51>) | `[[nodiscard]] ARRAY<TurretNames, kBigShieeTurretCount> MakeNames_()` |
| [L69](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/BigShiee.cpp:69>) | `[[nodiscard]] const TurretNames& GetNames_( const char _letter)` |
| [L77](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/BigShiee.cpp:77>) | `[[nodiscard]] const char* CannonModeName_( const eBigShieeCannonMode _mode)` |
| [L90](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/BigShiee.cpp:90>) | `[[nodiscard]] VECTOR2 World_( const VECTOR2 _origin, const VECTOR2 _rel)` |
| [L98](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/BigShiee.cpp:98>) | `[[nodiscard]] float Interp_( const Span<const VECTOR2> _keys, const float _t)` |
| [L122](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/BigShiee.cpp:122>) | `[[nodiscard]] float LastKeyTime_( const Span<const VECTOR2> _keys)` |
| [L133](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/BigShiee.cpp:133>) | `[[nodiscard]] int ClipTicks_( const EnemyEnv& _env, const StringView _name, const int _fallback)` |
| [L142](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/BigShiee.cpp:142>) | `[[nodiscard]] int ShotTicks_( const EnemyEnv& _env, const StringView _name)` |
| [L151](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/BigShiee.cpp:151>) | `[[nodiscard]] int RandTicks_( EnemyEnv& _env, const ARRAY<float, 2>& _rangeSec)` |
| [L160](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/BigShiee.cpp:160>) | `bool SubmitFrame_( const EnemyRenderEnv& _render, const StringView _name, const int _frame, const VECTOR2 _origin, const int _depth, const int _sort, const float _flash)` |
| [L185](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/BigShiee.cpp:185>) | `[[nodiscard]] float MuzzleX_( const BigShieeTurretConfig& _turret, const int _pos)` |
| [L192](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/BigShiee.cpp:192>) | `[[nodiscard]] float YawDeg_( const BigShieeConfig& _config, const int _yaw)` |
| [L204](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/BigShiee.cpp:204>) | `[[nodiscard]] int NearestYaw_( const BigShieeConfig& _config, const float _deg)` |
| [L220](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/BigShiee.cpp:220>) | `[[nodiscard]] int DeployClipTicks_( const BigShieeConfig& _config, const EnemyRenderEnv& _render, const int _ticks)` |
| [L235](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/BigShiee.cpp:235>) | `void BigShieeEnemy::Init( const EnemyDef& _def, EnemyEnv& _env)` |
| [L271](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/BigShiee.cpp:271>) | `void BigShieeEnemy::Tick( const EnemyDef& _def, EnemyEnv& _env)` |
| [L307](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/BigShiee.cpp:307>) | `else if ((Phase_() == eBigShieePhase::Battery \|\| Phase_() == eBigShieePhase::Cannon) && view.max.x > view.min.x)` |
| [L319](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/BigShiee.cpp:319>) | `else if (driving == 0 && bStopped && bScrolling && config.shuttleDriftSec > 0.f)` |
| [L426](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/BigShiee.cpp:426>) | `bool BigShieeEnemy::OnHit( const EnemyDef& _def, EnemyEnv& _env, const HitEvent& _event)` |
| [L474](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/BigShiee.cpp:474>) | `void BigShieeEnemy::OnDeath( const EnemyDef& _def, EnemyEnv& _env)` |
| [L487](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/BigShiee.cpp:487>) | `void BigShieeEnemy::TickDying( const EnemyDef& _def, EnemyEnv& _env)` |
| [L540](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/BigShiee.cpp:540>) | `void BigShieeEnemy::RegisterHurtboxes( const EnemyDef& _def, CombatSystem& _combat) const` |
| [L578](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/BigShiee.cpp:578>) | `void BigShieeEnemy::SubmitParts( const EnemyDef& _def, const EnemyRenderEnv& _render) const` |
| [L622](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/BigShiee.cpp:622>) | `bool BigShieeEnemy::DeathDone( const EnemyDef& _def) const` |
| [L632](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/BigShiee.cpp:632>) | `int ReadKeys_( DefReader& _r, const StringView _key, ARRAY<VECTOR2, kCapacity>& _out)` |
| [L651](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/BigShiee.cpp:651>) | `void ParseBigShieeConfig( DefReader& _r, EnemyDef& _def)` |
| [L797](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/BigShiee.cpp:797>) | `[[nodiscard]] eBigShieePhase BigShieeEnemy::Phase_() const` |
| [L803](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/BigShiee.cpp:803>) | `void BigShieeEnemy::SetTurretMode_( const int _i, const eBigShieeTurretMode _mode)` |
| [L811](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/BigShiee.cpp:811>) | `void BigShieeEnemy::SetCannonMode_( const eBigShieeCannonMode _mode)` |
| [L819](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/BigShiee.cpp:819>) | `[[nodiscard]] int BigShieeEnemy::AliveTurrets_() const` |
| [L832](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/BigShiee.cpp:832>) | `[[nodiscard]] float BigShieeEnemy::AgeSec_() const` |
| [L837](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/BigShiee.cpp:837>) | `[[nodiscard]] float BigShieeEnemy::DeploySec_() const` |
| [L843](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/BigShiee.cpp:843>) | `[[nodiscard]] bool BigShieeEnemy::IsScrollPaused_( const BigShieeConfig& _config) const` |
| [L868](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/BigShiee.cpp:868>) | `[[nodiscard]] bool BigShieeEnemy::IsScrollStopped_( const BigShieeConfig& _config) const` |
| [L874](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/BigShiee.cpp:874>) | `[[nodiscard]] VECTOR2 BigShieeEnemy::ExtraFromWorld_( const EnemyDef& _def, const int _weapon, const VECTOR2 _muzzle) const` |
| [L888](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/BigShiee.cpp:888>) | `[[nodiscard]] int BigShieeEnemy::AimPos_( const BigShieeConfig& _config, const BigShieeTurretConfig& _turret, const EnemyEnv& _env, const int _pos) const` |
| [L907](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/BigShiee.cpp:907>) | `void BigShieeEnemy::FireShell_( const EnemyDef& _def, EnemyEnv& _env, const int _turret, const int _barrel)` |
| [L924](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/BigShiee.cpp:924>) | `void BigShieeEnemy::TickTurret_( const EnemyDef& _def, EnemyEnv& _env, const int _i)` |
| [L978](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/BigShiee.cpp:978>) | `else if (turret.bTwin && ticks == config.fireTick + config.pairGapTicks)` |
| [L992](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/BigShiee.cpp:992>) | `void BigShieeEnemy::TickVolley_( const EnemyDef& _def, EnemyEnv& _env)` |
| [L1020](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/BigShiee.cpp:1020>) | `void BigShieeEnemy::TickChute_( const EnemyDef& _def, EnemyEnv& _env)` |
| [L1060](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/BigShiee.cpp:1060>) | `[[nodiscard]] bool BigShieeEnemy::CannonVulnerable_() const` |
| [L1070](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/BigShiee.cpp:1070>) | `[[nodiscard]] float BigShieeEnemy::PlayerAngleDeg_( const BigShieeConfig& _config, const EnemyEnv& _env) const` |
| [L1081](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/BigShiee.cpp:1081>) | `[[nodiscard]] int BigShieeEnemy::TargetYaw_( const BigShieeConfig& _config, const EnemyEnv& _env, const bool _bFire) const` |
| [L1106](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/BigShiee.cpp:1106>) | `void BigShieeEnemy::TurnCannon_( const BigShieeConfig& _config, const int _target)` |
| [L1125](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/BigShiee.cpp:1125>) | `[[nodiscard]] float BigShieeEnemy::CasingLandSec_( const BigShieeConfig& _config) const` |
| [L1135](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/BigShiee.cpp:1135>) | `bool BigShieeEnemy::CasingNow_( const BigShieeConfig& _config, VECTOR2& _outAt, int& _outFrame, float& _outAlpha) const` |
| [L1160](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/BigShiee.cpp:1160>) | `void BigShieeEnemy::FireMissile_( const EnemyDef& _def, EnemyEnv& _env)` |
| [L1187](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/BigShiee.cpp:1187>) | `void BigShieeEnemy::TickCannon_( const EnemyDef& _def, EnemyEnv& _env)` |
| [L1211](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/BigShiee.cpp:1211>) | `else if (mode == eBigShieeCannonMode::Aim)` |
| [L1222](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/BigShiee.cpp:1222>) | `else if (mode == eBigShieeCannonMode::Ready)` |
| [L1235](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/BigShiee.cpp:1235>) | `else if (mode == eBigShieeCannonMode::Shot)` |
| [L1249](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/BigShiee.cpp:1249>) | `void BigShieeEnemy::SubmitHull_( const BigShieeConfig& _config, const EnemyRenderEnv& _render, const VECTOR2 _feet) const` |
| [L1267](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/BigShiee.cpp:1267>) | `else if (phase == eBigShieePhase::Deploy)` |
| [L1273](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/BigShiee.cpp:1273>) | `else if (phase == eBigShieePhase::Cannon)` |
| [L1280](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/BigShiee.cpp:1280>) | `void BigShieeEnemy::SubmitTurret_( const BigShieeConfig& _config, const EnemyRenderEnv& _render, const VECTOR2 _feet, const int _i) const` |
| [L1320](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/BigShiee.cpp:1320>) | `void BigShieeEnemy::SubmitCannon_( const BigShieeConfig& _config, const EnemyRenderEnv& _render, const VECTOR2 _feet, const float _flash) const` |
| [L1349](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/BigShiee.cpp:1349>) | `void BigShieeEnemy::SubmitCasing_( const BigShieeConfig& _config, const EnemyRenderEnv& _render) const` |
| [L1377](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/BigShiee.cpp:1377>) | `void BigShieeEnemy::HashActionState(Fnv1a& _hash) const` |

## Source/Game/Enemies/BigShiee.h

[BigShiee.h](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/BigShiee.h>) · 57줄

| 위치 | 선언·함수 |
|---|---|
| [L5](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/BigShiee.h:5>) | `enum class eBigShieePhase` |
| [L13](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/BigShiee.h:13>) | `enum class eBigShieeTurretMode` |
| [L22](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/BigShiee.h:22>) | `enum class eBigShieeCannonMode` |
| [L31](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/BigShiee.h:31>) | `struct BigShieeData` |

## Source/Game/Enemies/BigShieeConfig.h

[BigShieeConfig.h](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/BigShieeConfig.h>) · 131줄

| 위치 | 선언·함수 |
|---|---|
| [L15](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/BigShieeConfig.h:15>) | `struct BigShieeTurretConfig` |
| [L28](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/BigShieeConfig.h:28>) | `struct BigShieeConfig` |

## Source/Game/Enemies/BigShieeEnemy.h

[BigShieeEnemy.h](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/BigShieeEnemy.h>) · 55줄

| 위치 | 선언·함수 |
|---|---|
| [L5](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/BigShieeEnemy.h:5>) | `class BigShieeEnemy : public Enemy` |
| [L11](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/BigShieeEnemy.h:11>) | `BigShieeEnemy() = default;` |
| [L12](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/BigShieeEnemy.h:12>) | `void               Init(const EnemyDef& _def, EnemyEnv& _env) override;` |
| [L13](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/BigShieeEnemy.h:13>) | `void               Tick(const EnemyDef& _def, EnemyEnv& _env) override;` |
| [L14](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/BigShieeEnemy.h:14>) | `[[nodiscard]] bool OnHit(const EnemyDef& _def, EnemyEnv& _env, const HitEvent& _event) override;` |
| [L15](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/BigShieeEnemy.h:15>) | `void               OnDeath(const EnemyDef& _def, EnemyEnv& _env) override;` |
| [L16](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/BigShieeEnemy.h:16>) | `void               TickDying(const EnemyDef& _def, EnemyEnv& _env) override;` |
| [L17](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/BigShieeEnemy.h:17>) | `void               RegisterHurtboxes(const EnemyDef& _def, CombatSystem& _combat) const override;` |
| [L18](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/BigShieeEnemy.h:18>) | `void               SubmitParts(const EnemyDef& _def, const EnemyRenderEnv& _render) const override;` |
| [L19](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/BigShieeEnemy.h:19>) | `[[nodiscard]] bool DeathDone(const EnemyDef& _def) const override;` |
| [L22](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/BigShieeEnemy.h:22>) | `void                         HashActionState(Fnv1a& _hash) const override;` |
| [L23](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/BigShieeEnemy.h:23>) | `[[nodiscard]] eBigShieePhase Phase_() const;` |
| [L25](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/BigShieeEnemy.h:25>) | `void SetTurretMode_(int _i, eBigShieeTurretMode _mode);` |
| [L27](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/BigShieeEnemy.h:27>) | `void                  SetCannonMode_(eBigShieeCannonMode _mode);` |
| [L28](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/BigShieeEnemy.h:28>) | `[[nodiscard]] int     AliveTurrets_() const;` |
| [L29](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/BigShieeEnemy.h:29>) | `[[nodiscard]] float   AgeSec_() const;` |
| [L30](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/BigShieeEnemy.h:30>) | `[[nodiscard]] float   DeploySec_() const;` |
| [L31](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/BigShieeEnemy.h:31>) | `[[nodiscard]] bool    IsScrollPaused_(const BigShieeConfig& _config) const;` |
| [L32](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/BigShieeEnemy.h:32>) | `[[nodiscard]] bool    IsScrollStopped_(const BigShieeConfig& _config) const;` |
| [L33](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/BigShieeEnemy.h:33>) | `[[nodiscard]] VECTOR2 ExtraFromWorld_(const EnemyDef& _def, int _weapon, VECTOR2 _muzzle) const;` |
| [L34](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/BigShieeEnemy.h:34>) | `[[nodiscard]] int     AimPos_(const BigShieeConfig& _config, const BigShieeTurretConfig& _turret, const EnemyEnv& _env, int _pos) const;` |
| [L35](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/BigShieeEnemy.h:35>) | `void                  FireShell_(const EnemyDef& _def, EnemyEnv& _env, int _turret, int _barrel);` |
| [L36](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/BigShieeEnemy.h:36>) | `void                  TickTurret_(const EnemyDef& _def, EnemyEnv& _env, int _i);` |
| [L37](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/BigShieeEnemy.h:37>) | `void                  TickVolley_(const EnemyDef& _def, EnemyEnv& _env);` |
| [L38](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/BigShieeEnemy.h:38>) | `void                  TickChute_(const EnemyDef& _def, EnemyEnv& _env);` |
| [L39](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/BigShieeEnemy.h:39>) | `[[nodiscard]] bool    CannonVulnerable_() const;` |
| [L40](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/BigShieeEnemy.h:40>) | `[[nodiscard]] float   PlayerAngleDeg_(const BigShieeConfig& _config, const EnemyEnv& _env) const;` |
| [L41](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/BigShieeEnemy.h:41>) | `[[nodiscard]] int     TargetYaw_(const BigShieeConfig& _config, const EnemyEnv& _env, bool _bFire) const;` |
| [L42](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/BigShieeEnemy.h:42>) | `void                  TurnCannon_(const BigShieeConfig& _config, int _target);` |
| [L43](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/BigShieeEnemy.h:43>) | `[[nodiscard]] float   CasingLandSec_(const BigShieeConfig& _config) const;` |
| [L44](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/BigShieeEnemy.h:44>) | `bool                  CasingNow_(const BigShieeConfig& _config, VECTOR2& _outAt, int& _outFrame, float& _outAlpha) const;` |
| [L45](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/BigShieeEnemy.h:45>) | `void                  FireMissile_(const EnemyDef& _def, EnemyEnv& _env);` |
| [L46](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/BigShieeEnemy.h:46>) | `void                  TickCannon_(const EnemyDef& _def, EnemyEnv& _env);` |
| [L47](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/BigShieeEnemy.h:47>) | `void                  SubmitHull_(const BigShieeConfig& _config, const EnemyRenderEnv& _render, VECTOR2 _feet) const;` |
| [L48](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/BigShieeEnemy.h:48>) | `void                  SubmitTurret_(const BigShieeConfig& _config, const EnemyRenderEnv& _render, VECTOR2 _feet, int _i) const;` |
| [L49](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/BigShieeEnemy.h:49>) | `void                  SubmitCannon_(const BigShieeConfig& _config, const EnemyRenderEnv& _render, VECTOR2 _feet, float _flash) const;` |
| [L50](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/BigShieeEnemy.h:50>) | `void                  SubmitCasing_(const BigShieeConfig& _config, const EnemyRenderEnv& _render) const;` |

## Source/Game/Enemies/BTankEnemy.h

[BTankEnemy.h](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/BTankEnemy.h>) · 30줄

| 위치 | 선언·함수 |
|---|---|
| [L4](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/BTankEnemy.h:4>) | `enum class eBTankState` |
| [L12](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/BTankEnemy.h:12>) | `class BTankEnemy : public Enemy` |
| [L17](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/BTankEnemy.h:17>) | `BTankEnemy() = default;` |
| [L18](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/BTankEnemy.h:18>) | `void              Init(const EnemyDef& _def, EnemyEnv& _env) override;` |
| [L19](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/BTankEnemy.h:19>) | `void              Tick(const EnemyDef& _def, EnemyEnv& _env) override;` |
| [L20](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/BTankEnemy.h:20>) | `void              OnDeath(const EnemyDef& _def, EnemyEnv& _env) override;` |
| [L21](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/BTankEnemy.h:21>) | `void              TickDying(const EnemyDef& _def, EnemyEnv& _env) override;` |
| [L22](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/BTankEnemy.h:22>) | `[[nodiscard]] int GetShotCounter() const noexcept override;` |
| [L25](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/BTankEnemy.h:25>) | `void              HashActionState(Fnv1a& _hash) const override;` |
| [L26](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/BTankEnemy.h:26>) | `[[nodiscard]] int ClipOf_(const BTankConfig& _config) const;` |
| [L27](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/BTankEnemy.h:27>) | `void              Go_(const EnemyDef& _def, eBTankState _state);` |

## Source/Game/Enemies/Bus.cpp

[Bus.cpp](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/Bus.cpp>) · 597줄

| 위치 | 선언·함수 |
|---|---|
| [L23](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/Bus.cpp:23>) | `[[nodiscard]] VECTOR2 Local_( const BusConfig& _config, const VECTOR2 _feet, const VECTOR2 _at)` |
| [L31](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/Bus.cpp:31>) | `void SubmitPart_( const EnemyRenderEnv& _render, const EnemyPartDepth& _depth, const StringView _clip, const VECTOR2 _at, const int _ticks, const bool _bLoop)` |
| [L43](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/Bus.cpp:43>) | `[[nodiscard]] int BodyLevel_( const int _stage)` |
| [L57](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/Bus.cpp:57>) | `[[nodiscard]] String Named_( const char* _base, const int _level)` |
| [L64](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/Bus.cpp:64>) | `void SpawnBursts_( EnemyEnv& _env, const EnemyState& _e, const EnemyDef& _def, const int _stage, const bool _bDeath)` |
| [L92](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/Bus.cpp:92>) | `void SubmitDamaged_( const EnemyRenderEnv& _render, const BusConfig& _config, const int _level, const VECTOR2 _feet, const bool _bCover)` |
| [L118](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/Bus.cpp:118>) | `void BusEnemy::Init( const EnemyDef& _def, EnemyEnv& _env)` |
| [L134](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/Bus.cpp:134>) | `void BusEnemy::Tick( const EnemyDef& _def, EnemyEnv& _env)` |
| [L159](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/Bus.cpp:159>) | `void BusEnemy::OnDeath( const EnemyDef& _def, EnemyEnv& _env)` |
| [L168](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/Bus.cpp:168>) | `void BusEnemy::TickDying( const EnemyDef& _def, EnemyEnv& _env)` |
| [L179](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/Bus.cpp:179>) | `void BusEnemy::SubmitParts( const EnemyDef& _def, const EnemyRenderEnv& _render) const` |
| [L201](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/Bus.cpp:201>) | `bool BusEnemy::DeathDone( const EnemyDef& _def) const` |
| [L210](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/Bus.cpp:210>) | `bool ReadNumField_( const JsonReader& _o, const StringView _key, float& _out)` |
| [L219](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/Bus.cpp:219>) | `bool ReadIntField_( const JsonReader& _o, const StringView _key, int& _out)` |
| [L233](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/Bus.cpp:233>) | `bool ReadStrField_( const JsonReader& _o, const StringView _key, String& _out)` |
| [L242](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/Bus.cpp:242>) | `bool ReadVecField_( const JsonReader& _o, const StringView _key, VECTOR2& _out)` |
| [L251](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/Bus.cpp:251>) | `void ParseBursts_( DefReader& _r, const StringView _key, Vector<BusBurstDef>& _out)` |
| [L280](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/Bus.cpp:280>) | `void ParseDebris_( DefReader& _r, Vector<BusDebrisDef>& _out)` |
| [L308](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/Bus.cpp:308>) | `void ParseInnerSoldiers_( DefReader& _r, Vector<BusInnerSoldierDef>& _out)` |
| [L336](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/Bus.cpp:336>) | `void ParseWindows_( DefReader& _r, Vector<BusWindowDef>& _out)` |
| [L365](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/Bus.cpp:365>) | `void ParseBusConfig( DefReader& _r, EnemyDef& _def)` |
| [L423](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/Bus.cpp:423>) | `void BusEnemy::EnterStage_( const EnemyDef& _def, EnemyEnv& _env, const int _stage)` |
| [L433](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/Bus.cpp:433>) | `void BusEnemy::SpawnStairSoldier_( const EnemyDef& _def, EnemyEnv& _env)` |
| [L447](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/Bus.cpp:447>) | `else if (roll < config.swordPercent + config.bazookaPercent)` |
| [L473](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/Bus.cpp:473>) | `void BusEnemy::TickSoldiers_( const EnemyDef& _def, EnemyEnv& _env)` |
| [L493](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/Bus.cpp:493>) | `void BusEnemy::UpdateStage_( const EnemyDef& _def, EnemyEnv& _env)` |
| [L506](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/Bus.cpp:506>) | `void BusEnemy::SubmitDebris_( const EnemyDef& _def, const EnemyRenderEnv& _render) const` |
| [L531](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/Bus.cpp:531>) | `void BusEnemy::SubmitLive_( const EnemyDef& _def, const EnemyRenderEnv& _render) const` |
| [L564](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/Bus.cpp:564>) | `else if (stairsT < kFlyTicks + kLandTicks)` |
| [L594](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/Bus.cpp:594>) | `void BusEnemy::HashActionState(Fnv1a& _hash) const` |

## Source/Game/Enemies/BusEnemy.h

[BusEnemy.h](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/BusEnemy.h>) · 39줄

| 위치 | 선언·함수 |
|---|---|
| [L4](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/BusEnemy.h:4>) | `enum class eBusState` |
| [L10](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/BusEnemy.h:10>) | `class BusEnemy : public Enemy` |
| [L17](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/BusEnemy.h:17>) | `BusEnemy() = default;` |
| [L18](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/BusEnemy.h:18>) | `void               Init(const EnemyDef& _def, EnemyEnv& _env) override;` |
| [L19](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/BusEnemy.h:19>) | `void               Tick(const EnemyDef& _def, EnemyEnv& _env) override;` |
| [L20](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/BusEnemy.h:20>) | `void               OnDeath(const EnemyDef& _def, EnemyEnv& _env) override;` |
| [L21](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/BusEnemy.h:21>) | `void               TickDying(const EnemyDef& _def, EnemyEnv& _env) override;` |
| [L22](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/BusEnemy.h:22>) | `void               SubmitParts(const EnemyDef& _def, const EnemyRenderEnv& _render) const override;` |
| [L23](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/BusEnemy.h:23>) | `[[nodiscard]] bool DeathDone(const EnemyDef& _def) const override;` |
| [L26](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/BusEnemy.h:26>) | `void HashActionState(Fnv1a& _hash) const override;` |
| [L27](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/BusEnemy.h:27>) | `void EnterStage_(const EnemyDef& _def, EnemyEnv& _env, int _stage);` |
| [L28](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/BusEnemy.h:28>) | `void SpawnStairSoldier_(const EnemyDef& _def, EnemyEnv& _env);` |
| [L29](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/BusEnemy.h:29>) | `void TickSoldiers_(const EnemyDef& _def, EnemyEnv& _env);` |
| [L30](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/BusEnemy.h:30>) | `void UpdateStage_(const EnemyDef& _def, EnemyEnv& _env);` |
| [L31](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/BusEnemy.h:31>) | `void SubmitDebris_(const EnemyDef& _def, const EnemyRenderEnv& _render) const;` |
| [L32](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/BusEnemy.h:32>) | `void SubmitLive_(const EnemyDef& _def, const EnemyRenderEnv& _render) const;` |

## Source/Game/Enemies/Bzk.cpp

[Bzk.cpp](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/Bzk.cpp>) · 368줄

| 위치 | 선언·함수 |
|---|---|
| [L11](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/Bzk.cpp:11>) | `void BzkEnemy::Init( const EnemyDef& _def, EnemyEnv& _env)` |
| [L20](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/Bzk.cpp:20>) | `void BzkEnemy::Tick( const EnemyDef& _def, EnemyEnv& _env)` |
| [L75](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/Bzk.cpp:75>) | `else if (WallAhead(_e, _env, config.wallProbeW, config.wallProbeH, config.wallProbeX, config.wallProbeY))` |
| [L79](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/Bzk.cpp:79>) | `else if (_e.stateTicks >= config.cooldownTicks)` |
| [L192](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/Bzk.cpp:192>) | `void BzkEnemy::OnDeath( const EnemyDef& _def, EnemyEnv& _env)` |
| [L200](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/Bzk.cpp:200>) | `void BzkEnemy::TickDying( const EnemyDef& _def, EnemyEnv& _env)` |
| [L207](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/Bzk.cpp:207>) | `void ParseBzkConfig( DefReader& _r, EnemyDef& _def)` |
| [L250](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/Bzk.cpp:250>) | `void BzkEnemy::Go_( const EnemyDef& _def, EnemyEnv& _env, const eBzkState _state)` |
| [L328](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/Bzk.cpp:328>) | `void BzkEnemy::TickClimb_( const EnemyDef& _def, EnemyEnv& _env)` |
| [L343](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/Bzk.cpp:343>) | `[[nodiscard]] int BzkEnemy::ChoosePosture_( const BzkConfig& _config, const EnemyEnv& _env, bool& _bDown) const` |
| [L365](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/Bzk.cpp:365>) | `void BzkEnemy::HashActionState(Fnv1a& _hash) const` |

## Source/Game/Enemies/BzkEnemy.h

[BzkEnemy.h](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/BzkEnemy.h>) · 42줄

| 위치 | 선언·함수 |
|---|---|
| [L4](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/BzkEnemy.h:4>) | `enum class eBzkState` |
| [L19](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/BzkEnemy.h:19>) | `class BzkEnemy : public Enemy` |
| [L25](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/BzkEnemy.h:25>) | `BzkEnemy() = default;` |
| [L26](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/BzkEnemy.h:26>) | `void Init(const EnemyDef& _def, EnemyEnv& _env) override;` |
| [L27](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/BzkEnemy.h:27>) | `void Tick(const EnemyDef& _def, EnemyEnv& _env) override;` |
| [L28](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/BzkEnemy.h:28>) | `void OnDeath(const EnemyDef& _def, EnemyEnv& _env) override;` |
| [L29](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/BzkEnemy.h:29>) | `void TickDying(const EnemyDef& _def, EnemyEnv& _env) override;` |
| [L32](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/BzkEnemy.h:32>) | `void              HashActionState(Fnv1a& _hash) const override;` |
| [L33](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/BzkEnemy.h:33>) | `void              Go_(const EnemyDef& _def, EnemyEnv& _env, eBzkState _state);` |
| [L34](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/BzkEnemy.h:34>) | `void              TickClimb_(const EnemyDef& _def, EnemyEnv& _env);` |
| [L35](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/BzkEnemy.h:35>) | `[[nodiscard]] int ChoosePosture_(const BzkConfig& _config, const EnemyEnv& _env, bool& _bDown) const;` |

## Source/Game/Enemies/Enemy.cpp

[Enemy.cpp](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/Enemy.cpp>) · 93줄

| 위치 | 선언·함수 |
|---|---|
| [L18](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/Enemy.cpp:18>) | `bool Enemy::OnHit(const EnemyDef&, EnemyEnv&, const HitEvent&)` |
| [L23](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/Enemy.cpp:23>) | `void Enemy::OnDeath(const EnemyDef&, EnemyEnv&)` |
| [L27](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/Enemy.cpp:27>) | `void Enemy::TickDying(const EnemyDef&, EnemyEnv&)` |
| [L32](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/Enemy.cpp:32>) | `void Enemy::RegisterHurtboxes(const EnemyDef& _def, CombatSystem& _combat) const` |
| [L42](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/Enemy.cpp:42>) | `void Enemy::SubmitParts(const EnemyDef&, const EnemyRenderEnv&) const` |
| [L46](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/Enemy.cpp:46>) | `bool Enemy::DeathDone(const EnemyDef&) const` |
| [L51](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/Enemy.cpp:51>) | `const AnimFrame* Enemy::BodyFrame(const EnemyDef&, const AnimationLibrary&) const` |
| [L56](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/Enemy.cpp:56>) | `Scoped<Enemy> MakeEnemy(const eEnemyBehavior _behavior)` |
| [L80](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/Enemy.cpp:80>) | `void Enemy::HashActionState(Fnv1a& _hash) const` |
| [L85](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/Enemy.cpp:85>) | `void Enemy::HashActionState_(Fnv1a& _hash, const int _shots, const int _progress, const int _secondary, const int _tertiary, const int _quaternary) const` |

## Source/Game/Enemies/Enemy.h

[Enemy.h](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/Enemy.h>) · 77줄

| 위치 | 선언·함수 |
|---|---|
| [L12](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/Enemy.h:12>) | `class EnemyWorld;` |
| [L14](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/Enemy.h:14>) | `struct EnemyFxRequest` |
| [L25](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/Enemy.h:25>) | `struct EnemyEnv` |
| [L34](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/Enemy.h:34>) | `struct EnemyRenderEnv` |
| [L47](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/Enemy.h:47>) | `class Enemy` |
| [L53](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/Enemy.h:53>) | `Enemy()          = default;` |
| [L54](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/Enemy.h:54>) | `virtual ~Enemy() = default;` |
| [L57](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/Enemy.h:57>) | `virtual void                           Init(const EnemyDef& _def, EnemyEnv& _env) = 0;` |
| [L58](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/Enemy.h:58>) | `virtual void                           Tick(const EnemyDef& _def, EnemyEnv& _env) = 0;` |
| [L59](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/Enemy.h:59>) | `virtual bool                           OnHit(const EnemyDef& _def, EnemyEnv& _env, const HitEvent& _event);` |
| [L60](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/Enemy.h:60>) | `virtual void                           OnDeath(const EnemyDef& _def, EnemyEnv& _env);` |
| [L61](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/Enemy.h:61>) | `virtual void                           TickDying(const EnemyDef& _def, EnemyEnv& _env);` |
| [L62](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/Enemy.h:62>) | `virtual void                           RegisterHurtboxes(const EnemyDef& _def, CombatSystem& _combat) const;` |
| [L63](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/Enemy.h:63>) | `virtual void                           SubmitParts(const EnemyDef& _def, const EnemyRenderEnv& _render) const;` |
| [L64](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/Enemy.h:64>) | `[[nodiscard]] virtual bool             DeathDone(const EnemyDef& _def) const;` |
| [L65](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/Enemy.h:65>) | `[[nodiscard]] virtual const AnimFrame* BodyFrame(const EnemyDef& _def, const AnimationLibrary& _anims) const;` |
| [L68](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/Enemy.h:68>) | `void                      HashActionState_(Fnv1a& _hash, int _shots, int _progress, int _secondary = 0, int _tertiary = 0, int _quaternary = 0) const;` |
| [L72](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/Enemy.h:72>) | `virtual void HashActionState(Fnv1a& _hash) const;` |
| [L77](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/Enemy.h:77>) | `[[nodiscard]] Scoped<Enemy> MakeEnemy(eEnemyBehavior _behavior);` |

## Source/Game/Enemies/EnemyCommon.cpp

[EnemyCommon.cpp](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/EnemyCommon.cpp>) · 769줄

| 위치 | 선언·함수 |
|---|---|
| [L27](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/EnemyCommon.cpp:27>) | `[[nodiscard]] bool IsFinalPhase_( const eDeathPhase _phase)` |
| [L33](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/EnemyCommon.cpp:33>) | `void EnterDeathLand_( EnemyState& _e, const DeathDef& _death)` |
| [L43](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/EnemyCommon.cpp:43>) | `float DistToPlayer( const EnemyState& _e, const EnemyEnv& _env)` |
| [L52](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/EnemyCommon.cpp:52>) | `int SideToPlayer( const EnemyState& _e, const EnemyEnv& _env)` |
| [L64](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/EnemyCommon.cpp:64>) | `void FacePlayer( EnemyState& _e, const EnemyEnv& _env)` |
| [L71](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/EnemyCommon.cpp:71>) | `void EnterState( EnemyState& _e, const int _state)` |
| [L79](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/EnemyCommon.cpp:79>) | `void SetClip( EnemyState& _e, const int _clip, const bool _bRestart)` |
| [L91](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/EnemyCommon.cpp:91>) | `bool ClipReachedFrame( const EnemyState& _e, const EnemyDef& _def, const int _frame)` |
| [L103](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/EnemyCommon.cpp:103>) | `bool ClipFinished( const EnemyState& _e, const EnemyDef& _def)` |
| [L110](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/EnemyCommon.cpp:110>) | `bool ClimbStep( EnemyState& _e, const EnemyDef& _def)` |
| [L119](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/EnemyCommon.cpp:119>) | `else if (_e.climbSegment == 2)` |
| [L149](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/EnemyCommon.cpp:149>) | `void ClimbLeap( EnemyState& _e, const EnemyDef& _def)` |
| [L160](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/EnemyCommon.cpp:160>) | `bool OnRoof( const EnemyState& _e)` |
| [L166](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/EnemyCommon.cpp:166>) | `bool AtRoofEdge( const EnemyState& _e)` |
| [L176](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/EnemyCommon.cpp:176>) | `bool CanLeaveRoof( const EnemyState& _e)` |
| [L186](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/EnemyCommon.cpp:186>) | `void HoldOnRoof( EnemyState& _e)` |
| [L201](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/EnemyCommon.cpp:201>) | `void PrepareBody( EnemyState& _e, const EnemyDef& _def)` |
| [L209](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/EnemyCommon.cpp:209>) | `MoveResult StepPhysics( EnemyState& _e, const EnemyDef& _def, EnemyEnv& _env)` |
| [L225](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/EnemyCommon.cpp:225>) | `bool WallAhead( const EnemyState& _e, const EnemyEnv& _env, const float _w, const float _h, const float _x, const float _y)` |
| [L261](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/EnemyCommon.cpp:261>) | `bool FloorAhead( const EnemyState& _e, const EnemyEnv& _env, const float _dist, const float _drop)` |
| [L274](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/EnemyCommon.cpp:274>) | `String ResolveSide( const String& _name, const int _facing)` |
| [L288](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/EnemyCommon.cpp:288>) | `void RequestFx( EnemyEnv& _env, const FxDef& _fx, const VECTOR2 _origin, const int _facing)` |
| [L315](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/EnemyCommon.cpp:315>) | `VECTOR2 GetMuzzleWorld( const EnemyState& _e, const EnemyDef& _def, const int _weapon)` |
| [L324](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/EnemyCommon.cpp:324>) | `void TickWeaponEffects( const EnemyState& _e, const EnemyDef& _def, EnemyEnv& _env)` |
| [L351](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/EnemyCommon.cpp:351>) | `bool FireWeapon( EnemyState& _e, const EnemyDef& _def, EnemyEnv& _env, const int _weapon, const VECTOR2 _dir, const VECTOR2 _muzzleExtra)` |
| [L421](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/EnemyCommon.cpp:421>) | `else if (proj.bRamp)` |
| [L463](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/EnemyCommon.cpp:463>) | `void StartMelee( EnemyState& _e, const EnemyDef& _def, EnemyEnv& _env, const int _weapon)` |
| [L477](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/EnemyCommon.cpp:477>) | `void TickMelee( EnemyState& _e, const EnemyDef& _def, EnemyEnv& _env)` |
| [L512](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/EnemyCommon.cpp:512>) | `void CancelMelee( EnemyState& _e, EnemyEnv& _env)` |
| [L524](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/EnemyCommon.cpp:524>) | `void BeginSoldierDeath( EnemyState& _e, const EnemyDef& _def, EnemyEnv& _env)` |
| [L552](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/EnemyCommon.cpp:552>) | `else if (_e.lastHitKind == eHitKind::Melee && death.clipSlash >= 0)` |
| [L557](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/EnemyCommon.cpp:557>) | `else if (_e.bGrounded && death.clipStand1 >= 0)` |
| [L588](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/EnemyCommon.cpp:588>) | `void TickSoldierDeath( EnemyState& _e, const EnemyDef& _def, EnemyEnv& _env)` |
| [L616](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/EnemyCommon.cpp:616>) | `else if (bFell)` |
| [L684](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/EnemyCommon.cpp:684>) | `bool SubmitNamedPart( const EnemyRenderEnv& _render, const StringView _name, const VECTOR2 _origin, const int _ticks, const bool _bLoop, const int _sortKey, const int _depth, const bool _bFlipX, const float _partFlash)` |
| [L738](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/EnemyCommon.cpp:738>) | `int GetNamedPartTicks( const AnimationLibrary& _anims, const StringView _name)` |
| [L750](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/EnemyCommon.cpp:750>) | `float DxToPlayer( const EnemyState& _e, const EnemyEnv& _env)` |
| [L757](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/EnemyCommon.cpp:757>) | `float DyToPlayer( const EnemyState& _e, const EnemyEnv& _env)` |
| [L764](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/EnemyCommon.cpp:764>) | `int Roll( EnemyEnv& _env, const int _n)` |

## Source/Game/Enemies/EnemyCommon.h

[EnemyCommon.h](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/EnemyCommon.h>) · 59줄

| 위치 | 선언·함수 |
|---|---|
| [L6](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/EnemyCommon.h:6>) | `[[nodiscard]] float DxToPlayer(const EnemyState& _e, const EnemyEnv& _env);` |
| [L8](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/EnemyCommon.h:8>) | `[[nodiscard]] float DyToPlayer(const EnemyState& _e, const EnemyEnv& _env);` |
| [L10](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/EnemyCommon.h:10>) | `[[nodiscard]] float DistToPlayer(const EnemyState& _e, const EnemyEnv& _env);` |
| [L12](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/EnemyCommon.h:12>) | `[[nodiscard]] int SideToPlayer(const EnemyState& _e, const EnemyEnv& _env);` |
| [L13](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/EnemyCommon.h:13>) | `void              FacePlayer(EnemyState& _e, const EnemyEnv& _env);` |
| [L15](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/EnemyCommon.h:15>) | `void EnterState(EnemyState& _e, int _state);` |
| [L16](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/EnemyCommon.h:16>) | `void SetClip(EnemyState& _e, int _clip, bool _bRestart = false);` |
| [L18](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/EnemyCommon.h:18>) | `[[nodiscard]] bool ClipReachedFrame(const EnemyState& _e, const EnemyDef& _def, int _frame);` |
| [L19](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/EnemyCommon.h:19>) | `[[nodiscard]] bool ClipFinished(const EnemyState& _e, const EnemyDef& _def);` |
| [L21](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/EnemyCommon.h:21>) | `[[nodiscard]] bool ClimbStep(EnemyState& _e, const EnemyDef& _def);` |
| [L22](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/EnemyCommon.h:22>) | `void               ClimbLeap(EnemyState& _e, const EnemyDef& _def);` |
| [L24](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/EnemyCommon.h:24>) | `[[nodiscard]] bool OnRoof(const EnemyState& _e);` |
| [L25](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/EnemyCommon.h:25>) | `[[nodiscard]] bool AtRoofEdge(const EnemyState& _e);` |
| [L26](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/EnemyCommon.h:26>) | `[[nodiscard]] bool CanLeaveRoof(const EnemyState& _e);` |
| [L28](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/EnemyCommon.h:28>) | `void HoldOnRoof(EnemyState& _e);` |
| [L30](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/EnemyCommon.h:30>) | `MoveResult StepPhysics(EnemyState& _e, const EnemyDef& _def, EnemyEnv& _env);` |
| [L31](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/EnemyCommon.h:31>) | `void       PrepareBody(EnemyState& _e, const EnemyDef& _def);` |
| [L33](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/EnemyCommon.h:33>) | `[[nodiscard]] bool WallAhead(const EnemyState& _e, const EnemyEnv& _env, float _w, float _h, float _x, float _y);` |
| [L35](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/EnemyCommon.h:35>) | `[[nodiscard]] bool FloorAhead(const EnemyState& _e, const EnemyEnv& _env, float _dist, float _drop);` |
| [L37](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/EnemyCommon.h:37>) | `[[nodiscard]] int Roll(EnemyEnv& _env, int _n);` |
| [L39](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/EnemyCommon.h:39>) | `void RequestFx(EnemyEnv& _env, const FxDef& _fx, VECTOR2 _origin, int _facing);` |
| [L41](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/EnemyCommon.h:41>) | `[[nodiscard]] String ResolveSide(const String& _name, int _facing);` |
| [L43](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/EnemyCommon.h:43>) | `bool FireWeapon(EnemyState& _e, const EnemyDef& _def, EnemyEnv& _env, int _weapon, VECTOR2 _dir = { 0.f, 0.f }, VECTOR2 _muzzleExtra = { 0.f, 0.f });` |
| [L45](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/EnemyCommon.h:45>) | `[[nodiscard]] VECTOR2 GetMuzzleWorld(const EnemyState& _e, const EnemyDef& _def, int _weapon);` |
| [L47](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/EnemyCommon.h:47>) | `void TickWeaponEffects(const EnemyState& _e, const EnemyDef& _def, EnemyEnv& _env);` |
| [L49](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/EnemyCommon.h:49>) | `void StartMelee(EnemyState& _e, const EnemyDef& _def, EnemyEnv& _env, int _weapon);` |
| [L51](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/EnemyCommon.h:51>) | `void TickMelee(EnemyState& _e, const EnemyDef& _def, EnemyEnv& _env);` |
| [L52](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/EnemyCommon.h:52>) | `void CancelMelee(EnemyState& _e, EnemyEnv& _env);` |
| [L54](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/EnemyCommon.h:54>) | `void BeginSoldierDeath(EnemyState& _e, const EnemyDef& _def, EnemyEnv& _env);` |
| [L55](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/EnemyCommon.h:55>) | `void TickSoldierDeath(EnemyState& _e, const EnemyDef& _def, EnemyEnv& _env);` |
| [L57](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/EnemyCommon.h:57>) | `bool SubmitNamedPart(const EnemyRenderEnv& _render, StringView _name, VECTOR2 _origin, int _ticks, bool _bLoop, int _sortKey, int _depth, bool _bFlipX = false, float _partFlash = 0.f);` |
| [L59](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/EnemyCommon.h:59>) | `[[nodiscard]] int GetNamedPartTicks(const AnimationLibrary& _anims, StringView _name);` |

## Source/Game/Enemies/EnemyDef.cpp

[EnemyDef.cpp](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/EnemyDef.cpp>) · 543줄

| 위치 | 선언·함수 |
|---|---|
| [L12](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/EnemyDef.cpp:12>) | `const char* GetBehaviorName( const eEnemyBehavior _behavior)` |
| [L36](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/EnemyDef.cpp:36>) | `std::optional<eEnemyBehavior> FindBehavior( const StringView _name)` |
| [L50](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/EnemyDef.cpp:50>) | `int SecToTicks( const float _sec)` |
| [L56](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/EnemyDef.cpp:56>) | `int ClipDef::TickOfFrame( const int _frame) const` |
| [L62](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/EnemyDef.cpp:62>) | `int ClipDef::GetDurationTicks() const` |
| [L67](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/EnemyDef.cpp:67>) | `int EnemyDef::FindClip( const StringView _logicalName) const` |
| [L80](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/EnemyDef.cpp:80>) | `bool EnemyDef::ShouldFlipArt( const int _facing) const` |
| [L86](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/EnemyDef.cpp:86>) | `int EnemyDef::FindWeapon( const StringView _name) const` |
| [L119](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/EnemyDef.cpp:119>) | `void DefReader::Fail( const StringView _field, const StringView _message)` |
| [L129](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/EnemyDef.cpp:129>) | `ParamEntry* DefReader::Find_( const StringView _key)` |
| [L143](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/EnemyDef.cpp:143>) | `bool DefReader::Has( const StringView _key) const` |
| [L156](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/EnemyDef.cpp:156>) | `const JsonReader* DefReader::FindRaw( const StringView _key)` |
| [L163](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/EnemyDef.cpp:163>) | `float DefReader::Num( const StringView _key)` |
| [L181](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/EnemyDef.cpp:181>) | `float DefReader::Num( const StringView _key, const float _default)` |
| [L188](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/EnemyDef.cpp:188>) | `int DefReader::Int( const StringView _key)` |
| [L194](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/EnemyDef.cpp:194>) | `int DefReader::Int( const StringView _key, const int _default)` |
| [L201](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/EnemyDef.cpp:201>) | `int DefReader::Ticks( const StringView _key)` |
| [L207](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/EnemyDef.cpp:207>) | `int DefReader::Ticks( const StringView _key, const float _defaultSec)` |
| [L214](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/EnemyDef.cpp:214>) | `bool DefReader::Bool( const StringView _key, const bool _bDefault)` |
| [L232](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/EnemyDef.cpp:232>) | `String DefReader::Str( const StringView _key)` |
| [L250](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/EnemyDef.cpp:250>) | `VECTOR2 DefReader::ReadVec2( const StringView _key, const VECTOR2 _default)` |
| [L279](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/EnemyDef.cpp:279>) | `BoxDef DefReader::Box( const StringView _key)` |
| [L296](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/EnemyDef.cpp:296>) | `void DefReader::ReadBoxField_( const ParamEntry& _entry, const StringView _key, const StringView _name, float& _out, const bool _bRequired)` |
| [L310](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/EnemyDef.cpp:310>) | `bool DefReader::NumArray( const StringView _key, const Span<float> _out)` |
| [L340](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/EnemyDef.cpp:340>) | `int DefReader::NumList( const StringView _key, const Span<float> _out)` |
| [L369](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/EnemyDef.cpp:369>) | `int DefReader::Clip( const StringView _logical)` |
| [L380](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/EnemyDef.cpp:380>) | `int DefReader::ClipOpt( const StringView _logical) const` |
| [L386](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/EnemyDef.cpp:386>) | `int DefReader::Weapon( const StringView _name)` |
| [L397](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/EnemyDef.cpp:397>) | `bool DefReader::Finish()` |
| [L409](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/EnemyDef.cpp:409>) | `const EnemyDef& EnemyDefLibrary::Get( const int _index) const` |
| [L415](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/EnemyDef.cpp:415>) | `const ProjectileDef& EnemyDefLibrary::GetProjectile( const int _index) const` |
| [L421](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/EnemyDef.cpp:421>) | `size_t EnemyDefLibrary::GetDefCount() const` |
| [L426](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/EnemyDef.cpp:426>) | `size_t EnemyDefLibrary::GetProjectileCount() const` |
| [L431](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/EnemyDef.cpp:431>) | `Span<const EnemyDef> EnemyDefLibrary::GetDefs() const` |
| [L436](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/EnemyDef.cpp:436>) | `Span<const ProjectileDef> EnemyDefLibrary::GetProjectiles() const` |
| [L441](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/EnemyDef.cpp:441>) | `const EnemyGlobals& EnemyDefLibrary::GetGlobals() const` |
| [L451](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/EnemyDef.cpp:451>) | `void GetEnemyPartSlots( EnemyDef& _def, Vector<EnemyPartSlot>& _out)` |
| [L468](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/EnemyDef.cpp:468>) | `else if (_def.behavior == eEnemyBehavior::Heli)` |
| [L475](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/EnemyDef.cpp:475>) | `else if (_def.behavior == eEnemyBehavior::BigShiee)` |
| [L494](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/EnemyDef.cpp:494>) | `void ParseEnemyParts( DefReader& _r, EnemyDef& _def)` |

## Source/Game/Enemies/EnemyDef.h

[EnemyDef.h](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/EnemyDef.h>) · 835줄

| 위치 | 선언·함수 |
|---|---|
| [L12](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/EnemyDef.h:12>) | `enum class eEnemyBehavior` |
| [L31](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/EnemyDef.h:31>) | `[[nodiscard]] const char*                   GetBehaviorName(eEnemyBehavior _behavior);` |
| [L32](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/EnemyDef.h:32>) | `[[nodiscard]] std::optional<eEnemyBehavior> FindBehavior(StringView _name);` |
| [L36](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/EnemyDef.h:36>) | `[[nodiscard]] int SecToTicks(float _sec);` |
| [L38](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/EnemyDef.h:38>) | `struct BoxDef` |
| [L47](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/EnemyDef.h:47>) | `const float cxWorld = _feet.x + cx * static_cast<float>(_facing < 0 ? -1 : 1);` |
| [L48](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/EnemyDef.h:48>) | `return Aabb::FromCenterSize({ cxWorld, _feet.y + cy }, { w, h });` |
| [L57](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/EnemyDef.h:57>) | `struct ClipDef` |
| [L68](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/EnemyDef.h:68>) | `return !name.empty();` |
| [L71](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/EnemyDef.h:71>) | `[[nodiscard]] int TickOfFrame(int _frame) const;` |
| [L72](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/EnemyDef.h:72>) | `[[nodiscard]] int GetDurationTicks() const;` |
| [L76](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/EnemyDef.h:76>) | `return !bLoop && _ticks >= GetDurationTicks();` |
| [L80](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/EnemyDef.h:80>) | `struct NamedClip` |
| [L86](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/EnemyDef.h:86>) | `struct FxDef` |
| [L96](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/EnemyDef.h:96>) | `struct ProjectileDef` |
| [L131](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/EnemyDef.h:131>) | `enum class eWeaponAim` |
| [L137](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/EnemyDef.h:137>) | `struct WeaponEffect` |
| [L145](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/EnemyDef.h:145>) | `struct WeaponDef` |
| [L166](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/EnemyDef.h:166>) | `struct DeathDef` |
| [L186](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/EnemyDef.h:186>) | `struct DropDef` |
| [L192](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/EnemyDef.h:192>) | `struct ActivationDef` |
| [L199](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/EnemyDef.h:199>) | `struct PhysicsDef` |
| [L210](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/EnemyDef.h:210>) | `enum class eSoldierAi` |
| [L217](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/EnemyDef.h:217>) | `struct SoldierConfig` |
| [L255](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/EnemyDef.h:255>) | `struct BzkConfig` |
| [L289](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/EnemyDef.h:289>) | `struct HangConfig` |
| [L299](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/EnemyDef.h:299>) | `struct FlyingBzkConfig` |
| [L327](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/EnemyDef.h:327>) | `struct ShielderConfig` |
| [L383](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/EnemyDef.h:383>) | `struct PowConfig` |
| [L417](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/EnemyDef.h:417>) | `struct JeepConfig` |
| [L441](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/EnemyDef.h:441>) | `struct BTankConfig` |
| [L455](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/EnemyDef.h:455>) | `struct MissileTankConfig` |
| [L479](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/EnemyDef.h:479>) | `struct SarubiaConfig` |
| [L499](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/EnemyDef.h:499>) | `struct BusBurstDef` |
| [L508](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/EnemyDef.h:508>) | `struct BusDebrisDef` |
| [L517](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/EnemyDef.h:517>) | `struct BusInnerSoldierDef` |
| [L524](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/EnemyDef.h:524>) | `struct BusWindowDef` |
| [L531](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/EnemyDef.h:531>) | `struct BusConfig` |
| [L631](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/EnemyDef.h:631>) | `struct HeliConfig` |
| [L691](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/EnemyDef.h:691>) | `struct HeliBombConfig` |
| [L709](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/EnemyDef.h:709>) | `enum class eExtraBoxKind` |
| [L715](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/EnemyDef.h:715>) | `struct ExtraBoxDef` |
| [L725](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/EnemyDef.h:725>) | `enum class eArtFacing` |
| [L732](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/EnemyDef.h:732>) | `struct EnemyDef` |
| [L783](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/EnemyDef.h:783>) | `return clips[static_cast<size_t>(_index)].clip;` |
| [L786](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/EnemyDef.h:786>) | `[[nodiscard]] int  FindClip(StringView _logicalName) const;` |
| [L787](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/EnemyDef.h:787>) | `[[nodiscard]] int  FindWeapon(StringView _name) const;` |
| [L788](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/EnemyDef.h:788>) | `[[nodiscard]] bool ShouldFlipArt(int _facing) const;` |
| [L791](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/EnemyDef.h:791>) | `struct EnemyGlobals` |
| [L799](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/EnemyDef.h:799>) | `class EnemyDefLibrary` |
| [L802](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/EnemyDef.h:802>) | `[[nodiscard]] static Result<EnemyDefLibrary> Parse(StringView _json, String* _pMessage = nullptr);` |
| [L803](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/EnemyDef.h:803>) | `[[nodiscard]] static Result<EnemyDefLibrary> Load(const FilePath& _path, String* _pMessage = nullptr);` |
| [L804](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/EnemyDef.h:804>) | `[[nodiscard]] static Result<EnemyDefLibrary> LoadDirectory(const FilePath& _dir, String* _pMessage = nullptr);` |
| [L805](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/EnemyDef.h:805>) | `[[nodiscard]] static FilePath                GetDefaultDirectory(const FilePath& _assetRoot);` |
| [L806](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/EnemyDef.h:806>) | `[[nodiscard]] const EnemyDef*                Find(StringView _name) const;` |
| [L807](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/EnemyDef.h:807>) | `[[nodiscard]] EnemyDef*                      FindMutable(StringView _name);` |
| [L808](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/EnemyDef.h:808>) | `[[nodiscard]] int                            FindIndex(StringView _name) const;` |
| [L809](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/EnemyDef.h:809>) | `[[nodiscard]] const EnemyDef&                Get(int _index) const;` |
| [L810](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/EnemyDef.h:810>) | `[[nodiscard]] const ProjectileDef*           FindProjectile(StringView _name) const;` |
| [L811](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/EnemyDef.h:811>) | `[[nodiscard]] int                            FindProjectileIndex(StringView _name) const;` |
| [L812](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/EnemyDef.h:812>) | `[[nodiscard]] const ProjectileDef&           GetProjectile(int _index) const;` |
| [L813](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/EnemyDef.h:813>) | `[[nodiscard]] size_t                         GetDefCount() const;` |
| [L814](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/EnemyDef.h:814>) | `[[nodiscard]] size_t                         GetProjectileCount() const;` |
| [L815](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/EnemyDef.h:815>) | `[[nodiscard]] Span<const EnemyDef>           GetDefs() const;` |
| [L816](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/EnemyDef.h:816>) | `[[nodiscard]] Span<const ProjectileDef>      GetProjectiles() const;` |
| [L817](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/EnemyDef.h:817>) | `[[nodiscard]] const EnemyGlobals&            GetGlobals() const;` |
| [L829](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/EnemyDef.h:829>) | `struct EnemyPartSlot` |
| [L835](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/EnemyDef.h:835>) | `void GetEnemyPartSlots(EnemyDef& _def, Vector<EnemyPartSlot>& _out);` |

## Source/Game/Enemies/EnemyDefLoader.cpp

[EnemyDefLoader.cpp](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/EnemyDefLoader.cpp>) · 1564줄

| 위치 | 선언·함수 |
|---|---|
| [L21](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/EnemyDefLoader.cpp:21>) | `[[nodiscard]] const String& GetError() const noexcept` |
| [L235](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/EnemyDefLoader.cpp:235>) | `void EnemyDefParser::Fail_( const StringView _def, const StringView _field, const StringView _msg)` |
| [L246](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/EnemyDefLoader.cpp:246>) | `bool EnemyDefParser::ReadNum_( const StringView _def, const StringView _field, const JsonReader& _reader, const StringView _key, float& _out, const bool _bRequired)` |
| [L263](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/EnemyDefLoader.cpp:263>) | `bool EnemyDefParser::ReadInt_( const StringView _def, const StringView _field, const JsonReader& _reader, const StringView _key, int& _out, const bool _bRequired)` |
| [L280](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/EnemyDefLoader.cpp:280>) | `bool EnemyDefParser::ReadBool_( const StringView _def, const StringView _field, const JsonReader& _reader, const StringView _key, bool& _out)` |
| [L296](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/EnemyDefLoader.cpp:296>) | `bool EnemyDefParser::ReadStr_( const StringView _def, const StringView _field, const JsonReader& _reader, const StringView _key, String& _out, const bool _bRequired)` |
| [L313](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/EnemyDefLoader.cpp:313>) | `bool EnemyDefParser::ReadVec2_( const StringView _def, const StringView _field, const JsonReader& _reader, const StringView _key, VECTOR2& _out)` |
| [L329](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/EnemyDefLoader.cpp:329>) | `bool EnemyDefParser::CheckObjectKeys_( const StringView _def, const StringView _field, const JsonReader& _object, const Span<const StringView> _allowed)` |
| [L349](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/EnemyDefLoader.cpp:349>) | `bool EnemyDefParser::ReadBox_( const StringView _def, const StringView _field, const JsonReader& _reader, BoxDef& _out)` |
| [L374](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/EnemyDefLoader.cpp:374>) | `bool EnemyDefParser::ReadClip_( const StringView _def, const StringView _field, const JsonReader& _reader, ClipDef& _out)` |
| [L401](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/EnemyDefLoader.cpp:401>) | `bool EnemyDefParser::ReadFx_( const StringView _def, const StringView _field, const JsonReader& _reader, FxDef& _out)` |
| [L419](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/EnemyDefLoader.cpp:419>) | `bool EnemyDefParser::BuildChain_( const StringView _name, const int _depth, Chain& _out)` |
| [L453](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/EnemyDefLoader.cpp:453>) | `bool EnemyDefParser::CheckKeys_( const Chain& _chain, const StringView _defName, const Span<const StringView> _allowed)` |
| [L468](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/EnemyDefLoader.cpp:468>) | `std::optional<JsonReader> EnemyDefParser::Find_( const Chain& _chain, const StringView _key) const` |
| [L486](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/EnemyDefLoader.cpp:486>) | `EnemyDefParser::Entries EnemyDefParser::Merge_( const Chain& _chain, const StringView _key) const` |
| [L518](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/EnemyDefLoader.cpp:518>) | `bool EnemyDefParser::ParseProjectile_( const RawDef& _raw, const Chain& _chain)` |
| [L699](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/EnemyDefLoader.cpp:699>) | `bool EnemyDefParser::ParseWeapon_( const String& _def, const String& _name, const JsonReader& _reader, WeaponDef& _out)` |
| [L738](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/EnemyDefLoader.cpp:738>) | `else if (aim == "given")` |
| [L842](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/EnemyDefLoader.cpp:842>) | `bool EnemyDefParser::ReadDeathClip_( const EnemyDef& _def, const JsonReader& _death, const StringView _key, int& _out)` |
| [L866](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/EnemyDefLoader.cpp:866>) | `bool EnemyDefParser::ParseDeath_( EnemyDef& _def, const JsonReader& _death)` |
| [L923](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/EnemyDefLoader.cpp:923>) | `bool EnemyDefParser::ParseEnemy_( const RawDef& _raw, const Chain& _chain)` |
| [L985](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/EnemyDefLoader.cpp:985>) | `else if (teamName == "neutral")` |
| [L1105](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/EnemyDefLoader.cpp:1105>) | `else if (!mode.empty() && mode != "screen")` |
| [L1215](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/EnemyDefLoader.cpp:1215>) | `else if (artFacing == "right")` |
| [L1219](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/EnemyDefLoader.cpp:1219>) | `else if (!artFacing.empty())` |
| [L1272](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/EnemyDefLoader.cpp:1272>) | `bool EnemyDefParser::Run( const JsonReader& _root)` |
| [L1278](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/EnemyDefLoader.cpp:1278>) | `bool EnemyDefParser::RunMany( const Span<const JsonReader> _roots)` |
| [L1291](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/EnemyDefLoader.cpp:1291>) | `bool EnemyDefParser::AddRoot_( const JsonReader& _root)` |
| [L1345](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/EnemyDefLoader.cpp:1345>) | `bool EnemyDefParser::Finish_()` |
| [L1409](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/EnemyDefLoader.cpp:1409>) | `Result<EnemyDefLibrary> EnemyDefLibrary::Parse( const StringView _json, String* _pMessage)` |
| [L1438](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/EnemyDefLoader.cpp:1438>) | `Result<EnemyDefLibrary> EnemyDefLibrary::Load( const FilePath& _path, String* _pMessage)` |
| [L1467](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/EnemyDefLoader.cpp:1467>) | `Result<EnemyDefLibrary> EnemyDefLibrary::LoadDirectory( const FilePath& _dir, String* _pMessage)` |
| [L1525](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/EnemyDefLoader.cpp:1525>) | `FilePath EnemyDefLibrary::GetDefaultDirectory( const FilePath& _assetRoot)` |
| [L1531](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/EnemyDefLoader.cpp:1531>) | `const EnemyDef* EnemyDefLibrary::Find( const StringView _name) const` |
| [L1538](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/EnemyDefLoader.cpp:1538>) | `EnemyDef* EnemyDefLibrary::FindMutable( const StringView _name)` |
| [L1545](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/EnemyDefLoader.cpp:1545>) | `int EnemyDefLibrary::FindIndex( const StringView _name) const` |
| [L1552](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/EnemyDefLoader.cpp:1552>) | `const ProjectileDef* EnemyDefLibrary::FindProjectile( const StringView _name) const` |
| [L1559](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/EnemyDefLoader.cpp:1559>) | `int EnemyDefLibrary::FindProjectileIndex( const StringView _name) const` |

## Source/Game/Enemies/EnemyDefParse.h

[EnemyDefParse.h](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/EnemyDefParse.h>) · 64줄

| 위치 | 선언·함수 |
|---|---|
| [L6](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/EnemyDefParse.h:6>) | `struct ParamEntry` |
| [L12](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/EnemyDefParse.h:12>) | `ParamEntry(String _key, JsonReader _value);` |
| [L15](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/EnemyDefParse.h:15>) | `class DefReader` |
| [L18](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/EnemyDefParse.h:18>) | `DefReader(StringView _defName, EnemyDef& _def, Vector<ParamEntry>& _params, String& _error);` |
| [L19](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/EnemyDefParse.h:19>) | `[[nodiscard]] float             Num(StringView _key);` |
| [L20](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/EnemyDefParse.h:20>) | `[[nodiscard]] float             Num(StringView _key, float _default);` |
| [L21](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/EnemyDefParse.h:21>) | `[[nodiscard]] int               Int(StringView _key);` |
| [L22](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/EnemyDefParse.h:22>) | `[[nodiscard]] int               Int(StringView _key, int _default);` |
| [L23](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/EnemyDefParse.h:23>) | `[[nodiscard]] int               Ticks(StringView _key);` |
| [L24](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/EnemyDefParse.h:24>) | `[[nodiscard]] int               Ticks(StringView _key, float _defaultSec);` |
| [L25](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/EnemyDefParse.h:25>) | `[[nodiscard]] bool              Bool(StringView _key, bool _bDefault);` |
| [L26](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/EnemyDefParse.h:26>) | `[[nodiscard]] String            Str(StringView _key);` |
| [L27](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/EnemyDefParse.h:27>) | `[[nodiscard]] VECTOR2           ReadVec2(StringView _key, VECTOR2 _default);` |
| [L28](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/EnemyDefParse.h:28>) | `[[nodiscard]] BoxDef            Box(StringView _key);` |
| [L29](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/EnemyDefParse.h:29>) | `[[nodiscard]] bool              NumArray(StringView _key, Span<float> _out);` |
| [L30](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/EnemyDefParse.h:30>) | `[[nodiscard]] int               NumList(StringView _key, Span<float> _out);` |
| [L31](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/EnemyDefParse.h:31>) | `[[nodiscard]] bool              Has(StringView _key) const;` |
| [L32](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/EnemyDefParse.h:32>) | `[[nodiscard]] const JsonReader* FindRaw(StringView _key);` |
| [L33](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/EnemyDefParse.h:33>) | `[[nodiscard]] int               Clip(StringView _logical);` |
| [L34](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/EnemyDefParse.h:34>) | `[[nodiscard]] int               ClipOpt(StringView _logical) const;` |
| [L35](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/EnemyDefParse.h:35>) | `[[nodiscard]] int               Weapon(StringView _name);` |
| [L36](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/EnemyDefParse.h:36>) | `void                            Fail(StringView _field, StringView _message);` |
| [L37](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/EnemyDefParse.h:37>) | `[[nodiscard]] bool              Finish();` |
| [L40](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/EnemyDefParse.h:40>) | `[[nodiscard]] ParamEntry* Find_(StringView _key);` |
| [L41](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/EnemyDefParse.h:41>) | `void                      ReadBoxField_(const ParamEntry& _entry, StringView _key, StringView _name, float& _out, bool _bRequired);` |
| [L49](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/EnemyDefParse.h:49>) | `void ParseSoldierConfig(DefReader& _reader, EnemyDef& _def);` |
| [L50](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/EnemyDefParse.h:50>) | `void ParseBzkConfig(DefReader& _reader, EnemyDef& _def);` |
| [L51](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/EnemyDefParse.h:51>) | `void ParseHangConfig(DefReader& _reader, EnemyDef& _def);` |
| [L52](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/EnemyDefParse.h:52>) | `void ParseFlyingBzkConfig(DefReader& _reader, EnemyDef& _def);` |
| [L53](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/EnemyDefParse.h:53>) | `void ParseShielderConfig(DefReader& _reader, EnemyDef& _def);` |
| [L54](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/EnemyDefParse.h:54>) | `void ParsePowConfig(DefReader& _reader, EnemyDef& _def);` |
| [L55](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/EnemyDefParse.h:55>) | `void ParseJeepConfig(DefReader& _reader, EnemyDef& _def);` |
| [L56](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/EnemyDefParse.h:56>) | `void ParseBTankConfig(DefReader& _reader, EnemyDef& _def);` |
| [L57](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/EnemyDefParse.h:57>) | `void ParseMissileTankConfig(DefReader& _reader, EnemyDef& _def);` |
| [L58](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/EnemyDefParse.h:58>) | `void ParseSarubiaConfig(DefReader& _reader, EnemyDef& _def);` |
| [L59](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/EnemyDefParse.h:59>) | `void ParseBusConfig(DefReader& _reader, EnemyDef& _def);` |
| [L60](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/EnemyDefParse.h:60>) | `void ParseHeliConfig(DefReader& _reader, EnemyDef& _def);` |
| [L61](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/EnemyDefParse.h:61>) | `void ParseHeliBombConfig(DefReader& _reader, EnemyDef& _def);` |
| [L62](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/EnemyDefParse.h:62>) | `void ParseBigShieeConfig(DefReader& _reader, EnemyDef& _def);` |
| [L64](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/EnemyDefParse.h:64>) | `void ParseEnemyParts(DefReader& _reader, EnemyDef& _def);` |

## Source/Game/Enemies/EnemyTypes.cpp

[EnemyTypes.cpp](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/EnemyTypes.cpp>) · 64줄

| 위치 | 선언·함수 |
|---|---|
| [L3](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/EnemyTypes.cpp:3>) | `uint32_t MakePartOwner( const uint32_t _enemyId, const uint32_t _partIndex)` |
| [L10](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/EnemyTypes.cpp:10>) | `int GetPartIndex( const uint32_t _owner)` |
| [L17](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/EnemyTypes.cpp:17>) | `bool IsEnemyOwner( const uint32_t _owner)` |
| [L24](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/EnemyTypes.cpp:24>) | `bool EnemyHandle::IsNull() const` |
| [L29](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/EnemyTypes.cpp:29>) | `VECTOR2 EnemyState::GetFeet() const` |
| [L34](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/EnemyTypes.cpp:34>) | `bool EnemyState::IsAlive() const` |
| [L39](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/EnemyTypes.cpp:39>) | `float HitFlashAmount( const EnemyState& _e)` |
| [L45](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/EnemyTypes.cpp:45>) | `float PartFlashAmount( const EnemyState& _e, const int _part)` |
| [L56](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/EnemyTypes.cpp:56>) | `void TriggerPartHitFlash( EnemyState& _e, const int _part)` |

## Source/Game/Enemies/EnemyTypes.h

[EnemyTypes.h](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/EnemyTypes.h>) · 178줄

| 위치 | 선언·함수 |
|---|---|
| [L18](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/EnemyTypes.h:18>) | `[[nodiscard]] uint32_t MakePartOwner(uint32_t _enemyId, uint32_t _partIndex);` |
| [L20](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/EnemyTypes.h:20>) | `[[nodiscard]] int GetPartIndex(uint32_t _owner);` |
| [L22](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/EnemyTypes.h:22>) | `[[nodiscard]] bool IsEnemyOwner(uint32_t _owner);` |
| [L24](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/EnemyTypes.h:24>) | `struct EnemyHandle` |
| [L28](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/EnemyTypes.h:28>) | `[[nodiscard]] bool           IsNull() const;` |
| [L29](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/EnemyTypes.h:29>) | `[[nodiscard]] constexpr bool operator==(const EnemyHandle&) const = default;` |
| [L32](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/EnemyTypes.h:32>) | `struct SpawnParams` |
| [L47](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/EnemyTypes.h:47>) | `enum class eEnemyEventKind` |
| [L57](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/EnemyTypes.h:57>) | `struct EnemyEvent` |
| [L69](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/EnemyTypes.h:69>) | `struct EnemyPlayerInfo` |
| [L78](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/EnemyTypes.h:78>) | `struct EnemyContext` |
| [L89](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/EnemyTypes.h:89>) | `enum class eEnemyLife` |
| [L97](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/EnemyTypes.h:97>) | `enum class eDeathStart` |
| [L104](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/EnemyTypes.h:104>) | `struct EnemyState` |
| [L106](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/EnemyTypes.h:106>) | `[[nodiscard]] VECTOR2 GetFeet() const;` |
| [L107](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/EnemyTypes.h:107>) | `[[nodiscard]] bool    IsAlive() const;` |
| [L167](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/EnemyTypes.h:167>) | `[[nodiscard]] float HitFlashAmount(const EnemyState& _e);` |
| [L169](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/EnemyTypes.h:169>) | `[[nodiscard]] float PartFlashAmount(const EnemyState& _e, int _part);` |
| [L171](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/EnemyTypes.h:171>) | `void TriggerPartHitFlash(EnemyState& _e, int _part);` |
| [L173](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/EnemyTypes.h:173>) | `struct EnemyFilter` |

## Source/Game/Enemies/EnemyWorld.cpp

[EnemyWorld.cpp](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/EnemyWorld.cpp>) · 1179줄

| 위치 | 선언·함수 |
|---|---|
| [L16](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/EnemyWorld.cpp:16>) | `[[nodiscard]] int FrameAt_( const AnimClip& _clip, const int64_t _timeUnits)` |
| [L38](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/EnemyWorld.cpp:38>) | `[[nodiscard]] float RotationDeg_( const VECTOR2 _velocity)` |
| [L44](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/EnemyWorld.cpp:44>) | `[[nodiscard]] PhysicsTuning MakePhys_( const EnemyDef& _def)` |
| [L53](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/EnemyWorld.cpp:53>) | `void MixHash_( Fnv1a& _hasher, const int64_t _value)` |
| [L60](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/EnemyWorld.cpp:60>) | `[[nodiscard]] int SideIndex_( const int _facing)` |
| [L76](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/EnemyWorld.cpp:76>) | `void EnemyWorld::SetLibrary( const EnemyDefLibrary* _pLibrary)` |
| [L87](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/EnemyWorld.cpp:87>) | `void EnemyWorld::Reset( const uint64_t _seed)` |
| [L102](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/EnemyWorld.cpp:102>) | `int GetEnemyDefaultZ( const eEnemyBehavior _behavior)` |
| [L108](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/EnemyWorld.cpp:108>) | `EnemyHandle EnemyWorld::Spawn( const StringView _defName, const VECTOR2 _feet, const SpawnParams& _params, const PhysicsWorld* _pSnapWorld)` |
| [L117](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/EnemyWorld.cpp:117>) | `EnemyHandle EnemyWorld::SpawnByIndex( const int _defIndex, const VECTOR2 _feet, const SpawnParams& _params, const PhysicsWorld* _pSnapWorld)` |
| [L133](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/EnemyWorld.cpp:133>) | `EnemyHandle EnemyWorld::SpawnDeferred( const int _defIndex, const VECTOR2 _feet, const SpawnParams& _params)` |
| [L148](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/EnemyWorld.cpp:148>) | `void EnemyWorld::Create_( const uint32_t _id, const int _defIndex, const VECTOR2 _feet, const SpawnParams& _params, const PhysicsWorld* _pSnapWorld)` |
| [L190](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/EnemyWorld.cpp:190>) | `void EnemyWorld::RegisterHurtboxes( CombatSystem& _combat) const` |
| [L219](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/EnemyWorld.cpp:219>) | `void EnemyWorld::OnHitEvents( const Span<const HitEvent> _events)` |
| [L263](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/EnemyWorld.cpp:263>) | `else if (applied > 0 && def.bHitFlash)` |
| [L270](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/EnemyWorld.cpp:270>) | `void EnemyWorld::BeginDying_( Enemy& _enemy, const EnemyDef& _def, EnemyEnv& _env)` |
| [L284](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/EnemyWorld.cpp:284>) | `void ReleaseExtraBoxHit_( EnemyState& _e, EnemyEnv& _env)` |
| [L295](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/EnemyWorld.cpp:295>) | `void TickExtraBoxes_( EnemyState& _e, const EnemyDef& _def, EnemyEnv& _env)` |
| [L346](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/EnemyWorld.cpp:346>) | `void EnemyWorld::Tick( const EnemyContext& _ctx)` |
| [L464](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/EnemyWorld.cpp:464>) | `else if (e.life == eEnemyLife::Dying)` |
| [L486](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/EnemyWorld.cpp:486>) | `void EnemyWorld::SpawnShotTrails_( const ProjectileSystem& _projectiles, const AnimationLibrary& _anims)` |
| [L516](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/EnemyWorld.cpp:516>) | `void EnemyWorld::RemoveGone_()` |
| [L540](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/EnemyWorld.cpp:540>) | `void EnemyWorld::FlushFx_( const AnimationLibrary* _pAnims)` |
| [L574](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/EnemyWorld.cpp:574>) | `void EnemyWorld::BindAnims( const AnimationLibrary& _anims)` |
| [L613](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/EnemyWorld.cpp:613>) | `void EnemyWorld::Submit( SpriteBatch& _sprites, const AnimationLibrary& _anims, const ProjectileSystem& _projectiles, const float _alpha)` |
| [L712](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/EnemyWorld.cpp:712>) | `else if (bBegin)` |
| [L716](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/EnemyWorld.cpp:716>) | `else if (bRoll && shot.bLanded)` |
| [L738](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/EnemyWorld.cpp:738>) | `size_t EnemyWorld::FindIndex_( const uint32_t _id) const` |
| [L751](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/EnemyWorld.cpp:751>) | `bool EnemyWorld::FireWeaponNow( const EnemyHandle _handle, const int _weapon)` |
| [L771](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/EnemyWorld.cpp:771>) | `bool EnemyWorld::PlayMotion( const EnemyHandle _handle, const int _weapon, const int _fireFrame)` |
| [L803](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/EnemyWorld.cpp:803>) | `const EnemyState* EnemyWorld::Get( const EnemyHandle _handle) const` |
| [L810](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/EnemyWorld.cpp:810>) | `bool EnemyWorld::SetZ( const EnemyHandle _handle, const int _z)` |
| [L823](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/EnemyWorld.cpp:823>) | `EnemyHandle EnemyWorld::ResolveHandle( const EnemyHandle _handle) const` |
| [L847](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/EnemyWorld.cpp:847>) | `Vector<Aabb> EnemyWorld::GetSolidBoxes() const` |
| [L878](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/EnemyWorld.cpp:878>) | `bool EnemyWorld::IsDefeated( const EnemyHandle _handle) const` |
| [L897](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/EnemyWorld.cpp:897>) | `float EnemyWorld::TakeCameraScroll()` |
| [L904](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/EnemyWorld.cpp:904>) | `bool EnemyWorld::IsDestroyFinished( const EnemyHandle _handle) const` |
| [L924](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/EnemyWorld.cpp:924>) | `bool EnemyWorld::IsTagDefeated( const uint32_t _userTag) const` |
| [L939](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/EnemyWorld.cpp:939>) | `int EnemyWorld::CountAlive( const EnemyFilter& _filter) const` |
| [L968](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/EnemyWorld.cpp:968>) | `int EnemyWorld::CountAlive( const StringView _defName) const` |
| [L981](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/EnemyWorld.cpp:981>) | `Vector<String> EnemyWorld::TakeSounds()` |
| [L988](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/EnemyWorld.cpp:988>) | `Vector<EnemyEvent> EnemyWorld::TakeEvents()` |
| [L995](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/EnemyWorld.cpp:995>) | `void EnemyWorld::RefreshBodies( const int _defIndex, const VECTOR2 _size)` |
| [L1010](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/EnemyWorld.cpp:1010>) | `void EnemyWorld::ClearMonsters( const bool _bIncludeNpc)` |
| [L1033](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/EnemyWorld.cpp:1033>) | `void EnemyWorld::KillAll( const bool _bAwardScore)` |
| [L1061](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/EnemyWorld.cpp:1061>) | `uint64_t EnemyWorld::HashState() const` |
| [L1086](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/EnemyWorld.cpp:1086>) | `int EnemyWorld::Roll( const int _n)` |
| [L1096](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/EnemyWorld.cpp:1096>) | `void EnemyWorld::PushEvent( EnemyEvent _event)` |
| [L1102](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/EnemyWorld.cpp:1102>) | `void EnemyWorld::RequestFx( EnemyFxRequest _request)` |
| [L1108](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/EnemyWorld.cpp:1108>) | `void EnemyWorld::NoteSuccessor( const EnemyHandle _old, const EnemyHandle _new)` |
| [L1118](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/EnemyWorld.cpp:1118>) | `EnemyEvent EnemyWorld::MakeEvent( const EnemyState& _e, const eEnemyEventKind _kind) const` |
| [L1137](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/EnemyWorld.cpp:1137>) | `const EnemyDefLibrary* EnemyWorld::GetLibrary() const` |
| [L1142](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/EnemyWorld.cpp:1142>) | `void EnemyWorld::ReseedRng( const uint64_t _seed)` |
| [L1148](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/EnemyWorld.cpp:1148>) | `void EnemyWorld::SetSpriteLibrary( const SpriteLibrary* _pSprites)` |
| [L1154](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/EnemyWorld.cpp:1154>) | `void EnemyWorld::RequestCameraScroll( const float _dx)` |
| [L1160](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/EnemyWorld.cpp:1160>) | `Span<const Scoped<Enemy>> EnemyWorld::GetEnemies() const` |
| [L1165](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/EnemyWorld.cpp:1165>) | `const EffectList& EnemyWorld::GetEffects() const` |
| [L1170](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/EnemyWorld.cpp:1170>) | `size_t EnemyWorld::GetCount() const` |
| [L1175](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/EnemyWorld.cpp:1175>) | `const Enemy* EnemyWorld::FindEnemy(const EnemyHandle _handle) const` |

## Source/Game/Enemies/EnemyWorld.h

[EnemyWorld.h](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/EnemyWorld.h>) · 126줄

| 위치 | 선언·함수 |
|---|---|
| [L17](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/EnemyWorld.h:17>) | `[[nodiscard]] int GetEnemyDefaultZ(eEnemyBehavior _behavior);` |
| [L19](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/EnemyWorld.h:19>) | `class EnemyWorld` |
| [L24](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/EnemyWorld.h:24>) | `explicit EnemyWorld(const EnemyDefLibrary* _pLibrary, uint64_t _seed = 1);` |
| [L25](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/EnemyWorld.h:25>) | `void                                 SetLibrary(const EnemyDefLibrary* _pLibrary);` |
| [L26](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/EnemyWorld.h:26>) | `[[nodiscard]] const EnemyDefLibrary* GetLibrary() const;` |
| [L27](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/EnemyWorld.h:27>) | `void                                 Reset(uint64_t _seed = 1);` |
| [L28](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/EnemyWorld.h:28>) | `void                                 ReseedRng(uint64_t _seed);` |
| [L29](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/EnemyWorld.h:29>) | `EnemyHandle                          Spawn(StringView _defName, VECTOR2 _feet, const SpawnParams& _params = {}, const PhysicsWorld* _pSnapWorld = nullptr);` |
| [L30](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/EnemyWorld.h:30>) | `EnemyHandle                          SpawnByIndex(int _defIndex, VECTOR2 _feet, const SpawnParams& _params = {}, const PhysicsWorld* _pSnapWorld = nullptr);` |
| [L31](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/EnemyWorld.h:31>) | `void                                 RegisterHurtboxes(CombatSystem& _combat) const;` |
| [L32](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/EnemyWorld.h:32>) | `void                                 Tick(const EnemyContext& _ctx);` |
| [L33](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/EnemyWorld.h:33>) | `void                                 OnHitEvents(Span<const HitEvent> _events);` |
| [L34](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/EnemyWorld.h:34>) | `void                                 BindAnims(const AnimationLibrary& _anims);` |
| [L35](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/EnemyWorld.h:35>) | `void                                 Submit(SpriteBatch& _sprites, const AnimationLibrary& _anims, const ProjectileSystem& _projectiles, float _alpha);` |
| [L36](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/EnemyWorld.h:36>) | `void                                 SetSpriteLibrary(const SpriteLibrary* _pSprites);` |
| [L37](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/EnemyWorld.h:37>) | `[[nodiscard]] const EnemyState*      Get(EnemyHandle _handle) const;` |
| [L38](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/EnemyWorld.h:38>) | `[[nodiscard]] const Enemy*           FindEnemy(EnemyHandle _handle) const;` |
| [L39](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/EnemyWorld.h:39>) | `bool                                 SetZ(EnemyHandle _handle, int _z);` |
| [L40](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/EnemyWorld.h:40>) | `[[nodiscard]] bool                   IsDefeated(EnemyHandle _handle) const;` |
| [L41](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/EnemyWorld.h:41>) | `[[nodiscard]] bool                   IsDestroyFinished(EnemyHandle _handle) const;` |
| [L42](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/EnemyWorld.h:42>) | `[[nodiscard]] bool                   IsTagDefeated(uint32_t _userTag) const;` |
| [L43](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/EnemyWorld.h:43>) | `[[nodiscard]] EnemyHandle            ResolveHandle(EnemyHandle _handle) const;` |
| [L44](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/EnemyWorld.h:44>) | `[[nodiscard]] int                    CountAlive(const EnemyFilter& _filter = {}) const;` |
| [L45](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/EnemyWorld.h:45>) | `[[nodiscard]] int                    CountAlive(StringView _defName) const;` |
| [L48](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/EnemyWorld.h:48>) | `void  RequestCameraScroll(float _dx);` |
| [L49](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/EnemyWorld.h:49>) | `float TakeCameraScroll();` |
| [L52](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/EnemyWorld.h:52>) | `[[nodiscard]] Vector<Aabb>              GetSolidBoxes() const;` |
| [L53](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/EnemyWorld.h:53>) | `bool                                    FireWeaponNow(EnemyHandle _handle, int _weapon);` |
| [L54](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/EnemyWorld.h:54>) | `bool                                    PlayMotion(EnemyHandle _handle, int _weapon, int _fireFrame);` |
| [L55](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/EnemyWorld.h:55>) | `[[nodiscard]] Span<const Scoped<Enemy>> GetEnemies() const;` |
| [L56](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/EnemyWorld.h:56>) | `[[nodiscard]] size_t                    GetCount() const;` |
| [L57](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/EnemyWorld.h:57>) | `[[nodiscard]] const EffectList&         GetEffects() const;` |
| [L58](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/EnemyWorld.h:58>) | `[[nodiscard]] Vector<EnemyEvent>        TakeEvents();` |
| [L59](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/EnemyWorld.h:59>) | `[[nodiscard]] Vector<String>            TakeSounds();` |
| [L60](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/EnemyWorld.h:60>) | `void                                    ClearMonsters(bool _bIncludeNpc = false);` |
| [L61](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/EnemyWorld.h:61>) | `void                                    RefreshBodies(int _defIndex, VECTOR2 _size);` |
| [L62](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/EnemyWorld.h:62>) | `void                                    KillAll(bool _bAwardScore = false);` |
| [L63](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/EnemyWorld.h:63>) | `[[nodiscard]] uint64_t                  HashState() const;` |
| [L64](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/EnemyWorld.h:64>) | `[[nodiscard]] int                       Roll(int _n);` |
| [L65](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/EnemyWorld.h:65>) | `void                                    PushEvent(EnemyEvent _event);` |
| [L66](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/EnemyWorld.h:66>) | `void                                    RequestFx(EnemyFxRequest _request);` |
| [L67](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/EnemyWorld.h:67>) | `EnemyHandle                             SpawnDeferred(int _defIndex, VECTOR2 _feet, const SpawnParams& _params);` |
| [L68](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/EnemyWorld.h:68>) | `void                                    NoteSuccessor(EnemyHandle _old, EnemyHandle _new);` |
| [L69](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/EnemyWorld.h:69>) | `[[nodiscard]] EnemyEvent                MakeEvent(const EnemyState& _e, eEnemyEventKind _kind) const;` |
| [L72](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/EnemyWorld.h:72>) | `struct MotionPlay` |
| [L82](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/EnemyWorld.h:82>) | `struct Successor` |
| [L88](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/EnemyWorld.h:88>) | `struct DeferredSpawn` |
| [L96](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/EnemyWorld.h:96>) | `[[nodiscard]] size_t FindIndex_(uint32_t _id) const;` |
| [L97](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/EnemyWorld.h:97>) | `void                 Create_(uint32_t _id, int _defIndex, VECTOR2 _feet, const SpawnParams& _params, const PhysicsWorld* _pSnapWorld);` |
| [L98](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/EnemyWorld.h:98>) | `void                 BeginDying_(Enemy& _enemy, const EnemyDef& _def, EnemyEnv& _env);` |
| [L99](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/EnemyWorld.h:99>) | `void                 RemoveGone_();` |
| [L100](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/EnemyWorld.h:100>) | `void                 FlushFx_(const AnimationLibrary* _pAnims);` |
| [L101](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/EnemyWorld.h:101>) | `void                 SpawnShotTrails_(const ProjectileSystem& _projectiles, const AnimationLibrary& _anims);` |

## Source/Game/Enemies/FlyingBzk.cpp

[FlyingBzk.cpp](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/FlyingBzk.cpp>) · 229줄

| 위치 | 선언·함수 |
|---|---|
| [L9](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/FlyingBzk.cpp:9>) | `void FlyingBzkEnemy::Init( const EnemyDef& _def, EnemyEnv&)` |
| [L20](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/FlyingBzk.cpp:20>) | `void FlyingBzkEnemy::Tick( const EnemyDef& _def, EnemyEnv& _env)` |
| [L117](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/FlyingBzk.cpp:117>) | `void FlyingBzkEnemy::OnDeath( const EnemyDef& _def, EnemyEnv& _env)` |
| [L126](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/FlyingBzk.cpp:126>) | `void FlyingBzkEnemy::TickDying( const EnemyDef& _def, EnemyEnv& _env)` |
| [L133](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/FlyingBzk.cpp:133>) | `void FlyingBzkEnemy::RegisterHurtboxes( const EnemyDef& _def, CombatSystem& _combat) const` |
| [L150](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/FlyingBzk.cpp:150>) | `void ParseFlyingBzkConfig( DefReader& _r, EnemyDef& _def)` |
| [L190](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/FlyingBzk.cpp:190>) | `[[nodiscard]] bool FlyingBzkEnemy::IsOnScreen_( const EnemyEnv& _env) const` |
| [L198](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/FlyingBzk.cpp:198>) | `void FlyingBzkEnemy::UpdatePosture_( const FlyingBzkConfig& _config, const EnemyEnv& _env)` |
| [L216](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/FlyingBzk.cpp:216>) | `void FlyingBzkEnemy::Go_( const EnemyDef& _def, const eFlyingState _state)` |
| [L226](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/FlyingBzk.cpp:226>) | `void FlyingBzkEnemy::HashActionState(Fnv1a& _hash) const` |

## Source/Game/Enemies/FlyingBzkEnemy.h

[FlyingBzkEnemy.h](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/FlyingBzkEnemy.h>) · 36줄

| 위치 | 선언·함수 |
|---|---|
| [L4](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/FlyingBzkEnemy.h:4>) | `enum class eFlyingState` |
| [L10](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/FlyingBzkEnemy.h:10>) | `class FlyingBzkEnemy : public Enemy` |
| [L16](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/FlyingBzkEnemy.h:16>) | `FlyingBzkEnemy() = default;` |
| [L17](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/FlyingBzkEnemy.h:17>) | `void Init(const EnemyDef& _def, EnemyEnv& _env) override;` |
| [L18](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/FlyingBzkEnemy.h:18>) | `void Tick(const EnemyDef& _def, EnemyEnv& _env) override;` |
| [L19](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/FlyingBzkEnemy.h:19>) | `void OnDeath(const EnemyDef& _def, EnemyEnv& _env) override;` |
| [L20](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/FlyingBzkEnemy.h:20>) | `void TickDying(const EnemyDef& _def, EnemyEnv& _env) override;` |
| [L21](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/FlyingBzkEnemy.h:21>) | `void RegisterHurtboxes(const EnemyDef& _def, CombatSystem& _combat) const override;` |
| [L24](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/FlyingBzkEnemy.h:24>) | `void               HashActionState(Fnv1a& _hash) const override;` |
| [L25](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/FlyingBzkEnemy.h:25>) | `[[nodiscard]] bool IsOnScreen_(const EnemyEnv& _env) const;` |
| [L26](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/FlyingBzkEnemy.h:26>) | `void               UpdatePosture_(const FlyingBzkConfig& _config, const EnemyEnv& _env);` |
| [L27](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/FlyingBzkEnemy.h:27>) | `void               Go_(const EnemyDef& _def, eFlyingState _state);` |

## Source/Game/Enemies/Hang.cpp

[Hang.cpp](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/Hang.cpp>) · 105줄

| 위치 | 선언·함수 |
|---|---|
| [L8](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/Hang.cpp:8>) | `void HangEnemy::Init( const EnemyDef& _def, EnemyEnv& _env)` |
| [L18](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/Hang.cpp:18>) | `void HangEnemy::Tick( const EnemyDef& _def, EnemyEnv& _env)` |
| [L51](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/Hang.cpp:51>) | `void HangEnemy::OnDeath( const EnemyDef& _def, EnemyEnv& _env)` |
| [L60](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/Hang.cpp:60>) | `void HangEnemy::TickDying( const EnemyDef& _def, EnemyEnv& _env)` |
| [L67](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/Hang.cpp:67>) | `void ParseHangConfig( DefReader& _r, EnemyDef& _def)` |
| [L81](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/Hang.cpp:81>) | `void HangEnemy::Go_( const EnemyDef& _def, EnemyEnv& _env, const eHangState _state)` |
| [L102](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/Hang.cpp:102>) | `void HangEnemy::HashActionState(Fnv1a& _hash) const` |

## Source/Game/Enemies/HangEnemy.h

[HangEnemy.h](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/HangEnemy.h>) · 26줄

| 위치 | 선언·함수 |
|---|---|
| [L4](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/HangEnemy.h:4>) | `enum class eHangState` |
| [L10](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/HangEnemy.h:10>) | `class HangEnemy : public Enemy` |
| [L15](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/HangEnemy.h:15>) | `HangEnemy() = default;` |
| [L16](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/HangEnemy.h:16>) | `void Init(const EnemyDef& _def, EnemyEnv& _env) override;` |
| [L17](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/HangEnemy.h:17>) | `void Tick(const EnemyDef& _def, EnemyEnv& _env) override;` |
| [L18](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/HangEnemy.h:18>) | `void OnDeath(const EnemyDef& _def, EnemyEnv& _env) override;` |
| [L19](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/HangEnemy.h:19>) | `void TickDying(const EnemyDef& _def, EnemyEnv& _env) override;` |
| [L22](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/HangEnemy.h:22>) | `void HashActionState(Fnv1a& _hash) const override;` |
| [L23](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/HangEnemy.h:23>) | `void Go_(const EnemyDef& _def, EnemyEnv& _env, eHangState _state);` |

## Source/Game/Enemies/Heli.cpp

[Heli.cpp](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/Heli.cpp>) · 530줄

| 위치 | 선언·함수 |
|---|---|
| [L17](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/Heli.cpp:17>) | `[[nodiscard]] int Pack_( const float _value)` |
| [L23](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/Heli.cpp:23>) | `[[nodiscard]] float Unpack_( const int _value)` |
| [L30](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/Heli.cpp:30>) | `[[nodiscard]] float SampleCurve_( const ARRAY<float, 13>& _curve, const float _u)` |
| [L40](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/Heli.cpp:40>) | `[[nodiscard]] float Sign_( const int _facing)` |
| [L46](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/Heli.cpp:46>) | `[[nodiscard]] float HoverY_( const HeliConfig& _config, const EnemyEnv& _env)` |
| [L53](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/Heli.cpp:53>) | `[[nodiscard]] float FollowTargetX_( const HeliConfig& _config, const EnemyEnv& _env)` |
| [L60](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/Heli.cpp:60>) | `[[nodiscard]] int BurstPeriod_( const HeliConfig& _config)` |
| [L67](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/Heli.cpp:67>) | `[[nodiscard]] int BurstPhaseEnd_( const HeliConfig& _config, const int _first, const int _bursts)` |
| [L75](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/Heli.cpp:75>) | `[[nodiscard]] bool IsBombTick_( const HeliConfig& _config, const int _ticks, const int _first)` |
| [L93](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/Heli.cpp:93>) | `void HeliEnemy::Init( const EnemyDef& _def, EnemyEnv& _env)` |
| [L109](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/Heli.cpp:109>) | `void HeliEnemy::Tick( const EnemyDef& _def, EnemyEnv& _env)` |
| [L135](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/Heli.cpp:135>) | `else if (WantsTurn_(config, _env))` |
| [L164](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/Heli.cpp:164>) | `else if (bFlipped && ClipFinished(_e, _def))` |
| [L220](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/Heli.cpp:220>) | `void HeliEnemy::OnDeath( const EnemyDef& _def, EnemyEnv& _env)` |
| [L230](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/Heli.cpp:230>) | `void HeliEnemy::TickDying( const EnemyDef& _def, EnemyEnv& _env)` |
| [L237](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/Heli.cpp:237>) | `void HeliEnemy::SubmitParts( const EnemyDef& _def, const EnemyRenderEnv& _render) const` |
| [L262](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/Heli.cpp:262>) | `const AnimFrame* HeliEnemy::BodyFrame( const EnemyDef& _def, const AnimationLibrary& _anims) const` |
| [L281](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/Heli.cpp:281>) | `void ParseHeliConfig( DefReader& _r, EnemyDef& _def)` |
| [L340](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/Heli.cpp:340>) | `void HeliEnemy::SetPodTicks_( const int _ticks)` |
| [L346](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/Heli.cpp:346>) | `void HeliEnemy::Go_( const EnemyDef& _def, const eHeliState _state)` |
| [L366](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/Heli.cpp:366>) | `void HeliEnemy::BeginMove_( const EnemyDef& _def, const EnemyEnv& _env, const eHeliState _state, const VECTOR2 _target)` |
| [L381](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/Heli.cpp:381>) | `[[nodiscard]] bool HeliEnemy::StepMove_( const HeliConfig& _config)` |
| [L392](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/Heli.cpp:392>) | `void HeliEnemy::StepFollow_( const HeliConfig& _config, const EnemyEnv& _env)` |
| [L410](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/Heli.cpp:410>) | `void HeliEnemy::DropBomb_( const EnemyDef& _def, EnemyEnv& _env)` |
| [L428](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/Heli.cpp:428>) | `[[nodiscard]] int HeliEnemy::FirstBombTicks_( const HeliConfig& _config) const` |
| [L434](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/Heli.cpp:434>) | `void HeliEnemy::StepBombing_( const EnemyDef& _def, EnemyEnv& _env, const int _first, const int _bursts)` |
| [L453](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/Heli.cpp:453>) | `[[nodiscard]] float HeliEnemy::LowTargetX_( const HeliConfig& _config, const EnemyEnv& _env) const` |
| [L461](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/Heli.cpp:461>) | `void HeliEnemy::StepLow_( const HeliConfig& _config, const EnemyEnv& _env)` |
| [L476](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/Heli.cpp:476>) | `void HeliEnemy::BeginLow_( const EnemyDef& _def, const EnemyEnv& _env)` |
| [L486](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/Heli.cpp:486>) | `[[nodiscard]] bool HeliEnemy::WantsTurn_( const HeliConfig& _config, const EnemyEnv& _env)` |
| [L503](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/Heli.cpp:503>) | `[[nodiscard]] int HeliEnemy::ShownPitchFrame_( const EnemyDef& _def, const AnimationLibrary& _anims) const` |
| [L527](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/Heli.cpp:527>) | `void HeliEnemy::HashActionState(Fnv1a& _hash) const` |

## Source/Game/Enemies/HeliBomb.cpp

[HeliBomb.cpp](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/HeliBomb.cpp>) · 152줄

| 위치 | 선언·함수 |
|---|---|
| [L11](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/HeliBomb.cpp:11>) | `[[nodiscard]] bool Touches_( const PhysicsWorld& _world, const VECTOR2 _to)` |
| [L31](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/HeliBomb.cpp:31>) | `void HeliBombEnemy::Init( const EnemyDef& _def, EnemyEnv& _env)` |
| [L43](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/HeliBomb.cpp:43>) | `void HeliBombEnemy::Tick( const EnemyDef& _def, EnemyEnv& _env)` |
| [L77](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/HeliBomb.cpp:77>) | `void HeliBombEnemy::OnDeath( const EnemyDef& _def, EnemyEnv& _env)` |
| [L87](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/HeliBomb.cpp:87>) | `void HeliBombEnemy::TickDying( const EnemyDef& _def, EnemyEnv& _env)` |
| [L107](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/HeliBomb.cpp:107>) | `void ParseHeliBombConfig( DefReader& _r, EnemyDef& _def)` |
| [L127](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/HeliBomb.cpp:127>) | `void HeliBombEnemy::AddBombHitbox_( const VECTOR2 _size, const int _damage, const eHitKind _kind, EnemyEnv& _env) const` |
| [L149](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/HeliBomb.cpp:149>) | `void HeliBombEnemy::HashActionState(Fnv1a& _hash) const` |

## Source/Game/Enemies/HeliBombEnemy.h

[HeliBombEnemy.h](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/HeliBombEnemy.h>) · 20줄

| 위치 | 선언·함수 |
|---|---|
| [L4](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/HeliBombEnemy.h:4>) | `class HeliBombEnemy : public Enemy` |
| [L9](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/HeliBombEnemy.h:9>) | `HeliBombEnemy() = default;` |
| [L10](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/HeliBombEnemy.h:10>) | `void Init(const EnemyDef& _def, EnemyEnv& _env) override;` |
| [L11](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/HeliBombEnemy.h:11>) | `void Tick(const EnemyDef& _def, EnemyEnv& _env) override;` |
| [L12](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/HeliBombEnemy.h:12>) | `void OnDeath(const EnemyDef& _def, EnemyEnv& _env) override;` |
| [L13](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/HeliBombEnemy.h:13>) | `void TickDying(const EnemyDef& _def, EnemyEnv& _env) override;` |
| [L16](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/HeliBombEnemy.h:16>) | `void HashActionState(Fnv1a& _hash) const override;` |
| [L17](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/HeliBombEnemy.h:17>) | `void AddBombHitbox_(VECTOR2 _size, int _damage, eHitKind _kind, EnemyEnv& _env) const;` |

## Source/Game/Enemies/HeliEnemy.h

[HeliEnemy.h](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/HeliEnemy.h>) · 61줄

| 위치 | 선언·함수 |
|---|---|
| [L4](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/HeliEnemy.h:4>) | `struct HeliFlight` |
| [L18](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/HeliEnemy.h:18>) | `enum class eHeliState` |
| [L29](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/HeliEnemy.h:29>) | `class HeliEnemy : public Enemy` |
| [L34](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/HeliEnemy.h:34>) | `HeliEnemy() = default;` |
| [L35](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/HeliEnemy.h:35>) | `void                           Init(const EnemyDef& _def, EnemyEnv& _env) override;` |
| [L36](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/HeliEnemy.h:36>) | `void                           Tick(const EnemyDef& _def, EnemyEnv& _env) override;` |
| [L37](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/HeliEnemy.h:37>) | `void                           OnDeath(const EnemyDef& _def, EnemyEnv& _env) override;` |
| [L38](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/HeliEnemy.h:38>) | `void                           TickDying(const EnemyDef& _def, EnemyEnv& _env) override;` |
| [L39](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/HeliEnemy.h:39>) | `void                           SubmitParts(const EnemyDef& _def, const EnemyRenderEnv& _render) const override;` |
| [L40](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/HeliEnemy.h:40>) | `[[nodiscard]] const AnimFrame* BodyFrame(const EnemyDef& _def, const AnimationLibrary& _anims) const override;` |
| [L43](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/HeliEnemy.h:43>) | `void                HashActionState(Fnv1a& _hash) const override;` |
| [L44](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/HeliEnemy.h:44>) | `void                SetPodTicks_(int _ticks);` |
| [L45](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/HeliEnemy.h:45>) | `void                Go_(const EnemyDef& _def, eHeliState _state);` |
| [L46](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/HeliEnemy.h:46>) | `void                BeginMove_(const EnemyDef& _def, const EnemyEnv& _env, eHeliState _state, VECTOR2 _target);` |
| [L47](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/HeliEnemy.h:47>) | `[[nodiscard]] bool  StepMove_(const HeliConfig& _config);` |
| [L48](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/HeliEnemy.h:48>) | `void                StepFollow_(const HeliConfig& _config, const EnemyEnv& _env);` |
| [L49](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/HeliEnemy.h:49>) | `void                DropBomb_(const EnemyDef& _def, EnemyEnv& _env);` |
| [L50](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/HeliEnemy.h:50>) | `[[nodiscard]] int   FirstBombTicks_(const HeliConfig& _config) const;` |
| [L51](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/HeliEnemy.h:51>) | `void                StepBombing_(const EnemyDef& _def, EnemyEnv& _env, int _first, int _bursts);` |
| [L52](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/HeliEnemy.h:52>) | `[[nodiscard]] float LowTargetX_(const HeliConfig& _config, const EnemyEnv& _env) const;` |
| [L53](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/HeliEnemy.h:53>) | `void                StepLow_(const HeliConfig& _config, const EnemyEnv& _env);` |
| [L54](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/HeliEnemy.h:54>) | `void                BeginLow_(const EnemyDef& _def, const EnemyEnv& _env);` |
| [L55](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/HeliEnemy.h:55>) | `[[nodiscard]] bool  WantsTurn_(const HeliConfig& _config, const EnemyEnv& _env);` |
| [L56](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/HeliEnemy.h:56>) | `[[nodiscard]] int   ShownPitchFrame_(const EnemyDef& _def, const AnimationLibrary& _anims) const;` |

## Source/Game/Enemies/JeepEnemy.h

[JeepEnemy.h](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/JeepEnemy.h>) · 43줄

| 위치 | 선언·함수 |
|---|---|
| [L4](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/JeepEnemy.h:4>) | `enum class eJeepState` |
| [L20](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/JeepEnemy.h:20>) | `class JeepEnemy : public Enemy` |
| [L26](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/JeepEnemy.h:26>) | `JeepEnemy() = default;` |
| [L27](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/JeepEnemy.h:27>) | `void Init(const EnemyDef& _def, EnemyEnv& _env) override;` |
| [L28](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/JeepEnemy.h:28>) | `void Tick(const EnemyDef& _def, EnemyEnv& _env) override;` |
| [L29](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/JeepEnemy.h:29>) | `void OnDeath(const EnemyDef& _def, EnemyEnv& _env) override;` |
| [L30](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/JeepEnemy.h:30>) | `void TickDying(const EnemyDef& _def, EnemyEnv& _env) override;` |
| [L33](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/JeepEnemy.h:33>) | `void                HashActionState(Fnv1a& _hash) const override;` |
| [L34](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/JeepEnemy.h:34>) | `[[nodiscard]] int   ClipOf_(const JeepConfig& _config) const;` |
| [L35](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/JeepEnemy.h:35>) | `void                Go_(const EnemyDef& _def, eJeepState _state);` |
| [L36](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/JeepEnemy.h:36>) | `[[nodiscard]] float TargetX_(const JeepConfig& _config, const Aabb& _view) const;` |
| [L37](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/JeepEnemy.h:37>) | `[[nodiscard]] int   ParkSide_(const JeepConfig& _config, const Aabb& _view) const;` |
| [L38](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/JeepEnemy.h:38>) | `[[nodiscard]] bool  PlayerOnFireSide_(const EnemyEnv& _env) const;` |

## Source/Game/Enemies/MissileTankEnemy.h

[MissileTankEnemy.h](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/MissileTankEnemy.h>) · 32줄

| 위치 | 선언·함수 |
|---|---|
| [L4](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/MissileTankEnemy.h:4>) | `enum class eMissileTankState` |
| [L12](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/MissileTankEnemy.h:12>) | `class MissileTankEnemy : public Enemy` |
| [L18](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/MissileTankEnemy.h:18>) | `MissileTankEnemy() = default;` |
| [L19](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/MissileTankEnemy.h:19>) | `void Init(const EnemyDef& _def, EnemyEnv& _env) override;` |
| [L20](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/MissileTankEnemy.h:20>) | `void Tick(const EnemyDef& _def, EnemyEnv& _env) override;` |
| [L21](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/MissileTankEnemy.h:21>) | `void OnDeath(const EnemyDef& _def, EnemyEnv& _env) override;` |
| [L22](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/MissileTankEnemy.h:22>) | `void TickDying(const EnemyDef& _def, EnemyEnv& _env) override;` |
| [L25](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/MissileTankEnemy.h:25>) | `void               HashActionState(Fnv1a& _hash) const override;` |
| [L26](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/MissileTankEnemy.h:26>) | `[[nodiscard]] bool IsUp_() const;` |
| [L27](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/MissileTankEnemy.h:27>) | `[[nodiscard]] int  ClipOf_(const MissileTankConfig& _config) const;` |
| [L28](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/MissileTankEnemy.h:28>) | `void               Go_(const EnemyDef& _def, eMissileTankState _state);` |

## Source/Game/Enemies/Pow.cpp

[Pow.cpp](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/Pow.cpp>) · 313줄

| 위치 | 선언·함수 |
|---|---|
| [L9](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/Pow.cpp:9>) | `void PowEnemy::Init( const EnemyDef& _def, EnemyEnv& _env)` |
| [L18](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/Pow.cpp:18>) | `void PowEnemy::Tick( const EnemyDef& _def, EnemyEnv& _env)` |
| [L65](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/Pow.cpp:65>) | `else if (!FloorAhead(_e, _env, config.edgeProbeDist, config.edgeProbeDrop))` |
| [L70](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/Pow.cpp:70>) | `else if (_e.bOnOneWay && m_patrolTicks >= config.dropMinPatrolTicks && DyToPlayer(_e, _env) >= config.dropDy)` |
| [L74](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/Pow.cpp:74>) | `else if (_env.ctx.player.bAlive && _e.body.GetBox().Overlaps(_env.ctx.player.bodyBox))` |
| [L162](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/Pow.cpp:162>) | `bool PowEnemy::OnHit( const EnemyDef&, EnemyEnv& _env, const HitEvent&)` |
| [L176](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/Pow.cpp:176>) | `void PowEnemy::RegisterHurtboxes( const EnemyDef& _def, CombatSystem& _combat) const` |
| [L192](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/Pow.cpp:192>) | `void ParsePowConfig( DefReader& _r, EnemyDef& _def)` |
| [L236](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/Pow.cpp:236>) | `[[nodiscard]] int PowEnemy::VariantClip_( const PowConfig& _config, const int _slot, const int _base) const` |
| [L254](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/Pow.cpp:254>) | `void PowEnemy::Go_( const EnemyDef& _def, EnemyEnv& _env, const ePowState _state)` |
| [L303](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/Pow.cpp:303>) | `bool PowEnemy::TouchingPlayer_( EnemyEnv& _env)` |
| [L310](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/Pow.cpp:310>) | `void PowEnemy::HashActionState(Fnv1a& _hash) const` |

## Source/Game/Enemies/PowEnemy.h

[PowEnemy.h](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/PowEnemy.h>) · 35줄

| 위치 | 선언·함수 |
|---|---|
| [L4](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/PowEnemy.h:4>) | `enum class ePowState` |
| [L15](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/PowEnemy.h:15>) | `class PowEnemy : public Enemy` |
| [L20](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/PowEnemy.h:20>) | `PowEnemy() = default;` |
| [L21](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/PowEnemy.h:21>) | `void               Init(const EnemyDef& _def, EnemyEnv& _env) override;` |
| [L22](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/PowEnemy.h:22>) | `void               Tick(const EnemyDef& _def, EnemyEnv& _env) override;` |
| [L23](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/PowEnemy.h:23>) | `[[nodiscard]] bool OnHit(const EnemyDef& _def, EnemyEnv& _env, const HitEvent& _event) override;` |
| [L24](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/PowEnemy.h:24>) | `void               RegisterHurtboxes(const EnemyDef& _def, CombatSystem& _combat) const override;` |
| [L27](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/PowEnemy.h:27>) | `void              HashActionState(Fnv1a& _hash) const override;` |
| [L28](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/PowEnemy.h:28>) | `[[nodiscard]] int VariantClip_(const PowConfig& _config, int _slot, int _base) const;` |
| [L29](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/PowEnemy.h:29>) | `void              Go_(const EnemyDef& _def, EnemyEnv& _env, ePowState _state);` |
| [L30](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/PowEnemy.h:30>) | `bool              TouchingPlayer_(EnemyEnv& _env);` |

## Source/Game/Enemies/SarubiaEnemy.h

[SarubiaEnemy.h](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/SarubiaEnemy.h>) · 29줄

| 위치 | 선언·함수 |
|---|---|
| [L4](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/SarubiaEnemy.h:4>) | `enum class eSarubiaState` |
| [L15](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/SarubiaEnemy.h:15>) | `class SarubiaEnemy : public Enemy` |
| [L20](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/SarubiaEnemy.h:20>) | `SarubiaEnemy() = default;` |
| [L21](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/SarubiaEnemy.h:21>) | `void Init(const EnemyDef& _def, EnemyEnv& _env) override;` |
| [L22](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/SarubiaEnemy.h:22>) | `void Tick(const EnemyDef& _def, EnemyEnv& _env) override;` |
| [L23](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/SarubiaEnemy.h:23>) | `void OnDeath(const EnemyDef& _def, EnemyEnv& _env) override;` |
| [L24](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/SarubiaEnemy.h:24>) | `void TickDying(const EnemyDef& _def, EnemyEnv& _env) override;` |
| [L27](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/SarubiaEnemy.h:27>) | `[[nodiscard]] int ClipOf_(const SarubiaConfig& _config) const;` |
| [L28](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/SarubiaEnemy.h:28>) | `void              Go_(const EnemyDef& _def, eSarubiaState _state);` |

## Source/Game/Enemies/Shielder.cpp

[Shielder.cpp](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/Shielder.cpp>) · 447줄

| 위치 | 선언·함수 |
|---|---|
| [L10](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/Shielder.cpp:10>) | `[[nodiscard]] int PickDir_( const int _dir, const int _fwd, const int _up, const int _down)` |
| [L21](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/Shielder.cpp:21>) | `void ShielderEnemy::Init( const EnemyDef& _def, EnemyEnv& _env)` |
| [L30](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/Shielder.cpp:30>) | `void ShielderEnemy::Tick( const EnemyDef& _def, EnemyEnv& _env)` |
| [L76](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/Shielder.cpp:76>) | `else if (dy > config.levelDy)` |
| [L87](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/Shielder.cpp:87>) | `else if (absDx >= config.detect)` |
| [L110](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/Shielder.cpp:110>) | `else if (_e.stateTicks >= config.moveMaxTicks)` |
| [L114](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/Shielder.cpp:114>) | `else if (absDx <= config.detect && Abs(dy) <= config.moveDy)` |
| [L118](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/Shielder.cpp:118>) | `else if (absDx <= config.guard && dy < -config.moveDy)` |
| [L122](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/Shielder.cpp:122>) | `else if (absDx <= config.guard && dy > config.moveDy)` |
| [L172](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/Shielder.cpp:172>) | `else if (_e.shieldDir == 2)` |
| [L244](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/Shielder.cpp:244>) | `bool ShielderEnemy::OnHit( const EnemyDef& _def, EnemyEnv&, const HitEvent& _event)` |
| [L258](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/Shielder.cpp:258>) | `void ShielderEnemy::OnDeath( const EnemyDef& _def, EnemyEnv& _env)` |
| [L267](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/Shielder.cpp:267>) | `void ShielderEnemy::TickDying( const EnemyDef& _def, EnemyEnv& _env)` |
| [L274](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/Shielder.cpp:274>) | `void ShielderEnemy::RegisterHurtboxes( const EnemyDef& _def, CombatSystem& _combat) const` |
| [L298](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/Shielder.cpp:298>) | `void ParseShielderConfig( DefReader& _r, EnemyDef& _def)` |
| [L360](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/Shielder.cpp:360>) | `[[nodiscard]] int ShielderEnemy::StateClip_( const ShielderConfig& _config) const` |
| [L382](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/Shielder.cpp:382>) | `[[nodiscard]] bool ShielderEnemy::StateDone_( const EnemyDef& _def) const` |
| [L389](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/Shielder.cpp:389>) | `void ShielderEnemy::Go_( const EnemyDef& _def, EnemyEnv& _env, const eShielderState _state, const int _dir)` |
| [L444](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/Shielder.cpp:444>) | `void ShielderEnemy::HashActionState(Fnv1a& _hash) const` |

## Source/Game/Enemies/ShielderEnemy.h

[ShielderEnemy.h](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/ShielderEnemy.h>) · 41줄

| 위치 | 선언·함수 |
|---|---|
| [L4](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/ShielderEnemy.h:4>) | `enum class eShielderState` |
| [L20](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/ShielderEnemy.h:20>) | `class ShielderEnemy : public Enemy` |
| [L26](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/ShielderEnemy.h:26>) | `ShielderEnemy() = default;` |
| [L27](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/ShielderEnemy.h:27>) | `void               Init(const EnemyDef& _def, EnemyEnv& _env) override;` |
| [L28](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/ShielderEnemy.h:28>) | `void               Tick(const EnemyDef& _def, EnemyEnv& _env) override;` |
| [L29](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/ShielderEnemy.h:29>) | `[[nodiscard]] bool OnHit(const EnemyDef& _def, EnemyEnv& _env, const HitEvent& _event) override;` |
| [L30](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/ShielderEnemy.h:30>) | `void               OnDeath(const EnemyDef& _def, EnemyEnv& _env) override;` |
| [L31](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/ShielderEnemy.h:31>) | `void               TickDying(const EnemyDef& _def, EnemyEnv& _env) override;` |
| [L32](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/ShielderEnemy.h:32>) | `void               RegisterHurtboxes(const EnemyDef& _def, CombatSystem& _combat) const override;` |
| [L35](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/ShielderEnemy.h:35>) | `void               HashActionState(Fnv1a& _hash) const override;` |
| [L36](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/ShielderEnemy.h:36>) | `[[nodiscard]] int  StateClip_(const ShielderConfig& _config) const;` |
| [L37](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/ShielderEnemy.h:37>) | `[[nodiscard]] bool StateDone_(const EnemyDef& _def) const;` |
| [L38](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/ShielderEnemy.h:38>) | `void               Go_(const EnemyDef& _def, EnemyEnv& _env, eShielderState _state, int _dir = -1);` |

## Source/Game/Enemies/Soldier.cpp

[Soldier.cpp](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/Soldier.cpp>) · 594줄

| 위치 | 선언·함수 |
|---|---|
| [L13](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/Soldier.cpp:13>) | `void SoldierEnemy::Init( const EnemyDef& _def, EnemyEnv& _env)` |
| [L22](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/Soldier.cpp:22>) | `void SoldierEnemy::Tick( const EnemyDef& _def, EnemyEnv& _env)` |
| [L200](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/Soldier.cpp:200>) | `void SoldierEnemy::OnDeath( const EnemyDef& _def, EnemyEnv& _env)` |
| [L208](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/Soldier.cpp:208>) | `void SoldierEnemy::TickDying( const EnemyDef& _def, EnemyEnv& _env)` |
| [L215](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/Soldier.cpp:215>) | `void ParseSoldierConfig( DefReader& _r, EnemyDef& _def)` |
| [L226](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/Soldier.cpp:226>) | `else if (ai == "sword")` |
| [L230](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/Soldier.cpp:230>) | `else if (ai == "missile")` |
| [L286](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/Soldier.cpp:286>) | `[[nodiscard]] float SoldierEnemy::AbsDx_( const EnemyEnv& _env) const` |
| [L293](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/Soldier.cpp:293>) | `void SoldierEnemy::Reposition_( const EnemyDef& _def, EnemyEnv& _env, const eSoldierState _fallback)` |
| [L305](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/Soldier.cpp:305>) | `else if (_e.bGrounded && dy < -config.jumpDy)` |
| [L315](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/Soldier.cpp:315>) | `void SoldierEnemy::Go_( const EnemyDef& _def, EnemyEnv& _env, const eSoldierState _requestedState)` |
| [L400](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/Soldier.cpp:400>) | `void SoldierEnemy::TickClimb_( const EnemyDef& _def, EnemyEnv& _env)` |
| [L415](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/Soldier.cpp:415>) | `void SoldierEnemy::DecideBomb_( const EnemyDef& _def, EnemyEnv& _env)` |
| [L461](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/Soldier.cpp:461>) | `else if (absDx <= config.closeStop)` |
| [L465](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/Soldier.cpp:465>) | `else if (WallAhead(_e, _env, config.wallProbeW, config.wallProbeH, config.wallProbeX, config.wallProbeY))` |
| [L471](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/Soldier.cpp:471>) | `void SoldierEnemy::DecideSword_( const EnemyDef& _def, EnemyEnv& _env)` |
| [L496](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/Soldier.cpp:496>) | `else if (absDx >= config.jumpAttack)` |
| [L532](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/Soldier.cpp:532>) | `else if (absDx <= config.jumpAttack)` |
| [L543](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/Soldier.cpp:543>) | `else if (WallAhead(_e, _env, config.wallProbeW, config.wallProbeH, config.wallProbeX, config.wallProbeY))` |
| [L549](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/Soldier.cpp:549>) | `void SoldierEnemy::DecideMissile_( const EnemyDef& _def, EnemyEnv& _env)` |
| [L585](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/Soldier.cpp:585>) | `else if (WallAhead(_e, _env, config.wallProbeW, config.wallProbeH, config.wallProbeX, config.wallProbeY))` |
| [L591](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/Soldier.cpp:591>) | `void SoldierEnemy::HashActionState(Fnv1a& _hash) const` |

## Source/Game/Enemies/SoldierEnemy.h

[SoldierEnemy.h](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/SoldierEnemy.h>) · 41줄

| 위치 | 선언·함수 |
|---|---|
| [L4](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/SoldierEnemy.h:4>) | `enum class eSoldierState` |
| [L21](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/SoldierEnemy.h:21>) | `class SoldierEnemy : public Enemy` |
| [L26](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/SoldierEnemy.h:26>) | `SoldierEnemy() = default;` |
| [L27](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/SoldierEnemy.h:27>) | `void Init(const EnemyDef& _def, EnemyEnv& _env) override;` |
| [L28](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/SoldierEnemy.h:28>) | `void Tick(const EnemyDef& _def, EnemyEnv& _env) override;` |
| [L29](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/SoldierEnemy.h:29>) | `void OnDeath(const EnemyDef& _def, EnemyEnv& _env) override;` |
| [L30](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/SoldierEnemy.h:30>) | `void TickDying(const EnemyDef& _def, EnemyEnv& _env) override;` |
| [L33](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/SoldierEnemy.h:33>) | `void                HashActionState(Fnv1a& _hash) const override;` |
| [L34](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/SoldierEnemy.h:34>) | `[[nodiscard]] float AbsDx_(const EnemyEnv& _env) const;` |
| [L35](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/SoldierEnemy.h:35>) | `void                Reposition_(const EnemyDef& _def, EnemyEnv& _env, eSoldierState _fallback);` |
| [L36](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/SoldierEnemy.h:36>) | `void                Go_(const EnemyDef& _def, EnemyEnv& _env, eSoldierState _requestedState);` |
| [L37](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/SoldierEnemy.h:37>) | `void                TickClimb_(const EnemyDef& _def, EnemyEnv& _env);` |
| [L38](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/SoldierEnemy.h:38>) | `void                DecideBomb_(const EnemyDef& _def, EnemyEnv& _env);` |
| [L39](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/SoldierEnemy.h:39>) | `void                DecideSword_(const EnemyDef& _def, EnemyEnv& _env);` |
| [L40](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/SoldierEnemy.h:40>) | `void                DecideMissile_(const EnemyDef& _def, EnemyEnv& _env);` |

## Source/Game/Enemies/VehicleBTank.cpp

[VehicleBTank.cpp](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/VehicleBTank.cpp>) · 156줄

| 위치 | 선언·함수 |
|---|---|
| [L10](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/VehicleBTank.cpp:10>) | `void BTankEnemy::Init( const EnemyDef& _def, EnemyEnv& _env)` |
| [L21](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/VehicleBTank.cpp:21>) | `void BTankEnemy::Tick( const EnemyDef& _def, EnemyEnv& _env)` |
| [L86](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/VehicleBTank.cpp:86>) | `void BTankEnemy::OnDeath( const EnemyDef& _def, EnemyEnv& _env)` |
| [L94](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/VehicleBTank.cpp:94>) | `void BTankEnemy::TickDying( const EnemyDef& _def, EnemyEnv& _env)` |
| [L101](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/VehicleBTank.cpp:101>) | `int BTankEnemy::GetShotCounter() const noexcept` |
| [L106](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/VehicleBTank.cpp:106>) | `void ParseBTankConfig( DefReader& _r, EnemyDef& _def)` |
| [L125](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/VehicleBTank.cpp:125>) | `[[nodiscard]] int BTankEnemy::ClipOf_( const BTankConfig& _config) const` |
| [L138](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/VehicleBTank.cpp:138>) | `void BTankEnemy::Go_( const EnemyDef& _def, const eBTankState _state)` |
| [L153](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/VehicleBTank.cpp:153>) | `void BTankEnemy::HashActionState(Fnv1a& _hash) const` |

## Source/Game/Enemies/VehicleCommon.cpp

[VehicleCommon.cpp](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/VehicleCommon.cpp>) · 58줄

| 위치 | 선언·함수 |
|---|---|
| [L5](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/VehicleCommon.cpp:5>) | `void BeginVehicleDeath( EnemyState& _e, const EnemyDef& _def, EnemyEnv& _env, const int _destroyClip)` |
| [L23](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/VehicleCommon.cpp:23>) | `void TickVehicleDeath( EnemyState& _e, const EnemyDef& _def, EnemyEnv& _env)` |
| [L37](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/VehicleCommon.cpp:37>) | `void RequestFxAt( EnemyEnv& _env, const StringView _clip, const VECTOR2 _at, const int _facing, const int _depth, const float _delaySec)` |
| [L53](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/VehicleCommon.cpp:53>) | `bool IsClipDone( const EnemyState& _e, const EnemyDef& _def)` |

## Source/Game/Enemies/VehicleCommon.h

[VehicleCommon.h](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/VehicleCommon.h>) · 11줄

| 위치 | 선언·함수 |
|---|---|
| [L5](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/VehicleCommon.h:5>) | `void BeginVehicleDeath(EnemyState& _e, const EnemyDef& _def, EnemyEnv& _env, int _destroyClip);` |
| [L7](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/VehicleCommon.h:7>) | `void TickVehicleDeath(EnemyState& _e, const EnemyDef& _def, EnemyEnv& _env);` |
| [L9](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/VehicleCommon.h:9>) | `void RequestFxAt(EnemyEnv& _env, StringView _clip, VECTOR2 _at, int _facing, int _depth = 60, float _delaySec = 0.f);` |
| [L11](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/VehicleCommon.h:11>) | `[[nodiscard]] bool IsClipDone(const EnemyState& _e, const EnemyDef& _def);` |

## Source/Game/Enemies/VehicleJeep.cpp

[VehicleJeep.cpp](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/VehicleJeep.cpp>) · 264줄

| 위치 | 선언·함수 |
|---|---|
| [L7](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/VehicleJeep.cpp:7>) | `void JeepEnemy::Init( const EnemyDef& _def, EnemyEnv& _env)` |
| [L17](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/VehicleJeep.cpp:17>) | `void JeepEnemy::Tick( const EnemyDef& _def, EnemyEnv& _env)` |
| [L162](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/VehicleJeep.cpp:162>) | `void JeepEnemy::OnDeath( const EnemyDef& _def, EnemyEnv& _env)` |
| [L170](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/VehicleJeep.cpp:170>) | `void JeepEnemy::TickDying( const EnemyDef& _def, EnemyEnv& _env)` |
| [L177](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/VehicleJeep.cpp:177>) | `void ParseJeepConfig( DefReader& _r, EnemyDef& _def)` |
| [L206](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/VehicleJeep.cpp:206>) | `[[nodiscard]] int JeepEnemy::ClipOf_( const JeepConfig& _config) const` |
| [L225](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/VehicleJeep.cpp:225>) | `void JeepEnemy::Go_( const EnemyDef& _def, const eJeepState _state)` |
| [L234](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/VehicleJeep.cpp:234>) | `[[nodiscard]] float JeepEnemy::TargetX_( const JeepConfig& _config, const Aabb& _view) const` |
| [L241](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/VehicleJeep.cpp:241>) | `[[nodiscard]] int JeepEnemy::ParkSide_( const JeepConfig& _config, const Aabb& _view) const` |
| [L254](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/VehicleJeep.cpp:254>) | `[[nodiscard]] bool JeepEnemy::PlayerOnFireSide_( const EnemyEnv& _env) const` |
| [L261](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/VehicleJeep.cpp:261>) | `void JeepEnemy::HashActionState(Fnv1a& _hash) const` |

## Source/Game/Enemies/VehicleMissileTank.cpp

[VehicleMissileTank.cpp](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/VehicleMissileTank.cpp>) · 179줄

| 위치 | 선언·함수 |
|---|---|
| [L12](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/VehicleMissileTank.cpp:12>) | `void MissileTankEnemy::Init( const EnemyDef& _def, EnemyEnv& _env)` |
| [L27](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/VehicleMissileTank.cpp:27>) | `void MissileTankEnemy::Tick( const EnemyDef& _def, EnemyEnv& _env)` |
| [L104](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/VehicleMissileTank.cpp:104>) | `void MissileTankEnemy::OnDeath( const EnemyDef& _def, EnemyEnv& _env)` |
| [L112](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/VehicleMissileTank.cpp:112>) | `void MissileTankEnemy::TickDying( const EnemyDef& _def, EnemyEnv& _env)` |
| [L119](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/VehicleMissileTank.cpp:119>) | `void ParseMissileTankConfig( DefReader& _r, EnemyDef& _def)` |
| [L148](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/VehicleMissileTank.cpp:148>) | `[[nodiscard]] bool MissileTankEnemy::IsUp_() const` |
| [L153](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/VehicleMissileTank.cpp:153>) | `[[nodiscard]] int MissileTankEnemy::ClipOf_( const MissileTankConfig& _config) const` |
| [L167](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/VehicleMissileTank.cpp:167>) | `void MissileTankEnemy::Go_( const EnemyDef& _def, const eMissileTankState _state)` |
| [L176](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/VehicleMissileTank.cpp:176>) | `void MissileTankEnemy::HashActionState(Fnv1a& _hash) const` |

## Source/Game/Enemies/VehicleSarubia.cpp

[VehicleSarubia.cpp](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/VehicleSarubia.cpp>) · 174줄

| 위치 | 선언·함수 |
|---|---|
| [L9](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/VehicleSarubia.cpp:9>) | `void SarubiaEnemy::Init( const EnemyDef& _def, EnemyEnv& _env)` |
| [L19](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/VehicleSarubia.cpp:19>) | `void SarubiaEnemy::Tick( const EnemyDef& _def, EnemyEnv& _env)` |
| [L46](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/VehicleSarubia.cpp:46>) | `else if (absDx > config.chaseDx)` |
| [L111](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/VehicleSarubia.cpp:111>) | `void SarubiaEnemy::OnDeath( const EnemyDef& _def, EnemyEnv& _env)` |
| [L119](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/VehicleSarubia.cpp:119>) | `void SarubiaEnemy::TickDying( const EnemyDef& _def, EnemyEnv& _env)` |
| [L126](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/VehicleSarubia.cpp:126>) | `void ParseSarubiaConfig( DefReader& _r, EnemyDef& _def)` |
| [L151](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/VehicleSarubia.cpp:151>) | `[[nodiscard]] int SarubiaEnemy::ClipOf_( const SarubiaConfig& _config) const` |
| [L167](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Enemies/VehicleSarubia.cpp:167>) | `void SarubiaEnemy::Go_( const EnemyDef& _def, const eSarubiaState _state)` |

## Source/Game/GameState.cpp

[GameState.cpp](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/GameState.cpp>) · 26줄

| 위치 | 선언·함수 |
|---|---|
| [L5](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/GameState.cpp:5>) | `void IGameState::OnEnter()` |
| [L9](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/GameState.cpp:9>) | `void IGameState::OnExit()` |
| [L13](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/GameState.cpp:13>) | `Scoped<IGameState> IGameState::TakeNextState()` |
| [L18](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/GameState.cpp:18>) | `bool IGameState::IsFinished() const` |
| [L23](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/GameState.cpp:23>) | `uint64_t IGameState::DebugHash() const` |

## Source/Game/GameState.h

[GameState.h](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/GameState.h>) · 38줄

| 위치 | 선언·함수 |
|---|---|
| [L8](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/GameState.h:8>) | `struct DebugSwitches` |
| [L25](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/GameState.h:25>) | `class IGameState` |
| [L28](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/GameState.h:28>) | `virtual ~IGameState() = default;` |
| [L29](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/GameState.h:29>) | `virtual void                             OnEnter();` |
| [L30](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/GameState.h:30>) | `virtual void                             OnExit();` |
| [L31](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/GameState.h:31>) | `virtual void                             Tick(const InputHistory& _input) = 0;` |
| [L32](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/GameState.h:32>) | `virtual void                             Render(float _alpha)             = 0;` |
| [L33](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/GameState.h:33>) | `[[nodiscard]] virtual Scoped<IGameState> TakeNextState();` |
| [L34](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/GameState.h:34>) | `[[nodiscard]] virtual bool               IsFinished() const;` |
| [L37](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/GameState.h:37>) | `[[nodiscard]] virtual uint64_t DebugHash() const;` |

## Source/Game/Player/PlayerAttack.cpp

[PlayerAttack.cpp](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Player/PlayerAttack.cpp>) · 336줄

| 위치 | 선언·함수 |
|---|---|
| [L10](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Player/PlayerAttack.cpp:10>) | `void PlayerController::TickActions_( const PlayerContext& _ctx, const PlayerCommand& _command)` |
| [L32](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Player/PlayerAttack.cpp:32>) | `void PlayerController::EndKnife_( const PlayerContext&)` |
| [L43](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Player/PlayerAttack.cpp:43>) | `void PlayerController::TickKnife_( const PlayerContext& _ctx, const PlayerCommand& _command)` |
| [L91](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Player/PlayerAttack.cpp:91>) | `void PlayerController::TickBomb_( const PlayerContext& _ctx, const PlayerCommand& _command)` |
| [L135](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Player/PlayerAttack.cpp:135>) | `void PlayerController::CancelBurst_()` |
| [L145](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Player/PlayerAttack.cpp:145>) | `void PlayerController::StopFlameSpawn_()` |
| [L155](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Player/PlayerAttack.cpp:155>) | `void PlayerController::EndFlame_()` |
| [L166](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Player/PlayerAttack.cpp:166>) | `void PlayerController::StartFlame_( const PlayerContext& _ctx, const WeaponTuning& _flame)` |
| [L180](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Player/PlayerAttack.cpp:180>) | `void PlayerController::TickFlame_( const PlayerContext& _ctx, const WeaponTuning& _flame)` |
| [L201](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Player/PlayerAttack.cpp:201>) | `void PlayerController::SpawnFlameParticle_( const PlayerContext& _ctx, const WeaponTuning& _flame)` |
| [L215](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Player/PlayerAttack.cpp:215>) | `void PlayerController::TickShot_( const PlayerContext& _ctx, const PlayerCommand& _command)` |
| [L295](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Player/PlayerAttack.cpp:295>) | `uint64_t PlayerController::MakeShotSeed_() const` |
| [L303](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Player/PlayerAttack.cpp:303>) | `VECTOR2 PlayerController::MakeMuzzle_( const Tuning& _tuning, const WeaponTuning& _weapon) const` |
| [L316](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Player/PlayerAttack.cpp:316>) | `void PlayerController::FireBullet_( const PlayerContext& _ctx, const WeaponTuning& _weapon)` |

## Source/Game/Player/PlayerController.cpp

[PlayerController.cpp](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Player/PlayerController.cpp>) · 515줄

| 위치 | 선언·함수 |
|---|---|
| [L10](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Player/PlayerController.cpp:10>) | `void PlayerController::Reset( const Tuning& _tuning, const VECTOR2 _spawnFeet)` |
| [L69](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Player/PlayerController.cpp:69>) | `void PlayerController::PickUpWeapon( const eWeapon _weapon, const Tuning& _tuning)` |
| [L76](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Player/PlayerController.cpp:76>) | `void PlayerController::SetVictory()` |
| [L98](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Player/PlayerController.cpp:98>) | `bool PlayerController::OnHitEvents( const Span<const HitEvent> _events)` |
| [L113](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Player/PlayerController.cpp:113>) | `void PlayerController::Kill()` |
| [L150](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Player/PlayerController.cpp:150>) | `Hurtbox PlayerController::MakeHurtbox( const Tuning& _tuning) const` |
| [L161](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Player/PlayerController.cpp:161>) | `void PlayerController::Tick( const PlayerContext& _ctx, const InputHistory& _input)` |
| [L210](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Player/PlayerController.cpp:210>) | `void PlayerController::TickDead_( const PlayerContext& _ctx)` |
| [L230](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Player/PlayerController.cpp:230>) | `void PlayerController::Respawn_( const PlayerContext& _ctx)` |
| [L258](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Player/PlayerController.cpp:258>) | `void PlayerController::TickAlive_( const PlayerContext& _ctx, const InputHistory& _input)` |
| [L308](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Player/PlayerController.cpp:308>) | `RigState PlayerController::MakeRigState() const` |
| [L344](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Player/PlayerController.cpp:344>) | `uint64_t PlayerController::HashState() const` |
| [L373](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Player/PlayerController.cpp:373>) | `void PlayerController::AddScore( const int _score)` |
| [L379](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Player/PlayerController.cpp:379>) | `void PlayerController::AddBombs( const int _count)` |
| [L385](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Player/PlayerController.cpp:385>) | `ePlayerState PlayerController::GetState() const` |
| [L390](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Player/PlayerController.cpp:390>) | `VECTOR2 PlayerController::GetFeet() const` |
| [L395](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Player/PlayerController.cpp:395>) | `VECTOR2 PlayerController::GetPrevFeet() const` |
| [L400](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Player/PlayerController.cpp:400>) | `VECTOR2 PlayerController::GetVelocity() const` |
| [L405](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Player/PlayerController.cpp:405>) | `int PlayerController::GetFacing() const` |
| [L410](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Player/PlayerController.cpp:410>) | `bool PlayerController::IsGrounded() const` |
| [L415](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Player/PlayerController.cpp:415>) | `ePose PlayerController::GetPose() const` |
| [L420](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Player/PlayerController.cpp:420>) | `eAim PlayerController::GetAim() const` |
| [L425](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Player/PlayerController.cpp:425>) | `eOverlay PlayerController::GetOverlay() const` |
| [L430](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Player/PlayerController.cpp:430>) | `const WeaponState& PlayerController::GetWeapon() const` |
| [L435](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Player/PlayerController.cpp:435>) | `int PlayerController::GetLives() const` |
| [L440](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Player/PlayerController.cpp:440>) | `int PlayerController::GetScore() const` |
| [L445](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Player/PlayerController.cpp:445>) | `bool PlayerController::IsAlive() const` |
| [L450](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Player/PlayerController.cpp:450>) | `bool PlayerController::IsInvincible() const` |
| [L455](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Player/PlayerController.cpp:455>) | `int PlayerController::GetInvincibleTicks() const` |
| [L460](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Player/PlayerController.cpp:460>) | `void PlayerController::ExtendInvincibility( const int _ticks)` |
| [L466](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Player/PlayerController.cpp:466>) | `bool PlayerController::IsGameOver() const` |
| [L471](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Player/PlayerController.cpp:471>) | `int PlayerController::GetGameOverTicks() const` |
| [L476](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Player/PlayerController.cpp:476>) | `bool PlayerController::IsKnifeInProgress() const` |
| [L481](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Player/PlayerController.cpp:481>) | `bool PlayerController::IsMoving() const` |
| [L486](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Player/PlayerController.cpp:486>) | `const PlayerStats& PlayerController::GetStats() const` |
| [L491](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Player/PlayerController.cpp:491>) | `const Body& PlayerController::GetBody() const` |
| [L496](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Player/PlayerController.cpp:496>) | `Aabb PlayerController::GetBodyBox() const` |
| [L501](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Player/PlayerController.cpp:501>) | `bool PlayerController::IsKnifeTargetInRange() const` |
| [L506](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Player/PlayerController.cpp:506>) | `int PlayerController::GetZ() const` |
| [L511](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Player/PlayerController.cpp:511>) | `void PlayerController::SetZ( const int _z)` |

## Source/Game/Player/PlayerController.h

[PlayerController.h](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Player/PlayerController.h>) · 174줄

| 위치 | 선언·함수 |
|---|---|
| [L13](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Player/PlayerController.h:13>) | `struct PlayerContext` |
| [L24](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Player/PlayerController.h:24>) | `struct PlayerStats` |
| [L34](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Player/PlayerController.h:34>) | `enum class eChutePhase` |
| [L41](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Player/PlayerController.h:41>) | `class PlayerController` |
| [L46](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Player/PlayerController.h:46>) | `PlayerController() = default;` |
| [L47](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Player/PlayerController.h:47>) | `void                             Reset(const Tuning& _tuning, VECTOR2 _spawnFeet);` |
| [L48](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Player/PlayerController.h:48>) | `bool                             SnapToGround(const PhysicsWorld& _world, float _maxDist);` |
| [L49](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Player/PlayerController.h:49>) | `void                             BeginChuteDescent(const Tuning& _tuning);` |
| [L50](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Player/PlayerController.h:50>) | `void                             Tick(const PlayerContext& _ctx, const InputHistory& _input);` |
| [L51](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Player/PlayerController.h:51>) | `bool                             OnHitEvents(Span<const HitEvent> _events);` |
| [L52](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Player/PlayerController.h:52>) | `void                             Kill();` |
| [L53](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Player/PlayerController.h:53>) | `void                             AddScore(int _score);` |
| [L54](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Player/PlayerController.h:54>) | `void                             PickUpWeapon(eWeapon _weapon, const Tuning& _tuning);` |
| [L55](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Player/PlayerController.h:55>) | `void                             AddBombs(int _count);` |
| [L56](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Player/PlayerController.h:56>) | `void                             SetVictory();` |
| [L57](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Player/PlayerController.h:57>) | `[[nodiscard]] eChutePhase        GetChutePhase() const;` |
| [L58](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Player/PlayerController.h:58>) | `[[nodiscard]] bool               IsChuteVisible(const Tuning& _tuning) const;` |
| [L59](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Player/PlayerController.h:59>) | `[[nodiscard]] VECTOR2            GetLogicalFeet() const;` |
| [L60](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Player/PlayerController.h:60>) | `void                             ExtendInvincibility(int _ticks);` |
| [L61](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Player/PlayerController.h:61>) | `[[nodiscard]] ePlayerState       GetState() const;` |
| [L62](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Player/PlayerController.h:62>) | `[[nodiscard]] VECTOR2            GetFeet() const;` |
| [L63](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Player/PlayerController.h:63>) | `[[nodiscard]] VECTOR2            GetPrevFeet() const;` |
| [L64](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Player/PlayerController.h:64>) | `[[nodiscard]] int                GetZ() const;` |
| [L65](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Player/PlayerController.h:65>) | `void                             SetZ(int _z);` |
| [L66](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Player/PlayerController.h:66>) | `[[nodiscard]] VECTOR2            GetVelocity() const;` |
| [L67](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Player/PlayerController.h:67>) | `[[nodiscard]] int                GetFacing() const;` |
| [L68](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Player/PlayerController.h:68>) | `[[nodiscard]] bool               IsGrounded() const;` |
| [L69](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Player/PlayerController.h:69>) | `[[nodiscard]] ePose              GetPose() const;` |
| [L70](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Player/PlayerController.h:70>) | `[[nodiscard]] eAim               GetAim() const;` |
| [L71](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Player/PlayerController.h:71>) | `[[nodiscard]] eOverlay           GetOverlay() const;` |
| [L72](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Player/PlayerController.h:72>) | `[[nodiscard]] const WeaponState& GetWeapon() const;` |
| [L73](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Player/PlayerController.h:73>) | `[[nodiscard]] int                GetLives() const;` |
| [L74](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Player/PlayerController.h:74>) | `[[nodiscard]] int                GetScore() const;` |
| [L75](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Player/PlayerController.h:75>) | `[[nodiscard]] bool               IsAlive() const;` |
| [L76](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Player/PlayerController.h:76>) | `[[nodiscard]] bool               IsInvincible() const;` |
| [L77](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Player/PlayerController.h:77>) | `[[nodiscard]] int                GetInvincibleTicks() const;` |
| [L78](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Player/PlayerController.h:78>) | `[[nodiscard]] bool               IsGameOver() const;` |
| [L79](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Player/PlayerController.h:79>) | `[[nodiscard]] int                GetGameOverTicks() const;` |
| [L80](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Player/PlayerController.h:80>) | `[[nodiscard]] bool               IsKnifeInProgress() const;` |
| [L81](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Player/PlayerController.h:81>) | `[[nodiscard]] bool               IsMoving() const;` |
| [L82](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Player/PlayerController.h:82>) | `[[nodiscard]] const PlayerStats& GetStats() const;` |
| [L83](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Player/PlayerController.h:83>) | `[[nodiscard]] const Body&        GetBody() const;` |
| [L84](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Player/PlayerController.h:84>) | `[[nodiscard]] Aabb               GetBodyBox() const;` |
| [L85](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Player/PlayerController.h:85>) | `[[nodiscard]] Hurtbox            MakeHurtbox(const Tuning& _tuning) const;` |
| [L86](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Player/PlayerController.h:86>) | `[[nodiscard]] bool               IsKnifeTargetInRange() const;` |
| [L87](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Player/PlayerController.h:87>) | `[[nodiscard]] RigState           MakeRigState() const;` |
| [L88](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Player/PlayerController.h:88>) | `[[nodiscard]] uint64_t           HashState() const;` |
| [L91](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Player/PlayerController.h:91>) | `void                  TickAlive_(const PlayerContext& _ctx, const InputHistory& _input);` |
| [L92](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Player/PlayerController.h:92>) | `void                  TickDead_(const PlayerContext& _ctx);` |
| [L93](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Player/PlayerController.h:93>) | `void                  TickMovement_(const PlayerContext& _ctx, const PlayerCommand& _command);` |
| [L94](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Player/PlayerController.h:94>) | `void                  TickActions_(const PlayerContext& _ctx, const PlayerCommand& _command);` |
| [L95](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Player/PlayerController.h:95>) | `void                  TickKnife_(const PlayerContext& _ctx, const PlayerCommand& _command);` |
| [L96](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Player/PlayerController.h:96>) | `void                  TickBomb_(const PlayerContext& _ctx, const PlayerCommand& _command);` |
| [L97](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Player/PlayerController.h:97>) | `void                  TickShot_(const PlayerContext& _ctx, const PlayerCommand& _command);` |
| [L98](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Player/PlayerController.h:98>) | `void                  FireBullet_(const PlayerContext& _ctx, const WeaponTuning& _weapon);` |
| [L99](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Player/PlayerController.h:99>) | `[[nodiscard]] VECTOR2 MakeMuzzle_(const Tuning& _tuning, const WeaponTuning& _weapon) const;` |
| [L100](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Player/PlayerController.h:100>) | `void                  CancelBurst_();` |
| [L101](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Player/PlayerController.h:101>) | `void                  StartFlame_(const PlayerContext& _ctx, const WeaponTuning& _flame);` |
| [L102](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Player/PlayerController.h:102>) | `void                  TickFlame_(const PlayerContext& _ctx, const WeaponTuning& _flame);` |
| [L103](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Player/PlayerController.h:103>) | `void                  SpawnFlameParticle_(const PlayerContext& _ctx, const WeaponTuning& _flame);` |
| [L104](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Player/PlayerController.h:104>) | `void                  StopFlameSpawn_();` |
| [L105](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Player/PlayerController.h:105>) | `void                  EndFlame_();` |
| [L107](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Player/PlayerController.h:107>) | `[[nodiscard]] uint64_t MakeShotSeed_() const;` |
| [L108](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Player/PlayerController.h:108>) | `void                   Respawn_(const PlayerContext& _ctx);` |
| [L109](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Player/PlayerController.h:109>) | `void                   UpdateAim_(const PlayerCommand& _command);` |
| [L110](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Player/PlayerController.h:110>) | `void                   DeriveState_();` |
| [L111](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Player/PlayerController.h:111>) | `void                   EndKnife_(const PlayerContext& _ctx);` |
| [L112](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Player/PlayerController.h:112>) | `void                   TickChuteDescend_(const Tuning& _tuning);` |
| [L113](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Player/PlayerController.h:113>) | `void                   StartChuteRelease_();` |

## Source/Game/Player/PlayerHitboxes.cpp

[PlayerHitboxes.cpp](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Player/PlayerHitboxes.cpp>) · 103줄

| 위치 | 선언·함수 |
|---|---|
| [L7](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Player/PlayerHitboxes.cpp:7>) | `[[nodiscard]] const MuzzleSet& GetSet_( const ePose _pose, const MuzzleTuning& _muzzle)` |
| [L20](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Player/PlayerHitboxes.cpp:20>) | `Aabb MakeKnifeTriggerBox( const VECTOR2 _feet, const int _facing, const float _bodyHalfWidth, const KnifeTuning& _knife)` |
| [L32](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Player/PlayerHitboxes.cpp:32>) | `Aabb MakeKnifeSwingBox( const VECTOR2 _feet, const int _facing, const KnifeTuning& _knife)` |
| [L42](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Player/PlayerHitboxes.cpp:42>) | `VECTOR2 GetMuzzlePosition( const VECTOR2 _feet, const int _facing, const ePose _pose, const eAim _aim, const MuzzleTuning& _muzzle)` |
| [L63](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Player/PlayerHitboxes.cpp:63>) | `VECTOR2 GetAimDirection( const eAim _aim, const int _facing)` |
| [L79](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Player/PlayerHitboxes.cpp:79>) | `VECTOR2 GetBombReleasePosition( const VECTOR2 _feet, const int _facing, const ePose _pose, const BombTuning& _bomb)` |
| [L89](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Player/PlayerHitboxes.cpp:89>) | `Aabb MakePlayerHurtboxBox( const VECTOR2 _feet, const ePose _pose, const BodyTuning& _body)` |
| [L98](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Player/PlayerHitboxes.cpp:98>) | `bool IsKnifeSwingActive( const int _ticksSinceStart, const KnifeTuning& _knife)` |

## Source/Game/Player/PlayerHitboxes.h

[PlayerHitboxes.h](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Player/PlayerHitboxes.h>) · 19줄

| 위치 | 선언·함수 |
|---|---|
| [L7](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Player/PlayerHitboxes.h:7>) | `[[nodiscard]] Aabb MakeKnifeTriggerBox(VECTOR2 _feet, int _facing, float _bodyHalfWidth, const KnifeTuning& _knife);` |
| [L9](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Player/PlayerHitboxes.h:9>) | `[[nodiscard]] Aabb MakeKnifeSwingBox(VECTOR2 _feet, int _facing, const KnifeTuning& _knife);` |
| [L11](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Player/PlayerHitboxes.h:11>) | `[[nodiscard]] VECTOR2 GetMuzzlePosition(VECTOR2 _feet, int _facing, ePose _pose, eAim _aim, const MuzzleTuning& _muzzle);` |
| [L13](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Player/PlayerHitboxes.h:13>) | `[[nodiscard]] VECTOR2 GetAimDirection(eAim _aim, int _facing);` |
| [L15](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Player/PlayerHitboxes.h:15>) | `[[nodiscard]] VECTOR2 GetBombReleasePosition(VECTOR2 _feet, int _facing, ePose _pose, const BombTuning& _bomb);` |
| [L17](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Player/PlayerHitboxes.h:17>) | `[[nodiscard]] Aabb MakePlayerHurtboxBox(VECTOR2 _feet, ePose _pose, const BodyTuning& _body);` |
| [L19](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Player/PlayerHitboxes.h:19>) | `[[nodiscard]] bool IsKnifeSwingActive(int _ticksSinceStart, const KnifeTuning& _knife);` |

## Source/Game/Player/PlayerInput.cpp

[PlayerInput.cpp](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Player/PlayerInput.cpp>) · 56줄

| 위치 | 선언·함수 |
|---|---|
| [L3](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Player/PlayerInput.cpp:3>) | `void PlayerInputBuffer::Reset()` |
| [L8](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Player/PlayerInput.cpp:8>) | `void PlayerInputBuffer::Update( const InputHistory& _history)` |
| [L18](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Player/PlayerInput.cpp:18>) | `else if (m_ticksSincePress[action] < kNever)` |
| [L25](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Player/PlayerInput.cpp:25>) | `bool PlayerInputBuffer::IsBuffered( const eAction _action, const int _windowTicks) const` |
| [L32](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Player/PlayerInput.cpp:32>) | `void PlayerInputBuffer::Consume( const eAction _action)` |
| [L38](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Player/PlayerInput.cpp:38>) | `PlayerCommand PlayerInputBuffer::Read( const InputHistory& _history, const MoveTuning& _move) const` |

## Source/Game/Player/PlayerInput.h

[PlayerInput.h](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Player/PlayerInput.h>) · 30줄

| 위치 | 선언·함수 |
|---|---|
| [L6](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Player/PlayerInput.h:6>) | `struct PlayerCommand` |
| [L17](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Player/PlayerInput.h:17>) | `class PlayerInputBuffer` |
| [L20](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Player/PlayerInput.h:20>) | `void                        Reset();` |
| [L21](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Player/PlayerInput.h:21>) | `void                        Update(const InputHistory& _history);` |
| [L22](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Player/PlayerInput.h:22>) | `[[nodiscard]] bool          IsBuffered(eAction _action, int _windowTicks) const;` |
| [L23](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Player/PlayerInput.h:23>) | `void                        Consume(eAction _action);` |
| [L24](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Player/PlayerInput.h:24>) | `[[nodiscard]] PlayerCommand Read(const InputHistory& _history, const MoveTuning& _move) const;` |

## Source/Game/Player/PlayerMovement.cpp

[PlayerMovement.cpp](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Player/PlayerMovement.cpp>) · 271줄

| 위치 | 선언·함수 |
|---|---|
| [L9](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Player/PlayerMovement.cpp:9>) | `[[nodiscard]] bool IsRegionBlocked_( const PhysicsWorld& _world, const Aabb& _region)` |
| [L40](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Player/PlayerMovement.cpp:40>) | `bool PlayerController::SnapToGround( const PhysicsWorld& _world, const float _maxDist)` |
| [L47](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Player/PlayerMovement.cpp:47>) | `void PlayerController::BeginChuteDescent( const Tuning& _tuning)` |
| [L69](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Player/PlayerMovement.cpp:69>) | `eChutePhase PlayerController::GetChutePhase() const` |
| [L74](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Player/PlayerMovement.cpp:74>) | `bool PlayerController::IsChuteVisible( const Tuning& _tuning) const` |
| [L89](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Player/PlayerMovement.cpp:89>) | `VECTOR2 PlayerController::GetLogicalFeet() const` |
| [L94](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Player/PlayerMovement.cpp:94>) | `void PlayerController::TickChuteDescend_( const Tuning& _tuning)` |
| [L112](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Player/PlayerMovement.cpp:112>) | `void PlayerController::StartChuteRelease_()` |
| [L118](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Player/PlayerMovement.cpp:118>) | `void PlayerController::TickMovement_( const PlayerContext& _ctx, const PlayerCommand& _command)` |
| [L204](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Player/PlayerMovement.cpp:204>) | `void PlayerController::UpdateAim_( const PlayerCommand& _command)` |
| [L215](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Player/PlayerMovement.cpp:215>) | `else if (_command.bDownHeld && !_command.bUpHeld && m_pose == ePose::Air)` |
| [L238](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Player/PlayerMovement.cpp:238>) | `else if (absDeg < 67.5f)` |
| [L248](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Player/PlayerMovement.cpp:248>) | `void PlayerController::DeriveState_()` |
| [L263](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Player/PlayerMovement.cpp:263>) | `else if (m_pose == ePose::Crouch)` |

## Source/Game/Player/PlayerRig.cpp

[PlayerRig.cpp](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Player/PlayerRig.cpp>) · 468줄

| 위치 | 선언·함수 |
|---|---|
| [L55](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Player/PlayerRig.cpp:55>) | `[[nodiscard]] constexpr bool AreSlotsFilled_()` |
| [L91](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Player/PlayerRig.cpp:91>) | `void ReplaceKey_( String& _inOut, const StringView _key, const StringView _value)` |
| [L103](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Player/PlayerRig.cpp:103>) | `[[nodiscard]] String Expand_( const char* _pPattern, const char* _pWeaponToken, const bool _bRight)` |
| [L114](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Player/PlayerRig.cpp:114>) | `[[nodiscard]] bool IsAirTimelineSlot_( const eRigClip _slot)` |
| [L120](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Player/PlayerRig.cpp:120>) | `[[nodiscard]] int FacingIndex_( const int _facing)` |
| [L127](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Player/PlayerRig.cpp:127>) | `const char* GetRigClipName( const eRigClip _clip)` |
| [L134](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Player/PlayerRig.cpp:134>) | `bool PlayerRig::IsSlotRequired( const eRigClip _slot)` |
| [L140](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Player/PlayerRig.cpp:140>) | `String PlayerRig::ResolveClipName( const AnimationLibrary& _library, const eRigClip _slot, const eWeapon _weapon, const int _facing)` |
| [L171](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Player/PlayerRig.cpp:171>) | `PlayerRig::BindReport PlayerRig::Bind( const AnimationLibrary& _library, const RigTuning& _tuning)` |
| [L219](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Player/PlayerRig.cpp:219>) | `const PlayerRig::SlotTable& PlayerRig::GetTable_( const eWeapon _weapon, const int _facing) const` |
| [L226](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Player/PlayerRig.cpp:226>) | `PlayerRig::Slot PlayerRig::GetSlot_( const SlotTable& _table, const eRigClip _slot) const` |
| [L248](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Player/PlayerRig.cpp:248>) | `AnimId PlayerRig::GetSlotClip( const eWeapon _weapon, const int _facing, const eRigClip _slot) const` |
| [L256](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Player/PlayerRig.cpp:256>) | `float PlayerRig::ClipTicks_( const AnimId _clip) const` |
| [L263](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Player/PlayerRig.cpp:263>) | `void PlayerRig::Update( const RigState& _state)` |
| [L285](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Player/PlayerRig.cpp:285>) | `else if (bVictory)` |
| [L289](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Player/PlayerRig.cpp:289>) | `else if (_state.pose == ePose::Crouch)` |
| [L344](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Player/PlayerRig.cpp:344>) | `else if (_state.pose == ePose::Air)` |
| [L395](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Player/PlayerRig.cpp:395>) | `else if (_state.overlay != eOverlay::None && !bDead && !bVictory)` |
| [L410](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Player/PlayerRig.cpp:410>) | `void PlayerRig::Submit( SpriteBatch& _sprites, const VECTOR2 _feet, const int _depth, const int _sortKey, const RGBA _tint) const` |
| [L439](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Player/PlayerRig.cpp:439>) | `AnimId PlayerRig::GetUpperClip() const` |
| [L444](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Player/PlayerRig.cpp:444>) | `AnimId PlayerRig::GetLowerClip() const` |
| [L449](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Player/PlayerRig.cpp:449>) | `int PlayerRig::GetUpperFrame() const` |
| [L454](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Player/PlayerRig.cpp:454>) | `int PlayerRig::GetLowerFrame() const` |
| [L459](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Player/PlayerRig.cpp:459>) | `float PlayerRig::GetOriginLift() const` |
| [L464](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Player/PlayerRig.cpp:464>) | `VECTOR2 PlayerRig::GetOrigin( const VECTOR2 _feet) const` |

## Source/Game/Player/PlayerRig.h

[PlayerRig.h](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Player/PlayerRig.h>) · 113줄

| 위치 | 선언·함수 |
|---|---|
| [L8](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Player/PlayerRig.h:8>) | `enum class eRigClip` |
| [L44](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Player/PlayerRig.h:44>) | `inline constexpr size_t kRigClipCount = static_cast<size_t>(eRigClip::Count);` |
| [L46](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Player/PlayerRig.h:46>) | `[[nodiscard]] const char* GetRigClipName(eRigClip _clip);` |
| [L48](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Player/PlayerRig.h:48>) | `struct RigState` |
| [L67](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Player/PlayerRig.h:67>) | `class PlayerRig` |
| [L70](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Player/PlayerRig.h:70>) | `struct BindReport` |
| [L76](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Player/PlayerRig.h:76>) | `BindReport                  Bind(const AnimationLibrary& _library, const RigTuning& _tuning);` |
| [L77](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Player/PlayerRig.h:77>) | `void                        Update(const RigState& _state);` |
| [L78](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Player/PlayerRig.h:78>) | `void                        Submit(SpriteBatch& _sprites, VECTOR2 _feet, int _depth, int _sortKey, RGBA _tint = { 255, 255, 255, 255 }) const;` |
| [L79](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Player/PlayerRig.h:79>) | `[[nodiscard]] AnimId        GetUpperClip() const;` |
| [L80](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Player/PlayerRig.h:80>) | `[[nodiscard]] AnimId        GetLowerClip() const;` |
| [L81](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Player/PlayerRig.h:81>) | `[[nodiscard]] int           GetUpperFrame() const;` |
| [L82](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Player/PlayerRig.h:82>) | `[[nodiscard]] int           GetLowerFrame() const;` |
| [L83](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Player/PlayerRig.h:83>) | `[[nodiscard]] float         GetOriginLift() const;` |
| [L84](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Player/PlayerRig.h:84>) | `[[nodiscard]] VECTOR2       GetOrigin(VECTOR2 _feet) const;` |
| [L85](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Player/PlayerRig.h:85>) | `[[nodiscard]] AnimId        GetSlotClip(eWeapon _weapon, int _facing, eRigClip _slot) const;` |
| [L86](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Player/PlayerRig.h:86>) | `[[nodiscard]] static String ResolveClipName(const AnimationLibrary& _library, eRigClip _slot, eWeapon _weapon, int _facing);` |
| [L87](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Player/PlayerRig.h:87>) | `[[nodiscard]] static bool   IsSlotRequired(eRigClip _slot);` |
| [L90](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Player/PlayerRig.h:90>) | `struct Slot` |
| [L97](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Player/PlayerRig.h:97>) | `[[nodiscard]] const SlotTable& GetTable_(eWeapon _weapon, int _facing) const;` |
| [L98](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Player/PlayerRig.h:98>) | `[[nodiscard]] Slot             GetSlot_(const SlotTable& _table, eRigClip _slot) const;` |
| [L99](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Player/PlayerRig.h:99>) | `[[nodiscard]] float            ClipTicks_(AnimId _clip) const;` |
| [L104](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Player/PlayerRig.h:104>) | `ARRAY<ARRAY<SlotTable, 2>, (CountOf<eWeapon>() - 1)> m_tables = {};` |

## Source/Game/Player/PlayerTypes.cpp

[PlayerTypes.cpp](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Player/PlayerTypes.cpp>) · 55줄

| 위치 | 선언·함수 |
|---|---|
| [L3](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Player/PlayerTypes.cpp:3>) | `const char* GetStateName( const ePlayerState _state)` |
| [L20](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Player/PlayerTypes.cpp:20>) | `const char* GetOverlayName( const eOverlay _overlay)` |
| [L32](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Player/PlayerTypes.cpp:32>) | `const char* GetGodModeName( const eGodMode _mode)` |
| [L43](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Player/PlayerTypes.cpp:43>) | `const char* GetAimName( const eAim _aim)` |

## Source/Game/Player/PlayerTypes.h

[PlayerTypes.h](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Player/PlayerTypes.h>) · 58줄

| 위치 | 선언·함수 |
|---|---|
| [L4](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Player/PlayerTypes.h:4>) | `enum class ePlayerState` |
| [L18](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Player/PlayerTypes.h:18>) | `enum class eGodMode` |
| [L26](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Player/PlayerTypes.h:26>) | `enum class eOverlay` |
| [L34](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Player/PlayerTypes.h:34>) | `enum class eAim` |
| [L43](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Player/PlayerTypes.h:43>) | `enum class ePose` |
| [L50](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Player/PlayerTypes.h:50>) | `[[nodiscard]] const char* GetStateName(ePlayerState _state);` |
| [L52](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Player/PlayerTypes.h:52>) | `[[nodiscard]] const char* GetOverlayName(eOverlay _overlay);` |
| [L54](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Player/PlayerTypes.h:54>) | `[[nodiscard]] const char* GetAimName(eAim _aim);` |
| [L56](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Player/PlayerTypes.h:56>) | `[[nodiscard]] const char* GetGodModeName(eGodMode _mode);` |

## Source/Game/Player/PlayerWeapons.cpp

[PlayerWeapons.cpp](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Player/PlayerWeapons.cpp>) · 236줄

| 위치 | 선언·함수 |
|---|---|
| [L10](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Player/PlayerWeapons.cpp:10>) | `[[nodiscard]] VECTOR2 OrientSize_( const VECTOR2 _size, const VECTOR2 _direction)` |
| [L29](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Player/PlayerWeapons.cpp:29>) | `void WeaponState::Reset( const Tuning& _tuning)` |
| [L38](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Player/PlayerWeapons.cpp:38>) | `void WeaponState::Give( const eWeapon _weapon, const int _ammo)` |
| [L53](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Player/PlayerWeapons.cpp:53>) | `void WeaponState::Tick()` |
| [L61](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Player/PlayerWeapons.cpp:61>) | `bool WeaponState::ShouldFire( const WeaponTuning& _tuning, const int _aliveBullets, const bool _bShotBuffered, const bool _bShotHeld) const` |
| [L82](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Player/PlayerWeapons.cpp:82>) | `void WeaponState::OnFired()` |
| [L92](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Player/PlayerWeapons.cpp:92>) | `eWeapon WeaponFromDebugSlot( const int _slot)` |
| [L105](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Player/PlayerWeapons.cpp:105>) | `BulletSpawn MakeBulletSpawn( const eWeapon _weapon, const WeaponTuning& _tuning, const uint32_t _owner, const VECTOR2 _muzzle, const VECTOR2 _sweepFrom, const VECTOR2 _direction)` |
| [L137](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Player/PlayerWeapons.cpp:137>) | `bool WeaponState::HasFiniteAmmo() const` |
| [L147](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Player/PlayerWeapons.cpp:147>) | `BulletSpawn Weapon::MakeShot(const WeaponTuning& _tuning, const ShotAim& _aim, RngMt19937&) const` |
| [L176](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Player/PlayerWeapons.cpp:176>) | `BulletSpawn HmgWeapon::MakeShot(const WeaponTuning& _tuning, const ShotAim& _aim, RngMt19937& _rng) const` |
| [L190](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Player/PlayerWeapons.cpp:190>) | `BulletSpawn FlameWeapon::MakeShot( const WeaponTuning& _tuning, const ShotAim& _aim, RngMt19937& _rng) const` |
| [L218](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Player/PlayerWeapons.cpp:218>) | `const Weapon& GetWeaponBehavior(const eWeapon _kind)` |

## Source/Game/Player/PlayerWeapons.h

[PlayerWeapons.h](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Player/PlayerWeapons.h>) · 51줄

| 위치 | 선언·함수 |
|---|---|
| [L6](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Player/PlayerWeapons.h:6>) | `struct WeaponState` |
| [L8](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Player/PlayerWeapons.h:8>) | `void               Reset(const Tuning& _tuning);` |
| [L9](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Player/PlayerWeapons.h:9>) | `void               Give(eWeapon _weapon, int _ammo);` |
| [L10](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Player/PlayerWeapons.h:10>) | `void               Tick();` |
| [L11](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Player/PlayerWeapons.h:11>) | `[[nodiscard]] bool ShouldFire(const WeaponTuning& _tuning, int _aliveBullets, bool _bShotBuffered, bool _bShotHeld) const;` |
| [L12](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Player/PlayerWeapons.h:12>) | `void               OnFired();` |
| [L13](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Player/PlayerWeapons.h:13>) | `[[nodiscard]] bool HasFiniteAmmo() const;` |
| [L23](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Player/PlayerWeapons.h:23>) | `struct ShotAim` |
| [L34](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Player/PlayerWeapons.h:34>) | `class Weapon` |
| [L39](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Player/PlayerWeapons.h:39>) | `explicit Weapon(eWeapon _kind);` |
| [L40](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Player/PlayerWeapons.h:40>) | `virtual ~Weapon() = default;` |
| [L41](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Player/PlayerWeapons.h:41>) | `[[nodiscard]] virtual BulletSpawn MakeShot(const WeaponTuning& _tuning, const ShotAim& _aim, RngMt19937& _rng) const;` |
| [L47](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Player/PlayerWeapons.h:47>) | `[[nodiscard]] const Weapon& GetWeaponBehavior(eWeapon _kind);` |
| [L49](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Player/PlayerWeapons.h:49>) | `[[nodiscard]] eWeapon WeaponFromDebugSlot(int _slot);` |
| [L51](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Player/PlayerWeapons.h:51>) | `[[nodiscard]] BulletSpawn MakeBulletSpawn(eWeapon _weapon, const WeaponTuning& _tuning, uint32_t _owner, VECTOR2 _muzzle, VECTOR2 _sweepFrom, VECTOR2 _direction);` |

## Source/Game/Props/DecorWorld.cpp

[DecorWorld.cpp](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Props/DecorWorld.cpp>) · 238줄

| 위치 | 선언·함수 |
|---|---|
| [L12](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Props/DecorWorld.cpp:12>) | `DecorSpawn MakeDecorSpawn( const StringView _clip, const eStageLayer _layer, const VECTOR2 _pos, const ParamValue* _pParams)` |
| [L70](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Props/DecorWorld.cpp:70>) | `uint32_t DecorWorld::Spawn( const DecorSpawn& _spawn)` |
| [L99](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Props/DecorWorld.cpp:99>) | `void DecorWorld::EnterStep_( Instance& _inst, const AnimationLibrary& _anims) const` |
| [L116](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Props/DecorWorld.cpp:116>) | `void DecorWorld::Advance_( Instance& _inst, const AnimationLibrary& _anims) const` |
| [L136](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Props/DecorWorld.cpp:136>) | `void DecorWorld::Tick( const AnimationLibrary& _anims, const float _dt)` |
| [L163](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Props/DecorWorld.cpp:163>) | `void DecorWorld::Submit( SpriteBatch& _sprites, const AnimationLibrary& _anims) const` |
| [L186](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Props/DecorWorld.cpp:186>) | `bool DecorWorld::SetZ( const uint32_t _id, const int _z)` |
| [L201](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Props/DecorWorld.cpp:201>) | `void DecorWorld::ClearByLayerMask( const uint32_t _layerMask)` |
| [L207](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Props/DecorWorld.cpp:207>) | `void DecorWorld::Clear()` |
| [L213](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Props/DecorWorld.cpp:213>) | `bool DecorWorld::Get( const uint32_t _id, VECTOR2& _outPos, VECTOR2& _outVelocity, StringView& _outClip, bool& _bOutFinished) const` |
| [L235](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Props/DecorWorld.cpp:235>) | `size_t DecorWorld::GetCount() const` |

## Source/Game/Props/DecorWorld.h

[DecorWorld.h](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Props/DecorWorld.h>) · 73줄

| 위치 | 선언·함수 |
|---|---|
| [L13](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Props/DecorWorld.h:13>) | `struct DecorStep` |
| [L21](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Props/DecorWorld.h:21>) | `struct DecorSpawn` |
| [L35](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Props/DecorWorld.h:35>) | `[[nodiscard]] DecorSpawn MakeDecorSpawn(StringView _clip, eStageLayer _layer, VECTOR2 _pos, const ParamValue* _pParams);` |
| [L37](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Props/DecorWorld.h:37>) | `class DecorWorld` |
| [L39](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Props/DecorWorld.h:39>) | `struct Instance` |
| [L58](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Props/DecorWorld.h:58>) | `uint32_t             Spawn(const DecorSpawn& _spawn);` |
| [L59](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Props/DecorWorld.h:59>) | `bool                 SetZ(uint32_t _id, int _z);` |
| [L60](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Props/DecorWorld.h:60>) | `void                 Tick(const AnimationLibrary& _anims, float _dt);` |
| [L61](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Props/DecorWorld.h:61>) | `void                 Submit(SpriteBatch& _sprites, const AnimationLibrary& _anims) const;` |
| [L62](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Props/DecorWorld.h:62>) | `void                 ClearByLayerMask(uint32_t _layerMask);` |
| [L63](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Props/DecorWorld.h:63>) | `void                 Clear();` |
| [L64](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Props/DecorWorld.h:64>) | `[[nodiscard]] size_t GetCount() const;` |
| [L65](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Props/DecorWorld.h:65>) | `[[nodiscard]] bool   Get(uint32_t _id, VECTOR2& _outPos, VECTOR2& _outVelocity, StringView& _outClip, bool& _bOutFinished) const;` |
| [L68](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Props/DecorWorld.h:68>) | `void EnterStep_(Instance& _inst, const AnimationLibrary& _anims) const;` |
| [L69](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Props/DecorWorld.h:69>) | `void Advance_(Instance& _inst, const AnimationLibrary& _anims) const;` |

## Source/Game/Props/ItemWorld.cpp

[ItemWorld.cpp](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Props/ItemWorld.cpp>) · 490줄

| 위치 | 선언·함수 |
|---|---|
| [L14](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Props/ItemWorld.cpp:14>) | `void MixValue_( uint64_t& _hash, const uint64_t _value)` |
| [L22](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Props/ItemWorld.cpp:22>) | `void MixFloat_( uint64_t& _hash, const float _value)` |
| [L37](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Props/ItemWorld.cpp:37>) | `[[nodiscard]] VECTOR2 BottomAlignedOrigin_( const AnimFrame& _frame, const VECTOR2 _feet)` |
| [L54](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Props/ItemWorld.cpp:54>) | `void ItemWorld::Reset( const uint64_t _seed)` |
| [L64](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Props/ItemWorld.cpp:64>) | `void ItemWorld::ClearAll()` |
| [L71](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Props/ItemWorld.cpp:71>) | `uint32_t ItemWorld::Spawn( const StringView _name, const VECTOR2 _feet, const bool _bBounce, const int _z)` |
| [L115](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Props/ItemWorld.cpp:115>) | `void ItemWorld::BindAnim_( PropItem& _item, const AnimationLibrary& _anims) const` |
| [L126](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Props/ItemWorld.cpp:126>) | `else if (def.visual.ref == eVisualRef::Anim)` |
| [L132](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Props/ItemWorld.cpp:132>) | `void ItemWorld::Tick( const PhysicsWorld& _world, const float _dt, const float _cameraLeft, const AnimationLibrary* _pAnims, const ItemTuning* _pTuning)` |
| [L224](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Props/ItemWorld.cpp:224>) | `void ItemWorld::SpawnPopup_( const VECTOR2 _pos, const int _score, const int _z)` |
| [L236](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Props/ItemWorld.cpp:236>) | `Vector<ItemPickup> ItemWorld::Collect( const Aabb& _playerBox)` |
| [L278](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Props/ItemWorld.cpp:278>) | `bool ItemWorld::SetZ( const uint32_t _id, const int _z)` |
| [L293](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Props/ItemWorld.cpp:293>) | `Vector<ItemFxRequest> ItemWorld::TakeFx()` |
| [L300](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Props/ItemWorld.cpp:300>) | `const PropItem* ItemWorld::Find( const uint32_t _id) const` |
| [L313](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Props/ItemWorld.cpp:313>) | `void ItemWorld::Submit( SpriteBatch& _sprites, const AnimationLibrary& _anims, const SpriteLibrary& _spriteLib, const BitmapFont* _pFont) const` |
| [L341](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Props/ItemWorld.cpp:341>) | `else if (pVisual->ref == eVisualRef::Anim)` |
| [L350](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Props/ItemWorld.cpp:350>) | `else if (item.bBegun && def.begin.ref == eVisualRef::Anim)` |
| [L355](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Props/ItemWorld.cpp:355>) | `else if (pVisual->ref == eVisualRef::Sprite)` |
| [L363](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Props/ItemWorld.cpp:363>) | `else if (pVisual->ref == eVisualRef::Anim)` |
| [L415](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Props/ItemWorld.cpp:415>) | `uint64_t ItemWorld::HashState() const` |
| [L440](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Props/ItemWorld.cpp:440>) | `void ItemWorld::SetLibrary( const PropDefLibrary* _pLibrary)` |
| [L447](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Props/ItemWorld.cpp:447>) | `Span<const PropItem> ItemWorld::GetItems() const` |
| [L452](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Props/ItemWorld.cpp:452>) | `Span<const ScorePopup> ItemWorld::GetPopups() const` |
| [L457](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Props/ItemWorld.cpp:457>) | `size_t ItemWorld::GetCount() const` |
| [L462](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Props/ItemWorld.cpp:462>) | `void ApplyItemPickup( PlayerController& _player, const ItemPickup& _pickup, const Tuning& _tuning)` |

## Source/Game/Props/ItemWorld.h

[ItemWorld.h](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Props/ItemWorld.h>) · 107줄

| 위치 | 선언·함수 |
|---|---|
| [L13](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Props/ItemWorld.h:13>) | `class PlayerController;` |
| [L14](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Props/ItemWorld.h:14>) | `class BitmapFont;` |
| [L28](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Props/ItemWorld.h:28>) | `struct ItemPickup` |
| [L40](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Props/ItemWorld.h:40>) | `struct ItemFxRequest` |
| [L47](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Props/ItemWorld.h:47>) | `struct PropItem` |
| [L66](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Props/ItemWorld.h:66>) | `struct ScorePopup` |
| [L74](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Props/ItemWorld.h:74>) | `class ItemWorld` |
| [L77](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Props/ItemWorld.h:77>) | `explicit ItemWorld(const PropDefLibrary* _pLibrary, uint64_t _seed = 1);` |
| [L78](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Props/ItemWorld.h:78>) | `void     SetLibrary(const PropDefLibrary* _pLibrary);` |
| [L79](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Props/ItemWorld.h:79>) | `void     Reset(uint64_t _seed = 1);` |
| [L80](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Props/ItemWorld.h:80>) | `uint32_t Spawn(StringView _name, VECTOR2 _feet, bool _bBounce = false, int _z = kDefaultZ);` |
| [L81](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Props/ItemWorld.h:81>) | `bool     SetZ(uint32_t _id, int _z);` |
| [L84](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Props/ItemWorld.h:84>) | `void                                 Tick(const PhysicsWorld& _world, float _dt, float _cameraLeft, const AnimationLibrary* _pAnims, const ItemTuning* _pTuning = nullptr);` |
| [L85](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Props/ItemWorld.h:85>) | `[[nodiscard]] Vector<ItemPickup>     Collect(const Aabb& _playerBox);` |
| [L86](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Props/ItemWorld.h:86>) | `[[nodiscard]] Vector<ItemFxRequest>  TakeFx();` |
| [L87](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Props/ItemWorld.h:87>) | `void                                 Submit(SpriteBatch& _sprites, const AnimationLibrary& _anims, const SpriteLibrary& _spriteLib, const BitmapFont* _pFont) const;` |
| [L88](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Props/ItemWorld.h:88>) | `void                                 ClearAll();` |
| [L89](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Props/ItemWorld.h:89>) | `[[nodiscard]] Span<const PropItem>   GetItems() const;` |
| [L90](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Props/ItemWorld.h:90>) | `[[nodiscard]] Span<const ScorePopup> GetPopups() const;` |
| [L91](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Props/ItemWorld.h:91>) | `[[nodiscard]] const PropItem*        Find(uint32_t _id) const;` |
| [L92](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Props/ItemWorld.h:92>) | `[[nodiscard]] size_t                 GetCount() const;` |
| [L93](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Props/ItemWorld.h:93>) | `[[nodiscard]] uint64_t               HashState() const;` |
| [L96](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Props/ItemWorld.h:96>) | `void SpawnPopup_(VECTOR2 _pos, int _score, int _z);` |
| [L97](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Props/ItemWorld.h:97>) | `void BindAnim_(PropItem& _item, const AnimationLibrary& _anims) const;` |
| [L107](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Props/ItemWorld.h:107>) | `void ApplyItemPickup(PlayerController& _player, const ItemPickup& _pickup, const Tuning& _tuning);` |

## Source/Game/Props/PropDef.cpp

[PropDef.cpp](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Props/PropDef.cpp>) · 1083줄

| 위치 | 선언·함수 |
|---|---|
| [L13](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Props/PropDef.cpp:13>) | `int GetDebrisClipCount( const eDebrisSet _set)` |
| [L25](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Props/PropDef.cpp:25>) | `String GetDebrisClipName( const eDebrisSet _set, const int _index)` |
| [L38](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Props/PropDef.cpp:38>) | `[[nodiscard]] eDebrisSet ParseDebrisSet_( const StringView _s, bool& _bOk)` |
| [L59](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Props/PropDef.cpp:59>) | `[[nodiscard]] String MissingAsset_( const AnimationLibrary& _anims, const SpriteLibrary& _sprites, const PropVisual& _visual, const String& _where)` |
| [L76](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Props/PropDef.cpp:76>) | `[[nodiscard]] String MissingBurstAsset_( const AnimationLibrary& _anims, const SpriteLibrary& _sprites, const PropBurst& _burst, const String& _where)` |
| [L105](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Props/PropDef.cpp:105>) | `explicit PropDefParser( PropDefLibrary& _lib) : m_lib(_lib)` |
| [L115](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Props/PropDef.cpp:115>) | `[[nodiscard]] const String& GetError() const noexcept` |
| [L174](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Props/PropDef.cpp:174>) | `bool PropDefParser::Fail_( const StringView _where, const StringView _field, const StringView _msg)` |
| [L186](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Props/PropDef.cpp:186>) | `bool PropDefParser::Keys_( const JsonReader& _obj, const StringView _where, const KeyList _allowed)` |
| [L203](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Props/PropDef.cpp:203>) | `bool PropDefParser::Num_( const JsonReader& _obj, const StringView _where, const StringView _key, float& _out, const bool _bRequired)` |
| [L214](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Props/PropDef.cpp:214>) | `bool PropDefParser::Int_( const JsonReader& _obj, const StringView _where, const StringView _key, int& _out, const bool _bRequired)` |
| [L238](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Props/PropDef.cpp:238>) | `bool PropDefParser::Bool_( const JsonReader& _obj, const StringView _where, const StringView _key, bool& _out)` |
| [L248](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Props/PropDef.cpp:248>) | `bool PropDefParser::Str_( const JsonReader& _obj, const StringView _where, const StringView _key, String& _out, const bool _bRequired)` |
| [L259](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Props/PropDef.cpp:259>) | `bool PropDefParser::Vec2_( const JsonReader& _obj, const StringView _where, const StringView _key, VECTOR2& _out, const bool _bRequired)` |
| [L270](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Props/PropDef.cpp:270>) | `bool PropDefParser::Visual_( const JsonReader& _obj, const StringView _where, const StringView _key, PropVisual& _out)` |
| [L299](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Props/PropDef.cpp:299>) | `bool PropDefParser::ParseRole_( const StringView _role, const StringView _where, Vector<ePropRole>& _outRoles)` |
| [L308](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Props/PropDef.cpp:308>) | `else if (_role == "wall")` |
| [L312](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Props/PropDef.cpp:312>) | `else if (_role == "platform")` |
| [L316](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Props/PropDef.cpp:316>) | `else if (_role == "enemyPlatform")` |
| [L327](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Props/PropDef.cpp:327>) | `bool PropDefParser::Parts_( const JsonReader& _arr, const StringView _where, Vector<PropPart>& _out)` |
| [L382](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Props/PropDef.cpp:382>) | `else if (ws == "destroyed")` |
| [L386](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Props/PropDef.cpp:386>) | `else if (ws == "always")` |
| [L410](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Props/PropDef.cpp:410>) | `bool PropDefParser::Burst_( const JsonReader& _obj, const StringView _where, PropBurst& _out, const bool _bAllowAfterSec)` |
| [L462](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Props/PropDef.cpp:462>) | `bool PropDefParser::Drops_( const JsonReader& _arr, const StringView _where, Vector<PropDrop>& _out)` |
| [L484](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Props/PropDef.cpp:484>) | `bool PropDefParser::Spawns_( const JsonReader& _arr, const StringView _where, Vector<PropChild>& _out)` |
| [L506](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Props/PropDef.cpp:506>) | `bool PropDefParser::Def_( const StringView _name, const JsonReader& _obj)` |
| [L616](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Props/PropDef.cpp:616>) | `bool PropDefParser::Item_( const StringView _name, const JsonReader& _obj)` |
| [L637](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Props/PropDef.cpp:637>) | `else if (kind == "supply")` |
| [L641](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Props/PropDef.cpp:641>) | `else if (kind == "score")` |
| [L657](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Props/PropDef.cpp:657>) | `else if (weapon == "shotgun")` |
| [L661](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Props/PropDef.cpp:661>) | `else if (weapon == "rocket")` |
| [L665](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Props/PropDef.cpp:665>) | `else if (weapon == "flame")` |
| [L681](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Props/PropDef.cpp:681>) | `else if (supply == "bullet")` |
| [L723](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Props/PropDef.cpp:723>) | `bool PropDefParser::CheckRefs_()` |
| [L746](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Props/PropDef.cpp:746>) | `bool PropDefParser::Run( const JsonReader& _root)` |
| [L832](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Props/PropDef.cpp:832>) | `bool PropDefParser::ParseParams( const PropDef& _def, const JsonReader& _params, PropSpawnParams& _out)` |
| [L905](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Props/PropDef.cpp:905>) | `Result<PropDefLibrary> PropDefLibrary::Parse( const StringView _json, String* _pMessageOrNull)` |
| [L933](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Props/PropDef.cpp:933>) | `Result<PropDefLibrary> PropDefLibrary::Load( const FilePath& _path, String* _pMessageOrNull)` |
| [L961](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Props/PropDef.cpp:961>) | `const PropDef* PropDefLibrary::Find( const StringView _name) const` |
| [L968](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Props/PropDef.cpp:968>) | `int PropDefLibrary::FindIndex( const StringView _name) const` |
| [L975](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Props/PropDef.cpp:975>) | `const PropItemDef* PropDefLibrary::FindItem( const StringView _name) const` |
| [L982](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Props/PropDef.cpp:982>) | `int PropDefLibrary::FindItemIndex( const StringView _name) const` |
| [L989](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Props/PropDef.cpp:989>) | `const PropItemGroup* PropDefLibrary::FindGroup( const StringView _name) const` |
| [L996](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Props/PropDef.cpp:996>) | `bool PropDefLibrary::ParseSpawnParams( const StringView _defName, const JsonReader& _params, PropSpawnParams& _out, String& _error) const` |
| [L1019](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Props/PropDef.cpp:1019>) | `String PropDefLibrary::FindMissingAsset( const AnimationLibrary& _anims, const SpriteLibrary& _sprites) const` |
| [L1075](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Props/PropDef.cpp:1075>) | `Span<const PropDef> PropDefLibrary::GetDefs() const` |
| [L1080](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Props/PropDef.cpp:1080>) | `Span<const PropItemDef> PropDefLibrary::GetItems() const` |

## Source/Game/Props/PropDef.h

[PropDef.h](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Props/PropDef.h>) · 234줄

| 위치 | 선언·함수 |
|---|---|
| [L13](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Props/PropDef.h:13>) | `class AnimationLibrary;` |
| [L14](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Props/PropDef.h:14>) | `class SpriteLibrary;` |
| [L16](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Props/PropDef.h:16>) | `enum class ePropRole` |
| [L24](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Props/PropDef.h:24>) | `enum class ePropWhen` |
| [L31](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Props/PropDef.h:31>) | `enum class eVisualRef` |
| [L38](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Props/PropDef.h:38>) | `enum class eDebrisSet` |
| [L45](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Props/PropDef.h:45>) | `[[nodiscard]] int    GetDebrisClipCount(eDebrisSet _set);` |
| [L46](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Props/PropDef.h:46>) | `[[nodiscard]] String GetDebrisClipName(eDebrisSet _set, int _index);` |
| [L48](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Props/PropDef.h:48>) | `struct PropVisual` |
| [L54](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Props/PropDef.h:54>) | `struct PropPart` |
| [L62](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Props/PropDef.h:62>) | `struct PropExplosion` |
| [L70](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Props/PropDef.h:70>) | `struct PropDebris` |
| [L76](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Props/PropDef.h:76>) | `struct PropBurst` |
| [L82](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Props/PropDef.h:82>) | `struct PropDrop` |
| [L90](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Props/PropDef.h:90>) | `struct PropChild` |
| [L96](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Props/PropDef.h:96>) | `struct PropFall` |
| [L104](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Props/PropDef.h:104>) | `struct PropSecondary` |
| [L110](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Props/PropDef.h:110>) | `struct PropGroundPatch` |
| [L117](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Props/PropDef.h:117>) | `struct PropVariant` |
| [L126](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Props/PropDef.h:126>) | `struct PropDef` |
| [L158](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Props/PropDef.h:158>) | `struct PropBoxOverride` |
| [L164](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Props/PropDef.h:164>) | `struct PropSpawnParams` |
| [L176](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Props/PropDef.h:176>) | `enum class eItemKind` |
| [L183](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Props/PropDef.h:183>) | `enum class eSupplyKind` |
| [L189](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Props/PropDef.h:189>) | `struct PropItemDef` |
| [L203](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Props/PropDef.h:203>) | `struct PropItemGroup` |
| [L209](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Props/PropDef.h:209>) | `class PropDefLibrary` |
| [L214](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Props/PropDef.h:214>) | `PropDefLibrary() = default;` |
| [L215](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Props/PropDef.h:215>) | `[[nodiscard]] static Result<PropDefLibrary> Load(const FilePath& _path, String* _pMessageOrNull = nullptr);` |
| [L216](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Props/PropDef.h:216>) | `[[nodiscard]] static Result<PropDefLibrary> Parse(StringView _json, String* _pMessageOrNull = nullptr);` |
| [L217](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Props/PropDef.h:217>) | `[[nodiscard]] const PropDef*                Find(StringView _name) const;` |
| [L218](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Props/PropDef.h:218>) | `[[nodiscard]] int                           FindIndex(StringView _name) const;` |
| [L219](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Props/PropDef.h:219>) | `[[nodiscard]] Span<const PropDef>           GetDefs() const;` |
| [L220](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Props/PropDef.h:220>) | `[[nodiscard]] const PropItemDef*            FindItem(StringView _name) const;` |
| [L221](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Props/PropDef.h:221>) | `[[nodiscard]] int                           FindItemIndex(StringView _name) const;` |
| [L222](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Props/PropDef.h:222>) | `[[nodiscard]] Span<const PropItemDef>       GetItems() const;` |
| [L223](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Props/PropDef.h:223>) | `[[nodiscard]] const PropItemGroup*          FindGroup(StringView _name) const;` |
| [L224](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Props/PropDef.h:224>) | `[[nodiscard]] bool                          ParseSpawnParams(StringView _defName, const JsonReader& _params, PropSpawnParams& _out, String& _error) const;` |
| [L225](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Props/PropDef.h:225>) | `[[nodiscard]] String                        FindMissingAsset(const AnimationLibrary& _anims, const SpriteLibrary& _sprites) const;` |

## Source/Game/Props/PropWorld.cpp

[PropWorld.cpp](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Props/PropWorld.cpp>) · 924줄

| 위치 | 선언·함수 |
|---|---|
| [L14](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Props/PropWorld.cpp:14>) | `[[nodiscard]] uint64_t MixHash_( const uint64_t _h, const uint64_t _v)` |
| [L21](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Props/PropWorld.cpp:21>) | `[[nodiscard]] uint64_t MixFloat_( const uint64_t _h, const float _f)` |
| [L40](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Props/PropWorld.cpp:40>) | `void PropWorld::SetLibrary( const PropDefLibrary* _pLibrary)` |
| [L48](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Props/PropWorld.cpp:48>) | `void PropWorld::Reset( const uint64_t _seed)` |
| [L57](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Props/PropWorld.cpp:57>) | `void PropWorld::ClearAll()` |
| [L70](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Props/PropWorld.cpp:70>) | `float PropWorld::RollFloat_( const float _lo, const float _hi)` |
| [L77](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Props/PropWorld.cpp:77>) | `int PropWorld::RollInt_( const int _n)` |
| [L87](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Props/PropWorld.cpp:87>) | `size_t PropWorld::FindIndex_( const PropHandle _id) const` |
| [L100](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Props/PropWorld.cpp:100>) | `const PropInstance* PropWorld::Get( const PropHandle _handle) const` |
| [L107](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Props/PropWorld.cpp:107>) | `bool PropWorld::IsDestroyed( const PropHandle _handle) const` |
| [L114](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Props/PropWorld.cpp:114>) | `bool PropWorld::IsDestroyFinished( const PropHandle _handle) const` |
| [L121](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Props/PropWorld.cpp:121>) | `bool PropWorld::IsAlive( const PropHandle _handle) const` |
| [L128](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Props/PropWorld.cpp:128>) | `int PropWorld::GetHp( const PropHandle _handle) const` |
| [L135](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Props/PropWorld.cpp:135>) | `Vector<PropEvent> PropWorld::TakeEvents()` |
| [L142](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Props/PropWorld.cpp:142>) | `PropHandle PropWorld::Spawn( const StringView _defName, const VECTOR2 _center, const PropSpawnParams& _params)` |
| [L150](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Props/PropWorld.cpp:150>) | `PropHandle PropWorld::SpawnByIndex( const int _defIndex, const VECTOR2 _center, const PropSpawnParams& _params)` |
| [L232](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Props/PropWorld.cpp:232>) | `uint32_t PropWorld::SpawnItem( const StringView _name, const VECTOR2 _feet, const bool _bBounce, const int _z)` |
| [L241](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Props/PropWorld.cpp:241>) | `bool PropWorld::PartActive_( const PropPart& _part, const PropInstance& _inst)` |
| [L254](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Props/PropWorld.cpp:254>) | `void PropWorld::RegisterHurtboxes( CombatSystem& _combat) const` |
| [L280](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Props/PropWorld.cpp:280>) | `void PropWorld::OnHitEvents( const Span<const HitEvent> _events)` |
| [L336](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Props/PropWorld.cpp:336>) | `void PropWorld::Destroy_( PropInstance& _inst, const PropDef& _def, const VECTOR2 _center)` |
| [L383](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Props/PropWorld.cpp:383>) | `void PropWorld::Detonate_( const PropBurst& _burst, const VECTOR2 _center)` |
| [L411](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Props/PropWorld.cpp:411>) | `void PropWorld::QueueDrops_( const PropDef& _def, const VECTOR2 _center)` |
| [L436](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Props/PropWorld.cpp:436>) | `ePropVisual PropWorld::ComputeVisual_( const PropInstance& _inst) const` |
| [L459](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Props/PropWorld.cpp:459>) | `void PropWorld::UpdateVisual_( PropInstance& _inst, const AnimationLibrary* _pAnims, const float _dt)` |
| [L494](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Props/PropWorld.cpp:494>) | `void PropWorld::Tick( const PropTickContext& _ctx)` |
| [L645](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Props/PropWorld.cpp:645>) | `void PropWorld::RebuildCollision_()` |
| [L674](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Props/PropWorld.cpp:674>) | `int PropWorld::SortKey( const PropInstance& _inst)` |
| [L686](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Props/PropWorld.cpp:686>) | `[[nodiscard]] bool ResolveFrame_( const PropInstance& _inst, const AnimationLibrary* _pAnims, const SpriteLibrary* _pSprites, AnimFrame& _out)` |
| [L702](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Props/PropWorld.cpp:702>) | `else if (pv.ref == eVisualRef::Anim)` |
| [L719](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Props/PropWorld.cpp:719>) | `bool PropWorld::GetVisualBounds( const PropInstance& _inst, const AnimationLibrary* _pAnims, Aabb& _out) const` |
| [L734](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Props/PropWorld.cpp:734>) | `void PropWorld::SetSpriteLibrary( const SpriteLibrary* _pSprites)` |
| [L740](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Props/PropWorld.cpp:740>) | `void PropWorld::Submit( SpriteBatch& _sprites, const AnimationLibrary& _anims, const SpriteLibrary& _spriteLib, const BitmapFont* _pFont)` |
| [L773](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Props/PropWorld.cpp:773>) | `bool ApplyGroundPatch( GroundMap& _map, const VECTOR2 _topLeft, const GroundMap& _footprint)` |
| [L843](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Props/PropWorld.cpp:843>) | `uint64_t PropWorld::HashState() const` |
| [L869](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Props/PropWorld.cpp:869>) | `const PropDefLibrary* PropWorld::GetLibrary() const` |
| [L874](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Props/PropWorld.cpp:874>) | `Span<const Platform> PropWorld::GetPlatforms() const` |
| [L879](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Props/PropWorld.cpp:879>) | `Span<const Platform> PropWorld::GetEnemyPlatforms() const` |
| [L884](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Props/PropWorld.cpp:884>) | `Span<const Wall> PropWorld::GetWalls() const` |
| [L889](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Props/PropWorld.cpp:889>) | `Span<const PropInstance> PropWorld::GetProps() const` |
| [L894](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Props/PropWorld.cpp:894>) | `size_t PropWorld::GetCount() const` |
| [L899](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Props/PropWorld.cpp:899>) | `Span<const PropDebrisPiece> PropWorld::GetDebris() const` |
| [L904](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Props/PropWorld.cpp:904>) | `ItemWorld& PropWorld::GetItems()` |
| [L909](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Props/PropWorld.cpp:909>) | `const ItemWorld& PropWorld::GetItems() const` |
| [L914](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Props/PropWorld.cpp:914>) | `uint32_t PropWorld::SpawnDecor( const DecorSpawn& _spawn)` |
| [L920](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Props/PropWorld.cpp:920>) | `void PropWorld::ClearDecorByLayerMask( const uint32_t _layerMask)` |

## Source/Game/Props/PropWorld.h

[PropWorld.h](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Props/PropWorld.h>) · 199줄

| 위치 | 선언·함수 |
|---|---|
| [L17](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Props/PropWorld.h:17>) | `class BitmapFont;` |
| [L28](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Props/PropWorld.h:28>) | `enum class ePropEventKind` |
| [L39](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Props/PropWorld.h:39>) | `struct PropEvent` |
| [L49](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Props/PropWorld.h:49>) | `enum class ePropVisual` |
| [L59](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Props/PropWorld.h:59>) | `struct PropInstance` |
| [L97](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Props/PropWorld.h:97>) | `struct PropFxRequest` |
| [L104](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Props/PropWorld.h:104>) | `struct PropDebrisPiece` |
| [L114](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Props/PropWorld.h:114>) | `struct PropTickContext` |
| [L124](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Props/PropWorld.h:124>) | `class PropWorld` |
| [L126](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Props/PropWorld.h:126>) | `struct PendingChild` |
| [L133](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Props/PropWorld.h:133>) | `explicit PropWorld(const PropDefLibrary* _pLibrary, uint64_t _seed = 1);` |
| [L134](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Props/PropWorld.h:134>) | `void                                SetLibrary(const PropDefLibrary* _pLibrary);` |
| [L135](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Props/PropWorld.h:135>) | `[[nodiscard]] const PropDefLibrary* GetLibrary() const;` |
| [L136](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Props/PropWorld.h:136>) | `void                                Reset(uint64_t _seed = 1);` |
| [L137](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Props/PropWorld.h:137>) | `PropHandle                          Spawn(StringView _defName, VECTOR2 _center, const PropSpawnParams& _params = {});` |
| [L138](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Props/PropWorld.h:138>) | `PropHandle                          SpawnByIndex(int _defIndex, VECTOR2 _center, const PropSpawnParams& _params = {});` |
| [L139](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Props/PropWorld.h:139>) | `uint32_t                            SpawnItem(StringView _name, VECTOR2 _feet, bool _bBounce = false, int _z = kDefaultZ);` |
| [L140](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Props/PropWorld.h:140>) | `void                                Tick(const PropTickContext& _ctx);` |
| [L143](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Props/PropWorld.h:143>) | `void                                      SetSpriteLibrary(const SpriteLibrary* _pSprites);` |
| [L144](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Props/PropWorld.h:144>) | `[[nodiscard]] bool                        GetVisualBounds(const PropInstance& _inst, const AnimationLibrary* _pAnims, Aabb& _out) const;` |
| [L145](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Props/PropWorld.h:145>) | `void                                      RegisterHurtboxes(CombatSystem& _combat) const;` |
| [L146](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Props/PropWorld.h:146>) | `void                                      OnHitEvents(Span<const HitEvent> _events);` |
| [L147](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Props/PropWorld.h:147>) | `[[nodiscard]] Span<const Platform>        GetPlatforms() const;` |
| [L148](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Props/PropWorld.h:148>) | `[[nodiscard]] Span<const Platform>        GetEnemyPlatforms() const;` |
| [L149](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Props/PropWorld.h:149>) | `[[nodiscard]] Span<const Wall>            GetWalls() const;` |
| [L150](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Props/PropWorld.h:150>) | `[[nodiscard]] bool                        IsDestroyed(PropHandle _handle) const;` |
| [L151](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Props/PropWorld.h:151>) | `[[nodiscard]] bool                        IsDestroyFinished(PropHandle _handle) const;` |
| [L152](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Props/PropWorld.h:152>) | `[[nodiscard]] bool                        IsAlive(PropHandle _handle) const;` |
| [L153](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Props/PropWorld.h:153>) | `[[nodiscard]] int                         GetHp(PropHandle _handle) const;` |
| [L154](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Props/PropWorld.h:154>) | `[[nodiscard]] const PropInstance*         Get(PropHandle _handle) const;` |
| [L155](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Props/PropWorld.h:155>) | `[[nodiscard]] Span<const PropInstance>    GetProps() const;` |
| [L156](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Props/PropWorld.h:156>) | `[[nodiscard]] size_t                      GetCount() const;` |
| [L157](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Props/PropWorld.h:157>) | `[[nodiscard]] Span<const PropDebrisPiece> GetDebris() const;` |
| [L158](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Props/PropWorld.h:158>) | `[[nodiscard]] Vector<PropEvent>           TakeEvents();` |
| [L159](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Props/PropWorld.h:159>) | `[[nodiscard]] ItemWorld&                  GetItems();` |
| [L160](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Props/PropWorld.h:160>) | `[[nodiscard]] const ItemWorld&            GetItems() const;` |
| [L163](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Props/PropWorld.h:163>) | `uint32_t                 SpawnDecor(const DecorSpawn& _spawn);` |
| [L164](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Props/PropWorld.h:164>) | `void                     ClearDecorByLayerMask(uint32_t _layerMask);` |
| [L165](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Props/PropWorld.h:165>) | `void                     ClearAll();` |
| [L166](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Props/PropWorld.h:166>) | `void                     Submit(SpriteBatch& _sprites, const AnimationLibrary& _anims, const SpriteLibrary& _spriteLib, const BitmapFont* _pFont = nullptr);` |
| [L167](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Props/PropWorld.h:167>) | `[[nodiscard]] static int SortKey(const PropInstance& _inst);` |
| [L168](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Props/PropWorld.h:168>) | `[[nodiscard]] uint64_t   HashState() const;` |
| [L171](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Props/PropWorld.h:171>) | `[[nodiscard]] size_t      FindIndex_(PropHandle _id) const;` |
| [L172](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Props/PropWorld.h:172>) | `[[nodiscard]] float       RollFloat_(float _lo, float _hi);` |
| [L173](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Props/PropWorld.h:173>) | `[[nodiscard]] int         RollInt_(int _n);` |
| [L174](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Props/PropWorld.h:174>) | `void                      Destroy_(PropInstance& _inst, const PropDef& _def, VECTOR2 _center);` |
| [L175](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Props/PropWorld.h:175>) | `void                      Detonate_(const PropBurst& _burst, VECTOR2 _center);` |
| [L176](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Props/PropWorld.h:176>) | `void                      QueueDrops_(const PropDef& _def, VECTOR2 _center);` |
| [L177](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Props/PropWorld.h:177>) | `void                      UpdateVisual_(PropInstance& _inst, const AnimationLibrary* _pAnims, float _dt);` |
| [L178](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Props/PropWorld.h:178>) | `void                      RebuildCollision_();` |
| [L179](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Props/PropWorld.h:179>) | `[[nodiscard]] ePropVisual ComputeVisual_(const PropInstance& _inst) const;` |
| [L180](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Props/PropWorld.h:180>) | `[[nodiscard]] static bool PartActive_(const PropPart& _part, const PropInstance& _inst);` |
| [L199](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Props/PropWorld.h:199>) | `bool ApplyGroundPatch(GroundMap& _map, VECTOR2 _topLeft, const GroundMap& _footprint);` |

## Source/Game/Sandbox/Effects.cpp

[Effects.cpp](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Sandbox/Effects.cpp>) · 110줄

| 위치 | 선언·함수 |
|---|---|
| [L9](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Sandbox/Effects.cpp:9>) | `void StepDebris_( Effect& _effect, const float _dt)` |
| [L20](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Sandbox/Effects.cpp:20>) | `void EffectList::Clear()` |
| [L25](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Sandbox/Effects.cpp:25>) | `void EffectList::Spawn( const AnimationLibrary& _library, const StringView _clipName, const VECTOR2 _position, const int _depth, const bool _bFlipX)` |
| [L46](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Sandbox/Effects.cpp:46>) | `void EffectList::SpawnDebris( const AnimationLibrary& _library, const StringView _clipName, const VECTOR2 _position, const int _depth, const VECTOR2 _velocity, const bool _bFlipX)` |
| [L64](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Sandbox/Effects.cpp:64>) | `void EffectList::Update( const AnimationLibrary& _library, const float _dt, const float _cullBelowY)` |
| [L87](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Sandbox/Effects.cpp:87>) | `void EffectList::Submit( const AnimationLibrary& _library, SpriteBatch& _sprites) const` |
| [L102](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Sandbox/Effects.cpp:102>) | `size_t EffectList::GetCount() const` |
| [L107](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Sandbox/Effects.cpp:107>) | `Span<const Effect> EffectList::GetEffects() const` |

## Source/Game/Sandbox/Effects.h

[Effects.h](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Sandbox/Effects.h>) · 42줄

| 위치 | 선언·함수 |
|---|---|
| [L15](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Sandbox/Effects.h:15>) | `class GroundMap;` |
| [L17](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Sandbox/Effects.h:17>) | `struct Effect` |
| [L29](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Sandbox/Effects.h:29>) | `class EffectList` |
| [L32](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Sandbox/Effects.h:32>) | `void                             Clear();` |
| [L33](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Sandbox/Effects.h:33>) | `void                             Spawn(const AnimationLibrary& _library, StringView _clipName, VECTOR2 _position, int _depth = 50, bool _bFlipX = false);` |
| [L34](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Sandbox/Effects.h:34>) | `void                             SpawnDebris(const AnimationLibrary& _library, StringView _clipName, VECTOR2 _position, int _depth, VECTOR2 _velocity, bool _bFlipX = false);` |
| [L35](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Sandbox/Effects.h:35>) | `void                             Update(const AnimationLibrary& _library, float _dt, float _cullBelowY = 1.0e9f);` |
| [L36](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Sandbox/Effects.h:36>) | `void                             Submit(const AnimationLibrary& _library, SpriteBatch& _sprites) const;` |
| [L37](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Sandbox/Effects.h:37>) | `[[nodiscard]] size_t             GetCount() const;` |
| [L38](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Sandbox/Effects.h:38>) | `[[nodiscard]] Span<const Effect> GetEffects() const;` |

## Source/Game/Sandbox/TrainingTargets.cpp

[TrainingTargets.cpp](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Sandbox/TrainingTargets.cpp>) · 241줄

| 위치 | 선언·함수 |
|---|---|
| [L7](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Sandbox/TrainingTargets.cpp:7>) | `VECTOR2 GetDummySize( const eDummyKind _kind)` |
| [L18](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Sandbox/TrainingTargets.cpp:18>) | `int GetDummyHp( const eDummyKind _kind, const SandboxTuning& _tuning)` |
| [L30](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Sandbox/TrainingTargets.cpp:30>) | `const char* GetDummyName( const eDummyKind _kind)` |
| [L41](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Sandbox/TrainingTargets.cpp:41>) | `void TrainingTargets::Clear()` |
| [L48](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Sandbox/TrainingTargets.cpp:48>) | `uint32_t TrainingTargets::Add( const eDummyKind _kind, const VECTOR2 _feet, const bool _bShooter, const SandboxTuning& _tuning)` |
| [L66](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Sandbox/TrainingTargets.cpp:66>) | `const TrainingDummy* TrainingTargets::Find( const uint32_t _id) const` |
| [L79](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Sandbox/TrainingTargets.cpp:79>) | `void TrainingTargets::RegisterHurtboxes( CombatSystem& _combat) const` |
| [L97](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Sandbox/TrainingTargets.cpp:97>) | `void TrainingTargets::TickShooters( const SandboxTuning& _tuning, const VECTOR2 _playerFeet, const bool _bPlayerAlive, ProjectileSystem& _projectiles, CombatSystem& _combat)` |
| [L142](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Sandbox/TrainingTargets.cpp:142>) | `int TrainingTargets::ApplyEvents( const Span<const HitEvent> _events, const SandboxTuning& _tuning)` |
| [L180](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Sandbox/TrainingTargets.cpp:180>) | `void TrainingTargets::Revive_( TrainingDummy& _dummy, const SandboxTuning& _tuning) const` |
| [L191](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Sandbox/TrainingTargets.cpp:191>) | `void TrainingTargets::Tick( const SandboxTuning& _tuning)` |
| [L208](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Sandbox/TrainingTargets.cpp:208>) | `uint64_t TrainingTargets::HashState() const` |
| [L223](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Sandbox/TrainingTargets.cpp:223>) | `Span<const TrainingDummy> TrainingTargets::GetDummies() const` |
| [L228](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Sandbox/TrainingTargets.cpp:228>) | `Span<TrainingDummy> TrainingTargets::GetDummies()` |
| [L233](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Sandbox/TrainingTargets.cpp:233>) | `int TrainingTargets::GetKillCount() const` |
| [L238](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Sandbox/TrainingTargets.cpp:238>) | `bool TrainingDummy::IsAlive() const` |

## Source/Game/Sandbox/TrainingTargets.h

[TrainingTargets.h](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Sandbox/TrainingTargets.h>) · 64줄

| 위치 | 선언·함수 |
|---|---|
| [L10](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Sandbox/TrainingTargets.h:10>) | `enum class eDummyKind` |
| [L20](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Sandbox/TrainingTargets.h:20>) | `struct TrainingDummy` |
| [L36](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Sandbox/TrainingTargets.h:36>) | `[[nodiscard]] bool IsAlive() const;` |
| [L39](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Sandbox/TrainingTargets.h:39>) | `class TrainingTargets` |
| [L42](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Sandbox/TrainingTargets.h:42>) | `void                                    Clear();` |
| [L43](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Sandbox/TrainingTargets.h:43>) | `uint32_t                                Add(eDummyKind _kind, VECTOR2 _feet, bool _bShooter, const SandboxTuning& _tuning);` |
| [L44](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Sandbox/TrainingTargets.h:44>) | `void                                    RegisterHurtboxes(CombatSystem& _combat) const;` |
| [L45](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Sandbox/TrainingTargets.h:45>) | `void                                    TickShooters(const SandboxTuning& _tuning, VECTOR2 _playerFeet, bool _bPlayerAlive, ProjectileSystem& _projectiles, CombatSystem& _combat);` |
| [L46](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Sandbox/TrainingTargets.h:46>) | `int                                     ApplyEvents(Span<const HitEvent> _events, const SandboxTuning& _tuning);` |
| [L47](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Sandbox/TrainingTargets.h:47>) | `void                                    Tick(const SandboxTuning& _tuning);` |
| [L48](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Sandbox/TrainingTargets.h:48>) | `[[nodiscard]] Span<const TrainingDummy> GetDummies() const;` |
| [L49](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Sandbox/TrainingTargets.h:49>) | `[[nodiscard]] Span<TrainingDummy>       GetDummies();` |
| [L50](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Sandbox/TrainingTargets.h:50>) | `[[nodiscard]] const TrainingDummy*      Find(uint32_t _id) const;` |
| [L51](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Sandbox/TrainingTargets.h:51>) | `[[nodiscard]] int                       GetKillCount() const;` |
| [L52](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Sandbox/TrainingTargets.h:52>) | `[[nodiscard]] uint64_t                  HashState() const;` |
| [L55](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Sandbox/TrainingTargets.h:55>) | `void Revive_(TrainingDummy& _dummy, const SandboxTuning& _tuning) const;` |
| [L62](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Sandbox/TrainingTargets.h:62>) | `[[nodiscard]] VECTOR2     GetDummySize(eDummyKind _kind);` |
| [L63](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Sandbox/TrainingTargets.h:63>) | `[[nodiscard]] int         GetDummyHp(eDummyKind _kind, const SandboxTuning& _tuning);` |
| [L64](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Sandbox/TrainingTargets.h:64>) | `[[nodiscard]] const char* GetDummyName(eDummyKind _kind);` |

## Source/Game/SandboxState.cpp

[SandboxState.cpp](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/SandboxState.cpp>) · 953줄

| 위치 | 선언·함수 |
|---|---|
| [L30](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/SandboxState.cpp:30>) | `void SandboxState::OnEnter()` |
| [L42](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/SandboxState.cpp:42>) | `void SandboxState::OnExit()` |
| [L47](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/SandboxState.cpp:47>) | `void SandboxState::BuildWorld_()` |
| [L155](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/SandboxState.cpp:155>) | `void SandboxState::RestartGame()` |
| [L220](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/SandboxState.cpp:220>) | `void SandboxState::TickBusLab_()` |
| [L238](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/SandboxState.cpp:238>) | `void SandboxState::AddDefaultTargets_()` |
| [L253](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/SandboxState.cpp:253>) | `uint32_t SandboxState::AddTarget( const eDummyKind _kind, const VECTOR2 _feet, const bool _bShooter)` |
| [L261](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/SandboxState.cpp:261>) | `bool SandboxState::LoadEnemyDefs_( const FilePath& _assetRoot, EnemyDefLibrary& _out) const` |
| [L284](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/SandboxState.cpp:284>) | `bool SandboxState::ReloadEnemyDefs( const FilePath& _assetRoot)` |
| [L299](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/SandboxState.cpp:299>) | `bool SandboxState::ReloadEnemyDefsKeep( const FilePath& _assetRoot)` |
| [L334](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/SandboxState.cpp:334>) | `const GroundMap& SandboxState::GetGround() const` |
| [L339](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/SandboxState.cpp:339>) | `EnemyHandle SandboxState::SpawnEnemy( const StringView _defName, const VECTOR2 _feet, const SpawnParams& _params)` |
| [L348](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/SandboxState.cpp:348>) | `void SandboxState::SpawnDebugEnemy_( const int _slot, const float _aheadPx)` |
| [L379](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/SandboxState.cpp:379>) | `else if (name == "soldier_hang")` |
| [L388](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/SandboxState.cpp:388>) | `PhysicsWorld SandboxState::MakeEnemyPhysicsWorld_() const` |
| [L397](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/SandboxState.cpp:397>) | `void SandboxState::UpdateWalls_()` |
| [L415](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/SandboxState.cpp:415>) | `void SandboxState::UpdatePropCollision_()` |
| [L425](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/SandboxState.cpp:425>) | `PhysicsWorld SandboxState::MakePhysicsWorld_() const` |
| [L434](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/SandboxState.cpp:434>) | `VECTOR2 SandboxState::GetRespawnFeet_() const` |
| [L440](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/SandboxState.cpp:440>) | `void SandboxState::Tick( const InputHistory& _input)` |
| [L566](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/SandboxState.cpp:566>) | `else if (request.kind == eFxKind::RocketExplosion)` |
| [L570](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/SandboxState.cpp:570>) | `else if (request.kind == eFxKind::RocketSmoke)` |
| [L623](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/SandboxState.cpp:623>) | `void SandboxState::RequestSounds_( const Vector<FxRequest>& _fx)` |
| [L652](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/SandboxState.cpp:652>) | `else if (request.kind == eFxKind::RocketExplosion)` |
| [L683](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/SandboxState.cpp:683>) | `uint64_t SandboxState::HashState() const` |
| [L701](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/SandboxState.cpp:701>) | `void SandboxState::Render( const float _alpha)` |
| [L723](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/SandboxState.cpp:723>) | `void SandboxState::DrawWorld_( const VECTOR2 _cameraPos, const float _alpha)` |
| [L772](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/SandboxState.cpp:772>) | `else if (m_player.IsAlive())` |
| [L809](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/SandboxState.cpp:809>) | `void SandboxState::DrawDebugBoxes_( Renderer& _renderer, const VECTOR2 _cameraPos) const` |
| [L839](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/SandboxState.cpp:839>) | `void SandboxState::DrawHud_()` |
| [L880](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/SandboxState.cpp:880>) | `PlayerController& SandboxState::GetPlayer()` |
| [L885](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/SandboxState.cpp:885>) | `const PlayerController& SandboxState::GetPlayer() const` |
| [L890](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/SandboxState.cpp:890>) | `TrainingTargets& SandboxState::GetTargets()` |
| [L895](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/SandboxState.cpp:895>) | `ProjectileSystem& SandboxState::GetProjectiles()` |
| [L900](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/SandboxState.cpp:900>) | `CombatSystem& SandboxState::GetCombat()` |
| [L905](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/SandboxState.cpp:905>) | `Tuning& SandboxState::GetTuning()` |
| [L910](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/SandboxState.cpp:910>) | `const Camera& SandboxState::GetCamera() const` |
| [L915](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/SandboxState.cpp:915>) | `PlayerRig& SandboxState::GetRig()` |
| [L920](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/SandboxState.cpp:920>) | `const PlayerRig& SandboxState::GetRig() const` |
| [L925](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/SandboxState.cpp:925>) | `uint64_t SandboxState::GetTickCount() const` |
| [L930](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/SandboxState.cpp:930>) | `EnemyWorld& SandboxState::GetEnemies()` |
| [L935](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/SandboxState.cpp:935>) | `const EnemyWorld& SandboxState::GetEnemies() const` |
| [L940](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/SandboxState.cpp:940>) | `const EnemyDefLibrary& SandboxState::GetEnemyLib() const` |
| [L945](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/SandboxState.cpp:945>) | `EnemyDefLibrary& SandboxState::GetEnemyLibMutable()` |
| [L950](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/SandboxState.cpp:950>) | `uint64_t SandboxState::DebugHash() const` |

## Source/Game/SandboxState.h

[SandboxState.h](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/SandboxState.h>) · 132줄

| 위치 | 선언·함수 |
|---|---|
| [L21](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/SandboxState.h:21>) | `struct SandboxConfig` |
| [L26](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/SandboxState.h:26>) | `Aabb             cameraBounds    = Aabb::FromMinMax({ 0.f, 0.f }, { 3520.f, 448.f });` |
| [L44](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/SandboxState.h:44>) | `class SandboxState : public IGameState` |
| [L47](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/SandboxState.h:47>) | `SandboxState();` |
| [L48](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/SandboxState.h:48>) | `explicit SandboxState(SandboxConfig _config);` |
| [L49](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/SandboxState.h:49>) | `void                                  OnEnter() override;` |
| [L50](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/SandboxState.h:50>) | `void                                  OnExit() override;` |
| [L51](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/SandboxState.h:51>) | `void                                  Tick(const InputHistory& _input) override;` |
| [L52](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/SandboxState.h:52>) | `void                                  Render(float _alpha) override;` |
| [L53](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/SandboxState.h:53>) | `[[nodiscard]] PlayerController&       GetPlayer();` |
| [L54](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/SandboxState.h:54>) | `[[nodiscard]] const PlayerController& GetPlayer() const;` |
| [L55](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/SandboxState.h:55>) | `[[nodiscard]] TrainingTargets&        GetTargets();` |
| [L56](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/SandboxState.h:56>) | `[[nodiscard]] ProjectileSystem&       GetProjectiles();` |
| [L57](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/SandboxState.h:57>) | `[[nodiscard]] CombatSystem&           GetCombat();` |
| [L58](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/SandboxState.h:58>) | `[[nodiscard]] Tuning&                 GetTuning();` |
| [L59](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/SandboxState.h:59>) | `[[nodiscard]] const Camera&           GetCamera() const;` |
| [L60](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/SandboxState.h:60>) | `[[nodiscard]] PlayerRig&              GetRig();` |
| [L61](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/SandboxState.h:61>) | `[[nodiscard]] const PlayerRig&        GetRig() const;` |
| [L62](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/SandboxState.h:62>) | `[[nodiscard]] uint64_t                GetTickCount() const;` |
| [L63](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/SandboxState.h:63>) | `[[nodiscard]] EnemyWorld&             GetEnemies();` |
| [L64](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/SandboxState.h:64>) | `[[nodiscard]] const EnemyWorld&       GetEnemies() const;` |
| [L65](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/SandboxState.h:65>) | `[[nodiscard]] const EnemyDefLibrary&  GetEnemyLib() const;` |
| [L66](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/SandboxState.h:66>) | `[[nodiscard]] EnemyDefLibrary&        GetEnemyLibMutable();` |
| [L67](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/SandboxState.h:67>) | `uint32_t                              AddTarget(eDummyKind _kind, VECTOR2 _feet, bool _bShooter = false);` |
| [L68](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/SandboxState.h:68>) | `EnemyHandle                           SpawnEnemy(StringView _defName, VECTOR2 _feet, const SpawnParams& _params = {});` |
| [L69](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/SandboxState.h:69>) | `void                                  RestartGame();` |
| [L70](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/SandboxState.h:70>) | `[[nodiscard]] bool                    ReloadEnemyDefs(const FilePath& _assetRoot);` |
| [L71](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/SandboxState.h:71>) | `[[nodiscard]] bool                    ReloadEnemyDefsKeep(const FilePath& _assetRoot);` |
| [L72](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/SandboxState.h:72>) | `[[nodiscard]] const GroundMap&        GetGround() const;` |
| [L73](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/SandboxState.h:73>) | `[[nodiscard]] uint64_t                HashState() const;` |
| [L74](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/SandboxState.h:74>) | `[[nodiscard]] uint64_t                DebugHash() const override;` |
| [L77](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/SandboxState.h:77>) | `void                       BuildWorld_();` |
| [L78](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/SandboxState.h:78>) | `void                       AddDefaultTargets_();` |
| [L79](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/SandboxState.h:79>) | `void                       UpdateWalls_();` |
| [L80](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/SandboxState.h:80>) | `void                       UpdatePropCollision_();` |
| [L81](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/SandboxState.h:81>) | `[[nodiscard]] PhysicsWorld MakePhysicsWorld_() const;` |
| [L82](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/SandboxState.h:82>) | `[[nodiscard]] VECTOR2      GetRespawnFeet_() const;` |
| [L83](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/SandboxState.h:83>) | `[[nodiscard]] PhysicsWorld MakeEnemyPhysicsWorld_() const;` |
| [L84](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/SandboxState.h:84>) | `void                       SpawnDebugEnemy_(int _slot, float _aheadPx = 180.f);` |
| [L85](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/SandboxState.h:85>) | `void                       TickBusLab_();` |
| [L86](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/SandboxState.h:86>) | `[[nodiscard]] bool         LoadEnemyDefs_(const FilePath& _assetRoot, EnemyDefLibrary& _out) const;` |
| [L87](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/SandboxState.h:87>) | `void                       DrawWorld_(VECTOR2 _cameraPos, float _alpha);` |
| [L88](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/SandboxState.h:88>) | `void                       DrawDebugBoxes_(Renderer& _renderer, VECTOR2 _cameraPos) const;` |
| [L89](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/SandboxState.h:89>) | `void                       DrawHud_();` |
| [L90](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/SandboxState.h:90>) | `void                       RequestSounds_(const Vector<FxRequest>& _fx);` |

## Source/Game/SimRunner.cpp

[SimRunner.cpp](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/SimRunner.cpp>) · 93줄

| 위치 | 선언·함수 |
|---|---|
| [L23](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/SimRunner.cpp:23>) | `void SimRunner::Step( const InputFrame _input)` |
| [L42](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/SimRunner.cpp:42>) | `void SimRunner::Run( const Span<const InputFrame> _inputs)` |
| [L51](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/SimRunner.cpp:51>) | `void SimRunner::Render( const float _alpha)` |
| [L60](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/SimRunner.cpp:60>) | `IGameState* SimRunner::GetState() const` |
| [L65](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/SimRunner.cpp:65>) | `uint64_t SimRunner::GetTickCount() const` |
| [L70](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/SimRunner.cpp:70>) | `const InputHistory& SimRunner::GetHistory() const` |
| [L75](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/SimRunner.cpp:75>) | `bool SimRunner::IsFinished() const` |
| [L80](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/SimRunner.cpp:80>) | `void SimRunner::SwitchState( Scoped<IGameState> _pNext)` |

## Source/Game/SimRunner.h

[SimRunner.h](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/SimRunner.h>) · 25줄

| 위치 | 선언·함수 |
|---|---|
| [L5](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/SimRunner.h:5>) | `class SimRunner` |
| [L8](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/SimRunner.h:8>) | `explicit SimRunner(Scoped<IGameState> _pState);` |
| [L9](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/SimRunner.h:9>) | `~SimRunner();` |
| [L10](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/SimRunner.h:10>) | `SimRunner(const SimRunner&)                                   = delete;` |
| [L11](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/SimRunner.h:11>) | `SimRunner&                        operator=(const SimRunner&) = delete;` |
| [L12](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/SimRunner.h:12>) | `void                              Step(InputFrame _input);` |
| [L13](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/SimRunner.h:13>) | `void                              Run(Span<const InputFrame> _inputs);` |
| [L14](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/SimRunner.h:14>) | `void                              Render(float _alpha);` |
| [L15](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/SimRunner.h:15>) | `void                              SwitchState(Scoped<IGameState> _pNext);` |
| [L16](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/SimRunner.h:16>) | `[[nodiscard]] IGameState*         GetState() const;` |
| [L17](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/SimRunner.h:17>) | `[[nodiscard]] uint64_t            GetTickCount() const;` |
| [L18](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/SimRunner.h:18>) | `[[nodiscard]] const InputHistory& GetHistory() const;` |
| [L19](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/SimRunner.h:19>) | `[[nodiscard]] bool                IsFinished() const;` |

## Source/Game/Stage/StageData.cpp

[StageData.cpp](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Stage/StageData.cpp>) · 2045줄

| 위치 | 선언·함수 |
|---|---|
| [L13](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Stage/StageData.cpp:13>) | `bool CameraAnchor::IsRandomX() const` |
| [L18](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Stage/StageData.cpp:18>) | `int SpawnSpec::GetCount() const` |
| [L23](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Stage/StageData.cpp:23>) | `float BoundsCurve::EvaluateTop( const float _playerX) const` |
| [L36](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Stage/StageData.cpp:36>) | `else if (_playerX < top.back().x)` |
| [L53](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Stage/StageData.cpp:53>) | `Aabb BoundsCurve::EvaluateBounds( const float _playerX) const` |
| [L60](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Stage/StageData.cpp:60>) | `int StageData::FindSection( const StringView _id) const` |
| [L73](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Stage/StageData.cpp:73>) | `const NamedRect* StageData::FindRect( const StringView _name) const` |
| [L86](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Stage/StageData.cpp:86>) | `const BackdropMode* StageData::FindBackdrop( const StringView _id) const` |
| [L99](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Stage/StageData.cpp:99>) | `bool StageData::HasGroundMap( const StringView _name) const` |
| [L114](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Stage/StageData.cpp:114>) | `[[nodiscard]] int CountSpawnsIn_( const Vector<StageAction>& _actions)` |
| [L132](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Stage/StageData.cpp:132>) | `[[nodiscard]] int CountWaveEntriesIn_( const Vector<StageAction>& _actions)` |
| [L153](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Stage/StageData.cpp:153>) | `[[nodiscard]] int CountGatesIn_( const Vector<StageAction>& _actions)` |
| [L169](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Stage/StageData.cpp:169>) | `int StageData::CountSpawns() const` |
| [L179](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Stage/StageData.cpp:179>) | `int StageData::CountWaveEntries() const` |
| [L189](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Stage/StageData.cpp:189>) | `int StageData::CountGates() const` |
| [L199](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Stage/StageData.cpp:199>) | `const char* GetActionTypeName( const eActionType _type)` |
| [L237](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Stage/StageData.cpp:237>) | `[[nodiscard]] bool LookupEnum_( const ARRAY<EnumEntry<E>, N>& _table, const StringView _name, E& _out)` |
| [L254](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Stage/StageData.cpp:254>) | `[[nodiscard]] String ListEnum_( const ARRAY<EnumEntry<E>, N>& _table)` |
| [L341](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Stage/StageData.cpp:341>) | `[[nodiscard]] bool IsCreature_( const eEntityKind _kind)` |
| [L347](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Stage/StageData.cpp:347>) | `[[nodiscard]] eOrigin DefaultOrigin_( const eEntityKind _kind)` |
| [L353](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Stage/StageData.cpp:353>) | `[[nodiscard]] eWhen DefaultWhen_( const StageAction& _action)` |
| [L377](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Stage/StageData.cpp:377>) | `[[nodiscard]] ParamValue ToParam_( const JsonReader& _json)` |
| [L417](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Stage/StageData.cpp:417>) | `explicit StageParser( StageData& _out) : m_out(_out)` |
| [L506](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Stage/StageData.cpp:506>) | `bool GetEnum_( const JsonReader& _obj, const String& _where, const StringView _key, const bool _bRequired, const ARRAY<EnumEntry<E>, N>& _table, E& _out, bool* const _pFound = nullptr)` |
| [L583](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Stage/StageData.cpp:583>) | `const String& StageParser::GetError() const` |
| [L588](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Stage/StageData.cpp:588>) | `bool StageParser::Fail_( const String& _where, const String& _message)` |
| [L599](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Stage/StageData.cpp:599>) | `bool StageParser::GetField_( const JsonReader& _obj, const String& _where, const StringView _key, const bool _bRequired, std::optional<JsonReader>& _out)` |
| [L615](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Stage/StageData.cpp:615>) | `bool StageParser::GetNumber_( const JsonReader& _obj, const String& _where, const StringView _key, const bool _bRequired, double& _out, bool* const _pFound)` |
| [L627](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Stage/StageData.cpp:627>) | `bool StageParser::GetFloat_( const JsonReader& _obj, const String& _where, const StringView _key, const bool _bRequired, float& _out, bool* const _pFound)` |
| [L644](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Stage/StageData.cpp:644>) | `bool StageParser::GetInt_( const JsonReader& _obj, const String& _where, const StringView _key, const bool _bRequired, int& _out, bool* const _pFound)` |
| [L673](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Stage/StageData.cpp:673>) | `bool StageParser::GetBool_( const JsonReader& _obj, const String& _where, const StringView _key, const bool _bRequired, bool& _out, bool* const _pFound)` |
| [L685](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Stage/StageData.cpp:685>) | `bool StageParser::GetString_( const JsonReader& _obj, const String& _where, const StringView _key, const bool _bRequired, String& _out, bool* const _pFound)` |
| [L697](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Stage/StageData.cpp:697>) | `bool StageParser::GetVec2_( const JsonReader& _obj, const String& _where, const StringView _key, const bool _bRequired, VECTOR2& _out, bool* const _pFound)` |
| [L709](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Stage/StageData.cpp:709>) | `bool StageParser::GetObject_( const JsonReader& _obj, const String& _where, const StringView _key, const bool _bRequired, std::optional<JsonReader>& _out)` |
| [L727](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Stage/StageData.cpp:727>) | `bool StageParser::GetArray_( const JsonReader& _obj, const String& _where, const StringView _key, const bool _bRequired, std::optional<JsonReader>& _out)` |
| [L745](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Stage/StageData.cpp:745>) | `bool StageParser::ExpectObject_( const JsonReader& _json, const String& _where)` |
| [L756](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Stage/StageData.cpp:756>) | `bool StageParser::CheckKeys_( const JsonReader& _obj, const String& _where, const std::initializer_list<StringView> _allowed)` |
| [L773](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Stage/StageData.cpp:773>) | `bool StageParser::Parse( const JsonReader& _root)` |
| [L798](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Stage/StageData.cpp:798>) | `bool StageParser::ParseWorld_( const JsonReader& _root)` |
| [L832](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Stage/StageData.cpp:832>) | `bool StageParser::ParseBgm_( const JsonReader& _root)` |
| [L844](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Stage/StageData.cpp:844>) | `bool StageParser::ParseGround_( const JsonReader& _root)` |
| [L878](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Stage/StageData.cpp:878>) | `bool StageParser::ParseBackdrop_( const JsonReader& _root)` |
| [L924](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Stage/StageData.cpp:924>) | `bool StageParser::ParseRectArray_( const JsonReader& _array, const String& _where, const StringView _key, Aabb& _out)` |
| [L952](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Stage/StageData.cpp:952>) | `bool StageParser::ParseRects_( const JsonReader& _root)` |
| [L987](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Stage/StageData.cpp:987>) | `bool StageParser::ParseRect_( const JsonReader& _obj, const String& _where, const StringView _key, const bool _bAllowCurrent, RectRef& _out)` |
| [L1026](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Stage/StageData.cpp:1026>) | `bool StageParser::ParseCamera_( const JsonReader& _root)` |
| [L1144](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Stage/StageData.cpp:1144>) | `bool StageParser::ParsePlayer_( const JsonReader& _root)` |
| [L1178](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Stage/StageData.cpp:1178>) | `bool StageParser::ParseCameraAnchor_( const JsonReader& _obj, const String& _where, CameraAnchor& _out)` |
| [L1210](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Stage/StageData.cpp:1210>) | `else if (dx->IsArray() && dx->GetSize() == 2)` |
| [L1237](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Stage/StageData.cpp:1237>) | `bool StageParser::ParseSpawn_( const JsonReader& _obj, const String& _where, const eSpawnContext _context, SpawnSpec& _out)` |
| [L1266](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Stage/StageData.cpp:1266>) | `else if (_context == eSpawnContext::WaveEntry)` |
| [L1374](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Stage/StageData.cpp:1374>) | `else if (facing == "right")` |
| [L1433](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Stage/StageData.cpp:1433>) | `else if (HasKey(_obj, "size") \|\| HasKey(_obj, "collisionLayer"))` |
| [L1457](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Stage/StageData.cpp:1457>) | `else if (!_out.anim.empty() \|\| !_out.sprite.empty())` |
| [L1478](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Stage/StageData.cpp:1478>) | `bool StageParser::ParseActionList_( const JsonReader& _array, const String& _where, Vector<StageAction>& _out)` |
| [L1506](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Stage/StageData.cpp:1506>) | `bool StageParser::ParseAction_( const JsonReader& _obj, const String& _where, StageAction& _out)` |
| [L1783](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Stage/StageData.cpp:1783>) | `bool StageParser::ParseSections_( const JsonReader& _root)` |
| [L1837](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Stage/StageData.cpp:1837>) | `bool StageParser::AddSpawnId_( const SpawnSpec& _spawn, const String& _at)` |
| [L1856](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Stage/StageData.cpp:1856>) | `bool StageParser::HasSpawnId_( const String& _id) const` |
| [L1869](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Stage/StageData.cpp:1869>) | `bool StageParser::CollectIds_( const Vector<StageAction>& _actions, const String& _where)` |
| [L1885](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Stage/StageData.cpp:1885>) | `else if (action.type == eActionType::SpawnWave)` |
| [L1896](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Stage/StageData.cpp:1896>) | `else if (action.type == eActionType::Gate)` |
| [L1915](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Stage/StageData.cpp:1915>) | `bool StageParser::CheckRefs_( const Vector<StageAction>& _actions, const String& _where)` |
| [L1937](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Stage/StageData.cpp:1937>) | `else if (action.type == eActionType::Command && !HasSpawnId_(action.target))` |
| [L1945](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Stage/StageData.cpp:1945>) | `bool StageParser::Validate_()` |
| [L1985](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Stage/StageData.cpp:1985>) | `[[nodiscard]] Result<StageData> MakeFailure_( String* const _pMessage, const String& _message, const eError _code)` |
| [L1998](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Stage/StageData.cpp:1998>) | `Result<StageData> LoadStageFromJson( const StringView _json, String* const _pMessage)` |
| [L2022](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Stage/StageData.cpp:2022>) | `Result<StageData> LoadStageFromFile( const FilePath& _path, String* const _pMessage)` |
| [L2034](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Stage/StageData.cpp:2034>) | `uint32_t LayerBit( const eStageLayer _layer)` |
| [L2040](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Stage/StageData.cpp:2040>) | `bool ShouldRunAction( const eWhen _when, const bool _bSilent)` |

## Source/Game/Stage/StageData.h

[StageData.h](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Stage/StageData.h>) · 336줄

| 위치 | 선언·함수 |
|---|---|
| [L12](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Stage/StageData.h:12>) | `enum class eEntityKind` |
| [L22](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Stage/StageData.h:22>) | `enum class eOrigin` |
| [L28](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Stage/StageData.h:28>) | `enum class eStageLayer` |
| [L41](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Stage/StageData.h:41>) | `[[nodiscard]] uint32_t LayerBit(eStageLayer _layer);` |
| [L43](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Stage/StageData.h:43>) | `enum class eCollisionLayer` |
| [L49](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Stage/StageData.h:49>) | `enum class eAnchorX` |
| [L56](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Stage/StageData.h:56>) | `enum class eAnchorY` |
| [L63](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Stage/StageData.h:63>) | `enum class eActionType` |
| [L84](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Stage/StageData.h:84>) | `enum class eWhen` |
| [L91](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Stage/StageData.h:91>) | `enum class eGateUntil` |
| [L97](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Stage/StageData.h:97>) | `enum class eFadeTarget` |
| [L103](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Stage/StageData.h:103>) | `struct CameraAnchor` |
| [L111](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Stage/StageData.h:111>) | `[[nodiscard]] bool IsRandomX() const;` |
| [L114](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Stage/StageData.h:114>) | `struct RepeatSpec` |
| [L121](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Stage/StageData.h:121>) | `struct AmbushSpec` |
| [L129](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Stage/StageData.h:129>) | `struct SpawnSpec` |
| [L170](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Stage/StageData.h:170>) | `ParamValue        params = ParamValue::MakeObject();` |
| [L171](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Stage/StageData.h:171>) | `[[nodiscard]] int GetCount() const;` |
| [L174](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Stage/StageData.h:174>) | `struct WaveEntry` |
| [L182](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Stage/StageData.h:182>) | `struct RectRef` |
| [L189](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Stage/StageData.h:189>) | `struct StageAction` |
| [L227](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Stage/StageData.h:227>) | `struct StageSection` |
| [L234](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Stage/StageData.h:234>) | `struct BackdropMode` |
| [L243](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Stage/StageData.h:243>) | `struct NamedRect` |
| [L249](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Stage/StageData.h:249>) | `struct BoundsCurve` |
| [L262](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Stage/StageData.h:262>) | `[[nodiscard]] float EvaluateTop(float _playerX) const;` |
| [L263](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Stage/StageData.h:263>) | `[[nodiscard]] Aabb  EvaluateBounds(float _playerX) const;` |
| [L266](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Stage/StageData.h:266>) | `struct CameraConfig` |
| [L275](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Stage/StageData.h:275>) | `struct PlayerStart` |
| [L287](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Stage/StageData.h:287>) | `struct EndlessTail` |
| [L295](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Stage/StageData.h:295>) | `struct StageData` |
| [L321](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Stage/StageData.h:321>) | `[[nodiscard]] int                 FindSection(StringView _id) const;` |
| [L322](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Stage/StageData.h:322>) | `[[nodiscard]] const NamedRect*    FindRect(StringView _name) const;` |
| [L323](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Stage/StageData.h:323>) | `[[nodiscard]] const BackdropMode* FindBackdrop(StringView _id) const;` |
| [L324](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Stage/StageData.h:324>) | `[[nodiscard]] bool                HasGroundMap(StringView _name) const;` |
| [L325](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Stage/StageData.h:325>) | `[[nodiscard]] int                 CountSpawns() const;` |
| [L326](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Stage/StageData.h:326>) | `[[nodiscard]] int                 CountWaveEntries() const;` |
| [L327](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Stage/StageData.h:327>) | `[[nodiscard]] int                 CountGates() const;` |
| [L330](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Stage/StageData.h:330>) | `[[nodiscard]] Result<StageData> LoadStageFromJson(StringView _json, String* _pMessage = nullptr);` |
| [L332](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Stage/StageData.h:332>) | `[[nodiscard]] Result<StageData> LoadStageFromFile(const FilePath& _path, String* _pMessage = nullptr);` |
| [L334](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Stage/StageData.h:334>) | `[[nodiscard]] bool ShouldRunAction(eWhen _when, bool _bSilent);` |
| [L336](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Stage/StageData.h:336>) | `[[nodiscard]] const char* GetActionTypeName(eActionType _type);` |

## Source/Game/Stage/StageHost.cpp

[StageHost.cpp](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Stage/StageHost.cpp>) · 5줄

| 위치 | 선언·함수 |
|---|---|
| [L3](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Stage/StageHost.cpp:3>) | `void IStageHost::OnSectionEntered(int, StringView)` |

## Source/Game/Stage/StageHost.h

[StageHost.h](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Stage/StageHost.h>) · 85줄

| 위치 | 선언·함수 |
|---|---|
| [L11](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Stage/StageHost.h:11>) | `enum class ePositionMode` |
| [L17](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Stage/StageHost.h:17>) | `struct SpawnRequest` |
| [L53](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Stage/StageHost.h:53>) | `const ParamValue* pParams = &ParamValue::EmptyObject();` |
| [L56](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Stage/StageHost.h:56>) | `struct EntityFilter` |
| [L61](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Stage/StageHost.h:61>) | `class IStageHost` |
| [L64](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Stage/StageHost.h:64>) | `IStageHost()                                                                                           = default;` |
| [L65](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Stage/StageHost.h:65>) | `virtual ~IStageHost()                                                                                  = default;` |
| [L66](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Stage/StageHost.h:66>) | `virtual SpawnId                            SpawnEntity(const SpawnRequest& _request)                   = 0;` |
| [L67](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Stage/StageHost.h:67>) | `[[nodiscard]] virtual bool                 IsDefeated(SpawnId _id) const                               = 0;` |
| [L68](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Stage/StageHost.h:68>) | `[[nodiscard]] virtual bool                 IsDestroyFinished(SpawnId _id) const                        = 0;` |
| [L69](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Stage/StageHost.h:69>) | `virtual void                               ClearEntities(const EntityFilter& _filter)                  = 0;` |
| [L70](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Stage/StageHost.h:70>) | `virtual void                               SendCommand(SpawnId _id, StringView _command)               = 0;` |
| [L71](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Stage/StageHost.h:71>) | `virtual void                               SetGround(StringView _name)                                 = 0;` |
| [L72](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Stage/StageHost.h:72>) | `[[nodiscard]] virtual std::optional<float> GetGroundFindY(float _x, float _fromY) const                = 0;` |
| [L73](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Stage/StageHost.h:73>) | `[[nodiscard]] virtual VECTOR2              PlayerFeet() const                                          = 0;` |
| [L74](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Stage/StageHost.h:74>) | `[[nodiscard]] virtual bool                 IsGameOver() const                                          = 0;` |
| [L75](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Stage/StageHost.h:75>) | `virtual void                               SetPlayerInvincible(float _seconds)                         = 0;` |
| [L76](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Stage/StageHost.h:76>) | `virtual void                               WarpPlayer(VECTOR2 _feet)                                   = 0;` |
| [L77](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Stage/StageHost.h:77>) | `virtual void                               SetBackdrop(StringView _modeId)                             = 0;` |
| [L78](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Stage/StageHost.h:78>) | `virtual void                               ShowBanner(const String& _text, float _seconds)             = 0;` |
| [L79](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Stage/StageHost.h:79>) | `virtual void                               FadeTo(eFadeTarget _target, float _seconds)                 = 0;` |
| [L80](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Stage/StageHost.h:80>) | `virtual void                               PlayBgm(StringView _name, bool _bLoop)                      = 0;` |
| [L81](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Stage/StageHost.h:81>) | `virtual void                               ChangeBgm(StringView _name, bool _bLoop, float _fadeOutSec) = 0;` |
| [L82](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Stage/StageHost.h:82>) | `virtual void                               PlaySfx(StringView _name)                                   = 0;` |
| [L83](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Stage/StageHost.h:83>) | `virtual void                               EndStage(StringView _nextState)                             = 0;` |
| [L84](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Stage/StageHost.h:84>) | `virtual void                               OnSectionEntered(int _index, StringView _id);` |

## Source/Game/Stage/StageMapping.cpp

[StageMapping.cpp](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Stage/StageMapping.cpp>) · 140줄

| 위치 | 선언·함수 |
|---|---|
| [L15](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Stage/StageMapping.cpp:15>) | `[[nodiscard]] VECTOR2 ReadVec2_( const ParamValue& _value, const VECTOR2 _default)` |
| [L26](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Stage/StageMapping.cpp:26>) | `[[nodiscard]] PropBoxOverride ReadBox_( const ParamValue& _value)` |
| [L46](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Stage/StageMapping.cpp:46>) | `PropSpawnParams MakePropSpawnParams( const ParamValue* const _pParams, const int _z)` |
| [L91](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Stage/StageMapping.cpp:91>) | `Aabb MakeColliderBox( const SpawnRequest& _request)` |
| [L103](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Stage/StageMapping.cpp:103>) | `std::optional<GroundMap> LoadGroundMask( const FilePath& _pngPath)` |
| [L136](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Stage/StageMapping.cpp:136>) | `VECTOR2 ItemFeetFromCenter( const VECTOR2 _center)` |

## Source/Game/Stage/StageMapping.h

[StageMapping.h](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Stage/StageMapping.h>) · 17줄

| 위치 | 선언·함수 |
|---|---|
| [L11](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Stage/StageMapping.h:11>) | `[[nodiscard]] PropSpawnParams MakePropSpawnParams(const ParamValue* _pParams, int _z = kDefaultZ);` |
| [L13](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Stage/StageMapping.h:13>) | `[[nodiscard]] Aabb MakeColliderBox(const SpawnRequest& _request);` |
| [L15](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Stage/StageMapping.h:15>) | `[[nodiscard]] std::optional<GroundMap> LoadGroundMask(const FilePath& _pngPath);` |
| [L17](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Stage/StageMapping.h:17>) | `[[nodiscard]] VECTOR2 ItemFeetFromCenter(VECTOR2 _center);` |

## Source/Game/Stage/StageParams.cpp

[StageParams.cpp](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Stage/StageParams.cpp>) · 208줄

| 위치 | 선언·함수 |
|---|---|
| [L3](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Stage/StageParams.cpp:3>) | `ParamValue ParamValue::MakeBool( const bool _bValue)` |
| [L12](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Stage/StageParams.cpp:12>) | `ParamValue ParamValue::MakeNumber( const double _value)` |
| [L21](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Stage/StageParams.cpp:21>) | `ParamValue ParamValue::MakeString( String _value)` |
| [L30](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Stage/StageParams.cpp:30>) | `ParamValue ParamValue::MakeArray()` |
| [L37](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Stage/StageParams.cpp:37>) | `ParamValue ParamValue::MakeObject()` |
| [L44](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Stage/StageParams.cpp:44>) | `void ParamValue::PushBack( ParamValue _value)` |
| [L53](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Stage/StageParams.cpp:53>) | `void ParamValue::Set( String _key, ParamValue _value)` |
| [L72](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Stage/StageParams.cpp:72>) | `bool ParamValue::IsNumber() const` |
| [L77](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Stage/StageParams.cpp:77>) | `bool ParamValue::IsString() const` |
| [L82](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Stage/StageParams.cpp:82>) | `bool ParamValue::IsArray() const` |
| [L87](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Stage/StageParams.cpp:87>) | `bool ParamValue::IsObject() const` |
| [L92](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Stage/StageParams.cpp:92>) | `bool ParamValue::AsBool( const bool _bDefault) const` |
| [L98](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Stage/StageParams.cpp:98>) | `float ParamValue::AsFloat( const float _default) const` |
| [L104](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Stage/StageParams.cpp:104>) | `int ParamValue::AsInt( const int _default) const` |
| [L110](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Stage/StageParams.cpp:110>) | `const String& ParamValue::AsString() const` |
| [L116](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Stage/StageParams.cpp:116>) | `size_t ParamValue::GetSize() const` |
| [L121](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Stage/StageParams.cpp:121>) | `const ParamValue& ParamValue::At( const size_t _index) const` |
| [L131](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Stage/StageParams.cpp:131>) | `const ParamValue* ParamValue::Find( const StringView _key) const` |
| [L148](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Stage/StageParams.cpp:148>) | `bool ParamValue::Has( const StringView _key) const` |
| [L154](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Stage/StageParams.cpp:154>) | `bool ParamValue::GetBool( const StringView _key, const bool _bDefault) const` |
| [L162](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Stage/StageParams.cpp:162>) | `int ParamValue::GetInt( const StringView _key, const int _default) const` |
| [L170](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Stage/StageParams.cpp:170>) | `String ParamValue::GetString( const StringView _key, const StringView _default) const` |
| [L178](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Stage/StageParams.cpp:178>) | `VECTOR2 ParamValue::GetVec2( const StringView _key, const VECTOR2 _default) const` |
| [L187](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Stage/StageParams.cpp:187>) | `bool ParamValue::TryGetVec2( VECTOR2& _out) const` |
| [L198](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Stage/StageParams.cpp:198>) | `const ParamValue& ParamValue::EmptyObject()` |
| [L204](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Stage/StageParams.cpp:204>) | `const ParamValue& ParamValue::NullValue()` |

## Source/Game/Stage/StageParams.h

[StageParams.h](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Stage/StageParams.h>) · 50줄

| 위치 | 선언·함수 |
|---|---|
| [L3](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Stage/StageParams.h:3>) | `class ParamValue` |
| [L6](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Stage/StageParams.h:6>) | `enum class eType` |
| [L16](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Stage/StageParams.h:16>) | `[[nodiscard]] static ParamValue        MakeBool(bool _bValue);` |
| [L17](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Stage/StageParams.h:17>) | `[[nodiscard]] static ParamValue        MakeNumber(double _value);` |
| [L18](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Stage/StageParams.h:18>) | `[[nodiscard]] static ParamValue        MakeString(String _value);` |
| [L19](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Stage/StageParams.h:19>) | `[[nodiscard]] static ParamValue        MakeArray();` |
| [L20](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Stage/StageParams.h:20>) | `[[nodiscard]] static ParamValue        MakeObject();` |
| [L21](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Stage/StageParams.h:21>) | `void                                   PushBack(ParamValue _value);` |
| [L22](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Stage/StageParams.h:22>) | `void                                   Set(String _key, ParamValue _value);` |
| [L23](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Stage/StageParams.h:23>) | `[[nodiscard]] bool                     IsNumber() const;` |
| [L24](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Stage/StageParams.h:24>) | `[[nodiscard]] bool                     IsString() const;` |
| [L25](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Stage/StageParams.h:25>) | `[[nodiscard]] bool                     IsArray() const;` |
| [L26](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Stage/StageParams.h:26>) | `[[nodiscard]] bool                     IsObject() const;` |
| [L27](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Stage/StageParams.h:27>) | `[[nodiscard]] bool                     AsBool(bool _bDefault = false) const;` |
| [L28](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Stage/StageParams.h:28>) | `[[nodiscard]] float                    AsFloat(float _default = 0.f) const;` |
| [L29](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Stage/StageParams.h:29>) | `[[nodiscard]] int                      AsInt(int _default = 0) const;` |
| [L30](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Stage/StageParams.h:30>) | `[[nodiscard]] const String&            AsString() const;` |
| [L31](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Stage/StageParams.h:31>) | `[[nodiscard]] size_t                   GetSize() const;` |
| [L32](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Stage/StageParams.h:32>) | `[[nodiscard]] const ParamValue&        At(size_t _index) const;` |
| [L33](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Stage/StageParams.h:33>) | `[[nodiscard]] const ParamValue*        Find(StringView _key) const;` |
| [L34](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Stage/StageParams.h:34>) | `[[nodiscard]] bool                     Has(StringView _key) const;` |
| [L35](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Stage/StageParams.h:35>) | `[[nodiscard]] bool                     GetBool(StringView _key, bool _bDefault = false) const;` |
| [L36](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Stage/StageParams.h:36>) | `[[nodiscard]] int                      GetInt(StringView _key, int _default = 0) const;` |
| [L37](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Stage/StageParams.h:37>) | `[[nodiscard]] String                   GetString(StringView _key, StringView _default = {}) const;` |
| [L38](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Stage/StageParams.h:38>) | `[[nodiscard]] VECTOR2                  GetVec2(StringView _key, VECTOR2 _default = { 0.f, 0.f }) const;` |
| [L39](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Stage/StageParams.h:39>) | `[[nodiscard]] bool                     TryGetVec2(VECTOR2& _out) const;` |
| [L40](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Stage/StageParams.h:40>) | `[[nodiscard]] static const ParamValue& EmptyObject();` |
| [L41](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Stage/StageParams.h:41>) | `[[nodiscard]] static const ParamValue& NullValue();` |

## Source/Game/Stage/StageRunner.cpp

[StageRunner.cpp](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Stage/StageRunner.cpp>) · 985줄

| 위치 | 선언·함수 |
|---|---|
| [L17](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Stage/StageRunner.cpp:17>) | `void MixU64_( Fnv1a& _hasher, const uint64_t _value)` |
| [L24](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Stage/StageRunner.cpp:24>) | `void MixFloat_( Fnv1a& _hasher, const float _value)` |
| [L46](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Stage/StageRunner.cpp:46>) | `int64_t StageRunner::ToTicks_( const float _seconds)` |
| [L52](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Stage/StageRunner.cpp:52>) | `void StageRunner::Begin()` |
| [L92](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Stage/StageRunner.cpp:92>) | `std::optional<VECTOR2> StageRunner::ResolvePlayerStart() const` |
| [L105](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Stage/StageRunner.cpp:105>) | `else if (start.origin == eOrigin::LegacyCenter)` |
| [L112](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Stage/StageRunner.cpp:112>) | `Aabb StageRunner::CurrentViewRect_() const` |
| [L117](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Stage/StageRunner.cpp:117>) | `const String& StageRunner::GetGroundName() const` |
| [L122](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Stage/StageRunner.cpp:122>) | `const String& StageRunner::GetBackdropMode() const` |
| [L127](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Stage/StageRunner.cpp:127>) | `VECTOR2 StageRunner::GetCameraOffset() const` |
| [L132](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Stage/StageRunner.cpp:132>) | `const Aabb& StageRunner::GetBaseBounds() const` |
| [L137](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Stage/StageRunner.cpp:137>) | `Aabb StageRunner::GetEffectiveBounds() const` |
| [L142](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Stage/StageRunner.cpp:142>) | `int StageRunner::GetLockDepth() const` |
| [L147](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Stage/StageRunner.cpp:147>) | `bool StageRunner::HasPermanentLock() const` |
| [L159](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Stage/StageRunner.cpp:159>) | `bool StageRunner::IsCurveActive( const size_t _curveIndex) const` |
| [L165](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Stage/StageRunner.cpp:165>) | `void StageRunner::ApplyCameraBounds_()` |
| [L170](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Stage/StageRunner.cpp:170>) | `void StageRunner::ApplyCameraLock_()` |
| [L182](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Stage/StageRunner.cpp:182>) | `VECTOR2 StageRunner::ComputeCameraTarget( const VECTOR2 _playerFeet) const` |
| [L188](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Stage/StageRunner.cpp:188>) | `ScreenWalls StageRunner::MakeScreenWalls() const` |
| [L210](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Stage/StageRunner.cpp:210>) | `void StageRunner::Tick()` |
| [L238](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Stage/StageRunner.cpp:238>) | `void StageRunner::AddCameraScroll( const float _dx)` |
| [L255](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Stage/StageRunner.cpp:255>) | `void StageRunner::CancelAbortables_()` |
| [L265](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Stage/StageRunner.cpp:265>) | `void StageRunner::ProcessPending_()` |
| [L315](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Stage/StageRunner.cpp:315>) | `else if (item.pAction)` |
| [L326](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Stage/StageRunner.cpp:326>) | `void StageRunner::ProcessCurves_()` |
| [L349](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Stage/StageRunner.cpp:349>) | `bool StageRunner::IsGateSatisfied_( const GateState& _gate) const` |
| [L372](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Stage/StageRunner.cpp:372>) | `void StageRunner::ProcessGates_()` |
| [L393](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Stage/StageRunner.cpp:393>) | `void StageRunner::FireSection_( const int _index, const bool _bSilent)` |
| [L423](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Stage/StageRunner.cpp:423>) | `void StageRunner::RunActions_( const Vector<StageAction>& _actions, const ExecContext& _ctx)` |
| [L454](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Stage/StageRunner.cpp:454>) | `void StageRunner::Execute_( const StageAction& _action, const ExecContext& _ctx)` |
| [L578](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Stage/StageRunner.cpp:578>) | `void StageRunner::ExecLock_( const StageAction& _action, const ExecContext& _ctx)` |
| [L597](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Stage/StageRunner.cpp:597>) | `void StageRunner::ExecUnlock_()` |
| [L611](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Stage/StageRunner.cpp:611>) | `void StageRunner::ExecClear_( const StageAction& _action)` |
| [L628](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Stage/StageRunner.cpp:628>) | `StageRunner::SpawnRecord* StageRunner::FindRecord_( const StringView _stageId)` |
| [L641](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Stage/StageRunner.cpp:641>) | `const StageRunner::SpawnRecord* StageRunner::FindRecord_( const StringView _stageId) const` |
| [L654](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Stage/StageRunner.cpp:654>) | `Span<const SpawnId> StageRunner::FindSpawnIds( const StringView _stageId) const` |
| [L661](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Stage/StageRunner.cpp:661>) | `bool StageRunner::WasSpawnSkipped( const StringView _stageId) const` |
| [L668](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Stage/StageRunner.cpp:668>) | `void StageRunner::ExecSpawn_( const SpawnSpec& _spec, const ExecContext& _ctx, const bool _bWave)` |
| [L789](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Stage/StageRunner.cpp:789>) | `bool StageRunner::WarpToSection( const StringView _id)` |
| [L796](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Stage/StageRunner.cpp:796>) | `bool StageRunner::WarpToBoss()` |
| [L806](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Stage/StageRunner.cpp:806>) | `bool StageRunner::WarpToX( const float _x)` |
| [L821](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Stage/StageRunner.cpp:821>) | `bool StageRunner::WarpToSection( const int _index)` |
| [L831](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Stage/StageRunner.cpp:831>) | `bool StageRunner::WarpToSection_( const int _index, const float _targetX)` |
| [L870](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Stage/StageRunner.cpp:870>) | `bool StageRunner::IsFinished() const` |
| [L875](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Stage/StageRunner.cpp:875>) | `int64_t StageRunner::GetTick() const` |
| [L880](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Stage/StageRunner.cpp:880>) | `int StageRunner::GetNextSectionIndex() const` |
| [L885](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Stage/StageRunner.cpp:885>) | `std::optional<float> StageRunner::GetNextTriggerX() const` |
| [L894](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Stage/StageRunner.cpp:894>) | `int StageRunner::GetPendingActionCount() const` |
| [L904](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Stage/StageRunner.cpp:904>) | `int StageRunner::GetPendingWaveCount() const` |
| [L914](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Stage/StageRunner.cpp:914>) | `int StageRunner::GetOpenGateCount() const` |
| [L919](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Stage/StageRunner.cpp:919>) | `Vector<String> StageRunner::GetOpenGateIds() const` |
| [L929](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Stage/StageRunner.cpp:929>) | `int StageRunner::GetSpawnedCount() const` |
| [L934](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Stage/StageRunner.cpp:934>) | `int StageRunner::GetSkippedCount() const` |
| [L939](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Stage/StageRunner.cpp:939>) | `uint64_t StageRunner::HashState() const` |

## Source/Game/Stage/StageRunner.h

[StageRunner.h](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Stage/StageRunner.h>) · 144줄

| 위치 | 선언·함수 |
|---|---|
| [L13](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Stage/StageRunner.h:13>) | `struct ScreenWalls` |
| [L21](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Stage/StageRunner.h:21>) | `class StageRunner` |
| [L24](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Stage/StageRunner.h:24>) | `StageRunner(const StageData& _data, IStageHost& _host, Camera& _camera, RngMt19937& _rng);` |
| [L25](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Stage/StageRunner.h:25>) | `StageRunner(const StageRunner&)                               = delete;` |
| [L26](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Stage/StageRunner.h:26>) | `StageRunner& operator=(const StageRunner&)                    = delete;` |
| [L27](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Stage/StageRunner.h:27>) | `StageRunner(StageRunner&&)                                    = delete;` |
| [L28](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Stage/StageRunner.h:28>) | `StageRunner&                         operator=(StageRunner&&) = delete;` |
| [L29](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Stage/StageRunner.h:29>) | `void                                 Begin();` |
| [L30](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Stage/StageRunner.h:30>) | `[[nodiscard]] std::optional<VECTOR2> ResolvePlayerStart() const;` |
| [L31](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Stage/StageRunner.h:31>) | `void                                 Tick();` |
| [L32](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Stage/StageRunner.h:32>) | `void                                 AddCameraScroll(float _dx);` |
| [L33](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Stage/StageRunner.h:33>) | `bool                                 WarpToSection(int _index);` |
| [L34](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Stage/StageRunner.h:34>) | `bool                                 WarpToSection(StringView _id);` |
| [L35](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Stage/StageRunner.h:35>) | `bool                                 WarpToBoss();` |
| [L36](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Stage/StageRunner.h:36>) | `bool                                 WarpToX(float _x);` |
| [L37](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Stage/StageRunner.h:37>) | `[[nodiscard]] VECTOR2                ComputeCameraTarget(VECTOR2 _playerFeet) const;` |
| [L38](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Stage/StageRunner.h:38>) | `[[nodiscard]] ScreenWalls            MakeScreenWalls() const;` |
| [L39](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Stage/StageRunner.h:39>) | `[[nodiscard]] bool                   IsFinished() const;` |
| [L40](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Stage/StageRunner.h:40>) | `[[nodiscard]] int64_t                GetTick() const;` |
| [L41](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Stage/StageRunner.h:41>) | `[[nodiscard]] int                    GetNextSectionIndex() const;` |
| [L42](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Stage/StageRunner.h:42>) | `[[nodiscard]] std::optional<float>   GetNextTriggerX() const;` |
| [L43](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Stage/StageRunner.h:43>) | `[[nodiscard]] int                    GetPendingActionCount() const;` |
| [L44](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Stage/StageRunner.h:44>) | `[[nodiscard]] int                    GetPendingWaveCount() const;` |
| [L45](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Stage/StageRunner.h:45>) | `[[nodiscard]] int                    GetOpenGateCount() const;` |
| [L46](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Stage/StageRunner.h:46>) | `[[nodiscard]] Vector<String>         GetOpenGateIds() const;` |
| [L47](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Stage/StageRunner.h:47>) | `[[nodiscard]] const String&          GetGroundName() const;` |
| [L48](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Stage/StageRunner.h:48>) | `[[nodiscard]] const String&          GetBackdropMode() const;` |
| [L49](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Stage/StageRunner.h:49>) | `[[nodiscard]] VECTOR2                GetCameraOffset() const;` |
| [L50](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Stage/StageRunner.h:50>) | `[[nodiscard]] const Aabb&            GetBaseBounds() const;` |
| [L51](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Stage/StageRunner.h:51>) | `[[nodiscard]] Aabb                   GetEffectiveBounds() const;` |
| [L52](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Stage/StageRunner.h:52>) | `[[nodiscard]] int                    GetLockDepth() const;` |
| [L53](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Stage/StageRunner.h:53>) | `[[nodiscard]] bool                   HasPermanentLock() const;` |
| [L54](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Stage/StageRunner.h:54>) | `[[nodiscard]] bool                   IsCurveActive(size_t _curveIndex) const;` |
| [L55](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Stage/StageRunner.h:55>) | `[[nodiscard]] Span<const SpawnId>    FindSpawnIds(StringView _stageId) const;` |
| [L56](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Stage/StageRunner.h:56>) | `[[nodiscard]] bool                   WasSpawnSkipped(StringView _stageId) const;` |
| [L57](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Stage/StageRunner.h:57>) | `[[nodiscard]] int                    GetSpawnedCount() const;` |
| [L58](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Stage/StageRunner.h:58>) | `[[nodiscard]] int                    GetSkippedCount() const;` |
| [L59](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Stage/StageRunner.h:59>) | `[[nodiscard]] uint64_t               HashState() const;` |
| [L62](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Stage/StageRunner.h:62>) | `struct PendingItem` |
| [L73](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Stage/StageRunner.h:73>) | `struct GateState` |
| [L80](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Stage/StageRunner.h:80>) | `struct LockEntry` |
| [L86](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Stage/StageRunner.h:86>) | `struct SpawnRecord` |
| [L92](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Stage/StageRunner.h:92>) | `struct ExecContext` |
| [L99](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Stage/StageRunner.h:99>) | `[[nodiscard]] static int64_t ToTicks_(float _seconds);` |
| [L100](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Stage/StageRunner.h:100>) | `void                         FireSection_(int _index, bool _bSilent);` |
| [L101](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Stage/StageRunner.h:101>) | `bool                         WarpToSection_(int _index, float _targetX);` |
| [L102](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Stage/StageRunner.h:102>) | `void                         RunActions_(const Vector<StageAction>& _actions, const ExecContext& _ctx);` |
| [L103](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Stage/StageRunner.h:103>) | `void                         Execute_(const StageAction& _action, const ExecContext& _ctx);` |
| [L104](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Stage/StageRunner.h:104>) | `void                         ExecSpawn_(const SpawnSpec& _spec, const ExecContext& _ctx, bool _bWave);` |
| [L105](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Stage/StageRunner.h:105>) | `void                         ExecLock_(const StageAction& _action, const ExecContext& _ctx);` |
| [L106](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Stage/StageRunner.h:106>) | `void                         ExecUnlock_();` |
| [L107](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Stage/StageRunner.h:107>) | `void                         ExecClear_(const StageAction& _action);` |
| [L108](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Stage/StageRunner.h:108>) | `void                         ProcessPending_();` |
| [L109](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Stage/StageRunner.h:109>) | `void                         ProcessCurves_();` |
| [L110](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Stage/StageRunner.h:110>) | `void                         ProcessGates_();` |
| [L111](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Stage/StageRunner.h:111>) | `void                         CancelAbortables_();` |
| [L112](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Stage/StageRunner.h:112>) | `[[nodiscard]] bool           IsGateSatisfied_(const GateState& _gate) const;` |
| [L113](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Stage/StageRunner.h:113>) | `void                         ApplyCameraBounds_();` |
| [L114](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Stage/StageRunner.h:114>) | `void                         ApplyCameraLock_();` |
| [L115](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Stage/StageRunner.h:115>) | `[[nodiscard]] Aabb           CurrentViewRect_() const;` |
| [L116](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Stage/StageRunner.h:116>) | `SpawnRecord*                 FindRecord_(StringView _stageId);` |
| [L117](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Stage/StageRunner.h:117>) | `const SpawnRecord*           FindRecord_(StringView _stageId) const;` |

## Source/Game/Stage/StageState.cpp

[StageState.cpp](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Stage/StageState.cpp>) · 1240줄

| 위치 | 선언·함수 |
|---|---|
| [L31](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Stage/StageState.cpp:31>) | `[[nodiscard]] RGBA GetPlaceholderColor_( const PlaceholderEntity& _entity)` |
| [L45](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Stage/StageState.cpp:45>) | `[[nodiscard]] String JoinIds_( const Vector<String>& _ids)` |
| [L69](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Stage/StageState.cpp:69>) | `void StageState::OnEnter()` |
| [L82](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Stage/StageState.cpp:82>) | `void StageState::OnExit()` |
| [L87](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Stage/StageState.cpp:87>) | `const InputHistory& StageState::MakeBotInput_()` |
| [L112](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Stage/StageState.cpp:112>) | `else if (Abs(bestDx) > 14.f)` |
| [L126](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Stage/StageState.cpp:126>) | `void StageState::Load_()` |
| [L274](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Stage/StageState.cpp:274>) | `void StageState::Restart_()` |
| [L310](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Stage/StageState.cpp:310>) | `void StageState::Tick( const InputHistory& _input)` |
| [L487](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Stage/StageState.cpp:487>) | `else if (request.kind == eFxKind::RocketExplosion)` |
| [L491](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Stage/StageState.cpp:491>) | `else if (request.kind == eFxKind::RocketSmoke)` |
| [L523](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Stage/StageState.cpp:523>) | `else if (m_player.IsGameOver())` |
| [L530](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Stage/StageState.cpp:530>) | `void StageState::StartFlow_( const eEndKind _kind)` |
| [L542](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Stage/StageState.cpp:542>) | `void StageState::HandleDebugWarp_()` |
| [L564](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Stage/StageState.cpp:564>) | `else if (request == 2)` |
| [L568](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Stage/StageState.cpp:568>) | `else if (request == 3)` |
| [L574](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Stage/StageState.cpp:574>) | `bool StageState::IsFlowing_() const` |
| [L579](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Stage/StageState.cpp:579>) | `void StageState::UpdateWalls_()` |
| [L630](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Stage/StageState.cpp:630>) | `PhysicsWorld StageState::MakePhysicsWorld_() const` |
| [L639](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Stage/StageState.cpp:639>) | `PhysicsWorld StageState::MakeEnemyPhysicsWorld_() const` |
| [L648](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Stage/StageState.cpp:648>) | `VECTOR2 StageState::GetRespawnFeet_() const` |
| [L654](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Stage/StageState.cpp:654>) | `void StageState::ConsumeEnemyEvents_()` |
| [L690](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Stage/StageState.cpp:690>) | `void StageState::ConsumePropEvents_()` |
| [L715](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Stage/StageState.cpp:715>) | `const GroundMap* StageState::FindPatchMask_( const String& _sprite)` |
| [L750](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Stage/StageState.cpp:750>) | `void StageState::ApplyGroundPatch_( const String& _sprite, const VECTOR2 _topLeft)` |
| [L762](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Stage/StageState.cpp:762>) | `void StageState::RequestSounds_( const Vector<FxRequest>& _fx)` |
| [L791](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Stage/StageState.cpp:791>) | `else if (request.kind == eFxKind::RocketExplosion)` |
| [L806](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Stage/StageState.cpp:806>) | `uint64_t StageState::GetTickCount() const` |
| [L811](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Stage/StageState.cpp:811>) | `uint64_t StageState::HashState() const` |
| [L830](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Stage/StageState.cpp:830>) | `bool StageState::IsFinished() const` |
| [L847](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Stage/StageState.cpp:847>) | `Scoped<IGameState> StageState::TakeNextState()` |
| [L862](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Stage/StageState.cpp:862>) | `void StageState::Render( const float _alpha)` |
| [L885](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Stage/StageState.cpp:885>) | `void StageState::DrawBackdrop_( const VECTOR2 _cameraPos)` |
| [L942](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Stage/StageState.cpp:942>) | `void StageState::DrawWorld_( const VECTOR2 _cameraPos, const float _alpha)` |
| [L1001](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Stage/StageState.cpp:1001>) | `else if (m_player.IsAlive())` |
| [L1038](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Stage/StageState.cpp:1038>) | `void StageState::DrawDebugBoxes_( Renderer& _renderer, const VECTOR2 _cameraPos) const` |
| [L1104](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Stage/StageState.cpp:1104>) | `HudModel StageState::BuildHudModel() const` |
| [L1112](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Stage/StageState.cpp:1112>) | `void StageState::DrawHud_( const VECTOR2)` |
| [L1172](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Stage/StageState.cpp:1172>) | `bool StageState::IsLoaded() const` |
| [L1177](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Stage/StageState.cpp:1177>) | `const StageData& StageState::GetData() const` |
| [L1182](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Stage/StageState.cpp:1182>) | `const StageRunner& StageState::GetRunner() const` |
| [L1187](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Stage/StageState.cpp:1187>) | `const StageWorldHost& StageState::GetHost() const` |
| [L1192](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Stage/StageState.cpp:1192>) | `PlayerController& StageState::GetPlayer()` |
| [L1197](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Stage/StageState.cpp:1197>) | `const PlayerController& StageState::GetPlayer() const` |
| [L1202](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Stage/StageState.cpp:1202>) | `const Camera& StageState::GetCamera() const` |
| [L1207](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Stage/StageState.cpp:1207>) | `const CombatSystem& StageState::GetCombat() const` |
| [L1212](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Stage/StageState.cpp:1212>) | `const ProjectileSystem& StageState::GetProjectiles() const` |
| [L1217](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Stage/StageState.cpp:1217>) | `const EnemyWorld& StageState::GetEnemies() const` |
| [L1222](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Stage/StageState.cpp:1222>) | `EnemyWorld& StageState::GetEnemies()` |
| [L1227](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Stage/StageState.cpp:1227>) | `const PropWorld& StageState::GetProps() const` |
| [L1232](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Stage/StageState.cpp:1232>) | `PropWorld& StageState::GetProps()` |
| [L1237](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Stage/StageState.cpp:1237>) | `uint64_t StageState::DebugHash() const` |

## Source/Game/Stage/StageState.h

[StageState.h](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Stage/StageState.h>) · 162줄

| 위치 | 선언·함수 |
|---|---|
| [L29](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Stage/StageState.h:29>) | `struct StageStateConfig` |
| [L38](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Stage/StageState.h:38>) | `class StageState : public IGameState` |
| [L41](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Stage/StageState.h:41>) | `StageState();` |
| [L42](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Stage/StageState.h:42>) | `explicit StageState(const StageStateConfig& _config);` |
| [L43](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Stage/StageState.h:43>) | `StageState(const StageState&)                                 = delete;` |
| [L44](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Stage/StageState.h:44>) | `StageState& operator=(const StageState&)                      = delete;` |
| [L45](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Stage/StageState.h:45>) | `StageState(StageState&&)                                      = delete;` |
| [L46](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Stage/StageState.h:46>) | `StageState&                           operator=(StageState&&) = delete;` |
| [L47](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Stage/StageState.h:47>) | `void                                  OnEnter() override;` |
| [L48](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Stage/StageState.h:48>) | `void                                  OnExit() override;` |
| [L49](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Stage/StageState.h:49>) | `void                                  Tick(const InputHistory& _input) override;` |
| [L50](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Stage/StageState.h:50>) | `void                                  Render(float _alpha) override;` |
| [L51](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Stage/StageState.h:51>) | `[[nodiscard]] Scoped<IGameState>      TakeNextState() override;` |
| [L52](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Stage/StageState.h:52>) | `[[nodiscard]] bool                    IsFinished() const override;` |
| [L53](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Stage/StageState.h:53>) | `[[nodiscard]] bool                    IsLoaded() const;` |
| [L54](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Stage/StageState.h:54>) | `[[nodiscard]] const StageData&        GetData() const;` |
| [L55](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Stage/StageState.h:55>) | `[[nodiscard]] const StageRunner&      GetRunner() const;` |
| [L56](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Stage/StageState.h:56>) | `[[nodiscard]] const StageWorldHost&   GetHost() const;` |
| [L57](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Stage/StageState.h:57>) | `[[nodiscard]] PlayerController&       GetPlayer();` |
| [L58](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Stage/StageState.h:58>) | `[[nodiscard]] const PlayerController& GetPlayer() const;` |
| [L59](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Stage/StageState.h:59>) | `[[nodiscard]] const Camera&           GetCamera() const;` |
| [L60](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Stage/StageState.h:60>) | `[[nodiscard]] const CombatSystem&     GetCombat() const;` |
| [L61](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Stage/StageState.h:61>) | `[[nodiscard]] const EnemyWorld&       GetEnemies() const;` |
| [L62](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Stage/StageState.h:62>) | `[[nodiscard]] EnemyWorld&             GetEnemies();` |
| [L63](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Stage/StageState.h:63>) | `[[nodiscard]] const PropWorld&        GetProps() const;` |
| [L64](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Stage/StageState.h:64>) | `[[nodiscard]] PropWorld&              GetProps();` |
| [L65](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Stage/StageState.h:65>) | `[[nodiscard]] const ProjectileSystem& GetProjectiles() const;` |
| [L66](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Stage/StageState.h:66>) | `[[nodiscard]] uint64_t                GetTickCount() const;` |
| [L67](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Stage/StageState.h:67>) | `[[nodiscard]] uint64_t                HashState() const;` |
| [L68](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Stage/StageState.h:68>) | `[[nodiscard]] uint64_t                DebugHash() const override;` |
| [L69](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Stage/StageState.h:69>) | `[[nodiscard]] HudModel                BuildHudModel() const;` |
| [L72](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Stage/StageState.h:72>) | `enum class eEndKind` |
| [L79](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Stage/StageState.h:79>) | `struct PatchMask` |
| [L86](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Stage/StageState.h:86>) | `[[nodiscard]] const InputHistory& MakeBotInput_();` |
| [L87](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Stage/StageState.h:87>) | `void                              Load_();` |
| [L88](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Stage/StageState.h:88>) | `void                              Restart_();` |
| [L89](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Stage/StageState.h:89>) | `void                              HandleDebugWarp_();` |
| [L90](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Stage/StageState.h:90>) | `void                              StartFlow_(eEndKind _kind);` |
| [L91](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Stage/StageState.h:91>) | `[[nodiscard]] bool                IsFlowing_() const;` |
| [L92](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Stage/StageState.h:92>) | `void                              UpdateWalls_();` |
| [L93](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Stage/StageState.h:93>) | `[[nodiscard]] PhysicsWorld        MakePhysicsWorld_() const;` |
| [L94](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Stage/StageState.h:94>) | `[[nodiscard]] PhysicsWorld        MakeEnemyPhysicsWorld_() const;` |
| [L95](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Stage/StageState.h:95>) | `[[nodiscard]] VECTOR2             GetRespawnFeet_() const;` |
| [L96](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Stage/StageState.h:96>) | `void                              ConsumeEnemyEvents_();` |
| [L97](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Stage/StageState.h:97>) | `void                              ConsumePropEvents_();` |
| [L98](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Stage/StageState.h:98>) | `void                              ApplyGroundPatch_(const String& _sprite, VECTOR2 _topLeft);` |
| [L99](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Stage/StageState.h:99>) | `[[nodiscard]] const GroundMap*    FindPatchMask_(const String& _sprite);` |
| [L100](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Stage/StageState.h:100>) | `void                              RequestSounds_(const Vector<FxRequest>& _fx);` |
| [L101](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Stage/StageState.h:101>) | `void                              DrawBackdrop_(VECTOR2 _cameraPos);` |
| [L102](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Stage/StageState.h:102>) | `void                              DrawWorld_(VECTOR2 _cameraPos, float _alpha);` |
| [L103](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Stage/StageState.h:103>) | `void                              DrawDebugBoxes_(Renderer& _renderer, VECTOR2 _cameraPos) const;` |
| [L104](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Stage/StageState.h:104>) | `void                              DrawHud_(VECTOR2 _cameraPos);` |

## Source/Game/Stage/StageWorldHost.cpp

[StageWorldHost.cpp](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Stage/StageWorldHost.cpp>) · 704줄

| 위치 | 선언·함수 |
|---|---|
| [L22](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Stage/StageWorldHost.cpp:22>) | `[[nodiscard]] int SecondsToTicks_( const float _seconds)` |
| [L28](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Stage/StageWorldHost.cpp:28>) | `[[nodiscard]] VECTOR2 EnemyFeet_( const SpawnRequest& _request)` |
| [L47](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Stage/StageWorldHost.cpp:47>) | `[[nodiscard]] DefaultSpec MakeSpec_( const int _hp, const float _width, const float _height)` |
| [L58](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Stage/StageWorldHost.cpp:58>) | `[[nodiscard]] DefaultSpec GetDefaultSpec_( const StringView _def, const eEntityKind _kind)` |
| [L115](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Stage/StageWorldHost.cpp:115>) | `[[nodiscard]] eStageLayer GetDefaultLayer_( const eEntityKind _kind)` |
| [L130](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Stage/StageWorldHost.cpp:130>) | `[[nodiscard]] bool IsHittableKind_( const eEntityKind _kind)` |
| [L137](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Stage/StageWorldHost.cpp:137>) | `Aabb PlaceholderEntity::GetBox() const` |
| [L142](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Stage/StageWorldHost.cpp:142>) | `bool PlaceholderEntity::IsAlive() const` |
| [L147](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Stage/StageWorldHost.cpp:147>) | `void StageWorldHost::BindWorld( EnemyWorld* const _pEnemies, PropWorld* const _pProps)` |
| [L155](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Stage/StageWorldHost.cpp:155>) | `void StageWorldHost::BindRects( const Span<const NamedRect> _rects)` |
| [L165](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Stage/StageWorldHost.cpp:165>) | `void StageWorldHost::BindPlayer( PlayerController* const _pPlayer, const Tuning* const _pTuning)` |
| [L173](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Stage/StageWorldHost.cpp:173>) | `void StageWorldHost::BindUi( UiSystem* const _pUi)` |
| [L179](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Stage/StageWorldHost.cpp:179>) | `void StageWorldHost::AddGroundMap( const StringView _name, GroundMap _map)` |
| [L190](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Stage/StageWorldHost.cpp:190>) | `void StageWorldHost::Reset()` |
| [L203](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Stage/StageWorldHost.cpp:203>) | `void StageWorldHost::RegisterPlaceholderHurtboxes( CombatSystem& _combat) const` |
| [L220](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Stage/StageWorldHost.cpp:220>) | `int StageWorldHost::ApplyPlaceholderEvents( const Span<const HitEvent> _events)` |
| [L245](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Stage/StageWorldHost.cpp:245>) | `void StageWorldHost::TickPlaceholders()` |
| [L259](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Stage/StageWorldHost.cpp:259>) | `const GroundMap* StageWorldHost::GetActiveGround() const` |
| [L268](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Stage/StageWorldHost.cpp:268>) | `GroundMap* StageWorldHost::GetActiveGroundMut()` |
| [L277](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Stage/StageWorldHost.cpp:277>) | `const String& StageWorldHost::GetActiveGroundName() const` |
| [L287](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Stage/StageWorldHost.cpp:287>) | `Span<const PlaceholderEntity> StageWorldHost::GetPlaceholderEntities() const` |
| [L292](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Stage/StageWorldHost.cpp:292>) | `const String& StageWorldHost::GetLastSectionId() const` |
| [L297](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Stage/StageWorldHost.cpp:297>) | `int StageWorldHost::GetLastSectionIndex() const` |
| [L302](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Stage/StageWorldHost.cpp:302>) | `const String& StageWorldHost::GetEndStateName() const` |
| [L307](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Stage/StageWorldHost.cpp:307>) | `bool StageWorldHost::IsEndRequested() const` |
| [L312](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Stage/StageWorldHost.cpp:312>) | `Span<const StageCollider> StageWorldHost::GetColliders() const` |
| [L317](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Stage/StageWorldHost.cpp:317>) | `const SpawnRoute* StageWorldHost::FindRoute( const SpawnId _id) const` |
| [L330](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Stage/StageWorldHost.cpp:330>) | `SpawnId StageWorldHost::AllocId_()` |
| [L335](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Stage/StageWorldHost.cpp:335>) | `void StageWorldHost::AddRoute_( const SpawnId _id, const eRouteKind _kind, const uint32_t _handle)` |
| [L347](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Stage/StageWorldHost.cpp:347>) | `PlaceholderEntity* StageWorldHost::FindAlive_( const SpawnId _id)` |
| [L360](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Stage/StageWorldHost.cpp:360>) | `const PlaceholderEntity* StageWorldHost::Find_( const SpawnId _id) const` |
| [L373](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Stage/StageWorldHost.cpp:373>) | `uint64_t StageWorldHost::HashState() const` |
| [L395](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Stage/StageWorldHost.cpp:395>) | `SpawnId StageWorldHost::SpawnPlaceholder_( const SpawnRequest& _request)` |
| [L417](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Stage/StageWorldHost.cpp:417>) | `SpawnId StageWorldHost::SpawnEntity( const SpawnRequest& _request)` |
| [L493](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Stage/StageWorldHost.cpp:493>) | `bool StageWorldHost::IsDefeated( const SpawnId _id) const` |
| [L512](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Stage/StageWorldHost.cpp:512>) | `return std::none_of(m_colliders.begin(), m_colliders.end(), [_id](const StageCollider& _c)` |
| [L517](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Stage/StageWorldHost.cpp:517>) | `bool StageWorldHost::IsDestroyFinished( const SpawnId _id) const` |
| [L540](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Stage/StageWorldHost.cpp:540>) | `void StageWorldHost::ClearEntities( const EntityFilter& _filter)` |
| [L557](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Stage/StageWorldHost.cpp:557>) | `void StageWorldHost::SendCommand( const SpawnId _id, const StringView _command)` |
| [L573](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Stage/StageWorldHost.cpp:573>) | `void StageWorldHost::SetGround( const StringView _name)` |
| [L587](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Stage/StageWorldHost.cpp:587>) | `std::optional<float> StageWorldHost::GetGroundFindY( const float _x, const float _fromY) const` |
| [L605](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Stage/StageWorldHost.cpp:605>) | `VECTOR2 StageWorldHost::PlayerFeet() const` |
| [L610](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Stage/StageWorldHost.cpp:610>) | `bool StageWorldHost::IsGameOver() const` |
| [L615](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Stage/StageWorldHost.cpp:615>) | `void StageWorldHost::SetPlayerInvincible( const float _seconds)` |
| [L624](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Stage/StageWorldHost.cpp:624>) | `void StageWorldHost::WarpPlayer( const VECTOR2 _feet)` |
| [L636](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Stage/StageWorldHost.cpp:636>) | `void StageWorldHost::SetBackdrop( StringView)` |
| [L641](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Stage/StageWorldHost.cpp:641>) | `void StageWorldHost::ShowBanner( const String& _text, const float _seconds)` |
| [L651](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Stage/StageWorldHost.cpp:651>) | `void StageWorldHost::FadeTo( const eFadeTarget _target, const float _seconds)` |
| [L670](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Stage/StageWorldHost.cpp:670>) | `void StageWorldHost::PlayBgm( const StringView _name, const bool _bLoop)` |
| [L677](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Stage/StageWorldHost.cpp:677>) | `void StageWorldHost::ChangeBgm( const StringView _name, const bool _bLoop, const float _fadeOutSec)` |
| [L685](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Stage/StageWorldHost.cpp:685>) | `void StageWorldHost::PlaySfx( const StringView _name)` |
| [L691](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Stage/StageWorldHost.cpp:691>) | `void StageWorldHost::EndStage( const StringView _nextState)` |
| [L698](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Stage/StageWorldHost.cpp:698>) | `void StageWorldHost::OnSectionEntered( const int _index, const StringView _id)` |

## Source/Game/Stage/StageWorldHost.h

[StageWorldHost.h](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Stage/StageWorldHost.h>) · 140줄

| 위치 | 선언·함수 |
|---|---|
| [L14](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Stage/StageWorldHost.h:14>) | `class EnemyWorld;` |
| [L15](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Stage/StageWorldHost.h:15>) | `class PropWorld;` |
| [L16](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Stage/StageWorldHost.h:16>) | `class PlayerController;` |
| [L17](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Stage/StageWorldHost.h:17>) | `class UiSystem;` |
| [L18](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Stage/StageWorldHost.h:18>) | `struct Tuning;` |
| [L24](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Stage/StageWorldHost.h:24>) | `struct PlaceholderEntity` |
| [L37](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Stage/StageWorldHost.h:37>) | `[[nodiscard]] Aabb GetBox() const;` |
| [L38](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Stage/StageWorldHost.h:38>) | `[[nodiscard]] bool IsAlive() const;` |
| [L41](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Stage/StageWorldHost.h:41>) | `enum class eRouteKind` |
| [L50](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Stage/StageWorldHost.h:50>) | `struct SpawnRoute` |
| [L57](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Stage/StageWorldHost.h:57>) | `struct StageCollider` |
| [L65](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Stage/StageWorldHost.h:65>) | `class StageWorldHost : public IStageHost` |
| [L68](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Stage/StageWorldHost.h:68>) | `void                                        BindWorld(EnemyWorld* _pEnemies, PropWorld* _pProps);` |
| [L69](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Stage/StageWorldHost.h:69>) | `void                                        BindRects(Span<const NamedRect> _rects);` |
| [L70](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Stage/StageWorldHost.h:70>) | `void                                        BindPlayer(PlayerController* _pPlayer, const Tuning* _pTuning);` |
| [L71](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Stage/StageWorldHost.h:71>) | `void                                        BindUi(UiSystem* _pUi);` |
| [L72](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Stage/StageWorldHost.h:72>) | `void                                        AddGroundMap(StringView _name, GroundMap _map);` |
| [L73](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Stage/StageWorldHost.h:73>) | `void                                        Reset();` |
| [L74](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Stage/StageWorldHost.h:74>) | `void                                        RegisterPlaceholderHurtboxes(CombatSystem& _combat) const;` |
| [L75](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Stage/StageWorldHost.h:75>) | `int                                         ApplyPlaceholderEvents(Span<const HitEvent> _events);` |
| [L76](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Stage/StageWorldHost.h:76>) | `void                                        TickPlaceholders();` |
| [L77](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Stage/StageWorldHost.h:77>) | `[[nodiscard]] Span<const PlaceholderEntity> GetPlaceholderEntities() const;` |
| [L78](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Stage/StageWorldHost.h:78>) | `[[nodiscard]] const String&                 GetLastSectionId() const;` |
| [L79](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Stage/StageWorldHost.h:79>) | `[[nodiscard]] int                           GetLastSectionIndex() const;` |
| [L80](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Stage/StageWorldHost.h:80>) | `[[nodiscard]] const String&                 GetEndStateName() const;` |
| [L81](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Stage/StageWorldHost.h:81>) | `[[nodiscard]] bool                          IsEndRequested() const;` |
| [L82](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Stage/StageWorldHost.h:82>) | `[[nodiscard]] Span<const StageCollider>     GetColliders() const;` |
| [L83](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Stage/StageWorldHost.h:83>) | `[[nodiscard]] const SpawnRoute*             FindRoute(SpawnId _id) const;` |
| [L84](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Stage/StageWorldHost.h:84>) | `[[nodiscard]] const GroundMap*              GetActiveGround() const;` |
| [L85](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Stage/StageWorldHost.h:85>) | `[[nodiscard]] GroundMap*                    GetActiveGroundMut();` |
| [L86](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Stage/StageWorldHost.h:86>) | `[[nodiscard]] const String&                 GetActiveGroundName() const;` |
| [L87](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Stage/StageWorldHost.h:87>) | `[[nodiscard]] uint64_t                      HashState() const;` |
| [L88](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Stage/StageWorldHost.h:88>) | `SpawnId                                     SpawnEntity(const SpawnRequest& _request) override;` |
| [L89](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Stage/StageWorldHost.h:89>) | `[[nodiscard]] bool                          IsDefeated(SpawnId _id) const override;` |
| [L90](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Stage/StageWorldHost.h:90>) | `[[nodiscard]] bool                          IsDestroyFinished(SpawnId _id) const override;` |
| [L91](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Stage/StageWorldHost.h:91>) | `void                                        ClearEntities(const EntityFilter& _filter) override;` |
| [L92](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Stage/StageWorldHost.h:92>) | `void                                        SendCommand(SpawnId _id, StringView _command) override;` |
| [L93](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Stage/StageWorldHost.h:93>) | `void                                        SetGround(StringView _name) override;` |
| [L94](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Stage/StageWorldHost.h:94>) | `[[nodiscard]] std::optional<float>          GetGroundFindY(float _x, float _fromY) const override;` |
| [L95](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Stage/StageWorldHost.h:95>) | `[[nodiscard]] VECTOR2                       PlayerFeet() const override;` |
| [L96](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Stage/StageWorldHost.h:96>) | `[[nodiscard]] bool                          IsGameOver() const override;` |
| [L97](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Stage/StageWorldHost.h:97>) | `void                                        SetPlayerInvincible(float _seconds) override;` |
| [L98](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Stage/StageWorldHost.h:98>) | `void                                        WarpPlayer(VECTOR2 _feet) override;` |
| [L99](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Stage/StageWorldHost.h:99>) | `void                                        SetBackdrop(StringView _modeId) override;` |
| [L100](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Stage/StageWorldHost.h:100>) | `void                                        ShowBanner(const String& _text, float _seconds) override;` |
| [L101](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Stage/StageWorldHost.h:101>) | `void                                        FadeTo(eFadeTarget _target, float _seconds) override;` |
| [L102](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Stage/StageWorldHost.h:102>) | `void                                        PlayBgm(StringView _name, bool _bLoop) override;` |
| [L103](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Stage/StageWorldHost.h:103>) | `void                                        ChangeBgm(StringView _name, bool _bLoop, float _fadeOutSec) override;` |
| [L104](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Stage/StageWorldHost.h:104>) | `void                                        PlaySfx(StringView _name) override;` |
| [L105](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Stage/StageWorldHost.h:105>) | `void                                        EndStage(StringView _nextState) override;` |
| [L106](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Stage/StageWorldHost.h:106>) | `void                                        OnSectionEntered(int _index, StringView _id) override;` |
| [L109](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Stage/StageWorldHost.h:109>) | `struct NamedGround` |
| [L115](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Stage/StageWorldHost.h:115>) | `[[nodiscard]] SpawnId                  AllocId_();` |
| [L116](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Stage/StageWorldHost.h:116>) | `void                                   AddRoute_(SpawnId _id, eRouteKind _kind, uint32_t _handle);` |
| [L117](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Stage/StageWorldHost.h:117>) | `[[nodiscard]] SpawnId                  SpawnPlaceholder_(const SpawnRequest& _request);` |
| [L118](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Stage/StageWorldHost.h:118>) | `[[nodiscard]] PlaceholderEntity*       FindAlive_(SpawnId _id);` |
| [L119](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Stage/StageWorldHost.h:119>) | `[[nodiscard]] const PlaceholderEntity* Find_(SpawnId _id) const;` |

## Source/Game/StateFactory.cpp

[StateFactory.cpp](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/StateFactory.cpp>) · 91줄

| 위치 | 선언·함수 |
|---|---|
| [L10](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/StateFactory.cpp:10>) | `Scoped<IGameState> CreateState( const StringView _name)` |
| [L88](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/StateFactory.cpp:88>) | `StringView GetDefaultStateName()` |

## Source/Game/StateFactory.h

[StateFactory.h](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/StateFactory.h>) · 7줄

| 위치 | 선언·함수 |
|---|---|
| [L5](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/StateFactory.h:5>) | `[[nodiscard]] Scoped<IGameState> CreateState(StringView _name);` |
| [L7](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/StateFactory.h:7>) | `[[nodiscard]] StringView GetDefaultStateName();` |

## Source/Game/TitleState.cpp

[TitleState.cpp](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/TitleState.cpp>) · 157줄

| 위치 | 선언·함수 |
|---|---|
| [L13](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/TitleState.cpp:13>) | `[[nodiscard]] uint8_t MixChannel_( const uint8_t _a, const uint8_t _b, const float _t)` |
| [L21](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/TitleState.cpp:21>) | `void DrawBackground_( Renderer& _renderer)` |
| [L43](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/TitleState.cpp:43>) | `void TitleState::OnEnter()` |
| [L61](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/TitleState.cpp:61>) | `void TitleState::OnExit()` |
| [L66](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/TitleState.cpp:66>) | `Scoped<IGameState> TitleState::CreateNext_() const` |
| [L79](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/TitleState.cpp:79>) | `void TitleState::Tick( const InputHistory& _input)` |
| [L104](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/TitleState.cpp:104>) | `Scoped<IGameState> TitleState::TakeNextState()` |
| [L109](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/TitleState.cpp:109>) | `void TitleState::Render( const float)` |
| [L139](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/TitleState.cpp:139>) | `bool TitleState::IsStarting() const` |
| [L144](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/TitleState.cpp:144>) | `int TitleState::GetTickCount() const` |
| [L149](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/TitleState.cpp:149>) | `const String& TitleState::GetSelectedStateName() const` |
| [L154](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/TitleState.cpp:154>) | `const UiSystem& TitleState::GetUi() const` |

## Source/Game/TitleState.h

[TitleState.h](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/TitleState.h>) · 40줄

| 위치 | 선언·함수 |
|---|---|
| [L6](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/TitleState.h:6>) | `struct TitleConfig` |
| [L15](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/TitleState.h:15>) | `class TitleState : public IGameState` |
| [L18](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/TitleState.h:18>) | `TitleState();` |
| [L19](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/TitleState.h:19>) | `explicit TitleState(TitleConfig _config);` |
| [L20](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/TitleState.h:20>) | `void                             OnEnter() override;` |
| [L21](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/TitleState.h:21>) | `void                             OnExit() override;` |
| [L22](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/TitleState.h:22>) | `void                             Tick(const InputHistory& _input) override;` |
| [L23](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/TitleState.h:23>) | `void                             Render(float _alpha) override;` |
| [L24](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/TitleState.h:24>) | `[[nodiscard]] Scoped<IGameState> TakeNextState() override;` |
| [L25](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/TitleState.h:25>) | `[[nodiscard]] bool               IsStarting() const;` |
| [L26](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/TitleState.h:26>) | `[[nodiscard]] int                GetTickCount() const;` |
| [L27](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/TitleState.h:27>) | `[[nodiscard]] const String&      GetSelectedStateName() const;` |
| [L28](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/TitleState.h:28>) | `[[nodiscard]] const UiSystem&    GetUi() const;` |
| [L31](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/TitleState.h:31>) | `[[nodiscard]] Scoped<IGameState> CreateNext_() const;` |

## Source/Game/Ui/Banner.cpp

[Banner.cpp](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Ui/Banner.cpp>) · 208줄

| 위치 | 선언·함수 |
|---|---|
| [L15](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Ui/Banner.cpp:15>) | `BannerSpec MakeBannerSpec( const StringView _text, const float _seconds)` |
| [L32](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Ui/Banner.cpp:32>) | `float ComputeBannerAlpha( const int _elapsed, const int _fadeIn, const int _hold, const int _fadeOut)` |
| [L54](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Ui/Banner.cpp:54>) | `GlyphPose ComputeMissionGlyphPose( const int _elapsed, const int _fadeIn, const int _hold, const int _fadeOut, const int _index)` |
| [L115](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Ui/Banner.cpp:115>) | `String MakeMissionGlyphName( const char _ch)` |
| [L124](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Ui/Banner.cpp:124>) | `else if (up == '0')` |
| [L128](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Ui/Banner.cpp:128>) | `else if (up == '!')` |
| [L132](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Ui/Banner.cpp:132>) | `else if (up == '?')` |
| [L143](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Ui/Banner.cpp:143>) | `void BannerSystem::Begin_( BannerSpec _spec)` |
| [L151](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Ui/Banner.cpp:151>) | `void BannerSystem::Show( BannerSpec _spec)` |
| [L157](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Ui/Banner.cpp:157>) | `void BannerSystem::Clear()` |
| [L164](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Ui/Banner.cpp:164>) | `void BannerSystem::Tick()` |
| [L181](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Ui/Banner.cpp:181>) | `float BannerSystem::GetAlpha() const` |
| [L190](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Ui/Banner.cpp:190>) | `bool BannerSystem::IsActive() const` |
| [L195](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Ui/Banner.cpp:195>) | `int BannerSystem::GetElapsedTicks() const` |
| [L200](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Ui/Banner.cpp:200>) | `const BannerSpec& BannerSystem::GetCurrent() const` |
| [L205](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Ui/Banner.cpp:205>) | `int BannerSpec::GetTotalTicks() const` |

## Source/Game/Ui/Banner.h

[Banner.h](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Ui/Banner.h>) · 53줄

| 위치 | 선언·함수 |
|---|---|
| [L7](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Ui/Banner.h:7>) | `struct BannerSpec` |
| [L17](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Ui/Banner.h:17>) | `[[nodiscard]] int GetTotalTicks() const;` |
| [L20](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Ui/Banner.h:20>) | `[[nodiscard]] BannerSpec MakeBannerSpec(StringView _text, float _seconds);` |
| [L22](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Ui/Banner.h:22>) | `[[nodiscard]] float ComputeBannerAlpha(int _elapsed, int _fadeIn, int _hold, int _fadeOut);` |
| [L24](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Ui/Banner.h:24>) | `struct GlyphPose` |
| [L31](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Ui/Banner.h:31>) | `[[nodiscard]] GlyphPose ComputeMissionGlyphPose(int _elapsed, int _fadeIn, int _hold, int _fadeOut, int _index);` |
| [L34](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Ui/Banner.h:34>) | `[[nodiscard]] String MakeMissionGlyphName(char _ch);` |
| [L36](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Ui/Banner.h:36>) | `class BannerSystem` |
| [L39](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Ui/Banner.h:39>) | `void                            Show(BannerSpec _spec);` |
| [L40](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Ui/Banner.h:40>) | `void                            Clear();` |
| [L41](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Ui/Banner.h:41>) | `void                            Tick();` |
| [L42](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Ui/Banner.h:42>) | `[[nodiscard]] bool              IsActive() const;` |
| [L43](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Ui/Banner.h:43>) | `[[nodiscard]] int               GetElapsedTicks() const;` |
| [L44](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Ui/Banner.h:44>) | `[[nodiscard]] float             GetAlpha() const;` |
| [L45](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Ui/Banner.h:45>) | `[[nodiscard]] const BannerSpec& GetCurrent() const;` |
| [L48](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Ui/Banner.h:48>) | `void Begin_(BannerSpec _spec);` |

## Source/Game/Ui/BitmapFont.cpp

[BitmapFont.cpp](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Ui/BitmapFont.cpp>) · 267줄

| 위치 | 선언·함수 |
|---|---|
| [L10](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Ui/BitmapFont.cpp:10>) | `[[nodiscard]] RGBA GetFontFallbackColor_( const eFontColor _color)` |
| [L24](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Ui/BitmapFont.cpp:24>) | `[[nodiscard]] const char* GetFontSpriteName_( const eFontColor _color)` |
| [L39](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Ui/BitmapFont.cpp:39>) | `TextLayout LayoutText( const StringView _text, const int _x, const int _y, const TextStyle& _style)` |
| [L70](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Ui/BitmapFont.cpp:70>) | `else if (_style.align == eTextAlign::Right)` |
| [L99](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Ui/BitmapFont.cpp:99>) | `else if (_style.align == eTextAlign::Right)` |
| [L107](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Ui/BitmapFont.cpp:107>) | `bool BitmapFont::Init( const SpriteLibrary& _sprites)` |
| [L129](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Ui/BitmapFont.cpp:129>) | `AnimFrame BitmapFont::GetGlyphFrame( const eFontColor _color, const bool _bBold, const int _glyph) const` |
| [L150](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Ui/BitmapFont.cpp:150>) | `AnimFrame BitmapFont::GetTimeDigitFrame( const int _digit) const` |
| [L168](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Ui/BitmapFont.cpp:168>) | `void BitmapFont::DrawString( Renderer& _renderer, SpriteBatch& _sprites, const int _x, const int _y, const StringView _text, const TextStyle& _style) const` |
| [L191](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Ui/BitmapFont.cpp:191>) | `else if (quad.ch != ' ' && pNative)` |
| [L206](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Ui/BitmapFont.cpp:206>) | `void BitmapFont::DrawTimeDigits( SpriteBatch& _sprites, const int _centerX, const int _topY, const StringView _digits, const uint8_t _alpha) const` |
| [L237](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Ui/BitmapFont.cpp:237>) | `bool BitmapFont::HasSprites() const` |
| [L242](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Ui/BitmapFont.cpp:242>) | `int GetGlyphIndex( const char _ch)` |
| [L261](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Ui/BitmapFont.cpp:261>) | `int MeasureChars( const int _charCount, const int _scale, const int _letterSpacing)` |

## Source/Game/Ui/BitmapFont.h

[BitmapFont.h](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Ui/BitmapFont.h>) · 80줄

| 위치 | 선언·함수 |
|---|---|
| [L10](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Ui/BitmapFont.h:10>) | `enum class eTextAlign` |
| [L17](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Ui/BitmapFont.h:17>) | `enum class eFontColor` |
| [L28](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Ui/BitmapFont.h:28>) | `struct TextStyle` |
| [L43](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Ui/BitmapFont.h:43>) | `[[nodiscard]] int GetGlyphIndex(char _ch);` |
| [L45](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Ui/BitmapFont.h:45>) | `[[nodiscard]] int MeasureChars(int _charCount, int _scale, int _letterSpacing = 0);` |
| [L47](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Ui/BitmapFont.h:47>) | `struct GlyphQuad` |
| [L55](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Ui/BitmapFont.h:55>) | `struct TextLayout` |
| [L64](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Ui/BitmapFont.h:64>) | `[[nodiscard]] TextLayout LayoutText(StringView _text, int _x, int _y, const TextStyle& _style);` |
| [L66](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Ui/BitmapFont.h:66>) | `class BitmapFont` |
| [L69](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Ui/BitmapFont.h:69>) | `bool                    Init(const SpriteLibrary& _sprites);` |
| [L70](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Ui/BitmapFont.h:70>) | `[[nodiscard]] bool      HasSprites() const;` |
| [L71](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Ui/BitmapFont.h:71>) | `void                    DrawString(Renderer& _renderer, SpriteBatch& _sprites, int _x, int _y, StringView _text, const TextStyle& _style) const;` |
| [L72](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Ui/BitmapFont.h:72>) | `void                    DrawTimeDigits(SpriteBatch& _sprites, int _centerX, int _topY, StringView _digits, uint8_t _alpha = 255) const;` |
| [L73](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Ui/BitmapFont.h:73>) | `[[nodiscard]] AnimFrame GetGlyphFrame(eFontColor _color, bool _bBold, int _glyph) const;` |
| [L74](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Ui/BitmapFont.h:74>) | `[[nodiscard]] AnimFrame GetTimeDigitFrame(int _digit) const;` |

## Source/Game/Ui/FadeController.cpp

[FadeController.cpp](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Ui/FadeController.cpp>) · 101줄

| 위치 | 선언·함수 |
|---|---|
| [L6](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Ui/FadeController.cpp:6>) | `void FadeController::SetClear()` |
| [L16](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Ui/FadeController.cpp:16>) | `void FadeController::SetBlack()` |
| [L26](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Ui/FadeController.cpp:26>) | `void FadeController::Begin_( const float _target, const int _ticks, const ePhase _movingPhase, const ePhase _donePhase)` |
| [L49](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Ui/FadeController.cpp:49>) | `void FadeController::FadeToBlack( const int _ticks)` |
| [L55](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Ui/FadeController.cpp:55>) | `void FadeController::FadeToClear( const int _ticks)` |
| [L61](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Ui/FadeController.cpp:61>) | `void FadeController::Tick()` |
| [L78](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Ui/FadeController.cpp:78>) | `uint8_t FadeController::GetAlpha255() const` |
| [L83](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Ui/FadeController.cpp:83>) | `float FadeController::GetAlpha() const` |
| [L88](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Ui/FadeController.cpp:88>) | `bool FadeController::IsBusy() const` |
| [L93](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Ui/FadeController.cpp:93>) | `bool FadeController::IsBlack() const` |
| [L98](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Ui/FadeController.cpp:98>) | `bool FadeController::IsClear() const` |

## Source/Game/Ui/FadeController.h

[FadeController.h](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Ui/FadeController.h>) · 35줄

| 위치 | 선언·함수 |
|---|---|
| [L4](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Ui/FadeController.h:4>) | `class FadeController` |
| [L7](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Ui/FadeController.h:7>) | `enum class ePhase` |
| [L15](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Ui/FadeController.h:15>) | `void                  SetClear();` |
| [L16](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Ui/FadeController.h:16>) | `void                  SetBlack();` |
| [L17](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Ui/FadeController.h:17>) | `void                  FadeToBlack(int _ticks);` |
| [L18](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Ui/FadeController.h:18>) | `void                  FadeToClear(int _ticks);` |
| [L19](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Ui/FadeController.h:19>) | `void                  Tick();` |
| [L20](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Ui/FadeController.h:20>) | `[[nodiscard]] float   GetAlpha() const;` |
| [L21](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Ui/FadeController.h:21>) | `[[nodiscard]] bool    IsBusy() const;` |
| [L22](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Ui/FadeController.h:22>) | `[[nodiscard]] bool    IsBlack() const;` |
| [L23](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Ui/FadeController.h:23>) | `[[nodiscard]] bool    IsClear() const;` |
| [L24](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Ui/FadeController.h:24>) | `[[nodiscard]] uint8_t GetAlpha255() const;` |
| [L27](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Ui/FadeController.h:27>) | `void Begin_(float _target, int _ticks, ePhase _movingPhase, ePhase _donePhase);` |

## Source/Game/Ui/Hud.cpp

[Hud.cpp](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Ui/Hud.cpp>) · 193줄

| 위치 | 선언·함수 |
|---|---|
| [L11](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Ui/Hud.cpp:11>) | `[[nodiscard]] HudItem MakeText_( String _text, const int _x, const int _y, const eTextAlign _align, const eFontColor _color)` |
| [L29](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Ui/Hud.cpp:29>) | `[[nodiscard]] HudItem MakeSprite_( String _name, const int _centerX, const int _centerY, const int _scale = 1)` |
| [L45](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Ui/Hud.cpp:45>) | `String FormatScore( const int _score)` |
| [L51](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Ui/Hud.cpp:51>) | `String FormatAmmo( const int _ammo)` |
| [L57](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Ui/Hud.cpp:57>) | `String FormatBombs( const int _bombs)` |
| [L63](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Ui/Hud.cpp:63>) | `String FormatTimer( const int _seconds)` |
| [L69](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Ui/Hud.cpp:69>) | `HudModel MakeHudModel( const PlayerController& _player)` |
| [L83](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Ui/Hud.cpp:83>) | `Vector<HudItem> BuildHudItems( const HudModel& _model, const UiLayout& _layout)` |
| [L165](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Ui/Hud.cpp:165>) | `void DisplayTimer::Reset()` |
| [L170](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Ui/Hud.cpp:170>) | `void DisplayTimer::Tick()` |
| [L178](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Ui/Hud.cpp:178>) | `int DisplayTimer::GetSeconds() const` |
| [L183](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Ui/Hud.cpp:183>) | `int DisplayTimer::GetRemainingTicks() const` |
| [L188](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Ui/Hud.cpp:188>) | `bool IsBlinkOn( const int _tick, const int _periodTicks)` |

## Source/Game/Ui/Hud.h

[Hud.h](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Ui/Hud.h>) · 89줄

| 위치 | 선언·함수 |
|---|---|
| [L8](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Ui/Hud.h:8>) | `class PlayerController;` |
| [L10](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Ui/Hud.h:10>) | `struct HudModel` |
| [L33](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Ui/Hud.h:33>) | `class DisplayTimer` |
| [L39](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Ui/Hud.h:39>) | `void              Reset();` |
| [L40](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Ui/Hud.h:40>) | `void              Tick();` |
| [L41](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Ui/Hud.h:41>) | `[[nodiscard]] int GetSeconds() const;` |
| [L42](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Ui/Hud.h:42>) | `[[nodiscard]] int GetRemainingTicks() const;` |
| [L48](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Ui/Hud.h:48>) | `struct MissionResultsView` |
| [L59](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Ui/Hud.h:59>) | `[[nodiscard]] HudModel MakeHudModel(const PlayerController& _player);` |
| [L61](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Ui/Hud.h:61>) | `enum class eHudItemKind` |
| [L69](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Ui/Hud.h:69>) | `struct HudItem` |
| [L82](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Ui/Hud.h:82>) | `[[nodiscard]] Vector<HudItem> BuildHudItems(const HudModel& _model, const UiLayout& _layout);` |
| [L84](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Ui/Hud.h:84>) | `[[nodiscard]] bool IsBlinkOn(int _tick, int _periodTicks);` |
| [L86](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Ui/Hud.h:86>) | `[[nodiscard]] String FormatScore(int _score);` |
| [L87](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Ui/Hud.h:87>) | `[[nodiscard]] String FormatAmmo(int _ammo);` |
| [L88](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Ui/Hud.h:88>) | `[[nodiscard]] String FormatBombs(int _bombs);` |
| [L89](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Ui/Hud.h:89>) | `[[nodiscard]] String FormatTimer(int _seconds);` |

## Source/Game/Ui/ScreenFlow.cpp

[ScreenFlow.cpp](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Ui/ScreenFlow.cpp>) · 282줄

| 위치 | 선언·함수 |
|---|---|
| [L11](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Ui/ScreenFlow.cpp:11>) | `ScreenFlowConfig MakeGameOverFlowConfig()` |
| [L27](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Ui/ScreenFlow.cpp:27>) | `StringView MissionRankText( const int _rescued)` |
| [L41](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Ui/ScreenFlow.cpp:41>) | `int MissionRankBonus( const int _rescued)` |
| [L55](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Ui/ScreenFlow.cpp:55>) | `int MissionBonusScore( const int _rescued)` |
| [L62](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Ui/ScreenFlow.cpp:62>) | `int MissionResultsTicks( const int _rescued)` |
| [L68](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Ui/ScreenFlow.cpp:68>) | `MissionResultsView MakeMissionResultsView( const int _rescued, const int _localTicks)` |
| [L98](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Ui/ScreenFlow.cpp:98>) | `ScreenFlowConfig MakeMissionCompleteFlowConfig( const int _rescued)` |
| [L115](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Ui/ScreenFlow.cpp:115>) | `void ScreenFlow::Reset()` |
| [L122](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Ui/ScreenFlow.cpp:122>) | `void ScreenFlow::Start( const ScreenFlowConfig& _config, UiSystem& _ui, AudioSystem& _audio)` |
| [L143](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Ui/ScreenFlow.cpp:143>) | `void ScreenFlow::Tick( UiSystem& _ui, AudioSystem& _audio)` |
| [L155](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Ui/ScreenFlow.cpp:155>) | `MissionResultsView ScreenFlow::GetResultsView() const` |
| [L164](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Ui/ScreenFlow.cpp:164>) | `GameOverArtView ScreenFlow::GetArtView() const` |
| [L185](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Ui/ScreenFlow.cpp:185>) | `void ScreenFlow::BeginBanner_( UiSystem& _ui, AudioSystem& _audio)` |
| [L213](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Ui/ScreenFlow.cpp:213>) | `void ScreenFlow::BeginFade_( UiSystem& _ui)` |
| [L220](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Ui/ScreenFlow.cpp:220>) | `void ScreenFlow::Advance_( UiSystem& _ui, AudioSystem& _audio)` |
| [L259](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Ui/ScreenFlow.cpp:259>) | `eFlowPhase ScreenFlow::GetPhase() const` |
| [L264](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Ui/ScreenFlow.cpp:264>) | `bool ScreenFlow::IsRunning() const` |
| [L269](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Ui/ScreenFlow.cpp:269>) | `bool ScreenFlow::IsDone() const` |
| [L274](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Ui/ScreenFlow.cpp:274>) | `int ScreenFlow::GetElapsedTicks() const` |
| [L279](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Ui/ScreenFlow.cpp:279>) | `int ScreenFlowConfig::GetTotalTicks() const` |

## Source/Game/Ui/ScreenFlow.h

[ScreenFlow.h](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Ui/ScreenFlow.h>) · 69줄

| 위치 | 선언·함수 |
|---|---|
| [L9](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Ui/ScreenFlow.h:9>) | `enum class eFlowPhase` |
| [L21](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Ui/ScreenFlow.h:21>) | `struct ScreenFlowConfig` |
| [L36](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Ui/ScreenFlow.h:36>) | `[[nodiscard]] int GetTotalTicks() const;` |
| [L39](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Ui/ScreenFlow.h:39>) | `[[nodiscard]] StringView         MissionRankText(int _rescued);` |
| [L40](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Ui/ScreenFlow.h:40>) | `[[nodiscard]] int                MissionRankBonus(int _rescued);` |
| [L41](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Ui/ScreenFlow.h:41>) | `[[nodiscard]] int                MissionBonusScore(int _rescued);` |
| [L42](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Ui/ScreenFlow.h:42>) | `[[nodiscard]] int                MissionResultsTicks(int _rescued);` |
| [L43](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Ui/ScreenFlow.h:43>) | `[[nodiscard]] MissionResultsView MakeMissionResultsView(int _rescued, int _localTicks);` |
| [L45](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Ui/ScreenFlow.h:45>) | `[[nodiscard]] ScreenFlowConfig MakeGameOverFlowConfig();` |
| [L46](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Ui/ScreenFlow.h:46>) | `[[nodiscard]] ScreenFlowConfig MakeMissionCompleteFlowConfig(int _rescued = 0);` |
| [L48](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Ui/ScreenFlow.h:48>) | `class ScreenFlow` |
| [L51](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Ui/ScreenFlow.h:51>) | `void                             Start(const ScreenFlowConfig& _config, UiSystem& _ui, AudioSystem& _audio);` |
| [L52](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Ui/ScreenFlow.h:52>) | `void                             Tick(UiSystem& _ui, AudioSystem& _audio);` |
| [L53](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Ui/ScreenFlow.h:53>) | `void                             Reset();` |
| [L54](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Ui/ScreenFlow.h:54>) | `[[nodiscard]] eFlowPhase         GetPhase() const;` |
| [L55](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Ui/ScreenFlow.h:55>) | `[[nodiscard]] bool               IsRunning() const;` |
| [L56](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Ui/ScreenFlow.h:56>) | `[[nodiscard]] bool               IsDone() const;` |
| [L57](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Ui/ScreenFlow.h:57>) | `[[nodiscard]] int                GetElapsedTicks() const;` |
| [L58](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Ui/ScreenFlow.h:58>) | `[[nodiscard]] MissionResultsView GetResultsView() const;` |
| [L59](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Ui/ScreenFlow.h:59>) | `[[nodiscard]] GameOverArtView    GetArtView() const;` |
| [L62](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Ui/ScreenFlow.h:62>) | `void BeginBanner_(UiSystem& _ui, AudioSystem& _audio);` |
| [L63](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Ui/ScreenFlow.h:63>) | `void BeginFade_(UiSystem& _ui);` |
| [L64](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Ui/ScreenFlow.h:64>) | `void Advance_(UiSystem& _ui, AudioSystem& _audio);` |

## Source/Game/Ui/UiDemoState.cpp

[UiDemoState.cpp](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Ui/UiDemoState.cpp>) · 79줄

| 위치 | 선언·함수 |
|---|---|
| [L7](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Ui/UiDemoState.cpp:7>) | `void UiDemoState::OnEnter()` |
| [L16](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Ui/UiDemoState.cpp:16>) | `HudModel UiDemoState::MakeModel_() const` |
| [L32](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Ui/UiDemoState.cpp:32>) | `void UiDemoState::Tick( const InputHistory&)` |
| [L43](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Ui/UiDemoState.cpp:43>) | `else if (m_tick == 260)` |
| [L49](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Ui/UiDemoState.cpp:49>) | `else if (m_tick == 420)` |
| [L53](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Ui/UiDemoState.cpp:53>) | `else if (m_tick == 1200)` |
| [L59](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Ui/UiDemoState.cpp:59>) | `void UiDemoState::Render( const float)` |
| [L71](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Ui/UiDemoState.cpp:71>) | `int UiDemoState::GetTickCount() const` |
| [L76](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Ui/UiDemoState.cpp:76>) | `bool UiDemoState::IsFinished() const` |

## Source/Game/Ui/UiDemoState.h

[UiDemoState.h](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Ui/UiDemoState.h>) · 21줄

| 위치 | 선언·함수 |
|---|---|
| [L6](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Ui/UiDemoState.h:6>) | `class UiDemoState : public IGameState` |
| [L9](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Ui/UiDemoState.h:9>) | `void               OnEnter() override;` |
| [L10](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Ui/UiDemoState.h:10>) | `void               Tick(const InputHistory& _input) override;` |
| [L11](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Ui/UiDemoState.h:11>) | `void               Render(float _alpha) override;` |
| [L12](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Ui/UiDemoState.h:12>) | `[[nodiscard]] bool IsFinished() const override;` |
| [L13](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Ui/UiDemoState.h:13>) | `[[nodiscard]] int  GetTickCount() const;` |
| [L16](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Ui/UiDemoState.h:16>) | `[[nodiscard]] HudModel MakeModel_() const;` |

## Source/Game/Ui/UiLayout.h

[UiLayout.h](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Ui/UiLayout.h>) · 67줄

| 위치 | 선언·함수 |
|---|---|
| [L4](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Ui/UiLayout.h:4>) | `struct UiLayout` |

## Source/Game/Ui/UiSystem.cpp

[UiSystem.cpp](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Ui/UiSystem.cpp>) · 343줄

| 위치 | 선언·함수 |
|---|---|
| [L13](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Ui/UiSystem.cpp:13>) | `void UiSystem::Init()` |
| [L37](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Ui/UiSystem.cpp:37>) | `void UiSystem::Tick()` |
| [L44](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Ui/UiSystem.cpp:44>) | `void UiSystem::DrawString( const int _x, const int _y, const StringView _text, const TextStyle& _style) const` |
| [L53](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Ui/UiSystem.cpp:53>) | `bool UiSystem::DrawSprite( const StringView _name, const int _centerX, const int _centerY, const int _scale, const uint8_t _alpha) const` |
| [L76](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Ui/UiSystem.cpp:76>) | `bool UiSystem::DrawStandalone( const StringView _name, const int _x, const int _y, const uint8_t _alpha) const` |
| [L97](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Ui/UiSystem.cpp:97>) | `void UiSystem::DrawHud( const HudModel& _model)` |
| [L127](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Ui/UiSystem.cpp:127>) | `void UiSystem::DrawBanner()` |
| [L201](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Ui/UiSystem.cpp:201>) | `void UiSystem::DrawMissionResults() const` |
| [L249](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Ui/UiSystem.cpp:249>) | `void UiSystem::DrawFade()` |
| [L259](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Ui/UiSystem.cpp:259>) | `void UiSystem::DrawGameOverArt_() const` |
| [L283](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Ui/UiSystem.cpp:283>) | `void UiSystem::DrawOverlays()` |
| [L291](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Ui/UiSystem.cpp:291>) | `BannerSystem& UiSystem::GetBanner()` |
| [L296](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Ui/UiSystem.cpp:296>) | `const BannerSystem& UiSystem::GetBanner() const` |
| [L301](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Ui/UiSystem.cpp:301>) | `FadeController& UiSystem::GetFade()` |
| [L306](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Ui/UiSystem.cpp:306>) | `const FadeController& UiSystem::GetFade() const` |
| [L311](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Ui/UiSystem.cpp:311>) | `const UiLayout& UiSystem::GetLayout() const` |
| [L316](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Ui/UiSystem.cpp:316>) | `int UiSystem::GetTick() const` |
| [L321](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Ui/UiSystem.cpp:321>) | `bool UiSystem::HasSprites() const` |
| [L326](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Ui/UiSystem.cpp:326>) | `void UiSystem::ShowBanner( const StringView _text, const float _seconds)` |
| [L333](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Ui/UiSystem.cpp:333>) | `void UiSystem::SetMissionResults( const MissionResultsView& _view)` |
| [L339](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Ui/UiSystem.cpp:339>) | `void UiSystem::SetGameOverArt( const GameOverArtView& _view)` |

## Source/Game/Ui/UiSystem.h

[UiSystem.h](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Ui/UiSystem.h>) · 56줄

| 위치 | 선언·함수 |
|---|---|
| [L13](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Ui/UiSystem.h:13>) | `struct GameOverArtView` |
| [L20](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Ui/UiSystem.h:20>) | `class UiSystem` |
| [L23](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Ui/UiSystem.h:23>) | `void                                Init();` |
| [L24](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Ui/UiSystem.h:24>) | `void                                Tick();` |
| [L25](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Ui/UiSystem.h:25>) | `void                                DrawHud(const HudModel& _model);` |
| [L26](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Ui/UiSystem.h:26>) | `void                                DrawBanner();` |
| [L27](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Ui/UiSystem.h:27>) | `void                                DrawMissionResults() const;` |
| [L28](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Ui/UiSystem.h:28>) | `void                                DrawFade();` |
| [L29](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Ui/UiSystem.h:29>) | `void                                DrawOverlays();` |
| [L30](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Ui/UiSystem.h:30>) | `void                                DrawString(int _x, int _y, StringView _text, const TextStyle& _style) const;` |
| [L31](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Ui/UiSystem.h:31>) | `bool                                DrawSprite(StringView _name, int _centerX, int _centerY, int _scale = 1, uint8_t _alpha = 255) const;` |
| [L32](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Ui/UiSystem.h:32>) | `bool                                DrawStandalone(StringView _name, int _x, int _y, uint8_t _alpha = 255) const;` |
| [L33](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Ui/UiSystem.h:33>) | `[[nodiscard]] BannerSystem&         GetBanner();` |
| [L34](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Ui/UiSystem.h:34>) | `[[nodiscard]] const BannerSystem&   GetBanner() const;` |
| [L35](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Ui/UiSystem.h:35>) | `[[nodiscard]] FadeController&       GetFade();` |
| [L36](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Ui/UiSystem.h:36>) | `[[nodiscard]] const FadeController& GetFade() const;` |
| [L37](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Ui/UiSystem.h:37>) | `[[nodiscard]] const UiLayout&       GetLayout() const;` |
| [L38](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Ui/UiSystem.h:38>) | `[[nodiscard]] int                   GetTick() const;` |
| [L39](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Ui/UiSystem.h:39>) | `[[nodiscard]] bool                  HasSprites() const;` |
| [L40](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Ui/UiSystem.h:40>) | `void                                ShowBanner(StringView _text, float _seconds);` |
| [L41](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Ui/UiSystem.h:41>) | `void                                SetMissionResults(const MissionResultsView& _view);` |
| [L42](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Ui/UiSystem.h:42>) | `void                                SetGameOverArt(const GameOverArtView& _view);` |
| [L45](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Game/Ui/UiSystem.h:45>) | `void DrawGameOverArt_() const;` |

## Source/Main.cpp

[Main.cpp](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Main.cpp>) · 22줄

| 위치 | 선언·함수 |
|---|---|
| [L10](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Main.cpp:10>) | `int main( const int _argc, char** _argv)` |

## Source/Render/Animation.cpp

[Animation.cpp](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Render/Animation.cpp>) · 506줄

| 위치 | 선언·함수 |
|---|---|
| [L14](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Render/Animation.cpp:14>) | `[[nodiscard]] int64_t ToUnits_( const double _sec)` |
| [L20](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Render/Animation.cpp:20>) | `[[nodiscard]] bool ReadOptionalInt_( const JsonReader& _object, const StringView _key, int& _out)` |
| [L28](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Render/Animation.cpp:28>) | `[[nodiscard]] Result<AnimFrame> ParseFrame_( const JsonReader& _json)` |
| [L48](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Render/Animation.cpp:48>) | `[[nodiscard]] Result<AnimClip> ParseClip_( const StringView _name, const JsonReader& _json)` |
| [L115](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Render/Animation.cpp:115>) | `[[nodiscard]] Result<AnimationLibrary> ParseAnimDocument_( const JsonReader& _root)` |
| [L145](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Render/Animation.cpp:145>) | `[[nodiscard]] Result<Vector<String>> ParseAtlasPages_( const JsonReader& _manifest)` |
| [L172](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Render/Animation.cpp:172>) | `int64_t AnimClip::GetFrameDurationUnits() const` |
| [L177](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Render/Animation.cpp:177>) | `Result<AnimationLibrary> AnimationLibrary::LoadFromJson( const StringView _animJson)` |
| [L188](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Render/Animation.cpp:188>) | `AnimationLibrary& AnimationLibrary::GetSingleton()` |
| [L194](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Render/Animation.cpp:194>) | `Result<AnimationLibrary> AnimationLibrary::LoadFromDirectory( const FilePath& _bakedRoot)` |
| [L244](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Render/Animation.cpp:244>) | `AnimId AnimationLibrary::AddClip( AnimClip _clip)` |
| [L259](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Render/Animation.cpp:259>) | `AnimId AnimationLibrary::Find( const StringView _name) const` |
| [L270](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Render/Animation.cpp:270>) | `bool AnimationLibrary::IsValid( const AnimId _id) const` |
| [L276](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Render/Animation.cpp:276>) | `const AnimClip& AnimationLibrary::Get( const AnimId _id) const` |
| [L283](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Render/Animation.cpp:283>) | `const AnimClip* AnimationLibrary::GetOrNull( const AnimId _id) const` |
| [L289](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Render/Animation.cpp:289>) | `const AnimClip* AnimationLibrary::FindClip( const StringView _name) const` |
| [L295](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Render/Animation.cpp:295>) | `void AnimPlayer::Play( const AnimId _clip, const bool _bRestart)` |
| [L310](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Render/Animation.cpp:310>) | `bool AnimPlayer::Update( const AnimationLibrary& _library, const float _dt, const float _speed)` |
| [L327](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Render/Animation.cpp:327>) | `void AnimPlayer::Settle_( const AnimClip& _clip)` |
| [L370](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Render/Animation.cpp:370>) | `void AnimPlayer::SetNormalizedTime( const AnimClip& _clip, const float _t)` |
| [L390](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Render/Animation.cpp:390>) | `void AnimPlayer::SyncPhase( const AnimClip& _clip, const AnimPlayer& _other, const AnimClip& _otherClip)` |
| [L413](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Render/Animation.cpp:413>) | `float AnimPlayer::GetNormalizedTime( const AnimClip& _clip) const` |
| [L424](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Render/Animation.cpp:424>) | `const AnimFrame* AnimPlayer::GetFrameData( const AnimationLibrary& _library) const` |
| [L435](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Render/Animation.cpp:435>) | `bool AnimFrame::IsEmpty() const` |
| [L440](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Render/Animation.cpp:440>) | `int AnimClip::GetFrameCount() const` |
| [L445](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Render/Animation.cpp:445>) | `int64_t AnimClip::GetTotalDurationUnits() const` |
| [L450](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Render/Animation.cpp:450>) | `float AnimClip::GetTotalDurationSec() const` |
| [L455](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Render/Animation.cpp:455>) | `int AnimClip::GetFootBaseline() const` |
| [L468](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Render/Animation.cpp:468>) | `size_t AnimationLibrary::GetClipCount() const` |
| [L473](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Render/Animation.cpp:473>) | `Span<const AnimClip> AnimationLibrary::GetClips() const` |
| [L478](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Render/Animation.cpp:478>) | `Span<const String> AnimationLibrary::GetAtlasPages() const` |
| [L483](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Render/Animation.cpp:483>) | `const FilePath& AnimationLibrary::GetRoot() const` |
| [L488](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Render/Animation.cpp:488>) | `AnimId AnimPlayer::GetClip() const` |
| [L493](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Render/Animation.cpp:493>) | `int AnimPlayer::GetFrame() const` |
| [L498](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Render/Animation.cpp:498>) | `int64_t AnimPlayer::GetTimeUnits() const` |
| [L503](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Render/Animation.cpp:503>) | `bool AnimPlayer::IsFinished() const` |

## Source/Render/Animation.h

[Animation.h](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Render/Animation.h>) · 89줄

| 위치 | 선언·함수 |
|---|---|
| [L9](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Render/Animation.h:9>) | `struct AnimIdTag;` |
| [L12](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Render/Animation.h:12>) | `struct AnimFrame` |
| [L22](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Render/Animation.h:22>) | `[[nodiscard]] bool IsEmpty() const;` |
| [L25](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Render/Animation.h:25>) | `struct AnimClip` |
| [L35](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Render/Animation.h:35>) | `[[nodiscard]] int     GetFrameCount() const;` |
| [L36](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Render/Animation.h:36>) | `[[nodiscard]] int64_t GetFrameDurationUnits() const;` |
| [L37](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Render/Animation.h:37>) | `[[nodiscard]] int64_t GetTotalDurationUnits() const;` |
| [L38](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Render/Animation.h:38>) | `[[nodiscard]] float   GetTotalDurationSec() const;` |
| [L39](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Render/Animation.h:39>) | `[[nodiscard]] int     GetFootBaseline() const;` |
| [L42](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Render/Animation.h:42>) | `class AnimationLibrary` |
| [L45](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Render/Animation.h:45>) | `[[nodiscard]] static AnimationLibrary&        GetSingleton();` |
| [L46](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Render/Animation.h:46>) | `[[nodiscard]] static Result<AnimationLibrary> LoadFromDirectory(const FilePath& _bakedRoot);` |
| [L47](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Render/Animation.h:47>) | `[[nodiscard]] static Result<AnimationLibrary> LoadFromJson(StringView _animJson);` |
| [L48](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Render/Animation.h:48>) | `AnimId                                        AddClip(AnimClip _clip);` |
| [L49](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Render/Animation.h:49>) | `[[nodiscard]] AnimId                          Find(StringView _name) const;` |
| [L50](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Render/Animation.h:50>) | `[[nodiscard]] bool                            IsValid(AnimId _id) const;` |
| [L51](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Render/Animation.h:51>) | `[[nodiscard]] const AnimClip&                 Get(AnimId _id) const;` |
| [L52](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Render/Animation.h:52>) | `[[nodiscard]] const AnimClip*                 GetOrNull(AnimId _id) const;` |
| [L53](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Render/Animation.h:53>) | `[[nodiscard]] const AnimClip*                 FindClip(StringView _name) const;` |
| [L54](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Render/Animation.h:54>) | `[[nodiscard]] size_t                          GetClipCount() const;` |
| [L55](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Render/Animation.h:55>) | `[[nodiscard]] Span<const AnimClip>            GetClips() const;` |
| [L56](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Render/Animation.h:56>) | `[[nodiscard]] Span<const String>              GetAtlasPages() const;` |
| [L57](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Render/Animation.h:57>) | `[[nodiscard]] const FilePath&                 GetRoot() const;` |
| [L68](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Render/Animation.h:68>) | `class AnimPlayer` |
| [L71](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Render/Animation.h:71>) | `void                           Play(AnimId _clip, bool _bRestart = false);` |
| [L72](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Render/Animation.h:72>) | `bool                           Update(const AnimationLibrary& _library, float _dt, float _speed = 1.f);` |
| [L73](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Render/Animation.h:73>) | `void                           SetNormalizedTime(const AnimClip& _clip, float _t);` |
| [L74](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Render/Animation.h:74>) | `void                           SyncPhase(const AnimClip& _clip, const AnimPlayer& _other, const AnimClip& _otherClip);` |
| [L75](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Render/Animation.h:75>) | `[[nodiscard]] AnimId           GetClip() const;` |
| [L76](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Render/Animation.h:76>) | `[[nodiscard]] int              GetFrame() const;` |
| [L77](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Render/Animation.h:77>) | `[[nodiscard]] int64_t          GetTimeUnits() const;` |
| [L78](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Render/Animation.h:78>) | `[[nodiscard]] bool             IsFinished() const;` |
| [L79](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Render/Animation.h:79>) | `[[nodiscard]] float            GetNormalizedTime(const AnimClip& _clip) const;` |
| [L80](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Render/Animation.h:80>) | `[[nodiscard]] const AnimFrame* GetFrameData(const AnimationLibrary& _library) const;` |
| [L83](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Render/Animation.h:83>) | `void Settle_(const AnimClip& _clip);` |

## Source/Render/Camera.cpp

[Camera.cpp](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Render/Camera.cpp>) · 654줄

| 위치 | 선언·함수 |
|---|---|
| [L23](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Render/Camera.cpp:23>) | `[[nodiscard]] float RoundToInt_( const float _value)` |
| [L29](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Render/Camera.cpp:29>) | `[[nodiscard]] float BrakeSpeed_( const float _gap)` |
| [L36](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Render/Camera.cpp:36>) | `[[nodiscard]] int MinMoveTicks_( const float _dist, const float _v0)` |
| [L48](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Render/Camera.cpp:48>) | `void SmoothDamp_( float& _current, float& _velocity, const float _target, const float _smoothTime, const float _dt, const float _maxSpeed)` |
| [L80](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Render/Camera.cpp:80>) | `[[nodiscard]] float ApplyDeadzone_( const float _center, const float _focus, const float _half)` |
| [L105](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Render/Camera.cpp:105>) | `void Camera::SetTuning( const CameraTuning& _tuning)` |
| [L111](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Render/Camera.cpp:111>) | `void Camera::SetBounds( const Aabb& _worldBounds)` |
| [L123](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Render/Camera.cpp:123>) | `void Camera::Lock( const Aabb& _lockBounds)` |
| [L151](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Render/Camera.cpp:151>) | `void Camera::Unlock()` |
| [L157](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Render/Camera.cpp:157>) | `void Camera::BeginMove( const VECTOR2 _targetCenter, const float _seconds)` |
| [L188](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Render/Camera.cpp:188>) | `void Camera::CancelMove()` |
| [L193](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Render/Camera.cpp:193>) | `void Camera::LockWithGlide( const Aabb& _lockBounds, const float _glideSeconds)` |
| [L208](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Render/Camera.cpp:208>) | `Aabb Camera::GetActiveBounds_( bool& _bHasBounds) const` |
| [L215](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Render/Camera.cpp:215>) | `bool Camera::GetTargetRect_( Aabb& _rect) const` |
| [L239](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Render/Camera.cpp:239>) | `void Camera::SyncEffectiveBounds_()` |
| [L245](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Render/Camera.cpp:245>) | `void Camera::ApproachEdge_( float& _edge, float& _vel, const float _target, const float _sign)` |
| [L267](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Render/Camera.cpp:267>) | `void Camera::UpdateEffectiveBounds_()` |
| [L289](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Render/Camera.cpp:289>) | `VECTOR2 Camera::ClampToRect_( const VECTOR2 _center, const Aabb& _bounds) const` |
| [L306](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Render/Camera.cpp:306>) | `VECTOR2 Camera::ClampCenter_( const VECTOR2 _center) const` |
| [L317](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Render/Camera.cpp:317>) | `VECTOR2 Camera::SnapTopLeft_( const VECTOR2 _center) const` |
| [L336](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Render/Camera.cpp:336>) | `void Camera::AddScrollFloor( const float _dx)` |
| [L347](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Render/Camera.cpp:347>) | `void Camera::ShiftLock( const float _dx)` |
| [L370](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Render/Camera.cpp:370>) | `void Camera::SnapTo( const VECTOR2 _viewCenter)` |
| [L384](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Render/Camera.cpp:384>) | `void Camera::SnapToTarget()` |
| [L389](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Render/Camera.cpp:389>) | `void Camera::StepX_( const float _desiredX, const float _focusDeltaX)` |
| [L449](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Render/Camera.cpp:449>) | `void Camera::Update( const float _dt)` |
| [L558](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Render/Camera.cpp:558>) | `VECTOR2 Camera::GetPosition() const` |
| [L563](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Render/Camera.cpp:563>) | `Aabb Camera::GetViewRect() const` |
| [L568](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Render/Camera.cpp:568>) | `const CameraTuning& Camera::GetTuning() const` |
| [L573](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Render/Camera.cpp:573>) | `bool Camera::IsLocked() const` |
| [L578](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Render/Camera.cpp:578>) | `void Camera::SetForwardOnly( const bool _bForwardOnly)` |
| [L584](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Render/Camera.cpp:584>) | `bool Camera::IsForwardOnly() const` |
| [L589](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Render/Camera.cpp:589>) | `void Camera::SetForwardOnlyY( const bool _bForwardOnlyY)` |
| [L595](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Render/Camera.cpp:595>) | `bool Camera::IsForwardOnlyY() const` |
| [L600](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Render/Camera.cpp:600>) | `bool Camera::IsMoving() const` |
| [L605](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Render/Camera.cpp:605>) | `bool Camera::HasPendingLock() const` |
| [L610](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Render/Camera.cpp:610>) | `const Aabb& Camera::GetBounds() const` |
| [L615](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Render/Camera.cpp:615>) | `const Aabb& Camera::GetLockBounds() const` |
| [L620](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Render/Camera.cpp:620>) | `void Camera::SetTarget( const VECTOR2 _target)` |
| [L626](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Render/Camera.cpp:626>) | `void Camera::ClearScrollFloor()` |
| [L631](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Render/Camera.cpp:631>) | `bool Camera::HasScrollFloor() const` |
| [L636](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Render/Camera.cpp:636>) | `float Camera::GetScrollFloorX() const` |
| [L641](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Render/Camera.cpp:641>) | `VECTOR2 Camera::GetViewCenter() const` |
| [L646](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Render/Camera.cpp:646>) | `CameraTuning MakeCameraTuning( const CameraSettings& _settings)` |

## Source/Render/Camera.h

[Camera.h](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Render/Camera.h>) · 100줄

| 위치 | 선언·함수 |
|---|---|
| [L8](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Render/Camera.h:8>) | `struct CameraTuning` |
| [L16](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Render/Camera.h:16>) | `class Camera` |
| [L19](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Render/Camera.h:19>) | `Camera() = default;` |
| [L20](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Render/Camera.h:20>) | `explicit Camera(const CameraTuning& _tuning);` |
| [L21](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Render/Camera.h:21>) | `void                              SetTuning(const CameraTuning& _tuning);` |
| [L22](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Render/Camera.h:22>) | `[[nodiscard]] const CameraTuning& GetTuning() const;` |
| [L23](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Render/Camera.h:23>) | `void                              SetBounds(const Aabb& _worldBounds);` |
| [L24](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Render/Camera.h:24>) | `void                              Lock(const Aabb& _lockBounds);` |
| [L25](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Render/Camera.h:25>) | `void                              Unlock();` |
| [L26](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Render/Camera.h:26>) | `[[nodiscard]] bool                IsLocked() const;` |
| [L27](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Render/Camera.h:27>) | `void                              SetForwardOnly(bool _bForwardOnly);` |
| [L28](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Render/Camera.h:28>) | `[[nodiscard]] bool                IsForwardOnly() const;` |
| [L29](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Render/Camera.h:29>) | `void                              SetForwardOnlyY(bool _bForwardOnlyY);` |
| [L30](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Render/Camera.h:30>) | `[[nodiscard]] bool                IsForwardOnlyY() const;` |
| [L31](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Render/Camera.h:31>) | `void                              BeginMove(VECTOR2 _targetCenter, float _seconds);` |
| [L32](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Render/Camera.h:32>) | `void                              CancelMove();` |
| [L33](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Render/Camera.h:33>) | `[[nodiscard]] bool                IsMoving() const;` |
| [L34](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Render/Camera.h:34>) | `void                              LockWithGlide(const Aabb& _lockBounds, float _glideSeconds);` |
| [L35](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Render/Camera.h:35>) | `[[nodiscard]] bool                HasPendingLock() const;` |
| [L36](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Render/Camera.h:36>) | `[[nodiscard]] const Aabb&         GetBounds() const;` |
| [L37](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Render/Camera.h:37>) | `[[nodiscard]] const Aabb&         GetLockBounds() const;` |
| [L38](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Render/Camera.h:38>) | `void                              SetTarget(VECTOR2 _target);` |
| [L39](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Render/Camera.h:39>) | `void                              AddScrollFloor(float _dx);` |
| [L42](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Render/Camera.h:42>) | `void                  ShiftLock(float _dx);` |
| [L43](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Render/Camera.h:43>) | `void                  ClearScrollFloor();` |
| [L44](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Render/Camera.h:44>) | `[[nodiscard]] bool    HasScrollFloor() const;` |
| [L45](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Render/Camera.h:45>) | `[[nodiscard]] float   GetScrollFloorX() const;` |
| [L46](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Render/Camera.h:46>) | `void                  SnapToTarget();` |
| [L47](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Render/Camera.h:47>) | `void                  SnapTo(VECTOR2 _viewCenter);` |
| [L48](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Render/Camera.h:48>) | `void                  Update(float _dt = kStepSec);` |
| [L49](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Render/Camera.h:49>) | `[[nodiscard]] VECTOR2 GetPosition() const;` |
| [L50](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Render/Camera.h:50>) | `[[nodiscard]] VECTOR2 GetViewCenter() const;` |
| [L51](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Render/Camera.h:51>) | `[[nodiscard]] Aabb    GetViewRect() const;` |
| [L54](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Render/Camera.h:54>) | `[[nodiscard]] Aabb    GetActiveBounds_(bool& _bHasBounds) const;` |
| [L55](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Render/Camera.h:55>) | `[[nodiscard]] VECTOR2 ClampCenter_(VECTOR2 _center) const;` |
| [L56](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Render/Camera.h:56>) | `[[nodiscard]] VECTOR2 ClampToRect_(VECTOR2 _center, const Aabb& _bounds) const;` |
| [L57](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Render/Camera.h:57>) | `[[nodiscard]] VECTOR2 SnapTopLeft_(VECTOR2 _center) const;` |
| [L58](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Render/Camera.h:58>) | `[[nodiscard]] bool    GetTargetRect_(Aabb& _rect) const;` |
| [L59](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Render/Camera.h:59>) | `void                  SyncEffectiveBounds_();` |
| [L60](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Render/Camera.h:60>) | `void                  UpdateEffectiveBounds_();` |
| [L61](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Render/Camera.h:61>) | `void                  ApproachEdge_(float& _edge, float& _vel, float _target, float _sign);` |
| [L62](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Render/Camera.h:62>) | `void                  StepX_(float _desiredX, float _focusDeltaX);` |
| [L100](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Render/Camera.h:100>) | `[[nodiscard]] CameraTuning MakeCameraTuning(const CameraSettings& _settings);` |

## Source/Render/Renderer.cpp

[Renderer.cpp](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Render/Renderer.cpp>) · 183줄

| 위치 | 선언·함수 |
|---|---|
| [L10](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Render/Renderer.cpp:10>) | `Renderer& Renderer::GetSingleton()` |
| [L21](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Render/Renderer.cpp:21>) | `Result<void> Renderer::Init( const char* _title, const int _windowScale)` |
| [L51](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Render/Renderer.cpp:51>) | `void Renderer::Shutdown()` |
| [L65](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Render/Renderer.cpp:65>) | `void Renderer::BeginFrame( const RGBA _clearColor)` |
| [L72](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Render/Renderer.cpp:72>) | `void Renderer::EndFrame()` |
| [L77](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Render/Renderer.cpp:77>) | `void Renderer::FillRect( const VECTOR2 _topLeft, const VECTOR2 _size, const RGBA _color)` |
| [L87](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Render/Renderer.cpp:87>) | `void Renderer::DrawRect( const VECTOR2 _topLeft, const VECTOR2 _size, const RGBA _color)` |
| [L97](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Render/Renderer.cpp:97>) | `void Renderer::DrawLine( const VECTOR2 _from, const VECTOR2 _to, const RGBA _color)` |
| [L106](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Render/Renderer.cpp:106>) | `void Renderer::DrawDebugText( const VECTOR2 _topLeft, const StringView _text, const RGBA _color)` |
| [L136](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Render/Renderer.cpp:136>) | `SDL_Renderer* Renderer::GetNative() const` |
| [L141](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Render/Renderer.cpp:141>) | `SDL_Window* Renderer::GetWindow() const` |
| [L146](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Render/Renderer.cpp:146>) | `Result<void> Renderer::SaveScreenshotBmp( const FilePath& _path) const` |
| [L167](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Render/Renderer.cpp:167>) | `VECTOR2 ToScreen( const VECTOR2 _world, const VECTOR2 _camera)` |
| [L174](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Render/Renderer.cpp:174>) | `void DrawBox( Renderer& _renderer, const Aabb& _box, const VECTOR2 _camera, const RGBA _color)` |

## Source/Render/Renderer.h

[Renderer.h](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Render/Renderer.h>) · 38줄

| 위치 | 선언·함수 |
|---|---|
| [L6](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Render/Renderer.h:6>) | `struct SDL_Window;` |
| [L7](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Render/Renderer.h:7>) | `struct SDL_Renderer;` |
| [L9](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Render/Renderer.h:9>) | `class Renderer` |
| [L15](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Render/Renderer.h:15>) | `~Renderer();` |
| [L16](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Render/Renderer.h:16>) | `[[nodiscard]] static Renderer& GetSingleton();` |
| [L17](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Render/Renderer.h:17>) | `[[nodiscard]] Result<void>     Init(const char* _title, int _windowScale);` |
| [L18](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Render/Renderer.h:18>) | `void                           Shutdown();` |
| [L19](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Render/Renderer.h:19>) | `void                           BeginFrame(RGBA _clearColor);` |
| [L20](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Render/Renderer.h:20>) | `void                           EndFrame();` |
| [L21](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Render/Renderer.h:21>) | `void                           FillRect(VECTOR2 _topLeft, VECTOR2 _size, RGBA _color);` |
| [L22](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Render/Renderer.h:22>) | `void                           DrawRect(VECTOR2 _topLeft, VECTOR2 _size, RGBA _color);` |
| [L23](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Render/Renderer.h:23>) | `void                           DrawLine(VECTOR2 _from, VECTOR2 _to, RGBA _color);` |
| [L26](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Render/Renderer.h:26>) | `void                        DrawDebugText(VECTOR2 _topLeft, StringView _text, RGBA _color);` |
| [L27](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Render/Renderer.h:27>) | `[[nodiscard]] SDL_Renderer* GetNative() const;` |
| [L28](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Render/Renderer.h:28>) | `[[nodiscard]] SDL_Window*   GetWindow() const;` |
| [L29](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Render/Renderer.h:29>) | `[[nodiscard]] Result<void>  SaveScreenshotBmp(const FilePath& _path) const;` |
| [L36](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Render/Renderer.h:36>) | `[[nodiscard]] VECTOR2 ToScreen(VECTOR2 _world, VECTOR2 _camera);` |
| [L38](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Render/Renderer.h:38>) | `void DrawBox(Renderer& _renderer, const Aabb& _box, VECTOR2 _camera, RGBA _color);` |

## Source/Render/SpriteBatch.cpp

[SpriteBatch.cpp](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Render/SpriteBatch.cpp>) · 872줄

| 위치 | 선언·함수 |
|---|---|
| [L23](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Render/SpriteBatch.cpp:23>) | `[[nodiscard]] SDL_FlipMode FlipFlags_( const SpriteDrawOptions& _options)` |
| [L31](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Render/SpriteBatch.cpp:31>) | `IntRect ComputeDestRect( const AnimFrame& _frame, const VECTOR2 _worldOrigin, const VECTOR2 _cameraPos, const bool _bFlipX, const bool _bFlipY)` |
| [L49](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Render/SpriteBatch.cpp:49>) | `void SortSpriteCommands( Vector<SpriteCommand>& _commands)` |
| [L65](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Render/SpriteBatch.cpp:65>) | `TextureViewRects ComputeTextureView( const int _texW, const int _texH, const VECTOR2 _texWorldPos, const VECTOR2 _cameraPos, const int _viewW, const int _viewH)` |
| [L101](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Render/SpriteBatch.cpp:101>) | `[[nodiscard]] Result<DecodedImage> DecodePngFile_( const FilePath& _pngPath)` |
| [L136](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Render/SpriteBatch.cpp:136>) | `[[nodiscard]] uint64_t HashRgba_( const Span<const uint8_t> _rgba)` |
| [L144](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Render/SpriteBatch.cpp:144>) | `void TraceAtlasPage_( const size_t _page, const FilePath& _path, const int _width, const int _height, const Span<const uint8_t> _rgba)` |
| [L157](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Render/SpriteBatch.cpp:157>) | `void TraceStandalone_( const int _handle, const FilePath& _path, const int _width, const int _height, const Span<const uint8_t> _rgba)` |
| [L171](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Render/SpriteBatch.cpp:171>) | `void SetLoadTraceEnabled( const bool _bEnabled)` |
| [L210](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Render/SpriteBatch.cpp:210>) | `uint8_t LerpChannel_( const int _from, const int _to, const int _t, const int _span)` |
| [L220](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Render/SpriteBatch.cpp:220>) | `Vector<uint8_t> BuildHitFlashRgba( const Span<const uint8_t> _rgba, const size_t _pixelCount)` |
| [L239](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Render/SpriteBatch.cpp:239>) | `[[nodiscard]] Result<DecodedImage> DecodeImage_( const FilePath& _path, const bool _bBuildFlash)` |
| [L258](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Render/SpriteBatch.cpp:258>) | `Vector<Result<DecodedImage>> DecodeImages( const Span<const FilePath> _paths, const bool _bBuildFlash, const int _numWorkers)` |
| [L281](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Render/SpriteBatch.cpp:281>) | `SDL_Texture* PickTexture_( SDL_Texture* const _pNormal, SDL_Texture* const _pFlash, const float _flashAmount)` |
| [L290](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Render/SpriteBatch.cpp:290>) | `RGBA ApplyHitFlashRamp( const RGBA _color)` |
| [L308](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Render/SpriteBatch.cpp:308>) | `SpriteBatch& SpriteBatch::GetSingleton()` |
| [L319](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Render/SpriteBatch.cpp:319>) | `void SpriteBatch::Init( SDL_Renderer* _pRenderer)` |
| [L326](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Render/SpriteBatch.cpp:326>) | `void SpriteBatch::Shutdown()` |
| [L354](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Render/SpriteBatch.cpp:354>) | `Result<void> SpriteBatch::AddAtlasPageFromRgba( const int _width, const int _height, const Span<const uint8_t> _rgba)` |
| [L367](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Render/SpriteBatch.cpp:367>) | `Result<void> SpriteBatch::AddAtlasPageFromRgba( const int _width, const int _height, const Span<const uint8_t> _rgba, const Span<const uint8_t> _flashRgba)` |
| [L416](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Render/SpriteBatch.cpp:416>) | `Result<void> SpriteBatch::AddAtlasPage( const DecodedImage& _image)` |
| [L426](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Render/SpriteBatch.cpp:426>) | `Result<int> SpriteBatch::AddStandaloneTexture( const DecodedImage& _image)` |
| [L432](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Render/SpriteBatch.cpp:432>) | `bool SpriteBatch::LoadAtlasPagesParallel( const AnimationLibrary& _library, const int _numWorkers)` |
| [L463](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Render/SpriteBatch.cpp:463>) | `Vector<int> SpriteBatch::LoadStandaloneTexturesParallel( const Span<const FilePath> _paths, const int _numWorkers)` |
| [L489](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Render/SpriteBatch.cpp:489>) | `Result<int> SpriteBatch::AddStandaloneTextureFromRgba( const int _width, const int _height, const Span<const uint8_t> _rgba)` |
| [L539](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Render/SpriteBatch.cpp:539>) | `void SpriteBatch::DrawStandaloneRect_( const StandaloneTexture& _texture, const SDL_FRect& _src, const SDL_FRect& _dst, const RGBA _tint) const` |
| [L570](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Render/SpriteBatch.cpp:570>) | `Result<int> SpriteBatch::LoadStandaloneTexture( const FilePath& _pngPath)` |
| [L586](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Render/SpriteBatch.cpp:586>) | `void SpriteBatch::DrawTexture( const int _handle, const VECTOR2 _texWorldPos, const VECTOR2 _cameraPos, const int _viewW, const int _viewH, const RGBA _tint) const` |
| [L611](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Render/SpriteBatch.cpp:611>) | `void SpriteBatch::DrawTextureRepeatX( const int _handle, const VECTOR2 _texWorldPos, const VECTOR2 _cameraPos, const int _viewW, const int _viewH, const float _fromX, const int _tileX, const int _tileW) const` |
| [L663](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Render/SpriteBatch.cpp:663>) | `void SpriteBatch::DrawFrame( const AnimFrame& _frame, const VECTOR2 _worldOrigin, const VECTOR2 _cameraPos, const SpriteDrawOptions& _options) const` |
| [L711](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Render/SpriteBatch.cpp:711>) | `void SpriteBatch::DrawFrame( const AnimFrame& _frame, const VECTOR2 _worldOrigin, const Camera& _camera, const SpriteDrawOptions& _options) const` |
| [L720](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Render/SpriteBatch.cpp:720>) | `void SpriteBatch::DrawFrameRect( const AnimFrame& _frame, const IntRect& _dest, const SpriteDrawOptions& _options) const` |
| [L745](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Render/SpriteBatch.cpp:745>) | `void SpriteBatch::Submit( const int _depth, const int _sortKey, const AnimFrame& _frame, const VECTOR2 _worldOrigin, const SpriteDrawOptions& _options)` |
| [L762](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Render/SpriteBatch.cpp:762>) | `void SpriteBatch::SetAssetRoot( const FilePath& _root)` |
| [L768](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Render/SpriteBatch.cpp:768>) | `int SpriteBatch::FindStandalone( const String& _relativeFile)` |
| [L787](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Render/SpriteBatch.cpp:787>) | `void SpriteBatch::SubmitStandalone( const int _depth, const int _sortKey, const int _handle, const IntRect& _src, const VECTOR2 _worldCenter, const SpriteDrawOptions& _options)` |
| [L806](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Render/SpriteBatch.cpp:806>) | `void SpriteBatch::DrawCommand_( const SpriteCommand& _command, const VECTOR2 _cameraPos) const` |
| [L827](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Render/SpriteBatch.cpp:827>) | `void SpriteBatch::Flush( const VECTOR2 _cameraPos)` |
| [L839](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Render/SpriteBatch.cpp:839>) | `void SpriteBatch::FlushBelowDepth( const VECTOR2 _cameraPos, const int _depth)` |
| [L853](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Render/SpriteBatch.cpp:853>) | `void SpriteBatch::Flush( const Camera& _camera)` |
| [L859](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Render/SpriteBatch.cpp:859>) | `size_t SpriteBatch::GetAtlasPageCount() const` |
| [L864](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Render/SpriteBatch.cpp:864>) | `void SpriteBatch::Clear()` |
| [L869](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Render/SpriteBatch.cpp:869>) | `const Vector<SpriteCommand>& SpriteBatch::GetPendingCommands() const` |

## Source/Render/SpriteBatch.h

[SpriteBatch.h](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Render/SpriteBatch.h>) · 148줄

| 위치 | 선언·함수 |
|---|---|
| [L9](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Render/SpriteBatch.h:9>) | `struct SDL_Renderer;` |
| [L10](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Render/SpriteBatch.h:10>) | `struct SDL_Texture;` |
| [L11](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Render/SpriteBatch.h:11>) | `struct SDL_FRect;` |
| [L13](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Render/SpriteBatch.h:13>) | `struct IntRect` |
| [L20](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Render/SpriteBatch.h:20>) | `[[nodiscard]] constexpr bool operator==(const IntRect&) const = default;` |
| [L23](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Render/SpriteBatch.h:23>) | `[[nodiscard]] IntRect ComputeDestRect(const AnimFrame& _frame, VECTOR2 _worldOrigin, VECTOR2 _cameraPos, bool _bFlipX, bool _bFlipY = false);` |
| [L25](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Render/SpriteBatch.h:25>) | `[[nodiscard]] RGBA ApplyHitFlashRamp(RGBA _color);` |
| [L27](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Render/SpriteBatch.h:27>) | `struct SpriteDrawOptions` |
| [L46](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Render/SpriteBatch.h:46>) | `struct SpriteCommand` |
| [L58](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Render/SpriteBatch.h:58>) | `void SortSpriteCommands(Vector<SpriteCommand>& _commands);` |
| [L60](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Render/SpriteBatch.h:60>) | `struct TextureViewRects` |
| [L68](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Render/SpriteBatch.h:68>) | `[[nodiscard]] Vector<uint8_t> BuildHitFlashRgba(Span<const uint8_t> _rgba, size_t _pixelCount);` |
| [L70](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Render/SpriteBatch.h:70>) | `struct DecodedImage` |
| [L78](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Render/SpriteBatch.h:78>) | `[[nodiscard]] Vector<Result<DecodedImage>> DecodeImages(Span<const FilePath> _paths, bool _bBuildFlash, int _numWorkers);` |
| [L80](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Render/SpriteBatch.h:80>) | `void SetLoadTraceEnabled(bool _bEnabled);` |
| [L82](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Render/SpriteBatch.h:82>) | `[[nodiscard]] TextureViewRects ComputeTextureView(int _texW, int _texH, VECTOR2 _texWorldPos, VECTOR2 _cameraPos, int _viewW, int _viewH);` |
| [L84](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Render/SpriteBatch.h:84>) | `class SpriteBatch` |
| [L87](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Render/SpriteBatch.h:87>) | `SpriteBatch() = default;` |
| [L88](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Render/SpriteBatch.h:88>) | `~SpriteBatch();` |
| [L89](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Render/SpriteBatch.h:89>) | `[[nodiscard]] static SpriteBatch& GetSingleton();` |
| [L90](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Render/SpriteBatch.h:90>) | `SpriteBatch(const SpriteBatch&)                          = delete;` |
| [L91](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Render/SpriteBatch.h:91>) | `SpriteBatch&               operator=(const SpriteBatch&) = delete;` |
| [L92](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Render/SpriteBatch.h:92>) | `void                       Init(SDL_Renderer* _pRenderer);` |
| [L93](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Render/SpriteBatch.h:93>) | `void                       Shutdown();` |
| [L94](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Render/SpriteBatch.h:94>) | `[[nodiscard]] Result<void> AddAtlasPageFromRgba(int _width, int _height, Span<const uint8_t> _rgba);` |
| [L95](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Render/SpriteBatch.h:95>) | `[[nodiscard]] Result<void> AddAtlasPageFromRgba(int _width, int _height, Span<const uint8_t> _rgba, Span<const uint8_t> _flashRgba);` |
| [L96](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Render/SpriteBatch.h:96>) | `[[nodiscard]] Result<void> AddAtlasPage(const DecodedImage& _image);` |
| [L98](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Render/SpriteBatch.h:98>) | `[[nodiscard]] bool        LoadAtlasPagesParallel(const AnimationLibrary& _library, int _numWorkers);` |
| [L99](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Render/SpriteBatch.h:99>) | `[[nodiscard]] size_t      GetAtlasPageCount() const;` |
| [L100](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Render/SpriteBatch.h:100>) | `[[nodiscard]] Result<int> LoadStandaloneTexture(const FilePath& _pngPath);` |
| [L101](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Render/SpriteBatch.h:101>) | `[[nodiscard]] Result<int> AddStandaloneTextureFromRgba(int _width, int _height, Span<const uint8_t> _rgba);` |
| [L102](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Render/SpriteBatch.h:102>) | `[[nodiscard]] Result<int> AddStandaloneTexture(const DecodedImage& _image);` |
| [L103](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Render/SpriteBatch.h:103>) | `[[nodiscard]] Vector<int> LoadStandaloneTexturesParallel(Span<const FilePath> _paths, int _numWorkers);` |
| [L104](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Render/SpriteBatch.h:104>) | `void                      DrawTexture(int _handle, VECTOR2 _texWorldPos, VECTOR2 _cameraPos, int _viewW, int _viewH, RGBA _tint = { 255, 255, 255, 255 }) const;` |
| [L105](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Render/SpriteBatch.h:105>) | `void                      DrawTextureRepeatX(int _handle, VECTOR2 _texWorldPos, VECTOR2 _cameraPos, int _viewW, int _viewH, float _fromX, int _tileX, int _tileW) const;` |
| [L106](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Render/SpriteBatch.h:106>) | `void                      SetAssetRoot(const FilePath& _root);` |
| [L107](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Render/SpriteBatch.h:107>) | `[[nodiscard]] int         FindStandalone(const String& _relativeFile);` |
| [L108](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Render/SpriteBatch.h:108>) | `void                      SubmitStandalone(int _depth, int _sortKey, int _handle, const IntRect& _src, VECTOR2 _worldCenter, const SpriteDrawOptions& _options = {});` |
| [L109](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Render/SpriteBatch.h:109>) | `void                      DrawFrame(const AnimFrame& _frame, VECTOR2 _worldOrigin, VECTOR2 _cameraPos, const SpriteDrawOptions& _options = {}) const;` |
| [L110](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Render/SpriteBatch.h:110>) | `void                      DrawFrame(const AnimFrame& _frame, VECTOR2 _worldOrigin, const Camera& _camera, const SpriteDrawOptions& _options = {}) const;` |
| [L111](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Render/SpriteBatch.h:111>) | `void                      DrawFrameRect(const AnimFrame& _frame, const IntRect& _dest, const SpriteDrawOptions& _options = {}) const;` |
| [L112](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Render/SpriteBatch.h:112>) | `void                      Submit(int _depth, int _sortKey, const AnimFrame& _frame, VECTOR2 _worldOrigin, const SpriteDrawOptions& _options = {});` |
| [L113](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Render/SpriteBatch.h:113>) | `void                      Flush(VECTOR2 _cameraPos);` |
| [L114](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Render/SpriteBatch.h:114>) | `void                      Flush(const Camera& _camera);` |
| [L117](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Render/SpriteBatch.h:117>) | `void                                       FlushBelowDepth(VECTOR2 _cameraPos, int _depth);` |
| [L118](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Render/SpriteBatch.h:118>) | `void                                       Clear();` |
| [L119](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Render/SpriteBatch.h:119>) | `[[nodiscard]] const Vector<SpriteCommand>& GetPendingCommands() const;` |
| [L122](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Render/SpriteBatch.h:122>) | `struct Page` |
| [L130](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Render/SpriteBatch.h:130>) | `struct StandaloneTexture` |
| [L138](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Render/SpriteBatch.h:138>) | `void DrawStandaloneRect_(const StandaloneTexture& _texture, const SDL_FRect& _src, const SDL_FRect& _dst, RGBA _tint) const;` |
| [L139](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Render/SpriteBatch.h:139>) | `void DrawCommand_(const SpriteCommand& _command, VECTOR2 _cameraPos) const;` |

## Source/Render/SpriteLibrary.cpp

[SpriteLibrary.cpp](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Render/SpriteLibrary.cpp>) · 102줄

| 위치 | 선언·함수 |
|---|---|
| [L9](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Render/SpriteLibrary.cpp:9>) | `[[nodiscard]] Result<SpriteLibrary> Parse_( const JsonReader& _root)` |
| [L57](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Render/SpriteLibrary.cpp:57>) | `Result<SpriteLibrary> SpriteLibrary::LoadFromJson( const StringView _json)` |
| [L68](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Render/SpriteLibrary.cpp:68>) | `Result<SpriteLibrary> SpriteLibrary::LoadFromDirectory( const FilePath& _bakedRoot)` |
| [L80](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Render/SpriteLibrary.cpp:80>) | `void SpriteLibrary::AddSprite( SpriteInfo _info)` |
| [L92](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Render/SpriteLibrary.cpp:92>) | `const SpriteInfo* SpriteLibrary::Find( const StringView _name) const` |
| [L99](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Render/SpriteLibrary.cpp:99>) | `size_t SpriteLibrary::GetCount() const` |

## Source/Render/SpriteLibrary.h

[SpriteLibrary.h](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Render/SpriteLibrary.h>) · 30줄

| 위치 | 선언·함수 |
|---|---|
| [L6](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Render/SpriteLibrary.h:6>) | `struct SpriteInfo` |
| [L16](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Render/SpriteLibrary.h:16>) | `class SpriteLibrary` |
| [L19](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Render/SpriteLibrary.h:19>) | `[[nodiscard]] static Result<SpriteLibrary> LoadFromJson(StringView _json);` |
| [L20](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Render/SpriteLibrary.h:20>) | `[[nodiscard]] static Result<SpriteLibrary> LoadFromDirectory(const FilePath& _bakedRoot);` |
| [L21](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Render/SpriteLibrary.h:21>) | `void                                       AddSprite(SpriteInfo _info);` |
| [L22](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Render/SpriteLibrary.h:22>) | `[[nodiscard]] const SpriteInfo*            Find(StringView _name) const;` |
| [L23](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/Render/SpriteLibrary.h:23>) | `[[nodiscard]] size_t                       GetCount() const;` |

## Source/World/Combat.cpp

[Combat.cpp](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/World/Combat.cpp>) · 200줄

| 위치 | 선언·함수 |
|---|---|
| [L9](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/World/Combat.cpp:9>) | `[[nodiscard]] uint64_t MakeTargetKey_( const Hurtbox& _hurtbox, const size_t _index)` |
| [L17](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/World/Combat.cpp:17>) | `Vector<HitEvent> HitResolver::Resolve( const Span<const Hitbox> _hitboxes, const Span<const Hurtbox> _hurtboxes)` |
| [L107](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/World/Combat.cpp:107>) | `void HitResolver::Release( const uint32_t _hitId)` |
| [L113](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/World/Combat.cpp:113>) | `void HitResolver::Clear()` |
| [L118](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/World/Combat.cpp:118>) | `size_t HitResolver::GetTrackedHitCount() const` |
| [L123](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/World/Combat.cpp:123>) | `HitKindMask AcceptsFor( const bool _bMeleeable)` |
| [L142](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/World/Combat.cpp:142>) | `HitKindMask HitKindMask::All()` |
| [L149](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/World/Combat.cpp:149>) | `bool HitKindMask::Has( const eHitKind _kind) const` |
| [L155](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/World/Combat.cpp:155>) | `uint32_t HitKindMask::Bit_( const eHitKind _kind)` |
| [L161](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/World/Combat.cpp:161>) | `bool CanTeamHit( const eTeam _attacker, const eTeam _target)` |
| [L174](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/World/Combat.cpp:174>) | `Health Health::Make( const int _maxHp)` |
| [L180](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/World/Combat.cpp:180>) | `bool Health::IsDead() const` |
| [L185](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/World/Combat.cpp:185>) | `bool Health::IsAlive() const` |
| [L190](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/World/Combat.cpp:190>) | `int Health::ApplyDamage( const int _damage)` |

## Source/World/Combat.h

[Combat.h](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/World/Combat.h>) · 105줄

| 위치 | 선언·함수 |
|---|---|
| [L8](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/World/Combat.h:8>) | `enum class eTeam` |
| [L15](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/World/Combat.h:15>) | `enum class eHitKind` |
| [L24](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/World/Combat.h:24>) | `struct HitKindMask` |
| [L26](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/World/Combat.h:26>) | `constexpr HitKindMask() = default;` |
| [L27](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/World/Combat.h:27>) | `HitKindMask(std::initializer_list<eHitKind> _kinds);` |
| [L28](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/World/Combat.h:28>) | `[[nodiscard]] static HitKindMask All();` |
| [L29](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/World/Combat.h:29>) | `[[nodiscard]] bool               Has(eHitKind _kind) const;` |
| [L32](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/World/Combat.h:32>) | `static uint32_t Bit_(eHitKind _kind);` |
| [L40](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/World/Combat.h:40>) | `[[nodiscard]] bool CanTeamHit(eTeam _attacker, eTeam _target);` |
| [L42](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/World/Combat.h:42>) | `struct Hitbox` |
| [L53](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/World/Combat.h:53>) | `struct Hurtbox` |
| [L58](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/World/Combat.h:58>) | `HitKindMask accepts       = HitKindMask::All();` |
| [L62](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/World/Combat.h:62>) | `[[nodiscard]] HitKindMask AcceptsFor(bool _bMeleeable);` |
| [L64](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/World/Combat.h:64>) | `struct HitEvent` |
| [L76](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/World/Combat.h:76>) | `class HitResolver` |
| [L79](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/World/Combat.h:79>) | `[[nodiscard]] Vector<HitEvent> Resolve(Span<const Hitbox> _hitboxes, Span<const Hurtbox> _hurtboxes);` |
| [L80](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/World/Combat.h:80>) | `void                           Release(uint32_t _hitId);` |
| [L81](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/World/Combat.h:81>) | `void                           Clear();` |
| [L82](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/World/Combat.h:82>) | `[[nodiscard]] size_t           GetTrackedHitCount() const;` |
| [L85](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/World/Combat.h:85>) | `struct HitRecord` |
| [L96](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/World/Combat.h:96>) | `struct Health` |
| [L101](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/World/Combat.h:101>) | `[[nodiscard]] static Health Make(int _maxHp);` |
| [L102](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/World/Combat.h:102>) | `[[nodiscard]] bool          IsDead() const;` |
| [L103](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/World/Combat.h:103>) | `[[nodiscard]] bool          IsAlive() const;` |
| [L104](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/World/Combat.h:104>) | `int                         ApplyDamage(int _damage);` |

## Source/World/Constants.h

[Constants.h](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/World/Constants.h>) · 35줄

| 위치 | 선언·함수 |
|---|---|
| [L10](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/World/Constants.h:10>) | `struct PhysicsTuning` |

## Source/World/Geometry.cpp

[Geometry.cpp](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/World/Geometry.cpp>) · 69줄

| 위치 | 선언·함수 |
|---|---|
| [L3](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/World/Geometry.cpp:3>) | `Aabb Aabb::FromMinMax( const VECTOR2 _min, const VECTOR2 _max)` |
| [L10](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/World/Geometry.cpp:10>) | `Aabb Aabb::FromMinSize( const VECTOR2 _min, const VECTOR2 _size)` |
| [L19](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/World/Geometry.cpp:19>) | `Aabb Aabb::FromCenterSize( const VECTOR2 _center, const VECTOR2 _size)` |
| [L30](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/World/Geometry.cpp:30>) | `Aabb Aabb::FromBottomCenter( const VECTOR2 _bottomCenter, const VECTOR2 _size)` |
| [L40](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/World/Geometry.cpp:40>) | `float Aabb::GetWidth() const` |
| [L45](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/World/Geometry.cpp:45>) | `float Aabb::GetHeight() const` |
| [L50](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/World/Geometry.cpp:50>) | `VECTOR2 Aabb::GetSize() const` |
| [L55](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/World/Geometry.cpp:55>) | `VECTOR2 Aabb::GetCenter() const` |
| [L60](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/World/Geometry.cpp:60>) | `bool Aabb::IsEmpty() const` |

## Source/World/Geometry.h

[Geometry.h](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/World/Geometry.h>) · 45줄

| 위치 | 선언·함수 |
|---|---|
| [L6](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/World/Geometry.h:6>) | `return static_cast<int>(Floor(_value));` |
| [L11](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/World/Geometry.h:11>) | `return static_cast<int>(Ceil(_value));` |
| [L14](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/World/Geometry.h:14>) | `struct Aabb` |
| [L19](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/World/Geometry.h:19>) | `[[nodiscard]] static Aabb FromMinMax(VECTOR2 _min, VECTOR2 _max);` |
| [L20](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/World/Geometry.h:20>) | `[[nodiscard]] static Aabb FromMinSize(VECTOR2 _min, VECTOR2 _size);` |
| [L21](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/World/Geometry.h:21>) | `[[nodiscard]] static Aabb FromCenterSize(VECTOR2 _center, VECTOR2 _size);` |
| [L22](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/World/Geometry.h:22>) | `[[nodiscard]] static Aabb FromBottomCenter(VECTOR2 _bottomCenter, VECTOR2 _size);` |
| [L23](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/World/Geometry.h:23>) | `[[nodiscard]] float       GetWidth() const;` |
| [L24](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/World/Geometry.h:24>) | `[[nodiscard]] float       GetHeight() const;` |
| [L25](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/World/Geometry.h:25>) | `[[nodiscard]] VECTOR2     GetSize() const;` |
| [L26](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/World/Geometry.h:26>) | `[[nodiscard]] VECTOR2     GetCenter() const;` |
| [L27](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/World/Geometry.h:27>) | `[[nodiscard]] bool        IsEmpty() const;` |
| [L44](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/World/Geometry.h:44>) | `[[nodiscard]] bool operator==(const Aabb& _other) const;` |

## Source/World/GroundMap.cpp

[GroundMap.cpp](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/World/GroundMap.cpp>) · 301줄

| 위치 | 선언·함수 |
|---|---|
| [L10](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/World/GroundMap.cpp:10>) | `[[nodiscard]] size_t StrideOf_( const int _width)` |
| [L17](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/World/GroundMap.cpp:17>) | `GroundMap GroundMap::MakeEmpty( const int _width, const int _height)` |
| [L31](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/World/GroundMap.cpp:31>) | `GroundMap GroundMap::MakeFromRows( const Span<const StringView> _rows, const char _solid)` |
| [L55](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/World/GroundMap.cpp:55>) | `void GroundMap::SetPeriodicTail( const int _from, const int _tailX, const int _tailW)` |
| [L66](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/World/GroundMap.cpp:66>) | `void GroundMap::MaterializeTail( const int _periods)` |
| [L96](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/World/GroundMap.cpp:96>) | `void GroundMap::SetSolid( const int _x, const int _y, const bool _bSolid)` |
| [L111](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/World/GroundMap.cpp:111>) | `void GroundMap::FillRect( const int _x0, const int _y0, const int _x1, const int _y1, const bool _bSolid)` |
| [L131](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/World/GroundMap.cpp:131>) | `uint64_t GroundMap::CountSolidPixels() const` |
| [L144](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/World/GroundMap.cpp:144>) | `bool GroundMap::IsOutOfBoundsSolid_( const int _x, const int _y) const` |
| [L151](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/World/GroundMap.cpp:151>) | `bool GroundMap::IsSolidAt( const VECTOR2 _point) const` |
| [L157](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/World/GroundMap.cpp:157>) | `bool GroundMap::IsRectSolid( const int _x0, const int _y0, const int _x1, const int _y1) const` |
| [L214](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/World/GroundMap.cpp:214>) | `std::optional<int> GroundMap::FindGroundY( const int _x, const int _fromY, const int _maxDist) const` |
| [L235](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/World/GroundMap.cpp:235>) | `float GroundMap::EstimateSlope( const int _x, const int _y, const int _halfWidth, const int _searchRange) const` |
| [L255](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/World/GroundMap.cpp:255>) | `VECTOR2 GroundMap::EstimateNormal( const int _x, const int _y, const int _halfWidth, const int _searchRange) const` |
| [L266](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/World/GroundMap.cpp:266>) | `int GroundMap::GetWidth() const` |
| [L271](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/World/GroundMap.cpp:271>) | `int GroundMap::GetHeight() const` |
| [L276](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/World/GroundMap.cpp:276>) | `void GroundMap::SetBoundsPolicy( const GroundBoundsPolicy& _policy)` |
| [L282](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/World/GroundMap.cpp:282>) | `bool GroundMap::HasPeriodicTail() const` |
| [L287](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/World/GroundMap.cpp:287>) | `bool GroundMap::IsSolid( const int _x, const int _y) const` |

## Source/World/GroundMap.h

[GroundMap.h](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/World/GroundMap.h>) · 51줄

| 위치 | 선언·함수 |
|---|---|
| [L8](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/World/GroundMap.h:8>) | `struct GroundBoundsPolicy` |
| [L16](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/World/GroundMap.h:16>) | `class GroundMap` |
| [L21](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/World/GroundMap.h:21>) | `[[nodiscard]] static GroundMap MakeEmpty(int _width, int _height);` |
| [L22](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/World/GroundMap.h:22>) | `[[nodiscard]] static GroundMap MakeFromRows(Span<const StringView> _rows, char _solid = '#');` |
| [L23](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/World/GroundMap.h:23>) | `GroundMap() = default;` |
| [L24](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/World/GroundMap.h:24>) | `void                             SetSolid(int _x, int _y, bool _bSolid);` |
| [L25](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/World/GroundMap.h:25>) | `void                             FillRect(int _x0, int _y0, int _x1, int _y1, bool _bSolid);` |
| [L26](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/World/GroundMap.h:26>) | `[[nodiscard]] int                GetWidth() const;` |
| [L27](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/World/GroundMap.h:27>) | `[[nodiscard]] int                GetHeight() const;` |
| [L28](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/World/GroundMap.h:28>) | `[[nodiscard]] uint64_t           CountSolidPixels() const;` |
| [L29](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/World/GroundMap.h:29>) | `void                             SetBoundsPolicy(const GroundBoundsPolicy& _policy);` |
| [L30](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/World/GroundMap.h:30>) | `void                             SetPeriodicTail(int _from, int _tailX, int _tailW);` |
| [L31](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/World/GroundMap.h:31>) | `[[nodiscard]] bool               HasPeriodicTail() const;` |
| [L32](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/World/GroundMap.h:32>) | `void                             MaterializeTail(int _periods);` |
| [L33](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/World/GroundMap.h:33>) | `[[nodiscard]] bool               IsSolid(int _x, int _y) const;` |
| [L34](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/World/GroundMap.h:34>) | `[[nodiscard]] bool               IsSolidAt(VECTOR2 _point) const;` |
| [L35](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/World/GroundMap.h:35>) | `[[nodiscard]] bool               IsRectSolid(int _x0, int _y0, int _x1, int _y1) const;` |
| [L36](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/World/GroundMap.h:36>) | `[[nodiscard]] std::optional<int> FindGroundY(int _x, int _fromY, int _maxDist) const;` |
| [L37](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/World/GroundMap.h:37>) | `[[nodiscard]] float              EstimateSlope(int _x, int _y, int _halfWidth, int _searchRange) const;` |
| [L38](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/World/GroundMap.h:38>) | `[[nodiscard]] VECTOR2            EstimateNormal(int _x, int _y, int _halfWidth, int _searchRange) const;` |
| [L41](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/World/GroundMap.h:41>) | `[[nodiscard]] bool IsOutOfBoundsSolid_(int _x, int _y) const;` |

## Source/World/Physics.cpp

[Physics.cpp](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/World/Physics.cpp>) · 561줄

| 위치 | 선언·함수 |
|---|---|
| [L32](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/World/Physics.cpp:32>) | `[[nodiscard]] float HalfWidthOf_( const Body& _body)` |
| [L38](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/World/Physics.cpp:38>) | `[[nodiscard]] float FootHalf_( const Body& _body)` |
| [L44](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/World/Physics.cpp:44>) | `[[nodiscard]] SupportResult FindSupport_( const StepContext& _ctx, const float _x, const float _yMin, const float _yMax, const float _yMinOneWay, const bool _bCheckBlocked)` |
| [L101](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/World/Physics.cpp:101>) | `else if (!body.bCollideSolids)` |
| [L146](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/World/Physics.cpp:146>) | `void TestLateralBlock_( const Aabb& _testBox, const Aabb& _block, const float _oldX, const float _half, const int _dir, std::optional<float>& _snapX)` |
| [L171](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/World/Physics.cpp:171>) | `void TestCeilingBlock_( const Aabb& _block, const float _left, const float _right, const float _top0, const float _top1, const float _bodyHeight, float& _stopFeet)` |
| [L190](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/World/Physics.cpp:190>) | `[[nodiscard]] std::optional<float> FindLateralBlock_( const StepContext& _ctx, const float _oldX, const float _newX, const float _feetY, const int _dir, const float _stepZone)` |
| [L258](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/World/Physics.cpp:258>) | `[[nodiscard]] bool IsHeadBlocked_( const StepContext& _ctx, const float _x, const float _oldFeet, const float _newFeet)` |
| [L305](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/World/Physics.cpp:305>) | `void SetSupport_( StepContext& _ctx, const SupportResult& _support)` |
| [L314](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/World/Physics.cpp:314>) | `[[nodiscard]] bool Resettle_( StepContext& _ctx, const float _x)` |
| [L345](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/World/Physics.cpp:345>) | `void MoveX_( StepContext& _ctx, const float _dx)` |
| [L387](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/World/Physics.cpp:387>) | `void MoveY_( StepContext& _ctx, const float _dy)` |
| [L469](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/World/Physics.cpp:469>) | `MoveResult Physics::Step( Body& _body, const PhysicsWorld& _world, const PhysicsTuning& _tuning, const float _dt)` |
| [L520](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/World/Physics.cpp:520>) | `else if (_world.pGround && result.platformIndex < 0 && result.wallIndex < 0)` |
| [L532](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/World/Physics.cpp:532>) | `bool Physics::SnapToGround( Body& _body, const PhysicsWorld& _world, const float _maxDist)` |
| [L552](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/World/Physics.cpp:552>) | `Aabb Body::GetBox() const` |
| [L557](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/World/Physics.cpp:557>) | `void Body::RequestDropThrough( const int _steps)` |

## Source/World/Physics.h

[Physics.h](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/World/Physics.h>) · 68줄

| 위치 | 선언·함수 |
|---|---|
| [L7](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/World/Physics.h:7>) | `struct Platform` |
| [L13](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/World/Physics.h:13>) | `struct Wall` |
| [L18](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/World/Physics.h:18>) | `struct PhysicsWorld` |
| [L25](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/World/Physics.h:25>) | `struct Body` |
| [L43](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/World/Physics.h:43>) | `[[nodiscard]] Aabb GetBox() const;` |
| [L44](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/World/Physics.h:44>) | `void               RequestDropThrough(int _steps = PhysicsTuning {}.dropThroughSteps);` |
| [L47](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/World/Physics.h:47>) | `struct MoveResult` |
| [L63](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/World/Physics.h:63>) | `class Physics` |
| [L66](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/World/Physics.h:66>) | `[[nodiscard]] static MoveResult Step(Body& _body, const PhysicsWorld& _world, const PhysicsTuning& _tuning = {}, float _dt = kStepSec);` |
| [L67](<C:/Users/Ahnjiwoo/Desktop/Projects/windowAPIProject(24.02.05)/MyMetalSlug2/Source/World/Physics.h:67>) | `static bool                     SnapToGround(Body& _body, const PhysicsWorld& _world, float _maxDist);` |

