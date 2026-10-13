#include "Editor/StageEditor.h"

#include <algorithm>
#include <charconv>
#include <cmath>
#include <format>
#include <fstream>
#include <iterator>

#include <SDL3/SDL.h>
#include <imgui.h>
#pragma warning(disable : 4505)
#define STB_IMAGE_STATIC
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#include "Base/Json.h"
#include "Editor/JsonScan.h"
#include "Game/Enemies/BigShieeCfg.h"
#include "Game/Enemies/EnemyWorld.h"
#include "Game/Props/PropDef.h"
#include "Core/AssetPath.h"

constexpr size_t kMaxUndo     = 100;
constexpr int    kStripWidth  = 2048;
constexpr float  kRulerHeight = 20.f;

struct Extractor
{
    const Vector<JsonNode>& n;
    Vector<StageEntity>&    entities;
    Vector<StageEditRect>*  pRects = nullptr;

    [[nodiscard]] int Find(const int _obj, const StringView _key) const
    {
        if (_obj < 0 || n[static_cast<size_t>(_obj)].type != 'o')
        {
            return -1;
        }
        const JsonNode& o = n[static_cast<size_t>(_obj)];
        for (size_t i = 0; i < o.keys.size(); ++i)
        {
            if (o.keys[i] == _key)
            {
                return o.kids[i];
            }
        }
        return -1;
    }

    [[nodiscard]] String Str(const int _node) const
    {
        if (_node >= 0 && n[static_cast<size_t>(_node)].type == 's')
        {
            return n[static_cast<size_t>(_node)].str;
        }
        return String {};
    }

    [[nodiscard]] bool Num(const int _node, double& _out) const
    {
        if (_node < 0 || n[static_cast<size_t>(_node)].type != 'n')
        {
            return false;
        }
        const String& s = n[static_cast<size_t>(_node)].str;
        return std::from_chars(s.data(), s.data() + s.size(), _out).ec == std::errc {};
    }

    void AddRect(const int _obj, const String& _type)
    {
        const int r = Find(_obj, "rect");
        if (r < 0 || n[static_cast<size_t>(r)].type != 'a' || n[static_cast<size_t>(r)].kids.size() != 4)
        {
            return;
        }
        StageEditRect rect;
        rect.type = _type;
        for (size_t i = 0; i < 4; ++i)
        {
            const int k = n[static_cast<size_t>(r)].kids[i];
            if (!Num(k, rect.v[i]))
            {
                return;
            }
            rect.begin[i] = n[static_cast<size_t>(k)].begin;
            rect.end[i]   = n[static_cast<size_t>(k)].end;
        }
        const int g = Find(_obj, "glide");
        if (Num(g, rect.glide))
        {
            rect.glideBegin = n[static_cast<size_t>(g)].begin;
            rect.glideEnd   = n[static_cast<size_t>(g)].end;
        }
        pRects->push_back(rect);
    }

    void AddEntity(const int _obj, const int _section, const String& _where, const bool _bPlayer)
    {
        StageEntity e;
        e.def  = _bPlayer ? String { "player" } : Str(Find(_obj, "def"));
        e.kind = _bPlayer ? String { "player" } : Str(Find(_obj, "kind"));
        if (e.def.empty() || e.kind.empty())
        {
            return;
        }
        const JsonNode& o = n[static_cast<size_t>(_obj)];
        e.section         = _section;
        e.where           = _where;
        e.begin           = o.begin;
        e.end             = o.end;
        e.bCanCopy        = !_bPlayer;
        e.bFaceLeft       = (Str(Find(_obj, "facing")) == "left");
        const int idN     = Find(_obj, "id");
        e.id              = Str(idN);
        if (idN >= 0 && n[static_cast<size_t>(idN)].type == 's')
        {
            e.idBegin = n[static_cast<size_t>(idN)].begin;
            e.idEnd   = n[static_cast<size_t>(idN)].end;
        }
        const int xN = Find(_obj, "x");
        const int yN = Find(_obj, "y");
        e.bHasX      = Num(xN, e.x);
        e.bHasY      = Num(yN, e.y);
        if (e.bHasX)
        {
            e.xBegin = n[static_cast<size_t>(xN)].begin;
            e.xEnd   = n[static_cast<size_t>(xN)].end;
        }
        if (e.bHasY)
        {
            e.yBegin = n[static_cast<size_t>(yN)].begin;
            e.yEnd   = n[static_cast<size_t>(yN)].end;
        }
        const int zN = Find(_obj, "z");
        double    zv = 0.0;
        if (Num(zN, zv))
        {
            e.bHasZ  = true;
            e.z      = static_cast<int>(zv);
            e.zBegin = n[static_cast<size_t>(zN)].begin;
            e.zEnd   = n[static_cast<size_t>(zN)].end;
        }
        const int snapN = Find(_obj, "groundSnap");
        e.bGroundSnap   = (snapN >= 0 && n[static_cast<size_t>(snapN)].str == "true");
        if (e.bGroundSnap)
        {
            e.snapBegin = n[static_cast<size_t>(snapN)].begin;
            e.snapEnd   = n[static_cast<size_t>(snapN)].end;
        }
        double fromY = e.fromY;
        if (Num(Find(_obj, "fromY"), fromY))
        {
            e.fromY = fromY;
        }
        entities.push_back(e);
    }

    void Action(const int _obj, const int _section, const String& _where)
    {
        const String type = Str(Find(_obj, "type"));
        if (type == "spawn")
        {
            AddEntity(_obj, _section, _where, false);
        }
        else if (type == "spawnWave")
        {
            const int entries = Find(_obj, "entries");
            if (entries >= 0)
            {
                for (const int entry: n[static_cast<size_t>(entries)].kids)
                {
                    AddEntity(entry, _section, _where + " (wave)", false);
                }
            }
        }
        else if ((type == "lockCamera" || type == "setBounds") && pRects != nullptr)
        {
            AddRect(_obj, type);
        }
        else if (type == "gate")
        {
            const int    actions = Find(_obj, "actions");
            const String where   = _where + " > " + Str(Find(_obj, "id"));
            if (actions >= 0)
            {
                for (const int a: n[static_cast<size_t>(actions)].kids)
                {
                    Action(a, _section, where);
                }
            }
        }
    }
};

namespace
{
[[nodiscard]] String WsBefore_(
    const String& _text,
    const int     _pos)
{
    int start = _pos;
    while (start > 0 && (_text[static_cast<size_t>(start - 1)] == ' ' || _text[static_cast<size_t>(start - 1)] == '\t' || _text[static_cast<size_t>(start - 1)] == '\n' || _text[static_cast<size_t>(start - 1)] == '\r'))
    {
        --start;
    }
    return _text.substr(static_cast<size_t>(start), static_cast<size_t>(_pos - start));
}

[[nodiscard]] bool IsWs_(
    const char _c)
{
    return _c == ' ' || _c == '\t' || _c == '\n' || _c == '\r';
}
}   // namespace

struct Replacement
{
    int    begin = 0;
    int    end   = 0;
    String text  = {};
};
namespace
{
void ApplyReplacements_(
    String&              _text,
    Vector<Replacement>& _list)
{
    std::sort(_list.begin(), _list.end(), [](const Replacement& _a, const Replacement& _b) {
        return _a.begin > _b.begin;
    });
    for (const Replacement& r: _list)
    {
        _text.replace(static_cast<size_t>(r.begin), static_cast<size_t>(r.end - r.begin), r.text);
    }
}

[[nodiscard]] String ReadFile_(
    const FilePath& _path,
    bool&           _bOk)
{
    std::ifstream file { _path, std::ios::binary };
    _bOk = static_cast<bool>(file);
    return String { std::istreambuf_iterator<char>(file), std::istreambuf_iterator<char>() };
}

[[nodiscard]] ImU32 KindColor_(
    const String& _kind)
{
    if (_kind == "enemy")
    {
        return IM_COL32(235, 70, 70, 255);
    }
    if (_kind == "npc")
    {
        return IM_COL32(90, 225, 90, 255);
    }
    if (_kind == "prop")
    {
        return IM_COL32(245, 160, 40, 255);
    }
    if (_kind == "item")
    {
        return IM_COL32(245, 235, 70, 255);
    }
    if (_kind == "decor")
    {
        return IM_COL32(150, 170, 215, 255);
    }
    if (_kind == "collider")
    {
        return IM_COL32(70, 225, 225, 255);
    }
    if (_kind == "player")
    {
        return IM_COL32(255, 255, 255, 255);
    }
    return IM_COL32(235, 90, 235, 255);
}

[[nodiscard]] const Vector<String>& Kinds_()
{
    static const Vector<String> kinds = { "enemy", "npc", "prop", "item", "decor", "collider" };
    return kinds;
}

bool ComboStr_(
    const char*           _label,
    int&                  _index,
    const Vector<String>& _items)
{
    bool        bChanged = false;
    const char* preview  = (_index >= 0 && _index < static_cast<int>(_items.size())) ? _items[static_cast<size_t>(_index)].c_str() : "-";
    if (ImGui::BeginCombo(_label, preview))
    {
        for (int i = 0; i < static_cast<int>(_items.size()); ++i)
        {
            if (ImGui::Selectable(_items[static_cast<size_t>(i)].c_str(), i == _index))
            {
                _index   = i;
                bChanged = true;
            }
        }
        ImGui::EndCombo();
    }
    return bChanged;
}

[[nodiscard]] double Snap_(
    const double _value,
    const double _step)
{
    return std::round(_value / _step) * _step;
}

[[nodiscard]] int ReadInt_(
    const JsonReader& _r,
    const char*       _key,
    const int         _default)
{
    const Result<int> v = TryRead<int>(_r, _key);
    return v ? v.GetValue() : _default;
}

[[nodiscard]] bool ReadFrame_(
    const JsonReader& _r,
    StageSpriteFrame& _out)
{
    const Result<String> file = TryRead<String>(_r, "file");
    if (file)
    {
        _out.file   = file.GetValue();
        _out.w      = ReadInt_(_r, "cellW", ReadInt_(_r, "w", 0));
        _out.h      = ReadInt_(_r, "cellH", ReadInt_(_r, "h", 0));
        _out.x      = ReadInt_(_r, "col", 0) * _out.w;
        _out.y      = ReadInt_(_r, "line", 0) * _out.h;
        _out.pivotX = ReadInt_(_r, "pivotX", _out.w / 2);
        _out.pivotY = ReadInt_(_r, "pivotY", _out.h / 2);
        return _out.w > 0 && _out.h > 0;
    }
    const int atlas = ReadInt_(_r, "atlas", -1);
    if (atlas < 0)
    {
        return false;
    }
    _out.file   = std::format("Atlas/atlas_{:03}.png", atlas);
    _out.x      = ReadInt_(_r, "x", 0);
    _out.y      = ReadInt_(_r, "y", 0);
    _out.w      = ReadInt_(_r, "w", 0);
    _out.h      = ReadInt_(_r, "h", 0);
    _out.pivotX = ReadInt_(_r, "pivotX", 0);
    _out.pivotY = ReadInt_(_r, "pivotY", 0);
    return _out.w > 0 && _out.h > 0;
}

[[nodiscard]] String VisualName_(
    const JsonReader& _r)
{
    for (const char* key: { "sprite", "anim" })
    {
        const Result<String> v = TryRead<String>(_r, key);
        if (v)
        {
            return v.GetValue();
        }
    }
    return String {};
}

[[nodiscard]] String ClipName_(
    const JsonReader& _clips,
    const char*       _key)
{
    const Result<JsonReader> c = TryField(_clips, _key);
    if (c)
    {
        const Result<String> n = TryRead<String>(*c, "name");
        if (n)
        {
            return n.GetValue();
        }
    }
    return String {};
}

[[nodiscard]] String IdleClip_(
    const JsonReader& _defs,
    const JsonReader& _def,
    const int         _depth)
{
    const Result<JsonReader> clips = TryField(_def, "clips");
    if (clips && clips->IsObject())
    {
        for (const char* key: { "idle", "stand" })
        {
            const String n = ClipName_(clips.GetValue(), key);
            if (!n.empty())
            {
                return n;
            }
        }
    }
    const Result<String> parent = TryRead<String>(_def, "extends");
    if (parent && _depth < 8)
    {
        const Result<JsonReader> p = TryField(_defs, parent.GetValue());
        if (p)
        {
            return IdleClip_(_defs, p.GetValue(), _depth + 1);
        }
    }
    return String {};
}

[[nodiscard]] String FirstClip_(
    const JsonReader& _def)
{
    const Result<JsonReader> clips = TryField(_def, "clips");
    if (clips && clips->IsObject())
    {
        for (const std::pair<StringView, JsonReader>& c: clips->GetObject())
        {
            const Result<String> n = TryRead<String>(c.second, "name");
            if (n)
            {
                return n.GetValue();
            }
        }
    }
    return String {};
}
}   // namespace

