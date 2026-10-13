#include "Base/Json.h"

#include <cmath>

#include "Base/PhaseTimer.h"

namespace
{
constexpr StringView kMissing = "필수 값이 없습니다";

// 키가 없으면 false
[[nodiscard]] bool Probe_(
    const JsonReader& _object,
    StringView        _key,
    bool              _bRequired,
    StringView&       _reason,
    bool* const       _pFound)
{
    const bool bFound = HasKey(_object, _key);
    if (_pFound)
    {
        *_pFound = bFound;
    }
    if (!bFound && _bRequired)
    {
        _reason = kMissing;
    }
    return bFound;
}
}   // namespace

Result<JsonDeserializer> LoadJsonFile(
    const FilePath&             _path,
    jug::Flags<eJsonLoadOption> _flags)
{
    const ScopedPhaseTimer timer { "json", _path.filename().string() };
    return JsonDeserializer::LoadFromFile(_path, _flags);
}

bool HasKey(
    const JsonReader& _object,
    StringView        _key)
{
    return _object.IsObject() && _object.HasField(_key);
}

Result<JsonReader> TryField(
    const JsonReader& _object,
    StringView        _key)
{
    if (!HasKey(_object, _key))
    {
        return Error { eError::NotFound };
    }
    return _object.FindField(_key);
}

bool ReadInt(
    const JsonReader& _object,
    StringView        _key,
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

bool ReadNumberField(
    const JsonReader& _object,
    StringView        _key,
    bool              _bRequired,
    double&           _out,
    StringView&       _reason,
    bool* const       _pFound)
{
    if (!Probe_(_object, _key, _bRequired, _reason, _pFound))
    {
        return !_bRequired;
    }
    const Result<JsonReader> field = _object.FindField(_key);
    if (!field->IsNumber())
    {
        _reason = "숫자가 아닙니다";
        return false;
    }
    _out = field->Read<double>().GetValue();
    return true;
}

bool ReadFloatField(
    const JsonReader& _object,
    StringView        _key,
    bool              _bRequired,
    float&            _out,
    StringView&       _reason)
{
    double value  = 0.0;
    bool   bFound = false;
    if (!ReadNumberField(_object, _key, _bRequired, value, _reason, &bFound))
    {
        return false;
    }
    if (bFound)
    {
        _out = static_cast<float>(value);
    }
    return true;
}

bool ReadBoolField(
    const JsonReader& _object,
    StringView        _key,
    bool              _bRequired,
    bool&             _out,
    StringView&       _reason,
    bool* const       _pFound)
{
    if (!Probe_(_object, _key, _bRequired, _reason, _pFound))
    {
        return !_bRequired;
    }
    const Result<JsonReader> field = _object.FindField(_key);
    if (!field->IsBool())
    {
        _reason = "true/false 가 아닙니다";
        return false;
    }
    _out = field->Read<bool>().GetValue();
    return true;
}

bool ReadStringField(
    const JsonReader& _object,
    StringView        _key,
    bool              _bRequired,
    String&           _out,
    StringView&       _reason,
    bool* const       _pFound)
{
    if (!Probe_(_object, _key, _bRequired, _reason, _pFound))
    {
        return !_bRequired;
    }
    const Result<JsonReader> field = _object.FindField(_key);
    if (!field->IsString())
    {
        _reason = "문자열이 아닙니다";
        return false;
    }
    _out = field->Read<String>().GetValue();
    return true;
}

bool ReadVec2Field(
    const JsonReader& _object,
    StringView        _key,
    bool              _bRequired,
    VECTOR2&          _out,
    StringView&       _reason,
    bool* const       _pFound)
{
    if (!Probe_(_object, _key, _bRequired, _reason, _pFound))
    {
        return !_bRequired;
    }
    const Result<JsonReader> field = _object.FindField(_key);
    if (!field->IsArray() || field->GetSize() != 2)
    {
        _reason = "[x, y] 숫자 배열이 아닙니다";
        return false;
    }
    ARRAY<float, 2> values = {};
    int             i      = 0;
    for (const JsonReader item: field->GetArray())
    {
        if (!item.IsNumber())
        {
            _reason = "[x, y] 숫자 배열이 아닙니다";
            return false;
        }
        values[i++] = static_cast<float>(item.Read<double>().GetValue());
    }
    _out = { values[0], values[1] };
    return true;
}

bool CheckJsonKeys(
    const JsonReader&      _object,
    Span<const StringView> _allowed,
    StringView&            _badKey)
{
    for (const std::pair<StringView, JsonReader> member: _object.GetObject())
    {
        bool bKnown = member.first == "note";
        for (const StringView allowed: _allowed)
        {
            bKnown = bKnown || allowed == member.first;
        }
        if (!bKnown)
        {
            _badKey = member.first;
            return false;
        }
    }
    return true;
}
