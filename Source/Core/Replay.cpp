#include "Core/Replay.h"

#include <cstring>

#include "Base/SlugError.h"
#include "Base/Types.h"
#include "Core/AssetPath.h"

constexpr ARRAY<char, 4> kMagic   = { 'S', 'R', 'E', 'P' };
constexpr uint32_t       kVersion = 1;

namespace
{
template<typename T>
void Append_(
    Vector<uint8_t>& _bytes,
    const T          _value)
{
    const size_t offset = _bytes.size();
    _bytes.resize(offset + sizeof(T));
    std::memcpy(_bytes.data() + offset, &_value, sizeof(T));
}
}   // namespace

class Reader
{
public:
    explicit Reader(
        const Span<const uint8_t> _bytes)
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
    Span<const uint8_t> m_bytes  = {};
    size_t              m_offset = 0;
};

Vector<uint8_t> SerializeReplay(
    const ReplayData& _replay)
{
    Vector<uint8_t> bytes;
    bytes.reserve(32 + _replay.stateName.size() + _replay.frames.size() * sizeof(uint16_t));

    bytes.insert(bytes.end(), kMagic.begin(), kMagic.end());
    Append_<uint32_t>(bytes, kVersion);
    Append_<uint64_t>(bytes, _replay.seed);
    Append_<uint32_t>(bytes, static_cast<uint32_t>(_replay.stateName.size()));
    bytes.insert(bytes.end(), _replay.stateName.begin(), _replay.stateName.end());
    Append_<uint32_t>(bytes, static_cast<uint32_t>(_replay.frames.size()));
    for (const InputFrame& frame: _replay.frames)
    {
        Append_<uint16_t>(bytes, frame.held.GetFlags());
    }
    return bytes;
}

Result<ReplayData> DeserializeReplay(
    const Span<const uint8_t> _bytes)
{
    Reader reader { _bytes };

    ARRAY<char, 4> magic = {};
    if (!reader.ReadBytes(magic.data(), magic.size()) || std::memcmp(magic.data(), kMagic.data(), kMagic.size()) != 0)
    {
        return Error { eError::InvalidData };
    }

    uint32_t   version = 0;
    ReplayData replay;
    uint32_t   nameLength = 0;
    if (!reader.Read(version) || version != kVersion || !reader.Read(replay.seed) || !reader.Read(nameLength))
    {
        return Error { eError::InvalidData };
    }

    constexpr uint32_t kMaxNameLength = 256;
    if (nameLength > kMaxNameLength)
    {
        return Error { eError::InvalidData };
    }
    replay.stateName.resize(nameLength);
    if (nameLength > 0 && !reader.ReadBytes(replay.stateName.data(), nameLength))
    {
        return Error { eError::InvalidData };
    }

    uint32_t frameCount = 0;
    if (!reader.Read(frameCount))
    {
        return Error { eError::InvalidData };
    }

    constexpr uint32_t kMaxFrames = 60u * 60u * 60u;   // 60분
    if (frameCount > kMaxFrames)
    {
        return Error { eError::InvalidData };
    }
    replay.frames.resize(frameCount);
    for (InputFrame& frame: replay.frames)
    {
        uint16_t held = 0;
        if (!reader.Read(held))
        {
            return Error { eError::InvalidData };
        }
        frame.held = IndexedFlags<eAction> { held };
    }
    return replay;
}

Result<void> SaveReplay(
    const FilePath&   _path,
    const ReplayData& _replay)
{
    const Vector<uint8_t> bytes = SerializeReplay(_replay);
    return WriteFileBytes(_path, bytes);
}

Result<ReplayData> LoadReplay(
    const FilePath& _path)
{
    const Result<Vector<uint8_t>> bytes = ReadFileBytes(_path);
    if (!bytes)
    {
        return bytes.GetError();
    }
    return DeserializeReplay(bytes.GetValue());
}
