#include "Editor/HitboxEditor.h"

#include <charconv>
#include <cmath>
#include <format>
#include <fstream>
#include <iterator>

#include <imgui.h>

#include <SDL3/SDL.h>

#include "Editor/EnemyLab.h"
#include "Render/Renderer.h"
#include "Editor/StageEditor.h"
#include "Game/Enemies/EnemyDef.h"
#include "Game/SandboxState.h"
#include "Base/RGBA.h"

static constexpr ARRAY<const char*, 6> kHitboxFileDirs  = { "Tuning", "Defs", "Defs", "Defs", "Defs", "Defs" };
static constexpr ARRAY<const char*, 6> kHitboxFileNames = { "Tuning.json", "Enemies.json", "BigShiee.json", "Bus.json", "Helicopter.json", "Vehicles.json" };

static bool IsNumberNode_(
    const Vector<JsonNode>& _nodes,
    const int               _index)
{
    return _index >= 0 && _nodes[static_cast<size_t>(_index)].type == 'n';
}

static int FindKey_(
    const JsonNode&  _object,
    const StringView _key)
{
    for (size_t i = 0; i < _object.keys.size(); ++i)
    {
        if (_object.keys[i] == _key)
        {
            return _object.kids[i];
        }
    }
    return -1;
}

static double ParseNumber_(
    const String& _text)
{
    double value = 0.0;
    (void)std::from_chars(_text.data(), _text.data() + _text.size(), value);
    return value;
}

static String RenderExtraBox_(
    const ExtraBoxDef& _box)
{
    String out = std::format("{{ \"id\": \"{}\", \"kind\": \"{}\", \"w\": {}, \"h\": {}, \"offset\": [{}, {}]",
                             _box.id,
                             _box.kind == eExtraBoxKind::Attack ? "attack" : "hurt",
                             StageDocument::FormatNumber(static_cast<double>(_box.box.w)),
                             StageDocument::FormatNumber(static_cast<double>(_box.box.h)),
                             StageDocument::FormatNumber(static_cast<double>(_box.box.cx)),
                             StageDocument::FormatNumber(static_cast<double>(_box.box.cy)));
    if (_box.kind == eExtraBoxKind::Attack)
    {
        if (_box.frameTo >= 0)
        {
            out += std::format(", \"frames\": [{}, {}]", _box.frameFrom, _box.frameTo);
        }
        if (_box.damage != 1)
        {
            out += std::format(", \"damage\": {}", _box.damage);
        }
    }
    return out + " }";
}

static String RenderExtraBoxes_(
    const Vector<ExtraBoxDef>& _boxes)
{
    String out = "[";
    for (size_t i = 0; i < _boxes.size(); ++i)
    {
        out += (i == 0 ? "" : ", ") + RenderExtraBox_(_boxes[i]);
    }
    return out + "]";
}

static bool IsSizeKey_(
    const String& _key)
{
    return _key.find("Size") != String::npos || _key.starts_with("hurt");
}

enum class eHitboxKind : int
{
    Body,
    Hurt,
    Attack,
    Other
};

struct HitboxTitle
{
    String      ko   = {};
    String      en   = {};
    eHitboxKind kind = eHitboxKind::Other;
};

static HitboxTitle MakeHitboxTitle_(
    const HitboxEntry& _entry)
{
    const size_t dot = _entry.label.rfind('.');
    const String key = dot == String::npos ? _entry.label : _entry.label.substr(dot + 1);
    HitboxTitle  t;
    if (key == "standSize")
    {
        t = { "서있을 때 충돌 상자 (standSize)", "STAND BODY (standSize)", eHitboxKind::Body };
    }
    else if (key == "crouchSize")
    {
        t = { "앉았을 때 충돌 상자 (crouchSize)", "CROUCH BODY (crouchSize)", eHitboxKind::Body };
    }
    else if (key == "hurtStand")
    {
        t = { "서있을 때 피격 상자 (hurtStand)", "STAND HURT (hurtStand)", eHitboxKind::Hurt };
    }
    else if (key == "hurtCrouch")
    {
        t = { "앉았을 때 피격 상자 (hurtCrouch)", "CROUCH HURT (hurtCrouch)", eHitboxKind::Hurt };
    }
    else if (key == "body")
    {
        t = { "충돌 상자 (body)", "BODY (body)", eHitboxKind::Body };
    }
    else if (key == "hurtbox")
    {
        t = { "피격 상자 (hurtbox)", "HURTBOX (hurtbox)", eHitboxKind::Hurt };
    }
    else if (key == "hitSize" || key == "hitbox")
    {
        String weapon = _entry.label;
        if (weapon.starts_with("weapons.") && dot != String::npos)
        {
            weapon = weapon.substr(8, dot - 8);
        }
        t = { weapon + " 공격 판정 (" + key + ")", weapon + " ATTACK (" + key + ")", eHitboxKind::Attack };
    }
    else
    {
        t = { _entry.label, _entry.label, eHitboxKind::Other };
    }
    return t;
}

