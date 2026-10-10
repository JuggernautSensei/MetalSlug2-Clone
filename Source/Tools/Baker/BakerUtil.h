#pragma once
#include <algorithm>
#include <atomic>
#include <cstdio>
#include <filesystem>
#include <format>
#include <fstream>
#include <mutex>
#include <string>
#include <thread>
#include <vector>

#include "Base/Result.h"

#include "Base/Error.h"
#include "Tools/Baker/PathResolve.h"

namespace slug::baker
{
inline std::mutex& LogMutex()
{
    static std::mutex s_mutex;
    return s_mutex;
}

template<typename... Args>
void Log(
    std::format_string<Args...> _format,
    Args&&... _args)
{
    const std::string line = std::format(_format, std::forward<Args>(_args)...);
    std::scoped_lock  lock(LogMutex());
    std::fputs(line.c_str(), stdout);
    std::fputc('\n', stdout);
    std::fflush(stdout);
}

[[nodiscard]] inline slug::Result<void> WriteBytes(
    const std::filesystem::path& _path,
    const void*                  _pData,
    const size_t                 _size)
{
    std::error_code ec;
    std::filesystem::create_directories(_path.parent_path(), ec);

    std::ofstream stream(_path, std::ios::binary | std::ios::trunc);
    if (!stream)
    {
        return slug::Error { slug::eError::FileOpen };
    }
    stream.write(static_cast<const char*>(_pData), static_cast<std::streamsize>(_size));
    if (!stream)
    {
        return slug::Error { slug::eError::FileRead };
    }
    return slug::Error { slug::kOK };
}

[[nodiscard]] inline slug::Result<void> WriteText(
    const std::filesystem::path& _path,
    const std::string&           _text)
{
    return WriteBytes(_path, _text.data(), _text.size());
}

[[nodiscard]] inline slug::Result<std::string> ReadText(
    const std::filesystem::path& _path)
{
    std::ifstream stream(ResolveCaseInsensitive(_path), std::ios::binary | std::ios::ate);
    if (!stream)
    {
        return slug::eError::FileOpen;
    }
    const std::streamsize size = stream.tellg();
    std::string           text(static_cast<size_t>(std::max<std::streamsize>(size, 0)), '\0');
    stream.seekg(0);
    if (size > 0 && !stream.read(text.data(), size))
    {
        return slug::eError::FileRead;
    }
    return text;
}

template<typename Fn>
void ParallelFor(
    const size_t _count,
    int          _threads,
    Fn&&         _fn)
{
    if (_threads <= 0)
    {
        _threads = static_cast<int>(std::max(1u, std::thread::hardware_concurrency()));
    }
    _threads = std::min<int>(_threads, 16);
    _threads = static_cast<int>(std::min<size_t>(static_cast<size_t>(_threads), std::max<size_t>(_count, 1)));

    std::atomic<size_t> next { 0 };
    auto                worker = [&]()
    {
        for (size_t i = next.fetch_add(1); i < _count; i = next.fetch_add(1))
        {
            _fn(i);
        }
    };

    if (_threads <= 1)
    {
        worker();
        return;
    }

    std::vector<std::thread> pool;
    pool.reserve(static_cast<size_t>(_threads));
    for (int t = 0; t < _threads; ++t)
    {
        pool.emplace_back(worker);
    }
    for (std::thread& thread: pool)
    {
        thread.join();
    }
}

[[nodiscard]] inline std::string SanitizeFileName(const std::string& _name)
{
    std::string out = _name;
    for (char& c: out)
    {
        const bool bOk = (c >= '0' && c <= '9') || (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || c == '_' || c == '-' || c == '.';
        if (!bOk)
        {
            c = '_';
        }
    }
    return out;
}
}
