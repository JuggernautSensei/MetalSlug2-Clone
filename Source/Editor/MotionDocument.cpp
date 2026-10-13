#include "Editor/MotionDocument.h"
#include "Editor/JsonScan.h"
#include "Core/AssetPath.h"

#include <cmath>
#include <cstdlib>
#include <format>

constexpr double kMotionEpsilon = 0.00005;

namespace
{
double MotionRound_(
    const double _value)
{
    return std::round(_value * 10000.0) / 10000.0;
}

int MotionKey_(
    const JsonNode&  _node,
    const StringView _key)
{
    if (_node.type == 'a')
    {
        const int index = _key.empty() ? -1 : _key[0] - '0';
        return index >= 0 && index < static_cast<int>(_node.kids.size()) ? _node.kids[static_cast<size_t>(index)] : -1;
    }
    for (size_t i = 0; i < _node.keys.size(); ++i)
    {
        if (_node.keys[i] == _key)
        {
            return _node.kids[i];
        }
    }
    return -1;
}

int MotionPath_(
    const Vector<JsonNode>& _nodes,
    const int               _root,
    const Vector<String>&   _path)
{
    int index = _root;
    for (const String& key: _path)
    {
        index = MotionKey_(_nodes[static_cast<size_t>(index)], key);
        if (index < 0)
        {
            return -1;
        }
    }
    return index;
}

Vector<String> MotionJoin_(
    Vector<String>   _base,
    const StringView _a,
    const StringView _b = {})
{
    _base.push_back(String { _a });
    if (!_b.empty())
    {
        _base.push_back(String { _b });
    }
    return _base;
}

void MotionInsertMember_(
    String&                 _text,
    const Vector<JsonNode>& _nodes,
    const int               _object,
    const String&           _member)
{
    const JsonNode& obj = _nodes[static_cast<size_t>(_object)];
    if (obj.kids.empty())
    {
        _text.insert(static_cast<size_t>(obj.begin) + 1, " " + _member + " ");
        return;
    }
    const JsonNode& last = _nodes[static_cast<size_t>(obj.kids.back())];
    _text.insert(static_cast<size_t>(last.end), ", " + _member);
}

bool MotionPatchNumber_(
    String&               _text,
    const Vector<String>& _path,
    const double          _value)
{
    JsonScanner scanner { _text };
    const int   root = scanner.Parse();
    if (root < 0)
    {
        return false;
    }
    const int index = MotionPath_(scanner.nodes, root, _path);
    if (index < 0 || scanner.nodes[static_cast<size_t>(index)].type != 'n')
    {
        return false;
    }
    const JsonNode& node = scanner.nodes[static_cast<size_t>(index)];
    const double    old  = std::strtod(node.str.c_str(), nullptr);
    const double    now  = MotionRound_(_value);
    if (Abs(old - now) > kMotionEpsilon)
    {
        _text.replace(static_cast<size_t>(node.begin), static_cast<size_t>(node.end - node.begin), FormatJsonNumber(now));
    }
    return true;
}

bool MotionPatchBool_(
    String&               _text,
    const Vector<String>& _path,
    const bool            _value)
{
    JsonScanner scanner { _text };
    const int   root = scanner.Parse();
    if (root < 0)
    {
        return false;
    }
    const int index = MotionPath_(scanner.nodes, root, _path);
    if (index < 0 || scanner.nodes[static_cast<size_t>(index)].type != 'l')
    {
        return false;
    }
    const JsonNode& node = scanner.nodes[static_cast<size_t>(index)];
    _text.replace(static_cast<size_t>(node.begin), static_cast<size_t>(node.end - node.begin), _value ? "true" : "false");
    return true;
}

void MotionCollectParams_(
    const Vector<JsonNode>& _nodes,
    const int               _object,
    const Vector<String>&   _path,
    const String&           _label,
    const int               _depth,
    Vector<MotionParam>&    _out)
{
    const JsonNode& object = _nodes[static_cast<size_t>(_object)];
    for (size_t i = 0; i < object.kids.size(); ++i)
    {
        const JsonNode& kid = _nodes[static_cast<size_t>(object.kids[i])];
        MotionParam     p;
        p.label = _label.empty() ? object.keys[i] : _label + "." + object.keys[i];
        p.path  = _path;
        p.path.push_back(object.keys[i]);
        if (kid.type == 'n')
        {
            p.v[0] = std::strtod(kid.str.c_str(), nullptr);
            p.bInt = kid.str.find_first_of(".eE") == String::npos;
            _out.push_back(p);
        }
        else if (kid.type == 'l' && (kid.str == "true" || kid.str == "false"))
        {
            p.type = 'b';
            p.v[0] = kid.str == "true" ? 1.0 : 0.0;
            _out.push_back(p);
        }
        else if (kid.type == 'a' && kid.kids.size() == 2 && _nodes[static_cast<size_t>(kid.kids[0])].type == 'n' && _nodes[static_cast<size_t>(kid.kids[1])].type == 'n')
        {
            p.type = 'v';
            p.v[0] = std::strtod(_nodes[static_cast<size_t>(kid.kids[0])].str.c_str(), nullptr);
            p.v[1] = std::strtod(_nodes[static_cast<size_t>(kid.kids[1])].str.c_str(), nullptr);
            _out.push_back(p);
        }
        else if (kid.type == 'o' && _depth < 3)
        {
            MotionCollectParams_(_nodes, object.kids[i], p.path, p.label, _depth + 1, _out);
        }
    }
}

bool MotionPatchVec2_(
    String&        _text,
    Vector<String> _path,
    const VECTOR2  _value)
{
    _path.push_back("0");
    if (!MotionPatchNumber_(_text, _path, static_cast<double>(_value.x)))
    {
        return false;
    }
    _path.back() = "1";
    return MotionPatchNumber_(_text, _path, static_cast<double>(_value.y));
}

String MotionRenderEffects_(
    const Vector<WeaponEffect>& _effects)
{
    String out = "[";
    for (size_t i = 0; i < _effects.size(); ++i)
    {
        const WeaponEffect& e = _effects[i];
        out += i == 0 ? "" : ", ";
        out += std::format("{{ \"clip\": \"{}\", \"offset\": [{}, {}], \"frame\": {}", e.clip, FormatJsonNumber(MotionRound_(static_cast<double>(e.offset.x))), FormatJsonNumber(MotionRound_(static_cast<double>(e.offset.y))), e.frame);
        if (e.depth != 60)
        {
            out += std::format(", \"depth\": {}", e.depth);
        }
        out += " }";
    }
    return out + "]";
}

bool MotionSameEffects_(
    const Vector<WeaponEffect>& _a,
    const Vector<WeaponEffect>& _b)
{
    if (_a.size() != _b.size())
    {
        return false;
    }
    for (size_t i = 0; i < _a.size(); ++i)
    {
        const double dx = MotionRound_(static_cast<double>(_a[i].offset.x)) - MotionRound_(static_cast<double>(_b[i].offset.x));
        const double dy = MotionRound_(static_cast<double>(_a[i].offset.y)) - MotionRound_(static_cast<double>(_b[i].offset.y));
        if (_a[i].clip != _b[i].clip || _a[i].frame != _b[i].frame || _a[i].depth != _b[i].depth || Abs(dx) > kMotionEpsilon || Abs(dy) > kMotionEpsilon)
        {
            return false;
        }
    }
    return true;
}

bool MotionPatchString_(
    String&               _text,
    const Vector<String>& _object,
    const StringView      _key,
    const String&         _value)
{
    JsonScanner scanner { _text };
    const int   root = scanner.Parse();
    if (root < 0)
    {
        return false;
    }
    const int obj = MotionPath_(scanner.nodes, root, _object);
    if (obj < 0)
    {
        return false;
    }
    const int kid = MotionKey_(scanner.nodes[static_cast<size_t>(obj)], _key);
    if (kid >= 0)
    {
        const JsonNode& node = scanner.nodes[static_cast<size_t>(kid)];
        if (node.type == 's' && node.str != _value)
        {
            _text.replace(static_cast<size_t>(node.begin), static_cast<size_t>(node.end - node.begin), "\"" + _value + "\"");
        }
    }
    else if (!_value.empty())
    {
        MotionInsertMember_(_text, scanner.nodes, obj, std::format("\"{}\": \"{}\"", _key, _value));
    }
    return true;
}

bool MotionPatchEffects_(
    String&                     _text,
    const Vector<String>&       _weapon,
    const Vector<WeaponEffect>& _effects)
{
    JsonScanner scanner { _text };
    const int   root = scanner.Parse();
    if (root < 0)
    {
        return false;
    }
    const int obj = MotionPath_(scanner.nodes, root, _weapon);
    if (obj < 0)
    {
        return false;
    }
    const int kid = MotionKey_(scanner.nodes[static_cast<size_t>(obj)], "effects");
    if (kid < 0)
    {
        if (!_effects.empty())
        {
            MotionInsertMember_(_text, scanner.nodes, obj, "\"effects\": " + MotionRenderEffects_(_effects));
        }
        return true;
    }
    const JsonNode&      array = scanner.nodes[static_cast<size_t>(kid)];
    Vector<WeaponEffect> old;
    for (const int itemIndex: array.kids)
    {
        const JsonNode& item = scanner.nodes[static_cast<size_t>(itemIndex)];
        WeaponEffect    e;
        const int       clip   = MotionKey_(item, "clip");
        const int       offset = MotionKey_(item, "offset");
        const int       frame  = MotionKey_(item, "frame");
        const int       depth  = MotionKey_(item, "depth");
        e.clip                 = clip >= 0 ? scanner.nodes[static_cast<size_t>(clip)].str : String {};
        if (offset >= 0 && scanner.nodes[static_cast<size_t>(offset)].kids.size() == 2)
        {
            const JsonNode& o = scanner.nodes[static_cast<size_t>(offset)];
            e.offset          = { static_cast<float>(std::strtod(scanner.nodes[static_cast<size_t>(o.kids[0])].str.c_str(), nullptr)), static_cast<float>(std::strtod(scanner.nodes[static_cast<size_t>(o.kids[1])].str.c_str(), nullptr)) };
        }
        e.frame = frame >= 0 ? static_cast<int>(std::strtol(scanner.nodes[static_cast<size_t>(frame)].str.c_str(), nullptr, 10)) : 0;
        e.depth = depth >= 0 ? static_cast<int>(std::strtol(scanner.nodes[static_cast<size_t>(depth)].str.c_str(), nullptr, 10)) : 60;
        old.push_back(e);
    }
    if (!MotionSameEffects_(old, _effects))
    {
        _text.replace(static_cast<size_t>(array.begin), static_cast<size_t>(array.end - array.begin), MotionRenderEffects_(_effects));
    }
    return true;
}
}   // namespace

