#pragma once

// 피격 플래시 (원작 실측: Docs/Spec_HitFlash.md).
// 피격 한 번이 "ON 펄스" 하나를 만든다. 펄스는 kHitFlashOnTicks 동안 켜지고 kHitFlashGapTicks 동안 꺼진다.
// ON 중의 재피격은 무시한다 (연사 중에도 켜진 채로 늘어나지 않고 반드시 깜빡인다).
// 타이머는 피격 시 kHitFlashTicks 로 채워져 0 까지 감소한다. 피격 이벤트가 Tick 앞에서 처리되므로
// 렌더에 보이는 첫 값은 kHitFlashTicks - 1 이다 (그래서 ON 구간 = 값 > kHitFlashGapTicks 인 kHitFlashOnTicks 틱).
inline constexpr int kHitFlashOnTicks  = 3;                                       // 한 번 켜져 있는 길이 (60Hz 틱, 영상 약 1.5 프레임)
inline constexpr int kHitFlashGapTicks = 3;                                       // 다음 펄스 전에 꺼져 있는 최소 길이
inline constexpr int kHitFlashTicks    = kHitFlashOnTicks + kHitFlashGapTicks + 1;   // 피격 시 타이머 초기값

// 타이머 값이 ON 구간이면 true. 0 이하(꺼짐)와 범위 밖 값은 false.
[[nodiscard]] bool IsHitFlashOn(
    const int _ticksLeft);

// 피격 시 호출. 지금 ON 이 아니면 새 펄스를 시작한다 (ON 중이면 무시). 시작했으면 true.
bool StartHitFlash(
    int& _ticksLeft);

// 매 틱 호출. 타이머를 1 줄인다 (0 아래로 내려가지 않는다).
void TickHitFlash(
    int& _ticksLeft);
