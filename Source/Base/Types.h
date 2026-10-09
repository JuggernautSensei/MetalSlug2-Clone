#pragma once
#include <array>
#include <chrono>
#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <functional>
#include <list>
#include <map>
#include <set>
#include <span>
#include <string>
#include <string_view>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <vector>

namespace slug
{

using FilePath = std::filesystem::path;
namespace fs = std::filesystem;

using SteadyClock = std::chrono::steady_clock;
using SystemClock = std::chrono::system_clock;
template<typename T>
using TimePoint = std::chrono::time_point<T>;
using Sec = std::chrono::seconds;
using Ms  = std::chrono::milliseconds;
using Us  = std::chrono::microseconds;
using Ns  = std::chrono::nanoseconds;

template<typename T, typename Alloc = std::allocator<T>>
using Vector = std::vector<T, Alloc>;

template<typename T, size_t kSize>
using ARRAY = std::array<T, kSize>;

template<typename T, typename Alloc = std::allocator<T>>
using List = std::list<T, Alloc>;

template<typename T, typename Cmp = std::less<T>>
using RbSet = std::set<T, Cmp>;

template<typename K, typename V, typename Cmp = std::less<K>>
using RbMap = std::map<K, V, Cmp>;

template<typename K, typename V, typename H = std::hash<K>, typename E = std::equal_to<K>>
using HashMap = std::unordered_map<K, V, H, E>;

template<typename K, typename H = std::hash<K>, typename E = std::equal_to<K>>
using HashSet = std::unordered_set<K, H, E>;

template<typename T, size_t kExtent = std::dynamic_extent>
using Span = std::span<T, kExtent>;

using String      = std::string;
using StringView  = std::string_view;
using WString     = std::wstring;
using WStringView = std::wstring_view;

}   // namespace slug
