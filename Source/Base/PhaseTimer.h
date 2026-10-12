#pragma once
#include "Base/Types.h"

// 로딩 단계 시간 측정: 소멸 시 "[load] 이름 상세 ms" 를 INFO 로 남긴다.
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
