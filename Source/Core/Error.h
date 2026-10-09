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
    InvalidData,
    EnemyDef,      // 적 정의 오류 (Enemies 에이전트가 사용; Error.cpp 에 메시지가 이미 있어 선언 누락을 보정)
    StageInvalid,  // 스테이지 스크립트(Stage*.json) 검증 실패 (상세 메시지는 StageLoadDiagnostics)
    PropDefInvalid // 소품/아이템 정의 오류 (Props.json, Props 에이전트)
};

class ErrorCategory final : public jug::IErrorCategory
{
public:
    [[nodiscard]] jug::StringView GetName() const noexcept override;
    [[nodiscard]] jug::String     MakeMessage(int _err) const override;
};

}   // namespace slug

JUG_DEFINE_ERROR_ENUM(slug::eError, slug::ErrorCategory);
