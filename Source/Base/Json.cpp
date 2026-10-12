#include "Base/Json.h"

#include <fstream>
#include <iterator>
#include <yyjson.h>

#include "Base/PhaseTimer.h"

namespace
{
[[nodiscard]] yyjson_read_flag MakeReadFlags_(
    const eJsonLoadOption _option)
{
    yyjson_read_flag flags = YYJSON_READ_ALLOW_BOM;
    if ((static_cast<uint32_t>(_option) & static_cast<uint32_t>(eJsonLoadOption::AllowComments)) != 0u)
    {
        flags |= YYJSON_READ_ALLOW_COMMENTS;
    }
    return flags;
}
}   // namespace

JsonReader::JsonReader(
    yyjson_val* _pValue)
    : m_pValue(_pValue)
{
}

Result<JsonReader> JsonReader::FindField(
    const StringView _key) const
{
    if (!m_pValue || !yyjson_is_obj(m_pValue))
    {
        return Error { eError::InvalidData };
    }
    yyjson_val* pField = yyjson_obj_getn(m_pValue, _key.data(), _key.size());
    if (!pField)
    {
        return Error { eError::NotFound };
    }
    return JsonReader { pField };
}

bool JsonReader::HasField(
    const StringView _key) const
{
    return m_pValue && yyjson_is_obj(m_pValue) && yyjson_obj_getn(m_pValue, _key.data(), _key.size()) != nullptr;
}

Vector<JsonReader> JsonReader::GetArray() const
{
    Vector<JsonReader> items;
    if (!m_pValue || !yyjson_is_arr(m_pValue))
    {
        return items;
    }
    items.reserve(yyjson_arr_size(m_pValue));
    yyjson_arr_iter iter = yyjson_arr_iter_with(m_pValue);
    while (yyjson_val* pItem = yyjson_arr_iter_next(&iter))
    {
        items.push_back(JsonReader { pItem });
    }
    return items;
}

Vector<std::pair<StringView, JsonReader>> JsonReader::GetObject() const
{
    Vector<std::pair<StringView, JsonReader>> items;
    if (!m_pValue || !yyjson_is_obj(m_pValue))
    {
        return items;
    }
    items.reserve(yyjson_obj_size(m_pValue));
    yyjson_obj_iter iter = yyjson_obj_iter_with(m_pValue);
    while (yyjson_val* pKey = yyjson_obj_iter_next(&iter))
    {
        yyjson_val* pVal = yyjson_obj_iter_get_val(pKey);
        items.emplace_back(StringView { yyjson_get_str(pKey), yyjson_get_len(pKey) }, JsonReader { pVal });
    }
    return items;
}

size_t JsonReader::GetSize() const
{
    if (IsArray())
    {
        return yyjson_arr_size(m_pValue);
    }
    if (IsObject())
    {
        return yyjson_obj_size(m_pValue);
    }
    return 0;
}

bool JsonReader::IsNull() const
{
    return m_pValue && yyjson_is_null(m_pValue);
}

bool JsonReader::IsBool() const
{
    return m_pValue && yyjson_is_bool(m_pValue);
}

bool JsonReader::IsNumber() const
{
    return m_pValue && yyjson_is_num(m_pValue);
}

bool JsonReader::IsString() const
{
    return m_pValue && yyjson_is_str(m_pValue);
}

bool JsonReader::IsArray() const
{
    return m_pValue && yyjson_is_arr(m_pValue);
}

bool JsonReader::IsObject() const
{
    return m_pValue && yyjson_is_obj(m_pValue);
}

Result<bool> JsonReader::ReadBool_() const
{
    if (!IsBool())
    {
        return Error { eError::InvalidData };
    }
    return yyjson_get_bool(m_pValue);
}

Result<int64_t> JsonReader::ReadInt_() const
{
    if (m_pValue && yyjson_is_sint(m_pValue))
    {
        return yyjson_get_sint(m_pValue);
    }
    if (m_pValue && yyjson_is_uint(m_pValue) && yyjson_get_uint(m_pValue) <= static_cast<uint64_t>(INT64_MAX))
    {
        return static_cast<int64_t>(yyjson_get_uint(m_pValue));
    }
    return Error { eError::InvalidData };
}

Result<uint64_t> JsonReader::ReadUInt_() const
{
    if (m_pValue && yyjson_is_uint(m_pValue))
    {
        return yyjson_get_uint(m_pValue);
    }
    if (m_pValue && yyjson_is_sint(m_pValue) && yyjson_get_sint(m_pValue) >= 0)
    {
        return static_cast<uint64_t>(yyjson_get_sint(m_pValue));
    }
    return Error { eError::InvalidData };
}

Result<double> JsonReader::ReadDouble_() const
{
    if (!IsNumber())
    {
        return Error { eError::InvalidData };
    }
    return yyjson_get_num(m_pValue);
}

Result<String> JsonReader::ReadString_() const
{
    if (!IsString())
    {
        return Error { eError::InvalidData };
    }
    return String { yyjson_get_str(m_pValue), yyjson_get_len(m_pValue) };
}

Result<JsonDeserializer> JsonDeserializer::Load(
    const StringView      _json,
    const eJsonLoadOption _option)
{
    yyjson_doc* pDoc = yyjson_read_opts(const_cast<char*>(_json.data()), _json.size(), MakeReadFlags_(_option), nullptr, nullptr);
    if (!pDoc)
    {
        return Error { eError::Parse };
    }
    return JsonDeserializer { pDoc };
}

Result<JsonDeserializer> JsonDeserializer::LoadFromFile(
    const FilePath&       _path,
    const eJsonLoadOption _option)
{
    const ScopedPhaseTimer timer { "json", _path.filename().string() };

    std::ifstream file(_path, std::ios::binary);
    if (!file)
    {
        return Error { eError::FileOpen };
    }
    const String text { std::istreambuf_iterator<char>(file), std::istreambuf_iterator<char>() };
    return Load(text, _option);
}

JsonDeserializer::JsonDeserializer(
    yyjson_doc* _pDoc)
    : m_pDoc(_pDoc)
{
}

JsonDeserializer::JsonDeserializer(
    JsonDeserializer&& _other) noexcept
    : m_pDoc(_other.m_pDoc)
{
    _other.m_pDoc = nullptr;
}

JsonDeserializer& JsonDeserializer::operator=(
    JsonDeserializer&& _other) noexcept
{
    if (this != &_other)
    {
        if (m_pDoc)
        {
            yyjson_doc_free(m_pDoc);
        }
        m_pDoc        = _other.m_pDoc;
        _other.m_pDoc = nullptr;
    }
    return *this;
}

JsonDeserializer::~JsonDeserializer()
{
    if (m_pDoc)
    {
        yyjson_doc_free(m_pDoc);
    }
}

JsonReader JsonDeserializer::GetReader() const
{
    return JsonReader { m_pDoc ? yyjson_doc_get_root(m_pDoc) : nullptr };
}
