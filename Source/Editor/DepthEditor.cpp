#include "Editor/DepthEditor.h"

#include <algorithm>
#include <format>
#include <fstream>
#include <iterator>

#include <imgui.h>

#include "Editor/EnemyLab.h"
#include "Editor/JsonScan.h"
#include "Editor/MotionEditor.h"
#include "Game/Enemies/BigShieeCfg.h"
#include "Game/Enemies/EnemyDef.h"
#include "Game/Enemies/EnemyWorld.h"
#include "Game/Player/SlugRender.h"
#include "Game/Props/DecorWorld.h"
#include "Game/Props/ItemWorld.h"
#include "Game/Props/PropWorld.h"
#include "Game/SandboxState.h"
#include "Game/Stage/StageData.h"
#include "Render/SpriteBatch.h"

static constexpr ARRAY<const char*, 6> kDepthFileDirs  = { "Defs", "Defs", "Defs", "Defs", "Defs", "Defs/Props" };
static constexpr ARRAY<const char*, 6> kDepthFileNames = { "Enemies.json", "BigShiee.json", "Bus.json", "Helicopter.json", "Vehicles.json", "Props.json" };

static void AddConst_(
    Vector<DepthRow>& _rows,
    const char*       _name,
    const int         _value)
{
    DepthRow row;
    row.name  = _name;
    row.value = _value;
    _rows.push_back(row);
}

static void Collect_(
    const Vector<JsonNode>& _nodes,
    const int               _index,
    const String&           _path,
    const int               _file,
    const String&           _fileName,
    Vector<DepthRow>&       _out)
{
    const JsonNode& node = _nodes[static_cast<size_t>(_index)];
    if (node.type != 'o' && node.type != 'a')
    {
        return;
    }
    for (size_t i = 0; i < node.kids.size(); ++i)
    {
        const String    key  = node.type == 'o' ? node.keys[i] : std::format("[{}]", i);
        const String    path = _path.empty() ? key : (node.type == 'o' ? _path + "." + key : _path + key);
        const JsonNode& kid  = _nodes[static_cast<size_t>(node.kids[i])];
        if (kid.type == 'n' && node.type == 'o' && (key == "depth" || key == "fxDepth") && _path.find("params.parts.") == String::npos)
        {
            DepthRow row;
            row.name  = _fileName + " " + path;
            row.value = static_cast<int>(std::strtol(kid.str.c_str(), nullptr, 10));
            row.file  = _file;
            row.begin = kid.begin;
            row.end   = kid.end;
            _out.push_back(row);
        }
        else
        {
            Collect_(_nodes, node.kids[i], path, _file, _fileName, _out);
        }
    }
}

void DepthEditor::Load_(
    const FilePath& _assetRoot)
{
    m_bLoaded = true;
    for (size_t i = 0; i < kDepthFileNames.size(); ++i)
    {
        const FilePath path = _assetRoot / kDepthFileDirs[i] / kDepthFileNames[i];
        std::ifstream  file { path, std::ios::binary };
        String         text;
        if (file)
        {
            text.assign(std::istreambuf_iterator<char>(file), std::istreambuf_iterator<char>());
        }
        m_names.push_back(kDepthFileNames[i]);
        m_paths.push_back(path);
        m_texts.push_back(text);
        m_dirty.push_back(false);
    }
    Rebuild_();
}

