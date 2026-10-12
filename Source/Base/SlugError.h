#pragma once

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

class SlugErrorCategory final : public jug::IErrorCategory
{
public:
    [[nodiscard]] StringView GetName() const noexcept override;
    [[nodiscard]] String     MakeMessage(int _err) const override;
};

JUG_DEFINE_ERROR_ENUM(eError, SlugErrorCategory);

[[nodiscard]] bool HasErrorCode(const Error& _error, eError _code);
