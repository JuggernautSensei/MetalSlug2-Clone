#include "Game/Stage/StageData.h"

#include <cmath>
#include <format>
#include <fstream>
#include <optional>
#include <sstream>
#include <utility>

#include "Base/Assert.h"
#include "Base/Error.h"
#include "Base/Json.h"
#include "Base/Math.h"
#include "Base/Types.h"

ParamValue ParamValue::MakeBool(
    const bool _bValue)
{
    ParamValue value = {};
    value.m_type     = eType::Bool;
    value.m_bValue   = _bValue;
    return value;
}

ParamValue ParamValue::MakeNumber(
    const double _value)
{
    ParamValue value = {};
    value.m_type     = eType::Number;
    value.m_number   = _value;
    return value;
}

ParamValue ParamValue::MakeString(
    String _value)
{
    ParamValue value = {};
    value.m_type     = eType::String;
    value.m_string   = std::move(_value);
    return value;
}

ParamValue ParamValue::MakeArray()
{
    ParamValue value = {};
    value.m_type     = eType::Array;
    return value;
}

ParamValue ParamValue::MakeObject()
{
    ParamValue value = {};
    value.m_type     = eType::Object;
    return value;
}

void ParamValue::PushBack(
    ParamValue _value)
{
    SLUG_ASSERT(
        m_type == eType::Array,
        "PushBack requires an array value");
    m_items.push_back(std::move(_value));
}

void ParamValue::Set(
    String     _key,
    ParamValue _value)
{
    SLUG_ASSERT(
        m_type == eType::Object,
        "Set requires an object value");
    for (size_t i = 0; i < m_keys.size(); ++i)
    {
        if (m_keys[i] == _key)
        {
            m_items[i] = std::move(_value);
            return;
        }
    }
    m_keys.push_back(std::move(_key));
    m_items.push_back(std::move(_value));
}

ParamValue::eType ParamValue::GetType() const
{
    return m_type;
}

bool ParamValue::IsNull() const
{
    return m_type == eType::Null;
}

bool ParamValue::IsBool() const
{
    return m_type == eType::Bool;
}

bool ParamValue::IsNumber() const
{
    return m_type == eType::Number;
}

bool ParamValue::IsString() const
{
    return m_type == eType::String;
}

bool ParamValue::IsArray() const
{
    return m_type == eType::Array;
}

bool ParamValue::IsObject() const
{
    return m_type == eType::Object;
}

bool ParamValue::AsBool(
    const bool _bDefault) const
{
    return m_type == eType::Bool ? m_bValue : _bDefault;
}

float ParamValue::AsFloat(
    const float _default) const
{
    return m_type == eType::Number ? static_cast<float>(m_number) : _default;
}

int ParamValue::AsInt(
    const int _default) const
{
    return m_type == eType::Number ? static_cast<int>(std::llround(m_number)) : _default;
}

const String& ParamValue::AsString() const
{
    static const String kEmpty = {};
    return m_type == eType::String ? m_string : kEmpty;
}

size_t ParamValue::GetSize() const
{
    return (m_type == eType::Array || m_type == eType::Object) ? m_items.size() : 0;
}

const ParamValue& ParamValue::At(
    const size_t _index) const
{
    if (m_type != eType::Array || _index >= m_items.size())
    {
        return NullValue();
    }
    return m_items[_index];
}

Span<const String> ParamValue::GetKeys() const
{
    return m_keys;
}

const ParamValue* ParamValue::Find(
    const StringView _key) const
{
    if (m_type != eType::Object)
    {
        return nullptr;
    }
    for (size_t i = 0; i < m_keys.size(); ++i)
    {
        if (m_keys[i] == _key)
        {
            return &m_items[i];
        }
    }
    return nullptr;
}

bool ParamValue::Has(
    const StringView _key) const
{
    return Find(_key) != nullptr;
}

bool ParamValue::GetBool(
    const StringView _key,
    const bool       _bDefault) const
{
    const ParamValue* pValue = Find(_key);
    return pValue ? pValue->AsBool(_bDefault) : _bDefault;
}

int ParamValue::GetInt(
    const StringView _key,
    const int        _default) const
{
    const ParamValue* pValue = Find(_key);
    return pValue ? pValue->AsInt(_default) : _default;
}

String ParamValue::GetString(
    const StringView _key,
    const StringView _default) const
{
    const ParamValue* pValue = Find(_key);
    return (pValue && pValue->IsString()) ? pValue->m_string : String { _default };
}

Vec2 ParamValue::GetVec2(
    const StringView _key,
    const Vec2       _default) const
{
    const ParamValue* pValue = Find(_key);
    Vec2              out    = _default;
    if (pValue && pValue->TryGetVec2(out))
    {
        return out;
    }
    return _default;
}

bool ParamValue::TryGetVec2(
    Vec2& _out) const
{
    if (m_type != eType::Array || m_items.size() != 2 || !m_items[0].IsNumber() || !m_items[1].IsNumber())
    {
        return false;
    }
    _out = { m_items[0].AsFloat(), m_items[1].AsFloat() };
    return true;
}

const ParamValue& ParamValue::EmptyObject()
{
    static const ParamValue kEmpty = ParamValue::MakeObject();
    return kEmpty;
}

const ParamValue& ParamValue::NullValue()
{
    static const ParamValue kNull = {};
    return kNull;
}

bool CameraAnchor::IsRandomX() const
{
    return dxMax != dxMin;
}

int SpawnSpec::GetCount() const
{
    return bHasRepeat ? repeat.count : 1;
}

float BoundsCurve::EvaluateTop(
    const float _playerX) const
{
    if (top.empty())
    {
        return 0.f;
    }

    float result = top.back().y;
    if (_playerX <= top.front().x)
    {
        result = top.front().y;
    }
    else if (_playerX < top.back().x)
    {
        for (size_t i = 1; i < top.size(); ++i)
        {
            if (_playerX <= top[i].x)
            {
                const Vec2  a = top[i - 1];
                const Vec2  b = top[i];
                const float t = (_playerX - a.x) / (b.x - a.x);
                result        = a.y + (b.y - a.y) * t;
                break;
            }
        }
    }
    return bTrunc ? std::trunc(result) : result;
}

Aabb BoundsCurve::EvaluateBounds(
    const float _playerX) const
{
    const float top0 = EvaluateTop(_playerX);
    return Aabb::FromMinMax({ left, top0 }, { right, top0 + height });
}

int StageData::FindSection(
    const StringView _id) const
{
    for (size_t i = 0; i < sections.size(); ++i)
    {
        if (sections[i].id == _id)
        {
            return static_cast<int>(i);
        }
    }
    return -1;
}

const NamedRect* StageData::FindRect(
    const StringView _name) const
{
    for (const NamedRect& rect: rects)
    {
        if (rect.name == _name)
        {
            return &rect;
        }
    }
    return nullptr;
}

const BackdropMode* StageData::FindBackdrop(
    const StringView _id) const
{
    for (const BackdropMode& mode: backdrops)
    {
        if (mode.id == _id)
        {
            return &mode;
        }
    }
    return nullptr;
}

bool StageData::HasGroundMap(
    const StringView _name) const
{
    for (const String& map: groundMaps)
    {
        if (map == _name)
        {
            return true;
        }
    }
    return false;
}

[[nodiscard]] static int CountSpawnsIn_(
    const Vector<StageAction>& _actions)
{
    int count = 0;
    for (const StageAction& action: _actions)
    {
        if (action.type == eActionType::Spawn)
        {
            count += action.spawn.GetCount();
        }
        if (action.type == eActionType::Gate)
        {
            count += CountSpawnsIn_(action.actions);
        }
    }
    return count;
}

[[nodiscard]] static int CountWaveEntriesIn_(
    const Vector<StageAction>& _actions)
{
    int count = 0;
    for (const StageAction& action: _actions)
    {
        if (action.type == eActionType::SpawnWave)
        {
            for (const WaveEntry& entry: action.waveEntries)
            {
                count += entry.spawn.GetCount();
            }
        }
        if (action.type == eActionType::Gate)
        {
            count += CountWaveEntriesIn_(action.actions);
        }
    }
    return count;
}

[[nodiscard]] static int CountGatesIn_(
    const Vector<StageAction>& _actions)
{
    int count = 0;
    for (const StageAction& action: _actions)
    {
        if (action.type == eActionType::Gate)
        {
            ++count;
            count += CountGatesIn_(action.actions);
        }
    }
    return count;
}

int StageData::CountSpawns() const
{
    int count = 0;
    for (const StageSection& section: sections)
    {
        count += CountSpawnsIn_(section.actions);
    }
    return count;
}

int StageData::CountWaveEntries() const
{
    int count = 0;
    for (const StageSection& section: sections)
    {
        count += CountWaveEntriesIn_(section.actions);
    }
    return count;
}

int StageData::CountGates() const
{
    int count = 0;
    for (const StageSection& section: sections)
    {
        count += CountGatesIn_(section.actions);
    }
    return count;
}

