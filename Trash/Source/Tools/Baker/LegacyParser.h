#pragma once
#include <charconv>
#include <cstdint>
#include <utility>

#include "Base/Types.h"

namespace slug::baker
{

enum class eTransparentMode
{
    None     = 0,   // BitBlt (알파 무시)
    ColorKey = 1,   // TransparentBlt (마젠타 컬러키)
    Alpha    = 2    // AlphaBlend (BMP 알파 채널)
};

struct TextureRecord
{
    String           name = {};
    String           path = {};
    eTransparentMode mode = eTransparentMode::None;
};

struct AnimRecord
{
    String name          = {};
    String texture       = {};
    int    start         = 0;
    int    end           = 0;
    int    width         = 0;
    int    height        = 0;
    int    line          = 0;   // .anime 에는 보통 없음 (레거시 기본값 0)
    double frameDuration = 1.0;
    bool   bLoop         = true;
    int    optionIndex   = 0;
    int    offsetX       = 0;
    int    offsetY       = 0;
    String sourceFile    = {};
};

struct SpriteRecord
{
    String name       = {};
    String texture    = {};
    int    width      = -1;   // -1 = default (텍스처 전체)
    int    height     = -1;
    int    column     = 0;
    int    line       = 0;
    String sourceFile = {};
};

struct SoundRecord
{
    String name = {};
    String path = {};
};

using KeyValue = std::pair<String, String>;

[[nodiscard]] inline StringView TrimView(StringView _s)
{
    while (!_s.empty() && (_s.front() == ' ' || _s.front() == '\t' || _s.front() == '\r' || _s.front() == '\n'))
    {
        _s.remove_prefix(1);
    }
    while (!_s.empty() && (_s.back() == ' ' || _s.back() == '\t' || _s.back() == '\r' || _s.back() == '\n'))
    {
        _s.remove_suffix(1);
    }
    return _s;
}

[[nodiscard]] inline bool ParseIntStrict(
    const StringView _text,
    int&             _out)
{
    const StringView s = TrimView(_text);
    if (s.empty())
    {
        return false;
    }
    const StringView             digits = (s.front() == '+') ? s.substr(1) : s;
    int                          value  = 0;
    const std::from_chars_result result = std::from_chars(digits.data(), digits.data() + digits.size(), value);
    if (result.ec != std::errc {} || result.ptr != digits.data() + digits.size())
    {
        return false;
    }
    _out = value;
    return true;
}

[[nodiscard]] inline bool ParseDoubleStrict(
    const StringView _text,
    double&          _out)
{
    const StringView s = TrimView(_text);
    if (s.empty())
    {
        return false;
    }
    double                       value  = 0.0;
    const std::from_chars_result result = std::from_chars(s.data(), s.data() + s.size(), value);
    if (result.ec != std::errc {} || result.ptr != s.data() + s.size())
    {
        return false;
    }
    _out = value;
    return true;
}

[[nodiscard]] inline String NormalizePath(const StringView _path)
{
    String out(TrimView(_path));
    for (char& c: out)
    {
        if (c == '\\')
        {
            c = '/';
        }
    }
    return out;
}

[[nodiscard]] inline Vector<KeyValue> SplitKeyValueLines(
    StringView      _text,
    Vector<String>* _pWarnings = nullptr)
{
    if (_text.size() >= 3 && static_cast<unsigned char>(_text[0]) == 0xEF && static_cast<unsigned char>(_text[1]) == 0xBB && static_cast<unsigned char>(_text[2]) == 0xBF)
    {
        _text.remove_prefix(3);
    }

    Vector<KeyValue> lines;
    size_t           lineNo = 0;
    while (!_text.empty())
    {
        const size_t     eol  = _text.find('\n');
        const StringView line = TrimView(_text.substr(0, eol));
        _text                 = (eol == StringView::npos) ? StringView {} : _text.substr(eol + 1);
        ++lineNo;

        if (line.empty())
        {
            continue;
        }
        const size_t colon = line.find(':');
        if (colon == StringView::npos)
        {
            if (_pWarnings != nullptr)
            {
                _pWarnings->push_back("line " + std::to_string(lineNo) + ": no ':' -> '" + String(line) + "'");
            }
            continue;
        }
        lines.emplace_back(String(TrimView(line.substr(0, colon))), String(TrimView(line.substr(colon + 1))));
    }
    return lines;
}

[[nodiscard]] inline Vector<TextureRecord> ParseTextureData(
    const StringView _text,
    Vector<String>&  _warnings)
{
    Vector<TextureRecord>  records;
    const Vector<KeyValue> lines = SplitKeyValueLines(_text, &_warnings);
    for (const KeyValue& line: lines)
    {
        const String& key   = line.first;
        const String& value = line.second;

        if (key == "File Name")
        {
            records.push_back({});
            records.back().name = value;
        }
        else if (records.empty())
        {
            _warnings.push_back("texture key before File Name: " + key);
        }
        else if (key == "File Path")
        {
            records.back().path = NormalizePath(value);
        }
        else if (key == "UseTransparent")
        {
            int mode = 0;
            if (!ParseIntStrict(value, mode) || mode < 0 || mode > 2)
            {
                _warnings.push_back("texture '" + records.back().name + "': bad UseTransparent '" + value + "'");
                mode = 0;
            }
            records.back().mode = static_cast<eTransparentMode>(mode);
        }
        else if (key == "Transparent")
        {
        }
        else
        {
            _warnings.push_back("texture '" + records.back().name + "': unknown key '" + key + "'");
        }
    }
    return records;
}

namespace legacy_parser_detail
{

