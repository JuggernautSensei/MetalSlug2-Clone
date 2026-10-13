#pragma once
#include "Base/SlugError.h"
#include "Editor/JsonScan.h"
#include "Game/Enemies/EnemyDef.h"

enum class eHitboxField
{
    W,
    H,
    Cx,
    Cy,
    Count
};

struct HitboxEntry
{
    String           owner = {};
    String           label = {};
    ARRAY<double, 4> value = {};
    ARRAY<int, 4>    begin = { -1, -1, -1, -1 };
    ARRAY<int, 4>    end   = { -1, -1, -1, -1 };

    [[nodiscard]] bool HasField(eHitboxField _field) const;
};

class HitboxDocument
{
public:
    [[nodiscard]] Result<void>            Load(const FilePath& _path, StringView _defaultOwner);
    [[nodiscard]] Result<void>            LoadFromText(StringView _json, StringView _defaultOwner);
    [[nodiscard]] Result<void>            Save(const FilePath& _path);
    bool                                  SetValue(int _entry, eHitboxField _field, double _value);
    void                                  Revert();
    bool                                  ApplyExtraBoxes(StringView _def, const Vector<ExtraBoxDef>& _boxes);
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
