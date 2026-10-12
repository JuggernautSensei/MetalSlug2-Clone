#pragma once
#include "Base/SlugError.h"
#include "Base/Types.h"
#include "Editor/JsonScan.h"
#include "Game/Enemies/EnemyDef.h"

class EnemyLab;
class Renderer;
class SandboxState;

enum class eHitboxField : int
{
    W,
    H,
    Cx,
    Cy,
    Count
};

// JSON 안의 박스 하나. {w,h,cx,cy} 객체이거나 [w,h] 두 숫자 배열(hurtStand, hitSize 등).
struct HitboxEntry
{
    String           owner = {};   // 플레이어("Player") 또는 정의 이름
    String           label = {};   // 파일 안의 점 경로
    ARRAY<double, 4> value = {};
    ARRAY<int, 4>    begin = { -1, -1, -1, -1 };
    ARRAY<int, 4>    end   = { -1, -1, -1, -1 };

    [[nodiscard]] bool HasField(eHitboxField _field) const;
};

// 파일 텍스트를 그대로 들고, 숫자 토큰만 제자리 교체한다 (키 순서/서식 유지, 무편집 저장은 바이트 동일).
class HitboxDocument
{
public:
    [[nodiscard]] Result<void> Load(const FilePath& _path, StringView _defaultOwner);
    [[nodiscard]] Result<void> LoadFromText(StringView _json, StringView _defaultOwner);
    [[nodiscard]] Result<void> Save(const FilePath& _path);

    bool SetValue(int _entry, eHitboxField _field, double _value);
    void Revert();

    // 정의 객체의 "hitboxes" 배열을 _boxes 로 맞춘다 (없으면 삽입, 비면 제거, 같으면 무변경). 정의가 이 파일에 없으면 false.
    bool ApplyExtraBoxes(StringView _def, const Vector<ExtraBoxDef>& _boxes);

    [[nodiscard]] const String&           GetText() const;
    [[nodiscard]] Span<const HitboxEntry> GetEntries() const;
    [[nodiscard]] bool                    IsDirty() const;

private:
    [[nodiscard]] bool Reparse_();
    void               Collect_(const Vector<JsonNode>& _nodes, int _index, const String& _path, const String& _owner);

    String              m_text         = {};
    String              m_savedText    = {};
    String              m_defaultOwner = {};
    Vector<HitboxEntry> m_entries      = {};
};

// AI 탭의 "Hitbox" 창: 플레이어/적/보스 박스 크기 편집 + 라이브 미리보기 + 원본 JSON 저장
class HitboxEditor final
{
public:
    void Draw(EnemyLab& _lab);
    void DrawOverlay(EnemyLab& _lab, Renderer& _renderer);

private:
    void Load_(const FilePath& _assetRoot);
    void ApplyLive_(SandboxState& _state, const HitboxEntry& _entry);
    void DrawEntries_(EnemyLab& _lab, SandboxState* _pState);
    void DrawExtraBoxes_(SandboxState& _state, HitboxDocument& _doc);
    void DrawExtraOverlay_(EnemyLab& _lab, Renderer& _renderer);

    Vector<String>         m_fileNames   = {};
    Vector<FilePath>       m_filePaths   = {};
    Vector<HitboxDocument> m_docs        = {};
    String                 m_status      = {};
    int                    m_fileIndex   = 0;
    int                    m_ownerIndex  = 0;
    int                    m_activeFile  = -1;
    int                    m_activeEntry = -1;
    bool                   m_bLoaded     = false;

    String   m_ownerName = {};
    int      m_newKind   = 0;
    int      m_boxSel    = -1;
    bool     m_bBoxDrag  = false;
    bool     m_bPrevDown = false;
    uint32_t m_dragEnemy = 0;
    Vec2     m_dragGrab  = { 0.f, 0.f };
};