    // 정수 필드를 읽고, 실패하면 경고를 남긴다.
    inline void ParseIntOrWarn_(
        const StringView _kind,
        const String&    _recordName,
        const String&    _key,
        const String&    _value,
        int&             _out,
        Vector<String>&  _warnings)
    {
        if (!ParseIntStrict(_value, _out))
        {
            _warnings.push_back(String(_kind) + " '" + _recordName + "': bad integer for " + _key + " '" + _value + "'");
        }
    }

}   // namespace legacy_parser_detail

[[nodiscard]] inline Vector<AnimRecord> ParseAnimeData(
    const StringView _text,
    const String&    _sourceFile,
    Vector<String>&  _warnings)
{
    Vector<AnimRecord>     records;
    const Vector<KeyValue> lines = SplitKeyValueLines(_text, &_warnings);
    for (const KeyValue& line: lines)
    {
        const String& key   = line.first;
        const String& value = line.second;

        if (key == "File Name")
        {
            records.push_back({});
            records.back().name       = value;
            records.back().sourceFile = _sourceFile;
            continue;
        }
        if (records.empty())
        {
            _warnings.push_back("anime key before File Name: " + key);
            continue;
        }

        AnimRecord& rec = records.back();

        if (key == "Texture Name")
        {
            rec.texture = value;
        }
        else if (key == "Start")
        {
            legacy_parser_detail::ParseIntOrWarn_("anime", rec.name, key, value, rec.start, _warnings);
        }
        else if (key == "End")
        {
            legacy_parser_detail::ParseIntOrWarn_("anime", rec.name, key, value, rec.end, _warnings);
        }
        else if (key == "Width")
        {
            legacy_parser_detail::ParseIntOrWarn_("anime", rec.name, key, value, rec.width, _warnings);
        }
        else if (key == "Height")
        {
            legacy_parser_detail::ParseIntOrWarn_("anime", rec.name, key, value, rec.height, _warnings);
        }
        else if (key == "Line")
        {
            legacy_parser_detail::ParseIntOrWarn_("anime", rec.name, key, value, rec.line, _warnings);
        }
        else if (key == "OptionIndex")
        {
            legacy_parser_detail::ParseIntOrWarn_("anime", rec.name, key, value, rec.optionIndex, _warnings);
        }
        else if (key == "OffsetX")
        {
            legacy_parser_detail::ParseIntOrWarn_("anime", rec.name, key, value, rec.offsetX, _warnings);
        }
        else if (key == "OffsetY")
        {
            legacy_parser_detail::ParseIntOrWarn_("anime", rec.name, key, value, rec.offsetY, _warnings);
        }
        else if (key == "FrameDuration")
        {
            if (!ParseDoubleStrict(value, rec.frameDuration))
            {
                _warnings.push_back("anime '" + rec.name + "': bad FrameDuration '" + value + "'");
            }
        }
        else if (key == "Loop")
        {
            int loop = 1;
            legacy_parser_detail::ParseIntOrWarn_("anime", rec.name, key, value, loop, _warnings);
            rec.bLoop = (loop != 0);
        }
        else
        {
            _warnings.push_back("anime '" + rec.name + "': unknown key '" + key + "'");
        }
    }
    return records;
}

[[nodiscard]] inline Vector<SpriteRecord> ParseSpriteData(
    const StringView _text,
    const String&    _sourceFile,
    Vector<String>&  _warnings)
{
    Vector<SpriteRecord>   records;
    const Vector<KeyValue> lines = SplitKeyValueLines(_text, &_warnings);
    for (const KeyValue& line: lines)
    {
        const String& key   = line.first;
        const String& value = line.second;

        if (key == "File Name")
        {
            records.push_back({});
            records.back().name       = value;
            records.back().sourceFile = _sourceFile;
            continue;
        }
        if (records.empty())
        {
            _warnings.push_back("sprite key before File Name: " + key);
            continue;
        }

        SpriteRecord& rec = records.back();

        if (key == "Texture Name")
        {
            rec.texture = value;
        }
        else if (key == "Width")
        {
            if (value != "default")
            {
                legacy_parser_detail::ParseIntOrWarn_("sprite", rec.name, key, value, rec.width, _warnings);
            }
        }
        else if (key == "Height")
        {
            if (value != "default")
            {
                legacy_parser_detail::ParseIntOrWarn_("sprite", rec.name, key, value, rec.height, _warnings);
            }
        }
        else if (key == "Column")
        {
            legacy_parser_detail::ParseIntOrWarn_("sprite", rec.name, key, value, rec.column, _warnings);
        }
        else if (key == "Line")
        {
            legacy_parser_detail::ParseIntOrWarn_("sprite", rec.name, key, value, rec.line, _warnings);
        }
        else
        {
            _warnings.push_back("sprite '" + rec.name + "': unknown key '" + key + "'");
        }
    }
    return records;
}

[[nodiscard]] inline Vector<SoundRecord> ParseSoundData(
    const StringView _text,
    Vector<String>&  _warnings)
{
    Vector<SoundRecord>    records;
    const Vector<KeyValue> lines = SplitKeyValueLines(_text, &_warnings);
    for (const KeyValue& line: lines)
    {
        const String& key   = line.first;
        const String& value = line.second;

        if (key == "Resource Name")
        {
            records.push_back({});
            records.back().name = value;
        }
        else if (records.empty())
        {
            _warnings.push_back("sound key before Resource Name: " + key);
        }
        else if (key == "File Path")
        {
            records.back().path = NormalizePath(value);
        }
        else
        {
            _warnings.push_back("sound '" + records.back().name + "': unknown key '" + key + "'");
        }
    }
    return records;
}

}   // namespace slug::baker
