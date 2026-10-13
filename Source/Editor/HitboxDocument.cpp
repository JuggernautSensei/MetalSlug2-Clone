#include "Editor/HitboxDocument.h"
#include "Core/AssetPath.h"
#include <cmath>
#include <format>

namespace
{

bool IsNumberNode_(
    const Vector<JsonNode>& _nodes,
    const int               _index)
{
    return _index >= 0 && _nodes[static_cast<size_t>(_index)].type == 'n';
}

int FindKey_(
    const JsonNode&  _object,
    const StringView _key)
{
    for (size_t i = 0; i < _object.keys.size(); ++i)
    {
        if (_object.keys[i] == _key)
        {
            return _object.kids[i];
        }
    }
    return -1;
}

double ParseNumber_(
    const String& _text)
{
    double value = 0.0;
    (void)std::from_chars(_text.data(), _text.data() + _text.size(), value);
    return value;
}

String RenderExtraBox_(
    const ExtraBoxDef& _box)
{
    String out = std::format("{{ \"id\": \"{}\", \"kind\": \"{}\", \"w\": {}, \"h\": {}, \"offset\": [{}, {}]",
                             _box.id,
                             _box.kind == eExtraBoxKind::Attack ? "attack" : "hurt",
                             FormatJsonNumber(static_cast<double>(_box.box.w)),
                             FormatJsonNumber(static_cast<double>(_box.box.h)),
                             FormatJsonNumber(static_cast<double>(_box.box.cx)),
                             FormatJsonNumber(static_cast<double>(_box.box.cy)));
    if (_box.kind == eExtraBoxKind::Attack)
    {
        if (_box.frameTo >= 0)
        {
            out += std::format(", \"frames\": [{}, {}]", _box.frameFrom, _box.frameTo);
        }
        if (_box.damage != 1)
        {
            out += std::format(", \"damage\": {}", _box.damage);
        }
    }
    return out + " }";
}

String RenderExtraBoxes_(
    const Vector<ExtraBoxDef>& _boxes)
{
    String out = "[";
    for (size_t i = 0; i < _boxes.size(); ++i)
    {
        out += (i == 0 ? "" : ", ") + RenderExtraBox_(_boxes[i]);
    }
    return out + "]";
}

bool IsSizeKey_(
    const String& _key)
{
    return _key.find("Size") != String::npos || _key.starts_with("hurt");
}
}   // namespace

bool HitboxEntry::HasField(
    const eHitboxField _field) const
{
    return begin[static_cast<size_t>(_field)] >= 0;
}

Result<void> HitboxDocument::Load(
    const FilePath&  _path,
    const StringView _defaultOwner)
{
    const Result<String> text = ReadFileText(_path);
    if (!text)
    {
        return Error { eError::FileOpen };
    }
    return LoadFromText(text.GetValue(), _defaultOwner);
}

Result<void> HitboxDocument::LoadFromText(
    const StringView _json,
    const StringView _defaultOwner)
{
    m_text         = String { _json };
    m_savedText    = m_text;
    m_defaultOwner = String { _defaultOwner };
    if (!Reparse_())
    {
        m_text.clear();
        m_savedText.clear();
        m_entries.clear();
        return Error { eError::FileOpen };
    }
    return Error { kOK };
}

Result<void> HitboxDocument::Save(
    const FilePath& _path)
{
    const Result<void> result = WriteFileBytes(_path, m_text);
    if (!result)
    {
        return result;
    }
    m_savedText = m_text;
    return Error { kOK };
}

bool HitboxDocument::SetValue(
    const int          _entry,
    const eHitboxField _field,
    const double       _value)
{
    if (_entry < 0 || _entry >= static_cast<int>(m_entries.size()))
    {
        return false;
    }
    const HitboxEntry& entry = m_entries[static_cast<size_t>(_entry)];
    const size_t       f     = static_cast<size_t>(_field);
    if (!entry.HasField(_field))
    {
        return false;
    }
    if (Abs(entry.value[f] - _value) < 1e-9)
    {
        return true;
    }
    m_text.replace(static_cast<size_t>(entry.begin[f]), static_cast<size_t>(entry.end[f] - entry.begin[f]), FormatJsonNumber(_value));
    return Reparse_();
}

bool HitboxDocument::ApplyExtraBoxes(
    const StringView           _def,
    const Vector<ExtraBoxDef>& _boxes)
{
    JsonScanner scanner { m_text };
    const int   root = scanner.Parse();
    if (root < 0)
    {
        return false;
    }
    const int defs = FindKey_(scanner.nodes[static_cast<size_t>(root)], "defs");
    const int obj  = defs < 0 ? -1 : FindKey_(scanner.nodes[static_cast<size_t>(defs)], _def);
    if (obj < 0 || scanner.nodes[static_cast<size_t>(obj)].type != 'o')
    {
        return false;
    }
    const JsonNode& object = scanner.nodes[static_cast<size_t>(obj)];
    const int       kid    = FindKey_(object, "hitboxes");
    const String    text   = RenderExtraBoxes_(_boxes);
    if (kid < 0)
    {
        if (_boxes.empty())
        {
            return true;
        }
        const String member = "\"hitboxes\": " + text;
        if (object.kids.empty())
        {
            m_text.insert(static_cast<size_t>(object.begin) + 1, " " + member + " ");
        }
        else
        {
            m_text.insert(static_cast<size_t>(scanner.nodes[static_cast<size_t>(object.kids.back())].end), ", " + member);
        }
        return Reparse_();
    }
    const JsonNode& array = scanner.nodes[static_cast<size_t>(kid)];
    if (m_text.compare(static_cast<size_t>(array.begin), static_cast<size_t>(array.end - array.begin), text) == 0)
    {
        return true;
    }
    if (!_boxes.empty())
    {
        m_text.replace(static_cast<size_t>(array.begin), static_cast<size_t>(array.end - array.begin), text);
        return Reparse_();
    }
    const size_t key = m_text.rfind("\"hitboxes\"", static_cast<size_t>(array.begin));
    if (key == String::npos)
    {
        return false;
    }
    size_t from = key;
    while (from > 0 && (m_text[from - 1] == ' ' || m_text[from - 1] == '\t' || m_text[from - 1] == '\r' || m_text[from - 1] == '\n'))
    {
        --from;
    }
    size_t to = static_cast<size_t>(array.end);
    if (from > 0 && m_text[from - 1] == ',')
    {
        from -= 1;
    }
    else
    {
        from = key;
        while (to < m_text.size() && (m_text[to] == ' ' || m_text[to] == '\t' || m_text[to] == '\r' || m_text[to] == '\n'))
        {
            ++to;
        }
        if (to < m_text.size() && m_text[to] == ',')
        {
            ++to;
        }
    }
    m_text.erase(from, to - from);
    return Reparse_();
}

