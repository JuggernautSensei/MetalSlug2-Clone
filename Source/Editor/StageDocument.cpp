#include "Editor/StageDocument.h"

#include <algorithm>
#include <charconv>
#include <cmath>
#include <format>
#include "Editor/JsonScan.h"
#include "Base/JsonFields.h"
#include "Core/AssetPath.h"
#include "Game/Stage/StageData.h"

constexpr size_t kMaxUndo = 100;

class StageDocumentReader
{
public:
    StageDocumentReader(const Vector<JsonNode>& _nodes, Vector<StageEntity>& _entities)
        : m_nodes(_nodes)
        , m_entities(_entities)
    {}
    void SetRectOutput(Vector<StageEditRect>& _rects) { m_pRects = &_rects; }

    [[nodiscard]] int Find(const int _obj, const StringView _key) const
    {
        if (_obj < 0 || m_nodes[static_cast<size_t>(_obj)].type != 'o')
        {
            return -1;
        }
        const JsonNode& o = m_nodes[static_cast<size_t>(_obj)];
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
        if (_node >= 0 && m_nodes[static_cast<size_t>(_node)].type == 's')
        {
            return m_nodes[static_cast<size_t>(_node)].str;
        }
        return String {};
    }

    [[nodiscard]] bool Num(const int _node, double& _out) const
    {
        if (_node < 0 || m_nodes[static_cast<size_t>(_node)].type != 'n')
        {
            return false;
        }
        const String& s = m_nodes[static_cast<size_t>(_node)].str;
        return std::from_chars(s.data(), s.data() + s.size(), _out).ec == std::errc {};
    }

    void AddRect(const int _obj, const String& _type)
    {
        const int r = Find(_obj, "rect");
        if (r < 0 || m_nodes[static_cast<size_t>(r)].type != 'a' || m_nodes[static_cast<size_t>(r)].kids.size() != 4)
        {
            return;
        }
        StageEditRect rect;
        rect.type = _type;
        for (size_t i = 0; i < 4; ++i)
        {
            const int k = m_nodes[static_cast<size_t>(r)].kids[i];
            if (!Num(k, rect.v[i]))
            {
                return;
            }
            rect.begin[i] = m_nodes[static_cast<size_t>(k)].begin;
            rect.end[i]   = m_nodes[static_cast<size_t>(k)].end;
        }
        const int g = Find(_obj, "glide");
        if (Num(g, rect.glide))
        {
            rect.glideBegin = m_nodes[static_cast<size_t>(g)].begin;
            rect.glideEnd   = m_nodes[static_cast<size_t>(g)].end;
        }
        m_pRects->push_back(rect);
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
        const JsonNode& o = m_nodes[static_cast<size_t>(_obj)];
        e.section         = _section;
        e.where           = _where;
        e.begin           = o.begin;
        e.end             = o.end;
        e.bCanCopy        = !_bPlayer;
        e.bFaceLeft       = (Str(Find(_obj, "facing")) == "left");
        const int idN     = Find(_obj, "id");
        e.id              = Str(idN);
        if (idN >= 0 && m_nodes[static_cast<size_t>(idN)].type == 's')
        {
            e.idBegin = m_nodes[static_cast<size_t>(idN)].begin;
            e.idEnd   = m_nodes[static_cast<size_t>(idN)].end;
        }
        const int xN = Find(_obj, "x");
        const int yN = Find(_obj, "y");
        e.bHasX      = Num(xN, e.x);
        e.bHasY      = Num(yN, e.y);
        if (e.bHasX)
        {
            e.xBegin = m_nodes[static_cast<size_t>(xN)].begin;
            e.xEnd   = m_nodes[static_cast<size_t>(xN)].end;
        }
        if (e.bHasY)
        {
            e.yBegin = m_nodes[static_cast<size_t>(yN)].begin;
            e.yEnd   = m_nodes[static_cast<size_t>(yN)].end;
        }
        const int zN = Find(_obj, "z");
        double    zv = 0.0;
        if (Num(zN, zv))
        {
            e.bHasZ  = true;
            e.z      = static_cast<int>(zv);
            e.zBegin = m_nodes[static_cast<size_t>(zN)].begin;
            e.zEnd   = m_nodes[static_cast<size_t>(zN)].end;
        }
        const int snapN = Find(_obj, "groundSnap");
        e.bGroundSnap   = (snapN >= 0 && m_nodes[static_cast<size_t>(snapN)].str == "true");
        if (e.bGroundSnap)
        {
            e.snapBegin = m_nodes[static_cast<size_t>(snapN)].begin;
            e.snapEnd   = m_nodes[static_cast<size_t>(snapN)].end;
        }
        double fromY = e.fromY;
        if (Num(Find(_obj, "fromY"), fromY))
        {
            e.fromY = fromY;
        }
        m_entities.push_back(e);
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
                for (const int entry: m_nodes[static_cast<size_t>(entries)].kids)
                {
                    AddEntity(entry, _section, _where + " (wave)", false);
                }
            }
        }
        else if ((type == "lockCamera" || type == "setBounds") && m_pRects != nullptr)
        {
            AddRect(_obj, type);
        }
        else if (type == "gate")
        {
            const int    actions = Find(_obj, "actions");
            const String where   = _where + " > " + Str(Find(_obj, "id"));
            if (actions >= 0)
            {
                for (const int a: m_nodes[static_cast<size_t>(actions)].kids)
                {
                    Action(a, _section, where);
                }
            }
        }
    }

