#pragma once

// --headless 모드: 창/오디오 없이 리플레이를 돌려 "ticks=.. finished=.. hash=.." 한 줄을 출력한다 (회귀 기준선용).
// 인자: --replay <file> [--state <n>] [--ticks <n>] | --gen-replay <out> <state> <ticks> <flip> | --dump-tuning
int RunHeadless(int _argc, char** _argv);
