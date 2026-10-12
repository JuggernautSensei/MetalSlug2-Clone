#pragma once
#include <array>
#include <chrono>
#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <functional>
#include <set>
#include <span>
#include <string>
#include <string_view>
#include <unordered_map>
#include <utility>
#include <vector>

using FilePath = std::filesystem::path;

using SteadyClock = std::chrono::steady_clock;

template<typename T, typename Alloc = std::allocator<T>>
using Vector = std::vector<T, Alloc>;

template<typename T, size_t kSize>
using ARRAY = std::array<T, kSize>;

template<typename T, typename Cmp = std::less<T>>
using RbSet = std::set<T, Cmp>;

template<typename K, typename V, typename H = std::hash<K>, typename E = std::equal_to<K>>
using HashMap = std::unordered_map<K, V, H, E>;

template<typename T, size_t kExtent = std::dynamic_extent>
using Span = std::span<T, kExtent>;

// 인스턴스 z 미지정 표시 (스폰 시 해당 종류의 기본 depth 를 쓴다)
inline constexpr int kDefaultZ = -2147483647 - 1;

// z 가 kDefaultZ 이면 기본 depth, 아니면 z 그대로
[[nodiscard]] constexpr int ResolveZ(const int _z, const int _defaultDepth)
{
    return _z == kDefaultZ ? _defaultDepth : _z;
}

// 기본 depth 로부터 벗어난 양 (kDefaultZ 이면 0). 파생 FX/꼬리가 기준 인스턴스의 z 이동을 그대로 따른다.
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

using String     = std::string;
using StringView = std::string_view;
