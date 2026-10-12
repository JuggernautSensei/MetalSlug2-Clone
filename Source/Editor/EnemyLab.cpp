#include "Editor/EnemyLab.h"

#include <cmath>
#include <filesystem>
#include <format>

#include <SDL3/SDL.h>
#include <imgui.h>

#include "Base/Log.h"
#include "Base/RGBA.h"
#include "Game/Enemies/EnemyDef.h"
#include "Game/SandboxState.h"
#include "Game/SimRunner.h"
#include "Game/StateFactory.h"
#include "Render/Renderer.h"

constexpr int   kLabProbeDepth = 64;
constexpr float kLabPlayerGap  = 120.f;

namespace
{
void LabDrawText_(
    Renderer&        _renderer,
    const Vec2       _topLeft,
    const StringView _text,
    const RGBA       _color)
{
    _renderer.DrawDebugText({ _topLeft.x + 1.f, _topLeft.y + 1.f }, _text, { 0, 0, 0, 255 });
    _renderer.DrawDebugText(_topLeft, _text, _color);
}

[[nodiscard]] const char* LabLifeName_(
    const eEnemyLife _life)
{
    switch (_life)
    {
        case eEnemyLife::Pending: return "Pending";
        case eEnemyLife::Alive: return "Alive";
        case eEnemyLife::Dying: return "Dying";
        case eEnemyLife::Gone: return "Gone";
    }
    return "?";
}

[[nodiscard]] String LabMakeStateText_(
    const Enemy& _e)
{
    return std::format("st{} t{} c{} hp{}", _e.state, _e.stateTicks, _e.counter, _e.health.hp);
}
}   // namespace

void EnemyLab::Init(
    SimRunner*      _pRunner,
    const String&   _stateName,
    const FilePath& _assetRoot)
{
    m_pRunner   = _pRunner;
    m_assetRoot = _assetRoot;
    if (_stateName != "enemylab" && !_stateName.empty())
    {
        m_returnState = _stateName;
    }
}

void EnemyLab::SetAutoSpawn(
    const String& _defName)
{
    m_autoSpawn = _defName;
}

EnemyHandle EnemyLab::Spawn(
    SandboxState&    _state,
    const StringView _defName,
    const Vec2       _feet,
    const int        _facing,
    const int        _z)
{
    SpawnParams params;
    params.facing            = _facing < 0 ? -1 : 1;
    params.z                 = _z;
    params.bActivated        = true;
    const EnemyHandle handle = _state.SpawnEnemy(_defName, _feet, params);
    if (!handle.IsNull())
    {
        m_set.push_back({ String { _defName }, _feet, params.facing, _z });
    }
    return handle;
}

EnemyHandle EnemyLab::SpawnNearPlayer(
    SandboxState&    _state,
    const StringView _defName,
    const float      _offsetX,
    const int        _facing,
    const int        _z)
{
    const Vec2 feet = _state.GetPlayer().GetFeet();
    return Spawn(_state, _defName, { feet.x + _offsetX, feet.y }, _facing, _z);
}

void EnemyLab::KillAll(
    SandboxState& _state)
{
    _state.GetEnemies().KillAll(false);
}

void EnemyLab::Clear(
    SandboxState& _state)
{
    _state.GetEnemies().Reset(1);
    _state.GetProjectiles().Clear();
    m_selected = {};
}

int EnemyLab::RespawnLast(
    SandboxState& _state)
{
    const Vector<EnemyLabSpawn> set = m_set;
    Clear(_state);
    m_set.clear();
    int count = 0;
    for (const EnemyLabSpawn& spawn: set)
    {
        if (!Spawn(_state, spawn.name, spawn.feet, spawn.facing, spawn.z).IsNull())
        {
            ++count;
        }
    }
    return count;
}

bool EnemyLab::ReloadDefs(
    SandboxState& _state)
{
    if (!_state.ReloadEnemyDefs(m_assetRoot))
    {
        return false;
    }
    m_selected = {};
    m_defIndex = 0;
    (void)RespawnLast(_state);
    return true;
}

