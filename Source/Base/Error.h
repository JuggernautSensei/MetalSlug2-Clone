#pragma once
#include <concepts>

#include "Base/Types.h"

namespace slug
{

enum class eError
{
    Ok = 0,
    SdlInit,
    WindowCreate,
    RendererCreate,
    TextureCreate,
    FileOpen,
    FileRead,
    Parse,
    NotFound,
    InvalidData,
    EnemyDef,
    StageInvalid,
    PropDefInvalid
};

inline constexpr eError kOK = eError::Ok;

class Error
{
public:
    constexpr Error() = default;
    constexpr Error(const eError _code)
        : m_code(_code)
    {
    }

    [[nodiscard]] constexpr bool IsOK() const { return m_code == eError::Ok; }
    [[nodiscard]] constexpr bool IsError() const { return m_code != eError::Ok; }
    [[nodiscard]] constexpr eError GetCode() const { return m_code; }
    [[nodiscard]] String MakeMessage() const;

    explicit constexpr operator bool() const { return IsError(); }
    [[nodiscard]] constexpr bool operator==(const Error& _other) const = default;

private:
    eError m_code = eError::Ok;
};

}   // namespace slug
