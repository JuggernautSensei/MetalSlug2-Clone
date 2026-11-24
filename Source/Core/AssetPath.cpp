#include "Core/AssetPath.h"

#include <filesystem>
#include <iterator>
#include <optional>
#include <string>
#include <system_error>

#if defined(_WIN32)
#    include <windows.h>
#endif

#include "Base/SlugError.h"

constexpr int kMaxParentDepth = 5;

namespace
{
std::optional<FilePath>& FallbackRoot_()
{
    static std::optional<FilePath> s_root;
    return s_root;
}

FilePath ExecutableDir_()
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

bool IsAssetRoot_(
    const FilePath& _assets)
{
    std::error_code ec;
    return std::filesystem::exists(_assets / "manifest.json", ec) || std::filesystem::exists(_assets / "Tuning" / "Tuning.json", ec);
}

template<typename Container>
[[nodiscard]] Result<Container> ReadAll_(
    const FilePath& _path)
{
    const Result<jug::FileReader> file = jug::FileReader::Open(_path);
    if (!file)
    {
        return Error { eError::FileOpen };
    }
    Container bytes = {};
    bytes.resize(static_cast<size_t>(file->GetSize()));
    if (bytes.empty())
    {
        return bytes;
    }
    const Result<size_t> read = file->Read(jug::MutableMemoryView { bytes });
    if (!read || read.GetValue() != bytes.size())
    {
        return Error { eError::FileRead };
    }
    return bytes;
}
}   // namespace

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

Result<Vector<uint8_t>> ReadFileBytes(
    const FilePath& _path)
{
    return ReadAll_<Vector<uint8_t>>(_path);
}

Result<String> ReadFileText(
    const FilePath& _path)
{
    return ReadAll_<String>(_path);
}

Result<void> WriteFileBytes(
    const FilePath&  _path,
    const MemoryView _bytes)
{
    const Result<jug::FileWriter> file = jug::FileWriter::Open(_path);
    if (!file)
    {
        return Error { eError::FileOpen };
    }
    if (!_bytes.IsEmpty() && !file->Write(_bytes))
    {
        return Error { eError::FileRead };
    }
    return Error { kOK };
}
