#pragma once
#include "Base/Handle.h"
#include <cstdint>
#include <optional>

#include "Base/Random.h"
#include <memory>
#include "Base/Types.h"
#include "Base/Math.h"

#include "Core/Tuning.h"
#include "Game/Combat/CombatSystem.h"
#include "Game/Combat/Projectiles.h"
#include "Game/Enemies/EnemyDef.h"
#include "Game/Enemies/EnemyWorld.h"
#include "Game/GameState.h"
#include "Game/Player/PlayerController.h"
#include "Game/Player/PlayerRig.h"
#include "Game/Props/PropDef.h"
#include "Game/Props/PropWorld.h"
#include "Game/Sandbox/Effects.h"
#include "Game/Stage/StageData.h"
#include "Game/Stage/StageRunner.h"
#include "Game/Stage/StageWorldHost.h"
#include "Game/Ui/ScreenFlow.h"
#include "Game/Ui/UiSystem.h"
#include "Render/Camera.h"
#include "Render/SpriteLibrary.h"
#include "World/GroundMap.h"
#include "World/Physics.h"

namespace slug
{
class StageState final : public IGameState
{
public:
    StageState();

    void OnEnter(GameContext& _ctx) override;
    void OnExit(GameContext& _ctx) override;
    void Tick(GameContext& _ctx, const InputHistory& _input) override;
    void Render(GameContext& _ctx, float _alpha) override;

    [[nodiscard]] std::unique_ptr<IGameState> TakeNextState() override;
    [[nodiscard]] bool                    IsFinished() const override;

    [[nodiscard]] bool                     IsLoaded() const { return m_bLoaded; }
    [[nodiscard]] const StageData&         GetData() const { return m_data; }
    [[nodiscard]] const StageRunner&       GetRunner() const { return *m_runner; }
    [[nodiscard]] const StageWorldHost&    GetHost() const { return m_host; }
    [[nodiscard]] PlayerController&        GetPlayer() { return m_player; }
    [[nodiscard]] const PlayerController&  GetPlayer() const { return m_player; }
    [[nodiscard]] const Camera&            GetCamera() const { return m_camera; }
    [[nodiscard]] const CombatSystem&      GetCombat() const { return m_combat; }
    [[nodiscard]] const EnemyWorld&        GetEnemies() const { return m_enemies; }
    [[nodiscard]] EnemyWorld&              GetEnemies() { return m_enemies; }
    [[nodiscard]] const PropWorld&         GetProps() const { return m_props; }
    [[nodiscard]] PropWorld&               GetProps() { return m_props; }
    [[nodiscard]] const UiSystem&          GetUi() const { return m_ui; }
    [[nodiscard]] UiSystem&                GetUi() { return m_ui; }
    [[nodiscard]] uint64_t                 GetTickCount() const { return m_tickCount; }
    [[nodiscard]] uint64_t                 HashState() const;
    [[nodiscard]] HudModel                 BuildHudModel() const;

private:
    enum class eEndKind : uint8_t
    {
        None,
        Mission,
        GameOver
    };

    struct PatchMask
    {
        slug::String name    = {};
        bool        bLoaded = false;
        GroundMap   mask    = {};
    };

    void Load_(GameContext& _ctx);
    void Restart_(GameContext& _ctx);
    void HandleDebugWarp_(GameContext& _ctx);
    void StartFlow_(GameContext& _ctx, eEndKind _kind);
    [[nodiscard]] bool IsFlowing_() const { return m_endKind != eEndKind::None && !m_flow.IsDone(); }

    void UpdateWalls_();
    [[nodiscard]] PhysicsWorld MakePhysicsWorld_() const;
    [[nodiscard]] PhysicsWorld MakeEnemyPhysicsWorld_() const;
    [[nodiscard]] Vec2 GetRespawnFeet_() const;

    void ConsumeEnemyEvents_(GameContext& _ctx);
    void ConsumePropEvents_(GameContext& _ctx);
    void ApplyGroundPatch_(GameContext& _ctx, const slug::String& _sprite, Vec2 _topLeft);
    [[nodiscard]] const GroundMap* FindPatchMask_(GameContext& _ctx, const slug::String& _sprite);
    void RequestSounds_(GameContext& _ctx, const slug::Vector<FxRequest>& _fx);

    void DrawBackdrop_(GameContext& _ctx, Vec2 _cameraPos);
    void DrawWorld_(GameContext& _ctx, Vec2 _cameraPos, float _alpha);
    void DrawDebugBoxes_(Renderer& _renderer, Vec2 _cameraPos) const;
    void DrawHud_(GameContext& _ctx, Vec2 _cameraPos);

    StageData                  m_data    = {};
    bool                       m_bLoaded = false;
    bool                       m_bFailed = false;
    bool                       m_bTakenNext = false;
    slug::FilePath              m_assetRoot = {};
    std::optional<StageRunner> m_runner  = std::nullopt;
    slug::RngMt19937            m_rng     = slug::RngMt19937 { 0x57A6E0ull };

    StageWorldHost    m_host   = {};
    Tuning            m_tuning = {};
    Camera            m_camera = {};
    Vec2      m_prevCameraPos = { 0.f, 0.f };
    Vec2      m_startFeet     = { 0.f, 0.f };

    slug::Vector<Wall>     m_walls         = {};
    slug::Vector<Wall>     m_enemyWalls    = {};
    slug::Vector<Platform> m_playerPlatforms = {};
    slug::Vector<Platform> m_enemyPlatforms  = {};

    EnemyDefLibrary  m_enemyLib = {};
    PropDefLibrary   m_propDefs = {};
    EnemyWorld       m_enemies  { &m_enemyLib };
    PropWorld        m_props    { nullptr, 1 };

    PlayerController m_player      = {};
    PlayerRig        m_rig         = {};
    CombatSystem     m_combat      = {};
    ProjectileSystem m_projectiles = {};
    EffectList       m_effects     = {};
    AnimPlayer       m_bombAnim    = {};

    slug::Vector<PatchMask> m_patchMasks = {};

    SpriteLibrary m_spriteLib = {};
    int           m_bgHandle  = -1;
    slug::Vector<int> m_backdropHandles = {};
    bool          m_bRigBound = false;
    AnimId        m_bombClip  = slug::kNullHandle;

    UiSystem    m_ui         = {};
    ScreenFlow  m_flow       = {};
    eEndKind    m_endKind    = eEndKind::None;
    PlayerStats m_prevStats  = {};
    eWeapon     m_prevWeapon = eWeapon::Pistol;
    int         m_prevScore  = 0;

    uint64_t m_tickCount = 0;
};
}