const char* GetEntityKindName(
    const eEntityKind _kind)
{
    switch (_kind)
    {
        case eEntityKind::Enemy: return "enemy";
        case eEntityKind::Npc: return "npc";
        case eEntityKind::Prop: return "prop";
        case eEntityKind::Decor: return "decor";
        case eEntityKind::Item: return "item";
        case eEntityKind::Collider: return "collider";
    }
    return "?";
}

const char* GetActionTypeName(
    const eActionType _type)
{
    switch (_type)
    {
        case eActionType::Spawn: return "spawn";
        case eActionType::SpawnWave: return "spawnWave";
        case eActionType::SetBounds: return "setBounds";
        case eActionType::LockCamera: return "lockCamera";
        case eActionType::UnlockCamera: return "unlockCamera";
        case eActionType::CameraMove: return "cameraMove";
        case eActionType::SetCameraOffset: return "setCameraOffset";
        case eActionType::SetBackdrop: return "setBackdrop";
        case eActionType::SetGround: return "setGround";
        case eActionType::Gate: return "gate";
        case eActionType::Command: return "command";
        case eActionType::Clear: return "clear";
        case eActionType::Banner: return "banner";
        case eActionType::Fade: return "fade";
        case eActionType::PlayBgm: return "playBgm";
        case eActionType::PlayerInvincible: return "playerInvincible";
        case eActionType::EndStage: return "endStage";
    }
    return "?";
}

using JsonEntry = std::pair<StringView, JsonReader>;

template<typename E>
struct EnumEntry
{
    StringView name = {};
    E          value;
};

template<typename E, size_t N>
[[nodiscard]] static bool LookupEnum_(
    const ARRAY<EnumEntry<E>, N>& _table,
    const StringView              _name,
    E&                            _out)
{
    for (const EnumEntry<E>& entry: _table)
    {
        if (entry.name == _name)
        {
            _out = entry.value;
            return true;
        }
    }
    return false;
}

template<typename E, size_t N>
[[nodiscard]] static String ListEnum_(
    const ARRAY<EnumEntry<E>, N>& _table)
{
    String list = {};
    for (const EnumEntry<E>& entry: _table)
    {
        if (!list.empty())
        {
            list += '|';
        }
        list += entry.name;
    }
    return list;
}

constexpr ARRAY<EnumEntry<eEntityKind>, 6> kKindTable = {
    EnumEntry<eEntityKind> {    "enemy",    eEntityKind::Enemy },
    EnumEntry<eEntityKind> {      "npc",      eEntityKind::Npc },
    EnumEntry<eEntityKind> {     "prop",     eEntityKind::Prop },
    EnumEntry<eEntityKind> {    "decor",    eEntityKind::Decor },
    EnumEntry<eEntityKind> {     "item",     eEntityKind::Item },
    EnumEntry<eEntityKind> { "collider", eEntityKind::Collider }
};
constexpr ARRAY<EnumEntry<eOrigin>, 2> kOriginTable = {
    EnumEntry<eOrigin> {         "feet",         eOrigin::Feet },
    EnumEntry<eOrigin> { "legacyCenter", eOrigin::LegacyCenter }
};
constexpr ARRAY<EnumEntry<eStageLayer>, 8> kLayerTable = {
    EnumEntry<eStageLayer> {   "farBackground",   eStageLayer::FarBackground },
    EnumEntry<eStageLayer> {      "background",      eStageLayer::Background },
    EnumEntry<eStageLayer> {        "interact",        eStageLayer::Interact },
    EnumEntry<eStageLayer> {         "monster",         eStageLayer::Monster },
    EnumEntry<eStageLayer> {             "npc",             eStageLayer::Npc },
    EnumEntry<eStageLayer> {            "item",            eStageLayer::Item },
    EnumEntry<eStageLayer> { "frontBackground", eStageLayer::FrontBackground },
    EnumEntry<eStageLayer> {        "collider",        eStageLayer::Collider }
};
constexpr ARRAY<EnumEntry<eCollisionLayer>, 2> kCollisionTable = {
    EnumEntry<eCollisionLayer> {               "wall",               eCollisionLayer::Wall },
    EnumEntry<eCollisionLayer> { "groundExceptPlayer", eCollisionLayer::GroundExceptPlayer }
};
constexpr ARRAY<EnumEntry<eAnchorX>, 3> kAnchorXTable = {
    EnumEntry<eAnchorX> {   "left",   eAnchorX::Left },
    EnumEntry<eAnchorX> {  "right",  eAnchorX::Right },
    EnumEntry<eAnchorX> { "center", eAnchorX::Center }
};
constexpr ARRAY<EnumEntry<eAnchorY>, 3> kAnchorYTable = {
    EnumEntry<eAnchorY> {    "top",    eAnchorY::Top },
    EnumEntry<eAnchorY> { "bottom", eAnchorY::Bottom },
    EnumEntry<eAnchorY> { "center", eAnchorY::Center }
};
constexpr ARRAY<EnumEntry<eActionType>, 17> kActionTable = {
    EnumEntry<eActionType> {            "spawn",            eActionType::Spawn },
    EnumEntry<eActionType> {        "spawnWave",        eActionType::SpawnWave },
    EnumEntry<eActionType> {        "setBounds",        eActionType::SetBounds },
    EnumEntry<eActionType> {       "lockCamera",       eActionType::LockCamera },
    EnumEntry<eActionType> {     "unlockCamera",     eActionType::UnlockCamera },
    EnumEntry<eActionType> {       "cameraMove",       eActionType::CameraMove },
    EnumEntry<eActionType> {  "setCameraOffset",  eActionType::SetCameraOffset },
    EnumEntry<eActionType> {      "setBackdrop",      eActionType::SetBackdrop },
    EnumEntry<eActionType> {        "setGround",        eActionType::SetGround },
    EnumEntry<eActionType> {             "gate",             eActionType::Gate },
    EnumEntry<eActionType> {          "command",          eActionType::Command },
    EnumEntry<eActionType> {            "clear",            eActionType::Clear },
    EnumEntry<eActionType> {           "banner",           eActionType::Banner },
    EnumEntry<eActionType> {             "fade",             eActionType::Fade },
    EnumEntry<eActionType> {          "playBgm",          eActionType::PlayBgm },
    EnumEntry<eActionType> { "playerInvincible", eActionType::PlayerInvincible },
    EnumEntry<eActionType> {         "endStage",         eActionType::EndStage }
};
constexpr ARRAY<EnumEntry<eWhen>, 3> kWhenTable = {
    EnumEntry<eWhen> { "normal", eWhen::Normal },
    EnumEntry<eWhen> { "silent", eWhen::Silent },
    EnumEntry<eWhen> { "always", eWhen::Always }
};
constexpr ARRAY<EnumEntry<eGateUntil>, 2> kUntilTable = {
    EnumEntry<eGateUntil> {        "defeated",        eGateUntil::Defeated },
    EnumEntry<eGateUntil> { "destroyFinished", eGateUntil::DestroyFinished }
};
constexpr ARRAY<EnumEntry<eFadeTarget>, 2> kFadeTable = {
    EnumEntry<eFadeTarget> { "black", eFadeTarget::Black },
    EnumEntry<eFadeTarget> { "clear", eFadeTarget::Clear }
};

[[nodiscard]] static bool IsCreature_(
    const eEntityKind _kind)
{
    return _kind == eEntityKind::Enemy || _kind == eEntityKind::Npc;
}

[[nodiscard]] static eOrigin DefaultOrigin_(
    const eEntityKind _kind)
{
    return IsCreature_(_kind) ? eOrigin::Feet : eOrigin::LegacyCenter;
}

[[nodiscard]] static eWhen DefaultWhen_(
    const StageAction& _action)
{
    switch (_action.type)
    {
        case eActionType::Spawn:
            switch (_action.spawn.kind)
            {
                case eEntityKind::Prop:
                case eEntityKind::Decor:
                case eEntityKind::Item:
                case eEntityKind::Collider:
                case eEntityKind::Npc: return eWhen::Always;
                case eEntityKind::Enemy: return eWhen::Normal;
            }
            return eWhen::Normal;
        case eActionType::SetBounds:
        case eActionType::SetBackdrop:
        case eActionType::SetCameraOffset:
        case eActionType::UnlockCamera: return eWhen::Always;
        default: return eWhen::Normal;
    }
}

[[nodiscard]] static ParamValue ToParam_(
    const JsonReader& _json)
{
    if (_json.IsBool())
    {
        return ParamValue::MakeBool(_json.Read<bool>().GetValue());
    }
    if (_json.IsNumber())
    {
        return ParamValue::MakeNumber(_json.Read<double>().GetValue());
    }
    if (_json.IsString())
    {
        return ParamValue::MakeString(_json.Read<String>().GetValue());
    }
    if (_json.IsArray())
    {
        ParamValue array = ParamValue::MakeArray();
        for (const JsonReader element: _json.GetArray())
        {
            array.PushBack(ToParam_(element));
        }
        return array;
    }
    if (_json.IsObject())
    {
        ParamValue object = ParamValue::MakeObject();
        for (const JsonEntry& entry: _json.GetObject())
        {
            object.Set(String { entry.first }, ToParam_(entry.second));
        }
        return object;
    }
    return ParamValue {};
}

