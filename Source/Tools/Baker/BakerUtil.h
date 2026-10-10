#pragma once
#include <algorithm>
#include <atomic>
#include <cstdio>
#include <filesystem>
#include <format>
#include <fstream>
#include <functional>
#include <mutex>
#include <thread>

#include "Base/Error.h"
#include "Base/Result.h"
#include "Base/Types.h"
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
    const String     line = std::format(_format, std::forward<Args>(_args)...);
    std::scoped_lock lock(LogMutex());
    std::fputs(line.c_str(), stdout);
    std::fputc('\n', stdout);
    std::fflush(stdout);
}

[[nodiscard]] inline slug::Result<void> WriteBytes(
    const FilePath& _path,
    const void*     _pData,
    const size_t    _size)
{
    std::error_code ec;
    fs::create_directories(_path.parent_path(), ec);

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
    const FilePath& _path,
    const String&   _text)
{
    return WriteBytes(_path, _text.data(), _text.size());
}

[[nodiscard]] inline slug::Result<String> ReadText(
    const FilePath& _path)
{
    std::ifstream stream(ResolveCaseInsensitive(_path), std::ios::binary | std::ios::ate);
    if (!stream)
    {
        return slug::eError::FileOpen;
    }
    const std::streamsize size = stream.tellg();
    String                text(static_cast<size_t>(std::max<std::streamsize>(size, 0)), '\0');
    stream.seekg(0);
    if (size > 0 && !stream.read(text.data(), size))
    {
        return slug::eError::FileRead;
    }
    return text;
}

namespace parallel_detail
{

    // 작업 인덱스를 하나씩 가져가며 _obj 의 멤버 함수를 호출하는 워커 루프.
    template<typename T, typename Method>
    void RunWorker_(
        std::atomic<size_t>& _next,
        const size_t         _count,
        T&                   _obj,
        Method               _pMethod)
    {
        for (size_t i = _next.fetch_add(1); i < _count; i = _next.fetch_add(1))
        {
            std::invoke(_pMethod, _obj, i);
        }
    }

}   // namespace parallel_detail

// _count 개의 작업(_obj.*_pMethod(i))을 스레드 풀에서 나눠 실행한다.
template<typename T, typename Method>
void ParallelFor(
    const size_t _count,
    int          _threads,
    T&           _obj,
    Method       _pMethod)
{
    if (_threads <= 0)
    {
        _threads = static_cast<int>(std::max(1u, std::thread::hardware_concurrency()));
    }
    _threads = std::min<int>(_threads, 16);
    _threads = static_cast<int>(std::min<size_t>(static_cast<size_t>(_threads), std::max<size_t>(_count, 1)));

    std::atomic<size_t> next { 0 };
    if (_threads <= 1)
    {
        parallel_detail::RunWorker_(next, _count, _obj, _pMethod);
        return;
    }

    Vector<std::thread> pool;
    pool.reserve(static_cast<size_t>(_threads));
    for (int t = 0; t < _threads; ++t)
    {
        pool.emplace_back(&parallel_detail::RunWorker_<T, Method>, std::ref(next), _count, std::ref(_obj), _pMethod);
    }
    for (std::thread& thread: pool)
    {
        thread.join();
    }
}

[[nodiscard]] inline String SanitizeFileName(const String& _name)
{
    String out = _name;
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

}   // namespace slug::baker
