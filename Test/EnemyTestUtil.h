#pragma once

#include <vector>

#include "Base/SlugError.h"
#include "Base/Types.h"
#include "Core/AssetPath.h"
#include "Game/Combat/CombatSystem.h"
#include "Game/Combat/Projectiles.h"
#include "Game/Enemies/EnemyDef.h"
#include "Game/Enemies/EnemyWorld.h"
#include "PlayerTestUtil.h"
#include "Render/Animation.h"
#include "World/Constants.h"

[[nodiscard]] FilePath EnemyDefsPath();

[[nodiscard]] bool OnGround(const EnemyState* _pEnemy);

bool LoadEnemyLib(
    EnemyDefLibrary& _out,
    String*          _pMessage = nullptr);

[[nodiscard]] SandboxConfig MakeEnemyConfig(
    const VECTOR2 _spawn = { 100.f, 378.f });

inline constexpr ARRAY<const char*, 8> kInfantryNames = { "soldier_bomb", "soldier_sword", "soldier_missile", "soldier_bzk", "soldier_hang", "flying_bzk", "shielder", "pow" };

inline constexpr uint32_t kTestPlayerOwner = 1;

class EnemyRig
{
public:
    explicit EnemyRig(
        const VECTOR2 _player  = { 100.f, 378.f },
        const float   _camLeft = 0.f);

    [[nodiscard]] bool          IsLoaded() const;
    [[nodiscard]] const String& GetLoadMessage() const;

    [[nodiscard]] EnemyWorld&         World();
    [[nodiscard]] const EnemyWorld&   World() const;
    [[nodiscard]] EnemyDefLibrary&    Lib();
    [[nodiscard]] CombatSystem&       Combat();
    [[nodiscard]] ProjectileSystem&   Projectiles();
    [[nodiscard]] Vector<EnemyEvent>& Events();

    void SetView(
        const float _camLeft);

    void SetPlayer(
        const VECTOR2 _feet);

    void SetAnims(
        const AnimationLibrary* _pAnims);

    [[nodiscard]] EnemyHandle Spawn(
        const char*        _name,
        const VECTOR2      _feet,
        const SpawnParams& _params = {});

    void Step(
        const int _count = 1);

    [[nodiscard]] int PlayerHits() const;

    size_t Strike(
        const uint32_t _owner,
        const eHitKind _kind,
        const int      _damage,
        const eTeam    _team = eTeam::Player);

    [[nodiscard]] int CountEvents(
        const eEnemyEventKind _kind) const;

    [[nodiscard]] int SumScore(
        const eEnemyEventKind _kind) const;

    [[nodiscard]] const EnemyState* E(
        const EnemyHandle _handle) const;

private:
    [[nodiscard]] PhysicsWorld MakeWorld_() const;

    [[nodiscard]] EnemyContext MakeCtx_();

    void RegisterPlayer_();

    void CollectEvents_();

    void CountPlayerHits_(
        const Vector<HitEvent>& _events);

    EnemyDefLibrary         m_lib        = {};
    int                     m_playerHits = 0;
    String                  m_message    = {};
    bool                    m_bLoaded    = false;
    EnemyWorld              m_world;
    GroundMap               m_ground;
    VECTOR2                 m_player = {};
    Aabb                    m_view;
    const AnimationLibrary* m_pAnims      = nullptr;
    CombatSystem            m_combat      = {};
    ProjectileSystem        m_projectiles = {};
    Vector<EnemyEvent>      m_events      = {};
};
