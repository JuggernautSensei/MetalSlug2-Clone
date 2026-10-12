#include "Base/PhaseTimer.h"

#include "Base/Log.h"

ScopedPhaseTimer::ScopedPhaseTimer(
    const char*      _pPhase,
    const StringView _detail)
    : m_pPhase(_pPhase)
    , m_detail(_detail)
    , m_start(SteadyClock::now())
{
}

ScopedPhaseTimer::~ScopedPhaseTimer()
{
    const double ms = std::chrono::duration<double, std::milli>(SteadyClock::now() - m_start).count();
    SLUG_LOG_INFO("[load] {} {} {:.1f} ms", m_pPhase, m_detail, ms);
}