private:
    const Vector<JsonNode>& m_nodes;
    Vector<StageEntity>&    m_entities;
    Vector<StageEditRect>*  m_pRects = nullptr;
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
    StageDocumentReader      ex { n, entities };

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
            ex.SetRectOutput(section.rects);
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
    const Result<String> text = ReadFileText(_path);
    if (!text)
    {
        return Error { eError::FileOpen };
    }
    return LoadFromText(text.GetValue());
}

Result<void> StageDocument::Save(
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
        list.push_back({ e.xBegin, e.xEnd, FormatJsonNumber(_x) });
    }
    if (e.bHasY && _y != e.y)
    {
        list.push_back({ e.yBegin, e.yEnd, FormatJsonNumber(_y) });
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
    const String            text = "{\"type\": \"spawn\", \"kind\": \"" + String { _kind } + "\", \"def\": \"" + String { _def } + "\", \"x\": " + FormatJsonNumber(_x) + ", \"y\": " + FormatJsonNumber(_y) + "}";

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
        list.push_back({ e.xBegin - e.begin, e.xEnd - e.begin, FormatJsonNumber(e.x + _dx) });
    }
    if (e.bHasY && _dy != 0.0)
    {
        list.push_back({ e.yBegin - e.begin, e.yEnd - e.begin, FormatJsonNumber(e.y + _dy) });
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
        list.push_back({ static_cast<int>(p), static_cast<int>(p), ", \"y\": " + FormatJsonNumber(_y) });
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
        list.push_back({ s.triggerBegin, s.triggerEnd, FormatJsonNumber(s.triggerX + _dx) });
    }
    if (_bRects)
    {
        for (const StageEditRect& r: s.rects)
        {
            for (const size_t i: { size_t { 0 }, size_t { 2 } })
            {
                list.push_back({ r.begin[i], r.end[i], FormatJsonNumber(r.v[i] + _dx) });
            }
        }
    }
    if (_bSpawns)
    {
        for (const StageEntity& e: m_entities)
        {
            if (e.section == _section && e.bHasX)
            {
                list.push_back({ e.xBegin, e.xEnd, FormatJsonNumber(e.x + _dx) });
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
        list.push_back({ s.triggerBegin, s.triggerEnd, FormatJsonNumber(_x) });
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
            list.push_back({ r.begin[i], r.end[i], FormatJsonNumber(_v[i]) });
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
        list.push_back({ r.glideBegin, r.glideEnd, FormatJsonNumber(_glide) });
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
