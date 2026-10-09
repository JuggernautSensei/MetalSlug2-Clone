#include "Core/Replay.h"
#include "Base/Types.h"
#include "Base/Result.h"

#include <cstring>
#include <fstream>

#include "Base/Error.h"

namespace slug
{

namespace
{
    constexpr char     kMagic[4] = { 'S', 'R', 'E', 'P' };
    constexpr uint32_t kVersion  = 1;

    template<typename T>
    void Append_(
        slug::Vector<uint8_t>& _bytes,
        const T               _value)
    {
        const size_t offset = _bytes.size();
        _bytes.resize(offset + sizeof(T));
        std::memcpy(_bytes.data() + offset, &_value, sizeof(T));
    }

    class Reader
    {
    public:
        explicit Reader(
            const slug::Span<const uint8_t> _bytes)
            : m_bytes(_bytes)
        {
        }

        template<typename T>
        [[nodiscard]] bool Read(
            T& _out)
        {
            if (m_offset + sizeof(T) > m_bytes.size())
            {
                return false;
            }
            std::memcpy(&_out, m_bytes.data() + m_offset, sizeof(T));
            m_offset += sizeof(T);
            return true;
        }

        [[nodiscard]] bool ReadBytes(
            void*        _pDest,
            const size_t _size)
        {
            if (m_offset + _size > m_bytes.size())
            {
                return false;
            }
            std::memcpy(_pDest, m_bytes.data() + m_offset, _size);
            m_offset += _size;
            return true;
        }

    private:
        slug::Span<const uint8_t> m_bytes  = {};
        size_t                   m_offset = 0;
    };
}   // namespace

slug::Vector<uint8_t> SerializeReplay(
    const ReplayData& _replay)
{
    slug::Vector<uint8_t> bytes;
    bytes.reserve(32 + _replay.stateName.size() + _replay.frames.size() * sizeof(uint16_t));

    bytes.insert(bytes.end(), std::begin(kMagic), std::end(kMagic));
    Append_<uint32_t>(bytes, kVersion);
    Append_<uint64_t>(bytes, _replay.seed);
    Append_<uint32_t>(bytes, static_cast<uint32_t>(_replay.stateName.size()));
    bytes.insert(bytes.end(), _replay.stateName.begin(), _replay.stateName.end());
    Append_<uint32_t>(bytes, static_cast<uint32_t>(_replay.frames.size()));
    for (const InputFrame& frame: _replay.frames)
    {
        Append_<uint16_t>(bytes, frame.held);
    }
    return bytes;
}

slug::Result<ReplayData> DeserializeReplay(
    const slug::Span<const uint8_t> _bytes)
{
    Reader reader { _bytes };

    char magic[4] = {};
    if (!reader.ReadBytes(magic, sizeof(magic)) || std::memcmp(magic, kMagic, sizeof(kMagic)) != 0)
    {
        return slug::Error { eError::InvalidData };
    }

    uint32_t version = 0;
    ReplayData replay;
    uint32_t nameLength = 0;
    if (!reader.Read(version) || version != kVersion || !reader.Read(replay.seed) || !reader.Read(nameLength))
    {
        return slug::Error { eError::InvalidData };
    }

    constexpr uint32_t kMaxNameLength = 256;
    if (nameLength > kMaxNameLength)
    {
        return slug::Error { eError::InvalidData };
    }
    replay.stateName.resize(nameLength);
    if (nameLength > 0 && !reader.ReadBytes(replay.stateName.data(), nameLength))
    {
        return slug::Error { eError::InvalidData };
    }

    uint32_t frameCount = 0;
    if (!reader.Read(frameCount))
    {
        return slug::Error { eError::InvalidData };
    }

    constexpr uint32_t kMaxFrames = 60u * 60u * 60u;   // 60분
    if (frameCount > kMaxFrames)
    {
        return slug::Error { eError::InvalidData };
    }
    replay.frames.resize(frameCount);
    for (InputFrame& frame: replay.frames)
    {
        if (!reader.Read(frame.held))
        {
            return slug::Error { eError::InvalidData };
        }
    }
    return replay;
}

slug::Result<void> SaveReplay(
    const slug::FilePath& _path,
    const ReplayData&    _replay)
{
    const slug::Vector<uint8_t> bytes = SerializeReplay(_replay);

    std::ofstream file { _path, std::ios::binary | std::ios::trunc };
    if (!file)
    {
        return slug::Error { eError::FileOpen };
    }
    file.write(reinterpret_cast<const char*>(bytes.data()), static_cast<std::streamsize>(bytes.size()));
    if (!file)
    {
        return slug::Error { eError::FileRead };
    }
    return slug::Error { slug::kOK };
}

slug::Result<ReplayData> LoadReplay(
    const slug::FilePath& _path)
{
    std::ifstream file { _path, std::ios::binary | std::ios::ate };
    if (!file)
    {
        return slug::Error { eError::FileOpen };
    }

    const std::streamsize size = file.tellg();
    file.seekg(0, std::ios::beg);

    slug::Vector<uint8_t> bytes(static_cast<size_t>(size));
    if (size > 0 && !file.read(reinterpret_cast<char*>(bytes.data()), size))
    {
        return slug::Error { eError::FileRead };
    }
    return DeserializeReplay(bytes);
}

}   // namespace slug
