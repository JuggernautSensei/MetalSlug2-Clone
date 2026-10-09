#pragma once
#include <cstdint>
#include <type_traits>
#include <utility>

#include "Base/Result.h"
#include "Base/Types.h"

struct yyjson_doc;
struct yyjson_val;

namespace slug
{

enum class eJsonLoadOption : uint32_t
{
    None          = 0,
    AllowComments = 1u << 0,
    AllowBOM      = 1u << 1
};

class JsonDeserializer;

class JsonReader
{
    friend class JsonDeserializer;

public:
    template<typename T>
    [[nodiscard]] Result<T> Read() const
    {
        if constexpr (std::is_same_v<T, bool>)
        {
            return ReadBool_();
        }
        else if constexpr (std::is_same_v<T, String>)
        {
            return ReadString_();
        }
        else if constexpr (std::is_floating_point_v<T>)
        {
            const Result<double> value = ReadDouble_();
            if (!value)
            {
                return value.GetError();
            }
            return static_cast<T>(value.GetValue());
        }
        else if constexpr (std::is_signed_v<T>)
        {
            const Result<int64_t> value = ReadInt_();
            if (!value)
            {
                return value.GetError();
            }
            return static_cast<T>(value.GetValue());
        }
        else
        {
            static_assert(std::is_unsigned_v<T>, "Unsupported JSON read type.");
            const Result<uint64_t> value = ReadUInt_();
            if (!value)
            {
                return value.GetError();
            }
            return static_cast<T>(value.GetValue());
        }
    }

    template<typename T>
    [[nodiscard]] Result<T> ReadField(const StringView _key) const
    {
        const Result<JsonReader> field = FindField(_key);
        if (!field)
        {
            return field.GetError();
        }
        return field->Read<T>();
    }

    [[nodiscard]] Result<JsonReader>                      FindField(StringView _key) const;
    [[nodiscard]] bool                                    HasField(StringView _key) const;
    [[nodiscard]] Vector<JsonReader>                      GetArray() const;
    [[nodiscard]] Vector<std::pair<StringView, JsonReader>> GetObject() const;

    [[nodiscard]] size_t GetSize() const;

        [[nodiscard]] bool IsNull() const;
    [[nodiscard]] bool IsBool() const;
    [[nodiscard]] bool IsNumber() const;
    [[nodiscard]] bool IsString() const;
    [[nodiscard]] bool IsArray() const;
    [[nodiscard]] bool IsObject() const;

private:
    explicit JsonReader(yyjson_val* _pValue);

    [[nodiscard]] Result<bool>     ReadBool_() const;
    [[nodiscard]] Result<int64_t>  ReadInt_() const;
    [[nodiscard]] Result<uint64_t> ReadUInt_() const;
    [[nodiscard]] Result<double>   ReadDouble_() const;
    [[nodiscard]] Result<String>   ReadString_() const;

    yyjson_val* m_pValue = nullptr;
};

class JsonDeserializer
{
public:
    [[nodiscard]] static Result<JsonDeserializer> Load(StringView _json, eJsonLoadOption _option = eJsonLoadOption::None);
    [[nodiscard]] static Result<JsonDeserializer> LoadFromFile(const FilePath& _path, eJsonLoadOption _option = eJsonLoadOption::None);

    JsonDeserializer(const JsonDeserializer&)            = delete;
    JsonDeserializer& operator=(const JsonDeserializer&) = delete;
    JsonDeserializer(JsonDeserializer&& _other) noexcept;
    JsonDeserializer& operator=(JsonDeserializer&& _other) noexcept;
    ~JsonDeserializer();

    [[nodiscard]] JsonReader GetReader() const;

private:
    explicit JsonDeserializer(yyjson_doc* _pDoc);

    yyjson_doc* m_pDoc = nullptr;
};
}
