#pragma once

namespace slug
{

// ===========================================
//  FixedStepClock
//   가변 프레임 시간을 고정 스텝(60Hz)으로 변환한다.
//   시뮬레이션은 항상 kStepSec 단위로만 진행되므로 프레임레이트와 무관하게 동일하게 동작한다.
// ===========================================

class FixedStepClock
{
public:
    static constexpr double kStepSec          = 1.0 / 60.0;
    static constexpr double kMaxFrameSec      = 0.25;   // 긴 정지(디버거 등) 후 따라잡기 폭주 방지
    static constexpr int    kMaxStepsPerFrame = 5;

    // 프레임 델타를 누적하고, 이번 프레임에 실행해야 하는 스텝 수를 반환한다.
    [[nodiscard]] int Advance(
        const double _deltaTimeSec)
    {
        const double delta = _deltaTimeSec < 0.0 ? 0.0 : (_deltaTimeSec > kMaxFrameSec ? kMaxFrameSec : _deltaTimeSec);
        m_accumulatorSec += delta;

        int steps = 0;
        while (m_accumulatorSec >= kStepSec && steps < kMaxStepsPerFrame)
        {
            m_accumulatorSec -= kStepSec;
            ++steps;
        }

        // 따라잡지 못한 시간은 버린다 (슬로모션 대신 프레임 드롭)
        if (steps == kMaxStepsPerFrame && m_accumulatorSec >= kStepSec)
        {
            m_accumulatorSec = 0.0;
        }
        return steps;
    }

    // 렌더 보간 계수 [0, 1)
    [[nodiscard]] float GetAlpha() const
    {
        return static_cast<float>(m_accumulatorSec / kStepSec);
    }

    void Reset()
    {
        m_accumulatorSec = 0.0;
    }

private:
    double m_accumulatorSec = 0.0;
};

}   // namespace slug
