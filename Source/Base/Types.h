#pragma once
#include <cstdint>
#include <cstring>

#include <JugX/EnumArray.h>
#include <JugX/EnumFlags.h>
#include <JugX/MemoryHasher.h>
#include <JugX/Typedef.h>

using jug::ARRAY;
using jug::DIRECT_ENUM_ARRAY;
using jug::FilePath;
using jug::Fnv1a;
using jug::HashMap;
using jug::IndexedFlags;
using jug::RbSet;
using jug::Span;
using jug::SteadyClock;
using jug::String;
using jug::StringView;
using jug::Vector;

// 인스턴스 z 미지정 표시 (스폰 시 해당 종류의 기본 depth 를 쓴다)
inline constexpr int kDefaultZ = -2147483647 - 1;

[[nodiscard]] constexpr int ResolveZ(const int _z, const int _defaultDepth)
{
    return _z == kDefaultZ ? _defaultDepth : _z;
}

// 기본 depth 로부터 벗어난 양 (kDefaultZ 이면 0)
[[nodiscard]] constexpr int ZDelta(const int _z, const int _defaultDepth)
{
    return _z == kDefaultZ ? 0 : _z - _defaultDepth;
}

// 기준 z 가 기본 depth 에서 벗어난 만큼 다른 기본 depth 를 옮긴다 (kDefaultZ 는 그대로)
[[nodiscard]] constexpr int ZShift(const int _z, const int _fromDepth, const int _toDepth)
{
    return _z == kDefaultZ ? kDefaultZ : _z - _fromDepth + _toDepth;
}

// 부품 그리기 깊이: depth = SpriteBatch 절대 레이어, sort = 같은 레이어 안 정렬 키
// (발 y 로 정렬하는 부품 -- 헬기/오버레이 -- 은 sort 를 발 y 에 더하는 오프셋으로 쓴다)
struct EnemyPartDepth
{
    int depth = 15;
    int sort  = 0;
};
