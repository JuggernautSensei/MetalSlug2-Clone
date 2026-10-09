#include "Core/AssetPath.h"

#include <filesystem>
#include <iterator>
#include <optional>
#include <string>
#include <system_error>

#if defined(_WIN32)
#include <windows.h>
#endif

namespace slug
{
namespace
{
constexpr int kMaxParentDepth = 5;

std::optional<FilePath>& FallbackRoot_()
{
    static std::optional<FilePath> s_root;
    return s_root;
}

FilePath ExecutableDir_()
{
#if defined(_WIN32)
    wchar_t buffer[MAX_PATH * 4] {};
    const DWORD length = ::GetModuleFileNameW(nullptr, buffer, static_cast<DWORD>(std::size(buffer)));
    if (length > 0 && length < std::size(buffer))
    {
        return FilePath { std::wstring { buffer, length } }.parent_path();
    }
#endif
    return std::filesystem::current_path();
}

bool IsAssetRoot_(
    const FilePath& _assets)
{
    std::error_code ec;
    return std::filesystem::exists(_assets / "Baked" / "manifest.json", ec) || std::filesystem::exists(_assets / "Tuning" / "Tuning.json", ec);
}
} // namespace

FilePath FindAssetRoot()
{
    FilePath dir = ExecutableDir_();
    for (int depth = 0; depth <= kMaxParentDepth; ++depth)
    {
        const FilePath assets = dir / "Assets";
        if (IsAssetRoot_(assets))
        {
            return assets;
        }
        if (!dir.has_parent_path() || dir.parent_path() == dir)
        {
            break;
        }
        dir = dir.parent_path();
    }

    if (FallbackRoot_())
    {
        return *FallbackRoot_();
    }
    return std::filesystem::current_path() / "Assets";
}

void SetAssetRootFallback(
    FilePath _root)
{
    FallbackRoot_() = std::move(_root);
}
} // namespace slug