void DepthEditor::Rebuild_()
{
    m_rows.clear();
    AddConst_(m_rows, "Backdrop", kBackdropDepth);
    AddConst_(m_rows, "Terrain (stage world.terrainDepth default)", kDefaultTerrainDepth);
    AddConst_(m_rows, "BigShiee hull", kBigShieeDepthHull);
    AddConst_(m_rows, "BigShiee antenna", kBigShieeDepthAntenna);
    AddConst_(m_rows, "BigShiee cannon", kBigShieeDepthCannon);
    AddConst_(m_rows, "BigShiee turret", kBigShieeDepthTurret);
    AddConst_(m_rows, "BigShiee death disc", kBigShieeDepthDisc);
    AddConst_(m_rows, "Decor far", kDecorFarDepth);
    AddConst_(m_rows, "Decor back", kDecorBackDepth);
    AddConst_(m_rows, "Prop back (default)", kPropDepthBack);
    AddConst_(m_rows, "Enemy overlay (parachute)", kOverlayDepth);
    AddConst_(m_rows, "Enemy body", kEnemyDepth);
    AddConst_(m_rows, "Slug body", kSlugBodyDepth);
    AddConst_(m_rows, "Player", kPlayerDepth);
    AddConst_(m_rows, "Enemy projectile trail", kTrailDepth);
    AddConst_(m_rows, "Player bullet", kPlayerBulletDepth);
    AddConst_(m_rows, "Slug shell", kPlayerBombDepth);
    AddConst_(m_rows, "Enemy projectile", kShotDepth);
    AddConst_(m_rows, "Slug FX", kSlugFxDepth);
    AddConst_(m_rows, "Item", kItemRenderDepth);
    AddConst_(m_rows, "Slug arrow", kSlugArrowDepth);
    AddConst_(m_rows, "Prop front", kPropDepthFront);
    AddConst_(m_rows, "Prop debris", kPropDebrisDepth);
    AddConst_(m_rows, "Score popup", kPopupRenderDepth);
    AddConst_(m_rows, "Prop explosion FX (default)", kPropEffectDepth);
    AddConst_(m_rows, "Bullet hit FX", kBulletHitDepth);
    AddConst_(m_rows, "Bomb blast FX", kBombBlastDepth);
    AddConst_(m_rows, "HUD/UI", kHudDepth);
    for (size_t i = 0; i < m_texts.size(); ++i)
    {
        JsonScanner scanner { m_texts[i] };
        const int   root = scanner.Parse();
        if (root >= 0)
        {
            Collect_(scanner.nodes, root, {}, static_cast<int>(i), m_names[i], m_rows);
        }
    }
}

void DepthEditor::Patch_(
    const int _row,
    const int _value)
{
    const DepthRow& row  = m_rows[static_cast<size_t>(_row)];
    String&         text = m_texts[static_cast<size_t>(row.file)];
    text.replace(static_cast<size_t>(row.begin), static_cast<size_t>(row.end - row.begin), std::to_string(_value));
    m_dirty[static_cast<size_t>(row.file)] = true;
    Rebuild_();
}

void DepthEditor::PatchPart_(
    const StringView       _def,
    const StringView       _part,
    const EnemyPartDepth&  _value,
    const EnemyPartDepth&  _default)
{
    for (size_t i = 0; i < m_texts.size(); ++i)
    {
        MotionDocument doc;
        if (!doc.LoadFromText(m_texts[i]))
        {
            continue;
        }
        JsonScanner scanner { m_texts[i] };
        const int   root = scanner.Parse();
        bool        bHas = false;
        if (root >= 0)
        {
            for (size_t k = 0; k < scanner.nodes[static_cast<size_t>(root)].keys.size(); ++k)
            {
                if (scanner.nodes[static_cast<size_t>(root)].keys[k] != "defs")
                {
                    continue;
                }
                const JsonNode& defs = scanner.nodes[static_cast<size_t>(scanner.nodes[static_cast<size_t>(root)].kids[k])];
                for (const String& key: defs.keys)
                {
                    bHas = bHas || key == _def;
                }
            }
        }
        if (!bHas)
        {
            continue;
        }
        if (doc.SetPartDepth(_def, _part, _value.depth, _value.sort, _default.depth, _default.sort) && doc.GetText() != m_texts[i])
        {
            m_texts[i] = doc.GetText();
            m_dirty[i] = true;
        }
        return;
    }
}

