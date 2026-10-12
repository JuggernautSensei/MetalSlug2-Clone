#pragma once

// 피격 플래시 (원작 실측: Docs/Spec_HitFlash.md).
// ON 중의 재피격은 무시한다 (연사 중에도 켜진 채로 늘어나지 않고 반드시 깜빡인다).
// 렌더에 보이는 첫 값은 kHitFlashTicks - 1 이다 (피격 이벤트가 Tick 앞에서 처리된다)
inline constexpr int kHitFlashOnTicks  = 3;   // 한 번 켜져 있는 길이 (60Hz 틱, 영상 약 1.5 프레임)
inline constexpr int kHitFlashGapTicks = 3;
inline constexpr int kHitFlashTicks    = kHitFlashOnTicks + kHitFlashGapTicks + 1;

[[nodiscard]] bool IsHitFlashOn(const int _ticksLeft);

bool StartHitFlash(int& _ticksLeft);

void TickHitFlash(int& _ticksLeft);
