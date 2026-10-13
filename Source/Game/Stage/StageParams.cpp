#include "Game/Stage/StageParams.h"

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
    JUG_ASSERT(
        m_type == eType::Array,
        "PushBack requires an array value");
    m_items.push_back(std::move(_value));
}

void ParamValue::Set(
    String     _key,
    ParamValue _value)
{
    JUG_ASSERT(
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

VECTOR2 ParamValue::GetVec2(
    const StringView _key,
    const VECTOR2    _default) const
{
    const ParamValue* pValue = Find(_key);
    VECTOR2           out;
    return pValue && pValue->TryGetVec2(out) ? out : _default;
}

bool ParamValue::TryGetVec2(
    VECTOR2& _out) const
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
