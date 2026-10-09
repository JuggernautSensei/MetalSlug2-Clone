#pragma once
#include <JugX/Result.h>
#include <JugX/Typedef.h>

#include "Core/Input.h"

namespace slug
{

// ===========================================
//  ReplayData
//   결정적 시뮬레이션을 재현하기 위한 최소 정보: 시작 상태 이름 + RNG 시드 + 틱별 입력.
//   파일 포맷(리틀엔디안): "SREP" u32 version(1) u64 seed u32 nameLen name[nameLen] u32 frameCount u16 held[frameCount]
// ===========================================

struct ReplayData
{
    uint64_t                 seed      = 1;
    jug::String              stateName = "sandbox";
    jug::Vector<InputFrame>  frames    = {};
};

[[nodiscard]] jug::Result<void>       SaveReplay(const jug::FilePath& _path, const ReplayData& _replay);
[[nodiscard]] jug::Result<ReplayData> LoadReplay(const jug::FilePath& _path);

// 메모리 직렬화 (테스트용)
[[nodiscard]] jug::Vector<uint8_t>    SerializeReplay(const ReplayData& _replay);
[[nodiscard]] jug::Result<ReplayData> DeserializeReplay(jug::Span<const uint8_t> _bytes);

}   // namespace slug
