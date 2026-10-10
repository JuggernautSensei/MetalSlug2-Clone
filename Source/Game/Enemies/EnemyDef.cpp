#include "Game/Enemies/EnemyDef.h"

#include <cmath>
#include <format>

#include "Base/Error.h"
#include "Base/Json.h"
#include "Base/Log.h"
#include "Base/Math.h"
#include "Base/Types.h"
#include "Game/Enemies/EnemyDefParse.h"

namespace slug
{

const char* GetBehaviorName(
    const eEnemyBehavior _behavior)
{
    switch (_behavior)
    {
        case eEnemyBehavior::Soldier: return "soldier";
        case eEnemyBehavior::Bzk: return "bzk";
        case eEnemyBehavior::Hang: return "hang";
        case eEnemyBehavior::FlyingBzk: return "flying_bzk";
        case eEnemyBehavior::Shielder: return "shielder";
        case eEnemyBehavior::Pow: return "pow";
        case eEnemyBehavior::Jeep: return "jeep";
        case eEnemyBehavior::BTank: return "btank";
        case eEnemyBehavior::MissleTank: return "missle_tank";
        case eEnemyBehavior::Sarubia: return "sarubia";
        case eEnemyBehavior::Count: break;
    }
    return "?";
}

std::optional<eEnemyBehavior> FindBehavior(
    const StringView _name)
{
    for (int i = 0; i < static_cast<int>(eEnemyBehavior::Count); ++i)
    {
        const eEnemyBehavior behavior = static_cast<eEnemyBehavior>(i);
        if (_name == GetBehaviorName(behavior))
        {
            return behavior;
        }
    }
    return std::nullopt;
}

int SecToTicks(
    const float _sec)
{
    return static_cast<int>(std::lround(static_cast<double>(_sec) * 60.));
}

int ClipDef::TickOfFrame(
    const int _frame) const
{
    return static_cast<int>((static_cast<int64_t>(_frame) * frameUnits + kAnimUnitsPerTick - 1) / kAnimUnitsPerTick);
}

int ClipDef::GetDurationTicks() const
{
    return TickOfFrame(frames);
}

int EnemyDef::FindClip(
    const StringView _logicalName) const
{
    for (size_t i = 0; i < clips.size(); ++i)
    {
        if (clips[i].name == _logicalName)
        {
            return static_cast<int>(i);
        }
    }
    return -1;
}

int EnemyDef::FindWeapon(
    const StringView _name) const
{
    for (size_t i = 0; i < weapons.size(); ++i)
    {
        if (weapons[i].name == _name)
        {
            return static_cast<int>(i);
        }
    }
    return -1;
}

ParamEntry::ParamEntry(
    String           _key,
    const JsonReader _value)
    : key(std::move(_key))
    , value(_value)
{
}

DefReader::DefReader(
    const StringView    _defName,
    EnemyDef&           _def,
    Vector<ParamEntry>& _params,
    String&             _error)
    : m_defName(_defName)
    , m_def(_def)
    , m_params(_params)
    , m_error(_error)
{
}

void DefReader::Fail(
    const StringView _field,
    const StringView _message)
{
    if (m_error.empty())
    {
        m_error = std::format("{}: {}: {}", m_defName, _field, _message);
    }
}

ParamEntry* DefReader::Find_(
    const StringView _key)
{
    for (ParamEntry& entry: m_params)
    {
        if (entry.key == _key)
        {
            entry.bUsed = true;
            return &entry;
        }
    }
    return nullptr;
}

bool DefReader::Has(
    const StringView _key) const
{
    for (const ParamEntry& entry: m_params)
    {
        if (entry.key == _key)
        {
            return true;
        }
    }
    return false;
}

float DefReader::Num(
    const StringView _key)
{
    ParamEntry* pEntry = Find_(_key);
    if (!pEntry)
    {
        Fail(std::format("params.{}", _key), "필수 값이 없습니다");
        return 0.f;
    }
    const Result<double> value = pEntry->value.Read<double>();
    if (!value)
    {
        Fail(std::format("params.{}", _key), "숫자가 아닙니다");
        return 0.f;
    }
    return static_cast<float>(value.GetValue());
}

float DefReader::Num(
    const StringView _key,
    const float      _default)
{
    return Has(_key) ? Num(_key) : _default;
}

int DefReader::Int(
    const StringView _key)
{
    return static_cast<int>(std::lround(static_cast<double>(Num(_key))));
}

int DefReader::Int(
    const StringView _key,
    const int        _default)
{
    return Has(_key) ? Int(_key) : _default;
}

int DefReader::Ticks(
    const StringView _key)
{
    return SecToTicks(Num(_key));
}

int DefReader::Ticks(
    const StringView _key,
    const float      _defaultSec)
{
    return SecToTicks(Num(_key, _defaultSec));
}

bool DefReader::Bool(
    const StringView _key,
    const bool       _bDefault)
{
    ParamEntry* pEntry = Find_(_key);
    if (!pEntry)
    {
        return _bDefault;
    }
    const Result<bool> value = pEntry->value.Read<bool>();
    if (!value)
    {
        Fail(std::format("params.{}", _key), "true/false 가 아닙니다");
        return _bDefault;
    }
    return value.GetValue();
}

String DefReader::Str(
    const StringView _key)
{
    ParamEntry* pEntry = Find_(_key);
    if (!pEntry)
    {
        Fail(std::format("params.{}", _key), "필수 값이 없습니다");
        return {};
    }
    Result<String> value = pEntry->value.Read<String>();
    if (!value)
    {
        Fail(std::format("params.{}", _key), "문자열이 아닙니다");
        return {};
    }
    return value.GetValue();
}

Vec2 DefReader::ReadVec2(
    const StringView _key,
    const Vec2       _default)
{
    ParamEntry* pEntry = Find_(_key);
    if (!pEntry)
    {
        return _default;
    }
    if (!pEntry->value.IsArray() || pEntry->value.GetSize() != 2)
    {
        Fail(std::format("params.{}", _key), "[x, y] 배열이 아닙니다");
        return _default;
    }
    ARRAY<float, 2> values = {};
    int             i      = 0;
    for (const JsonReader item: pEntry->value.GetArray())
    {
        const Result<double> value = item.Read<double>();
        if (!value)
        {
            Fail(std::format("params.{}", _key), "숫자가 아닌 원소가 있습니다");
            return _default;
        }
        values[i++] = static_cast<float>(value.GetValue());
    }
    return { values[0], values[1] };
}

BoxDef DefReader::Box(
    const StringView _key)
{
    BoxDef      box;
    ParamEntry* pEntry = Find_(_key);
    if (!pEntry || !pEntry->value.IsObject())
    {
        Fail(std::format("params.{}", _key), "{w,h,cx,cy} 객체가 필요합니다");
        return box;
    }
    ReadBoxField_(*pEntry, _key, "w", box.w, true);
    ReadBoxField_(*pEntry, _key, "h", box.h, true);
    ReadBoxField_(*pEntry, _key, "cx", box.cx, false);
    ReadBoxField_(*pEntry, _key, "cy", box.cy, false);
    return box;
}

void DefReader::ReadBoxField_(
    const ParamEntry& _entry,
    const StringView  _key,
    const StringView  _name,
    float&            _out,
    const bool        _bRequired)
{
    if (!_entry.value.HasField(_name))
    {
        if (_bRequired)
        {
            Fail(std::format("params.{}.{}", _key, _name), "필수 값이 없습니다");
        }
        return;
    }
    const Result<double> value = _entry.value.ReadField<double>(_name);
    if (!value)
    {
        Fail(std::format("params.{}.{}", _key, _name), "숫자가 아닙니다");
        return;
    }
    _out = static_cast<float>(value.GetValue());
}

bool DefReader::NumArray(
    const StringView  _key,
    const Span<float> _out)
{
    ParamEntry* pEntry = Find_(_key);
    if (!pEntry)
    {
        return false;
    }
    const int count = static_cast<int>(_out.size());
    if (!pEntry->value.IsArray() || static_cast<int>(pEntry->value.GetSize()) != count)
    {
        Fail(std::format("params.{}", _key), std::format("길이 {} 의 숫자 배열이 필요합니다", count));
        return false;
    }

    int i = 0;
    for (const JsonReader item: pEntry->value.GetArray())
    {
        const Result<double> value = item.Read<double>();
        if (!value)
        {
            Fail(std::format("params.{}", _key), "숫자가 아닌 원소가 있습니다");
            return false;
        }
        _out[static_cast<size_t>(i++)] = static_cast<float>(value.GetValue());
    }
    return true;
}

int DefReader::Clip(
    const StringView _logical)
{
    const int index = m_def.FindClip(_logical);
    if (index < 0)
    {
        Fail(std::format("clips.{}", _logical), "필수 클립이 정의되지 않았습니다");
    }
    return index;
}

int DefReader::ClipOpt(
    const StringView _logical) const
{
    return m_def.FindClip(_logical);
}

int DefReader::Weapon(
    const StringView _name)
{
    const int index = m_def.FindWeapon(_name);
    if (index < 0)
    {
        Fail(std::format("weapons.{}", _name), "필수 무기가 정의되지 않았습니다");
    }
    return index;
}

int DefReader::WeaponOpt(
    const StringView _name) const
{
    return m_def.FindWeapon(_name);
}

bool DefReader::Finish()
{
    for (const ParamEntry& entry: m_params)
    {
        if (!entry.bUsed)
        {
            Fail(std::format("params.{}", entry.key), "알 수 없는 키입니다 (이 행동이 쓰지 않는 값)");
        }
    }
    return m_error.empty();
}

bool DefReader::HasError() const
{
    return !m_error.empty();
}

const EnemyDef& EnemyDefLibrary::Get(
    const int _index) const
{
    return m_defs[static_cast<size_t>(_index)];
}

const ProjectileDef& EnemyDefLibrary::GetProjectile(
    const int _index) const
{
    return m_projectiles[static_cast<size_t>(_index)];
}

size_t EnemyDefLibrary::GetDefCount() const
{
    return m_defs.size();
}

size_t EnemyDefLibrary::GetProjectileCount() const
{
    return m_projectiles.size();
}

Span<const EnemyDef> EnemyDefLibrary::GetDefs() const
{
    return m_defs;
}

Span<const ProjectileDef> EnemyDefLibrary::GetProjectiles() const
{
    return m_projectiles;
}

const EnemyGlobals& EnemyDefLibrary::GetGlobals() const
{
    return m_globals;
}

}   // namespace slug