bool EnemyLab::ReloadDefsKeep(
    SandboxState& _state)
{
    if (_state.ReloadEnemyDefsKeep(m_assetRoot))
    {
        m_status = "Defs reloaded in place (enemies kept).";
        return true;
    }
    const bool bOk = ReloadDefs(_state);
    m_status       = bOk ? "Def set changed: reloaded and respawned." : "Reload failed (see log).";
    return bOk;
}

void EnemyLab::SetAutoReloadBlocked(
    const bool _bBlocked)
{
    m_bAutoBlocked = _bBlocked;
}

void EnemyLab::SetMouseCaptured(
    const bool _bCaptured)
{
    m_bMouseCaptured = _bCaptured;
}

int EnemyLab::GetDefIndex() const
{
    return m_defIndex;
}

void EnemyLab::PollDefFiles_(
    SandboxState& _state)
{
    if (m_frame % 60 != 0)
    {
        return;
    }
    int64_t         stamp = 0;
    std::error_code ec;
    for (const std::filesystem::directory_entry& entry: std::filesystem::directory_iterator(m_assetRoot / "Defs", ec))
    {
        if (entry.is_regular_file(ec) && entry.path().extension() == ".json")
        {
            const int64_t t = static_cast<int64_t>(entry.last_write_time(ec).time_since_epoch().count());
            stamp           = t > stamp ? t : stamp;
        }
    }
    if (m_defsStamp == 0 || m_bAutoBlocked)
    {
        m_defsStamp = m_bAutoBlocked ? m_defsStamp : stamp;
        return;
    }
    if (stamp != m_defsStamp)
    {
        m_defsStamp = stamp;
        (void)ReloadDefsKeep(_state);
    }
}

void EnemyLab::ForgetSet()
{
    m_set.clear();
}

void EnemyLab::SelectAt(
    const SandboxState& _state,
    const Vec2          _world)
{
    m_selected                 = {};
    const EnemyDefLibrary& lib = _state.GetEnemyLib();
    for (const Enemy& e: _state.GetEnemies().GetEnemies())
    {
        if (e.life == eEnemyLife::Pending || e.life == eEnemyLife::Gone)
        {
            continue;
        }
        const EnemyDef& def = lib.Get(e.defIndex);
        if (def.hurtbox.At(e.GetFeet(), e.facing).Contains(_world) || e.body.GetBox().Contains(_world))
        {
            m_selected.id = e.id;
            return;
        }
    }
}

EnemyHandle EnemyLab::GetSelected() const
{
    return m_selected;
}

size_t EnemyLab::GetSetCount() const
{
    return m_set.size();
}

void EnemyLab::SetPaused(
    const bool _bPaused)
{
    m_bPaused = _bPaused;
}

void EnemyLab::RequestSteps(
    const int _count)
{
    if (_count > 0)
    {
        m_pendingSteps += _count;
    }
}

void EnemyLab::SetTimeScale(
    const float _scale)
{
    m_timeScale = _scale < 0.05f ? 0.05f : _scale;
}

float EnemyLab::GetTimeScale() const
{
    return m_timeScale;
}

int EnemyLab::TakeSteps(
    const int _naturalSteps)
{
    if (!m_bPaused)
    {
        m_pendingSteps = 0;
        return _naturalSteps;
    }
    const int steps = m_pendingSteps;
    m_pendingSteps  = 0;
    return steps;
}

const FilePath& EnemyLab::GetAssetRoot() const
{
    return m_assetRoot;
}

SandboxState* EnemyLab::GetSandbox() const
{
    if (!m_pRunner)
    {
        return nullptr;
    }
    return dynamic_cast<SandboxState*>(m_pRunner->GetState());
}

void EnemyLab::EnterLab()
{
    if (!m_pRunner)
    {
        return;
    }
    Scoped<IGameState> pState = CreateState("enemylab");
    if (pState)
    {
        m_pRunner->SwitchState(std::move(pState));
        m_set.clear();
        m_selected = {};
    }
}

void EnemyLab::LeaveLab()
{
    if (!m_pRunner)
    {
        return;
    }
    Scoped<IGameState> pState = CreateState(m_returnState);
    if (pState)
    {
        m_pRunner->SwitchState(std::move(pState));
        m_set.clear();
        m_selected = {};
    }
}