static RGBA HitboxKindColor_(
    const eHitboxKind _kind)
{
    switch (_kind)
    {
        case eHitboxKind::Body: return { 0, 220, 255, 255 };
        case eHitboxKind::Hurt: return { 60, 255, 90, 255 };
        case eHitboxKind::Attack: return { 255, 50, 50, 255 };
        case eHitboxKind::Other: break;
    }
    return { 255, 255, 255, 255 };
}

bool HitboxEntry::HasField(
    const eHitboxField _field) const
{
    return begin[static_cast<size_t>(_field)] >= 0;
}

Result<void> HitboxDocument::Load(
    const FilePath&  _path,
    const StringView _defaultOwner)
{
    std::ifstream file { _path, std::ios::binary };
    if (!file)
    {
        return Error { eError::FileOpen };
    }
    const String text { std::istreambuf_iterator<char>(file), std::istreambuf_iterator<char>() };
    return LoadFromText(text, _defaultOwner);
}

Result<void> HitboxDocument::LoadFromText(
    const StringView _json,
    const StringView _defaultOwner)
{
    m_text         = String { _json };
    m_savedText    = m_text;
    m_defaultOwner = String { _defaultOwner };
    if (!Reparse_())
    {
        m_text.clear();
        m_savedText.clear();
        m_entries.clear();
        return Error { eError::FileOpen };
    }
    return Error { kOK };
}

Result<void> HitboxDocument::Save(
    const FilePath& _path)
{
    std::ofstream file { _path, std::ios::binary | std::ios::trunc };
    if (!file)
    {
        return Error { eError::FileOpen };
    }
    file.write(m_text.data(), static_cast<std::streamsize>(m_text.size()));
    file.close();
    if (!file)
    {
        return Error { eError::FileOpen };
    }
    m_savedText = m_text;
    return Error { kOK };
}

bool HitboxDocument::SetValue(
    const int          _entry,
    const eHitboxField _field,
    const double       _value)
{
    if (_entry < 0 || _entry >= static_cast<int>(m_entries.size()))
    {
        return false;
    }
    const HitboxEntry& entry = m_entries[static_cast<size_t>(_entry)];
    const size_t       f     = static_cast<size_t>(_field);
    if (!entry.HasField(_field))
    {
        return false;
    }
    if (std::fabs(entry.value[f] - _value) < 1e-9)
    {
        return true;
    }
    m_text.replace(static_cast<size_t>(entry.begin[f]), static_cast<size_t>(entry.end[f] - entry.begin[f]), StageDocument::FormatNumber(_value));
    return Reparse_();
}

