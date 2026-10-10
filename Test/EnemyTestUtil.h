#pragma once

// 적 테스트 공용 도구: 정의 로드, 샌드박스 설정, 월드를 직접 구동하는 EnemyRig.

#include <vector>

#include "Base/Math.h"
#include "Base/Result.h"
#include "Base/Types.h"
#include "Core/AssetPath.h"
#include "Game/Combat/CombatSystem.h"
#include "Game/Combat/Projectiles.h"
#include "Game/Enemies/EnemyDef.h"
#include "Game/Enemies/EnemyWorld.h"
#include "PlayerTestUtil.h"
#include "World/Constants.h"

namespace slugtest
{

using namespace slug;

[[nodiscard]] inline slug::FilePath EnemyDefsPath()
{
    return slug::FindAssetRoot() / "Defs" / "Enemies.json";
}

inline bool LoadEnemyLib(
    EnemyDefLibrary& _out,
    slug::String*    _pMessage = nullptr)
{
    slug::Result<EnemyDefLibrary> lib = EnemyDefLibrary::Load(EnemyDefsPath(), _pMessage);
    if (!lib)
    {
        return false;
    }
    _out = lib.Take();
    return true;
}

[[nodiscard]] inline SandboxConfig MakeEnemyConfig(
    const Vec2 _spawn = { 100.f, 378.f })
{
    SandboxConfig config = MakeFlatConfig();
    config.spawnFeet     = _spawn;
    config.enemyDefPath  = slug::FindAssetRoot() / "Defs";
    return config;
}

inline constexpr ARRAY<const char*, 8> kInfantryNames = { "soldier_bomb", "soldier_sword", "soldier_missile", "soldier_bzk", "soldier_hang", "flying_bzk", "shielder", "pow" };

inline constexpr uint32_t kTestPlayerOwner = 1;

class EnemyRig
{
public:
    explicit EnemyRig(
        const Vec2  _player  = { 100.f, 378.f },
        const float _camLeft = 0.f)
        : m_world(&m_lib, 7)
        , m_ground(MakeFlatGround(3000))
        , m_player(_player)
        , m_view(Aabb::FromMinSize({ _camLeft, 0.f }, { 640.f, 448.f }))
    {
        m_bLoaded = LoadEnemyLib(m_lib, &m_message);
        m_world.SetLibrary(&m_lib);
    }

    [[nodiscard]] bool                IsLoaded() const { return m_bLoaded; }
    [[nodiscard]] const slug::String& GetMessage() const { return m_message; }

    [[nodiscard]] EnemyWorld&               World() { return m_world; }
    [[nodiscard]] EnemyDefLibrary&          Lib() { return m_lib; }
    [[nodiscard]] CombatSystem&             Combat() { return m_combat; }
    [[nodiscard]] ProjectileSystem&         Projectiles() { return m_projectiles; }
    [[nodiscard]] slug::Vector<EnemyEvent>& Events() { return m_events; }

    void SetView(
        const float _camLeft)
    {
        m_view = Aabb::FromMinSize({ _camLeft, 0.f }, { 640.f, 448.f });
    }

    [[nodiscard]] EnemyHandle Spawn(
        const char*        _name,
        const Vec2         _feet,
        const SpawnParams& _params = {})
    {
        const PhysicsWorld world = MakeWorld_();
        return m_world.Spawn(_name, _feet, _params, &world);
    }

    void Step(
        const int _count = 1)
    {
        for (int i = 0; i < _count; ++i)
        {
            m_combat.BeginTick();
            RegisterPlayer_();
            m_world.RegisterHurtboxes(m_combat);
            const EnemyContext ctx = MakeCtx_();
            m_world.Tick(ctx);
            m_projectiles.Step(&m_ground, m_combat, kStepSec);
            const slug::Vector<HitEvent>& events = m_combat.Resolve();
            CountPlayerHits_(events);
            m_world.OnHitEvents(events);
            CollectEvents_();
        }
    }

