#pragma once
#include <cstdint>
#include <cstring>

#include <JugX/EnumArray.h>
#include <JugX/EnumFlags.h>
#include <JugX/MemoryHasher.h>
#include <JugX/Typedef.h>
#include <JugX/Timer.h>

using jug::ARRAY;
using jug::DIRECT_ENUM_ARRAY;
using jug::eTimeUnit;
using jug::FilePath;
using jug::Fnv1a;
using jug::HashMap;
using jug::IndexedFlags;
using jug::MemoryView;
using jug::RbMap;
using jug::RbSet;
using jug::Span;
using jug::String;
using jug::StringView;
using jug::TimeCast;
using jug::TimeCastF;
using jug::Timer;
using jug::Vector;

// 인스턴스 z 미지정 표시
inline constexpr int kDefaultZ = -2147483647 - 1;

[[nodiscard]] constexpr int ResolveZ(const int _z, const int _defaultDepth)
{
    return _z == kDefaultZ ? _defaultDepth : _z;
}

// 기본 depth 로부터 벗어난 양
[[nodiscard]] constexpr int ZDelta(const int _z, const int _defaultDepth)
{
    return _z == kDefaultZ ? 0 : _z - _defaultDepth;
}

// 기준 z 가 기본 depth 에서 벗어난 만큼 다른 기본 depth 를 옮긴다
[[nodiscard]] constexpr int ZShift(const int _z, const int _fromDepth, const int _toDepth)
{
    return _z == kDefaultZ ? kDefaultZ : _z - _fromDepth + _toDepth;
}

struct EnemyPartDepth
{
    int depth = 15;
    int sort  = 0;
};
