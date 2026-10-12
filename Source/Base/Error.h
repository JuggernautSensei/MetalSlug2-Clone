#pragma once
#include <concepts>

#include "Base/Types.h"

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
    Error(const eError _code);

    [[nodiscard]] bool   IsOK() const;
    [[nodiscard]] bool   IsError() const;
    [[nodiscard]] eError GetCode() const;
    [[nodiscard]] String           MakeMessage() const;

    explicit operator bool() const;
    [[nodiscard]] constexpr bool operator==(const Error& _other) const = default;

private:
    eError m_code = eError::Ok;
};
