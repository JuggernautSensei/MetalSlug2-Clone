#pragma once
#include <cstdint>
#include <vector>

#include "Base/Math.h"
#include "Base/SlugError.h"
#include "Base/Types.h"
#include "Core/AssetPath.h"
#include "EnemyTestUtil.h"
#include "Game/Combat/CombatSystem.h"
#include "Game/Combat/Projectiles.h"
#include "Game/Enemies/EnemyCommon.h"
#include "Game/Enemies/EnemyDef.h"
#include "Game/Enemies/EnemyWorld.h"
#include "PlayerTestUtil.h"
#include "World/Constants.h"

[[nodiscard]] FilePath VehicleDefsDir();

[[nodiscard]] bool LoadVehicleLib(
    EnemyDefLibrary& _out,
    String*          _pMessage = nullptr);

inline constexpr ARRAY<const char*, 4> kVehicleNames = { "jeep", "btank", "missle_tank", "sarubia" };

inline constexpr float kVehicleGroundY = 378.f;

class VehicleRig
{
public:
    explicit VehicleRig(
        VECTOR2 _player  = { 100.f, 378.f },
        float   _camLeft = 0.f);

    [[nodiscard]] bool          IsLoaded() const;
    [[nodiscard]] const String& GetMessage() const;

    [[nodiscard]] EnemyWorld&         World();
    [[nodiscard]] EnemyDefLibrary&    Lib();
    [[nodiscard]] CombatSystem&       Combat();
    [[nodiscard]] ProjectileSystem&   Projectiles();
    [[nodiscard]] Vector<EnemyEvent>& Events();

    void SetPlayer(
        VECTOR2 _feet);

    void SetView(
        float _camLeft);

    EnemyHandle Spawn(
        const char*        _name,
        VECTOR2            _feet,
        const SpawnParams& _params = {});

    void Step(
        int _count = 1);

    [[nodiscard]] int Ticks() const;

    size_t Strike(
        uint32_t _owner,
        eHitKind _kind,
        int      _damage,
        eTeam    _team = eTeam::Player);

    [[nodiscard]] int CountEvents(
        eEnemyEventKind _kind) const;

    [[nodiscard]] int SumScore(
        eEnemyEventKind _kind) const;

    [[nodiscard]] int CountShots(
        EnemyHandle _owner) const;

    [[nodiscard]] const Enemy* E(
        EnemyHandle _handle) const;

    [[nodiscard]] int PlayerHits() const;

private:
    [[nodiscard]] PhysicsWorld MakeWorld_() const;

    [[nodiscard]] EnemyContext MakeCtx_();

    void RegisterPlayer_();

    void CollectEvents_();

    void CountPlayerHits_(
        const Vector<HitEvent>& _events);

    EnemyDefLibrary    m_lib        = {};
    int                m_playerHits = 0;
    int                m_ticks      = 0;
    String             m_message    = {};
    bool               m_bLoaded    = false;
    EnemyWorld         m_world;
    GroundMap          m_ground;
    VECTOR2            m_player = {};
    Aabb               m_view;
    CombatSystem       m_combat      = {};
    ProjectileSystem   m_projectiles = {};
    Vector<EnemyEvent> m_events      = {};
    Vector<Wall>       m_walls       = {};
};
