#pragma once
#include "Base/Types.h"

class ScopedPhaseTimer final
{
public:
    ScopedPhaseTimer(const char* _pPhase, StringView _detail = {});
    ~ScopedPhaseTimer();

    ScopedPhaseTimer(const ScopedPhaseTimer&)            = delete;
    ScopedPhaseTimer& operator=(const ScopedPhaseTimer&) = delete;

private:
    const char*             m_pPhase = nullptr;
    String                  m_detail = {};
    SteadyClock::time_point m_start  = {};
};
