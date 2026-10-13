#include "Editor/AnimDocument.h"
#include "Base/JsonFields.h"
#include "Core/AssetPath.h"
#include <algorithm>
#include <charconv>
#include <format>

constexpr size_t kMaxUndo = 200;

namespace
{
[[nodiscard]] Result<AnimFrame> ParseFrame_(
    const JsonReader& _frame)
{
    if (!_frame.IsObject() || _frame.GetSize() != 7)
    {
        return Error { eError::InvalidData };
    }
    AnimFrame                  f;
    const ARRAY<int*, 7>       fields = { &f.atlas, &f.x, &f.y, &f.w, &f.h, &f.pivotX, &f.pivotY };
    const ARRAY<StringView, 7> names  = { "atlas", "x", "y", "w", "h", "pivotX", "pivotY" };
    for (size_t i = 0; i < fields.size(); ++i)
    {
        const Result<int> v = _frame.ReadField<int>(names[i]);
        if (!v)
        {
            return v.GetError();
        }
        *fields[i] = v.GetValue();
    }
    return f;
}

[[nodiscard]] Result<AnimEditClip> ParseClip_(
    const StringView  _name,
    const JsonReader& _clip)
{
    if (!_clip.IsObject())
    {
        return Error { eError::InvalidData };
    }
    AnimEditClip clip;
    clip.name       = String { _name };
    clip.bHasCancel = _clip.HasField("cancelFrame");
    if (_clip.GetSize() != (clip.bHasCancel ? 7u : 6u))
    {
        return Error { eError::InvalidData };   // 모르는 필드가 있으면 저장 때 잃으므로 거부
    }

    const Result<bool>   bLoop     = _clip.ReadField<bool>("loop");
    const Result<int>    loopStart = _clip.ReadField<int>("loopStart");
    const Result<double> duration  = _clip.ReadField<double>("frameDuration");
    const Result<int>    cellW     = _clip.ReadField<int>("cellW");
    const Result<int>    cellH     = _clip.ReadField<int>("cellH");
    if (!bLoop || !loopStart || !duration || !cellW || !cellH)
    {
        return Error { eError::InvalidData };
    }
    clip.bLoop         = bLoop.GetValue();
    clip.loopStart     = loopStart.GetValue();
    clip.frameDuration = duration.GetValue();
    clip.cellW         = cellW.GetValue();
    clip.cellH         = cellH.GetValue();
    if (clip.bHasCancel)
    {
        const Result<int> cancel = _clip.ReadField<int>("cancelFrame");
        if (!cancel)
        {
            return cancel.GetError();
        }
        clip.cancelFrame = cancel.GetValue();
    }

    const Result<JsonReader> frames = _clip.FindField("frames");
    if (!frames || !frames->IsArray())
    {
        return Error { eError::InvalidData };
    }
    for (const JsonReader& item: frames->GetArray())
    {
        const Result<AnimFrame> frame = ParseFrame_(item);
        if (!frame)
        {
            return frame.GetError();
        }
        clip.frames.push_back(frame.GetValue());
    }
    return clip;
}

// 가장 짧게 왕복되는 표기
void AppendNumber_(
    String&      _out,
    const double _value)
{
    ARRAY<char, 64>            buf = {};
    const std::to_chars_result r   = std::to_chars(buf.data(), buf.data() + buf.size(), _value);
    _out.append(buf.data(), r.ptr);
}

void AppendString_(
    String&          _out,
    const StringView _text)
{
    _out += '"';
    for (const char c: _text)
    {
        if (c == '"' || c == '\\')
        {
            _out += '\\';
        }
        _out += c;
    }
    _out += '"';
}

void AppendField_(
    String&          _out,
    const StringView _key,
    const int        _value)
{
    _out += std::format("      \"{}\": {},\n", _key, _value);
}
}   // namespace

Result<void> AnimDocument::LoadFromText(
    const StringView _json)
{
    Result<JsonDeserializer> doc = JsonDeserializer::Load(_json, kJsonBom);
    if (!doc)
    {
        return Error { eError::Parse };
    }
    const JsonReader root = doc->GetReader();
    if (!root.IsObject() || root.GetSize() != 2)
    {
        return Error { eError::InvalidData };
    }
    const Result<int>        version = root.ReadField<int>("version");
    const Result<JsonReader> clips   = root.FindField("animations");
    if (!version || !clips || !clips->IsObject())
    {
        return Error { eError::InvalidData };
    }

    Vector<AnimEditClip> parsed;
    for (const std::pair<StringView, JsonReader>& entry: clips->GetObject())
    {
        Result<AnimEditClip> clip = ParseClip_(entry.first, entry.second);
        if (!clip)
        {
            return clip.GetError();
        }
        parsed.push_back(clip.Take());
    }

    m_clips   = std::move(parsed);
    m_version = version.GetValue();
    m_undo.clear();
    m_bDirty      = false;
    m_lastChanged = -1;
    ++m_revision;
    return Error { kOK };
}

