#include "Render/Animation.h"

#include <cmath>

#include <JugX/CoreLogger.h>
#include <JugX/JsonDeserializer.h>
#include <JugX/Math.h>

#include "Core/Error.h"

namespace slug
{

namespace
{
    constexpr double kUnitsPerSec = static_cast<double>(kAnimTimeUnitsPerSec);

    [[nodiscard]] int64_t ToUnits_(
        const double _sec)
    {
        return static_cast<int64_t>(std::llround(_sec * kUnitsPerSec));
    }

    // ===========================================
    //  JSON 헬퍼 (숫자는 모두 double 로 읽어 정수 형태 JSON 과 실수 형태 JSON 을 모두 허용)
    // ===========================================

    [[nodiscard]] bool ReadInt_(
        const jug::JsonReader& _object,
        const jug::StringView  _key,
        int&                   _out)
    {
        const jug::Result<double> value = _object.ReadField<double>(_key);
        if (!value)
        {
            return false;
        }
        _out = static_cast<int>(std::llround(value.GetValue()));
        return true;
    }

    [[nodiscard]] bool ReadOptionalInt_(
        const jug::JsonReader& _object,
        const jug::StringView  _key,
        int&                   _out)
    {
        return !_object.HasField(_key) || ReadInt_(_object, _key, _out);
    }

    [[nodiscard]] jug::Result<AnimFrame> ParseFrame_(
        const jug::JsonReader& _json)
    {
        if (!_json.IsObject())
        {
            return jug::Error { eError::Parse };
        }

        AnimFrame frame;
        if (!ReadInt_(_json, "atlas", frame.atlas) || !ReadInt_(_json, "x", frame.x) || !ReadInt_(_json, "y", frame.y) || !ReadInt_(_json, "w", frame.w) || !ReadInt_(_json, "h", frame.h) || !ReadInt_(_json, "pivotX", frame.pivotX) || !ReadInt_(_json, "pivotY", frame.pivotY))
        {
            return jug::Error { eError::Parse };
        }
        if (frame.atlas < 0 || frame.w < 0 || frame.h < 0)
        {
            return jug::Error { eError::InvalidData };
        }
        return frame;
    }

    [[nodiscard]] jug::Result<AnimClip> ParseClip_(
        const jug::StringView  _name,
        const jug::JsonReader& _json)
    {
        if (!_json.IsObject())
        {
            return jug::Error { eError::Parse };
        }

        AnimClip clip;
        clip.name = jug::String { _name };

        if (_json.HasField("loop"))
        {
            const jug::Result<bool> bLoop = _json.ReadField<bool>("loop");
            if (!bLoop)
            {
                return jug::Error { eError::Parse };
            }
            clip.bLoop = bLoop.GetValue();
        }

        if (!ReadOptionalInt_(_json, "loopStart", clip.loopStart) || !ReadOptionalInt_(_json, "cellW", clip.cellW) || !ReadOptionalInt_(_json, "cellH", clip.cellH))
        {
            return jug::Error { eError::Parse };
        }

        const jug::Result<double> duration = _json.ReadField<double>("frameDuration");
        if (!duration)
        {
            return jug::Error { eError::Parse };
        }
        clip.frameDuration = static_cast<float>(duration.GetValue());
        if (!(clip.frameDuration > 0.f))
        {
            return jug::Error { eError::InvalidData };
        }

        const jug::Result<jug::JsonReader> frames = _json.FindField("frames");
        if (!frames || !frames->IsArray())
        {
            return jug::Error { eError::Parse };
        }

        clip.frames.reserve(frames->GetSize());
        for (const jug::JsonReader frameJson: frames->GetArray())
        {
            jug::Result<AnimFrame> frame = ParseFrame_(frameJson);
            if (!frame)
            {
                return frame.GetError();
            }
            clip.frames.push_back(frame.GetValue());
        }

        if (clip.frames.empty())
        {
            return jug::Error { eError::InvalidData };
        }

        // 레거시 데이터에는 OptionIndex 가 프레임 수를 넘는 항목이 있다 (Baker 리포트 LoopStartOutOfRange).
        // 거부하면 라이브러리 전체가 로드되지 않으므로 마지막 프레임으로 클램프한다.
        if (clip.loopStart < 0 || clip.loopStart >= clip.GetFrameCount())
        {
            clip.loopStart = jug::Clamp(clip.loopStart, 0, clip.GetFrameCount() - 1);
        }
        return clip;
    }

