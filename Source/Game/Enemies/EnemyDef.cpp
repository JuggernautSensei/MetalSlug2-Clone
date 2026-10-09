#include "Game/Enemies/EnemyDef.h"
#include "Base/Types.h"
#include "Base/Math.h"

#include <cmath>
#include <format>

#include "Base/Log.h"
#include "Base/Json.h"

#include "Base/Error.h"
#include "Game/Enemies/EnemyDefParse.h"

namespace slug
{

// ===========================================
//  기본 유틸
// ===========================================

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
        case eEnemyBehavior::Bus: return "bus";
        case eEnemyBehavior::BattleShip: return "battleship";
        case eEnemyBehavior::Jeep: return "jeep";
        case eEnemyBehavior::BTank: return "btank";
        case eEnemyBehavior::MissleTank: return "missle_tank";
        case eEnemyBehavior::Sarubia: return "sarubia";
        case eEnemyBehavior::Aparch: return "aparch";
        case eEnemyBehavior::Count: break;
    }
    return "?";
}

std::optional<eEnemyBehavior> FindBehavior(
    const slug::StringView _name)
{
    for (int i = 0; i < static_cast<int>(eEnemyBehavior::Count); ++i)
    {
        const auto behavior = static_cast<eEnemyBehavior>(i);
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
    return static_cast<int>(std::lround(static_cast<double>(_sec) * 60.0));
}

int ClipDef::FrameAt(
    const int _ticks) const
{
    if (frames <= 1 || frameUnits <= 0)
    {
        return 0;
    }
    const int64_t index = (static_cast<int64_t>(_ticks < 0 ? 0 : _ticks) * kAnimUnitsPerTick) / frameUnits;
    if (bLoop)
    {
        return static_cast<int>(index % frames);
    }
    return static_cast<int>(index >= frames ? frames - 1 : index);
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
    const slug::StringView _logicalName) const
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
    const slug::StringView _name) const
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

// ===========================================
//  DefReader
// ===========================================

DefReader::DefReader(
    const slug::StringView    _defName,
    EnemyDef&                _def,
    slug::Vector<ParamEntry>& _params,
    slug::String&             _error)
    : m_defName(_defName)
    , m_def(_def)
    , m_params(_params)
    , m_error(_error)
{
}

void DefReader::Fail(
    const slug::StringView _field,
    const slug::StringView _message)
{
    if (m_error.empty())
    {
        m_error = std::format("{}: {}: {}", m_defName, _field, _message);
    }
}

ParamEntry* DefReader::Find_(
    const slug::StringView _key)
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
    const slug::StringView _key) const
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
    const slug::StringView _key)
{
    ParamEntry* pEntry = Find_(_key);
    if (!pEntry)
    {
        Fail(std::format("params.{}", _key), "필수 값이 없습니다");
        return 0.f;
    }
    const slug::Result<double> value = pEntry->value.Read<double>();
    if (!value)
    {
        Fail(std::format("params.{}", _key), "숫자가 아닙니다");
        return 0.f;
    }
    return static_cast<float>(value.GetValue());
}

float DefReader::Num(
    const slug::StringView _key,
    const float           _default)
{
    return Has(_key) ? Num(_key) : _default;
}

int DefReader::Int(
    const slug::StringView _key)
{
    return static_cast<int>(std::lround(static_cast<double>(Num(_key))));
}

int DefReader::Int(
    const slug::StringView _key,
    const int             _default)
{
    return Has(_key) ? Int(_key) : _default;
}

int DefReader::Ticks(
    const slug::StringView _key)
{
    return SecToTicks(Num(_key));
}

int DefReader::Ticks(
    const slug::StringView _key,
    const float           _defaultSec)
{
    return SecToTicks(Num(_key, _defaultSec));
}

bool DefReader::Bool(
    const slug::StringView _key,
    const bool            _default)
{
    ParamEntry* pEntry = Find_(_key);
    if (!pEntry)
    {
        return _default;
    }
    const slug::Result<bool> value = pEntry->value.Read<bool>();
    if (!value)
    {
        Fail(std::format("params.{}", _key), "true/false 가 아닙니다");
        return _default;
    }
    return value.GetValue();
}

slug::String DefReader::Str(
    const slug::StringView _key)
{
    ParamEntry* pEntry = Find_(_key);
    if (!pEntry)
    {
        Fail(std::format("params.{}", _key), "필수 값이 없습니다");
        return {};
    }
    slug::Result<slug::String> value = pEntry->value.Read<slug::String>();
    if (!value)
    {
        Fail(std::format("params.{}", _key), "문자열이 아닙니다");
        return {};
    }
    return value.GetValue();
}

Vec2 DefReader::ReadVec2(
    const slug::StringView _key,
    const Vec2    _default)
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
    float values[2] = {};
    int   i         = 0;
    for (const slug::JsonReader item: pEntry->value.GetArray())
    {
        const slug::Result<double> value = item.Read<double>();
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
    const slug::StringView _key)
{
    BoxDef      box;
    ParamEntry* pEntry = Find_(_key);
    if (!pEntry || !pEntry->value.IsObject())
    {
        Fail(std::format("params.{}", _key), "{w,h,cx,cy} 객체가 필요합니다");
        return box;
    }
    const auto Read = [&](const char* _name, float& _out, const bool _bRequired) {
        if (!pEntry->value.HasField(_name))
        {
            if (_bRequired)
            {
                Fail(std::format("params.{}.{}", _key, _name), "필수 값이 없습니다");
            }
            return;
        }
        const slug::Result<double> value = pEntry->value.ReadField<double>(_name);
        if (!value)
        {
            Fail(std::format("params.{}.{}", _key, _name), "숫자가 아닙니다");
            return;
        }
        _out = static_cast<float>(value.GetValue());
    };
    Read("w", box.w, true);
    Read("h", box.h, true);
    Read("cx", box.cx, false);
    Read("cy", box.cy, false);
    return box;
}

bool DefReader::NumArray(
    const slug::StringView _key,
    float*                _pOut,
    const int             _count)
{
    ParamEntry* pEntry = Find_(_key);
    if (!pEntry)
    {
        return false;
    }
    if (!pEntry->value.IsArray() || static_cast<int>(pEntry->value.GetSize()) != _count)
    {
        Fail(std::format("params.{}", _key), std::format("길이 {} 의 숫자 배열이 필요합니다", _count));
        return false;
    }
    int i = 0;
    for (const slug::JsonReader item: pEntry->value.GetArray())
    {
        const slug::Result<double> value = item.Read<double>();
        if (!value)
        {
            Fail(std::format("params.{}", _key), "숫자가 아닌 원소가 있습니다");
            return false;
        }
        _pOut[i++] = static_cast<float>(value.GetValue());
    }
    return true;
}

int DefReader::Clip(
    const slug::StringView _logical)
{
    const int index = m_def.FindClip(_logical);
    if (index < 0)
    {
        Fail(std::format("clips.{}", _logical), "필수 클립이 정의되지 않았습니다");
    }
    return index;
}

int DefReader::ClipOpt(
    const slug::StringView _logical) const
{
    return m_def.FindClip(_logical);
}

int DefReader::Weapon(
    const slug::StringView _name)
{
    const int index = m_def.FindWeapon(_name);
    if (index < 0)
    {
        Fail(std::format("weapons.{}", _name), "필수 무기가 정의되지 않았습니다");
    }
    return index;
}

int DefReader::WeaponOpt(
    const slug::StringView _name) const
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

}   // namespace slug
