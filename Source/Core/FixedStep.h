#pragma once

class FixedStepClock
{
public:
    static constexpr double kStepSec          = 1.0 / 60.0;
    static constexpr double kMaxFrameSec      = 0.25;   // 긴 정지(디버거 등) 후 따라잡기 폭주 방지
    static constexpr int    kMaxStepsPerFrame = 5;

    [[nodiscard]] int Advance(double _deltaTimeSec);

    [[nodiscard]] float GetAlpha() const;

    void Reset();

private:
    double m_accumulatorSec = 0.0;
};