bool HitboxDocument::ApplyExtraBoxes(
    const StringView           _def,
    const Vector<ExtraBoxDef>& _boxes)
{
    JsonScanner scanner { m_text };
    const int   root = scanner.Parse();
    if (root < 0)
    {
        return false;
    }
    const int defs = FindKey_(scanner.nodes[static_cast<size_t>(root)], "defs");
    const int obj  = defs < 0 ? -1 : FindKey_(scanner.nodes[static_cast<size_t>(defs)], _def);
    if (obj < 0 || scanner.nodes[static_cast<size_t>(obj)].type != 'o')
    {
        return false;
    }
    const JsonNode& object = scanner.nodes[static_cast<size_t>(obj)];
    const int       kid    = FindKey_(object, "hitboxes");
    const String    text   = RenderExtraBoxes_(_boxes);
    if (kid < 0)
    {
        if (_boxes.empty())
        {
            return true;
        }
        const String member = "\"hitboxes\": " + text;
        if (object.kids.empty())
        {
            m_text.insert(static_cast<size_t>(object.begin) + 1, " " + member + " ");
        }
        else
        {
            m_text.insert(static_cast<size_t>(scanner.nodes[static_cast<size_t>(object.kids.back())].end), ", " + member);
        }
        return Reparse_();
    }
    const JsonNode& array = scanner.nodes[static_cast<size_t>(kid)];
    if (m_text.compare(static_cast<size_t>(array.begin), static_cast<size_t>(array.end - array.begin), text) == 0)
    {
        return true;
    }
    if (!_boxes.empty())
    {
        m_text.replace(static_cast<size_t>(array.begin), static_cast<size_t>(array.end - array.begin), text);
        return Reparse_();
    }
    const size_t key = m_text.rfind("\"hitboxes\"", static_cast<size_t>(array.begin));
    if (key == String::npos)
    {
        return false;
    }
    size_t from = key;
    while (from > 0 && (m_text[from - 1] == ' ' || m_text[from - 1] == '\t' || m_text[from - 1] == '\r' || m_text[from - 1] == '\n'))
    {
        --from;
    }
    size_t to = static_cast<size_t>(array.end);
    if (from > 0 && m_text[from - 1] == ',')
    {
        from -= 1;
    }
    else
    {
        from = key;
        while (to < m_text.size() && (m_text[to] == ' ' || m_text[to] == '\t' || m_text[to] == '\r' || m_text[to] == '\n'))
        {
            ++to;
        }
        if (to < m_text.size() && m_text[to] == ',')
        {
            ++to;
        }
    }
    m_text.erase(from, to - from);
    return Reparse_();
}

void HitboxDocument::Revert()
{
    m_text = m_savedText;
    (void)Reparse_();
}

const String& HitboxDocument::GetText() const
{
    return m_text;
}

Span<const HitboxEntry> HitboxDocument::GetEntries() const
{
    return Span<const HitboxEntry> { m_entries.data(), m_entries.size() };
}

bool HitboxDocument::IsDirty() const
{
    return m_text != m_savedText;
}

bool HitboxDocument::Reparse_()
{
    JsonScanner scanner { m_text };
    const int   root = scanner.Parse();
    if (root < 0 || scanner.nodes[static_cast<size_t>(root)].type != 'o')
    {
        return false;
    }
    m_entries.clear();
    Collect_(scanner.nodes, root, String {}, m_defaultOwner);
    return true;
}