Result<void> MotionDocument::Load(
    const FilePath& _path)
{
    const Result<String> text = ReadFileText(_path);
    if (!text)
    {
        return Error { eError::FileOpen };
    }
    return LoadFromText(text.GetValue());
}

Result<void> MotionDocument::LoadFromText(
    const StringView _json)
{
    m_text      = String { _json };
    m_savedText = m_text;
    return Error { kOK };
}

Result<void> MotionDocument::Save(
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

bool MotionDocument::ApplyWeapon(
    const StringView          _def,
    const StringView          _weapon,
    const MotionWeaponValues& _values)
{
    const Vector<String> base = MotionJoin_({ "defs" }, _def);
    const Vector<String> path = MotionJoin_(MotionJoin_(base, "weapons"), _weapon);
    {
        JsonScanner scanner { m_text };
        const int   root = scanner.Parse();
        if (root < 0 || MotionPath_(scanner.nodes, root, path) < 0)
        {
            return false;
        }
    }
    (void)MotionPatchVec2_(m_text, MotionJoin_(path, "muzzle"), _values.muzzle);
    if (_values.bHasFx)
    {
        (void)MotionPatchVec2_(m_text, MotionJoin_(path, "muzzleFx", "offset"), _values.fxOffset);
    }
    (void)MotionPatchString_(m_text, path, "motionClip", _values.motionClip);
    (void)MotionPatchEffects_(m_text, path, _values.effects);
    return true;
}

bool MotionDocument::ApplyClipSec(
    const StringView _def,
    const StringView _clip,
    const double     _sec)
{
    return MotionPatchNumber_(m_text, MotionJoin_(MotionJoin_(MotionJoin_({ "defs" }, _def), "clips", _clip), "sec"), _sec);
}

bool MotionDocument::ApplyPartDepth(
    const StringView _def,
    const StringView _part,
    const int        _depth,
    const int        _sort)
{
    const Vector<String> part = MotionJoin_(MotionJoin_(MotionJoin_({ "defs" }, _def), "params", "parts"), _part);
    return MotionPatchNumber_(m_text, MotionJoin_(part, "depth"), static_cast<double>(_depth)) && MotionPatchNumber_(m_text, MotionJoin_(part, "sort"), static_cast<double>(_sort));
}

namespace
{
bool MotionSetInt_(
    String&               _text,
    const Vector<String>& _object,
    const StringView      _key,
    const int             _value,
    const int             _default)
{
    JsonScanner scanner { _text };
    const int   root = scanner.Parse();
    const int   obj  = root < 0 ? -1 : MotionPath_(scanner.nodes, root, _object);
    if (obj < 0 || scanner.nodes[static_cast<size_t>(obj)].type != 'o')
    {
        return false;
    }
    if (MotionKey_(scanner.nodes[static_cast<size_t>(obj)], _key) >= 0)
    {
        return MotionPatchNumber_(_text, MotionJoin_(_object, _key), static_cast<double>(_value));
    }
    if (_value != _default)
    {
        MotionInsertMember_(_text, scanner.nodes, obj, std::format("\"{}\": {}", _key, _value));
    }
    return true;
}
}   // namespace

bool MotionDocument::SetPartDepth(
    const StringView _def,
    const StringView _part,
    const int        _depth,
    const int        _sort,
    const int        _defaultDepth,
    const int        _defaultSort)
{
    const Vector<String> params = MotionJoin_(MotionJoin_({ "defs" }, _def), "params");
    const bool           bSame  = _depth == _defaultDepth && _sort == _defaultSort;
    const String         body   = std::format("\"{}\": {{ \"depth\": {}, \"sort\": {} }}", _part, _depth, _sort);
    {
        JsonScanner scanner { m_text };
        const int   root      = scanner.Parse();
        const int   paramsIdx = root < 0 ? -1 : MotionPath_(scanner.nodes, root, params);
        if (paramsIdx < 0 || scanner.nodes[static_cast<size_t>(paramsIdx)].type != 'o')
        {
            return false;
        }
        const int partsIdx = MotionKey_(scanner.nodes[static_cast<size_t>(paramsIdx)], "parts");
        if (partsIdx < 0)
        {
            if (!bSame)
            {
                MotionInsertMember_(m_text, scanner.nodes, paramsIdx, "\"parts\": { " + body + " }");
            }
            return true;
        }
        if (scanner.nodes[static_cast<size_t>(partsIdx)].type != 'o')
        {
            return false;
        }
        if (MotionKey_(scanner.nodes[static_cast<size_t>(partsIdx)], _part) < 0)
        {
            if (!bSame)
            {
                MotionInsertMember_(m_text, scanner.nodes, partsIdx, body);
            }
            return true;
        }
    }
    const Vector<String> part = MotionJoin_(MotionJoin_(params, "parts"), _part);
    return MotionSetInt_(m_text, part, "depth", _depth, _defaultDepth) && MotionSetInt_(m_text, part, "sort", _sort, _defaultSort);
}

Vector<MotionParam> MotionDocument::ListParams(
    const StringView _def) const
{
    Vector<MotionParam> out;
    JsonScanner         scanner { m_text };
    const int           root = scanner.Parse();
    if (root < 0)
    {
        return out;
    }
    const Vector<String> path   = MotionJoin_({ "defs" }, _def, "params");
    const int            params = MotionPath_(scanner.nodes, root, path);
    if (params >= 0 && scanner.nodes[static_cast<size_t>(params)].type == 'o')
    {
        MotionCollectParams_(scanner.nodes, params, {}, {}, 0, out);
    }
    return out;
}

bool MotionDocument::SetParam(
    const StringView   _def,
    const MotionParam& _param)
{
    Vector<String> path = MotionJoin_({ "defs" }, _def, "params");
    for (const String& key: _param.path)
    {
        path.push_back(key);
    }
    if (_param.type == 'b')
    {
        return MotionPatchBool_(m_text, path, _param.v[0] != 0.0);
    }
    if (_param.type == 'v')
    {
        return MotionPatchVec2_(m_text, path, { static_cast<float>(_param.v[0]), static_cast<float>(_param.v[1]) });
    }
    return MotionPatchNumber_(m_text, path, _param.v[0]);
}

const String& MotionDocument::GetText() const
{
    return m_text;
}

bool MotionDocument::IsDirty() const
{
    return m_text != m_savedText;
}
