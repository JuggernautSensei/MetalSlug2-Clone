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

namespace slug
{

using FilePath = std::filesystem::path;
namespace fs = std::filesystem;

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

using String     = std::string;
using StringView = std::string_view;
}
