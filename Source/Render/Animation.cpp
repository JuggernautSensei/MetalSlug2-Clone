#include "Render/Animation.h"

#include <cmath>

#include "Base/Assert.h"
#include "Base/Error.h"
#include "Base/Handle.h"
#include "Base/Json.h"
#include "Base/Log.h"
#include "Base/Math.h"
#include "Base/Types.h"

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

    [[nodiscard]] bool ReadInt_(
        const JsonReader& _object,
        const StringView  _key,
        int&              _out)
    {
        const Result<double> value = _object.ReadField<double>(_key);
        if (!value)
        {
            return false;
        }
        _out = static_cast<int>(std::llround(value.GetValue()));
        return true;
    }

    [[nodiscard]] bool ReadOptionalInt_(
        const JsonReader& _object,
        const StringView  _key,
        int&              _out)
    {
        return !_object.HasField(_key) || ReadInt_(_object, _key, _out);
    }

    [[nodiscard]] Result<AnimFrame> ParseFrame_(
        const JsonReader& _json)
    {
        if (!_json.IsObject())
        {
            return Error { eError::Parse };
        }

        AnimFrame frame;
        if (!ReadInt_(_json, "atlas", frame.atlas) || !ReadInt_(_json, "x", frame.x) || !ReadInt_(_json, "y", frame.y) || !ReadInt_(_json, "w", frame.w) || !ReadInt_(_json, "h", frame.h) || !ReadInt_(_json, "pivotX", frame.pivotX) || !ReadInt_(_json, "pivotY", frame.pivotY))
        {
            return Error { eError::Parse };
        }
        if (frame.atlas < 0 || frame.w < 0 || frame.h < 0)
        {
            return Error { eError::InvalidData };
        }
        return frame;
    }

    [[nodiscard]] Result<AnimClip> ParseClip_(
        const StringView  _name,
        const JsonReader& _json)
    {
        if (!_json.IsObject())
        {
            return Error { eError::Parse };
        }

        AnimClip clip;
        clip.name = String { _name };

        if (_json.HasField("loop"))
        {
            const Result<bool> bLoop = _json.ReadField<bool>("loop");
            if (!bLoop)
            {
                return Error { eError::Parse };
            }
            clip.bLoop = bLoop.GetValue();
        }

        if (!ReadOptionalInt_(_json, "loopStart", clip.loopStart) || !ReadOptionalInt_(_json, "cellW", clip.cellW) || !ReadOptionalInt_(_json, "cellH", clip.cellH))
        {
            return Error { eError::Parse };
        }

        const Result<double> duration = _json.ReadField<double>("frameDuration");
        if (!duration)
        {
            return Error { eError::Parse };
        }
        clip.frameDuration = static_cast<float>(duration.GetValue());
        if (!(clip.frameDuration > 0.f))
        {
            return Error { eError::InvalidData };
        }

        const Result<JsonReader> frames = _json.FindField("frames");
        if (!frames || !frames->IsArray())
        {
            return Error { eError::Parse };
        }

        clip.frames.reserve(frames->GetSize());
        for (const JsonReader frameJson: frames->GetArray())
        {
            Result<AnimFrame> frame = ParseFrame_(frameJson);
            if (!frame)
            {
                return frame.GetError();
            }
            clip.frames.push_back(frame.GetValue());
        }

        if (clip.frames.empty())
        {
            return Error { eError::InvalidData };
        }

        if (clip.loopStart < 0 || clip.loopStart >= clip.GetFrameCount())
        {
            clip.loopStart = Clamp(clip.loopStart, 0, clip.GetFrameCount() - 1);
        }
        return clip;
    }

    [[nodiscard]] Result<AnimationLibrary> ParseAnimDocument_(
        const JsonReader& _root)
    {
        if (!_root.IsObject())
        {
            return Error { eError::Parse };
        }

        const Result<JsonReader> animations = _root.FindField("animations");
        if (!animations || !animations->IsObject())
        {
            return Error { eError::Parse };
        }

        AnimationLibrary library;
        for (const std::pair<StringView, JsonReader>& item: animations->GetObject())
        {
            const StringView  name     = item.first;
            const JsonReader& clipJson = item.second;
            Result<AnimClip>  clip     = ParseClip_(name, clipJson);
            if (!clip)
            {
                SLUG_LOG_ERROR("애니메이션 '{}' 파싱 실패: {}", name, clip.GetError().MakeMessage());
                return clip.GetError();
            }
            (void)library.AddClip(clip.Take());
        }
        return library;
    }

    [[nodiscard]] Result<Vector<String>> ParseAtlasPages_(
        const JsonReader& _manifest)
    {
        Vector<String> pages;
        if (!_manifest.HasField("atlasPages"))
        {
            return pages;
        }

        const Result<JsonReader> list = _manifest.FindField("atlasPages");
        if (!list || !list->IsArray())
        {
            return Error { eError::Parse };
        }
        for (const JsonReader item: list->GetArray())
        {
            Result<String> page = item.Read<String>();
            if (!page)
            {
                return Error { eError::Parse };
            }
            pages.push_back(page.GetValue());
        }
        return pages;
    }

}   // namespace

int64_t AnimClip::GetFrameDurationUnits() const
{
    return ToUnits_(static_cast<double>(frameDuration));
}

Result<AnimationLibrary> AnimationLibrary::LoadFromJson(
    const StringView _animJson)
{
    Result<JsonDeserializer> doc = JsonDeserializer::Load(_animJson);
    if (!doc)
    {
        return Error { eError::Parse };
    }
    return ParseAnimDocument_(doc->GetReader());
}

Result<AnimationLibrary> AnimationLibrary::LoadFromDirectory(
    const FilePath& _bakedRoot)
{
    Result<JsonDeserializer> manifestDoc = JsonDeserializer::LoadFromFile(_bakedRoot / "manifest.json");
    if (!manifestDoc)
    {
        return Error { eError::FileOpen };
    }
    const JsonReader manifest = manifestDoc->GetReader();
    if (!manifest.IsObject())
    {
        return Error { eError::Parse };
    }

    String animFile = "anim.json";
    if (manifest.HasField("animations"))
    {
        Result<String> name = manifest.ReadField<String>("animations");
        if (!name)
        {
            return Error { eError::Parse };
        }
        animFile = name.GetValue();
    }

    Result<Vector<String>> pages = ParseAtlasPages_(manifest);
    if (!pages)
    {
        return pages.GetError();
    }

    Result<JsonDeserializer> animDoc = JsonDeserializer::LoadFromFile(_bakedRoot / animFile);
    if (!animDoc)
    {
        return Error { eError::FileOpen };
    }

    Result<AnimationLibrary> library = ParseAnimDocument_(animDoc->GetReader());
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
    if (const NameMap::iterator it = m_nameToIndex.find(_clip.name); it != m_nameToIndex.end())
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
    const StringView _name) const
{
    const NameMap::const_iterator it = m_nameToIndex.find(String { _name });
    if (it == m_nameToIndex.end())
    {
        return kNullHandle;
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
    const StringView _name) const
{
    return GetOrNull(Find(_name));
}

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
            const int     loopStart      = Clamp(_clip.loopStart, 0, count - 1);
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
        t = Clamp(t, 0.0, 1.0);
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
