#pragma once
#include <charconv>
#include <cstdint>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

namespace slug::baker
{

// ===========================================
//  레거시 텍스트 포맷 파서
//    "key:value" 줄들의 블록. 레거시 로더처럼 첫 키(File Name / Resource Name)가 나오면 새 레코드가 시작된다.
// ===========================================

enum class eTransparentMode
{
    None     = 0,   // BitBlt (알파 무시)
    ColorKey = 1,   // TransparentBlt (마젠타 컬러키)
    Alpha    = 2    // AlphaBlend (BMP 알파 채널)
};

struct TextureRecord
{
    std::string      name;
    std::string      path;   // Resources 기준, '/' 구분자로 정규화
    eTransparentMode mode = eTransparentMode::None;
};

struct AnimRecord
{
    std::string name;
    std::string texture;
    int         start         = 0;
    int         end           = 0;
    int         width         = 0;
    int         height        = 0;
    int         line          = 0;   // .anime 에는 보통 없음 (레거시 기본값 0)
    double      frameDuration = 1.0;
    bool        bLoop         = true;
    int         optionIndex   = 0;
    int         offsetX       = 0;
    int         offsetY       = 0;
    std::string sourceFile;          // 어느 .anime 에서 왔는지
};

struct SpriteRecord
{
    std::string name;
    std::string texture;
    int         width  = -1;   // -1 = default (텍스처 전체)
    int         height = -1;
    int         column = 0;
    int         line   = 0;
    std::string sourceFile;
};

struct SoundRecord
{
    std::string name;
    std::string path;   // '/' 구분자로 정규화
};

// ===========================================
//  유틸
// ===========================================

[[nodiscard]] inline std::string_view TrimView(std::string_view _s)
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
    const std::string_view _text,
    int&                   _out)
{
    const std::string_view s = TrimView(_text);
    if (s.empty())
    {
        return false;
    }
    const std::string_view digits = (s.front() == '+') ? s.substr(1) : s;
    int                    value  = 0;
    const auto [ptr, ec]          = std::from_chars(digits.data(), digits.data() + digits.size(), value);
    if (ec != std::errc {} || ptr != digits.data() + digits.size())
    {
        return false;
    }
    _out = value;
    return true;
}

[[nodiscard]] inline bool ParseDoubleStrict(
    const std::string_view _text,
    double&                _out)
{
    const std::string_view s = TrimView(_text);
    if (s.empty())
    {
        return false;
    }
    double value         = 0.0;
    const auto [ptr, ec] = std::from_chars(s.data(), s.data() + s.size(), value);
    if (ec != std::errc {} || ptr != s.data() + s.size())
    {
        return false;
    }
    _out = value;
    return true;
}

[[nodiscard]] inline std::string NormalizePath(const std::string_view _path)
{
    std::string out(TrimView(_path));
    for (char& c: out)
    {
        if (c == '\\')
        {
            c = '/';
        }
    }
    return out;
}

// 텍스트를 (key, value) 줄 목록으로 쪼갠다. BOM / CRLF / 빈 줄 / ':' 없는 줄을 처리한다.
// 키와 값 모두 앞뒤 공백을 제거한다. ':' 가 없는 줄은 _warnings 에 기록하고 건너뛴다.
[[nodiscard]] inline std::vector<std::pair<std::string, std::string>> SplitKeyValueLines(
    std::string_view          _text,
    std::vector<std::string>* _pWarnings = nullptr)
{
    if (_text.size() >= 3 && static_cast<unsigned char>(_text[0]) == 0xEF && static_cast<unsigned char>(_text[1]) == 0xBB && static_cast<unsigned char>(_text[2]) == 0xBF)
    {
        _text.remove_prefix(3);
    }

    std::vector<std::pair<std::string, std::string>> lines;
    size_t                                           lineNo = 0;
    while (!_text.empty())
    {
        const size_t           eol  = _text.find('\n');
        const std::string_view line = TrimView(_text.substr(0, eol));
        _text                       = (eol == std::string_view::npos) ? std::string_view {} : _text.substr(eol + 1);
        ++lineNo;

        if (line.empty())
        {
            continue;
        }
        const size_t colon = line.find(':');
        if (colon == std::string_view::npos)
        {
            if (_pWarnings != nullptr)
            {
                _pWarnings->push_back("line " + std::to_string(lineNo) + ": no ':' -> '" + std::string(line) + "'");
            }
            continue;
        }
        lines.emplace_back(std::string(TrimView(line.substr(0, colon))), std::string(TrimView(line.substr(colon + 1))));
    }
    return lines;
}

