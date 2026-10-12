#include "Core/AssetPath.h"

#include <filesystem>
#include <iterator>
#include <optional>
#include <string>
#include <system_error>

#if defined(_WIN32)
#    include <windows.h>
#endif

    constexpr int kMaxParentDepth = 5;

    static std::optional<FilePath>& FallbackRoot_()
    {
        static std::optional<FilePath> s_root;
        return s_root;
    }

    static FilePath ExecutableDir_()
    {
#if defined(_WIN32)
        ARRAY<wchar_t, MAX_PATH * 4> buffer = {};
        const DWORD                  length = ::GetModuleFileNameW(nullptr, buffer.data(), static_cast<DWORD>(buffer.size()));
        if (length > 0 && length < buffer.size())
        {
            return FilePath {
                std::wstring { buffer.data(), length }
            }.parent_path();
        }
#endif
        return std::filesystem::current_path();
    }

    static bool IsAssetRoot_(
        const FilePath& _assets)
    {
        std::error_code ec;
        return std::filesystem::exists(_assets / "manifest.json", ec) || std::filesystem::exists(_assets / "Tuning" / "Tuning.json", ec);
    }

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