    [[nodiscard]] jug::Result<AnimationLibrary> ParseAnimDocument_(
        const jug::JsonReader& _root)
    {
        if (!_root.IsObject())
        {
            return jug::Error { eError::Parse };
        }

        const jug::Result<jug::JsonReader> animations = _root.FindField("animations");
        if (!animations || !animations->IsObject())
        {
            return jug::Error { eError::Parse };
        }

        AnimationLibrary library;
        for (const auto [name, clipJson]: animations->GetObject())
        {
            jug::Result<AnimClip> clip = ParseClip_(name, clipJson);
            if (!clip)
            {
                JUG_CORE_LOG_ERROR("애니메이션 '{}' 파싱 실패: {}", name, clip.GetError().MakeMessage());
                return clip.GetError();
            }
            (void)library.AddClip(clip.Take());
        }
        return library;
    }

    [[nodiscard]] jug::Result<jug::Vector<jug::String>> ParseAtlasPages_(
        const jug::JsonReader& _manifest)
    {
        jug::Vector<jug::String> pages;
        if (!_manifest.HasField("atlasPages"))
        {
            return pages;
        }

        const jug::Result<jug::JsonReader> list = _manifest.FindField("atlasPages");
        if (!list || !list->IsArray())
        {
            return jug::Error { eError::Parse };
        }
        for (const jug::JsonReader item: list->GetArray())
        {
            jug::Result<jug::String> page = item.Read<jug::String>();
            if (!page)
            {
                return jug::Error { eError::Parse };
            }
            pages.push_back(page.GetValue());
        }
        return pages;
    }
}   // namespace

// ===========================================
//  AnimClip
// ===========================================

int64_t AnimClip::GetFrameDurationUnits() const
{
    return ToUnits_(static_cast<double>(frameDuration));
}

// ===========================================
//  AnimationLibrary
// ===========================================

jug::Result<AnimationLibrary> AnimationLibrary::LoadFromJson(
    const jug::StringView _animJson)
{
    jug::Result<jug::JsonDeserializer> doc = jug::JsonDeserializer::Load(_animJson);
    if (!doc)
    {
        return jug::Error { eError::Parse };
    }
    return ParseAnimDocument_(doc->GetReader());
}

jug::Result<AnimationLibrary> AnimationLibrary::LoadFromDirectory(
    const jug::FilePath& _bakedRoot)
{
    // --- manifest.json ---
    jug::Result<jug::JsonDeserializer> manifestDoc = jug::JsonDeserializer::LoadFromFile(_bakedRoot / "manifest.json");
    if (!manifestDoc)
    {
        return jug::Error { eError::FileOpen };
    }
    const jug::JsonReader manifest = manifestDoc->GetReader();
    if (!manifest.IsObject())
    {
        return jug::Error { eError::Parse };
    }

    jug::String animFile = "anim.json";
    if (manifest.HasField("animations"))
    {
        jug::Result<jug::String> name = manifest.ReadField<jug::String>("animations");
        if (!name)
        {
            return jug::Error { eError::Parse };
        }
        animFile = name.GetValue();
    }

    jug::Result<jug::Vector<jug::String>> pages = ParseAtlasPages_(manifest);
    if (!pages)
    {
        return pages.GetError();
    }

    // --- anim.json ---
    jug::Result<jug::JsonDeserializer> animDoc = jug::JsonDeserializer::LoadFromFile(_bakedRoot / animFile);
    if (!animDoc)
    {
        return jug::Error { eError::FileOpen };
    }

    jug::Result<AnimationLibrary> library = ParseAnimDocument_(animDoc->GetReader());
    if (!library)
    {
        return library.GetError();
    }

    AnimationLibrary result = library.Take();
    result.m_atlasPages     = pages.Take();
    result.m_root           = _bakedRoot;
    return result;
}

AnimId AnimationLibrary::AddClip(
    AnimClip _clip)
{
    if (const auto it = m_nameToIndex.find(_clip.name); it != m_nameToIndex.end())
    {
        m_clips[it->second] = std::move(_clip);
        return AnimId { it->second, 0 };
    }

    const uint32_t index = static_cast<uint32_t>(m_clips.size());
    m_nameToIndex.emplace(_clip.name, index);
    m_clips.push_back(std::move(_clip));
    return AnimId { index, 0 };
}

AnimId AnimationLibrary::Find(
    const jug::StringView _name) const
{
    const auto it = m_nameToIndex.find(jug::String { _name });
    if (it == m_nameToIndex.end())
    {
        return jug::kNullHandle;
    }
    return AnimId { it->second, 0 };
}

bool AnimationLibrary::IsValid(
    const AnimId _id) const
{
    return !_id.IsNull() && _id.GetIndex() < m_clips.size();
}

const AnimClip& AnimationLibrary::Get(
    const AnimId _id) const
{
    JUG_ASSERT(IsValid(_id), "유효하지 않은 AnimId 이다.\n");
    return m_clips[_id.GetIndex()];
}

const AnimClip* AnimationLibrary::GetOrNull(
    const AnimId _id) const
{
    return IsValid(_id) ? &m_clips[_id.GetIndex()] : nullptr;
}

const AnimClip* AnimationLibrary::FindClip(
    const jug::StringView _name) const
{
    return GetOrNull(Find(_name));
}

// ===========================================
//  AnimPlayer
// ===========================================

void AnimPlayer::Play(
    const AnimId _clip,
    const bool   _bRestart)
{
    if (_clip == m_clip && !_bRestart)
    {
        return;
    }

    m_clip      = _clip;
    m_timeUnits = 0;
    m_frame     = 0;
    m_bFinished = false;
}

bool AnimPlayer::Update(
    const AnimationLibrary& _library,
    const float             _dt,
    const float             _speed)
{
    const AnimClip* pClip = _library.GetOrNull(m_clip);
    if (!pClip || m_bFinished)
    {
        return false;
    }

    const int prevFrame = m_frame;
    m_timeUnits += ToUnits_(static_cast<double>(_dt) * static_cast<double>(_speed));
    Settle_(*pClip);
    return m_frame != prevFrame;
}

void AnimPlayer::Settle_(
    const AnimClip& _clip)
{
    const int     count         = _clip.GetFrameCount();
    const int64_t durationUnits = _clip.GetFrameDurationUnits();
    if (count <= 0 || durationUnits <= 0)
    {
        m_timeUnits = 0;
        m_frame     = 0;
        m_bFinished = !_clip.bLoop;
        return;
    }

    const int64_t totalUnits = durationUnits * count;
    if (m_timeUnits < 0)
    {
        m_timeUnits = 0;
    }

    if (m_timeUnits >= totalUnits)
    {
        if (_clip.bLoop)
        {
            const int     loopStart      = jug::Clamp(_clip.loopStart, 0, count - 1);
            const int64_t loopStartUnits = durationUnits * loopStart;
            const int64_t loopLenUnits   = totalUnits - loopStartUnits;
            m_timeUnits                  = loopStartUnits + (m_timeUnits - loopStartUnits) % loopLenUnits;
            m_bFinished                  = false;
        }
        else
        {
            m_timeUnits = totalUnits;
            m_bFinished = true;
        }
    }
    else
    {
        m_bFinished = false;
    }

    m_frame = m_bFinished ? count - 1 : static_cast<int>(m_timeUnits / durationUnits);
}

void AnimPlayer::SetNormalizedTime(
    const AnimClip& _clip,
    const float     _t)
{
    const int64_t totalUnits = _clip.GetTotalDurationUnits();
    double        t          = static_cast<double>(_t);
    if (_clip.bLoop)
    {
        t -= std::floor(t);
    }
    else
    {
        t = jug::Clamp(t, 0.0, 1.0);
    }

    m_bFinished = false;
    m_timeUnits = static_cast<int64_t>(std::llround(t * static_cast<double>(totalUnits)));
    Settle_(_clip);
}

void AnimPlayer::SyncPhase(
    const AnimClip&   _clip,
    const AnimPlayer& _other,
    const AnimClip&   _otherClip)
{
    const int64_t otherTotalUnits = _otherClip.GetTotalDurationUnits();
    const int64_t totalUnits      = _clip.GetTotalDurationUnits();
    if (otherTotalUnits <= 0 || totalUnits <= 0)
    {
        return;
    }

    // 정수 비례식: 반올림 오차 없이 같은 비율 위치로 옮긴다.
    m_bFinished = false;
    m_timeUnits = _other.m_timeUnits * totalUnits / otherTotalUnits;
    Settle_(_clip);
    if (_other.m_bFinished && !_clip.bLoop)
    {
        m_timeUnits = totalUnits;
        m_bFinished = true;
        m_frame     = _clip.GetFrameCount() - 1;
    }
}

float AnimPlayer::GetNormalizedTime(
    const AnimClip& _clip) const
{
    const int64_t totalUnits = _clip.GetTotalDurationUnits();
    if (totalUnits <= 0)
    {
        return 0.f;
    }
    return static_cast<float>(static_cast<double>(m_timeUnits) / static_cast<double>(totalUnits));
}

const AnimFrame* AnimPlayer::GetFrameData(
    const AnimationLibrary& _library) const
{
    const AnimClip* pClip = _library.GetOrNull(m_clip);
    if (!pClip || m_frame < 0 || m_frame >= pClip->GetFrameCount())
    {
        return nullptr;
    }
    return &pClip->frames[static_cast<size_t>(m_frame)];
}

}   // namespace slug
