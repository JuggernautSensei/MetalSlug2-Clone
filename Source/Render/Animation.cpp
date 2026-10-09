#include "Render/Animation.h"
#include "Base/Types.h"
#include "Base/Assert.h"
#include "Base/Handle.h"

#include <cmath>

#include "Base/Log.h"
#include "Base/Json.h"
#include "Base/Math.h"

#include "Base/Error.h"

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
        const slug::JsonReader& _object,
        const slug::StringView  _key,
        int&                   _out)
    {
        const slug::Result<double> value = _object.ReadField<double>(_key);
        if (!value)
        {
            return false;
        }
        _out = static_cast<int>(std::llround(value.GetValue()));
        return true;
    }

    [[nodiscard]] bool ReadOptionalInt_(
        const slug::JsonReader& _object,
        const slug::StringView  _key,
        int&                   _out)
    {
        return !_object.HasField(_key) || ReadInt_(_object, _key, _out);
    }

    [[nodiscard]] slug::Result<AnimFrame> ParseFrame_(
        const slug::JsonReader& _json)
    {
        if (!_json.IsObject())
        {
            return slug::Error { eError::Parse };
        }

        AnimFrame frame;
        if (!ReadInt_(_json, "atlas", frame.atlas) || !ReadInt_(_json, "x", frame.x) || !ReadInt_(_json, "y", frame.y) || !ReadInt_(_json, "w", frame.w) || !ReadInt_(_json, "h", frame.h) || !ReadInt_(_json, "pivotX", frame.pivotX) || !ReadInt_(_json, "pivotY", frame.pivotY))
        {
            return slug::Error { eError::Parse };
        }
        if (frame.atlas < 0 || frame.w < 0 || frame.h < 0)
        {
            return slug::Error { eError::InvalidData };
        }
        return frame;
    }

    [[nodiscard]] slug::Result<AnimClip> ParseClip_(
        const slug::StringView  _name,
        const slug::JsonReader& _json)
    {
        if (!_json.IsObject())
        {
            return slug::Error { eError::Parse };
        }

        AnimClip clip;
        clip.name = slug::String { _name };

        if (_json.HasField("loop"))
        {
            const slug::Result<bool> bLoop = _json.ReadField<bool>("loop");
            if (!bLoop)
            {
                return slug::Error { eError::Parse };
            }
            clip.bLoop = bLoop.GetValue();
        }

        if (!ReadOptionalInt_(_json, "loopStart", clip.loopStart) || !ReadOptionalInt_(_json, "cellW", clip.cellW) || !ReadOptionalInt_(_json, "cellH", clip.cellH))
        {
            return slug::Error { eError::Parse };
        }

        const slug::Result<double> duration = _json.ReadField<double>("frameDuration");
        if (!duration)
        {
            return slug::Error { eError::Parse };
        }
        clip.frameDuration = static_cast<float>(duration.GetValue());
        if (!(clip.frameDuration > 0.f))
        {
            return slug::Error { eError::InvalidData };
        }

        const slug::Result<slug::JsonReader> frames = _json.FindField("frames");
        if (!frames || !frames->IsArray())
        {
            return slug::Error { eError::Parse };
        }

        clip.frames.reserve(frames->GetSize());
        for (const slug::JsonReader frameJson: frames->GetArray())
        {
            slug::Result<AnimFrame> frame = ParseFrame_(frameJson);
            if (!frame)
            {
                return frame.GetError();
            }
            clip.frames.push_back(frame.GetValue());
        }

        if (clip.frames.empty())
        {
            return slug::Error { eError::InvalidData };
        }

        // 레거시 데이터에는 OptionIndex 가 프레임 수를 넘는 항목이 있다 (Baker 리포트 LoopStartOutOfRange).
        // 거부하면 라이브러리 전체가 로드되지 않으므로 마지막 프레임으로 클램프한다.
        if (clip.loopStart < 0 || clip.loopStart >= clip.GetFrameCount())
        {
            clip.loopStart = slug::Clamp(clip.loopStart, 0, clip.GetFrameCount() - 1);
        }
        return clip;
    }

    [[nodiscard]] slug::Result<AnimationLibrary> ParseAnimDocument_(
        const slug::JsonReader& _root)
    {
        if (!_root.IsObject())
        {
            return slug::Error { eError::Parse };
        }

        const slug::Result<slug::JsonReader> animations = _root.FindField("animations");
        if (!animations || !animations->IsObject())
        {
            return slug::Error { eError::Parse };
        }

        AnimationLibrary library;
        for (const auto [name, clipJson]: animations->GetObject())
        {
            slug::Result<AnimClip> clip = ParseClip_(name, clipJson);
            if (!clip)
            {
                SLUG_LOG_ERROR("애니메이션 '{}' 파싱 실패: {}", name, clip.GetError().MakeMessage());
                return clip.GetError();
            }
            (void)library.AddClip(clip.Take());
        }
        return library;
    }

    [[nodiscard]] slug::Result<slug::Vector<slug::String>> ParseAtlasPages_(
        const slug::JsonReader& _manifest)
    {
        slug::Vector<slug::String> pages;
        if (!_manifest.HasField("atlasPages"))
        {
            return pages;
        }

        const slug::Result<slug::JsonReader> list = _manifest.FindField("atlasPages");
        if (!list || !list->IsArray())
        {
            return slug::Error { eError::Parse };
        }
        for (const slug::JsonReader item: list->GetArray())
        {
            slug::Result<slug::String> page = item.Read<slug::String>();
            if (!page)
            {
                return slug::Error { eError::Parse };
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

slug::Result<AnimationLibrary> AnimationLibrary::LoadFromJson(
    const slug::StringView _animJson)
{
    slug::Result<slug::JsonDeserializer> doc = slug::JsonDeserializer::Load(_animJson);
    if (!doc)
    {
        return slug::Error { eError::Parse };
    }
    return ParseAnimDocument_(doc->GetReader());
}

slug::Result<AnimationLibrary> AnimationLibrary::LoadFromDirectory(
    const slug::FilePath& _bakedRoot)
{
    // --- manifest.json ---
    slug::Result<slug::JsonDeserializer> manifestDoc = slug::JsonDeserializer::LoadFromFile(_bakedRoot / "manifest.json");
    if (!manifestDoc)
    {
        return slug::Error { eError::FileOpen };
    }
    const slug::JsonReader manifest = manifestDoc->GetReader();
    if (!manifest.IsObject())
    {
        return slug::Error { eError::Parse };
    }

    slug::String animFile = "anim.json";
    if (manifest.HasField("animations"))
    {
        slug::Result<slug::String> name = manifest.ReadField<slug::String>("animations");
        if (!name)
        {
            return slug::Error { eError::Parse };
        }
        animFile = name.GetValue();
    }

    slug::Result<slug::Vector<slug::String>> pages = ParseAtlasPages_(manifest);
    if (!pages)
    {
        return pages.GetError();
    }

    // --- anim.json ---
    slug::Result<slug::JsonDeserializer> animDoc = slug::JsonDeserializer::LoadFromFile(_bakedRoot / animFile);
    if (!animDoc)
    {
        return slug::Error { eError::FileOpen };
    }

    slug::Result<AnimationLibrary> library = ParseAnimDocument_(animDoc->GetReader());
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
    const slug::StringView _name) const
{
    const auto it = m_nameToIndex.find(slug::String { _name });
    if (it == m_nameToIndex.end())
    {
        return slug::kNullHandle;
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
    SLUG_ASSERT(IsValid(_id), "유효하지 않은 AnimId 이다.\n");
    return m_clips[_id.GetIndex()];
}

const AnimClip* AnimationLibrary::GetOrNull(
    const AnimId _id) const
{
    return IsValid(_id) ? &m_clips[_id.GetIndex()] : nullptr;
}

const AnimClip* AnimationLibrary::FindClip(
    const slug::StringView _name) const
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
            const int     loopStart      = slug::Clamp(_clip.loopStart, 0, count - 1);
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
        t = slug::Clamp(t, 0.0, 1.0);
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