class Parser
{
public:
    explicit Parser(
        StageData& _out)
        : m_out(_out)
    {
    }

    [[nodiscard]] bool Parse(const JsonReader& _root);

    [[nodiscard]] const String& GetError() const;

private:
    enum class eSpawnContext
    {
        Action,
        WaveEntry
    };

    bool Fail_(const String& _where, const String& _message);

    bool GetField_(
        const JsonReader&          _obj,
        const String&              _where,
        StringView                 _key,
        bool                       _bRequired,
        std::optional<JsonReader>& _out);

    bool GetNumber_(
        const JsonReader& _obj,
        const String&     _where,
        StringView        _key,
        bool              _bRequired,
        double&           _out,
        bool*             _pFound = nullptr);

    bool GetFloat_(
        const JsonReader& _obj,
        const String&     _where,
        StringView        _key,
        bool              _bRequired,
        float&            _out,
        bool*             _pFound = nullptr);

    bool GetInt_(
        const JsonReader& _obj,
        const String&     _where,
        StringView        _key,
        bool              _bRequired,
        int&              _out,
        bool*             _pFound = nullptr);

    bool GetBool_(
        const JsonReader& _obj,
        const String&     _where,
        StringView        _key,
        bool              _bRequired,
        bool&             _out,
        bool*             _pFound = nullptr);

    bool GetString_(
        const JsonReader& _obj,
        const String&     _where,
        StringView        _key,
        bool              _bRequired,
        String&           _out,
        bool*             _pFound = nullptr);

    bool GetVec2_(
        const JsonReader& _obj,
        const String&     _where,
        StringView        _key,
        bool              _bRequired,
        Vec2&             _out,
        bool*             _pFound = nullptr);

    bool GetObject_(
        const JsonReader&          _obj,
        const String&              _where,
        StringView                 _key,
        bool                       _bRequired,
        std::optional<JsonReader>& _out);

    bool GetArray_(
        const JsonReader&          _obj,
        const String&              _where,
        StringView                 _key,
        bool                       _bRequired,
        std::optional<JsonReader>& _out);

    template<typename E, size_t N>
    bool GetEnum_(
        const JsonReader&             _obj,
        const String&                 _where,
        const StringView              _key,
        const bool                    _bRequired,
        const ARRAY<EnumEntry<E>, N>& _table,
        E&                            _out,
        bool* const                   _pFound = nullptr)
    {
        String text   = {};
        bool   bFound = false;
        if (!GetString_(_obj, _where, _key, _bRequired, text, &bFound))
        {
            return false;
        }
        if (_pFound)
        {
            *_pFound = bFound;
        }
        if (!bFound)
        {
            return true;
        }
        if (!LookupEnum_(_table, text, _out))
        {
            return Fail_(_where, std::format("field '{}' has unknown value '{}' (expected {})", _key, text, ListEnum_(_table)));
        }
        return true;
    }

    [[nodiscard]] bool ExpectObject_(const JsonReader& _json, const String& _where);
    bool               CheckKeys_(const JsonReader& _obj, const String& _where, std::initializer_list<StringView> _allowed);

    bool ParseWorld_(const JsonReader& _root);
    bool ParseBgm_(const JsonReader& _root);
    bool ParseGround_(const JsonReader& _root);
    bool ParseBackdrop_(const JsonReader& _root);
    bool ParseRects_(const JsonReader& _root);
    bool ParseCamera_(const JsonReader& _root);
    bool ParsePlayer_(const JsonReader& _root);
    bool ParseSections_(const JsonReader& _root);

    bool ParseRect_(
        const JsonReader& _obj,
        const String&     _where,
        StringView        _key,
        bool              _bAllowCurrent,
        RectRef&          _out);

    bool ParseRectArray_(
        const JsonReader& _array,
        const String&     _where,
        StringView        _key,
        Aabb&             _out);

    bool ParseSpawn_(
        const JsonReader& _obj,
        const String&     _where,
        eSpawnContext     _context,
        SpawnSpec&        _out);

    bool ParseCameraAnchor_(const JsonReader& _obj, const String& _where, CameraAnchor& _out);
    bool ParseAction_(const JsonReader& _obj, const String& _where, StageAction& _out);
    bool ParseActionList_(const JsonReader& _array, const String& _where, Vector<StageAction>& _out);

    bool               Validate_();
    bool               AddSpawnId_(const SpawnSpec& _spawn, const String& _at);
    [[nodiscard]] bool HasSpawnId_(const String& _id) const;
    bool               CollectIds_(const Vector<StageAction>& _actions, const String& _where);
    bool               CheckRefs_(const Vector<StageAction>& _actions, const String& _where);

    StageData&     m_out;
    String         m_error    = {};
    Vector<String> m_spawnIds = {};
    Vector<String> m_gateIds  = {};
};

const String& Parser::GetError() const
{
    return m_error;
}

bool Parser::Fail_(
    const String& _where,
    const String& _message)
{
    if (m_error.empty())
    {
        m_error = _where.empty() ? _message : std::format("{}: {}", _where, _message);
    }
    return false;
}

bool Parser::GetField_(
    const JsonReader&          _obj,
    const String&              _where,
    const StringView           _key,
    const bool                 _bRequired,
    std::optional<JsonReader>& _out)
{
    _out.reset();
    if (!_obj.HasField(_key))
    {
        if (_bRequired)
        {
            return Fail_(_where, std::format("missing required field '{}'", _key));
        }
        return true;
    }
    const Result<JsonReader> found = _obj.FindField(_key);
    _out                           = found.GetValue();
    return true;
}

bool Parser::GetNumber_(
    const JsonReader& _obj,
    const String&     _where,
    const StringView  _key,
    const bool        _bRequired,
    double&           _out,
    bool* const       _pFound)
{
    std::optional<JsonReader> field = std::nullopt;
    if (!GetField_(_obj, _where, _key, _bRequired, field))
    {
        return false;
    }
    if (_pFound)
    {
        *_pFound = field.has_value();
    }
    if (!field)
    {
        return true;
    }
    if (!field->IsNumber())
    {
        return Fail_(_where, std::format("field '{}' must be a number", _key));
    }
    _out = field->Read<double>().GetValue();
    return true;
}

bool Parser::GetFloat_(
    const JsonReader& _obj,
    const String&     _where,
    const StringView  _key,
    const bool        _bRequired,
    float&            _out,
    bool* const       _pFound)
{
    double value = static_cast<double>(_out);
    if (!GetNumber_(_obj, _where, _key, _bRequired, value, _pFound))
    {
        return false;
    }
    _out = static_cast<float>(value);
    return true;
}

bool Parser::GetInt_(
    const JsonReader& _obj,
    const String&     _where,
    const StringView  _key,
    const bool        _bRequired,
    int&              _out,
    bool* const       _pFound)
{
    double value  = static_cast<double>(_out);
    bool   bFound = false;
    if (!GetNumber_(_obj, _where, _key, _bRequired, value, &bFound))
    {
        return false;
    }
    if (_pFound)
    {
        *_pFound = bFound;
    }
    if (bFound)
    {
        if (value != std::floor(value) || std::abs(value) > 2147483647.0)
        {
            return Fail_(_where, std::format("field '{}' must be an integer", _key));
        }
        _out = static_cast<int>(value);
    }
    return true;
}

bool Parser::GetBool_(
    const JsonReader& _obj,
    const String&     _where,
    const StringView  _key,
    const bool        _bRequired,
    bool&             _out,
    bool* const       _pFound)
{
    std::optional<JsonReader> field = std::nullopt;
    if (!GetField_(_obj, _where, _key, _bRequired, field))
    {
        return false;
    }
    if (_pFound)
    {
        *_pFound = field.has_value();
    }
    if (!field)
    {
        return true;
    }
    if (!field->IsBool())
    {
        return Fail_(_where, std::format("field '{}' must be a boolean", _key));
    }
    _out = field->Read<bool>().GetValue();
    return true;
}

bool Parser::GetString_(
    const JsonReader& _obj,
    const String&     _where,
    const StringView  _key,
    const bool        _bRequired,
    String&           _out,
    bool* const       _pFound)
{
    std::optional<JsonReader> field = std::nullopt;
    if (!GetField_(_obj, _where, _key, _bRequired, field))
    {
        return false;
    }
    if (_pFound)
    {
        *_pFound = field.has_value();
    }
    if (!field)
    {
        return true;
    }
    if (!field->IsString())
    {
        return Fail_(_where, std::format("field '{}' must be a string", _key));
    }
    _out = field->Read<String>().GetValue();
    return true;
}