void HitboxDocument::Collect_(
    const Vector<JsonNode>& _nodes,
    const int               _index,
    const String&           _path,
    const String&           _owner)
{
    const JsonNode& node = _nodes[static_cast<size_t>(_index)];
    if (node.type == 'a')
    {
        for (size_t i = 0; i < node.kids.size(); ++i)
        {
            Collect_(_nodes, node.kids[i], std::format("{}[{}]", _path, i), _owner);
        }
        return;
    }
    if (node.type != 'o')
    {
        return;
    }

    const int w = FindKey_(node, "w");
    const int h = FindKey_(node, "h");
    if (IsNumberNode_(_nodes, w) && IsNumberNode_(_nodes, h))
    {
        HitboxEntry entry;
        entry.owner              = _owner;
        entry.label              = _path;
        const ARRAY<int, 4> kids = { w, h, FindKey_(node, "cx"), FindKey_(node, "cy") };
        for (size_t f = 0; f < kids.size(); ++f)
        {
            if (IsNumberNode_(_nodes, kids[f]))
            {
                const JsonNode& number = _nodes[static_cast<size_t>(kids[f])];
                entry.value[f]         = ParseNumber_(number.str);
                entry.begin[f]         = number.begin;
                entry.end[f]           = number.end;
            }
        }
        m_entries.push_back(entry);
    }
    for (size_t i = 0; i < node.kids.size(); ++i)
    {
        const String& key = node.keys[i];
        if (key == "hitboxes")
        {
            continue;
        }
        const int       kid       = node.kids[i];
        const JsonNode& kidNode   = _nodes[static_cast<size_t>(kid)];
        const String    childPath = _path.empty() ? key : _path + "." + key;
        const bool      bPair     = kidNode.type == 'a' && kidNode.kids.size() == 2 && IsSizeKey_(key) && IsNumberNode_(_nodes, kidNode.kids[0]) && IsNumberNode_(_nodes, kidNode.kids[1]);
        if (bPair)
        {
            HitboxEntry entry;
            entry.owner = _owner;
            entry.label = childPath;
            for (size_t f = 0; f < 2; ++f)
            {
                const JsonNode& number = _nodes[static_cast<size_t>(kidNode.kids[f])];
                entry.value[f]         = ParseNumber_(number.str);
                entry.begin[f]         = number.begin;
                entry.end[f]           = number.end;
            }
            const size_t sizePos = key.rfind("Size");
            if (sizePos != String::npos && sizePos + 4 == key.size())
            {
                const int offsetNode = FindKey_(node, key.substr(0, sizePos) + "Offset");
                if (offsetNode >= 0)
                {
                    const JsonNode& offsetArr = _nodes[static_cast<size_t>(offsetNode)];
                    if (offsetArr.type == 'a' && offsetArr.kids.size() == 2 && IsNumberNode_(_nodes, offsetArr.kids[0]) && IsNumberNode_(_nodes, offsetArr.kids[1]))
                    {
                        for (size_t f = 0; f < 2; ++f)
                        {
                            const JsonNode& number = _nodes[static_cast<size_t>(offsetArr.kids[f])];
                            entry.value[2 + f]     = ParseNumber_(number.str);
                            entry.begin[2 + f]     = number.begin;
                            entry.end[2 + f]       = number.end;
                        }
                    }
                }
            }
            m_entries.push_back(entry);
            continue;
        }
        const bool bOwnerLevel = _path == "defs" || _path == "projectiles";
        Collect_(_nodes, kid, childPath, bOwnerLevel ? key : _owner);
    }
}

void HitboxEditor::Load_(
    const FilePath& _assetRoot)
{
    m_bLoaded = true;
    for (size_t i = 0; i < kHitboxFileNames.size(); ++i)
    {
        const FilePath path = _assetRoot / kHitboxFileDirs[i] / kHitboxFileNames[i];
        HitboxDocument doc;
        (void)doc.Load(path, i == 0 ? "Player" : "(file)");
        m_fileNames.push_back(kHitboxFileNames[i]);
        m_filePaths.push_back(path);
        m_docs.push_back(std::move(doc));
    }
}

void HitboxEditor::ApplyLive_(
    SandboxState&      _state,
    const HitboxEntry& _entry)
{
    const float w  = static_cast<float>(_entry.value[0]);
    const float h  = static_cast<float>(_entry.value[1]);
    const float cx = static_cast<float>(_entry.value[2]);
    const float cy = static_cast<float>(_entry.value[3]);
    if (m_fileIndex == 0)
    {
        BodyTuning& body = _state.GetTuning().body;
        if (_entry.label == "body.standSize")
        {
            body.standSize = { w, h };
        }
        else if (_entry.label == "body.crouchSize")
        {
            body.crouchSize = { w, h };
        }
        else if (_entry.label == "body.hurtStand")
        {
            body.hurtStand = { w, h };
        }
        else if (_entry.label == "body.hurtCrouch")
        {
            body.hurtCrouch = { w, h };
        }
        return;
    }
    EnemyDef* pDef = _state.GetEnemyLibMutable().FindMutable(_entry.owner);
    if (pDef == nullptr)
    {
        return;
    }
    if (_entry.label == "defs." + _entry.owner + ".body")
    {
        pDef->body = BoxDef { w, h, cx, cy };
        _state.GetEnemies().RefreshBodies(_state.GetEnemyLib().FindIndex(_entry.owner), { w, h });
    }
    else if (_entry.label == "defs." + _entry.owner + ".hurtbox")
    {
        pDef->hurtbox = BoxDef { w, h, cx, cy };
    }
    else if (_entry.label == "defs." + _entry.owner + ".params.cannonBoxSize")
    {
        pDef->bigShiee.cannonBoxSize   = { w, h };
        pDef->bigShiee.cannonBoxOffset = { cx, cy };
    }
}

