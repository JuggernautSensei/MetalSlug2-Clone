#pragma once

// 피격 플래시
// ON 중의 재피격은 무시한다
inline constexpr int kHitFlashOnTicks  = 3;   // 한 번 켜져 있는 길이
inline constexpr int kHitFlashGapTicks = 3;
inline constexpr int kHitFlashTicks    = kHitFlashOnTicks + kHitFlashGapTicks + 1;

[[nodiscard]] bool IsHitFlashOn(const int _ticksLeft);

bool StartHitFlash(int& _ticksLeft);

void TickHitFlash(int& _ticksLeft);
