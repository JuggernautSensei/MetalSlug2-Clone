// 탈것(Jeep/BTank/MissleTank/Sarubia) 테스트 공용 도우미: 정의 로드와 VehicleRig(적 월드 + 전투 + 투사체 틱 하네스).
#pragma once
#include <cstdint>
#include <vector>

#include "Base/Math.h"
#include "Base/Result.h"
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
    String*    _pMessage = nullptr);

inline constexpr ARRAY<const char*, 4> kVehicleNames = { "jeep", "btank", "missle_tank", "sarubia" };

inline constexpr float kVehicleGroundY = 378.f;

class VehicleRig
{
public:
    explicit VehicleRig(
        const Vec2  _player  = { 100.f, 378.f },
        const float _camLeft = 0.f);

    [[nodiscard]] bool                IsLoaded() const;
    [[nodiscard]] const String& GetMessage() const;

    [[nodiscard]] EnemyWorld&              World();
    [[nodiscard]] EnemyDefLibrary&         Lib();
    [[nodiscard]] CombatSystem&            Combat();
    [[nodiscard]] ProjectileSystem&        Projectiles();
    [[nodiscard]] std::vector<EnemyEvent>& Events();

    void SetPlayer(
        const Vec2 _feet);

    void SetView(
        const float _camLeft);

    EnemyHandle Spawn(
        const char*        _name,
        const Vec2         _feet,
        const SpawnParams& _params = {});

    void Step(
        const int _count = 1);

    [[nodiscard]] int Ticks() const;

    size_t Strike(
        const uint32_t _owner,
        const eHitKind _kind,
        const int      _damage,
        const eTeam    _team = eTeam::Player);

    [[nodiscard]] int CountEvents(
        const eEnemyEventKind _kind) const;

    [[nodiscard]] int SumScore(
        const eEnemyEventKind _kind) const;

    [[nodiscard]] int CountShots(
        const EnemyHandle _owner) const;

    [[nodiscard]] int CountAllShots() const;

    [[nodiscard]] const Enemy* E(
        const EnemyHandle _handle) const;

    [[nodiscard]] int PlayerHits() const;

private:
    [[nodiscard]] PhysicsWorld MakeWorld_() const;

    [[nodiscard]] EnemyContext MakeCtx_();

    void RegisterPlayer_();

    void CollectEvents_();

    void CountPlayerHits_(
        const Vector<HitEvent>& _events);

    EnemyDefLibrary         m_lib        = {};
    int                     m_playerHits = 0;
    int                     m_ticks      = 0;
    String            m_message    = {};
    bool                    m_bLoaded    = false;
    EnemyWorld              m_world;
    GroundMap               m_ground;
    Vec2                    m_player;
    Aabb                    m_view;
    CombatSystem            m_combat      = {};
    ProjectileSystem        m_projectiles = {};
    std::vector<EnemyEvent> m_events      = {};
    std::vector<Wall>       m_walls       = {};
};