bool Parser::GetVec2_(
    const JsonReader& _obj,
    const String&     _where,
    const StringView  _key,
    const bool        _bRequired,
    Vec2&             _out,
    bool* const       _pFound)
{
    std::optional<JsonReader> field = std::nullopt;
    if (!GetField_(_obj, _where, _key, _bRequired, field))
    {
        return false;
    }
    if (_pFound)
    {
        *_pFound = field.has_value();
    }
    if (!field)
    {
        return true;
    }
    if (!field->IsArray() || field->GetSize() != 2)
    {
        return Fail_(_where, std::format("field '{}' must be an array of 2 numbers", _key));
    }
    ARRAY<float, 2> values = { 0.f, 0.f };
    int             index  = 0;
    for (const JsonReader element: field->GetArray())
    {
        if (!element.IsNumber())
        {
            return Fail_(_where, std::format("field '{}' must be an array of 2 numbers", _key));
        }
        values[index++] = static_cast<float>(element.Read<double>().GetValue());
    }
    _out = { values[0], values[1] };
    return true;
}

bool Parser::GetObject_(
    const JsonReader&          _obj,
    const String&              _where,
    const StringView           _key,
    const bool                 _bRequired,
    std::optional<JsonReader>& _out)
{
    if (!GetField_(_obj, _where, _key, _bRequired, _out))
    {
        return false;
    }
    if (_out && !_out->IsObject())
    {
        return Fail_(_where, std::format("field '{}' must be an object", _key));
    }
    return true;
}

bool Parser::GetArray_(
    const JsonReader&          _obj,
    const String&              _where,
    const StringView           _key,
    const bool                 _bRequired,
    std::optional<JsonReader>& _out)
{
    if (!GetField_(_obj, _where, _key, _bRequired, _out))
    {
        return false;
    }
    if (_out && !_out->IsArray())
    {
        return Fail_(_where, std::format("field '{}' must be an array", _key));
    }
    return true;
}

bool Parser::ExpectObject_(
    const JsonReader& _json,
    const String&     _where)
{
    if (!_json.IsObject())
    {
        return Fail_(_where, "must be an object");
    }
    return true;
}

bool Parser::CheckKeys_(
    const JsonReader&                       _obj,
    const String&                           _where,
    const std::initializer_list<StringView> _allowed)
{
    if (!ExpectObject_(_obj, _where))
    {
        return false;
    }
    for (const JsonEntry& entry: _obj.GetObject())
    {
        const StringView key = entry.first;
        if (key == "note")
        {
            continue;
        }
        bool bKnown = false;
        for (const StringView allowed: _allowed)
        {
            if (allowed == key)
            {
                bKnown = true;
                break;
            }
        }
        if (!bKnown)
        {
            return Fail_(_where, std::format("unknown key '{}'", key));
        }
    }
    return true;
}

bool Parser::Parse(
    const JsonReader& _root)
{
    const String where = "root";
    if (!CheckKeys_(_root, where, { "version", "name", "world", "bgm", "ground", "backdrop", "player", "rects", "camera", "sections" }))
    {
        return false;
    }

    if (!GetInt_(_root, where, "version", true, m_out.version) || !GetString_(_root, where, "name", true, m_out.name))
    {
        return false;
    }
    if (m_out.version != 1)
    {
        return Fail_(where, std::format("unsupported version {} (expected 1)", m_out.version));
    }

    if (!ParseWorld_(_root) || !ParseBgm_(_root) || !ParseGround_(_root) || !ParseBackdrop_(_root) || !ParseRects_(_root) || !ParseCamera_(_root) || !ParsePlayer_(_root) || !ParseSections_(_root))
    {
        return false;
    }
    return Validate_();
}

bool Parser::ParseWorld_(
    const JsonReader& _root)
{
    const String              where = "world";
    std::optional<JsonReader> world = std::nullopt;
    if (!GetObject_(_root, "root", "world", true, world) || !CheckKeys_(*world, where, { "size", "view", "background", "endless", "terrainDepth" }))
    {
        return false;
    }
    if (!GetVec2_(*world, where, "size", true, m_out.worldSize) || !GetVec2_(*world, where, "view", false, m_out.viewSize) || !GetString_(*world, where, "background", true, m_out.background) || !GetInt_(*world, where, "terrainDepth", false, m_out.terrainDepth))
    {
        return false;
    }
    std::optional<JsonReader> endless = std::nullopt;
    if (!GetObject_(*world, where, "endless", false, endless))
    {
        return false;
    }
    if (endless)
    {
        const String endlessWhere = "world.endless";
        if (!CheckKeys_(*endless, endlessWhere, { "from", "tileX", "tileW" }) || !GetFloat_(*endless, endlessWhere, "from", true, m_out.endless.fromX) || !GetFloat_(*endless, endlessWhere, "tileX", true, m_out.endless.tileX) || !GetFloat_(*endless, endlessWhere, "tileW", true, m_out.endless.tileW))
        {
            return false;
        }
        m_out.endless.bEnabled = true;
    }
    if (m_out.worldSize.x <= 0.f || m_out.worldSize.y <= 0.f || m_out.viewSize.x <= 0.f || m_out.viewSize.y <= 0.f)
    {
        return Fail_(where, "size and view must be positive");
    }
    return true;
}

bool Parser::ParseBgm_(
    const JsonReader& _root)
{
    std::optional<JsonReader> bgm = std::nullopt;
    if (!GetObject_(_root, "root", "bgm", false, bgm) || !bgm)
    {
        return !m_error.size();
    }
    const String where = "bgm";
    return CheckKeys_(*bgm, where, { "name", "loop" }) && GetString_(*bgm, where, "name", true, m_out.bgmName) && GetBool_(*bgm, where, "loop", false, m_out.bBgmLoop);
}

bool Parser::ParseGround_(
    const JsonReader& _root)
{
    const String              where  = "ground";
    std::optional<JsonReader> ground = std::nullopt;
    if (!GetObject_(_root, "root", "ground", true, ground) || !CheckKeys_(*ground, where, { "initial", "maps" }))
    {
        return false;
    }
    std::optional<JsonReader> maps = std::nullopt;
    if (!GetString_(*ground, where, "initial", true, m_out.groundInitial) || !GetArray_(*ground, where, "maps", true, maps))
    {
        return false;
    }
    for (const JsonReader element: maps->GetArray())
    {
        if (!element.IsString())
        {
            return Fail_(where, "maps entries must be strings");
        }
        String name = element.Read<String>().GetValue();
        if (m_out.HasGroundMap(name))
        {
            return Fail_(where, std::format("duplicate ground map '{}'", name));
        }
        m_out.groundMaps.push_back(std::move(name));
    }
    if (!m_out.HasGroundMap(m_out.groundInitial))
    {
        return Fail_(where, std::format("initial '{}' is not in maps", m_out.groundInitial));
    }
    return true;
}

bool Parser::ParseBackdrop_(
    const JsonReader& _root)
{
    std::optional<JsonReader> backdrop = std::nullopt;
    if (!GetObject_(_root, "root", "backdrop", false, backdrop))
    {
        return false;
    }
    if (!backdrop)
    {
        return true;
    }
    const String where = "backdrop";
    if (!CheckKeys_(*backdrop, where, { "initial", "modes" }))
    {
        return false;
    }
    std::optional<JsonReader> modes = std::nullopt;
    if (!GetString_(*backdrop, where, "initial", true, m_out.backdropInitial) || !GetObject_(*backdrop, where, "modes", true, modes))
    {
        return false;
    }
    for (const JsonEntry& entry: modes->GetObject())
    {
        const StringView id        = entry.first;
        const JsonReader json      = entry.second;
        const String     modeWhere = std::format("backdrop.modes.{}", id);
        if (!CheckKeys_(json, modeWhere, { "sprite", "offset", "parallaxX", "period" }))
        {
            return false;
        }
        BackdropMode mode = {};
        mode.id           = String { id };
        if (!GetString_(json, modeWhere, "sprite", true, mode.sprite) || !GetVec2_(json, modeWhere, "offset", false, mode.offset) || !GetFloat_(json, modeWhere, "parallaxX", false, mode.parallaxX) || !GetFloat_(json, modeWhere, "period", false, mode.period))
        {
            return false;
        }
        m_out.backdrops.push_back(std::move(mode));
    }
    if (!m_out.FindBackdrop(m_out.backdropInitial))
    {
        return Fail_(where, std::format("initial '{}' is not in modes", m_out.backdropInitial));
    }
    return true;
}

bool Parser::ParseRectArray_(
    const JsonReader& _array,
    const String&     _where,
    const StringView  _key,
    Aabb&             _out)
{
    if (!_array.IsArray() || _array.GetSize() != 4)
    {
        return Fail_(_where, std::format("'{}' must be an array [left, top, right, bottom]", _key));
    }
    ARRAY<float, 4> values = {};
    int             index  = 0;
    for (const JsonReader element: _array.GetArray())
    {
        if (!element.IsNumber())
        {
            return Fail_(_where, std::format("'{}' must be an array of 4 numbers", _key));
        }
        values[index++] = static_cast<float>(element.Read<double>().GetValue());
    }
    if (values[2] <= values[0] || values[3] <= values[1])
    {
        return Fail_(_where, std::format("'{}' must have right > left and bottom > top", _key));
    }
    _out = Aabb::FromMinMax({ values[0], values[1] }, { values[2], values[3] });
    return true;
}