    [[nodiscard]] int PlayerHits() const { return m_playerHits; }

    size_t Strike(
        const uint32_t _owner,
        const eHitKind _kind,
        const int      _damage,
        const eTeam    _team = eTeam::Player)
    {
        m_combat.BeginTick();
        RegisterPlayer_();
        m_world.RegisterHurtboxes(m_combat);
        bool bFound = false;
        Aabb box    = {};
        for (const Hurtbox& hurtbox: m_combat.GetHurtboxes())
        {
            if (hurtbox.owner == _owner)
            {
                box    = hurtbox.box;
                bFound = true;
                break;
            }
        }
        if (!bFound)
        {
            return 0;
        }
        Hitbox hitbox;
        hitbox.box    = box;
        hitbox.team   = _team;
        hitbox.kind   = _kind;
        hitbox.damage = _damage;
        hitbox.owner  = _team == eTeam::Player ? kTestPlayerOwner : 2000u;
        hitbox.id     = m_combat.NewHitId();
        m_combat.AddHitbox(hitbox);
        const slug::Vector<HitEvent>& events = m_combat.Resolve();
        const size_t                  count  = events.size();
        m_world.OnHitEvents(events);
        CollectEvents_();
        return count;
    }

    [[nodiscard]] int CountEvents(
        const eEnemyEventKind _kind) const
    {
        int count = 0;
        for (const EnemyEvent& event: m_events)
        {
            count += event.kind == _kind ? 1 : 0;
        }
        return count;
    }

    [[nodiscard]] int SumScore(
        const eEnemyEventKind _kind) const
    {
        int sum = 0;
        for (const EnemyEvent& event: m_events)
        {
            if (event.kind == _kind)
            {
                sum += event.score;
            }
        }
        return sum;
    }

    [[nodiscard]] const Enemy* E(
        const EnemyHandle _handle) const
    {
        return m_world.Get(_handle);
    }

private:
    [[nodiscard]] PhysicsWorld MakeWorld_() const
    {
        PhysicsWorld world;
        world.pGround = &m_ground;
        return world;
    }

    [[nodiscard]] EnemyContext MakeCtx_()
    {
        EnemyContext ctx;
        ctx.physics              = MakeWorld_();
        ctx.pCombat              = &m_combat;
        ctx.pProjectiles         = &m_projectiles;
        ctx.player.feet          = m_player;
        ctx.player.center        = { m_player.x, m_player.y - 30.f };
        ctx.player.bodyBox       = Aabb::FromBottomCenter(m_player, { 20.f, 58.f });
        ctx.player.bAlive        = true;
        ctx.player.bInvulnerable = false;
        ctx.cameraView           = m_view;
        ctx.pAnims               = nullptr;
        return ctx;
    }

    void RegisterPlayer_()
    {
        Hurtbox hurtbox;
        hurtbox.box   = Aabb::FromBottomCenter(m_player, { 16.f, 54.f });
        hurtbox.team  = eTeam::Player;
        hurtbox.owner = kTestPlayerOwner;
        m_combat.AddHurtbox(hurtbox);
    }

    void CollectEvents_()
    {
        for (EnemyEvent& event: m_world.TakeEvents())
        {
            m_events.push_back(std::move(event));
        }
    }

    void CountPlayerHits_(
        const slug::Vector<HitEvent>& _events)
    {
        for (const HitEvent& event: _events)
        {
            if (event.target == kTestPlayerOwner)
            {
                ++m_playerHits;
            }
        }
    }

    EnemyDefLibrary          m_lib        = {};
    int                      m_playerHits = 0;
    slug::String             m_message    = {};
    bool                     m_bLoaded    = false;
    EnemyWorld               m_world;
    GroundMap                m_ground;
    Vec2                     m_player;
    Aabb                     m_view;
    CombatSystem             m_combat      = {};
    ProjectileSystem         m_projectiles = {};
    slug::Vector<EnemyEvent> m_events      = {};
};
}   // namespace slugtest
