#pragma once

namespace slug
{
class FixedStepClock
{
public:
    static constexpr double kStepSec          = 1.0 / 60.0;
    static constexpr double kMaxFrameSec      = 0.25;   // 긴 정지(디버거 등) 후 따라잡기 폭주 방지
    static constexpr int    kMaxStepsPerFrame = 5;

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

        if (steps == kMaxStepsPerFrame && m_accumulatorSec >= kStepSec)
        {
            m_accumulatorSec = 0.0;
        }
        return steps;
    }

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
}