bool Parser::ParseRects_(
    const JsonReader& _root)
{
    std::optional<JsonReader> rects = std::nullopt;
    if (!GetObject_(_root, "root", "rects", false, rects))
    {
        return false;
    }
    if (!rects)
    {
        return true;
    }
    for (const JsonEntry& entry: rects->GetObject())
    {
        const StringView name = entry.first;
        const JsonReader json = entry.second;
        if (name == "note")
        {
            continue;
        }
        if (name == "current")
        {
            return Fail_("rects", "'current' is a reserved rect name");
        }
        NamedRect rect = {};
        rect.name      = String { name };
        if (!ParseRectArray_(json, std::format("rects.{}", name), name, rect.rect))
        {
            return false;
        }
        m_out.rects.push_back(std::move(rect));
    }
    return true;
}

bool Parser::ParseRect_(
    const JsonReader& _obj,
    const String&     _where,
    const StringView  _key,
    const bool        _bAllowCurrent,
    RectRef&          _out)
{
    std::optional<JsonReader> field = std::nullopt;
    if (!GetField_(_obj, _where, _key, true, field))
    {
        return false;
    }
    if (field->IsString())
    {
        const String name = field->Read<String>().GetValue();
        if (name == "current")
        {
            if (!_bAllowCurrent)
            {
                return Fail_(_where, "'current' is not allowed here");
            }
            _out.bCurrent = true;
            _out.name     = name;
            return true;
        }
        const NamedRect* pRect = m_out.FindRect(name);
        if (!pRect)
        {
            return Fail_(_where, std::format("unknown rect '{}'", name));
        }
        _out.bCurrent = false;
        _out.rect     = pRect->rect;
        _out.name     = name;
        return true;
    }
    _out.bCurrent = false;
    return ParseRectArray_(*field, _where, _key, _out.rect);
}

bool Parser::ParseCamera_(
    const JsonReader& _root)
{
    m_out.camera.initialBounds = Aabb::FromMinMax({ 0.f, 0.f }, m_out.worldSize);

    std::optional<JsonReader> camera = std::nullopt;
    if (!GetObject_(_root, "root", "camera", false, camera))
    {
        return false;
    }
    if (!camera)
    {
        return true;
    }
    const String where = "camera";
    if (!CheckKeys_(*camera, where, { "policy", "initialBounds", "initialOffset", "playerMargin", "boundsCurves" }))
    {
        return false;
    }

    String policy = "forward";
    if (!GetString_(*camera, where, "policy", false, policy))
    {
        return false;
    }
    if (policy != "forward" && policy != "free")
    {
        return Fail_(where, std::format("unknown policy '{}' (expected forward|free)", policy));
    }
    m_out.camera.bForward = policy == "forward";

    if (camera->HasField("initialBounds"))
    {
        std::optional<JsonReader> bounds = std::nullopt;
        if (!GetField_(*camera, where, "initialBounds", true, bounds) || !ParseRectArray_(*bounds, where, "initialBounds", m_out.camera.initialBounds))
        {
            return false;
        }
    }
    if (!GetVec2_(*camera, where, "initialOffset", false, m_out.camera.initialOffset) || !GetFloat_(*camera, where, "playerMargin", false, m_out.camera.playerMargin))
    {
        return false;
    }
    if (m_out.camera.playerMargin < 0.f)
    {
        return Fail_(where, "playerMargin must be >= 0");
    }

    std::optional<JsonReader> curves = std::nullopt;
    if (!GetArray_(*camera, where, "boundsCurves", false, curves) || !curves)
    {
        return !m_error.size();
    }
    int curveIndex = 0;
    for (const JsonReader json: curves->GetArray())
    {
        const String curveWhere = std::format("camera.boundsCurves[{}]", curveIndex++);
        if (!CheckKeys_(json, curveWhere, { "id", "source", "from", "until", "left", "right", "height", "top", "round" }))
        {
            return false;
        }
        BoundsCurve curve  = {};
        String      source = "playerX";
        String      round  = "none";
        if (!GetString_(json, curveWhere, "id", true, curve.id) || !GetString_(json, curveWhere, "source", false, source) || !GetString_(json, curveWhere, "from", true, curve.from) || !GetString_(json, curveWhere, "until", true, curve.until) || !GetFloat_(json, curveWhere, "left", true, curve.left) || !GetFloat_(json, curveWhere, "right", true, curve.right) || !GetFloat_(json, curveWhere, "height", true, curve.height) || !GetString_(json, curveWhere, "round", false, round))
        {
            return false;
        }
        if (source != "playerX")
        {
            return Fail_(curveWhere, std::format("unsupported source '{}' (expected playerX)", source));
        }
        if (round != "trunc" && round != "none")
        {
            return Fail_(curveWhere, std::format("unknown round '{}' (expected trunc|none)", round));
        }
        curve.bTrunc = round == "trunc";
        if (curve.right <= curve.left || curve.height <= 0.f)
        {
            return Fail_(curveWhere, "right must be > left and height must be > 0");
        }

        std::optional<JsonReader> top = std::nullopt;
        if (!GetArray_(json, curveWhere, "top", true, top))
        {
            return false;
        }
        if (top->GetSize() < 2)
        {
            return Fail_(curveWhere, "top needs at least 2 points");
        }
        for (const JsonReader point: top->GetArray())
        {
            if (!point.IsArray() || point.GetSize() != 2)
            {
                return Fail_(curveWhere, "top points must be [x, y]");
            }
            ARRAY<float, 2> values = {};
            int             index  = 0;
            for (const JsonReader element: point.GetArray())
            {
                if (!element.IsNumber())
                {
                    return Fail_(curveWhere, "top points must be [x, y] numbers");
                }
                values[index++] = static_cast<float>(element.Read<double>().GetValue());
            }
            if (!curve.top.empty() && values[0] <= curve.top.back().x)
            {
                return Fail_(curveWhere, "top points must have strictly increasing x");
            }
            curve.top.push_back({ values[0], values[1] });
        }
        m_out.camera.boundsCurves.push_back(std::move(curve));
    }
    return true;
}

bool Parser::ParsePlayer_(
    const JsonReader& _root)
{
    std::optional<JsonReader> player = std::nullopt;
    if (!GetObject_(_root, "root", "player", false, player))
    {
        return false;
    }
    if (!player)
    {
        return true;
    }
    const String where = "player";
    if (!CheckKeys_(*player, where, { "x", "y", "origin", "halfHeight", "settle", "groundSnap", "fromY" }))
    {
        return false;
    }
    PlayerStart& start    = m_out.player;
    bool         bHasHalf = false;
    if (!GetFloat_(*player, where, "x", true, start.x) || !GetFloat_(*player, where, "y", false, start.y, &start.bHasY) || !GetEnum_(*player, where, "origin", false, kOriginTable, start.origin) || !GetFloat_(*player, where, "halfHeight", false, start.halfHeight, &bHasHalf) || !GetBool_(*player, where, "settle", false, start.bSettle) || !GetBool_(*player, where, "groundSnap", false, start.bGroundSnap) || !GetFloat_(*player, where, "fromY", false, start.fromY))
    {
        return false;
    }
    if (!start.bHasY && !start.bGroundSnap)
    {
        return Fail_(where, "needs 'y' unless groundSnap is true");
    }
    if (start.bHasY && start.bGroundSnap)
    {
        return Fail_(where, "'y' must be omitted when groundSnap is true");
    }
    return true;
}

bool Parser::ParseCameraAnchor_(
    const JsonReader& _obj,
    const String&     _where,
    CameraAnchor&     _out)
{
    std::optional<JsonReader> anchor = std::nullopt;
    if (!GetObject_(_obj, _where, "camera", true, anchor))
    {
        return false;
    }
    const String where = _where + " camera";
    if (!CheckKeys_(*anchor, where, { "ax", "dx", "ay", "dy" }) || !GetEnum_(*anchor, where, "ax", true, kAnchorXTable, _out.ax) || !GetEnum_(*anchor, where, "ay", true, kAnchorYTable, _out.ay) || !GetFloat_(*anchor, where, "dy", false, _out.dy))
    {
        return false;
    }

    if (anchor->HasField("dx"))
    {
        std::optional<JsonReader> dx = std::nullopt;
        if (!GetField_(*anchor, where, "dx", true, dx))
        {
            return false;
        }
        if (dx->IsNumber())
        {
            const double value = dx->Read<double>().GetValue();
            if (value != std::floor(value))
            {
                return Fail_(where, "field 'dx' must be an integer or [min, max] integers");
            }
            _out.dxMin = _out.dxMax = static_cast<int>(value);
        }
        else if (dx->IsArray() && dx->GetSize() == 2)
        {
            ARRAY<int, 2> values = {};
            int           index  = 0;
            for (const JsonReader element: dx->GetArray())
            {
                if (!element.IsNumber() || element.Read<double>().GetValue() != std::floor(element.Read<double>().GetValue()))
                {
                    return Fail_(where, "field 'dx' must be an integer or [min, max] integers");
                }
                values[index++] = static_cast<int>(element.Read<double>().GetValue());
            }
            if (values[0] > values[1])
            {
                return Fail_(where, "field 'dx' range must have min <= max");
            }
            _out.dxMin = values[0];
            _out.dxMax = values[1];
        }
        else
        {
            return Fail_(where, "field 'dx' must be an integer or [min, max] integers");
        }
    }
    return true;
}