Result<void> AnimDocument::Load(
    const FilePath& _path)
{
    const Result<String> text = ReadFileText(_path);
    if (!text)
    {
        return Error { eError::FileOpen };
    }
    return LoadFromText(text.GetValue());
}

String AnimDocument::Serialize() const
{
    String out = std::format("{{\n  \"version\": {},\n  \"animations\": {{\n", m_version);
    for (size_t i = 0; i < m_clips.size(); ++i)
    {
        const AnimEditClip& clip = m_clips[i];
        out += "    ";
        AppendString_(out, clip.name);
        out += ": {\n";
        out += clip.bLoop ? "      \"loop\": true,\n" : "      \"loop\": false,\n";
        AppendField_(out, "loopStart", clip.loopStart);
        if (clip.bHasCancel)
        {
            AppendField_(out, "cancelFrame", clip.cancelFrame);
        }
        out += "      \"frameDuration\": ";
        AppendNumber_(out, clip.frameDuration);
        out += ",\n";
        AppendField_(out, "cellW", clip.cellW);
        AppendField_(out, "cellH", clip.cellH);
        out += "      \"frames\": [\n";
        for (size_t f = 0; f < clip.frames.size(); ++f)
        {
            const AnimFrame& fr = clip.frames[f];
            out += std::format("        {{ \"atlas\": {}, \"x\": {}, \"y\": {}, \"w\": {}, \"h\": {}, \"pivotX\": {}, \"pivotY\": {} }}", fr.atlas, fr.x, fr.y, fr.w, fr.h, fr.pivotX, fr.pivotY);
            out += (f + 1 < clip.frames.size()) ? ",\n" : "\n";
        }
        out += "      ]\n    }";
        out += (i + 1 < m_clips.size()) ? ",\n" : "\n";
    }
    out += "  }\n}\n";
    return out;
}

Result<void> AnimDocument::Save(
    const FilePath& _path) const
{
    const String text = Serialize();
    return WriteFileBytes(_path, text);
}

int AnimDocument::FindClip(
    const StringView _name) const
{
    for (size_t i = 0; i < m_clips.size(); ++i)
    {
        if (m_clips[i].name == _name)
        {
            return static_cast<int>(i);
        }
    }
    return -1;
}

void AnimDocument::PushUndo_(
    const int _clip)
{
    if (m_undo.size() >= kMaxUndo)
    {
        m_undo.erase(m_undo.begin());
    }
    UndoEntry entry;
    entry.clip     = _clip;
    entry.snapshot = m_clips[static_cast<size_t>(_clip)];
    m_undo.push_back(std::move(entry));
    m_bDirty      = true;
    m_lastChanged = _clip;
    ++m_revision;
}

void AnimDocument::SetPivot(
    const int _clip,
    const int _frame,
    const int _pivotX,
    const int _pivotY)
{
    AnimEditClip& clip = m_clips[static_cast<size_t>(_clip)];
    if (_frame < 0 || _frame >= static_cast<int>(clip.frames.size()))
    {
        return;
    }
    const AnimFrame& before = clip.frames[static_cast<size_t>(_frame)];
    if (before.pivotX == _pivotX && before.pivotY == _pivotY)
    {
        return;
    }
    PushUndo_(_clip);
    AnimFrame& frame = clip.frames[static_cast<size_t>(_frame)];
    frame.pivotX     = _pivotX;
    frame.pivotY     = _pivotY;
}

void AnimDocument::ApplyPivotToAll(
    const int _clip,
    const int _pivotX,
    const int _pivotY)
{
    PushUndo_(_clip);
    for (AnimFrame& frame: m_clips[static_cast<size_t>(_clip)].frames)
    {
        frame.pivotX = _pivotX;
        frame.pivotY = _pivotY;
    }
}

void AnimDocument::SetFrameDuration(
    const int    _clip,
    const double _duration)
{
    PushUndo_(_clip);
    m_clips[static_cast<size_t>(_clip)].frameDuration = _duration;
}

void AnimDocument::SetLoop(
    const int  _clip,
    const bool _bLoop)
{
    PushUndo_(_clip);
    m_clips[static_cast<size_t>(_clip)].bLoop = _bLoop;
}

void AnimDocument::SetLoopStart(
    const int _clip,
    const int _loopStart)
{
    PushUndo_(_clip);
    m_clips[static_cast<size_t>(_clip)].loopStart = _loopStart;
}

void AnimDocument::Undo()
{
    if (m_undo.empty())
    {
        return;
    }
    UndoEntry entry = std::move(m_undo.back());
    m_undo.pop_back();
    m_lastChanged                            = entry.clip;
    m_clips[static_cast<size_t>(entry.clip)] = std::move(entry.snapshot);
    m_bDirty                                 = true;
    ++m_revision;
}