void DepthEditor::DrawParts_(
    EnemyLab& _lab)
{
    SandboxState* pState = _lab.GetSandbox();
    if (pState == nullptr || !ImGui::CollapsingHeader("Parts (params.parts.<name>.{depth,sort})", ImGuiTreeNodeFlags_DefaultOpen))
    {
        return;
    }
    ImGui::TextDisabled("depth = absolute layer at default z; sort = tie key (feet-sorted parts: offset added to feet y). Instance z shifts all parts.");
    EnemyDefLibrary&     lib = pState->GetEnemyLibMutable();
    Vector<String>       names;
    for (const EnemyDef& def: lib.GetDefs())
    {
        names.push_back(def.name);
    }
    for (const String& name: names)
    {
        EnemyDef* pDef = lib.FindMutable(name);
        if (pDef == nullptr)
        {
            continue;
        }
        Vector<EnemyPartSlot> slots;
        GetEnemyPartSlots(*pDef, slots);
        if (slots.empty())
        {
            continue;
        }
        EnemyDef base;
        base.behavior    = pDef->behavior;
        base.overlayClip = pDef->overlayClip;
        Vector<EnemyPartSlot> defaults;
        GetEnemyPartSlots(base, defaults);
        ImGui::PushID(name.c_str());
        if (ImGui::TreeNode(name.c_str()))
        {
            for (size_t i = 0; i < slots.size() && i < defaults.size(); ++i)
            {
                EnemyPartDepth& part = *slots[i].pDepth;
                ARRAY<int, 2>   v    = { part.depth, part.sort };
                ImGui::PushID(static_cast<int>(i));
                ImGui::SetNextItemWidth(140.f);
                if (ImGui::DragInt2(slots[i].pName, v.data(), 0.2f))
                {
                    part.depth = v[0];
                    part.sort  = v[1];
                    PatchPart_(name, slots[i].pName, part, *defaults[i].pDepth);
                }
                ImGui::PopID();
            }
            ImGui::TreePop();
        }
        ImGui::PopID();
    }
}

void DepthEditor::Draw(
    EnemyLab& _lab)
{
    if (!m_bLoaded)
    {
        Load_(_lab.GetAssetRoot());
    }
    if (!ImGui::Begin("Depth"))
    {
        ImGui::End();
        return;
    }
    ImGui::TextUnformatted("Smaller = farther back. Ties: feet y, then submit order. Code constants are read-only.");
    ImGui::TextDisabled("These are defaults: an instance z (stage spawn \"z\", Map/AI tab Z field) overrides them.");
    Vector<int> order;
    for (size_t i = 0; i < m_rows.size(); ++i)
    {
        order.push_back(static_cast<int>(i));
    }
    std::stable_sort(order.begin(), order.end(), [this](const int _a, const int _b) { return m_rows[static_cast<size_t>(_a)].value < m_rows[static_cast<size_t>(_b)].value; });
    int editRow   = -1;
    int editValue = 0;
    for (const int index: order)
    {
        const DepthRow& row = m_rows[static_cast<size_t>(index)];
        ImGui::PushID(index);
        if (row.file < 0)
        {
            ImGui::Text("%6d  %s", row.value, row.name.c_str());
        }
        else
        {
            int value = row.value;
            ImGui::SetNextItemWidth(80.f);
            if (ImGui::DragInt("##d", &value, 0.2f) && value != row.value)
            {
                editRow   = index;
                editValue = value;
            }
            ImGui::SameLine();
            ImGui::TextUnformatted(row.name.c_str());
        }
        ImGui::PopID();
    }
    if (editRow >= 0)
    {
        Patch_(editRow, editValue);
    }
    DrawParts_(_lab);
    ImGui::Separator();
    if (ImGui::Button("Save"))
    {
        bool bAny = false;
        for (size_t i = 0; i < m_texts.size(); ++i)
        {
            if (!m_dirty[i])
            {
                continue;
            }
            std::ofstream file { m_paths[i], std::ios::binary | std::ios::trunc };
            file.write(m_texts[i].data(), static_cast<std::streamsize>(m_texts[i].size()));
            m_dirty[i] = false;
            bAny       = true;
        }
        if (bAny && _lab.GetSandbox() != nullptr)
        {
            (void)_lab.ReloadDefs(*_lab.GetSandbox());
        }
        m_status = bAny ? "Saved + reloaded defs." : "Nothing to save.";
    }
    ImGui::SameLine();
    ImGui::TextUnformatted(m_status.c_str());
    ImGui::End();
}