// ===========================================
//  TextureData.data
// ===========================================

[[nodiscard]] inline std::vector<TextureRecord> ParseTextureData(
    const std::string_view    _text,
    std::vector<std::string>& _warnings)
{
    std::vector<TextureRecord> records;
    for (const auto& [key, value]: SplitKeyValueLines(_text, &_warnings))
    {
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
            // 레거시: "default" -> RGB(255, 0, 255) 고정. 그 외 값은 NULL(검정)로 해석되므로 여기서는 무시한다.
        }
        else
        {
            _warnings.push_back("texture '" + records.back().name + "': unknown key '" + key + "'");
        }
    }
    return records;
}

// ===========================================
//  *.anime
// ===========================================

[[nodiscard]] inline std::vector<AnimRecord> ParseAnimeData(
    const std::string_view    _text,
    const std::string&        _sourceFile,
    std::vector<std::string>& _warnings)
{
    std::vector<AnimRecord> records;
    for (const auto& [key, value]: SplitKeyValueLines(_text, &_warnings))
    {
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

        AnimRecord& rec  = records.back();
        auto        asInt = [&](int& _out)
        {
            if (!ParseIntStrict(value, _out))
            {
                _warnings.push_back("anime '" + rec.name + "': bad integer for " + key + " '" + value + "'");
            }
        };

        if (key == "Texture Name") { rec.texture = value; }
        else if (key == "Start") { asInt(rec.start); }
        else if (key == "End") { asInt(rec.end); }
        else if (key == "Width") { asInt(rec.width); }
        else if (key == "Height") { asInt(rec.height); }
        else if (key == "Line") { asInt(rec.line); }
        else if (key == "OptionIndex") { asInt(rec.optionIndex); }
        else if (key == "OffsetX") { asInt(rec.offsetX); }
        else if (key == "OffsetY") { asInt(rec.offsetY); }
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
            asInt(loop);
            rec.bLoop = (loop != 0);
        }
        else
        {
            _warnings.push_back("anime '" + rec.name + "': unknown key '" + key + "'");
        }
    }
    return records;
}

// ===========================================
//  *.sprite
// ===========================================

[[nodiscard]] inline std::vector<SpriteRecord> ParseSpriteData(
    const std::string_view    _text,
    const std::string&        _sourceFile,
    std::vector<std::string>& _warnings)
{
    std::vector<SpriteRecord> records;
    for (const auto& [key, value]: SplitKeyValueLines(_text, &_warnings))
    {
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
        auto          asInt = [&](int& _out)
        {
            if (!ParseIntStrict(value, _out))
            {
                _warnings.push_back("sprite '" + rec.name + "': bad integer for " + key + " '" + value + "'");
            }
        };

        if (key == "Texture Name") { rec.texture = value; }
        else if (key == "Width") { if (value != "default") asInt(rec.width); }
        else if (key == "Height") { if (value != "default") asInt(rec.height); }
        else if (key == "Column") { asInt(rec.column); }
        else if (key == "Line") { asInt(rec.line); }
        else
        {
            _warnings.push_back("sprite '" + rec.name + "': unknown key '" + key + "'");
        }
    }
    return records;
}

// ===========================================
//  SoundData.txt
// ===========================================

[[nodiscard]] inline std::vector<SoundRecord> ParseSoundData(
    const std::string_view    _text,
    std::vector<std::string>& _warnings)
{
    std::vector<SoundRecord> records;
    for (const auto& [key, value]: SplitKeyValueLines(_text, &_warnings))
    {
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