bool Parser::ParseSpawn_(
    const JsonReader&   _obj,
    const String&       _where,
    const eSpawnContext _context,
    SpawnSpec&          _out)
{
    static constexpr ARRAY<StringView, 24> kSpawnKeys = {
        { "kind", "def", "id", "x", "y", "z", "origin", "halfHeight", "groundSnap", "fromY", "settle", "camera", "repeat", "facing", "hp", "drop", "ambush", "layer", "cullMargin", "size", "collisionLayer", "anim", "sprite", "params" }
    };
    if (!ExpectObject_(_obj, _where))
    {
        return false;
    }
    for (const JsonEntry& entry: _obj.GetObject())
    {
        const StringView key = entry.first;
        if (key == "note")
        {
            continue;
        }
        bool bKnown = false;
        for (const StringView allowed: kSpawnKeys)
        {
            bKnown = bKnown || allowed == key;
        }
        if (_context == eSpawnContext::Action)
        {
            bKnown = bKnown || key == "type" || key == "delay" || key == "when";
        }
        else if (_context == eSpawnContext::WaveEntry)
        {
            bKnown = bKnown || key == "delay" || key == "every" || key == "times";
        }
        if (!bKnown)
        {
            return Fail_(_where, std::format("unknown key '{}'", key));
        }
    }

    if (!GetEnum_(_obj, _where, "kind", true, kKindTable, _out.kind) || !GetString_(_obj, _where, "def", true, _out.def) || !GetString_(_obj, _where, "id", false, _out.id))
    {
        return false;
    }
    if (_out.def.empty())
    {
        return Fail_(_where, "field 'def' must not be empty");
    }

    _out.origin          = DefaultOrigin_(_out.kind);
    bool bOriginExplicit = false;
    if (!GetEnum_(_obj, _where, "origin", false, kOriginTable, _out.origin, &bOriginExplicit) || !GetFloat_(_obj, _where, "x", false, _out.x, &_out.bHasX) || !GetFloat_(_obj, _where, "y", false, _out.y, &_out.bHasY) || !GetFloat_(_obj, _where, "halfHeight", false, _out.halfHeight, &_out.bHasHalfHeight) || !GetBool_(_obj, _where, "groundSnap", false, _out.bGroundSnap) || !GetFloat_(_obj, _where, "fromY", false, _out.fromY) || !GetBool_(_obj, _where, "settle", false, _out.bSettle))
    {
        return false;
    }

    _out.bHasCamera = _obj.HasField("camera");
    if (_out.bHasCamera)
    {
        if (_out.bHasX || _out.bHasY)
        {
            return Fail_(_where, "'camera' cannot be combined with 'x'/'y'");
        }
        if (_out.bGroundSnap)
        {
            return Fail_(_where, "'camera' cannot be combined with groundSnap");
        }
        if (!ParseCameraAnchor_(_obj, _where, _out.camera))
        {
            return false;
        }
    }
    else
    {
        if (!_out.bHasX)
        {
            return Fail_(_where, "spawn needs a position: 'x' (and 'y' or groundSnap) or 'camera'");
        }
        if (_out.bGroundSnap && _out.bHasY)
        {
            return Fail_(_where, "'y' must be omitted when groundSnap is true");
        }
        if (!_out.bGroundSnap && !_out.bHasY)
        {
            return Fail_(_where, "spawn needs 'y' unless groundSnap is true");
        }
    }
    if (_out.bGroundSnap && _out.origin != eOrigin::Feet)
    {
        return Fail_(_where, "groundSnap requires origin 'feet' (set origin explicitly for non-creature kinds)");
    }

    if (_out.bHasHalfHeight)
    {
        if (_out.origin != eOrigin::LegacyCenter)
        {
            return Fail_(_where, "'halfHeight' is only valid with origin 'legacyCenter'");
        }
        if (_out.halfHeight < 0.f)
        {
            return Fail_(_where, "'halfHeight' must be >= 0");
        }
    }
    if (IsCreature_(_out.kind) && _out.origin == eOrigin::LegacyCenter && !_out.bHasHalfHeight)
    {
        return Fail_(_where, "legacyCenter creature needs 'halfHeight'");
    }

    if (_obj.HasField("repeat"))
    {
        std::optional<JsonReader> repeat = std::nullopt;
        if (!GetObject_(_obj, _where, "repeat", true, repeat))
        {
            return false;
        }
        const String repeatWhere = _where + " repeat";
        if (!CheckKeys_(*repeat, repeatWhere, { "count", "stepX", "stepY" }) || !GetInt_(*repeat, repeatWhere, "count", true, _out.repeat.count) || !GetFloat_(*repeat, repeatWhere, "stepX", false, _out.repeat.stepX) || !GetFloat_(*repeat, repeatWhere, "stepY", false, _out.repeat.stepY))
        {
            return false;
        }
        if (_out.repeat.count < 1)
        {
            return Fail_(repeatWhere, "'count' must be >= 1");
        }
        _out.bHasRepeat = true;
    }

    if (_obj.HasField("facing"))
    {
        String facing = {};
        if (!GetString_(_obj, _where, "facing", true, facing))
        {
            return false;
        }
        if (facing == "left")
        {
            _out.facing = -1;
        }
        else if (facing == "right")
        {
            _out.facing = 1;
        }
        else
        {
            return Fail_(_where, std::format("field 'facing' has unknown value '{}' (expected left|right)", facing));
        }
    }

    if (!GetInt_(_obj, _where, "z", false, _out.z, &_out.bHasZ) || !GetInt_(_obj, _where, "hp", false, _out.hp, &_out.bHasHp) || !GetString_(_obj, _where, "drop", false, _out.drop) || !GetFloat_(_obj, _where, "cullMargin", false, _out.cullMargin))
    {
        return false;
    }
    if (_out.bHasHp && _out.hp <= 0)
    {
        return Fail_(_where, "'hp' must be > 0");
    }
    if (_out.cullMargin < 0.f)
    {
        return Fail_(_where, "'cullMargin' must be >= 0");
    }

    if (_obj.HasField("ambush"))
    {
        std::optional<JsonReader> ambush = std::nullopt;
        if (!GetObject_(_obj, _where, "ambush", true, ambush))
        {
            return false;
        }
        const String ambushWhere = _where + " ambush";
        if (!CheckKeys_(*ambush, ambushWhere, { "xMin", "xMax" }) || !GetFloat_(*ambush, ambushWhere, "xMin", true, _out.ambush.xMin) || !GetFloat_(*ambush, ambushWhere, "xMax", true, _out.ambush.xMax))
        {
            return false;
        }
        if (_out.ambush.xMax < _out.ambush.xMin)
        {
            return Fail_(ambushWhere, "'xMax' must be >= 'xMin'");
        }
        _out.bHasAmbush = true;
    }

    if (!GetEnum_(_obj, _where, "layer", false, kLayerTable, _out.layer, &_out.bHasLayer))
    {
        return false;
    }

    const bool bCollider = _out.kind == eEntityKind::Collider;
    if (bCollider)
    {
        if (!GetVec2_(_obj, _where, "size", true, _out.size) || !GetEnum_(_obj, _where, "collisionLayer", true, kCollisionTable, _out.collisionLayer))
        {
            return false;
        }
        if (_out.size.x <= 0.f || _out.size.y <= 0.f)
        {
            return Fail_(_where, "'size' must be positive");
        }
    }
    else if (_obj.HasField("size") || _obj.HasField("collisionLayer"))
    {
        return Fail_(_where, "'size'/'collisionLayer' are only valid for kind 'collider'");
    }

    if (!GetString_(_obj, _where, "anim", false, _out.anim) || !GetString_(_obj, _where, "sprite", false, _out.sprite))
    {
        return false;
    }
    if (_out.kind == eEntityKind::Decor)
    {
        if (_out.def == "anim_decor" && _out.anim.empty())
        {
            return Fail_(_where, "decor 'anim_decor' needs 'anim'");
        }
        if (_out.def == "sprite_decor" && _out.sprite.empty())
        {
            return Fail_(_where, "decor 'sprite_decor' needs 'sprite'");
        }
        if (_out.anim.empty() && _out.sprite.empty())
        {
            return Fail_(_where, "decor needs 'anim' or 'sprite'");
        }
    }
    else if (!_out.anim.empty() || !_out.sprite.empty())
    {
        return Fail_(_where, "'anim'/'sprite' are only valid for kind 'decor'");
    }

    if (_obj.HasField("params"))
    {
        std::optional<JsonReader> params = std::nullopt;
        if (!GetObject_(_obj, _where, "params", true, params))
        {
            return false;
        }
        _out.params = ToParam_(*params);
        if (const ParamValue* pArena = _out.params.Find("arena"); pArena && pArena->IsString() && !m_out.FindRect(pArena->AsString()))
        {
            return Fail_(_where, std::format("params.arena references unknown rect '{}'", pArena->AsString()));
        }
    }
    return true;
}