void EnemyLab::Draw()
{
    SandboxState* pState = GetSandbox();
    if (!pState)
    {
        ImGui::TextWrapped("Not in a sandbox state.");
        if (ImGui::Button("Enter lab"))
        {
            EnterLab();
        }
        return;
    }

    if (ImGui::Button("Back"))
    {
        LeaveLab();
        return;
    }
    ImGui::SameLine();
    ImGui::Text("to '%s'   alive %d", m_returnState.c_str(), pState->GetEnemies().CountAlive());
    if (!m_status.empty())
    {
        ImGui::TextUnformatted(m_status.c_str());
    }

    if (ImGui::CollapsingHeader("Time", ImGuiTreeNodeFlags_DefaultOpen))
    {
        if (ImGui::Checkbox("Pause", &m_bPaused))
        {
            m_pendingSteps = 0;
        }
        ImGui::SameLine();
        if (ImGui::Button("Step 1"))
        {
            m_bPaused = true;
            RequestSteps(1);
        }
        ImGui::SameLine();
        if (ImGui::Button("Step 10"))
        {
            m_bPaused = true;
            RequestSteps(10);
        }
        ImGui::Text("Speed");
        static constexpr ARRAY<float, 5> kScales = { 0.25f, 0.5f, 1.f, 1.5f, 2.f };
        for (const float scale: kScales)
        {
            ImGui::SameLine();
            const String label = std::format("{}x##scale{}", scale, static_cast<int>(scale * 100.f));
            if (ImGui::RadioButton(label.c_str(), std::abs(m_timeScale - scale) < 0.001f))
            {
                SetTimeScale(scale);
            }
        }
    }

    DrawSpawnUi_(*pState);

    if (ImGui::CollapsingHeader("Overlay", ImGuiTreeNodeFlags_DefaultOpen))
    {
        ImGui::Checkbox("Boxes", &m_bShowBoxes);
        ImGui::SameLine();
        ImGui::Checkbox("AI state", &m_bShowState);
        ImGui::SameLine();
        ImGui::Checkbox("Target line", &m_bShowTarget);
        ImGui::Checkbox("Ground probe", &m_bShowProbe);
        ImGui::SameLine();
        ImGui::Checkbox("Camera bounds", &m_bShowCamera);
        ImGui::SameLine();
        ImGui::Checkbox("Solids", &m_bShowSolids);
    }

    DrawInspector_(*pState);
}

void EnemyLab::DrawSpawnUi_(
    SandboxState& _state)
{
    if (!ImGui::CollapsingHeader("Spawn", ImGuiTreeNodeFlags_DefaultOpen))
    {
        return;
    }
    const Span<const EnemyDef> defs = _state.GetEnemyLib().GetDefs();
    if (defs.empty())
    {
        ImGui::TextUnformatted("No enemy defs loaded.");
    }
    else
    {
        if (m_defIndex < 0 || m_defIndex >= static_cast<int>(defs.size()))
        {
            m_defIndex = 0;
        }
        if (ImGui::BeginCombo("Def", defs[static_cast<size_t>(m_defIndex)].name.c_str()))
        {
            for (int i = 0; i < static_cast<int>(defs.size()); ++i)
            {
                if (ImGui::Selectable(defs[static_cast<size_t>(i)].name.c_str(), i == m_defIndex))
                {
                    m_defIndex = i;
                }
            }
            ImGui::EndCombo();
        }
    }
    ImGui::SliderFloat("Offset X", &m_offsetX, -300.f, 300.f);
    ImGui::RadioButton("Face left", &m_facing, -1);
    ImGui::SameLine();
    ImGui::RadioButton("Face right", &m_facing, 1);
    {
        const int defZ = defs.empty() ? kEnemyDepth : GetEnemyDefaultZ(defs[static_cast<size_t>(m_defIndex)].behavior);
        int       z    = m_spawnZ == kDefaultZ ? defZ : m_spawnZ;
        if (ImGui::InputInt("Z", &z))
        {
            m_spawnZ = z == defZ ? kDefaultZ : z;
        }
    }
    if (!defs.empty())
    {
        const String name = defs[static_cast<size_t>(m_defIndex)].name;
        if (ImGui::Button("Spawn at player"))
        {
            (void)SpawnNearPlayer(_state, name, m_offsetX, m_facing, m_spawnZ);
        }
        ImGui::SameLine();
    }
    ImGui::Checkbox("Click to place", &m_bPlaceMode);
    if (ImGui::Button("Kill all"))
    {
        KillAll(_state);
    }
    ImGui::SameLine();
    if (ImGui::Button("Clear"))
    {
        Clear(_state);
    }
    ImGui::SameLine();
    if (ImGui::Button("Respawn last set"))
    {
        (void)RespawnLast(_state);
    }
    ImGui::SameLine();
    if (ImGui::Button("Forget set"))
    {
        ForgetSet();
    }
    ImGui::Text("Set: %d spawns", static_cast<int>(m_set.size()));
    if (ImGui::Button("Reload defs + respawn"))
    {
        m_status = ReloadDefs(_state) ? "Defs reloaded." : "Reload failed (see log).";
    }
    ImGui::SameLine();
    if (ImGui::Button("Reload defs (keep enemies)"))
    {
        (void)ReloadDefsKeep(_state);
    }
}