void HitboxDocument::Revert()
{
    m_text = m_savedText;
    (void)Reparse_();
}

const String& HitboxDocument::GetText() const
{
    return m_text;
}

Span<const HitboxEntry> HitboxDocument::GetEntries() const
{
    return Span<const HitboxEntry> { m_entries.data(), m_entries.size() };
}

bool HitboxDocument::IsDirty() const
{
    return m_text != m_savedText;
}

bool HitboxDocument::Reparse_()
{
    JsonScanner scanner { m_text };
    const int   root = scanner.Parse();
    if (root < 0 || scanner.nodes[static_cast<size_t>(root)].type != 'o')
    {
        return false;
    }
    m_entries.clear();
    Collect_(scanner.nodes, root, String {}, m_defaultOwner);
    return true;
}

void HitboxDocument::Collect_(
    const Vector<JsonNode>& _nodes,
    const int               _index,
    const String&           _path,
    const String&           _owner)
{
    const JsonNode& node = _nodes[static_cast<size_t>(_index)];
    if (node.type == 'a')
    {
        for (size_t i = 0; i < node.kids.size(); ++i)
        {
            Collect_(_nodes, node.kids[i], std::format("{}[{}]", _path, i), _owner);
        }
        return;
    }
    if (node.type != 'o')
    {
        return;
    }

    const int w = FindKey_(node, "w");
    const int h = FindKey_(node, "h");
    if (IsNumberNode_(_nodes, w) && IsNumberNode_(_nodes, h))
    {
        HitboxEntry entry;
        entry.owner              = _owner;
        entry.label              = _path;
        const ARRAY<int, 4> kids = { w, h, FindKey_(node, "cx"), FindKey_(node, "cy") };
        for (size_t f = 0; f < kids.size(); ++f)
        {
            if (IsNumberNode_(_nodes, kids[f]))
            {
                const JsonNode& number = _nodes[static_cast<size_t>(kids[f])];
                entry.value[f]         = ParseNumber_(number.str);
                entry.begin[f]         = number.begin;
                entry.end[f]           = number.end;
            }
        }
        m_entries.push_back(entry);
    }
    for (size_t i = 0; i < node.kids.size(); ++i)
    {
        const String& key = node.keys[i];
        if (key == "hitboxes")
        {
            continue;
        }
        const int       kid       = node.kids[i];
        const JsonNode& kidNode   = _nodes[static_cast<size_t>(kid)];
        const String    childPath = _path.empty() ? key : _path + "." + key;
        const bool      bPair     = kidNode.type == 'a' && kidNode.kids.size() == 2 && IsSizeKey_(key) && IsNumberNode_(_nodes, kidNode.kids[0]) && IsNumberNode_(_nodes, kidNode.kids[1]);
        if (bPair)
        {
            HitboxEntry entry;
            entry.owner = _owner;
            entry.label = childPath;
            for (size_t f = 0; f < 2; ++f)
            {
                const JsonNode& number = _nodes[static_cast<size_t>(kidNode.kids[f])];
                entry.value[f]         = ParseNumber_(number.str);
                entry.begin[f]         = number.begin;
                entry.end[f]           = number.end;
            }
            const size_t sizePos = key.rfind("Size");
            if (sizePos != String::npos && sizePos + 4 == key.size())
            {
                const int offsetNode = FindKey_(node, key.substr(0, sizePos) + "Offset");
                if (offsetNode >= 0)
                {
                    const JsonNode& offsetArr = _nodes[static_cast<size_t>(offsetNode)];
                    if (offsetArr.type == 'a' && offsetArr.kids.size() == 2 && IsNumberNode_(_nodes, offsetArr.kids[0]) && IsNumberNode_(_nodes, offsetArr.kids[1]))
                    {
                        for (size_t f = 0; f < 2; ++f)
                        {
                            const JsonNode& number = _nodes[static_cast<size_t>(offsetArr.kids[f])];
                            entry.value[2 + f]     = ParseNumber_(number.str);
                            entry.begin[2 + f]     = number.begin;
                            entry.end[2 + f]       = number.end;
                        }
                    }
                }
            }
            m_entries.push_back(entry);
            continue;
        }
        const bool bOwnerLevel = _path == "defs" || _path == "projectiles";
        Collect_(_nodes, kid, childPath, bOwnerLevel ? key : _owner);
    }
}
