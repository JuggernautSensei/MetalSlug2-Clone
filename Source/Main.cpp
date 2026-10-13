#include <cstdio>
#include <cstdlib>
#include <cstring>

#include "Base/Log.h"
#include "Base/SlugError.h"
#include "Core/App.h"
#include "Core/Replay.h"
#include "Core/Tuning.h"

namespace
{
// 결정적 합성 입력: 오른쪽 이동 + 주기적 점프/사격/폭탄. _flipPeriod > 0 이면 그 주기로 좌우를 번갈아 걷는다.
InputFrame MakeScriptedFrame_(
    const uint64_t _tick,
    const uint64_t _flipPeriod)
{
    InputFrame frame;
    const bool bLeft = _flipPeriod > 0 && ((_tick / _flipPeriod) % 2) == 1;
    frame.Set(bLeft ? eAction::Left : eAction::Right, true);
    frame.Set(eAction::Jump, (_tick % 150) < 12 && _tick > 60);
    frame.Set(eAction::Shot, (_tick % 10) < 3);
    frame.Set(eAction::Up, (_tick % 400) >= 380);
    frame.Set(eAction::Bomb, (_tick % 420) == 300);
    return frame;
}

// --gen-replay <out> <state> <ticks> <flipPeriod>: 합성 입력 리플레이를 만든다 (회귀 기준선 생성용)
int GenReplay_(
    char** _argv)
{
    ReplayData data;
    data.seed                 = 12345;
    data.stateName            = _argv[2];
    const uint64_t ticks      = std::strtoull(_argv[3], nullptr, 10);
    const uint64_t flipPeriod = std::strtoull(_argv[4], nullptr, 10);
    for (uint64_t tick = 0; tick < ticks; ++tick)
    {
        data.frames.push_back(MakeScriptedFrame_(tick, flipPeriod));
    }
    if (const Result<void> saved = SaveReplay(_argv[1], data); !saved)
    {
        SLUG_LOG_ERROR("리플레이 저장 실패: {}", saved.GetError().MakeMessage());
        return 1;
    }
    std::printf("wrote %s frames=%llu\n", _argv[1], static_cast<unsigned long long>(ticks));
    return 0;
}
}   // namespace

int main(
    const int _argc,
    char**    _argv)
{
    InstallCoreLogger();
    if (_argc > 1 && std::strcmp(_argv[1], "--dump-tuning") == 0)
    {
        std::fputs(DumpTuning(Tuning {}).c_str(), stdout);
        return 0;
    }
    if (_argc > 5 && std::strcmp(_argv[1], "--gen-replay") == 0)
    {
        return GenReplay_(_argv + 1);
    }
    App app;
    return app.Run(_argc, _argv);
}
