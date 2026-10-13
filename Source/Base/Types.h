#pragma once
#include <cstdint>
#include <cstring>

// 인스턴스 z 미지정 표시
inline constexpr int kDefaultZ = -2147483647 - 1;

[[nodiscard]] constexpr int ResolveZ(int _z, int _defaultDepth)
{
    return _z == kDefaultZ ? _defaultDepth : _z;
}

// 기본 depth 로부터 벗어난 양
[[nodiscard]] constexpr int ZDelta(int _z, int _defaultDepth)
{
    return _z == kDefaultZ ? 0 : _z - _defaultDepth;
}

// 기준 z 가 기본 depth 에서 벗어난 만큼 다른 기본 depth 를 옮긴다
[[nodiscard]] constexpr int ZShift(int _z, int _fromDepth, int _toDepth)
{
    return _z == kDefaultZ ? kDefaultZ : _z - _fromDepth + _toDepth;
}

struct EnemyPartDepth
{
    int depth = 15;
    int sort  = 0;
};
