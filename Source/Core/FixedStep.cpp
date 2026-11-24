#include "Core/FixedStep.h"

int FixedStepClock::Advance(
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

float FixedStepClock::GetAlpha() const
{
    return static_cast<float>(m_accumulatorSec / kStepSec);
}

void FixedStepClock::Reset()
{
    m_accumulatorSec = 0.0;
}
