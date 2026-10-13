#include "Base/PhaseTimer.h"

#include "Base/Log.h"

ScopedPhaseTimer::ScopedPhaseTimer(
    const char*      _pPhase,
    const StringView _detail)
    : m_pPhase(_pPhase)
    , m_detail(_detail)
{
    m_timer.Start();
}

ScopedPhaseTimer::~ScopedPhaseTimer()
{
    m_timer.Stop();
    const double ms = TimeCast(m_timer.GetElapsedCount(), eTimeUnit::Milli);
    SLUG_LOG_INFO("[load] {} {} {:.1f} ms", m_pPhase, m_detail, ms);
}
