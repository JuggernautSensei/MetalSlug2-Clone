#pragma once
#include <charconv>
#include <cstdint>
#include <cstdio>
#include <string>
#include <string_view>
#include <vector>

namespace slug::baker
{

// ===========================================
//  최소 JSON 작성기 (들여쓰기 출력, 인라인 객체 지원)
// ===========================================

class JsonWriter
{
public:
    [[nodiscard]] const std::string& Str() const { return m_text; }

    // _bInline = true 면 해당 컨테이너를 한 줄로 출력한다.
    void BeginObject(const bool _bInline = false) { Open_('{', _bInline); }
    void EndObject() { Close_('}'); }
    void BeginArray(const bool _bInline = false) { Open_('[', _bInline); }
    void EndArray() { Close_(']'); }

    void Key(const std::string_view _key)
    {
        Separator_();
        AppendString_(_key);
        m_text += ": ";
        m_bAfterKey = true;
    }

    void Value(const std::string_view _v)
    {
        Separator_();
        AppendString_(_v);
    }
    void Value(const char* _v) { Value(std::string_view { _v }); }
    void Value(const std::string& _v) { Value(std::string_view { _v }); }
    void Value(const bool _v)
    {
        Separator_();
        m_text += _v ? "true" : "false";
    }
    void Value(const int _v) { Number_(static_cast<long long>(_v)); }
    void Value(const long long _v) { Number_(_v); }
    void Value(const uint32_t _v) { Number_(static_cast<long long>(_v)); }
    void Value(const uint64_t _v)
    {
        Separator_();
        m_text += std::to_string(_v);
    }
    void Value(const double _v)
    {
        Separator_();
        char buffer[64];
        const auto [end, ec] = std::to_chars(buffer, buffer + sizeof(buffer), _v);
        m_text.append(buffer, ec == std::errc {} ? end : buffer);
    }
    void Null()
    {
        Separator_();
        m_text += "null";
    }

    template<typename T>
    void Field(
        const std::string_view _key,
        const T&               _value)
    {
        Key(_key);
        Value(_value);
    }

private:
    struct Level
    {
        bool bInline = false;
        bool bFirst  = true;
    };

    void Open_(
        const char _c,
        const bool _bInline)
    {
        Separator_();
        m_text += _c;
        const bool bParentInline = !m_stack.empty() && m_stack.back().bInline;
        m_stack.push_back({ _bInline || bParentInline, true });
    }

    void Close_(const char _c)
    {
        const Level level = m_stack.back();
        m_stack.pop_back();
        if (!level.bFirst)
        {
            if (level.bInline)
            {
                m_text += ' ';
            }
            else
            {
                NewLine_(m_stack.size());
            }
        }
        m_text += _c;
        if (m_stack.empty())
        {
            m_text += '\n';
        }
    }

    // 값/키 앞에서 호출: 쉼표와 줄바꿈 처리
    void Separator_()
    {
        if (m_bAfterKey)
        {
            m_bAfterKey = false;
            return;
        }
        if (m_stack.empty())
        {
            return;
        }

        Level& level = m_stack.back();
        if (!level.bFirst)
        {
            m_text += ',';
        }
        if (level.bInline)
        {
            m_text += ' ';
        }
        else
        {
            NewLine_(m_stack.size());
        }
        level.bFirst = false;
    }

    void NewLine_(const size_t _depth)
    {
        m_text += '\n';
        m_text.append(_depth * 2, ' ');
    }

    void Number_(const long long _v)
    {
        Separator_();
        m_text += std::to_string(_v);
    }

    void AppendString_(const std::string_view _s)
    {
        m_text += '"';
        for (const char c: _s)
        {
            switch (c)
            {
                case '"': m_text += "\\\""; break;
                case '\\': m_text += "\\\\"; break;
                case '\n': m_text += "\\n"; break;
                case '\r': m_text += "\\r"; break;
                case '\t': m_text += "\\t"; break;
                default:
                    if (static_cast<unsigned char>(c) < 0x20)
                    {
                        char buffer[8];
                        std::snprintf(buffer, sizeof(buffer), "\\u%04x", static_cast<unsigned>(static_cast<unsigned char>(c)));
                        m_text += buffer;
                    }
                    else
                    {
                        m_text += c;
                    }
                    break;
            }
        }
        m_text += '"';
    }

    std::string        m_text;
    std::vector<Level> m_stack;
    bool               m_bAfterKey = false;
};

}   // namespace slug::baker