void EnemyLab::DrawInspector_(
    SandboxState& _state)
{
    if (!ImGui::CollapsingHeader("Inspector", ImGuiTreeNodeFlags_DefaultOpen))
    {
        return;
    }
    const Enemy* pEnemy = _state.GetEnemies().Get(m_selected);
    if (!pEnemy)
    {
        ImGui::TextUnformatted("Click an enemy in the world.");
        return;
    }
    const EnemyDef& def = _state.GetEnemyLib().Get(pEnemy->defIndex);
    ImGui::Text("def %s  (%s)", def.name.c_str(), GetBehaviorName(def.behavior));
    ImGui::Text("life %s  active %d  grounded %d", LabLifeName_(pEnemy->life), pEnemy->bActive ? 1 : 0, pEnemy->bGrounded ? 1 : 0);
    ImGui::Text("hp %d / %d", pEnemy->health.hp, pEnemy->health.maxHp);
    ImGui::Text("pos %.1f, %.1f", static_cast<double>(pEnemy->body.position.x), static_cast<double>(pEnemy->body.position.y));
    ImGui::Text("vel %.1f, %.1f", static_cast<double>(pEnemy->body.velocity.x), static_cast<double>(pEnemy->body.velocity.y));
    ImGui::Text("facing %d  variant %d", pEnemy->facing, pEnemy->aiVariant);
    int z = pEnemy->z;
    if (ImGui::InputInt("Z", &z))
    {
        (void)_state.GetEnemies().SetZ(m_selected, z);
    }
    ImGui::Text("state %d  stateTicks %d  counter %d", pEnemy->state, pEnemy->stateTicks, pEnemy->counter);
    ImGui::Text("aux %d %d %d %d", pEnemy->aux, pEnemy->aux2, pEnemy->aux3, pEnemy->aux4);
    ImGui::Text("delay %d  shots %d  melee %d  flash %d", pEnemy->delayTicks, pEnemy->shotsFired, pEnemy->meleeTicksLeft, pEnemy->hitFlashTicks);
    if (pEnemy->clip >= 0 && pEnemy->clip < static_cast<int>(def.clips.size()))
    {
        ImGui::Text("clip %s  t%d", def.clips[static_cast<size_t>(pEnemy->clip)].name.c_str(), pEnemy->clipTicks);
    }
}

