#pragma once
#include <algorithm>
#include <cctype>
#include <filesystem>
#include <string>

namespace slug::baker
{
namespace detail
{
[[nodiscard]] inline std::string LowerAscii(
    const std::string& _text)
{
    std::string lower = _text;
    std::transform(lower.begin(), lower.end(), lower.begin(), [](unsigned char _c) { return static_cast<char>(std::tolower(_c)); });
    return lower;
}
} // namespace detail

// 레거시 리소스는 Windows(대소문자 무시)에서 만들어졌으므로 Linux 에서는 경로의 대소문자가 달라도 찾을 수 있게 한다.
// 그대로 존재하면 그대로 돌려주고, 아니면 각 경로 요소를 대소문자 무시로 맞춰 본다. 못 찾으면 원본을 돌려준다.
[[nodiscard]] inline std::filesystem::path ResolveCaseInsensitive(
    const std::filesystem::path& _path)
{
    namespace fs = std::filesystem;

    std::error_code ec;
    if (fs::exists(_path, ec))
    {
        return _path;
    }

    fs::path resolved = _path.is_absolute() ? _path.root_path() : fs::path {};
    for (const fs::path& part: _path.relative_path())
    {
        const fs::path direct = resolved / part;
        if (fs::exists(direct, ec))
        {
            resolved = direct;
            continue;
        }

        const fs::path dir = resolved.empty() ? fs::path { "." } : resolved;
        const std::string wanted = detail::LowerAscii(part.string());
        bool              bFound = false;
        for (const fs::directory_entry& entry: fs::directory_iterator(dir, ec))
        {
            if (detail::LowerAscii(entry.path().filename().string()) == wanted)
            {
                resolved = resolved / entry.path().filename();
                bFound   = true;
                break;
            }
        }
        if (!bFound)
        {
            return _path;
        }
    }
    return resolved;
}
} // namespace slug::baker
