#pragma once
#include "Core/AssetPath.h"
#include <cmath>
#include "Base/Types.h"
#include "Base/Math.h"
#include "Base/Result.h"
#include <filesystem>
#include <string>
#include <system_error>
#include <vector>

#include <gtest/gtest.h>

#include "Game/Combat/CombatSystem.h"
#include "Game/Combat/Projectiles.h"
#include "Game/Enemies/BattleShip.h"
#include "Game/Enemies/EnemyDef.h"
#include "Game/Enemies/EnemyWorld.h"
#include "Render/Animation.h"
#include "Render/SpriteLibrary.h"
#include "World/Constants.h"

namespace slugtest
{

using namespace slug;

inline constexpr uint32_t kTestPlayerOwner = 1;

inline std::filesystem::path BossDefsDir()
{
    static int sCounter = 0;
    const std::filesystem::path defs = slug::FindAssetRoot() / "Defs";
    const std::filesystem::path dir  = std::filesystem::temp_directory_path() / ("slug_boss_test_defs_" + std::to_string(++sCounter));
    std::error_code             ec;
    std::filesystem::create_directories(dir, ec);
    for (const char* name: { "Enemies.json", "Boss.json" })
    {
        bool bCopied = false;
        for (int attempt = 0; attempt < 5 && !bCopied; ++attempt)
        {
            bCopied = std::filesystem::copy_file(defs / name, dir / name, std::filesystem::copy_options::overwrite_existing, ec) && !ec;
        }
        if (!bCopied)
        {
            return {};
        }
    }
    return dir;
}

inline bool LoadBossLibrary(
    EnemyDefLibrary& _out)
{
    const std::filesystem::path dir = BossDefsDir();
    if (dir.empty())
    {
        return false;
    }
    slug::Result<EnemyDefLibrary> lib = EnemyDefLibrary::LoadDirectory(dir);
    if (!lib)
    {
        return false;
    }
    _out = lib.Take();
    return true;
}

class BossRig
{
public:
    BossRig()
    {
        m_ok = LoadBossLibrary(m_lib);
        m_world.SetLibrary(&m_lib);
        m_ctx.pCombat      = &m_combat;
        m_ctx.pProjectiles = &m_projectiles;
        m_ctx.cameraView   = Aabb::FromMinSize({ -20000.f, -20000.f }, { 40000.f, 40000.f });   // 화면 제약 없음
        m_ctx.pAnims       = nullptr;
    }

    [[nodiscard]] bool IsReady() const { return m_ok; }
    [[nodiscard]] const EnemyDefLibrary& GetLib() const { return m_lib; }
    [[nodiscard]] EnemyWorld&            World() { return m_world; }
    [[nodiscard]] ProjectileSystem&      Projectiles() { return m_projectiles; }
    [[nodiscard]] CombatSystem&          Combat() { return m_combat; }
    [[nodiscard]] const std::vector<EnemyEvent>& Events() const { return m_events; }
    [[nodiscard]] const std::vector<HitEvent>&   PlayerHits() const { return m_playerHits; }

    void SetPlayer(const Vec2 _feet, const bool _bAlive = true)
    {
        m_feet   = _feet;
        m_bAlive = _bAlive;
    }

    void SetPlayerInvulnerable(const bool _b) { m_bInvulnerable = _b; }

    void Frame()
    {
        m_ctx.player.feet          = m_feet;
        m_ctx.player.center        = { m_feet.x, m_feet.y - 30.f };
        m_ctx.player.bodyBox       = Aabb::FromCenterSize(m_ctx.player.center, { 24.f, 60.f });
        m_ctx.player.bAlive        = m_bAlive;
        m_ctx.player.bInvulnerable = m_bInvulnerable;

        m_combat.BeginTick();
        m_world.RegisterHurtboxes(m_combat);
        Hurtbox player;
        player.box           = m_ctx.player.bodyBox;
        player.team          = eTeam::Player;
        player.owner         = kTestPlayerOwner;
        player.bInvulnerable = m_bInvulnerable;
        m_combat.AddHurtbox(player);

        m_world.Tick(m_ctx);
        m_projectiles.Step(nullptr, m_combat, kStepSec);
        const slug::Vector<HitEvent>& events = m_combat.Resolve();
        m_world.OnHitEvents(events);
        m_projectiles.OnHitEvents(events, m_combat);
        for (const HitEvent& ev: events)
        {
            if (ev.target == kTestPlayerOwner)
            {
                m_playerHits.push_back(ev);
            }
        }
        slug::Vector<EnemyEvent> taken = m_world.TakeEvents();
        for (const EnemyEvent& ev: taken)
        {
            m_events.push_back(ev);
        }
        ++m_tick;
    }

    void Run(const int _ticks)
    {
        for (int i = 0; i < _ticks; ++i)
        {
            Frame();
        }
    }

    [[nodiscard]] int Tick() const { return m_tick; }

    [[nodiscard]] int CountEvents(const eEnemyEventKind _kind) const
    {
        int n = 0;
        for (const EnemyEvent& ev: m_events)
        {
            n += ev.kind == _kind ? 1 : 0;
        }
        return n;
    }

    [[nodiscard]] int CountEventsOf(const eEnemyEventKind _kind, const EnemyHandle _h) const
    {
        int n = 0;
        for (const EnemyEvent& ev: m_events)
        {
            n += (ev.kind == _kind && ev.handle == _h) ? 1 : 0;
        }
        return n;
    }

    void Hit(const uint32_t _target, const int _damage)
    {
        HitEvent ev;
        ev.damage       = _damage;
        ev.kind         = eHitKind::Bullet;
        ev.attackerTeam = eTeam::Player;
        ev.attacker     = kTestPlayerOwner;
        ev.target       = _target;
        ev.hitId        = 0;
        m_world.OnHitEvents(slug::Span<const HitEvent> { &ev, 1 });
        for (const EnemyEvent& e: m_world.TakeEvents())
        {
            m_events.push_back(e);
        }
    }

    [[nodiscard]] const Enemy* Find(const EnemyHandle _h) const { return m_world.Get(_h); }

    [[nodiscard]] EnemyHandle SpawnAt(const slug::StringView _def, const Vec2 _feet, const SpawnParams& _params = {})
    {
        return m_world.Spawn(_def, _feet, _params, nullptr);
    }

private:
    bool                      m_ok          = false;
    EnemyDefLibrary           m_lib         = {};
    EnemyWorld                m_world {};
    CombatSystem              m_combat      = {};
    ProjectileSystem          m_projectiles = {};
    EnemyContext              m_ctx         = {};
    Vec2              m_feet        = { 0.f, 0.f };
    bool                      m_bAlive      = true;
    bool                      m_bInvulnerable = true;
    int                       m_tick        = 0;
    std::vector<EnemyEvent>   m_events      = {};
    std::vector<HitEvent>     m_playerHits  = {};
};

inline Aabb BossArena()
{
    return Aabb::FromMinMax({ 0.f, 0.f }, { 640.f, 448.f });
}
}