bool Parser::ParseActionList_(
    const JsonReader&    _array,
    const String&        _where,
    Vector<StageAction>& _out)
{
    if (!_array.IsArray())
    {
        return Fail_(_where, "must be an array");
    }
    int index = 0;
    for (const JsonReader json: _array.GetArray())
    {
        StageAction action   = {};
        String      typeName = "?";
        if (json.IsObject() && json.HasField("type") && json.FindField("type")->IsString())
        {
            typeName = json.FindField("type")->Read<String>().GetValue();
        }
        const String where = std::format("{} action #{} ({})", _where, index++, typeName);
        if (!ParseAction_(json, where, action))
        {
            return false;
        }
        _out.push_back(std::move(action));
    }
    return true;
}

bool Parser::ParseAction_(
    const JsonReader& _obj,
    const String&     _where,
    StageAction&      _out)
{
    if (!ExpectObject_(_obj, _where))
    {
        return false;
    }
    if (!GetEnum_(_obj, _where, "type", true, kActionTable, _out.type))
    {
        return false;
    }

    if (!GetFloat_(_obj, _where, "delay", false, _out.delay))
    {
        return false;
    }
    if (_out.delay < 0.f)
    {
        return Fail_(_where, "'delay' must be >= 0");
    }

    bool bWhenExplicit = false;
    switch (_out.type)
    {
        case eActionType::Spawn:
            if (!ParseSpawn_(_obj, _where, eSpawnContext::Action, _out.spawn))
            {
                return false;
            }
            break;

        case eActionType::SpawnWave:
        {
            if (!CheckKeys_(_obj, _where, { "type", "delay", "when", "entries" }))
            {
                return false;
            }
            std::optional<JsonReader> entries = std::nullopt;
            if (!GetArray_(_obj, _where, "entries", true, entries))
            {
                return false;
            }
            if (entries->GetSize() == 0)
            {
                return Fail_(_where, "'entries' must not be empty");
            }
            int index = 0;
            for (const JsonReader json: entries->GetArray())
            {
                const String entryWhere = std::format("{} entries[{}]", _where, index++);
                WaveEntry    entry      = {};
                if (!ParseSpawn_(json, entryWhere, eSpawnContext::WaveEntry, entry.spawn))
                {
                    return false;
                }
                bool bHasDelay = false;
                if (!GetFloat_(json, entryWhere, "delay", true, entry.delay, &bHasDelay))
                {
                    return false;
                }
                if (entry.delay < 0.f)
                {
                    return Fail_(entryWhere, "'delay' must be >= 0");
                }
                if (!GetFloat_(json, entryWhere, "every", false, entry.every) || !GetInt_(json, entryWhere, "times", false, entry.times))
                {
                    return false;
                }
                if (entry.times < 1)
                {
                    return Fail_(entryWhere, "'times' must be >= 1");
                }
                if (entry.times > 1 && entry.every <= 0.f)
                {
                    return Fail_(entryWhere, "'every' must be > 0 when 'times' > 1");
                }
                _out.waveEntries.push_back(std::move(entry));
            }
            break;
        }

        case eActionType::SetBounds:
            if (!CheckKeys_(_obj, _where, { "type", "delay", "when", "rect" }) || !ParseRect_(_obj, _where, "rect", false, _out.rect))
            {
                return false;
            }
            break;

        case eActionType::LockCamera:
            if (!CheckKeys_(_obj, _where, { "type", "delay", "when", "rect", "glide", "permanent" }) || !ParseRect_(_obj, _where, "rect", true, _out.rect) || !GetFloat_(_obj, _where, "glide", false, _out.glide) || !GetBool_(_obj, _where, "permanent", false, _out.bPermanent))
            {
                return false;
            }
            if (_out.glide < 0.f)
            {
                return Fail_(_where, "'glide' must be >= 0");
            }
            break;

        case eActionType::UnlockCamera:
            if (!CheckKeys_(_obj, _where, { "type", "delay", "when" }))
            {
                return false;
            }
            break;

        case eActionType::CameraMove:
            if (!CheckKeys_(_obj, _where, { "type", "delay", "when", "to", "seconds" }) || !GetVec2_(_obj, _where, "to", true, _out.to) || !GetFloat_(_obj, _where, "seconds", true, _out.seconds))
            {
                return false;
            }
            if (_out.seconds < 0.f)
            {
                return Fail_(_where, "'seconds' must be >= 0");
            }
            break;

        case eActionType::SetCameraOffset:
            if (!CheckKeys_(_obj, _where, { "type", "delay", "when", "offset" }) || !GetVec2_(_obj, _where, "offset", true, _out.offset))
            {
                return false;
            }
            break;

        case eActionType::SetBackdrop:
            if (!CheckKeys_(_obj, _where, { "type", "delay", "when", "mode" }) || !GetString_(_obj, _where, "mode", true, _out.mode))
            {
                return false;
            }
            if (!m_out.FindBackdrop(_out.mode))
            {
                return Fail_(_where, std::format("unknown backdrop mode '{}'", _out.mode));
            }
            break;

        case eActionType::SetGround:
            if (!CheckKeys_(_obj, _where, { "type", "delay", "when", "name" }) || !GetString_(_obj, _where, "name", true, _out.name))
            {
                return false;
            }
            if (!m_out.HasGroundMap(_out.name))
            {
                return Fail_(_where, std::format("unknown ground map '{}'", _out.name));
            }
            break;

        case eActionType::Gate:
        {
            if (!CheckKeys_(_obj, _where, { "type", "delay", "when", "id", "waitFor", "until", "actions", "abortOnGameOver" }) || !GetString_(_obj, _where, "id", true, _out.gateId) || !GetEnum_(_obj, _where, "until", false, kUntilTable, _out.until) || !GetBool_(_obj, _where, "abortOnGameOver", false, _out.bAbortOnGameOver))
            {
                return false;
            }
            std::optional<JsonReader> waitFor = std::nullopt;
            std::optional<JsonReader> actions = std::nullopt;
            if (!GetArray_(_obj, _where, "waitFor", true, waitFor) || !GetArray_(_obj, _where, "actions", true, actions))
            {
                return false;
            }
            if (waitFor->GetSize() == 0)
            {
                return Fail_(_where, "'waitFor' must not be empty");
            }
            for (const JsonReader element: waitFor->GetArray())
            {
                if (!element.IsString())
                {
                    return Fail_(_where, "'waitFor' entries must be strings");
                }
                _out.waitFor.push_back(element.Read<String>().GetValue());
            }
            if (!ParseActionList_(*actions, std::format("{} (gate '{}') actions", _where, _out.gateId), _out.actions))
            {
                return false;
            }
            break;
        }

        case eActionType::Command:
            if (!CheckKeys_(_obj, _where, { "type", "delay", "when", "target", "command" }) || !GetString_(_obj, _where, "target", true, _out.target) || !GetString_(_obj, _where, "command", true, _out.command))
            {
                return false;
            }
            break;

        case eActionType::Clear:
        {
            if (!CheckKeys_(_obj, _where, { "type", "delay", "when", "waves", "layers" }) || !GetBool_(_obj, _where, "waves", false, _out.bClearWaves))
            {
                return false;
            }
            std::optional<JsonReader> layers = std::nullopt;
            if (!GetArray_(_obj, _where, "layers", false, layers))
            {
                return false;
            }
            if (layers)
            {
                for (const JsonReader element: layers->GetArray())
                {
                    eStageLayer layer = eStageLayer::Monster;
                    if (!element.IsString() || !LookupEnum_(kLayerTable, element.Read<String>().GetValue(), layer))
                    {
                        return Fail_(_where, std::format("'layers' entries must be one of {}", ListEnum_(kLayerTable)));
                    }
                    _out.clearLayers |= LayerBit(layer);
                }
            }
            if (!_out.bClearWaves && _out.clearLayers == 0)
            {
                return Fail_(_where, "clear needs 'waves': true and/or a non-empty 'layers'");
            }
            break;
        }

        case eActionType::Banner:
            if (!CheckKeys_(_obj, _where, { "type", "delay", "when", "text", "seconds", "sfx" }) || !GetString_(_obj, _where, "text", true, _out.text) || !GetFloat_(_obj, _where, "seconds", true, _out.seconds) || !GetString_(_obj, _where, "sfx", false, _out.name))
            {
                return false;
            }
            if (_out.seconds < 0.f)
            {
                return Fail_(_where, "'seconds' must be >= 0");
            }
            break;

        case eActionType::Fade:
            if (!CheckKeys_(_obj, _where, { "type", "delay", "when", "to", "seconds" }) || !GetEnum_(_obj, _where, "to", true, kFadeTable, _out.fadeTo) || !GetFloat_(_obj, _where, "seconds", true, _out.seconds))
            {
                return false;
            }
            if (_out.seconds < 0.f)
            {
                return Fail_(_where, "'seconds' must be >= 0");
            }
            break;

        case eActionType::PlayBgm:
            if (!CheckKeys_(_obj, _where, { "type", "delay", "when", "name", "loop", "fadeOutSec" }) || !GetString_(_obj, _where, "name", true, _out.name) || !GetBool_(_obj, _where, "loop", false, _out.bLoop) || !GetFloat_(_obj, _where, "fadeOutSec", false, _out.fadeOutSec))
            {
                return false;
            }
            if (_out.fadeOutSec < 0.f)
            {
                return Fail_(_where, "'fadeOutSec' must be >= 0");
            }
            break;

        case eActionType::PlayerInvincible:
            if (!CheckKeys_(_obj, _where, { "type", "delay", "when", "seconds" }) || !GetFloat_(_obj, _where, "seconds", true, _out.seconds))
            {
                return false;
            }
            if (_out.seconds < 0.f)
            {
                return Fail_(_where, "'seconds' must be >= 0");
            }
            break;

        case eActionType::EndStage:
            _out.next = "title";
            if (!CheckKeys_(_obj, _where, { "type", "delay", "when", "next" }) || !GetString_(_obj, _where, "next", false, _out.next))
            {
                return false;
            }
            break;
    }

    _out.when = DefaultWhen_(_out);
    if (!GetEnum_(_obj, _where, "when", false, kWhenTable, _out.when, &bWhenExplicit))
    {
        return false;
    }
    return true;
}