void EnemyLab::HandleMouse_(
    Renderer&     _renderer,
    SandboxState& _state)
{
    float                      mouseX  = 0.f;
    float                      mouseY  = 0.f;
    const SDL_MouseButtonFlags buttons = SDL_GetMouseState(&mouseX, &mouseY);
    const bool                 bDown   = (buttons & SDL_BUTTON_LMASK) != 0;
    const bool                 bClick  = bDown && !m_bPrevMouseDown;
    m_bPrevMouseDown                   = bDown;
    if (!bClick || ImGui::GetIO().WantCaptureMouse || m_bMouseCaptured)
    {
        return;
    }
    float logicalX = 0.f;
    float logicalY = 0.f;
    if (!SDL_RenderCoordinatesFromWindow(_renderer.GetNative(), mouseX, mouseY, &logicalX, &logicalY))
    {
        return;
    }
    if (logicalX < 0.f || logicalY < 0.f || logicalX >= static_cast<float>(Renderer::kLogicalWidth) || logicalY >= static_cast<float>(Renderer::kLogicalHeight))
    {
        return;
    }
    const Vec2                 camera = _state.GetCamera().GetPosition();
    const Vec2                 world  = { logicalX + std::floor(camera.x), logicalY + std::floor(camera.y) };
    const Span<const EnemyDef> defs   = _state.GetEnemyLib().GetDefs();
    if (m_bPlaceMode && m_defIndex >= 0 && m_defIndex < static_cast<int>(defs.size()))
    {
        m_selected = Spawn(_state, defs[static_cast<size_t>(m_defIndex)].name, world, m_facing, m_spawnZ);
    }
    else
    {
        SelectAt(_state, world);
    }
}

void EnemyLab::DrawMark(
    Renderer&        _renderer,
    const Aabb&      _world,
    const Vec2       _anchor,
    const RGBA       _color,
    const StringView _label) const
{
    const SandboxState* pState = GetSandbox();
    if (pState == nullptr)
    {
        return;
    }
    const Vec2 camRaw = pState->GetCamera().GetPosition();
    const Vec2 camera = { std::floor(camRaw.x), std::floor(camRaw.y) };
    for (int i = 0; i < 3; ++i)
    {
        const float grow = static_cast<float>(i);
        DrawBox(_renderer, Aabb::FromMinMax({ _world.min.x - grow, _world.min.y - grow }, { _world.max.x + grow, _world.max.y + grow }), camera, _color);
    }
    const Vec2 a = ToScreen(_anchor, camera);
    _renderer.DrawLine({ a.x - 6.f, a.y }, { a.x + 6.f, a.y }, _color);
    _renderer.DrawLine({ a.x, a.y - 6.f }, { a.x, a.y + 6.f }, _color);
    const Vec2 topLeft = ToScreen(_world.min, camera);
    LabDrawText_(_renderer, { topLeft.x, topLeft.y - 14.f }, _label, _color);
}

