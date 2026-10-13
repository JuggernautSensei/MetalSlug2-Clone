#pragma once
#include <JugX/JsonDeserializer.h>

#include "Base/Math.h"
#include "Base/SlugError.h"
#include "Base/Types.h"

using jug::eJsonLoadOption;
using jug::JsonDeserializer;
using jug::JsonReader;

inline constexpr jug::Flags<eJsonLoadOption> kJsonBom         = eJsonLoadOption::AllowBOM;
inline constexpr jug::Flags<eJsonLoadOption> kJsonBomComments = { eJsonLoadOption::AllowBOM, eJsonLoadOption::AllowComments };

[[nodiscard]] Result<JsonDeserializer> LoadJsonFile(const FilePath& _path, jug::Flags<eJsonLoadOption> _flags = kJsonBom);

// JsonReader::HasField/FindField 는 비객체/없는 키에서 assert/ERROR 로그라서 선택 필드는 이쪽으로 조회한다
[[nodiscard]] bool               HasKey(const JsonReader& _object, StringView _key);
[[nodiscard]] Result<JsonReader> TryField(const JsonReader& _object, StringView _key);

template<typename T>
[[nodiscard]] Result<T> TryRead(
    const JsonReader& _object,
    const StringView  _key)
{
    const Result<JsonReader> field = TryField(_object, _key);
    if (!field)
    {
        return field.GetError();
    }
    return field->Read<T>();
}

// 반올림한 정수 (필수 필드)
[[nodiscard]] bool ReadInt(const JsonReader& _object, StringView _key, int& _out);

// 정의 파서 공용 필드 읽기: 필드가 없으면 (필수가 아닐 때) true 이고 _out/_pFound 는 그대로(false), 틀리면 false 와 _reason
[[nodiscard]] bool ReadNumberField(const JsonReader& _object, StringView _key, bool _bRequired, double& _out, StringView& _reason, bool* _pFound = nullptr);
[[nodiscard]] bool ReadFloatField(const JsonReader& _object, StringView _key, bool _bRequired, float& _out, StringView& _reason);
[[nodiscard]] bool ReadBoolField(const JsonReader& _object, StringView _key, bool _bRequired, bool& _out, StringView& _reason, bool* _pFound = nullptr);
[[nodiscard]] bool ReadStringField(const JsonReader& _object, StringView _key, bool _bRequired, String& _out, StringView& _reason, bool* _pFound = nullptr);
[[nodiscard]] bool ReadVec2Field(const JsonReader& _object, StringView _key, bool _bRequired, Vec2& _out, StringView& _reason, bool* _pFound = nullptr);

// "note" 는 항상 허용. 허용 밖의 키가 있으면 false 와 그 키 (_object 는 객체여야 한다)
[[nodiscard]] bool CheckJsonKeys(const JsonReader& _object, Span<const StringView> _allowed, StringView& _badKey);
