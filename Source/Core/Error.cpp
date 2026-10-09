#include "Core/Error.h"

namespace slug
{

jug::StringView ErrorCategory::GetName() const noexcept
{
    return "slug";
}

jug::String ErrorCategory::MakeMessage(
    const int _err) const
{
    switch (static_cast<eError>(_err))
    {
        case eError::Ok: return "ok";
        case eError::SdlInit: return "SDL 초기화 실패";
        case eError::WindowCreate: return "윈도우 생성 실패";
        case eError::RendererCreate: return "렌더러 생성 실패";
        case eError::TextureCreate: return "텍스처 생성 실패";
        case eError::FileOpen: return "파일 열기 실패";
        case eError::FileRead: return "파일 읽기 실패";
        case eError::Parse: return "파싱 실패";
        case eError::NotFound: return "찾을 수 없음";
        case eError::InvalidData: return "잘못된 데이터";
        case eError::EnemyDef: return "적 정의 오류";
        case eError::StageInvalid: return "스테이지 스크립트 검증 실패";
        case eError::PropDefInvalid: return "소품 정의 오류";
    }
    return "알 수 없는 에러";
}

}   // namespace slug
