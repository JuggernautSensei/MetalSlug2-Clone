#pragma once
#include <vector>

#include "Game/Combat/CombatSystem.h"
#include "Game/Combat/Projectiles.h"
#include "Game/Enemies/EnemyDef.h"
#include "Game/Enemies/EnemyWorld.h"
#include "PlayerTestUtil.h"
#include "World/Constants.h"

// ===========================================
//  적 테스트 도우미
//   EnemyRig: 적 월드만 따로 돌린다 (플레이어는 판정 상자만 있는 가짜 표적). 틱 순서는 SandboxState 와 같다.
//   샌드박스 Sim: 실제 PlayerController + SandboxState 를 SimRunner 로 돌린다 (EnemyDefsPath 로 정의를 읽는다).
// ===========================================

namespace slugtest
{

using namespace slug;

inline jug::FilePath EnemyDefsPath()
{
    return jug::FilePath { SLUG_ASSET_DIR } / "Defs" / "Enemies.json";
}

inline bool LoadEnemyLib(
    EnemyDefLibrary& _out,
    jug::String*     _pMessage = nullptr)
{
    jug::Result<EnemyDefLibrary> lib = EnemyDefLibrary::Load(EnemyDefsPath(), _pMessage);
    if (!lib)
    {
        return false;
    }
    _out = lib.Take();
    return true;
}

// 샌드박스 설정: 평지 + 표적 없음 + 적 정의 파일
inline SandboxConfig MakeEnemyConfig(
    const jug::VECTOR2 _spawn = { 100.f, 378.f })
{
    SandboxConfig config = MakeFlatConfig();
    config.spawnFeet     = _spawn;
    config.enemyDefPath  = EnemyDefsPath();
    return config;
}

// 보병 정의 8종 (숫자키 1..8 과 같은 순서)
inline constexpr const char* kInfantryNames[] = { "soldier_bomb", "soldier_sword", "soldier_missile", "soldier_bzk", "soldier_hang", "flying_bzk", "shielder", "pow" };

// 플레이어 오너 ID (SandboxState 의 플레이어 HitResolver 소유자와 같은 규약: 1)
inline constexpr uint32_t kTestPlayerOwner = 1;

class EnemyRig
{
public:
    explicit EnemyRig(
        const jug::VECTOR2 _player = { 100.f, 378.f },
        const float        _camLeft = 0.f)
        : m_world(&m_lib, 7)
        , m_ground(MakeFlatGround(3000))
        , m_player(_player)
        , m_view(Aabb::FromMinSize({ _camLeft, 0.f }, { 640.f, 448.f }))
    {
        m_bLoaded = LoadEnemyLib(m_lib, &m_message);
        m_world.SetLibrary(&m_lib);
    }

    [[nodiscard]] bool               IsLoaded() const { return m_bLoaded; }
    [[nodiscard]] const jug::String& GetMessage() const { return m_message; }

    EnemyWorld&        World() { return m_world; }
    EnemyDefLibrary&   Lib() { return m_lib; }
    CombatSystem&      Combat() { return m_combat; }
    ProjectileSystem&  Projectiles() { return m_projectiles; }
    std::vector<EnemyEvent>& Events() { return m_events; }

    void SetView(const float _camLeft)
    {
        m_view = Aabb::FromMinSize({ _camLeft, 0.f }, { 640.f, 448.f });
    }

    EnemyHandle Spawn(
        const char*        _name,
        const jug::VECTOR2 _feet,
        const SpawnParams& _params = {})
    {
        const PhysicsWorld world = MakeWorld_();
        return m_world.Spawn(_name, _feet, _params, &world);
    }

    // 플레이어 판정 상자 + 적 월드 한 틱 (SandboxState 순서: 판정 등록 → 적 → 투사체 → 해소 → 이벤트)
    void Step(const int _count = 1)
    {
        for (int i = 0; i < _count; ++i)
        {
            m_combat.BeginTick();
            RegisterPlayer_();
            m_world.RegisterHurtboxes(m_combat);
            const EnemyContext ctx = MakeCtx_();
            m_world.Tick(ctx);
            m_projectiles.Step(&m_ground, m_combat, kStepSec);
            const jug::Vector<HitEvent>& events = m_combat.Resolve();
            CountPlayerHits_(events);
            m_world.OnHitEvents(events);
            CollectEvents_();
        }
    }

    // 플레이어(소유자 1)가 맞은 HitEvent 누적 수 (적 공격이 플레이어에게 닿았는지 확인용)
    [[nodiscard]] int PlayerHits() const { return m_playerHits; }

    // 소유자 _owner 의 첫 Hurtbox 와 겹치는 한 번짜리 판정을 즉시 해소한다. 반환: 이번 판정의 HitEvent 수
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
        const jug::Vector<HitEvent>& events = m_combat.Resolve();
        const size_t count = events.size();
        m_world.OnHitEvents(events);
        CollectEvents_();
        return count;
    }

    int CountEvents(const eEnemyEventKind _kind) const
    {
        int count = 0;
        for (const EnemyEvent& event: m_events)
        {
            count += event.kind == _kind ? 1 : 0;
        }
        return count;
    }

    int SumScore(const eEnemyEventKind _kind) const
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

    [[nodiscard]] const Enemy* E(const EnemyHandle _handle) const { return m_world.Get(_handle); }

private:
    PhysicsWorld MakeWorld_() const
    {
        PhysicsWorld world;
        world.pGround = &m_ground;
        return world;
    }

    EnemyContext MakeCtx_()
    {
        EnemyContext ctx;
        ctx.physics            = MakeWorld_();
        ctx.pCombat            = &m_combat;
        ctx.pProjectiles       = &m_projectiles;
        ctx.player.feet        = m_player;
        ctx.player.center      = { m_player.x, m_player.y - 30.f };
        ctx.player.bodyBox     = Aabb::FromBottomCenter(m_player, { 20.f, 58.f });
        ctx.player.bAlive      = true;
        ctx.player.bInvulnerable = false;
        ctx.cameraView         = m_view;
        ctx.pAnims             = nullptr;
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

    void CountPlayerHits_(const jug::Vector<HitEvent>& _events)
    {
        for (const HitEvent& event: _events)
        {
            if (event.target == kTestPlayerOwner)
            {
                ++m_playerHits;
            }
        }
    }

    EnemyDefLibrary          m_lib       = {};
    int                      m_playerHits = 0;
    jug::String              m_message   = {};
    bool                     m_bLoaded   = false;
    EnemyWorld               m_world;
    GroundMap                m_ground;
    jug::VECTOR2             m_player;
    Aabb                     m_view;
    CombatSystem             m_combat    = {};
    ProjectileSystem         m_projectiles = {};
    std::vector<EnemyEvent>  m_events    = {};
};

}   // namespace slugtest
