#include "Editor/MotionEditor.h"

#include <cmath>
#include <cstdlib>
#include <filesystem>
#include <format>
#include <fstream>
#include <iterator>

#include <SDL3/SDL.h>
#include <imgui.h>

#include "Base/RGBA.h"
#include "Editor/EnemyLab.h"
#include "Editor/JsonScan.h"
#include "Editor/StageEditor.h"
#include "Game/SandboxState.h"
#include "Render/Animation.h"
#include "Render/Renderer.h"

constexpr float  kMotionPickRadius = 8.f;
constexpr double kMotionEpsilon    = 0.00005;

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
        _text.replace(static_cast<size_t>(node.begin), static_cast<size_t>(node.end - node.begin), StageDocument::FormatNumber(now));
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
    const Vec2     _value)
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
        out += std::format("{{ \"clip\": \"{}\", \"offset\": [{}, {}], \"frame\": {}", e.clip, StageDocument::FormatNumber(MotionRound_(static_cast<double>(e.offset.x))), StageDocument::FormatNumber(MotionRound_(static_cast<double>(e.offset.y))), e.frame);
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
    std::ifstream file { _path, std::ios::binary };
    if (!file)
    {
        return Error { eError::FileOpen };
    }
    const String text { std::istreambuf_iterator<char>(file), std::istreambuf_iterator<char>() };
    return LoadFromText(text);
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
    std::ofstream file { _path, std::ios::binary | std::ios::trunc };
    if (!file)
    {
        return Error { eError::FileOpen };
    }
    file.write(m_text.data(), static_cast<std::streamsize>(m_text.size()));
    file.close();
    if (!file)
    {
        return Error { eError::FileOpen };
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

namespace
{
MotionWeaponValues MotionRead_(
    const WeaponDef& _w)
{
    MotionWeaponValues v;
    v.muzzle     = _w.muzzle;
    v.bHasFx     = !_w.muzzleFx.clip.empty();
    v.fxOffset   = _w.muzzleFx.offset;
    v.motionClip = _w.motionClip;
    v.effects    = _w.effects;
    return v;
}

bool MotionIsAttackClip_(
    const String& _name)
{
    static constexpr ARRAY<const char*, 6> kKeys = { "shot", "ready", "fire", "attack", "throw", "slash" };
    for (const char* key: kKeys)
    {
        if (_name.find(key) != String::npos)
        {
            return true;
        }
    }
    return false;
}

ImVec4 MotionColor_(
    const RGBA _c)
{
    return ImVec4 { _c.r / 255.f, _c.g / 255.f, _c.b / 255.f, 1.f };
}

const RGBA kMotionMuzzleColor = { 255, 80, 80, 255 };
const RGBA kMotionFxColor     = { 255, 200, 40, 255 };
const RGBA kMotionEffectColor = { 80, 255, 255, 255 };

EnemyPartDepth* MotionPart_(
    BusCfg&          _c,
    const StringView _name)
{
    if (_name == "innerSoldier")
    {
        return &_c.partInnerSoldier;
    }
    if (_name == "wreck")
    {
        return &_c.partWreck;
    }
    if (_name == "back")
    {
        return &_c.partBack;
    }
    if (_name == "stairs")
    {
        return &_c.partStairs;
    }
    if (_name == "body")
    {
        return &_c.partBody;
    }
    if (_name == "cover")
    {
        return &_c.partCover;
    }
    if (_name == "window")
    {
        return &_c.partWindow;
    }
    if (_name == "debris")
    {
        return &_c.partDebris;
    }
    return nullptr;
}
}   // namespace

void MotionEditor::SetAnimLibrary(
    const AnimationLibrary* _pAnims)
{
    m_pAnims = _pAnims;
}

void MotionEditor::Load_(
    const FilePath& _assetRoot)
{
    m_bLoaded = true;
    std::error_code ec;
    for (const std::filesystem::directory_entry& entry: std::filesystem::directory_iterator(_assetRoot / "Defs", ec))
    {
        if (entry.is_regular_file(ec) && entry.path().extension() == ".json")
        {
            MotionDocument doc;
            if (doc.Load(entry.path()))
            {
                m_paths.push_back(entry.path());
                m_docs.push_back(std::move(doc));
            }
        }
    }
}

void MotionEditor::MarkChanged_(
    const String& _key)
{
    for (const String& key: m_changed)
    {
        if (key == _key)
        {
            return;
        }
    }
    m_changed.push_back(_key);
}

void MotionEditor::Save_(
    EnemyLab& _lab)
{
    SandboxState* pState = _lab.GetSandbox();
    if (pState == nullptr)
    {
        return;
    }
    EnemyDefLibrary& lib     = pState->GetEnemyLibMutable();
    int              saved   = 0;
    int              skipped = 0;
    for (const String& key: m_changed)
    {
        const size_t p1   = key.find('|');
        const size_t p2   = key.find('|', p1 + 1);
        const String kind = key.substr(0, p1);
        const String def  = key.substr(p1 + 1, p2 - p1 - 1);
        const String name = key.substr(p2 + 1);
        EnemyDef*    pDef = lib.FindMutable(def);
        if (pDef == nullptr)
        {
            continue;
        }
        bool bDone = false;
        for (MotionDocument& doc: m_docs)
        {
            if (kind == "w")
            {
                const int w = pDef->FindWeapon(name);
                bDone       = w >= 0 && doc.ApplyWeapon(def, name, MotionRead_(pDef->weapons[static_cast<size_t>(w)]));
            }
            else if (kind == "c")
            {
                const int c = pDef->FindClip(name);
                bDone       = c >= 0 && doc.ApplyClipSec(def, name, static_cast<double>(pDef->GetClip(c).frameSec));
            }
            else
            {
                const EnemyPartDepth* pPart = MotionPart_(pDef->bus, name);
                bDone                       = pPart != nullptr && doc.ApplyPartDepth(def, name, pPart->depth, pPart->sort);
            }
            if (bDone)
            {
                break;
            }
        }
        if (bDone)
        {
            ++saved;
        }
        else
        {
            ++skipped;
        }
    }
    for (size_t i = 0; i < m_docs.size(); ++i)
    {
        if (m_docs[i].IsDirty() && !m_docs[i].Save(m_paths[i]))
        {
            m_status = "저장 실패";
            return;
        }
    }
    m_changed.clear();
    if (saved == 0 && skipped == 0)
    {
        m_status = "저장할 변경 없음 (params 는 고치는 즉시 저장되고, 히트박스는 Hitbox 창에서 저장)";
        return;
    }
    m_status = std::format("저장됨 {}건 (상속/없는 항목 {}건은 파일 미반영)", saved, skipped);
}

namespace
{
const char* MotionParamKo_(
    const String& _key)
{
    if (_key == "fireFrame")
    {
        return "발사 프레임";
    }
    if (_key == "cooldown")
    {
        return "쿨다운";
    }
    if (_key == "readySec")
    {
        return "준비 시간(초)";
    }
    if (_key == "idleSec")
    {
        return "대기 시간(초)";
    }
    if (_key == "maxShots")
    {
        return "최대 발수";
    }
    if (_key == "descendSpeed")
    {
        return "하강 속도";
    }
    if (_key == "driveSpeed")
    {
        return "주행 속도";
    }
    if (_key == "moveMaxSec")
    {
        return "이동 최대 시간(초)";
    }
    if (_key == "detect")
    {
        return "감지 거리";
    }
    return nullptr;
}

String MotionParamPrefix_(
    const String& _label)
{
    size_t end = 0;
    while (end < _label.size() && _label[end] != '.' && !(end > 0 && _label[end] >= 'A' && _label[end] <= 'Z'))
    {
        ++end;
    }
    return _label.substr(0, end);
}
}   // namespace

void MotionEditor::DrawParams_(
    EnemyLab&     _lab,
    const String& _def)
{
    SandboxState* pState = _lab.GetSandbox();
    if (pState == nullptr || !ImGui::CollapsingHeader("패턴 값 (params)"))
    {
        return;
    }
    if (m_paramDef != _def || m_paramDoc >= m_docs.size() || m_docs[m_paramDoc].GetText().size() != m_paramSize)
    {
        m_params.clear();
        m_paramDef = _def;
        for (size_t d = 0; d < m_docs.size() && m_params.empty(); ++d)
        {
            m_params   = m_docs[d].ListParams(_def);
            m_paramDoc = d;
        }
        m_paramSize = m_paramDoc < m_docs.size() ? m_docs[m_paramDoc].GetText().size() : 0;
    }
    if (m_params.empty())
    {
        ImGui::TextUnformatted("이 정의 JSON 에 직접 쓴 params 값이 없습니다 (상속값은 부모 정의에서 편집).");
        return;
    }
    int    edited = -1;
    String prefix;
    for (int i = 0; i < static_cast<int>(m_params.size()); ++i)
    {
        MotionParam& p     = m_params[static_cast<size_t>(i)];
        const String group = MotionParamPrefix_(p.label);
        if (group != prefix)
        {
            ImGui::SeparatorText(group.c_str());
            prefix = group;
        }
        const char*  pKo   = MotionParamKo_(p.path.back());
        const String label = pKo != nullptr ? std::format("{} ({})###{}", pKo, p.label, p.label) : p.label;
        ImGui::PushID(i);
        bool bEdit = false;
        if (p.type == 'b')
        {
            bool b = p.v[0] != 0.0;
            bEdit  = ImGui::Checkbox(label.c_str(), &b);
            p.v[0] = b ? 1.0 : 0.0;
        }
        else if (p.type == 'v')
        {
            ARRAY<float, 2> v = { static_cast<float>(p.v[0]), static_cast<float>(p.v[1]) };
            bEdit             = ImGui::DragFloat2(label.c_str(), v.data(), 0.5f, -4000.f, 4000.f, "%.1f");
            p.v               = { static_cast<double>(v[0]), static_cast<double>(v[1]) };
        }
        else if (p.bInt)
        {
            int v  = static_cast<int>(p.v[0]);
            bEdit  = ImGui::DragInt(label.c_str(), &v, 0.2f);
            p.v[0] = static_cast<double>(v);
        }
        else
        {
            float v = static_cast<float>(p.v[0]);
            bEdit   = ImGui::DragFloat(label.c_str(), &v, 0.01f + Abs(v) * 0.005f, 0.f, 0.f, "%.3f");
            p.v[0]  = static_cast<double>(v);
        }
        ImGui::PopID();
        if (bEdit)
        {
            edited = i;
        }
    }
    if (edited < 0)
    {
        return;
    }
    if (!m_changed.empty())
    {
        Save_(_lab);
    }
    MotionDocument& doc = m_docs[m_paramDoc];
    if (doc.SetParam(_def, m_params[static_cast<size_t>(edited)]) && doc.Save(m_paths[m_paramDoc]))
    {
        m_paramSize = doc.GetText().size();
        (void)_lab.ReloadDefsKeep(*pState);
        m_status = "패턴 값 적용 + 파일 저장: " + m_params[static_cast<size_t>(edited)].label;
    }
    else
    {
        m_status = "패턴 값을 파일에 쓰지 못했습니다";
    }
}

void MotionEditor::DrawParts_(
    EnemyDef& _def)
{
    if (_def.behavior != eEnemyBehavior::Bus || !ImGui::CollapsingHeader("깊이 (z) Depth", ImGuiTreeNodeFlags_DefaultOpen))
    {
        return;
    }
    ImGui::TextUnformatted("참고: 병사 depth 15, 플레이어 20 (클수록 앞)");
    static constexpr ARRAY<const char*, 8> kParts = { "innerSoldier", "wreck", "back", "stairs", "body", "cover", "window", "debris" };
    for (const char* name: kParts)
    {
        EnemyPartDepth* pDepth = MotionPart_(_def.bus, name);
        ImGui::PushID(name);
        ARRAY<int, 2> v = { pDepth->depth, pDepth->sort };
        if (ImGui::DragInt2(name, v.data(), 0.2f, -10, 100))
        {
            pDepth->depth = v[0];
            pDepth->sort  = v[1];
            MarkChanged_(String { "p|" } + _def.name + "|" + name);
        }
        if (ImGui::IsItemHovered())
        {
            ImGui::SetTooltip("params.parts.%s.{depth, sort}", name);
        }
        ImGui::PopID();
    }
}

void MotionEditor::DrawWeapon_(
    EnemyLab& _lab,
    EnemyDef& _def,
    const int _weapon)
{
    WeaponDef& w = _def.weapons[static_cast<size_t>(_weapon)];
    ImGui::PushID(_weapon);
    if (!ImGui::TreeNodeEx(w.name.c_str(), ImGuiTreeNodeFlags_DefaultOpen))
    {
        ImGui::PopID();
        return;
    }
    const String key = "w|" + _def.name + "|" + w.name;
    ImGui::Text("투사체: %s  조준: %s", w.projectile >= 0 ? "있음" : "없음", w.aim == eWeaponAim::Given ? "given" : "facing");
    ARRAY<float, 2> muzzle = { w.muzzle.x, w.muzzle.y };
    ImGui::PushStyleColor(ImGuiCol_Text, MotionColor_(kMotionMuzzleColor));
    ImGui::TextUnformatted("총구 위치 (muzzle)");
    ImGui::PopStyleColor();
    if (ImGui::DragFloat2("##muzzle", muzzle.data(), 0.5f, -2000.f, 2000.f, "%.1f"))
    {
        w.muzzle = { muzzle[0], muzzle[1] };
        MarkChanged_(key);
    }
    if (ImGui::IsItemActive() || ImGui::IsItemHovered())
    {
        m_pick       = ePick::Muzzle;
        m_pickWeapon = _weapon;
    }
    if (!w.muzzleFx.clip.empty())
    {
        ARRAY<float, 2> fx = { w.muzzleFx.offset.x, w.muzzleFx.offset.y };
        ImGui::PushStyleColor(ImGuiCol_Text, MotionColor_(kMotionFxColor));
        ImGui::Text("총구 이펙트 %s (muzzleFx.offset)", w.muzzleFx.clip.c_str());
        ImGui::PopStyleColor();
        if (ImGui::DragFloat2("##fx", fx.data(), 0.5f, -2000.f, 2000.f, "%.1f"))
        {
            w.muzzleFx.offset = { fx[0], fx[1] };
            MarkChanged_(key);
        }
        if (ImGui::IsItemActive() || ImGui::IsItemHovered())
        {
            m_pick       = ePick::MuzzleFx;
            m_pickWeapon = _weapon;
        }
    }

    if (ImGui::BeginCombo("모션 클립 (motionClip)", w.motionClip.empty() ? "(없음)" : w.motionClip.c_str()))
    {
        for (const NamedClip& clip: _def.clips)
        {
            if (ImGui::Selectable(clip.name.c_str(), clip.name == w.motionClip))
            {
                w.motionClip = clip.name;
                MarkChanged_(key);
            }
        }
        ImGui::EndCombo();
    }
    ImGui::DragInt("발사 프레임 (재생용)", &m_fireFrame, 0.1f, 0, 200);
    if (ImGui::Button("지금 발사 (Fire now)"))
    {
        if (SandboxState* pState = _lab.GetSandbox())
        {
            (void)pState->GetEnemies().FireWeaponNow(_lab.GetSelected(), _weapon);
        }
    }
    ImGui::SameLine();
    if (ImGui::Button("모션 재생 (Play motion)"))
    {
        if (SandboxState* pState = _lab.GetSandbox())
        {
            if (!pState->GetEnemies().PlayMotion(_lab.GetSelected(), _weapon, m_fireFrame))
            {
                m_status = "motionClip 을 먼저 고르세요";
            }
        }
    }

    ImGui::PushStyleColor(ImGuiCol_Text, MotionColor_(kMotionEffectColor));
    ImGui::TextUnformatted("이펙트 목록 (effects)");
    ImGui::PopStyleColor();
    int removeAt = -1;
    for (int i = 0; i < static_cast<int>(w.effects.size()); ++i)
    {
        WeaponEffect& e = w.effects[static_cast<size_t>(i)];
        ImGui::PushID(i);
        ImGui::Text("%d: %s", i, e.clip.c_str());
        ARRAY<float, 2> o = { e.offset.x, e.offset.y };
        ImGui::SetNextItemWidth(150.f);
        if (ImGui::DragFloat2("##o", o.data(), 0.5f, -2000.f, 2000.f, "%.1f"))
        {
            e.offset = { o[0], o[1] };
            MarkChanged_(key);
        }
        if (ImGui::IsItemActive() || ImGui::IsItemHovered())
        {
            m_pick       = ePick::Effect;
            m_pickWeapon = _weapon;
            m_pickEffect = i;
        }
        ImGui::SameLine();
        ImGui::SetNextItemWidth(60.f);
        if (ImGui::DragInt("프레임", &e.frame, 0.1f, 0, 200))
        {
            MarkChanged_(key);
        }
        ImGui::SameLine();
        if (ImGui::SmallButton("삭제"))
        {
            removeAt = i;
        }
        ImGui::PopID();
    }
    if (removeAt >= 0)
    {
        w.effects.erase(w.effects.begin() + removeAt);
        MarkChanged_(key);
    }

    ImGui::InputText("클립 검색", m_filter.data(), m_filter.size());
    if (ImGui::BeginCombo("이펙트 추가 (Add effect)", "클립 선택"))
    {
        const String filter = m_filter.data();
        if (m_pAnims != nullptr)
        {
            for (const AnimClip& clip: m_pAnims->GetClips())
            {
                if (!filter.empty() && clip.name.find(filter) == String::npos)
                {
                    continue;
                }
                if (ImGui::Selectable(clip.name.c_str()))
                {
                    WeaponEffect e;
                    e.clip = clip.name;
                    w.effects.push_back(e);
                    if (w.motionClip.empty() && !_def.clips.empty())
                    {
                        w.motionClip = _def.clips[0].name;
                    }
                    MarkChanged_(key);
                }
            }
        }
        ImGui::EndCombo();
    }
    ImGui::TreePop();
    ImGui::PopID();
}

void MotionEditor::Draw(
    EnemyLab& _lab)
{
    if (!m_bLoaded)
    {
        Load_(_lab.GetAssetRoot());
    }
    _lab.SetAutoReloadBlocked(!m_changed.empty());
    const ImGuiViewport* pHost = ImGui::GetMainViewport();
    ImGui::SetNextWindowPos(ImVec2 { pHost->Pos.x + 8.f, pHost->Pos.y + pHost->Size.y - 250.f }, ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowSize(ImVec2 { 330.f, 240.f }, ImGuiCond_FirstUseEver);
    const String title = String { "Motions" } + (m_changed.empty() ? "" : "*") + "###motions";
    if (ImGui::Begin(title.c_str()))
    {
        SandboxState* pState = _lab.GetSandbox();
        EnemyDef*     pDef   = nullptr;
        if (pState != nullptr)
        {
            const Enemy* pEnemy = pState->GetEnemies().Get(_lab.GetSelected());
            if (pEnemy != nullptr)
            {
                pDef = pState->GetEnemyLibMutable().FindMutable(pState->GetEnemyLib().Get(pEnemy->defIndex).name);
            }
            else if (_lab.GetDefIndex() >= 0 && _lab.GetDefIndex() < static_cast<int>(pState->GetEnemyLib().GetDefCount()))
            {
                pDef = pState->GetEnemyLibMutable().FindMutable(pState->GetEnemyLib().Get(_lab.GetDefIndex()).name);
            }
        }
        if (pDef == nullptr)
        {
            ImGui::TextUnformatted("월드에서 적을 클릭하거나 Spawn 의 Def 를 고르세요.");
        }
        else
        {
            ImGui::Text("정의: %s", pDef->name.c_str());
            const bool bKey = ImGui::GetIO().KeyCtrl && ImGui::IsKeyPressed(ImGuiKey_S, false);
            if (ImGui::Button("저장 (Ctrl+S)") || (bKey && !m_changed.empty()))
            {
                Save_(_lab);
            }
            if (!m_status.empty())
            {
                ImGui::TextUnformatted(m_status.c_str());
            }
            if (ImGui::CollapsingHeader("무기 / 모션", ImGuiTreeNodeFlags_DefaultOpen))
            {
                for (int i = 0; i < static_cast<int>(pDef->weapons.size()); ++i)
                {
                    DrawWeapon_(_lab, *pDef, i);
                }
            }
            if (ImGui::CollapsingHeader("공격 클립 (clips)", ImGuiTreeNodeFlags_DefaultOpen))
            {
                for (int i = 0; i < static_cast<int>(pDef->clips.size()); ++i)
                {
                    NamedClip& clip = pDef->clips[static_cast<size_t>(i)];
                    if (!MotionIsAttackClip_(clip.name))
                    {
                        continue;
                    }
                    ImGui::PushID(i);
                    float sec = clip.clip.frameSec;
                    ImGui::SetNextItemWidth(90.f);
                    if (ImGui::DragFloat("##sec", &sec, 0.005f, 0.01f, 2.f, "%.3f s"))
                    {
                        clip.clip.frameSec   = sec;
                        clip.clip.frameUnits = static_cast<int64_t>(std::llround(static_cast<double>(sec) * static_cast<double>(kAnimTimeUnitsPerSec)));
                        MarkChanged_("c|" + pDef->name + "|" + clip.name);
                    }
                    ImGui::SameLine();
                    ImGui::Text("%s  %d프레임%s", clip.name.c_str(), clip.clip.frames, clip.clip.bLoop ? " 반복" : "");
                    ImGui::PopID();
                }
            }
            DrawParts_(*pDef);
            DrawParams_(_lab, String { pDef->name });
        }
    }
    ImGui::End();
}

void MotionEditor::DrawOverlay(
    EnemyLab& _lab,
    Renderer& _renderer)
{
    SandboxState* pState = _lab.GetSandbox();
    if (pState == nullptr)
    {
        return;
    }
    const Enemy* pEnemy = pState->GetEnemies().Get(_lab.GetSelected());
    if (pEnemy == nullptr || pEnemy->life != eEnemyLife::Alive)
    {
        _lab.SetMouseCaptured(false);
        return;
    }
    EnemyDef* pDef = pState->GetEnemyLibMutable().FindMutable(pState->GetEnemyLib().Get(pEnemy->defIndex).name);
    if (pDef == nullptr)
    {
        return;
    }
    const float facing = static_cast<float>(pEnemy->facing < 0 ? -1 : 1);
    const Vec2  feet   = pEnemy->body.position;
    const Vec2  camRaw = pState->GetCamera().GetPosition();
    const Vec2  camera = { std::floor(camRaw.x), std::floor(camRaw.y) };

    float      mouseX     = 0.f;
    float      mouseY     = 0.f;
    const bool bDown      = (SDL_GetMouseState(&mouseX, &mouseY) & SDL_BUTTON_LMASK) != 0;
    const bool bClick     = bDown && !m_bPrevDown;
    m_bPrevDown           = bDown;
    float      logicalX   = 0.f;
    float      logicalY   = 0.f;
    const bool bMouse     = SDL_RenderCoordinatesFromWindow(_renderer.GetNative(), mouseX, mouseY, &logicalX, &logicalY) && !ImGui::GetIO().WantCaptureMouse;
    const Vec2 mouseWorld = { logicalX + camera.x, logicalY + camera.y };

    if (!bDown)
    {
        m_bDragging = false;
    }
    bool bHover = false;
    for (int wi = 0; wi < static_cast<int>(pDef->weapons.size()); ++wi)
    {
        WeaponDef& w = pDef->weapons[static_cast<size_t>(wi)];
        for (int kind = 0; kind < 2 + static_cast<int>(w.effects.size()); ++kind)
        {
            if (kind == 1 && w.muzzleFx.clip.empty())
            {
                continue;
            }
            Vec2 local = w.muzzle;
            if (kind == 1)
            {
                local = { w.muzzle.x + w.muzzleFx.offset.x, w.muzzle.y + w.muzzleFx.offset.y };
            }
            else if (kind >= 2)
            {
                local = w.effects[static_cast<size_t>(kind - 2)].offset;
            }
            const Vec2  at    = { feet.x + local.x * facing, feet.y + local.y };
            const RGBA  color = kind == 0 ? kMotionMuzzleColor : (kind == 1 ? kMotionFxColor : kMotionEffectColor);
            const bool  bSel  = m_pickWeapon == wi && ((kind == 0 && m_pick == ePick::Muzzle) || (kind == 1 && m_pick == ePick::MuzzleFx) || (kind >= 2 && m_pick == ePick::Effect && m_pickEffect == kind - 2));
            const Vec2  s     = { std::floor(at.x - camera.x), std::floor(at.y - camera.y) };
            const float r     = bSel ? 9.f : 6.f;
            _renderer.DrawLine({ s.x - r, s.y }, { s.x + r, s.y }, color);
            _renderer.DrawLine({ s.x, s.y - r }, { s.x, s.y + r }, color);
            const String label = kind == 0 ? w.name + " muzzle" : (kind == 1 ? w.name + " fx" : std::format("{} fx{}", w.name, kind - 2));
            _renderer.DrawDebugText({ s.x + 4.f, s.y - 12.f }, label, color);
            if (bMouse)
            {
                const float dx   = at.x - mouseWorld.x;
                const float dy   = at.y - mouseWorld.y;
                const bool  bHit = dx * dx + dy * dy <= kMotionPickRadius * kMotionPickRadius;
                bHover           = bHover || bHit;
                if (bClick && bHit)
                {
                    m_pick       = kind == 0 ? ePick::Muzzle : (kind == 1 ? ePick::MuzzleFx : ePick::Effect);
                    m_pickWeapon = wi;
                    m_pickEffect = kind - 2;
                    m_bDragging  = true;
                }
            }
        }
    }
    _lab.SetMouseCaptured(bHover || m_bDragging);

    if (m_bDragging && m_pick != ePick::None && m_pickWeapon < static_cast<int>(pDef->weapons.size()))
    {
        WeaponDef& w     = pDef->weapons[static_cast<size_t>(m_pickWeapon)];
        const Vec2 value = { std::round((mouseWorld.x - feet.x) * facing), std::round(mouseWorld.y - feet.y) };
        if (m_pick == ePick::Muzzle)
        {
            w.muzzle = value;
        }
        else if (m_pick == ePick::MuzzleFx)
        {
            w.muzzleFx.offset = { value.x - w.muzzle.x, value.y - w.muzzle.y };
        }
        else if (m_pickEffect < static_cast<int>(w.effects.size()))
        {
            w.effects[static_cast<size_t>(m_pickEffect)].offset = value;
        }
        MarkChanged_("w|" + pDef->name + "|" + w.name);
    }
}
