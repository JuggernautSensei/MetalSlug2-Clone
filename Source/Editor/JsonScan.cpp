#include "Editor/JsonScan.h"

JsonScanner::JsonScanner(
    const StringView _text)
    : m_text(_text)
{
    if (m_text.size() >= 3 && m_text.substr(0, 3) == "\xEF\xBB\xBF")
    {
        m_pos = 3;
    }
}

int JsonScanner::Parse()
{
    const int root = Value_(0);
    SkipWs_();
    if (m_bFail || m_pos != m_text.size())
    {
        return -1;
    }
    return root;
}

void JsonScanner::SkipWs_()
{
    while (m_pos < m_text.size() && (m_text[m_pos] == ' ' || m_text[m_pos] == '\t' || m_text[m_pos] == '\r' || m_text[m_pos] == '\n'))
    {
        ++m_pos;
    }
}

bool JsonScanner::ReadString_(
    String& _out)
{
    ++m_pos;
    const size_t start = m_pos;
    while (m_pos < m_text.size() && m_text[m_pos] != '"')
    {
        m_pos += (m_text[m_pos] == '\\') ? 2 : 1;
    }
    if (m_pos >= m_text.size())
    {
        return false;
    }
    _out.assign(m_text.substr(start, m_pos - start));
    ++m_pos;
    return true;
}

int JsonScanner::Value_(
    const int _depth)
{
    SkipWs_();
    if (m_bFail || m_pos >= m_text.size() || _depth > 64)
    {
        m_bFail = true;
        return -1;
    }
    const int idx = static_cast<int>(nodes.size());
    nodes.emplace_back();
    nodes[static_cast<size_t>(idx)].begin = static_cast<int>(m_pos);
    const char c = m_text[m_pos];
    if (c == '{' || c == '[')
    {
        const bool bObject = (c == '{');
        const char close   = bObject ? '}' : ']';
        nodes[static_cast<size_t>(idx)].type = bObject ? 'o' : 'a';
        ++m_pos;
        for (;;)
        {
            SkipWs_();
            if (m_pos >= m_text.size())
            {
                m_bFail = true;
                return -1;
            }
            if (m_text[m_pos] == close)
            {
                ++m_pos;
                break;
            }
            String key;
            if (bObject)
            {
                if (m_text[m_pos] != '"' || !ReadString_(key))
                {
                    m_bFail = true;
                    return -1;
                }
                SkipWs_();
                if (m_pos >= m_text.size() || m_text[m_pos] != ':')
                {
                    m_bFail = true;
                    return -1;
                }
                ++m_pos;
            }
            const int kid = Value_(_depth + 1);
            if (m_bFail)
            {
                return -1;
            }
            JsonNode& node = nodes[static_cast<size_t>(idx)];
            if (bObject)
            {
                node.keys.push_back(key);
            }
            node.kids.push_back(kid);
            SkipWs_();
            if (m_pos < m_text.size() && m_text[m_pos] == ',')
            {
                ++m_pos;
            }
        }
    }
    else if (c == '"')
    {
        nodes[static_cast<size_t>(idx)].type = 's';
        String text;
        if (!ReadString_(text))
        {
            m_bFail = true;
            return -1;
        }
        nodes[static_cast<size_t>(idx)].str = text;
    }
    else
    {
        const size_t start = m_pos;
        while (m_pos < m_text.size() && String { ",]} \t\r\n" }.find(m_text[m_pos]) == String::npos)
        {
            ++m_pos;
        }
        if (m_pos == start)
        {
            m_bFail = true;
            return -1;
        }
        JsonNode& node = nodes[static_cast<size_t>(idx)];
        node.str       = String { m_text.substr(start, m_pos - start) };
        node.type      = (c == '-' || (c >= '0' && c <= '9')) ? 'n' : 'l';
    }
    nodes[static_cast<size_t>(idx)].end = static_cast<int>(m_pos);
    return idx;
}