void HitboxEditor::Draw(
    EnemyLab& _lab)
{
    if (!m_bLoaded)
    {
        Load_(_lab.GetAssetRoot());
    }
    const ImGuiViewport* pHost = ImGui::GetMainViewport();
    ImGui::SetNextWindowPos(ImVec2 { pHost->Pos.x + pHost->Size.x - 360.f, pHost->Pos.y + 40.f }, ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowSize(ImVec2 { 340.f, 360.f }, ImGuiCond_FirstUseEver);
    if (ImGui::Begin("Hitbox"))
    {
        DrawEntries_(_lab, _lab.GetSandbox());
    }
    ImGui::End();
}

void HitboxEditor::DrawEntries_(
    EnemyLab&     _lab,
    SandboxState* _pState)
{
    if (m_docs.empty())
    {
        return;
    }
    if (ImGui::BeginCombo("File", m_fileNames[static_cast<size_t>(m_fileIndex)].c_str()))
    {
        for (int i = 0; i < static_cast<int>(m_fileNames.size()); ++i)
        {
            if (ImGui::Selectable(m_fileNames[static_cast<size_t>(i)].c_str(), i == m_fileIndex))
            {
                m_fileIndex  = i;
                m_ownerIndex = 0;
            }
        }
        ImGui::EndCombo();
    }
    HitboxDocument& doc = m_docs[static_cast<size_t>(m_fileIndex)];

    Vector<String> owners;
    for (const HitboxEntry& entry: doc.GetEntries())
    {
        bool bKnown = false;
        for (const String& owner: owners)
        {
            bKnown = bKnown || owner == entry.owner;
        }
        if (!bKnown)
        {
            owners.push_back(entry.owner);
        }
    }
    if (owners.empty())
    {
        ImGui::TextUnformatted("No boxes.");
        return;
    }
    if (m_ownerIndex >= static_cast<int>(owners.size()))
    {
        m_ownerIndex = 0;
    }
    if (ImGui::BeginCombo("Target", owners[static_cast<size_t>(m_ownerIndex)].c_str()))
    {
        for (int i = 0; i < static_cast<int>(owners.size()); ++i)
        {
            if (ImGui::Selectable(owners[static_cast<size_t>(i)].c_str(), i == m_ownerIndex))
            {
                m_ownerIndex = i;
            }
        }
        ImGui::EndCombo();
    }
    const String owner = owners[static_cast<size_t>(m_ownerIndex)];

    bool bOwnerActive = m_activeFile == m_fileIndex && m_activeEntry >= 0 && m_activeEntry < static_cast<int>(doc.GetEntries().size());
    bOwnerActive      = bOwnerActive && doc.GetEntries()[static_cast<size_t>(m_activeEntry)].owner == owner;
    for (int i = 0; i < static_cast<int>(doc.GetEntries().size()); ++i)
    {
        const HitboxEntry entry = doc.GetEntries()[static_cast<size_t>(i)];
        if (entry.owner != owner)
        {
            continue;
        }
        if (!bOwnerActive)
        {
            m_activeFile  = m_fileIndex;
            m_activeEntry = i;
            bOwnerActive  = true;
        }
        ImGui::PushID(i);
        const HitboxTitle title    = MakeHitboxTitle_(entry);
        const RGBA        tint     = HitboxKindColor_(title.kind);
        const ImVec2      blockMin = ImGui::GetCursorScreenPos();
        ImGui::TextColored(ImVec4 { tint.r / 255.f, tint.g / 255.f, tint.b / 255.f, 1.f }, "%s", title.ko.c_str());
        if (ImGui::IsItemHovered())
        {
            ImGui::SetTooltip("%s", entry.label.c_str());
        }
        ARRAY<float, 2> size    = { static_cast<float>(entry.value[0]), static_cast<float>(entry.value[1]) };
        bool            bEdited = ImGui::DragFloat2("w, h", size.data(), 0.5f, 0.f, 4000.f, "%.1f");
        ImGui::SameLine();
        if (ImGui::SmallButton("-"))
        {
            size[0] -= 1.f;
            size[1] -= 1.f;
            bEdited = true;
        }
        ImGui::SameLine();
        if (ImGui::SmallButton("+"))
        {
            size[0] += 1.f;
            size[1] += 1.f;
            bEdited = true;
        }
        ARRAY<float, 2> offset  = { static_cast<float>(entry.value[2]), static_cast<float>(entry.value[3]) };
        bool            bOffset = false;
        if (entry.HasField(eHitboxField::Cx))
        {
            bOffset = ImGui::DragFloat2("cx, cy", offset.data(), 0.5f, -4000.f, 4000.f, "%.1f");
        }
        const ImVec2 blockMax = { blockMin.x + ImGui::GetContentRegionAvail().x, ImGui::GetCursorScreenPos().y };
        if (bEdited || bOffset || ImGui::IsAnyItemActive() || (ImGui::IsWindowHovered() && ImGui::IsMouseHoveringRect(blockMin, blockMax)))
        {
            m_activeFile  = m_fileIndex;
            m_activeEntry = i;
        }
        if (bEdited || bOffset)
        {
            (void)doc.SetValue(i, eHitboxField::W, static_cast<double>(size[0]));
            (void)doc.SetValue(i, eHitboxField::H, static_cast<double>(size[1]));
            if (bOffset)
            {
                (void)doc.SetValue(i, eHitboxField::Cx, static_cast<double>(offset[0]));
                (void)doc.SetValue(i, eHitboxField::Cy, static_cast<double>(offset[1]));
            }
            if (_pState != nullptr)
            {
                ApplyLive_(*_pState, doc.GetEntries()[static_cast<size_t>(i)]);
            }
        }
        ImGui::PopID();
    }

    if (m_fileIndex > 0 && _pState != nullptr)
    {
        m_ownerName = owner;
        DrawExtraBoxes_(*_pState, doc);
    }

    ImGui::Separator();
    const bool bCtrlS = ImGui::GetIO().KeyCtrl && ImGui::IsKeyPressed(ImGuiKey_S, false);
    if (ImGui::Button("Save") || (bCtrlS && doc.IsDirty()))
    {
        if (doc.Save(m_filePaths[static_cast<size_t>(m_fileIndex)]))
        {
            m_status = "Saved " + m_fileNames[static_cast<size_t>(m_fileIndex)];
            if (m_fileIndex > 0 && _pState != nullptr)
            {
                (void)_lab.ReloadDefs(*_pState);
            }
        }
        else
        {
            m_status = "Save failed.";
        }
    }
    ImGui::SameLine();
    if (ImGui::Button("Revert"))
    {
        doc.Revert();
        if (m_fileIndex > 0 && _pState != nullptr)
        {
            (void)_lab.ReloadDefs(*_pState);
        }
        m_status = "Reverted.";
    }
    ImGui::SameLine();
    ImGui::TextUnformatted(doc.IsDirty() ? "modified" : "saved");
    if (!m_status.empty())
    {
        ImGui::TextUnformatted(m_status.c_str());
    }
}

void HitboxEditor::DrawExtraBoxes_(
    SandboxState&   _state,
    HitboxDocument& _doc)
{
    EnemyDef* pDef = _state.GetEnemyLibMutable().FindMutable(m_ownerName);
    if (pDef == nullptr)
    {
        return;
    }
    ImGui::Separator();
    ImGui::TextUnformatted("생성 상자 (hitboxes)");
    ImGui::Combo("종류", &m_newKind, "피격 (hurt)\0공격 (attack)\0");
    ImGui::SameLine();
    bool bChanged = false;
    if (ImGui::Button("+ 추가"))
    {
        ExtraBoxDef box;
        box.id   = std::format("box{}", pDef->hitboxes.size() + 1);
        box.kind = m_newKind == 1 ? eExtraBoxKind::Attack : eExtraBoxKind::Hurt;
        box.box  = { 30.f, 30.f, 0.f, -20.f };
        m_boxSel = static_cast<int>(pDef->hitboxes.size());
        pDef->hitboxes.push_back(box);
        bChanged = true;
    }
    int removeAt = -1;
    for (int i = 0; i < static_cast<int>(pDef->hitboxes.size()); ++i)
    {
        ExtraBoxDef& box     = pDef->hitboxes[static_cast<size_t>(i)];
        const bool   bAttack = box.kind == eExtraBoxKind::Attack;
        const RGBA   tint    = HitboxKindColor_(bAttack ? eHitboxKind::Attack : eHitboxKind::Hurt);
        ImGui::PushID(i);
        ImGui::TextColored(ImVec4 { tint.r / 255.f, tint.g / 255.f, tint.b / 255.f, 1.f }, "%s%s (%s)", i == m_boxSel ? "> " : "", box.id.c_str(), bAttack ? "공격" : "피격");
        ARRAY<float, 2> size   = { box.box.w, box.box.h };
        ARRAY<float, 2> offset = { box.box.cx, box.box.cy };
        bool            bEdit  = ImGui::DragFloat2("w, h", size.data(), 0.5f, 1.f, 4000.f, "%.1f");
        bEdit                  = ImGui::DragFloat2("offset", offset.data(), 0.5f, -4000.f, 4000.f, "%.1f") || bEdit;
        if (bEdit)
        {
            box.box = { size[0], size[1], offset[0], offset[1] };
        }
        if (bAttack)
        {
            ARRAY<int, 2> frames = { box.frameFrom, box.frameTo };
            if (ImGui::DragInt2("frames (-1=항상)", frames.data(), 0.2f, -1, 200))
            {
                box.frameFrom = frames[0];
                box.frameTo   = frames[1];
                bEdit         = true;
            }
            bEdit = ImGui::DragInt("damage", &box.damage, 0.1f, 1, 99) || bEdit;
        }
        if (ImGui::SmallButton("삭제"))
        {
            removeAt = i;
        }
        if (bEdit || ImGui::IsAnyItemActive())
        {
            m_boxSel = i;
        }
        bChanged = bChanged || bEdit;
        ImGui::PopID();
    }
    if (removeAt >= 0)
    {
        pDef->hitboxes.erase(pDef->hitboxes.begin() + removeAt);
        m_boxSel = -1;
        bChanged = true;
    }
    if (bChanged && !_doc.ApplyExtraBoxes(m_ownerName, pDef->hitboxes))
    {
        m_status = "이 파일에 정의가 없어 파일에 반영하지 못했습니다.";
    }
}

void HitboxEditor::DrawExtraOverlay_(
    EnemyLab& _lab,
    Renderer& _renderer)
{
    SandboxState* pState = _lab.GetSandbox();
    if (pState == nullptr || m_fileIndex <= 0 || m_ownerName.empty() || m_docs.empty())
    {
        return;
    }
    EnemyDef* pDef = pState->GetEnemyLibMutable().FindMutable(m_ownerName);
    if (pDef == nullptr || pDef->hitboxes.empty())
    {
        m_bBoxDrag = false;
        return;
    }
    const Vec2 camRaw = pState->GetCamera().GetPositionNoShake();
    const Vec2 camera = { std::floor(camRaw.x), std::floor(camRaw.y) };

    float      mouseX = 0.f;
    float      mouseY = 0.f;
    const bool bDown  = (SDL_GetMouseState(&mouseX, &mouseY) & SDL_BUTTON_LMASK) != 0;
    const bool bClick = bDown && !m_bPrevDown;
    m_bPrevDown       = bDown;
    float      lx     = 0.f;
    float      ly     = 0.f;
    const bool bMouse = SDL_RenderCoordinatesFromWindow(_renderer.GetNative(), mouseX, mouseY, &lx, &ly) && !ImGui::GetIO().WantCaptureMouse;
    const Vec2 mouse  = { lx + camera.x, ly + camera.y };
    if (!bDown)
    {
        m_bBoxDrag = false;
    }

    const int defIndex = pState->GetEnemyLib().FindIndex(m_ownerName);
    bool      bHover   = false;
    for (const Enemy& e: pState->GetEnemies().GetEnemies())
    {
        if (e.life != eEnemyLife::Alive || e.defIndex != defIndex)
        {
            continue;
        }
        const float facing = static_cast<float>(e.facing < 0 ? -1 : 1);
        const Vec2  local  = { (mouse.x - e.body.position.x) * facing, mouse.y - e.body.position.y };
        for (int i = 0; i < static_cast<int>(pDef->hitboxes.size()); ++i)
        {
            const ExtraBoxDef& box   = pDef->hitboxes[static_cast<size_t>(i)];
            const bool         bSel  = i == m_boxSel;
            const Aabb         world = box.box.At(e.body.position, e.facing);
            const RGBA         color = box.kind == eExtraBoxKind::Attack ? RGBA { 255, 50, 50, 255 } : RGBA { 60, 255, 90, 255 };
            _lab.DrawMark(_renderer, world, { world.GetCenter().x, world.GetCenter().y }, color, bSel ? "> " + box.id : box.id);
            const bool bIn = bMouse && std::fabs(local.x - box.box.cx) <= box.box.w * 0.5f && std::fabs(local.y - box.box.cy) <= box.box.h * 0.5f;
            bHover         = bHover || bIn;
            if (bClick && bIn && !m_bBoxDrag)
            {
                m_boxSel    = i;
                m_bBoxDrag  = true;
                m_dragEnemy = e.id;
                m_dragGrab  = { box.box.cx - local.x, box.box.cy - local.y };
            }
        }
    }
    if (m_bBoxDrag && m_boxSel >= 0 && m_boxSel < static_cast<int>(pDef->hitboxes.size()))
    {
        for (const Enemy& e: pState->GetEnemies().GetEnemies())
        {
            if (e.id != m_dragEnemy)
            {
                continue;
            }
            const float  facing = static_cast<float>(e.facing < 0 ? -1 : 1);
            ExtraBoxDef& box    = pDef->hitboxes[static_cast<size_t>(m_boxSel)];
            box.box.cx          = std::round((mouse.x - e.body.position.x) * facing + m_dragGrab.x);
            box.box.cy          = std::round(mouse.y - e.body.position.y + m_dragGrab.y);
            (void)m_docs[static_cast<size_t>(m_fileIndex)].ApplyExtraBoxes(m_ownerName, pDef->hitboxes);
        }
    }
    if (bHover || m_bBoxDrag)
    {
        _lab.SetMouseCaptured(true);
    }
}

void HitboxEditor::DrawOverlay(
    EnemyLab& _lab,
    Renderer& _renderer)
{
    DrawExtraOverlay_(_lab, _renderer);
    SandboxState* pState = _lab.GetSandbox();
    if (pState == nullptr || m_activeFile < 0 || m_activeFile >= static_cast<int>(m_docs.size()))
    {
        return;
    }
    const Span<const HitboxEntry> entries = m_docs[static_cast<size_t>(m_activeFile)].GetEntries();
    if (m_activeEntry < 0 || m_activeEntry >= static_cast<int>(entries.size()))
    {
        return;
    }
    const HitboxEntry& entry = entries[static_cast<size_t>(m_activeEntry)];
    const HitboxTitle  title = MakeHitboxTitle_(entry);
    const RGBA         color = HitboxKindColor_(title.kind);
    const Vec2         size  = { static_cast<float>(entry.value[0]), static_cast<float>(entry.value[1]) };
    if (m_activeFile == 0)
    {
        const PlayerController& player = pState->GetPlayer();
        const Vec2              feet   = player.GetFeet();
        if (title.kind == eHitboxKind::Attack)
        {
            const Vec2 center = { feet.x + static_cast<float>(player.GetFacing()) * 30.f, feet.y - 30.f };
            _lab.DrawMark(_renderer, Aabb::FromCenterSize(center, size), center, color, title.en);
        }
        else
        {
            _lab.DrawMark(_renderer, Aabb::FromBottomCenter(feet, size), feet, color, title.en);
        }
        return;
    }
    const BoxDef def = { size.x, size.y, static_cast<float>(entry.value[2]), static_cast<float>(entry.value[3]) };
    for (const Enemy& e: pState->GetEnemies().GetEnemies())
    {
        if (e.life != eEnemyLife::Alive || pState->GetEnemyLib().Get(e.defIndex).name != entry.owner)
        {
            continue;
        }
        const Aabb box = title.kind == eHitboxKind::Body ? e.body.GetBox() : def.At(e.body.position, e.facing);
        _lab.DrawMark(_renderer, box, e.body.position, color, title.en);
    }
}
