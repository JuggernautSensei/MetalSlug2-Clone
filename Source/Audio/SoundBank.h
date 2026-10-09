#pragma once
#include <JugX/Result.h>
#include <JugX/Typedef.h>

namespace slug
{

// ===========================================
//  SoundEntry / SoundBank
//   sounds.json (Docs/AssetFormat.md) 를 이름 → 파일 경로 표로 파싱한다. 장치/디코딩과 무관한 순수 데이터라 테스트 가능하다.
//    - bBgm 인 항목은 스트리밍 재생, 나머지는 메모리에 디코딩해 둔다.
// ===========================================

struct SoundEntry
{
    jug::String   name = {};
    jug::String   file = {};   // Baked 루트 기준 상대 경로 ("Audio/PlayerBomb.wav")
    bool          bBgm = false;
    jug::FilePath path = {};   // 루트가 정해진 뒤의 전체 경로
};

class SoundBank
{
public:
    [[nodiscard]] static jug::Result<SoundBank> LoadFromJson(jug::StringView _json, const jug::FilePath& _root = {});
    [[nodiscard]] static jug::Result<SoundBank> LoadFromDirectory(const jug::FilePath& _bakedRoot);

    void Add(SoundEntry _entry);   // 같은 이름이면 교체

    [[nodiscard]] const SoundEntry*           Find(jug::StringView _name) const;
    [[nodiscard]] int                         FindIndex(jug::StringView _name) const;   // 없으면 -1
    [[nodiscard]] const SoundEntry&           Get(const int _index) const { return m_entries[static_cast<size_t>(_index)]; }
    [[nodiscard]] size_t                      GetCount() const { return m_entries.size(); }
    [[nodiscard]] bool                        IsEmpty() const { return m_entries.empty(); }
    [[nodiscard]] jug::Span<const SoundEntry> GetEntries() const { return m_entries; }

private:
    jug::Vector<SoundEntry>             m_entries     = {};
    jug::HashMap<jug::String, uint32_t> m_nameToIndex = {};
};

}   // namespace slug
