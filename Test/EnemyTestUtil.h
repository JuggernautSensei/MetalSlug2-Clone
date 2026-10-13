#pragma once

#include <vector>

#include "Base/Math.h"
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

[[nodiscard]] bool OnGround(const Enemy* _pEnemy);

bool LoadEnemyLib(
    EnemyDefLibrary& _out,
    String*          _pMessage = nullptr);

[[nodiscard]] SandboxConfig MakeEnemyConfig(
    VECTOR2 _spawn = { 100.f, 378.f });

inline constexpr ARRAY<const char*, 8> kInfantryNames = { "soldier_bomb", "soldier_sword", "soldier_missile", "soldier_bzk", "soldier_hang", "flying_bzk", "shielder", "pow" };

inline constexpr uint32_t kTestPlayerOwner = 1;

class EnemyRig
{
public:
    explicit EnemyRig(
        VECTOR2 _player  = { 100.f, 378.f },
        float   _camLeft = 0.f);

    [[nodiscard]] bool          IsLoaded() const;
    [[nodiscard]] const String& GetMessage() const;

    [[nodiscard]] EnemyWorld&         World();
    [[nodiscard]] EnemyDefLibrary&    Lib();
    [[nodiscard]] CombatSystem&       Combat();
    [[nodiscard]] ProjectileSystem&   Projectiles();
    [[nodiscard]] Vector<EnemyEvent>& Events();

    void SetView(
        float _camLeft);

    void SetPlayer(
        VECTOR2 _feet);

    void SetAnims(
        const AnimationLibrary* _pAnims);

    [[nodiscard]] EnemyHandle Spawn(
        const char*        _name,
        VECTOR2            _feet,
        const SpawnParams& _params = {});

    void Step(
        int _count = 1);

    [[nodiscard]] int PlayerHits() const;

    size_t Strike(
        uint32_t _owner,
        eHitKind _kind,
        int      _damage,
        eTeam    _team = eTeam::Player);

    [[nodiscard]] int CountEvents(
        eEnemyEventKind _kind) const;

    [[nodiscard]] int SumScore(
        eEnemyEventKind _kind) const;

    [[nodiscard]] const Enemy* E(
        EnemyHandle _handle) const;

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
