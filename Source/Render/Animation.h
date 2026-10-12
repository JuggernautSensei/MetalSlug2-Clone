#pragma once
#include <cstdint>

#include "Base/Assert.h"
#include "Base/Handle.h"
#include "Base/Result.h"
#include "Base/Types.h"

inline constexpr int64_t kAnimTimeUnitsPerSec = 6'000'000;

struct AnimIdTag;
using AnimId = Handle<AnimIdTag>;

struct AnimFrame
{
    int atlas  = 0;   // 아틀라스 페이지 인덱스
    int x      = 0;   // 페이지 안의 트림된 사각형
    int y      = 0;
    int w      = 0;
    int h      = 0;
    int pivotX = 0;   // 엔티티 원점이 트림된 프레임 좌상단 기준 어디인가
    int pivotY = 0;

    [[nodiscard]] bool IsEmpty() const;
};

struct AnimClip
{
    String            name          = {};
    bool              bLoop         = false;
    int               loopStart     = 0;      // 루프 시 되돌아갈 프레임
    float             frameDuration = 0.1f;   // 초, 전 프레임 동일
    int               cellW         = 0;
    int               cellH         = 0;
    Vector<AnimFrame> frames        = {};

    [[nodiscard]] int GetFrameCount() const;

    [[nodiscard]] int64_t GetFrameDurationUnits() const;

    [[nodiscard]] int64_t GetTotalDurationUnits() const;

    [[nodiscard]] float GetTotalDurationSec() const;

    [[nodiscard]] int GetFootBaseline() const;
};

class AnimationLibrary
{
public:
    [[nodiscard]] static Result<AnimationLibrary> LoadFromDirectory(const FilePath& _bakedRoot);

    [[nodiscard]] static Result<AnimationLibrary> LoadFromJson(StringView _animJson);

    AnimId AddClip(AnimClip _clip);

    [[nodiscard]] AnimId Find(StringView _name) const;

    [[nodiscard]] bool            IsValid(AnimId _id) const;
    [[nodiscard]] const AnimClip& Get(AnimId _id) const;
    [[nodiscard]] const AnimClip* GetOrNull(AnimId _id) const;
    [[nodiscard]] const AnimClip* FindClip(StringView _name) const;

    [[nodiscard]] size_t               GetClipCount() const;
    [[nodiscard]] Span<const AnimClip> GetClips() const;

    [[nodiscard]] Span<const String> GetAtlasPages() const;
    [[nodiscard]] const FilePath&    GetRoot() const;

private:
    using NameMap = HashMap<String, uint32_t>;

    Vector<AnimClip> m_clips       = {};
    NameMap          m_nameToIndex = {};
    Vector<String>   m_atlasPages  = {};
    FilePath         m_root        = {};
};

class AnimPlayer
{
public:
    void Play(AnimId _clip, bool _bRestart = false);

    bool Update(const AnimationLibrary& _library, float _dt, float _speed = 1.f);

    void SetNormalizedTime(const AnimClip& _clip, float _t);

    void SyncPhase(const AnimClip& _clip, const AnimPlayer& _other, const AnimClip& _otherClip);

    [[nodiscard]] AnimId  GetClip() const;
    [[nodiscard]] int     GetFrame() const;
    [[nodiscard]] int64_t GetTimeUnits() const;
    [[nodiscard]] bool    IsFinished() const;
    [[nodiscard]] float   GetNormalizedTime(const AnimClip& _clip) const;

    [[nodiscard]] const AnimFrame* GetFrameData(const AnimationLibrary& _library) const;

private:
    void Settle_(const AnimClip& _clip);

    AnimId  m_clip      = kNullHandle;
    int64_t m_timeUnits = 0;
    int     m_frame     = 0;
    bool    m_bFinished = false;
};
