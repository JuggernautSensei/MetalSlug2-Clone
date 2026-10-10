#pragma once
#include <cstdint>

#include "Base/Assert.h"
#include "Base/Handle.h"
#include "Base/Result.h"
#include "Base/Types.h"

namespace slug
{

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

    [[nodiscard]] bool IsEmpty() const
    {
        return w <= 0 || h <= 0;   // 완전 투명 프레임
    }
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

    [[nodiscard]] int GetFrameCount() const
    {
        return static_cast<int>(frames.size());
    }

    [[nodiscard]] int64_t GetFrameDurationUnits() const;

    [[nodiscard]] int64_t GetTotalDurationUnits() const
    {
        return GetFrameDurationUnits() * static_cast<int64_t>(frames.size());
    }

    [[nodiscard]] float GetTotalDurationSec() const
    {
        return static_cast<float>(static_cast<double>(GetTotalDurationUnits()) / static_cast<double>(kAnimTimeUnitsPerSec));
    }

    [[nodiscard]] int GetFootBaseline() const
    {
        int baseline = 0;
        for (const AnimFrame& frame: frames)
        {
            if (!frame.IsEmpty() && frame.h - frame.pivotY > baseline)
            {
                baseline = frame.h - frame.pivotY;
            }
        }
        return baseline;
    }
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

    [[nodiscard]] size_t               GetClipCount() const { return m_clips.size(); }
    [[nodiscard]] Span<const AnimClip> GetClips() const { return m_clips; }

    [[nodiscard]] Span<const String> GetAtlasPages() const { return m_atlasPages; }
    [[nodiscard]] const FilePath&    GetRoot() const { return m_root; }

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

    [[nodiscard]] AnimId  GetClip() const { return m_clip; }
    [[nodiscard]] int     GetFrame() const { return m_frame; }
    [[nodiscard]] int64_t GetTimeUnits() const { return m_timeUnits; }
    [[nodiscard]] bool    IsFinished() const { return m_bFinished; }
    [[nodiscard]] float   GetNormalizedTime(const AnimClip& _clip) const;

    [[nodiscard]] const AnimFrame* GetFrameData(const AnimationLibrary& _library) const;

private:
    void Settle_(const AnimClip& _clip);

    AnimId  m_clip      = kNullHandle;
    int64_t m_timeUnits = 0;
    int     m_frame     = 0;
    bool    m_bFinished = false;
};

}   // namespace slug