bool StageEntity::IsPlaceable() const
{
    return bHasX && (bHasY || bGroundSnap);
}

const String& StageDocument::GetText() const
{
    return m_text;
}

Span<const StageEntity> StageDocument::GetEntities() const
{
    return m_entities;
}

Span<const StageEditSection> StageDocument::GetSections() const
{
    return m_sections;
}

int StageDocument::GetWorldWidth() const
{
    return m_worldW;
}

int StageDocument::GetWorldHeight() const
{
    return m_worldH;
}

const String& StageDocument::GetBackground() const
{
    return m_background;
}

const String& StageDocument::GetGroundInitial() const
{
    return m_groundInitial;
}

Span<const String> StageDocument::GetGroundMaps() const
{
    return m_groundMaps;
}

bool StageDocument::IsDirty() const
{
    return m_text != m_savedText;
}

bool StageDocument::CanUndo() const
{
    return !m_undo.empty();
}

String StageDocument::FormatNumber(
    const double _value)
{
    ARRAY<char, 64>      buf = {};
    std::to_chars_result r   = {};
    if (_value == std::floor(_value) && std::fabs(_value) < 1e9)
    {
        r = std::to_chars(buf.data(), buf.data() + buf.size(), static_cast<long long>(_value));
    }
    else
    {
        r = std::to_chars(buf.data(), buf.data() + buf.size(), _value);
    }
    return String { buf.data(), r.ptr };
}

bool StageDocument::Reparse_()
{
    JsonScanner scanner { m_text };
    const int   root = scanner.Parse();
    if (root < 0 || scanner.nodes[static_cast<size_t>(root)].type != 'o')
    {
        return false;
    }
    const Vector<JsonNode>& n = scanner.nodes;

    Vector<StageEntity>      entities;
    Vector<StageEditSection> sections;
    Vector<String>           groundMaps;
    Extractor                ex { n, entities };

    int       worldW = 0;
    int       worldH = 0;
    const int world  = ex.Find(root, "world");
    const int size   = ex.Find(world, "size");
    if (size >= 0 && n[static_cast<size_t>(size)].kids.size() == 2)
    {
        double w = 0.0;
        double h = 0.0;
        if (ex.Num(n[static_cast<size_t>(size)].kids[0], w))
        {
            worldW = static_cast<int>(w);
        }
        if (ex.Num(n[static_cast<size_t>(size)].kids[1], h))
        {
            worldH = static_cast<int>(h);
        }
    }
    const String background = ex.Str(ex.Find(world, "background"));

    const int    ground        = ex.Find(root, "ground");
    const String groundInitial = ex.Str(ex.Find(ground, "initial"));
    const int    maps          = ex.Find(ground, "maps");
    if (maps >= 0)
    {
        for (const int m: n[static_cast<size_t>(maps)].kids)
        {
            groundMaps.push_back(ex.Str(m));
        }
    }

    const int player = ex.Find(root, "player");
    if (player >= 0 && n[static_cast<size_t>(player)].type == 'o')
    {
        ex.AddEntity(player, -1, "player", true);
    }

    const int sectionsNode = ex.Find(root, "sections");
    if (sectionsNode >= 0)
    {
        for (const int s: n[static_cast<size_t>(sectionsNode)].kids)
        {
            StageEditSection section;
            section.id         = ex.Str(ex.Find(s, "id"));
            const int triggerN = ex.Find(s, "triggerX");
            if (ex.Num(triggerN, section.triggerX))
            {
                section.triggerBegin = n[static_cast<size_t>(triggerN)].begin;
                section.triggerEnd   = n[static_cast<size_t>(triggerN)].end;
            }
            ex.pRects         = &section.rects;
            const int actions = ex.Find(s, "actions");
            if (actions >= 0)
            {
                const JsonNode& a  = n[static_cast<size_t>(actions)];
                section.arrayBegin = a.begin;
                if (!a.kids.empty())
                {
                    section.lastBegin = n[static_cast<size_t>(a.kids.back())].begin;
                    section.lastEnd   = n[static_cast<size_t>(a.kids.back())].end;
                }
                const int sectionIndex = static_cast<int>(sections.size());
                for (const int act: a.kids)
                {
                    section.actionTypes.push_back(ex.Str(ex.Find(act, "type")));
                    ex.Action(act, sectionIndex, section.id);
                }
            }
            sections.push_back(section);
        }
    }

    m_entities      = std::move(entities);
    m_sections      = std::move(sections);
    m_groundMaps    = std::move(groundMaps);
    m_background    = background;
    m_groundInitial = groundInitial;
    m_worldW        = worldW;
    m_worldH        = worldH;
    return true;
}

Result<void> StageDocument::LoadFromText(
    const StringView _json)
{
    if (!JsonDeserializer::Load(_json, kJsonBom))
    {
        return Error { eError::Parse };
    }
    const String old = m_text;
    m_text           = String { _json };
    if (!Reparse_())
    {
        m_text = old;
        return Error { eError::InvalidData };
    }
    m_savedText = m_text;
    m_undo.clear();
    return Error { kOK };
}

Result<void> StageDocument::Load(
    const FilePath& _path)
{
    bool         bOk  = false;
    const String text = ReadFile_(_path, bOk);
    if (!bOk)
    {
        return Error { eError::FileOpen };
    }
    return LoadFromText(text);
}

