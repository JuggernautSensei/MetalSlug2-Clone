#pragma once
#include <JugX/Error.h>

namespace slug
{

// ===========================================
//  게임 전역 에러 코드
// ===========================================

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
    InvalidData
};

class ErrorCategory final : public jug::IErrorCategory
{
public:
    [[nodiscard]] jug::StringView GetName() const noexcept override;
    [[nodiscard]] jug::String     MakeMessage(int _err) const override;
};

}   // namespace slug

JUG_DEFINE_ERROR_ENUM(slug::eError, slug::ErrorCategory);
