#pragma once
#include <cstdint>

#include <JugX/Handle.h>
#include <JugX/Result.h>
#include <JugX/Typedef.h>

namespace slug
{

// ===========================================
//  AnimId
//   라이브러리 안의 클립을 가리키는 값 타입 핸들 (이름 → 정수 인터닝).
//   기본 생성은 null 핸들이다.
// ===========================================

// 애니메이션 시간 단위: 1/6,000,000 초. 1/60 초(고정 스텝)가 정확히 100,000 단위이므로 60Hz 누적에 오차가 없다.
inline constexpr int64_t kAnimTimeUnitsPerSec = 6'000'000;

struct AnimIdTag;
using AnimId = jug::Handle<AnimIdTag>;

// ===========================================
//  AnimFrame / AnimClip
//   Docs/AssetFormat.md 의 anim.json 과 1:1 대응.
// ===========================================

struct AnimFrame
{
    int atlas  = 0;   // 아틀라스 페이지 인덱스
    int x      = 0;   // 페이지 안의 트림된 사각형
    int y      = 0;
    int w      = 0;
    int h      = 0;
    int pivotX = 0;   // 엔티티 원점이 트림된 프레임 좌상단 기준 어디인가
    int pivotY = 0;

    [[nodiscard]] bool IsEmpty() const
    {
        return w <= 0 || h <= 0;   // 완전 투명 프레임
    }
};

struct AnimClip
{
    jug::String            name          = {};
    bool                   bLoop         = false;
    int                    loopStart     = 0;      // 루프 시 되돌아갈 프레임
    float                  frameDuration = 0.1f;   // 초, 전 프레임 동일
    int                    cellW         = 0;
    int                    cellH         = 0;
    jug::Vector<AnimFrame> frames        = {};

    [[nodiscard]] int GetFrameCount() const
    {
        return static_cast<int>(frames.size());
    }

    // 프레임 하나의 길이를 정수 시간 단위(kAnimTimeUnitsPerSec)로 환산한 값. 시간 누적을 정수로 해서 오차가 쌓이지 않게 한다.
    [[nodiscard]] int64_t GetFrameDurationUnits() const;

    [[nodiscard]] int64_t GetTotalDurationUnits() const
    {
        return GetFrameDurationUnits() * static_cast<int64_t>(frames.size());
    }

    [[nodiscard]] float GetTotalDurationSec() const
    {
        return static_cast<float>(static_cast<double>(GetTotalDurationUnits()) / static_cast<double>(kAnimTimeUnitsPerSec));
    }
};

// ===========================================
//  AnimationLibrary
//   anim.json + manifest.json 을 읽어 클립을 보관한다. 읽기 전용으로 쓰이며 AnimId 는 클립 인덱스이다.
//   클립 추가는 로드 시점에만 하고, 이후 참조(AnimId, const AnimClip&)는 AddClip 호출 전까지만 유효하다고 가정한다.
// ===========================================

class AnimationLibrary
{
public:
    // Assets/Baked 같은 루트 폴더에서 manifest.json 과 그가 가리키는 anim.json 을 읽는다.
    [[nodiscard]] static jug::Result<AnimationLibrary> LoadFromDirectory(const jug::FilePath& _bakedRoot);

    // anim.json 텍스트만 파싱한다 (테스트/도구용). 아틀라스 페이지 목록은 비어 있다.
    [[nodiscard]] static jug::Result<AnimationLibrary> LoadFromJson(jug::StringView _animJson);

    // 프로그램에서 직접 클립을 추가한다. 같은 이름이 있으면 내용만 교체하고 같은 id 를 돌려준다.
    AnimId AddClip(AnimClip _clip);

    [[nodiscard]] AnimId Find(jug::StringView _name) const;   // 없으면 null 핸들

    [[nodiscard]] bool            IsValid(AnimId _id) const;
    [[nodiscard]] const AnimClip& Get(AnimId _id) const;   // 유효한 id 필요 (JUG_ASSERT)
    [[nodiscard]] const AnimClip* GetOrNull(AnimId _id) const;
    [[nodiscard]] const AnimClip* FindClip(jug::StringView _name) const;

    [[nodiscard]] size_t                    GetClipCount() const { return m_clips.size(); }
    [[nodiscard]] jug::Span<const AnimClip> GetClips() const { return m_clips; }

    [[nodiscard]] jug::Span<const jug::String> GetAtlasPages() const { return m_atlasPages; }
    [[nodiscard]] const jug::FilePath&         GetRoot() const { return m_root; }

    void SetAtlasPages(jug::Vector<jug::String> _pages) { m_atlasPages = std::move(_pages); }

private:
    jug::Vector<AnimClip>               m_clips       = {};
    jug::HashMap<jug::String, uint32_t> m_nameToIndex = {};
    jug::Vector<jug::String>            m_atlasPages  = {};
    jug::FilePath                       m_root        = {};
};

// ===========================================
//  AnimPlayer
//   클립 하나를 재생하는 값 타입 재생기. 시간은 정수 단위(1/6,000,000 초)로 누적하므로 dt = 1/60 을
//   몇 번을 더해도 프레임 경계가 어긋나지 않는다.
//   프레임 i 는 시간 [i*frameDuration, (i+1)*frameDuration) 동안 보인다.
//   루프 클립은 끝에 닿으면 loopStart 프레임으로 되돌아간다. 비루프 클립은 마지막 프레임에서 멈추고 IsFinished() 가 true.
// ===========================================

class AnimPlayer
{
public:
    // 다른 클립으로 바꾸거나(_bRestart 무관) 같은 클립을 처음부터 다시 재생한다(_bRestart = true).
    // 이미 같은 클립이 재생 중이고 _bRestart 가 false 면 아무것도 하지 않는다.
    void Play(AnimId _clip, bool _bRestart = false);

    // 시간을 진행한다. 프레임이 바뀌었으면 true. _speed 는 재생 배속.
    bool Update(const AnimationLibrary& _library, float _dt, float _speed = 1.f);

    // 클립 전체 길이 대비 비율 [0, 1] 로 위치를 지정한다. 루프 클립은 소수부만 사용한다.
    void SetNormalizedTime(const AnimClip& _clip, float _t);

    // 다른 재생기와 같은 위상(전체 길이 대비 같은 비율)으로 맞춘다. 상체/하체가 하나의 타임라인을 공유할 때 사용.
    void SyncPhase(const AnimClip& _clip, const AnimPlayer& _other, const AnimClip& _otherClip);

    [[nodiscard]] AnimId  GetClip() const { return m_clip; }
    [[nodiscard]] int     GetFrame() const { return m_frame; }
    [[nodiscard]] int64_t GetTimeUnits() const { return m_timeUnits; }
    [[nodiscard]] float   GetTimeSec() const { return static_cast<float>(static_cast<double>(m_timeUnits) / static_cast<double>(kAnimTimeUnitsPerSec)); }
    [[nodiscard]] bool    IsFinished() const { return m_bFinished; }
    [[nodiscard]] float   GetNormalizedTime(const AnimClip& _clip) const;

    // 현재 프레임 데이터. 클립이 없거나 프레임이 없으면 nullptr.
    [[nodiscard]] const AnimFrame* GetFrameData(const AnimationLibrary& _library) const;

private:
    void Settle_(const AnimClip& _clip);   // m_timeUnits 를 클립 규칙(루프/종료)에 맞게 정리하고 m_frame 을 갱신

    AnimId  m_clip      = jug::kNullHandle;
    int64_t m_timeUnits = 0;
    int     m_frame     = 0;
    bool    m_bFinished = false;
};

}   // namespace slug