Result<void> StageDocument::Save(
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

int StageDocument::FindEntityAt_(
    const int _begin) const
{
    for (size_t i = 0; i < m_entities.size(); ++i)
    {
        if (m_entities[i].begin == _begin)
        {
            return static_cast<int>(i);
        }
    }
    return -1;
}

bool StageDocument::IsIdUsed_(
    const StringView _id) const
{
    const String quoted = "\"" + String { _id } + "\"";
    return m_text.find(quoted) != String::npos;
}

bool StageDocument::SetPosition(
    const int    _entity,
    const double _x,
    const double _y,
    const bool   _bUndo)
{
    if (_entity < 0 || _entity >= static_cast<int>(m_entities.size()))
    {
        return false;
    }
    const StageEntity&  e = m_entities[static_cast<size_t>(_entity)];
    Vector<Replacement> list;
    if (e.bHasX && _x != e.x)
    {
        list.push_back({ e.xBegin, e.xEnd, FormatNumber(_x) });
    }
    if (e.bHasY && _y != e.y)
    {
        list.push_back({ e.yBegin, e.yEnd, FormatNumber(_y) });
    }
    if (list.empty())
    {
        return true;
    }
    const String before = m_text;
    ApplyReplacements_(m_text, list);
    if (!Reparse_())
    {
        m_text = before;
        (void)Reparse_();
        return false;
    }
    if (_bUndo)
    {
        m_undo.push_back(before);
        if (m_undo.size() > kMaxUndo)
        {
            m_undo.erase(m_undo.begin());
        }
    }
    return true;
}

int StageDocument::Add(
    const StringView _def,
    const StringView _kind,
    const int        _section,
    const double     _x,
    const double     _y)
{
    if (_section < 0 || _section >= static_cast<int>(m_sections.size()) || m_sections[static_cast<size_t>(_section)].arrayBegin < 0)
    {
        return -1;
    }
    const StageEditSection& s    = m_sections[static_cast<size_t>(_section)];
    const String            text = "{\"type\": \"spawn\", \"kind\": \"" + String { _kind } + "\", \"def\": \"" + String { _def } + "\", \"x\": " + FormatNumber(_x) + ", \"y\": " + FormatNumber(_y) + "}";

    const String before = m_text;
    int          objPos = 0;
    if (s.lastBegin >= 0)
    {
        String ws = WsBefore_(m_text, s.lastBegin);
        if (ws.empty())
        {
            ws = " ";
        }
        m_text.insert(static_cast<size_t>(s.lastEnd), "," + ws + text);
        objPos = s.lastEnd + 1 + static_cast<int>(ws.size());
    }
    else
    {
        m_text.insert(static_cast<size_t>(s.arrayBegin) + 1, text);
        objPos = s.arrayBegin + 1;
    }
    if (!Reparse_())
    {
        m_text = before;
        (void)Reparse_();
        return -1;
    }
    m_undo.push_back(before);
    if (m_undo.size() > kMaxUndo)
    {
        m_undo.erase(m_undo.begin());
    }
    return FindEntityAt_(objPos);
}

int StageDocument::Duplicate(
    const int    _entity,
    const double _dx,
    const double _dy)
{
    if (_entity < 0 || _entity >= static_cast<int>(m_entities.size()) || !m_entities[static_cast<size_t>(_entity)].bCanCopy)
    {
        return -1;
    }
    const StageEntity e    = m_entities[static_cast<size_t>(_entity)];
    String            copy = m_text.substr(static_cast<size_t>(e.begin), static_cast<size_t>(e.end - e.begin));

    Vector<Replacement> list;
    if (e.bHasX && _dx != 0.0)
    {
        list.push_back({ e.xBegin - e.begin, e.xEnd - e.begin, FormatNumber(e.x + _dx) });
    }
    if (e.bHasY && _dy != 0.0)
    {
        list.push_back({ e.yBegin - e.begin, e.yEnd - e.begin, FormatNumber(e.y + _dy) });
    }
    if (e.idBegin >= 0)
    {
        String newId;
        for (int n = 2; n < 1000; ++n)
        {
            newId = e.id + "_" + std::to_string(n);
            if (!IsIdUsed_(newId))
            {
                break;
            }
        }
        list.push_back({ e.idBegin - e.begin, e.idEnd - e.begin, "\"" + newId + "\"" });
    }
    ApplyReplacements_(copy, list);

    String ws = WsBefore_(m_text, e.begin);
    if (ws.empty())
    {
        ws = " ";
    }
    const String before = m_text;
    m_text.insert(static_cast<size_t>(e.end), "," + ws + copy);
    const int newPos = e.end + 1 + static_cast<int>(ws.size());
    if (!Reparse_())
    {
        m_text = before;
        (void)Reparse_();
        return -1;
    }
    m_undo.push_back(before);
    if (m_undo.size() > kMaxUndo)
    {
        m_undo.erase(m_undo.begin());
    }
    return FindEntityAt_(newPos);
}

bool StageDocument::Delete(
    const int _entity)
{
    if (_entity < 0 || _entity >= static_cast<int>(m_entities.size()) || !m_entities[static_cast<size_t>(_entity)].bCanCopy)
    {
        return false;
    }
    const StageEntity& e      = m_entities[static_cast<size_t>(_entity)];
    const String       before = m_text;
    const size_t       size   = m_text.size();

    size_t after = static_cast<size_t>(e.end);
    while (after < size && IsWs_(m_text[after]))
    {
        ++after;
    }
    if (after < size && m_text[after] == ',')
    {
        size_t cut = after + 1;
        while (cut < size && IsWs_(m_text[cut]))
        {
            ++cut;
        }
        m_text.erase(static_cast<size_t>(e.begin), cut - static_cast<size_t>(e.begin));
    }
    else
    {
        size_t prev = static_cast<size_t>(e.begin);
        while (prev > 0 && IsWs_(m_text[prev - 1]))
        {
            --prev;
        }
        if (prev > 0 && m_text[prev - 1] == ',')
        {
            m_text.erase(prev - 1, static_cast<size_t>(e.end) - (prev - 1));
        }
        else
        {
            m_text.erase(static_cast<size_t>(e.begin), static_cast<size_t>(e.end - e.begin));
        }
    }
    if (!Reparse_())
    {
        m_text = before;
        (void)Reparse_();
        return false;
    }
    m_undo.push_back(before);
    if (m_undo.size() > kMaxUndo)
    {
        m_undo.erase(m_undo.begin());
    }
    return true;
}

bool StageDocument::Patch_(
    Vector<Replacement>& _list,
    const bool           _bUndo)
{
    if (_list.empty())
    {
        return true;
    }
    const String before = m_text;
    ApplyReplacements_(m_text, _list);
    if (!Reparse_())
    {
        m_text = before;
        (void)Reparse_();
        return false;
    }
    if (_bUndo)
    {
        m_undo.push_back(before);
        if (m_undo.size() > kMaxUndo)
        {
            m_undo.erase(m_undo.begin());
        }
    }
    return true;
}

bool StageDocument::SetZ(
    const int  _entity,
    const int  _z,
    const int  _defaultZ,
    const bool _bUndo)
{
    if (_entity < 0 || _entity >= static_cast<int>(m_entities.size()))
    {
        return false;
    }
    const StageEntity&  e = m_entities[static_cast<size_t>(_entity)];
    Vector<Replacement> list;
    if (e.bHasZ)
    {
        size_t p = static_cast<size_t>(e.zBegin);
        while (p > 0 && IsWs_(m_text[p - 1]))
        {
            --p;
        }
        bool bOk = p > 0 && m_text[p - 1] == ':';
        if (bOk)
        {
            --p;
            while (p > 0 && IsWs_(m_text[p - 1]))
            {
                --p;
            }
            bOk = p >= 3 && m_text.compare(p - 3, 3, "\"z\"") == 0;
        }
        if (bOk)
        {
            p -= 3;
            while (p > 0 && IsWs_(m_text[p - 1]))
            {
                --p;
            }
            bOk = p > 0 && m_text[p - 1] == ',';
        }
        if (_z == _defaultZ && bOk)
        {
            list.push_back({ static_cast<int>(p - 1), e.zEnd, String {} });
        }
        else if (_z != e.z)
        {
            list.push_back({ e.zBegin, e.zEnd, std::to_string(_z) });
        }
    }
    else if (_z != _defaultZ)
    {
        size_t p = static_cast<size_t>(e.end) - 1;
        while (p > 0 && IsWs_(m_text[p - 1]))
        {
            --p;
        }
        list.push_back({ static_cast<int>(p), static_cast<int>(p), ", \"z\": " + std::to_string(_z) });
    }
    return Patch_(list, _bUndo);
}

bool StageDocument::UnlockGround(
    const int    _entity,
    const double _y,
    const bool   _bUndo)
{
    if (_entity < 0 || _entity >= static_cast<int>(m_entities.size()))
    {
        return false;
    }
    const StageEntity& e = m_entities[static_cast<size_t>(_entity)];
    if (!e.bGroundSnap || e.snapBegin < 0)
    {
        return false;
    }
    Vector<Replacement> list;
    list.push_back({ e.snapBegin, e.snapEnd, "false" });
    if (!e.bHasY)
    {
        size_t p = static_cast<size_t>(e.end) - 1;
        while (p > 0 && IsWs_(m_text[p - 1]))
        {
            --p;
        }
        list.push_back({ static_cast<int>(p), static_cast<int>(p), ", \"y\": " + FormatNumber(_y) });
    }
    return Patch_(list, _bUndo);
}

bool StageDocument::MoveSection(
    const int    _section,
    const double _dx,
    const bool   _bTrigger,
    const bool   _bRects,
    const bool   _bSpawns,
    const bool   _bUndo)
{
    if (_section < 0 || _section >= static_cast<int>(m_sections.size()))
    {
        return false;
    }
    const StageEditSection& s = m_sections[static_cast<size_t>(_section)];
    Vector<Replacement>     list;
    if (_dx == 0.0)
    {
        return true;
    }
    if (_bTrigger && s.triggerBegin >= 0)
    {
        list.push_back({ s.triggerBegin, s.triggerEnd, FormatNumber(s.triggerX + _dx) });
    }
    if (_bRects)
    {
        for (const StageEditRect& r: s.rects)
        {
            for (const size_t i: { size_t { 0 }, size_t { 2 } })
            {
                list.push_back({ r.begin[i], r.end[i], FormatNumber(r.v[i] + _dx) });
            }
        }
    }
    if (_bSpawns)
    {
        for (const StageEntity& e: m_entities)
        {
            if (e.section == _section && e.bHasX)
            {
                list.push_back({ e.xBegin, e.xEnd, FormatNumber(e.x + _dx) });
            }
        }
    }
    return Patch_(list, _bUndo);
}

bool StageDocument::SetTriggerX(
    const int    _section,
    const double _x,
    const bool   _bUndo)
{
    if (_section < 0 || _section >= static_cast<int>(m_sections.size()))
    {
        return false;
    }
    const StageEditSection& s = m_sections[static_cast<size_t>(_section)];
    Vector<Replacement>     list;
    if (s.triggerBegin >= 0 && _x != s.triggerX)
    {
        list.push_back({ s.triggerBegin, s.triggerEnd, FormatNumber(_x) });
    }
    return Patch_(list, _bUndo);
}

bool StageDocument::SetRect(
    const int               _section,
    const int               _rect,
    const ARRAY<double, 4>& _v,
    const bool              _bUndo)
{
    if (_section < 0 || _section >= static_cast<int>(m_sections.size()) || _rect < 0 || _rect >= static_cast<int>(m_sections[static_cast<size_t>(_section)].rects.size()))
    {
        return false;
    }
    const StageEditRect& r = m_sections[static_cast<size_t>(_section)].rects[static_cast<size_t>(_rect)];
    Vector<Replacement>  list;
    for (size_t i = 0; i < 4; ++i)
    {
        if (_v[i] != r.v[i])
        {
            list.push_back({ r.begin[i], r.end[i], FormatNumber(_v[i]) });
        }
    }
    return Patch_(list, _bUndo);
}

bool StageDocument::SetGlide(
    const int    _section,
    const int    _rect,
    const double _glide,
    const bool   _bUndo)
{
    if (_section < 0 || _section >= static_cast<int>(m_sections.size()) || _rect < 0 || _rect >= static_cast<int>(m_sections[static_cast<size_t>(_section)].rects.size()))
    {
        return false;
    }
    const StageEditRect& r = m_sections[static_cast<size_t>(_section)].rects[static_cast<size_t>(_rect)];
    Vector<Replacement>  list;
    if (r.glideBegin >= 0 && _glide != r.glide)
    {
        list.push_back({ r.glideBegin, r.glideEnd, FormatNumber(_glide) });
    }
    return Patch_(list, _bUndo);
}

void StageDocument::Undo()
{
    if (m_undo.empty())
    {
        return;
    }
    m_text = std::move(m_undo.back());
    m_undo.pop_back();
    (void)Reparse_();
}

void StageEditor::Init(
    SDL_Renderer* _pRenderer)
{
    m_pRenderer         = _pRenderer;
    const FilePath  dir = FindAssetRoot() / "Stage";
    std::error_code ec;
    for (const std::filesystem::directory_entry& entry: std::filesystem::directory_iterator(dir, ec))
    {
        if (entry.path().extension() == ".json")
        {
            m_stages.push_back(entry.path().stem().string());
        }
    }
    std::sort(m_stages.begin(), m_stages.end());
    LoadDefs_();
    LoadFrames_();
    if (!m_stages.empty())
    {
        const bool bHasStage0 = std::find(m_stages.begin(), m_stages.end(), "Stage0") != m_stages.end();
        LoadStage_(bHasStage0 ? String { "Stage0" } : m_stages.front());
    }
    else
    {
        m_status = "no stages in " + dir.string();
    }
}

void StageEditor::Shutdown()
{
    ClearTextures_();
    for (std::pair<const String, Page>& p: m_pages)
    {
        SDL_DestroyTexture(p.second.pTex);
    }
    m_pages.clear();
    m_pRenderer = nullptr;
}

void StageEditor::SelectStage(
    const StringView _name)
{
    if (std::find(m_stages.begin(), m_stages.end(), _name) != m_stages.end() && _name != m_stage)
    {
        LoadStage_(String { _name });
    }
}

void StageEditor::ClearTextures_()
{
    for (Strip& s: m_bgStrips)
    {
        SDL_DestroyTexture(s.pTex);
    }
    for (Strip& s: m_groundStrips)
    {
        SDL_DestroyTexture(s.pTex);
    }
    m_bgStrips.clear();
    m_groundStrips.clear();
    m_solid.clear();
    m_bgW = m_bgH = m_groundW = m_groundH = 0;
}

void StageEditor::LoadDefs_()
{
    m_defs.clear();
    std::error_code ec;
    for (const std::filesystem::directory_entry& entry: std::filesystem::recursive_directory_iterator(FindAssetRoot() / "Defs", ec))
    {
        if (entry.path().extension() != ".json")
        {
            continue;
        }
        const Result<JsonDeserializer> doc = LoadJsonFile(entry.path(), kJsonBomComments);
        if (!doc)
        {
            continue;
        }
        const Result<JsonReader> defs = TryField(doc->GetReader(), "defs");
        if (!defs || !defs->IsObject())
        {
            continue;
        }
        const String fileKind = (entry.path().stem() == "Props") ? "prop" : "enemy";
        for (const std::pair<StringView, JsonReader>& def: defs->GetObject())
        {
            const JsonReader& v = def.second;
            if (!v.IsObject())
            {
                continue;
            }
            const Result<bool>   bAbstract = TryRead<bool>(v, "abstract");
            const Result<String> kind      = TryRead<String>(v, "kind");
            if ((bAbstract && bAbstract.GetValue()) || (kind && kind.GetValue() == "projectile"))
            {
                continue;
            }
            int defZ = kEnemyDepth;
            if (fileKind == "prop")
            {
                defZ = kPropDepthBack;
                if (const Result<int> depth = TryRead<int>(v, "depth"))
                {
                    defZ = depth.GetValue();
                }
            }
            else if (const Result<String> behavior = TryRead<String>(v, "behavior"); behavior && behavior.GetValue() == "bigshiee")
            {
                defZ = kBigShieeDepthHull;
            }
            m_defs.push_back({ String { def.first }, fileKind, defZ });
            String name;
            if (fileKind == "prop")
            {
                for (const char* state: { "idle", "damaged", "destroyed" })
                {
                    const Result<JsonReader> s = TryField(v, state);
                    if (s && name.empty())
                    {
                        name = VisualName_(s.GetValue());
                    }
                }
                const Result<JsonReader> variants = TryField(v, "variants");
                if (name.empty() && variants && variants->IsArray())
                {
                    const Result<JsonReader> idle = variants->GetSize() == 0 ? Result<JsonReader> { Error { eError::InvalidData } } : TryField(*variants->BeginArray(), "idle");
                    if (idle)
                    {
                        name = VisualName_(idle.GetValue());
                    }
                }
            }
            else
            {
                name = IdleClip_(defs.GetValue(), v, 0);
                if (name.empty())
                {
                    name = FirstClip_(v);
                }
            }
            if (!name.empty())
            {
                m_defSprite[fileKind + "/" + String { def.first }] = name;
            }
        }
        const Result<JsonReader> items = TryField(doc->GetReader(), "items");
        if (items && items->IsObject())
        {
            for (const std::pair<StringView, JsonReader>& it: items->GetObject())
            {
                const Result<JsonReader> visual = TryField(it.second, "visual");
                if (visual)
                {
                    m_defSprite["item/" + String { it.first }] = VisualName_(visual.GetValue());
                }
            }
        }
        const Result<JsonReader> groups = TryField(doc->GetReader(), "groups");
        if (groups && groups->IsObject())
        {
            for (const std::pair<StringView, JsonReader>& g: groups->GetObject())
            {
                if (!g.second.IsArray() || g.second.GetSize() == 0)
                {
                    continue;
                }
                const Result<String> first = (*g.second.BeginArray()).Read<String>();
                if (first && m_defSprite.contains("item/" + first.GetValue()))
                {
                    const String value                        = m_defSprite["item/" + first.GetValue()];
                    m_defSprite["item/" + String { g.first }] = value;
                }
            }
        }
    }
    std::sort(m_defs.begin(), m_defs.end(), [](const DefInfo& _a, const DefInfo& _b) {
        return _a.name < _b.name;
    });
}

void StageEditor::LoadFrames_()
{
    m_frames.clear();
    const Result<JsonDeserializer> anim = LoadJsonFile(FindAssetRoot() / "anim.json");
    if (anim)
    {
        const Result<JsonReader> all = TryField(anim->GetReader(), "animations");
        if (all && all->IsObject())
        {
            for (const std::pair<StringView, JsonReader>& a: all->GetObject())
            {
                const Result<JsonReader> frames = TryField(a.second, "frames");
                if (!frames)
                {
                    continue;
                }
                StageSpriteFrame f;
                if (frames->IsArray() && frames->GetSize() > 0 && ReadFrame_(*frames->BeginArray(), f))
                {
                    m_frames[String { a.first }] = f;
                }
            }
        }
    }
    const Result<JsonDeserializer> sprites = LoadJsonFile(FindAssetRoot() / "sprites.json");
    if (sprites)
    {
        const Result<JsonReader> all = TryField(sprites->GetReader(), "sprites");
        if (all && all->IsObject())
        {
            for (const std::pair<StringView, JsonReader>& s: all->GetObject())
            {
                StageSpriteFrame f;
                if (!m_frames.contains(String { s.first }) && ReadFrame_(s.second, f))
                {
                    m_frames[String { s.first }] = f;
                }
            }
        }
    }
}

String StageEditor::SpriteName_(
    const StringView _kind,
    const StringView _def) const
{
    String group;
    if (_kind == "enemy" || _kind == "npc")
    {
        group = "enemy";
    }
    else if (_kind == "prop" || _kind == "item")
    {
        group = String { _kind };
    }
    else
    {
        return String {};
    }
    const HashMap<String, String>::const_iterator it = m_defSprite.find(group + "/" + String { _def });
    return it != m_defSprite.end() ? it->second : String {};
}

const StageEditor::Page* StageEditor::GetPage_(
    const String& _file)
{
    const HashMap<String, Page>::const_iterator found = m_pages.find(_file);
    if (found != m_pages.end())
    {
        return found->second.pTex != nullptr ? &found->second : nullptr;
    }
    Page     page;
    int      ch     = 0;
    stbi_uc* pPixel = stbi_load((FindAssetRoot() / _file).string().c_str(), &page.w, &page.h, &ch, 4);
    if (pPixel != nullptr)
    {
        page.pTex = SDL_CreateTexture(m_pRenderer, SDL_PIXELFORMAT_RGBA32, SDL_TEXTUREACCESS_STATIC, page.w, page.h);
        if (page.pTex != nullptr)
        {
            SDL_UpdateTexture(page.pTex, nullptr, pPixel, page.w * 4);
            SDL_SetTextureBlendMode(page.pTex, SDL_BLENDMODE_BLEND);
            SDL_SetTextureScaleMode(page.pTex, SDL_SCALEMODE_NEAREST);
        }
        stbi_image_free(pPixel);
    }
    m_pages[_file] = page;
    return page.pTex != nullptr ? &m_pages[_file] : nullptr;
}

bool StageEditor::DrawSprite_(
    ImDrawList*        _pDraw,
    const StageEntity& _e,
    const float        _px,
    const float        _py,
    const bool         _bSelected)
{
    String name = SpriteName_(_e.kind, _e.def);
    if (name.empty())
    {
        return false;
    }
    bool         bMirror = false;
    const size_t pos     = name.find("{side}");
    if (pos != String::npos)
    {
        String same  = name;
        String other = name;
        same.replace(pos, 6, _e.bFaceLeft ? "Left" : "Right");
        other.replace(pos, 6, _e.bFaceLeft ? "Right" : "Left");
        if (m_frames.contains(same))
        {
            name = same;
        }
        else
        {
            name    = other;
            bMirror = true;
        }
    }
    else
    {
        bMirror = _e.bFaceLeft;
    }
    const HashMap<String, StageSpriteFrame>::const_iterator it = m_frames.find(name);
    if (it == m_frames.end())
    {
        return false;
    }
    const StageSpriteFrame& f     = it->second;
    const Page*             pPage = GetPage_(f.file);
    if (pPage == nullptr)
    {
        return false;
    }
    const float  left = bMirror ? static_cast<float>(f.w - f.pivotX) : static_cast<float>(f.pivotX);
    const ImVec2 p0   = ImVec2 { std::round(_px - left * m_zoom), std::round(_py - static_cast<float>(f.pivotY) * m_zoom) };
    const ImVec2 p1   = ImVec2 { p0.x + static_cast<float>(f.w) * m_zoom, p0.y + static_cast<float>(f.h) * m_zoom };
    float        u0   = static_cast<float>(f.x) / static_cast<float>(pPage->w);
    float        u1   = static_cast<float>(f.x + f.w) / static_cast<float>(pPage->w);
    if (bMirror)
    {
        std::swap(u0, u1);
    }
    const float v0 = static_cast<float>(f.y) / static_cast<float>(pPage->h);
    const float v1 = static_cast<float>(f.y + f.h) / static_cast<float>(pPage->h);
    _pDraw->AddImage(reinterpret_cast<ImTextureID>(pPage->pTex), p0, p1, ImVec2 { u0, v0 }, ImVec2 { u1, v1 });
    if (_bSelected)
    {
        _pDraw->AddRect(p0, p1, IM_COL32(255, 255, 0, 255), 0.f, 0, 2.f);
    }
    return true;
}

bool StageEditor::PickSection_(
    const float _mx,
    const float _my,
    const float _originX,
    const float _originY,
    int&        _section,
    int&        _rect,
    int&        _mask) const
{
    constexpr float              tol      = 5.f;
    Span<const StageEditSection> sections = m_doc.GetSections();
    for (int i = 0; i < static_cast<int>(sections.size()); ++i)
    {
        const StageEditSection& s = sections[static_cast<size_t>(i)];
        for (int r = 0; r < static_cast<int>(s.rects.size()); ++r)
        {
            const StageEditRect& rc   = s.rects[static_cast<size_t>(r)];
            const float          ax   = _originX + (static_cast<float>(rc.v[0]) - m_scrollX) * m_zoom;
            const float          ay   = _originY + (static_cast<float>(rc.v[1]) - m_scrollY) * m_zoom;
            const float          bx   = _originX + (static_cast<float>(rc.v[2]) - m_scrollX) * m_zoom;
            const float          by   = _originY + (static_cast<float>(rc.v[3]) - m_scrollY) * m_zoom;
            const bool           bInY = _my >= ay - tol && _my <= by + tol;
            const bool           bInX = _mx >= ax - tol && _mx <= bx + tol;
            int                  mask = 0;
            mask |= (bInY && std::fabs(_mx - ax) <= tol) ? 1 : 0;
            mask |= (bInX && std::fabs(_my - ay) <= tol) ? 2 : 0;
            mask |= (bInY && std::fabs(_mx - bx) <= tol) ? 4 : 0;
            mask |= (bInX && std::fabs(_my - by) <= tol) ? 8 : 0;
            if (mask != 0)
            {
                _section = i;
                _rect    = r;
                _mask    = mask;
                return true;
            }
        }
    }
    for (int i = 0; i < static_cast<int>(sections.size()); ++i)
    {
        const StageEditSection& s      = sections[static_cast<size_t>(i)];
        const float             px     = _originX + (static_cast<float>(s.triggerX) - m_scrollX) * m_zoom;
        const float             ty     = _originY + static_cast<float>(i % 3) * 12.f;
        const float             tw     = ImGui::CalcTextSize(s.id.c_str()).x;
        const bool              bLine  = std::fabs(_mx - px) <= tol;
        const bool              bLabel = _mx >= px && _mx <= px + tw + 6.f && _my >= ty && _my <= ty + 14.f;
        if (bLine || bLabel)
        {
            _section = i;
            _rect    = -1;
            _mask    = 0;
            return true;
        }
    }
    return false;
}

void StageEditor::LoadStrips_(
    const FilePath& _path,
    const bool      _bGroundMask,
    Vector<Strip>&  _out,
    int&            _w,
    int&            _h)
{
    int      w      = 0;
    int      h      = 0;
    int      ch     = 0;
    stbi_uc* pPixel = stbi_load(_path.string().c_str(), &w, &h, &ch, 4);
    if (pPixel == nullptr)
    {
        return;
    }
    _w = w;
    _h = h;
    if (_bGroundMask)
    {
        m_solid.assign(static_cast<size_t>(w) * static_cast<size_t>(h), 0);
        for (size_t i = 0; i < m_solid.size(); ++i)
        {
            m_solid[i] = (pPixel[i * 4] < 64 && pPixel[i * 4 + 1] < 64 && pPixel[i * 4 + 2] > 192) ? 1 : 0;
        }
    }
    Vector<uint8_t> tmp;
    for (int x = 0; x < w; x += kStripWidth)
    {
        const int    sw   = std::min(kStripWidth, w - x);
        SDL_Texture* pTex = SDL_CreateTexture(m_pRenderer, SDL_PIXELFORMAT_RGBA32, SDL_TEXTUREACCESS_STATIC, sw, h);
        if (pTex == nullptr)
        {
            continue;
        }
        if (_bGroundMask)
        {
            tmp.assign(static_cast<size_t>(sw) * static_cast<size_t>(h) * 4, 0);
            for (int row = 0; row < h; ++row)
            {
                for (int col = 0; col < sw; ++col)
                {
                    if (m_solid[static_cast<size_t>(row) * static_cast<size_t>(w) + static_cast<size_t>(x + col)] != 0)
                    {
                        uint8_t* p = &tmp[(static_cast<size_t>(row) * static_cast<size_t>(sw) + static_cast<size_t>(col)) * 4];
                        p[0]       = 0;
                        p[1]       = 90;
                        p[2]       = 255;
                        p[3]       = 255;
                    }
                }
            }
            SDL_UpdateTexture(pTex, nullptr, tmp.data(), sw * 4);
        }
        else
        {
            SDL_UpdateTexture(pTex, nullptr, pPixel + static_cast<size_t>(x) * 4, w * 4);
        }
        SDL_SetTextureBlendMode(pTex, SDL_BLENDMODE_BLEND);
        SDL_SetTextureScaleMode(pTex, SDL_SCALEMODE_NEAREST);
        _out.push_back({ pTex, x, sw });
    }
    stbi_image_free(pPixel);
}

void StageEditor::LoadGround_(
    const String& _name)
{
    for (Strip& s: m_groundStrips)
    {
        SDL_DestroyTexture(s.pTex);
    }
    m_groundStrips.clear();
    m_solid.clear();
    m_groundW = m_groundH = 0;
    m_ground              = _name;
    if (!_name.empty())
    {
        LoadStrips_(FindAssetRoot() / "Ground" / (_name + ".png"), true, m_groundStrips, m_groundW, m_groundH);
    }
}

void StageEditor::LoadStage_(
    const String& _name)
{
    const FilePath     path   = FindAssetRoot() / "Stage" / (_name + ".json");
    const Result<void> result = m_doc.Load(path);
    if (!result)
    {
        m_status = "load FAILED: " + path.string();
        return;
    }
    m_path     = path;
    m_stage    = _name;
    m_selected = -1;
    m_section  = 0;
    m_bPlace   = false;
    m_scrollX  = 0.f;
    m_scrollY  = 0.f;
    ClearTextures_();
    const Result<JsonDeserializer> sprites = LoadJsonFile(FindAssetRoot() / "sprites.json");
    if (sprites)
    {
        const Result<JsonReader> all = TryField(sprites->GetReader(), "sprites");
        if (all)
        {
            const Result<JsonReader> sprite = TryField(*all, m_doc.GetBackground());
            if (sprite)
            {
                const Result<String> file = TryRead<String>(*sprite, "file");
                if (file)
                {
                    LoadStrips_(FindAssetRoot() / file.GetValue(), false, m_bgStrips, m_bgW, m_bgH);
                }
            }
        }
    }
    LoadGround_(m_doc.GetGroundInitial());
    m_status = "loaded " + path.string();
}

void StageEditor::Save_()
{
    const Result<void> result = m_doc.Save(m_path);
    m_status                  = result ? "saved " + m_path.string() : "save FAILED: " + m_path.string();
}

int StageEditor::GroundY_(
    const int _x,
    const int _fromY) const
{
    if (_x < 0 || _x >= m_groundW)
    {
        return -1;
    }
    for (int y = std::max(0, _fromY); y < m_groundH; ++y)
    {
        if (m_solid[static_cast<size_t>(y) * static_cast<size_t>(m_groundW) + static_cast<size_t>(_x)] != 0)
        {
            return y;
        }
    }
    return -1;
}

int StageEditor::DefaultZ_(
    const StageEntity& _e) const
{
    const String fileKind = _e.kind == "prop" ? "prop" : "enemy";
    for (const DefInfo& d: m_defs)
    {
        if (d.name == _e.def && d.kind == fileKind)
        {
            return d.z;
        }
    }
    return fileKind == "prop" ? kPropDepthBack : kEnemyDepth;
}

int StageEditor::EffectiveZ_(
    const StageEntity& _e) const
{
    return _e.bHasZ ? _e.z : DefaultZ_(_e);
}

double StageEditor::MarkerY_(
    const StageEntity& _e) const
{
    if (_e.bHasY)
    {
        return _e.y;
    }
    const int y = GroundY_(static_cast<int>(_e.x), static_cast<int>(_e.fromY));
    return (y >= 0) ? static_cast<double>(y) : _e.fromY;
}

const char* StageEditor::DefaultKind_(
    const StringView _def) const
{
    for (const StageEntity& e: m_doc.GetEntities())
    {
        if (e.def == _def)
        {
            return e.kind.c_str();
        }
    }
    for (const DefInfo& d: m_defs)
    {
        if (d.name == _def)
        {
            return d.kind.c_str();
        }
    }
    return "enemy";
}

void StageEditor::Select_(
    const int  _entity,
    const bool _bCenter)
{
    m_selected = _entity;
    if (_entity >= 0)
    {
        const StageEntity& e = m_doc.GetEntities()[static_cast<size_t>(_entity)];
        if (e.section >= 0)
        {
            m_section = e.section;
        }
        m_bNeedCenter = _bCenter && e.IsPlaceable();
    }
}

void StageEditor::Draw()
{
    ImGui::TextUnformatted("Stage map editor: see the \"Stage Map\" window.");
    ImGui::TextWrapped("%s%s", m_doc.IsDirty() ? "[modified] " : "", m_status.c_str());

    const ImGuiIO&       io     = ImGui::GetIO();
    const ImGuiViewport* pHost  = ImGui::GetMainViewport();
    const bool           bSmall = io.DisplaySize.x < 900.f;
    ImGui::SetNextWindowPos(ImVec2 { pHost->Pos.x + (bSmall ? 0.f : 344.f), pHost->Pos.y + 40.f }, ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowSize(ImVec2 { bSmall ? io.DisplaySize.x : std::max(480.f, io.DisplaySize.x - 360.f), std::max(320.f, io.DisplaySize.y - 56.f) }, ImGuiCond_FirstUseEver);
    if (ImGui::Begin("Stage Map"))
    {
        if (ImGui::IsWindowFocused(ImGuiFocusedFlags_RootAndChildWindows) && !io.WantTextInput)
        {
            if (io.KeyCtrl && ImGui::IsKeyPressed(ImGuiKey_S, false))
            {
                Save_();
            }
            if (io.KeyCtrl && ImGui::IsKeyPressed(ImGuiKey_Z, false))
            {
                m_doc.Undo();
                m_selected = -1;
            }
            if (io.KeyCtrl && ImGui::IsKeyPressed(ImGuiKey_D, false) && m_selected >= 0)
            {
                const int n = m_doc.Duplicate(m_selected, 16.0, 0.0);
                if (n >= 0)
                {
                    Select_(n, false);
                }
            }
            if (ImGui::IsKeyPressed(ImGuiKey_Delete, false) && m_selected >= 0 && m_doc.Delete(m_selected))
            {
                m_selected = -1;
            }
            if (ImGui::IsKeyPressed(ImGuiKey_Escape, false))
            {
                m_bPlace = false;
            }
        }
        ImGui::BeginChild("Panel", ImVec2 { 300.f, 0.f }, ImGuiChildFlags_Borders);
        DrawPanel_();
        ImGui::EndChild();
        ImGui::SameLine();
        ImGui::BeginChild("View", ImVec2 { 0.f, 0.f }, ImGuiChildFlags_None, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);
        DrawView_();
        ImGui::EndChild();
    }
    ImGui::End();
}

void StageEditor::DrawPanel_()
{
    int stageIndex = 0;
    for (int i = 0; i < static_cast<int>(m_stages.size()); ++i)
    {
        if (m_stages[static_cast<size_t>(i)] == m_stage)
        {
            stageIndex = i;
        }
    }
    ImGui::BeginDisabled(m_doc.IsDirty());
    if (ComboStr_("Stage", stageIndex, m_stages))
    {
        LoadStage_(m_stages[static_cast<size_t>(stageIndex)]);
    }
    ImGui::EndDisabled();
    if (m_doc.IsDirty())
    {
        ImGui::TextColored(ImVec4 { 1.f, 0.8f, 0.3f, 1.f }, "modified: save or reload to switch");
    }
    if (ImGui::Button("Save (Ctrl+S)"))
    {
        Save_();
    }
    ImGui::SameLine();
    ImGui::BeginDisabled(!m_doc.CanUndo());
    if (ImGui::Button("Undo"))
    {
        m_doc.Undo();
        m_selected = -1;
    }
    ImGui::EndDisabled();
    ImGui::SameLine();
    if (ImGui::Button("Reload"))
    {
        LoadStage_(m_stage);
    }
    ImGui::TextWrapped("%s", m_status.c_str());
    ImGui::Separator();
    Vector<String> sectionNames;
    for (const StageEditSection& s: m_doc.GetSections())
    {
        sectionNames.push_back(s.id);
    }
    ComboStr_("Section", m_section, sectionNames);
    Vector<String> defNames;
    for (const DefInfo& d: m_defs)
    {
        defNames.push_back(d.name);
    }
    if (ComboStr_("Def", m_defIndex, defNames))
    {
        const String kind = DefaultKind_(defNames[static_cast<size_t>(m_defIndex)]);
        for (int i = 0; i < static_cast<int>(Kinds_().size()); ++i)
        {
            if (Kinds_()[static_cast<size_t>(i)] == kind)
            {
                m_kindIndex = i;
            }
        }
    }
    ComboStr_("Kind", m_kindIndex, Kinds_());
    ImGui::Checkbox("Place mode (click map, Esc ends)", &m_bPlace);
    ImGui::Separator();
    if (m_selected >= 0 && m_selected < static_cast<int>(m_doc.GetEntities().size()))
    {
        const StageEntity e = m_doc.GetEntities()[static_cast<size_t>(m_selected)];
        ImGui::Text("%s  (%s)", e.def.c_str(), e.kind.c_str());
        ImGui::TextDisabled("%s%s%s", e.where.c_str(), e.id.empty() ? "" : "  id=", e.id.c_str());
        double x     = e.x;
        double y     = e.y;
        bool   bEdit = false;
        ImGui::BeginDisabled(!e.bHasX);
        bEdit |= ImGui::InputDouble("X", &x, 1.0, 8.0, "%.2f");
        ImGui::EndDisabled();
        ImGui::BeginDisabled(!e.bHasY);
        bEdit |= ImGui::InputDouble("Y", &y, 1.0, 8.0, "%.2f");
        ImGui::EndDisabled();
        if (e.bGroundSnap)
        {
            ImGui::TextDisabled("groundSnap: y 가 지면을 따라감 (Y 잠김)");
            if (ImGui::Button("Y 잠금 해제 (지금 위치로 고정)"))
            {
                m_doc.UnlockGround(m_selected, MarkerY_(e));
            }
        }
        else if (!e.bHasX)
        {
            ImGui::TextDisabled("camera-relative spawn: no x/y");
        }
        if (bEdit)
        {
            m_doc.SetPosition(m_selected, x, y);
        }
        const bool bZKind = e.kind == "enemy" || e.kind == "npc" || e.kind == "prop";
        int        z      = EffectiveZ_(e);
        ImGui::BeginDisabled(!bZKind);
        if (ImGui::InputInt("Z", &z))
        {
            m_doc.SetZ(m_selected, z, DefaultZ_(e));
        }
        ImGui::EndDisabled();
        ImGui::TextDisabled("Z: draw depth (larger = in front); default %d", DefaultZ_(e));
        ImGui::BeginDisabled(!e.bCanCopy);
        if (ImGui::Button("Duplicate (Ctrl+D)"))
        {
            const int n = m_doc.Duplicate(m_selected, 16.0, 0.0);
            if (n >= 0)
            {
                Select_(n, false);
            }
        }
        ImGui::SameLine();
        if (ImGui::Button("Delete (Del)"))
        {
            if (!e.id.empty())
            {
                m_status = "deleted id=" + e.id + " (gates/commands may still reference it)";
            }
            if (m_doc.Delete(m_selected))
            {
                m_selected = -1;
            }
        }
        ImGui::EndDisabled();
    }
    else
    {
        ImGui::TextDisabled("nothing selected");
    }
    ImGui::Separator();

    if (ImGui::CollapsingHeader("섹션", ImGuiTreeNodeFlags_DefaultOpen))
    {
        Span<const StageEditSection> secs = m_doc.GetSections();
        ImGui::BeginChild("SectionList", ImVec2 { 0.f, 110.f }, ImGuiChildFlags_Borders);
        for (int i = 0; i < static_cast<int>(secs.size()); ++i)
        {
            const StageEditSection& s     = secs[static_cast<size_t>(i)];
            const String            label = std::format("{}  x={}  ({})##sec{}", s.id, StageDocument::FormatNumber(s.triggerX), s.actionTypes.size(), i);
            if (ImGui::Selectable(label.c_str(), i == m_section))
            {
                m_section = i;
                m_scrollX = static_cast<float>(s.triggerX) - 100.f / m_zoom;
            }
        }
        ImGui::EndChild();
        if (m_section >= 0 && m_section < static_cast<int>(secs.size()))
        {
            const StageEditSection s  = secs[static_cast<size_t>(m_section)];
            double                 tx = s.triggerX;
            if (s.triggerBegin >= 0 && ImGui::InputDouble("triggerX", &tx, 1.0, 8.0, "%.2f"))
            {
                m_doc.SetTriggerX(m_section, tx);
            }
            for (int r = 0; r < static_cast<int>(s.rects.size()); ++r)
            {
                const StageEditRect& rc = s.rects[static_cast<size_t>(r)];
                ARRAY<double, 4>     v  = rc.v;
                const String         id = std::format("{} L T R B##rect{}", rc.type, r);
                if (ImGui::InputScalarN(id.c_str(), ImGuiDataType_Double, v.data(), 4, nullptr, nullptr, "%.0f"))
                {
                    m_doc.SetRect(m_section, r, v);
                }
                if (rc.glideBegin >= 0)
                {
                    double       g  = rc.glide;
                    const String gi = std::format("glide (s)##glide{}", r);
                    if (ImGui::InputDouble(gi.c_str(), &g, 0.1, 1.0, "%.2f"))
                    {
                        m_doc.SetGlide(m_section, r, g);
                    }
                }
                if (rc.type == "lockCamera" && rc.v[2] > static_cast<double>(m_doc.GetWorldWidth()))
                {
                    ImGui::TextColored(ImVec4 { 1.f, 0.3f, 0.3f, 1.f }, "경고: 오른쪽 끝이 월드 폭(%d) 초과", m_doc.GetWorldWidth());
                }
            }
            String types;
            for (const String& type: s.actionTypes)
            {
                types += (types.empty() ? "" : ", ") + type;
            }
            ImGui::TextDisabled("actions: %s", types.c_str());
        }
    }
    ImGui::Separator();
    ImGui::Checkbox("Only new-spawn section", &m_bOnlySection);
    ImGui::BeginChild("List", ImVec2 { 0.f, 0.f }, ImGuiChildFlags_Borders);
    Span<const StageEntity> entities = m_doc.GetEntities();
    for (int i = 0; i < static_cast<int>(entities.size()); ++i)
    {
        const StageEntity& e = entities[static_cast<size_t>(i)];
        if (m_bOnlySection && e.section != m_section)
        {
            continue;
        }
        const String xText = e.bHasX ? StageDocument::FormatNumber(e.x) : String { "-" };
        const String yText = e.bHasY ? StageDocument::FormatNumber(e.y) : String { e.bGroundSnap ? "ground" : "-" };
        const String label = std::format("{}  ({}, {})##{}", e.def, xText, yText, i);
        ImGui::PushStyleColor(ImGuiCol_Text, KindColor_(e.kind));
        if (ImGui::Selectable(label.c_str(), i == m_selected))
        {
            Select_(i, true);
        }
        ImGui::PopStyleColor();
    }
    ImGui::EndChild();
}

void StageEditor::DrawView_()
{
    ImGui::Checkbox("BG", &m_bShowBg);
    ImGui::SameLine();
    ImGui::Checkbox("Ground", &m_bShowGround);
    ImGui::SameLine();
    ImGui::Checkbox("Grid", &m_bShowGrid);
    ImGui::SameLine();
    ImGui::Checkbox("Labels", &m_bShowLabels);
    ImGui::SameLine();
    ImGui::Checkbox("Sprites", &m_bShowSprites);
    ImGui::SameLine();
    int            groundIndex = 0;
    Vector<String> maps;
    for (const String& m: m_doc.GetGroundMaps())
    {
        maps.push_back(m);
    }
    for (int i = 0; i < static_cast<int>(maps.size()); ++i)
    {
        if (maps[static_cast<size_t>(i)] == m_ground)
        {
            groundIndex = i;
        }
    }
    ImGui::SetNextItemWidth(110.f);
    if (ComboStr_("##ground", groundIndex, maps))
    {
        LoadGround_(maps[static_cast<size_t>(groundIndex)]);
    }
    ImGui::SameLine();
    ImGui::SetNextItemWidth(90.f);
    ImGui::SliderFloat("zoom", &m_zoom, 0.05f, 4.f, "%.2f", ImGuiSliderFlags_Logarithmic);

    const ImVec2 canvasPos  = ImGui::GetCursorScreenPos();
    const ImVec2 canvasSize = ImGui::GetContentRegionAvail();
    if (canvasSize.x < 32.f || canvasSize.y < 32.f)
    {
        return;
    }
    ImGui::InvisibleButton("canvas", canvasSize, ImGuiButtonFlags_MouseButtonLeft | ImGuiButtonFlags_MouseButtonRight | ImGuiButtonFlags_MouseButtonMiddle);
    const bool     bHovered = ImGui::IsItemHovered();
    const ImGuiIO& io       = ImGui::GetIO();
    const float    originX  = canvasPos.x;
    const float    originY  = canvasPos.y + kRulerHeight;
    if (bHovered && io.MouseWheel != 0.f)
    {
        if (io.KeyShift)
        {
            m_scrollX -= io.MouseWheel * 200.f / m_zoom;
        }
        else
        {
            const float worldX = m_scrollX + (io.MousePos.x - originX) / m_zoom;
            const float worldY = m_scrollY + (io.MousePos.y - originY) / m_zoom;
            m_zoom             = std::clamp(m_zoom * (io.MouseWheel > 0.f ? 1.25f : 0.8f), 0.05f, 4.f);
            m_scrollX          = worldX - (io.MousePos.x - originX) / m_zoom;
            m_scrollY          = worldY - (io.MousePos.y - originY) / m_zoom;
        }
    }
    if (bHovered && (ImGui::IsMouseDragging(ImGuiMouseButton_Middle, 0.f) || ImGui::IsMouseDragging(ImGuiMouseButton_Right, 0.f)))
    {
        m_scrollX -= io.MouseDelta.x / m_zoom;
        m_scrollY -= io.MouseDelta.y / m_zoom;
    }
    const float viewW = canvasSize.x / m_zoom;
    const float viewH = (canvasSize.y - kRulerHeight) / m_zoom;
    if (m_bNeedCenter && m_selected >= 0)
    {
        const StageEntity& e = m_doc.GetEntities()[static_cast<size_t>(m_selected)];
        m_scrollX            = static_cast<float>(e.x) - viewW * 0.5f;
        m_scrollY            = static_cast<float>(MarkerY_(e)) - viewH * 0.5f;
        m_bNeedCenter        = false;
    }
    const float worldW = static_cast<float>(m_doc.GetWorldWidth());
    const float worldH = static_cast<float>(m_doc.GetWorldHeight());
    m_scrollX          = std::clamp(m_scrollX, -64.f / m_zoom, std::max(0.f, worldW - viewW) + 64.f / m_zoom);
    m_scrollY          = std::clamp(m_scrollY, -64.f / m_zoom, std::max(0.f, worldH - viewH) + 64.f / m_zoom);

    const float zoom = m_zoom;
    const float sx   = m_scrollX;
    const float sy   = m_scrollY;

    ImDrawList* pDraw = ImGui::GetWindowDrawList();
    pDraw->PushClipRect(canvasPos, ImVec2 { canvasPos.x + canvasSize.x, canvasPos.y + canvasSize.y }, true);
    pDraw->AddRectFilled(canvasPos, ImVec2 { canvasPos.x + canvasSize.x, canvasPos.y + canvasSize.y }, IM_COL32(24, 26, 32, 255));
    const ImVec2 w0 { std::round(originX + (0.f - sx) * zoom), std::round(originY + (0.f - sy) * zoom) };
    const ImVec2 w1 { std::round(originX + (worldW - sx) * zoom), std::round(originY + (worldH - sy) * zoom) };
    pDraw->AddRectFilled(w0, w1, IM_COL32(48, 52, 62, 255));

    if (m_bShowBg)
    {
        for (const Strip& s: m_bgStrips)
        {
            const ImVec2 a { std::round(originX + (static_cast<float>(s.x) - sx) * zoom), w0.y };
            const ImVec2 b { std::round(originX + (static_cast<float>(s.x + s.w) - sx) * zoom), std::round(originY + (static_cast<float>(m_bgH) - sy) * zoom) };
            pDraw->AddImage(reinterpret_cast<ImTextureID>(s.pTex), a, b);
        }
    }
    if (m_bShowGround)
    {
        for (const Strip& s: m_groundStrips)
        {
            const ImVec2 a { std::round(originX + (static_cast<float>(s.x) - sx) * zoom), w0.y };
            const ImVec2 b { std::round(originX + (static_cast<float>(s.x + s.w) - sx) * zoom), std::round(originY + (static_cast<float>(m_groundH) - sy) * zoom) };
            pDraw->AddImage(reinterpret_cast<ImTextureID>(s.pTex), a, b, ImVec2 { 0.f, 0.f }, ImVec2 { 1.f, 1.f }, IM_COL32(255, 255, 255, 130));
        }
    }
    pDraw->AddRect(w0, w1, IM_COL32(200, 200, 90, 255));
    float step = 10.f;
    for (const float candidate: { 10.f, 20.f, 50.f, 100.f, 200.f, 500.f, 1000.f, 2000.f, 5000.f })
    {
        step = candidate;
        if (candidate * zoom >= 60.f)
        {
            break;
        }
    }
    pDraw->AddRectFilled(canvasPos, ImVec2 { canvasPos.x + canvasSize.x, canvasPos.y + kRulerHeight }, IM_COL32(16, 16, 20, 255));
    const float firstX = std::floor(sx / step) * step;
    for (float wx = firstX; (wx - sx) * zoom < canvasSize.x; wx += step)
    {
        const float px = std::round(originX + (wx - sx) * zoom);
        if (m_bShowGrid)
        {
            pDraw->AddLine(ImVec2 { px, originY }, ImVec2 { px, canvasPos.y + canvasSize.y }, IM_COL32(255, 255, 255, 28));
        }
        pDraw->AddLine(ImVec2 { px, canvasPos.y + kRulerHeight - 6.f }, ImVec2 { px, canvasPos.y + kRulerHeight }, IM_COL32(200, 200, 200, 255));
        const String text = std::to_string(static_cast<int>(wx));
        pDraw->AddText(ImVec2 { px + 2.f, canvasPos.y + 2.f }, IM_COL32(210, 210, 210, 255), text.c_str());
    }
    if (m_bShowGrid)
    {
        const float firstY = std::floor(sy / step) * step;
        for (float wy = firstY; (wy - sy) * zoom < canvasSize.y - kRulerHeight; wy += step)
        {
            const float py = std::round(originY + (wy - sy) * zoom);
            pDraw->AddLine(ImVec2 { canvasPos.x, py }, ImVec2 { canvasPos.x + canvasSize.x, py }, IM_COL32(255, 255, 255, 28));
            const String text = std::to_string(static_cast<int>(wy));
            pDraw->AddText(ImVec2 { canvasPos.x + 2.f, py + 1.f }, IM_COL32(170, 170, 170, 200), text.c_str());
        }
    }
    Span<const StageEditSection> sections = m_doc.GetSections();
    for (int i = 0; i < static_cast<int>(sections.size()); ++i)
    {
        const StageEditSection& sec  = sections[static_cast<size_t>(i)];
        const bool              bSel = (i == m_section);
        for (const StageEditRect& rc: sec.rects)
        {
            const ImVec2 a { std::round(originX + (static_cast<float>(rc.v[0]) - sx) * zoom), std::round(originY + (static_cast<float>(rc.v[1]) - sy) * zoom) };
            const ImVec2 b { std::round(originX + (static_cast<float>(rc.v[2]) - sx) * zoom), std::round(originY + (static_cast<float>(rc.v[3]) - sy) * zoom) };
            const bool   bLock = (rc.type == "lockCamera");
            const ImU32  rcol  = bLock ? IM_COL32(255, 150, 30, bSel ? 255 : 150) : IM_COL32(60, 200, 255, bSel ? 220 : 110);
            if (bLock)
            {
                pDraw->AddRectFilled(a, b, IM_COL32(255, 150, 30, bSel ? 70 : 30));
            }
            pDraw->AddRect(a, b, rcol, 0.f, 0, bSel ? 2.f : 1.f);
            String label = sec.id + (bLock ? " lock" : " bounds");
            if (rc.glideBegin >= 0)
            {
                label += std::format(" glide {}s", StageDocument::FormatNumber(rc.glide));
            }
            pDraw->AddText(ImVec2 { a.x + 3.f, a.y + 3.f }, rcol, label.c_str());
            if (bLock && rc.v[2] > static_cast<double>(worldW))
            {
                pDraw->AddText(ImVec2 { b.x + 4.f, a.y + 18.f }, IM_COL32(255, 70, 70, 255), std::format("경고: 오른쪽 끝 > 월드 폭 {}", m_doc.GetWorldWidth()).c_str());
            }
        }
    }
    for (int i = 0; i < static_cast<int>(sections.size()); ++i)
    {
        const float px  = std::round(originX + (static_cast<float>(sections[static_cast<size_t>(i)].triggerX) - sx) * zoom);
        const ImU32 col = (i == m_section) ? IM_COL32(255, 200, 60, 230) : IM_COL32(120, 160, 255, 140);
        pDraw->AddLine(ImVec2 { px, originY }, ImVec2 { px, canvasPos.y + canvasSize.y }, col);
        pDraw->AddText(ImVec2 { px + 3.f, originY + 2.f + static_cast<float>(i % 3) * 12.f }, col, sections[static_cast<size_t>(i)].id.c_str());
    }
    const float             mouseWorldX = sx + (io.MousePos.x - originX) / zoom;
    const float             mouseWorldY = sy + (io.MousePos.y - originY) / zoom;
    Span<const StageEntity> entities    = m_doc.GetEntities();
    int                     hit         = -1;
    float                   hitDist     = 11.f;
    Vector<int>             order;
    for (int i = 0; i < static_cast<int>(entities.size()); ++i)
    {
        order.push_back(i);
    }
    std::stable_sort(order.begin(), order.end(), [&](const int _a, const int _b) {
        const StageEntity& a  = entities[static_cast<size_t>(_a)];
        const StageEntity& b  = entities[static_cast<size_t>(_b)];
        const int          za = EffectiveZ_(a);
        const int          zb = EffectiveZ_(b);
        return za != zb ? za < zb : MarkerY_(a) < MarkerY_(b);
    });
    for (const int i: order)
    {
        const StageEntity& e = entities[static_cast<size_t>(i)];
        if (!e.IsPlaceable() || (m_bOnlySection && e.section != m_section))
        {
            continue;
        }
        const float px = originX + (static_cast<float>(e.x) - sx) * zoom;
        const float py = originY + (static_cast<float>(MarkerY_(e)) - sy) * zoom;
        if (px < canvasPos.x - 300.f || px > canvasPos.x + canvasSize.x + 300.f || py < originY - 300.f || py > canvasPos.y + canvasSize.y + 300.f)
        {
            continue;
        }
        const bool bSprite = m_bShowSprites && DrawSprite_(pDraw, e, px, py, i == m_selected);
        if (!bSprite)
        {
            pDraw->AddCircleFilled(ImVec2 { px, py }, 4.f, KindColor_(e.kind));
            pDraw->AddCircle(ImVec2 { px, py }, 4.f, IM_COL32(0, 0, 0, 255));
            if (i == m_selected)
            {
                pDraw->AddCircle(ImVec2 { px, py }, 8.f, IM_COL32(255, 255, 0, 255), 0, 2.f);
            }
        }
        if ((m_bShowLabels && (zoom >= 0.3f || !bSprite)) || i == m_selected)
        {
            pDraw->AddText(ImVec2 { px + 7.f, py - 14.f }, IM_COL32(255, 255, 255, 230), e.def.c_str());
        }
        const float dist = std::hypot(io.MousePos.x - px, io.MousePos.y - py);
        if (dist < hitDist)
        {
            hitDist = dist;
            hit     = i;
        }
    }
    pDraw->PopClipRect();
    const double step1    = io.KeyShift ? 8.0 : 1.0;
    const bool   bInRuler = io.MousePos.y < originY;
    if (bHovered && !bInRuler && ImGui::IsMouseClicked(ImGuiMouseButton_Left))
    {
        if (m_bPlace)
        {
            if (m_defIndex >= 0 && m_defIndex < static_cast<int>(m_defs.size()))
            {
                const double x = Snap_(static_cast<double>(mouseWorldX), step1);
                const double y = Snap_(static_cast<double>(mouseWorldY), step1);
                const int    n = m_doc.Add(m_defs[static_cast<size_t>(m_defIndex)].name, Kinds_()[static_cast<size_t>(m_kindIndex)], m_section, x, y);
                if (n >= 0)
                {
                    Select_(n, false);
                }
                else
                {
                    m_status = "add FAILED";
                }
            }
        }
        else if (hit >= 0)
        {
            Select_(hit, false);
            const StageEntity& e = entities[static_cast<size_t>(hit)];
            m_grabDx             = e.x - static_cast<double>(mouseWorldX);
            m_grabDy             = MarkerY_(e) - static_cast<double>(mouseWorldY);
            m_bDragging          = true;
            m_bDragUndone        = false;
        }
        else
        {
            int sec  = -1;
            int rect = -1;
            int mask = 0;
            if (PickSection_(io.MousePos.x, io.MousePos.y, originX, originY, sec, rect, mask))
            {
                m_section                 = sec;
                m_secDrag                 = sec;
                m_rectDrag                = rect;
                m_edgeMask                = mask;
                m_secMode                 = io.KeyAlt ? 2 : (io.KeyShift ? 1 : 0);
                m_bSecUndone              = false;
                const StageEditSection& s = sections[static_cast<size_t>(sec)];
                if (rect < 0)
                {
                    m_secPos   = s.triggerX;
                    m_secGrabX = s.triggerX - static_cast<double>(mouseWorldX);
                }
                else
                {
                    const StageEditRect& rc = s.rects[static_cast<size_t>(rect)];
                    m_secGrabX              = ((mask & 1) != 0 ? rc.v[0] : rc.v[2]) - static_cast<double>(mouseWorldX);
                    m_secGrabY              = ((mask & 2) != 0 ? rc.v[1] : rc.v[3]) - static_cast<double>(mouseWorldY);
                }
            }
            else
            {
                m_selected = -1;
            }
        }
    }
    if (m_secDrag >= 0)
    {
        if (ImGui::IsMouseDown(ImGuiMouseButton_Left) && m_secDrag < static_cast<int>(sections.size()))
        {
            const double snap = io.KeyCtrl ? 8.0 : 1.0;
            if (m_rectDrag < 0)
            {
                const double pos = Snap_(static_cast<double>(mouseWorldX) + m_secGrabX, snap);
                const double dx  = pos - m_secPos;
                if (dx != 0.0 && m_doc.MoveSection(m_secDrag, dx, m_secMode != 2, m_secMode != 2, m_secMode != 1, !m_bSecUndone))
                {
                    m_secPos     = pos;
                    m_bSecUndone = true;
                }
            }
            else if (m_rectDrag < static_cast<int>(sections[static_cast<size_t>(m_secDrag)].rects.size()))
            {
                const ARRAY<double, 4> old = sections[static_cast<size_t>(m_secDrag)].rects[static_cast<size_t>(m_rectDrag)].v;
                ARRAY<double, 4>       v   = old;
                const double           wx  = Snap_(static_cast<double>(mouseWorldX) + m_secGrabX, snap);
                const double           wy  = Snap_(static_cast<double>(mouseWorldY) + m_secGrabY, snap);
                v[0]                       = (m_edgeMask & 1) != 0 ? wx : v[0];
                v[1]                       = (m_edgeMask & 2) != 0 ? wy : v[1];
                v[2]                       = (m_edgeMask & 4) != 0 ? wx : v[2];
                v[3]                       = (m_edgeMask & 8) != 0 ? wy : v[3];
                if (v != old && m_doc.SetRect(m_secDrag, m_rectDrag, v, !m_bSecUndone))
                {
                    m_bSecUndone = true;
                }
            }
        }
        else
        {
            m_secDrag = -1;
        }
    }
    if (m_bDragging)
    {
        if (ImGui::IsMouseDown(ImGuiMouseButton_Left) && m_selected >= 0)
        {
            const StageEntity& e = m_doc.GetEntities()[static_cast<size_t>(m_selected)];
            const double       x = Snap_(static_cast<double>(mouseWorldX) + m_grabDx, step1);
            const double       y = e.bHasY ? Snap_(static_cast<double>(mouseWorldY) + m_grabDy, step1) : e.y;
            if ((x != e.x || y != e.y) && m_doc.SetPosition(m_selected, x, y, !m_bDragUndone))
            {
                m_bDragUndone = true;
            }
        }
        else
        {
            m_bDragging = false;
        }
    }
    const String info = std::format("x {}  y {}   zoom {:.2f}   {}", static_cast<int>(mouseWorldX), static_cast<int>(mouseWorldY), zoom, m_bPlace ? "PLACE: click to add" : "wheel zoom, Shift+wheel / mid-drag scroll");
    ImGui::GetWindowDrawList()->AddText(ImVec2 { canvasPos.x + 6.f, canvasPos.y + canvasSize.y - 16.f }, IM_COL32(255, 255, 255, 220), info.c_str());
}