void EnemyLab::DrawOverlay(
    Renderer& _renderer)
{
    ++m_frame;
    if (!m_autoSpawn.empty() && m_frame == 3 && !GetSandbox())
    {
        EnterLab();
    }
    SandboxState* pState = GetSandbox();
    if (!pState)
    {
        return;
    }
    if (!m_autoSpawn.empty() && m_frame == 5)
    {
        m_selected = SpawnNearPlayer(*pState, m_autoSpawn, kLabPlayerGap, -1);
    }
    HandleMouse_(_renderer, *pState);
    PollDefFiles_(*pState);

    const Vec2 camRaw = pState->GetCamera().GetPosition();
    const Vec2 camera = { std::floor(camRaw.x), std::floor(camRaw.y) };
    const Vec2 player = pState->GetPlayer().GetBodyBox().GetCenter();

    if (m_bShowCamera)
    {
        DrawBox(_renderer, pState->GetCamera().GetBounds(), camera, { 0, 200, 255, 255 });
    }
    if (m_bShowBoxes)
    {
        for (const Hurtbox& hurtbox: pState->GetCombat().GetHurtboxes())
        {
            if (IsEnemyOwner(hurtbox.owner))
            {
                DrawBox(_renderer, hurtbox.box, camera, hurtbox.bInvulnerable ? RGBA { 120, 200, 255, 255 } : RGBA { 60, 255, 90, 255 });
            }
        }
        for (const Hitbox& hitbox: pState->GetCombat().GetHitboxes())
        {
            if (hitbox.team == eTeam::Enemy)
            {
                DrawBox(_renderer, hitbox.box, camera, { 255, 50, 50, 255 });
            }
        }
    }

    const EnemyDefLibrary& lib    = pState->GetEnemyLib();
    const GroundMap&       ground = pState->GetGround();
    for (const Enemy& e: pState->GetEnemies().GetEnemies())
    {
        if (e.life == eEnemyLife::Pending || e.life == eEnemyLife::Gone)
        {
            continue;
        }
        const Aabb body   = e.body.GetBox();
        const Vec2 center = body.GetCenter();
        if (m_bShowBoxes)
        {
            DrawBox(_renderer, body, camera, { 255, 220, 40, 255 });
        }
        if (e.id == m_selected.id)
        {
            const Aabb mark = Aabb::FromMinMax({ body.min.x - 2.f, body.min.y - 2.f }, { body.max.x + 2.f, body.max.y + 2.f });
            DrawBox(_renderer, mark, camera, { 255, 255, 255, 255 });
        }
        if (m_bShowSolids)
        {
            const EnemyDef& solidDef = lib.Get(e.defIndex);
            if (!solidDef.solid.IsEmpty() && e.life == eEnemyLife::Alive)
            {
                const Aabb solid = solidDef.solid.At(e.body.position, e.facing);
                DrawBox(_renderer, solid, camera, { 255, 0, 255, 255 });
                const Vec2 solidTop = ToScreen(solid.min, camera);
                LabDrawText_(_renderer, { solidTop.x + 2.f, solidTop.y + 2.f }, "solid", { 255, 0, 255, 255 });
            }
            if (solidDef.behavior == eEnemyBehavior::Bus)
            {
                const Vec2 a  = { e.body.position.x + solidDef.bus.stairsBase.x, e.body.position.y + solidDef.bus.stairsBase.y };
                const Vec2 b  = { e.body.position.x + solidDef.bus.stairsTop.x, e.body.position.y + solidDef.bus.stairsTop.y };
                const Vec2 lo = ToScreen({ Min(a.x, b.x), Min(a.y, b.y) }, camera);
                const Vec2 hi = ToScreen({ Max(a.x, b.x), Max(a.y, b.y) }, camera);
                _renderer.FillRect(lo, { Max(1.f, hi.x - lo.x), Max(1.f, hi.y - lo.y) }, { 255, 160, 0, 70 });
                LabDrawText_(_renderer, { lo.x + 2.f, lo.y + 2.f }, "stairs", { 255, 200, 80, 255 });
            }
        }
        if (m_bShowTarget)
        {
            _renderer.DrawLine(ToScreen(center, camera), ToScreen(player, camera), { 255, 80, 255, 255 });
        }
        if (m_bShowProbe)
        {
            const Vec2 feet  = e.GetFeet();
            const int  x     = static_cast<int>(std::floor(feet.x));
            const int  y     = static_cast<int>(std::floor(feet.y));
            int        depth = kLabProbeDepth;
            for (int dy = 0; dy < kLabProbeDepth; ++dy)
            {
                if (ground.IsSolid(x, y + dy))
                {
                    depth = dy;
                    break;
                }
            }
            const RGBA color = depth < kLabProbeDepth ? RGBA { 0, 255, 255, 255 } : RGBA { 255, 120, 0, 255 };
            _renderer.DrawLine(ToScreen(feet, camera), ToScreen({ feet.x, feet.y + static_cast<float>(depth) }, camera), color);
        }
        if (m_bShowState)
        {
            const EnemyDef& def  = lib.Get(e.defIndex);
            const Vec2      top  = ToScreen({ center.x, body.min.y }, camera);
            const float     half = static_cast<float>(def.name.size() * Renderer::kDebugCharSize) * 0.5f;
            LabDrawText_(_renderer, { top.x - half, top.y - 20.f }, def.name, { 255, 255, 255, 255 });
            const String text = LabMakeStateText_(e);
            LabDrawText_(_renderer, { top.x - static_cast<float>(text.size() * Renderer::kDebugCharSize) * 0.5f, top.y - 10.f }, text, { 255, 255, 120, 255 });
        }
    }
}