bool Parser::ParseSections_(
    const JsonReader& _root)
{
    std::optional<JsonReader> sections = std::nullopt;
    if (!GetArray_(_root, "root", "sections", true, sections))
    {
        return false;
    }

    int index = 0;
    for (const JsonReader json: sections->GetArray())
    {
        String id = "?";
        if (json.IsObject() && json.HasField("id") && json.FindField("id")->IsString())
        {
            id = json.FindField("id")->Read<String>().GetValue();
        }
        const String where = std::format("sections[{}] '{}'", index++, id);
        if (!CheckKeys_(json, where, { "id", "triggerX", "actions" }))
        {
            return false;
        }

        StageSection              section = {};
        std::optional<JsonReader> actions = std::nullopt;
        if (!GetString_(json, where, "id", true, section.id) || !GetFloat_(json, where, "triggerX", true, section.triggerX) || !GetArray_(json, where, "actions", true, actions))
        {
            return false;
        }
        if (section.id.empty())
        {
            return Fail_(where, "section 'id' must not be empty");
        }
        if (m_out.FindSection(section.id) >= 0)
        {
            return Fail_(where, "duplicate section id");
        }
        if (!m_out.sections.empty() && section.triggerX < m_out.sections.back().triggerX)
        {
            return Fail_(where, std::format("triggerX {} is smaller than the previous section's {} (sections must be ordered)", section.triggerX, m_out.sections.back().triggerX));
        }
        if (!ParseActionList_(*actions, where, section.actions))
        {
            return false;
        }
        m_out.sections.push_back(std::move(section));
    }
    if (m_out.sections.empty())
    {
        return Fail_("sections", "at least one section is required");
    }
    return true;
}

bool Parser::AddSpawnId_(
    const SpawnSpec& _spawn,
    const String&    _at)
{
    if (_spawn.id.empty())
    {
        return true;
    }
    for (const String& existing: m_spawnIds)
    {
        if (existing == _spawn.id)
        {
            return Fail_(_at, std::format("duplicate spawn id '{}'", _spawn.id));
        }
    }
    m_spawnIds.push_back(_spawn.id);
    return true;
}

bool Parser::HasSpawnId_(
    const String& _id) const
{
    for (const String& existing: m_spawnIds)
    {
        if (existing == _id)
        {
            return true;
        }
    }
    return false;
}

bool Parser::CollectIds_(
    const Vector<StageAction>& _actions,
    const String&              _where)
{
    int index = 0;
    for (const StageAction& action: _actions)
    {
        const String where = std::format("{} action #{} ({})", _where, index++, GetActionTypeName(action.type));

        if (action.type == eActionType::Spawn)
        {
            if (!AddSpawnId_(action.spawn, where))
            {
                return false;
            }
        }
        else if (action.type == eActionType::SpawnWave)
        {
            int entryIndex = 0;
            for (const WaveEntry& entry: action.waveEntries)
            {
                if (!AddSpawnId_(entry.spawn, std::format("{} entries[{}]", where, entryIndex++)))
                {
                    return false;
                }
            }
        }
        else if (action.type == eActionType::Gate)
        {
            for (const String& existing: m_gateIds)
            {
                if (existing == action.gateId)
                {
                    return Fail_(where, std::format("duplicate gate id '{}'", action.gateId));
                }
            }
            m_gateIds.push_back(action.gateId);
            if (!CollectIds_(action.actions, std::format("{} (gate '{}') actions", _where, action.gateId)))
            {
                return false;
            }
        }
    }
    return true;
}

bool Parser::CheckRefs_(
    const Vector<StageAction>& _actions,
    const String&              _where)
{
    int index = 0;
    for (const StageAction& action: _actions)
    {
        const String where = std::format("{} action #{} ({})", _where, index++, GetActionTypeName(action.type));
        if (action.type == eActionType::Gate)
        {
            for (const String& id: action.waitFor)
            {
                if (!HasSpawnId_(id))
                {
                    return Fail_(where, std::format("gate '{}' waitFor unknown spawn id '{}'", action.gateId, id));
                }
            }
            if (!CheckRefs_(action.actions, std::format("{} (gate '{}') actions", _where, action.gateId)))
            {
                return false;
            }
        }
        else if (action.type == eActionType::Command && !HasSpawnId_(action.target))
        {
            return Fail_(where, std::format("command target unknown spawn id '{}'", action.target));
        }
    }
    return true;
}

bool Parser::Validate_()
{
    for (const StageSection& section: m_out.sections)
    {
        if (!CollectIds_(section.actions, std::format("section '{}'", section.id)))
        {
            return false;
        }
    }
    for (const StageSection& section: m_out.sections)
    {
        if (!CheckRefs_(section.actions, std::format("section '{}'", section.id)))
        {
            return false;
        }
    }

    for (BoundsCurve& curve: m_out.camera.boundsCurves)
    {
        const String where = std::format("camera.boundsCurves '{}'", curve.id);
        curve.fromIndex    = m_out.FindSection(curve.from);
        curve.untilIndex   = m_out.FindSection(curve.until);
        if (curve.fromIndex < 0)
        {
            return Fail_(where, std::format("'from' references unknown section '{}'", curve.from));
        }
        if (curve.untilIndex < 0)
        {
            return Fail_(where, std::format("'until' references unknown section '{}'", curve.until));
        }
        if (curve.fromIndex >= curve.untilIndex)
        {
            return Fail_(where, "'from' section must come before the 'until' section");
        }
    }
    return true;
}

[[nodiscard]] static Result<StageData> MakeFailure_(
    StageLoadDiagnostics* const _pDiag,
    const String&               _message,
    const eError                _code)
{
    if (_pDiag)
    {
        _pDiag->message = _message;
    }
    return Error { _code };
}

Result<StageData> LoadStageFromJson(
    const StringView            _json,
    StageLoadDiagnostics* const _pDiag)
{
    if (_pDiag)
    {
        _pDiag->message.clear();
    }

    Result<JsonDeserializer> doc = JsonDeserializer::Load(_json, eJsonLoadOption::AllowBOM);
    if (!doc)
    {
        return MakeFailure_(_pDiag, "JSON syntax error", eError::Parse);
    }

    StageData data = {};
    Parser    parser { data };
    if (!parser.Parse(doc->GetReader()))
    {
        return MakeFailure_(_pDiag, parser.GetError(), eError::StageInvalid);
    }
    return data;
}

Result<StageData> LoadStageFromFile(
    const FilePath&             _path,
    StageLoadDiagnostics* const _pDiag)
{
    std::ifstream file { _path, std::ios::binary };
    if (!file)
    {
        return MakeFailure_(_pDiag, std::format("cannot open '{}'", _path.string()), eError::FileOpen);
    }
    std::ostringstream buffer;
    buffer << file.rdbuf();
    const String text = buffer.str();
    return LoadStageFromJson(text, _pDiag);
}

uint32_t LayerBit(
    const eStageLayer _layer)
{
    return 1u << static_cast<unsigned>(_layer);
}

bool ShouldRunAction(
    const eWhen _when,
    const bool  _bSilent)
{
    return _when == eWhen::Always || (_bSilent ? _when == eWhen::Silent : _when == eWhen::Normal);
}
