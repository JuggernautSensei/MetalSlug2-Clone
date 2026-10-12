#pragma once
#include <algorithm>

#include "Base/Types.h"
#include "Tools/Baker/LegacyParser.h"

namespace slug::baker
{

// 레거시 .anime 의 오타/좌우 불일치를 굽기 직전에 바로잡는 표 (Docs/AnimAudit.md §4).
// 값은 모두 쌍둥이(_Left/_Right) 클립과 원본 아트(텍스처 열 수)에 맞춘 것이다.
// 필드를 지정하지 않으려면 sentinel(kKeep) 을 둔다.
struct LegacyAnimFixup
{
    static constexpr int    kKeep         = -9999;
    static constexpr double kKeepDuration = -1.0;

    const char* name          = "";
    int         end           = kKeep;           // 마지막 열
    double      frameDuration = kKeepDuration;   // 초
    int         offsetY       = kKeep;
    int         loop          = kKeep;           // 0/1
    const char* reason        = "";
};

inline constexpr size_t kLegacyAnimFixupCount = 6;

inline constexpr ARRAY<LegacyAnimFixup, kLegacyAnimFixupCount> kLegacyAnimFixups = { {
    { "SD_Hang_Idle_Right", 3, 0.1, LegacyAnimFixup::kKeep, LegacyAnimFixup::kKeep, "Right 만 End:0 (텍스처는 4열). Left 와 같은 4프레임/0.1s" },
    { "SD_Die_Stand_2_Right", LegacyAnimFixup::kKeep, 0.1, LegacyAnimFixup::kKeep, LegacyAnimFixup::kKeep, "Right 만 0.08s (Left 0.1s, def 0.1s)" },
    { "SD_Die_Land_Right", LegacyAnimFixup::kKeep, 0.1, LegacyAnimFixup::kKeep, LegacyAnimFixup::kKeep, "Right 만 0.08s (Left 0.1s, def 0.1s)" },
    { "MissleTank_Down_Idle_Left", LegacyAnimFixup::kKeep, 0.08, LegacyAnimFixup::kKeep, LegacyAnimFixup::kKeep, "Left 만 0.1s (Right 0.08s, def 0.08s)" },
    { "Player_Stand_Idle_Lookup_SG_Right", LegacyAnimFixup::kKeep, LegacyAnimFixup::kKeepDuration, -43, LegacyAnimFixup::kKeep, "OffsetY -430 오타 (Left -43). 387px 위로 튄다" },
    { "MissleTank_Up_Proj_Effect_Right", LegacyAnimFixup::kKeep, LegacyAnimFixup::kKeepDuration, LegacyAnimFixup::kKeep, 0, "Right 만 Loop:1 (Left 0). 한 번 재생하는 총구 이펙트가 영원히 반복" },
} };

// 이름이 표에 있으면 레코드를 고치고 항목을 반환한다. 없으면 nullptr.
[[nodiscard]] inline const LegacyAnimFixup* ApplyLegacyAnimFixup(
    AnimRecord& _rec)
{
    const LegacyAnimFixup* const it = std::find_if(kLegacyAnimFixups.data(), kLegacyAnimFixups.data() + kLegacyAnimFixups.size(), [&_rec](const LegacyAnimFixup& _fix) {
        return _rec.name == _fix.name;
    });
    if (it == kLegacyAnimFixups.data() + kLegacyAnimFixups.size())
    {
        return nullptr;
    }
    if (it->end != LegacyAnimFixup::kKeep)
    {
        _rec.end = it->end;
    }
    if (it->frameDuration > 0.0)
    {
        _rec.frameDuration = it->frameDuration;
    }
    if (it->offsetY != LegacyAnimFixup::kKeep)
    {
        _rec.offsetY = it->offsetY;
    }
    if (it->loop != LegacyAnimFixup::kKeep)
    {
        _rec.bLoop = it->loop != 0;
    }
    return it;
}

}   // namespace slug::baker
